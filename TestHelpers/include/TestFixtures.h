#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "../Processors/FakeSourceNode.h"

#include <Audio/AudioComponent.h>
#include <Processors/ProcessorGraph/ProcessorGraph.h>
#include <Processors/SourceNode/SourceNode.h>
#include <UI/ControlPanel.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <Processors/GenericProcessor/GenericProcessor.h>

enum class TestSourceNodeType
{
    Fake,
    Base
};

class TestSourceNodeBuilder
{
public:
    TestSourceNodeBuilder (FakeSourceNodeParams params) : 
        mockSourceNodeParams (params),
        sourceNodeType (TestSourceNodeType::Fake),
        dataThreadCreator (nullptr)
    {}

    TestSourceNodeBuilder (DataThreadCreator creator) : 
        dataThreadCreator (creator),
        sourceNodeType (TestSourceNodeType::Base) 
    {}

    SourceNode* buildSourceNode()
    {
        switch (sourceNodeType)
        {
            case TestSourceNodeType::Fake:
            {
                return (SourceNode*) (new FakeSourceNode (mockSourceNodeParams));
            }
            case TestSourceNodeType::Base:
            {
                return new SourceNode ("BaseSourceNode", dataThreadCreator);
            }
            default:
                break;
        }

        return nullptr;
    }

    TestSourceNodeType getTestSourceNodeType() const
    {
        return sourceNodeType;
    }

private:
    FakeSourceNodeParams mockSourceNodeParams;
    DataThreadCreator dataThreadCreator;
    TestSourceNodeType sourceNodeType;
};

class ProcessorTester
{
public:
    ProcessorTester (TestSourceNodeBuilder sourceNodeBuilder)
    {
        // Singletons...
        MessageManager::deleteInstance();

        // initializes the singleton instance
        MessageManager::getInstance();

        // Reset all state so no interactions between tests.
        AccessClass::clearAccessClassStateForTesting();

        //Create LookAndFeel object
        customLookAndFeel = std::make_unique<CustomLookAndFeel>();
        LookAndFeel::setDefaultLookAndFeel (customLookAndFeel.get());

        // All of these sets the global state in AccessClass in their constructors
        audioComponent = std::make_unique<AudioComponent>();
        processorGraph = std::make_unique<ProcessorGraph> (true);
        controlPanel = std::make_unique<ControlPanel> (processorGraph.get(), audioComponent.get(), true);

        SourceNode* snTemp = sourceNodeBuilder.buildSourceNode();
        sourceNodeId = nextProcessorId++;
        snTemp->setNodeId (sourceNodeId);
        snTemp->registerParameters();
        juce::AudioProcessorGraph::Node* n = processorGraph->addNode (
            std::move (std::unique_ptr<AudioProcessor> (snTemp)),
            juce::AudioProcessorGraph::NodeID (sourceNodeId));

        // Create the source node, and place it in the graph
        auto sn = (GenericProcessor*) n->getProcessor();
        sn->setHeadlessMode (true);
        sn->initialize (false);
        sn->setDestNode (nullptr);

        controlPanel->updateRecordEngineList();

        // Refresh everything
        processorGraph->updateSettings (sn);

        controlPanel->colourChanged();
    }

    virtual ~ProcessorTester()
    {
        controlPanel = nullptr;
        processorGraph = nullptr;
        audioComponent = nullptr;

        AccessClass::clearAccessClassStateForTesting();

        DeletedAtShutdown::deleteAll();
        MessageManager::deleteInstance();
    }

    /**
     * Create a new GenericProcessor instance for testing. In addition to creating the processor, it attaches it into
     * the processor graph as needed, and refreshes necessary state. Note that currently, it always attaches this to
     * the FakeSourceNode created in SetUp(), and thus you cannot call this multiple times at this point (though it
     * would not be hard to extend this to create a longer processor graph).
     * @tparam T processor class, derived from GenericProcessor
     * @param args parameters to pass to constructor of the processor, i.e. new FooProcessor(<args>)
     */
    template <
        typename T,
        class... Args,
        typename std::enable_if<std::is_base_of<GenericProcessor, T>::value>::type* = nullptr>
    T* createProcessor (Plugin::Processor::Type processorType, Args&&... args)
    {
        T* ptr = new T (std::forward<Args> (args)...);
        ptr->setProcessorType (processorType);
        ptr->setHeadlessMode (true);

        int nodeId = nextProcessorId++;
        ptr->setNodeId (nodeId);
        ptr->registerParameters();
        processorGraph->addNode (
            std::move (std::unique_ptr<AudioProcessor> (ptr)),
            juce::AudioProcessorGraph::NodeID (nodeId));

        ptr = (T*) processorGraph->getProcessorWithNodeId (nodeId);
        ptr->initialize (false);
        ptr->setDestNode (nullptr);

        // Place the newly created node into the graph
        auto sourceNode = processorGraph->getProcessorWithNodeId (sourceNodeId);
        ptr->setSourceNode (sourceNode);
        sourceNode->setDestNode (ptr);

        // Refresh everything
        processorGraph->updateSettings (ptr);
        return (T*) processorGraph->getProcessorWithNodeId (nodeId);
    }

