#include "gtest/gtest.h"

#include <JuceHeader.h>

class MockAudioProcessorGraph : public AudioProcessorGraph
{
public:
    MockAudioProcessorGraph() = default;
    ~MockAudioProcessorGraph() override = default;
};

class MockAudioProcessor : public AudioProcessor
{
public:
    MockAudioProcessor() = default;
    ~MockAudioProcessor() override = default;

    // Inherited via AudioProcessor
    const String getName() const override
    {
        return String();
    }
    void prepareToPlay (double sampleRate, int maximumExpectedSamplesPerBlock) override
    {
    }
    void releaseResources() override
    {
    }
    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
    }
    double getTailLengthSeconds() const override
    {
        return 0.0;
    }
    bool acceptsMidi() const override
    {
        return true;
    }
    bool producesMidi() const override
    {
        return true;
    }
    AudioProcessorEditor* createEditor() override
    {
        return nullptr;
    }
    bool hasEditor() const override
    {
        return false;
    }
    int getNumPrograms() override
    {
        return 0;
    }
    int getCurrentProgram() override
    {
        return 0;
    }
    void setCurrentProgram (int index) override
    {
    }
    const String getProgramName (int index) override
    {
        return String();
    }
    void changeProgramName (int index, const String& newName) override
    {
    }
    void getStateInformation (juce::MemoryBlock& destData) override
    {
    }
    void setStateInformation (const void* data, int sizeInBytes) override
    {
    }
};

// Test fixture for AudioProcessorGraph
class AudioProcessorGraphTests : public testing::Test
{
protected:
    void SetUp() override
    {
        MessageManager::getInstance();

        // Create an AudioProcessorGraph
        graph = std::make_unique<MockAudioProcessorGraph>();
    }

    void TearDown() override
    {
        MessageManager::deleteInstance();
    }

protected:
    std::unique_ptr<AudioProcessorGraph> graph;
};

// Test case for adding and removing nodes
TEST_F (AudioProcessorGraphTests, AddAndRemoveNode)
{
    //Add a new node
    auto newNode = graph->addNode (std::make_unique<MockAudioProcessorGraph>(), AudioProcessorGraph::NodeID (0));
    ASSERT_NE (newNode, nullptr);

    // Check if the new node is added to the graph
    ASSERT_EQ (graph->getNumNodes(), 1);

    // Remove the new node
    graph->removeNode (newNode->nodeID);
    ASSERT_EQ (graph->getNumNodes(), 0);
}

// Test case for adding and removing connections
TEST_F (AudioProcessorGraphTests, AddAndRemoveConnection)
{
    // Arrange
    auto node1 = graph->addNode(std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID(0));
    auto node2 = graph->addNode(std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID(1));

    AudioProcessorGraph::NodeAndChannel source{ node1->nodeID, juce::AudioProcessorGraph::midiChannelIndex };
    AudioProcessorGraph::NodeAndChannel dest{ node2->nodeID, juce::AudioProcessorGraph::midiChannelIndex };

    auto newConnection = AudioProcessorGraph::Connection(source, dest);

    // Act
    bool connectionAdded = graph->addConnection(newConnection);

    // Assert
    EXPECT_TRUE(connectionAdded);
    EXPECT_EQ(graph->getConnections().size(), 1);

    // Act
    bool connectionRemoved = graph->removeConnection(newConnection);

    // Assert
    EXPECT_TRUE(connectionRemoved);
    EXPECT_EQ(graph->getConnections().size(), 0);
}

// Test case for checking if a connection exists
TEST_F (AudioProcessorGraphTests, IsConnected)
{
    // Arrange
    auto node1 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (0));
    auto node2 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (1));

    AudioProcessorGraph::NodeAndChannel source { node1->nodeID, juce::AudioProcessorGraph::midiChannelIndex };
    AudioProcessorGraph::NodeAndChannel dest { node2->nodeID, juce::AudioProcessorGraph::midiChannelIndex };

    auto connection = AudioProcessorGraph::Connection (source, dest);
    graph->addConnection (connection);

    // Check if the existing connection exists
    ASSERT_TRUE (graph->isConnected (connection));

    // Check if a non-existing connection exists
    AudioProcessorGraph::NodeAndChannel source1 { AudioProcessorGraph::NodeID (1), 0 };
    AudioProcessorGraph::NodeAndChannel dest1 { AudioProcessorGraph::NodeID (3), 0 };
    AudioProcessorGraph::Connection nonExistingConnection (source1, dest1);
    ASSERT_FALSE (graph->isConnected (nonExistingConnection));
}

// Test case for checking if a node is an input to another node
TEST_F (AudioProcessorGraphTests, IsAnInputTo)
{
    // Arrange
    auto node1 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (0));
    auto node2 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (1));

    AudioProcessorGraph::NodeAndChannel source { node1->nodeID, juce::AudioProcessorGraph::midiChannelIndex };
    AudioProcessorGraph::NodeAndChannel dest { node2->nodeID, juce::AudioProcessorGraph::midiChannelIndex };

    auto connection = AudioProcessorGraph::Connection (source, dest);
    graph->addConnection (connection);

    // Act
    bool node1ToNode2 = graph->isAnInputTo (*node1, *node2);

    // Assert
    ASSERT_TRUE (node1ToNode2);

    // Act
    bool node2ToNode1 = graph->isAnInputTo (*node2, *node1);

    // Assert
    ASSERT_FALSE (node2ToNode1);
}

// Test case for checking if a connection is legal
TEST_F (AudioProcessorGraphTests, IsConnectionLegal)
{
    // Arrange
    auto node1 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (0));
    auto node2 = graph->addNode (std::make_unique<MockAudioProcessor>(), AudioProcessorGraph::NodeID (1));

    AudioProcessorGraph::NodeAndChannel source { node1->nodeID, juce::AudioProcessorGraph::midiChannelIndex };
    AudioProcessorGraph::NodeAndChannel dest { node2->nodeID, juce::AudioProcessorGraph::midiChannelIndex };

    // Act
    auto connection = AudioProcessorGraph::Connection (source, dest);
    graph->addConnection (connection);
    bool legalConnection = graph->isConnectionLegal (connection);

    // Assert
    ASSERT_TRUE (legalConnection);

    // Arrange
    AudioProcessorGraph::NodeAndChannel source1 { AudioProcessorGraph::NodeID (1), 0 };
    AudioProcessorGraph::NodeAndChannel dest1 { AudioProcessorGraph::NodeID (3), 1 };
    AudioProcessorGraph::Connection nonExistingConnection (source1, dest1);

    // Act
    legalConnection = graph->isConnectionLegal (nonExistingConnection);

    // Assert
    ASSERT_FALSE (legalConnection);
}