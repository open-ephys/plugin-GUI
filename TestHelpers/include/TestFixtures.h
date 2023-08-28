#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "gtest/gtest.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <Processors/ProcessorGraph/ProcessorGraph.h>
#include <Processors/SourceNode/SourceNode.h>
#include <Audio/AudioComponent.h>
#include <UI/ControlPanel.h>

enum class TestSourceNodeType {Fake, Base};


 class TestSourceNodeBuilder{
 public:
     TestSourceNodeBuilder(FakeSourceNodeParams params) : _fake_source_node_params(params), _source_node_type(TestSourceNodeType::Fake) {}
     TestSourceNodeBuilder(DataThreadCreator creator) : _data_thread_creator(creator), _source_node_type(TestSourceNodeType::Base) {}

     SourceNode* buildSourceNode(){
         switch(_source_node_type) {
             case TestSourceNodeType::Fake : {
                 return (SourceNode*)(new FakeSourceNode(_fake_source_node_params));
             }
             case TestSourceNodeType::Base : {
                 return new SourceNode("BaseSourceNode", _data_thread_creator);
             }
         }
     }

     TestSourceNodeType getTestSourceNodeType() const {
         return _source_node_type;
     }


 private:
     FakeSourceNodeParams _fake_source_node_params;
     DataThreadCreator _data_thread_creator;
     TestSourceNodeType _source_node_type;
 };

class ProcessorTester {
 public:
     ProcessorTester(TestSourceNodeBuilder source_node_builder) {
         // Singletons...
         MessageManager::deleteInstance();

         // initializes the singleton instance
         MessageManager::getInstance();

         // Reset all state so no interactions between tests.
         AccessClass::clearAccessClassStateForTesting();



         // All of these sets the global state in AccessClass in their constructors
         audio_component = std::make_unique<AudioComponent>();
         processor_graph = std::make_unique<ProcessorGraph>(true);
         control_panel = std::make_unique<ControlPanel>(processor_graph.get(), audio_component.get(), true);

         SourceNode* sn_temp = source_node_builder.buildSourceNode();
         source_node_id = next_processor_id_++;
         sn_temp->setNodeId(source_node_id);
         juce::AudioProcessorGraph::Node* n = processor_graph->addNode(
             std::move(std::unique_ptr<AudioProcessor>(sn_temp)),
             juce::AudioProcessorGraph::NodeID(source_node_id));

         // Create the source node, and place it in the graph
         auto sn = (GenericProcessor*) n->getProcessor();
         sn->setHeadlessMode(true);
         sn->initialize(false);
         sn->setDestNode(nullptr);

         control_panel->updateRecordEngineList();

         // Refresh everything
         processor_graph->updateSettings(sn, false);

     }

    ~ProcessorTester() {
        control_panel = nullptr;
        processor_graph = nullptr;
        audio_component = nullptr;

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
    template<
        typename T,
        class... Args,
        typename std::enable_if<std::is_base_of<GenericProcessor, T>::value>::type * = nullptr>
    T *Create(Plugin::Processor::Type processorType, Args &&...args) {
        T *ptr = new T(std::forward<Args>(args)...);
        ptr->setProcessorType(processorType);
        ptr->setHeadlessMode(true);

        int node_id = next_processor_id_++;
        ptr->setNodeId(node_id);
        processor_graph->addNode(
            std::move(std::unique_ptr<AudioProcessor>(ptr)),
            juce::AudioProcessorGraph::NodeID(node_id));

        ptr = (T *) processor_graph->getProcessorWithNodeId(node_id);
        ptr->initialize(false);
        ptr->setDestNode(nullptr);

        // Place the newly created node into the graph
        auto source_node = processor_graph->getProcessorWithNodeId(source_node_id);
        ptr->setSourceNode(source_node);
        source_node->setDestNode(ptr);

        // Refresh everything
        processor_graph->updateSettings(ptr, false);
        return (T *) processor_graph->getProcessorWithNodeId(node_id);
    }

    void startAcquisition(bool startRecording, bool forceRecording = false) {
        if (startRecording) {
            // Do it this way to ensure the GUI elements (which apparently control logic) are set properly
            control_panel->setRecordingState(true, forceRecording);
        } else {
            control_panel->startAcquisition(false);
        }
    }

    void stopAcquisition() {
        control_panel->stopAcquisition();
    }

    const DataStream *GetProcessorDataStream(uint16 node_id, uint16 stream_id) {
        auto streams = processor_graph->getProcessorWithNodeId(node_id)->getDataStreams();
        for (auto stream : streams) {
            if (stream->getStreamId() == stream_id) {
                return stream;
            }
        }
        return nullptr;
    }

    const DataStream *GetSourceNodeDataStream(uint16 stream_id) {
        return GetProcessorDataStream(source_node_id, stream_id);
    }

    static void setRecordingParentDirectory(const std::string& parent_directory) {
        CoreServices::setRecordingParentDirectory(String(parent_directory));
    }

    GenericProcessor* getSourceNode() {
        return (GenericProcessor*)processor_graph->getProcessorWithNodeId(source_node_id);
    }
    AudioBuffer<float> ProcessBlock(
        GenericProcessor *processor,
        const AudioBuffer<float> &buffer,
        TTLEvent* maybe_ttl_event = nullptr) {
        auto audio_processor = (AudioProcessor *)processor;
        auto data_streams = processor->getDataStreams();

        MidiBuffer eventBuffer;
        for (const auto* datastream : data_streams) {
            HeapBlock<char> data;
            auto streamId = datastream->getStreamId();
            size_t dataSize = SystemEvent::fillTimestampAndSamplesData(
                data,
                processor,
                streamId,
                current_sample_index,
                // NOTE: this timestamp is actually ignored in the current implementation?
                0,
                buffer.getNumSamples(),
                0);
            eventBuffer.addEvent(data, dataSize, 0);

            if (maybe_ttl_event != nullptr) {
                size_t ttl_size = maybe_ttl_event->getChannelInfo()->getDataSize() +
                    maybe_ttl_event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
                HeapBlock<char> ttl_buffer(ttl_size);
                maybe_ttl_event->serialize(ttl_buffer, ttl_size);
                eventBuffer.addEvent(ttl_buffer, ttl_size, 0);
            }
        }

        // Copies the input buffer so that remains unmodified
        AudioBuffer<float> output_buffer = buffer;
        audio_processor->processBlock(output_buffer, eventBuffer);
        current_sample_index += buffer.getNumSamples();
        return output_buffer;
    }
    
    void updateSourceNodeSettings() {
        processor_graph -> updateSettings(getSourceNode());
    }

private:
    int source_node_id; // should dynamically retrieve the processor if needed, since it apparently gets re-allocated

    std::unique_ptr<AudioComponent> audio_component;
    std::unique_ptr<ProcessorGraph> processor_graph;
    std::unique_ptr<ControlPanel> control_panel;

    int next_processor_id_ = 1;
    int current_sample_index = 0;
};

class ProcessorTest : public ::testing::Test {
protected:
    ProcessorTest(int channels, int sampleRate) : channels_(channels), sampleRate_(sampleRate) {}

    ~ProcessorTest() override {}

    void SetUp() override {
        tester = std::make_unique<ProcessorTester>(FakeSourceNodeParams({channels_, (float) sampleRate_}));
    }

    void TearDown() override {

    }

private:
    std::unique_ptr<ProcessorTester> tester;
    const int channels_;
    const int sampleRate_;

};


#endif
