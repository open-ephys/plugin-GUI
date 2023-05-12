#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "gtest/gtest.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <Processors/ProcessorGraph/ProcessorGraph.h>
#include <Audio/AudioComponent.h>
#include <UI/ControlPanel.h>

class ProcessorTester {
public:
    ProcessorTester(FakeSourceNodeParams fake_source_node_params) {
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

        auto sn_temp = new FakeSourceNode(fake_source_node_params);
        source_node_id = next_processor_id_++;
        sn_temp->setNodeId(source_node_id);
        juce::AudioProcessorGraph::Node* n = processor_graph->addNode(
            std::move(std::unique_ptr<AudioProcessor>(sn_temp)),
            juce::AudioProcessorGraph::NodeID(source_node_id));

        // Create the source node, and place it in the graph
        auto sn = (FakeSourceNode *) n->getProcessor();
        sn->initialize(false);
        sn->setDestNode(nullptr);

        control_panel->updateRecordEngineList();

        // Refresh everything
        processor_graph->updateSettings(sn, false);
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
    T *Create(Args &&...args) {
        T *ptr = new T(std::forward<Args>(args)...);
        ptr->setProcessorType(Plugin::Processor::RECORD_NODE);
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

    void startAcquisition(bool startRecording) {
        if (startRecording) {
            // Do it this way to ensure the GUI elements (which apparently control logic) are set properly
            control_panel->setRecordingState(true, false);
        } else {
            control_panel->startAcquisition(false);
        }
    }

    void stopAcquisition() {
        control_panel->stopAcquisition();
    }

    static void setRecordingParentDirectory(const std::string& parent_directory) {
        CoreServices::setRecordingParentDirectory(String(parent_directory));
    }

private:
    int source_node_id; // should dynamically retrieve the processor if needed, since it apparently gets re-allocated

    std::unique_ptr<AudioComponent> audio_component;
    std::unique_ptr<ProcessorGraph> processor_graph;
    std::unique_ptr<ControlPanel> control_panel;

    int next_processor_id_ = 1;
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