    void startAcquisition (bool startRecording, bool forceRecording = false)
    {
        if (startRecording)
        {
            // Do it this way to ensure the GUI elements (which apparently control logic) are set properly
            controlPanel->setRecordingState (true, forceRecording);
        }
        else
        {
            controlPanel->startAcquisition (false);
        }
    }

    void stopAcquisition()
    {
        controlPanel->stopAcquisition();
    }

    const DataStream* getProcessorDataStream (uint16 nodeId, uint16 streamId)
    {
        auto streams = processorGraph->getProcessorWithNodeId (nodeId)->getDataStreams();
        for (auto stream : streams)
        {
            if (stream->getStreamId() == streamId)
            {
                return stream;
            }
        }
        return nullptr;
    }

    const DataStream* getSourceNodeDataStream (uint16 streamId)
    {
        return getProcessorDataStream (sourceNodeId, streamId);
    }

    static void setRecordingParentDirectory (const std::string& parent_directory)
    {
        CoreServices::setRecordingParentDirectory (String (parent_directory));
    }

    GenericProcessor* getSourceNode()
    {
        return processorGraph->getProcessorWithNodeId (sourceNodeId);
    }

    AudioBuffer<float> processBlock (
        GenericProcessor* processor,
        const AudioBuffer<float>& buffer,
        TTLEvent* maybeTtlEvent = nullptr)
    {
        auto audioProcessor = (AudioProcessor*) processor;
        auto dataStreams = processor->getDataStreams();

        MidiBuffer eventBuffer;
        for (const auto* datastream : dataStreams)
        {
            HeapBlock<char> data;
            auto streamId = datastream->getStreamId();
            size_t dataSize = SystemEvent::fillTimestampAndSamplesData (
                data,
                processor,
                streamId,
                currentSampleIndex,
                // NOTE: this timestamp is actually ignored in the current implementation?
                0,
                buffer.getNumSamples(),
                0);
            eventBuffer.addEvent (data, dataSize, 0);

            if (maybeTtlEvent != nullptr)
            {
                size_t ttlSize = maybeTtlEvent->getChannelInfo()->getDataSize() + maybeTtlEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
                HeapBlock<char> ttlBuffer (ttlSize);
                maybeTtlEvent->serialize (ttlBuffer, ttlSize);
                eventBuffer.addEvent (ttlBuffer, ttlSize, 0);
            }
        }

        // Copies the input buffer so that remains unmodified
        AudioBuffer<float> outputBuffer = buffer;
        audioProcessor->processBlock (outputBuffer, eventBuffer);
        currentSampleIndex += buffer.getNumSamples();
        return outputBuffer;
    }

    void updateSourceNodeSettings()
    {
        processorGraph->updateSettings (getSourceNode());
    }

public:
    int sourceNodeId; // should dynamically retrieve the processor if needed, since it apparently gets re-allocated

    std::unique_ptr<AudioComponent> audioComponent;
    std::unique_ptr<ProcessorGraph> processorGraph;
    std::unique_ptr<ControlPanel> controlPanel;

    int nextProcessorId = 1;
    int currentSampleIndex = 0;
    std::unique_ptr<CustomLookAndFeel> customLookAndFeel;
};

class DataThreadTester : public ProcessorTester
{
public:
    DataThreadTester (TestSourceNodeBuilder sourceNodeBuilder) : 
        ProcessorTester (sourceNodeBuilder)
    {}

    template <
        typename T,
        class... Args,
        typename std::enable_if<std::is_base_of<DataThread, T>::value>::type* = nullptr>
    T* createDataThread (Args&&... args)
    {
        T* ptr = new T ((SourceNode*) getSourceNode(), std::forward<Args> (args)...);

        return ptr;
    }
};

#endif