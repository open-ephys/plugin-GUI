#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockProcessor : public GenericProcessor
{
public:
    MockProcessor() : 
        GenericProcessor("MockProcessor") 
    {}

    ~MockProcessor() noexcept override = default;

    void process (AudioBuffer<float>& continuousBuffer) override
    {}

    void VerifyDataStreams(const FakeSourceNodeParams& params) const
    {
        EXPECT_EQ(dataStreams.size(), params.streams);
        EXPECT_EQ(dataStreams.getFirst()->getChannelCount(), params.channels);
        EXPECT_EQ(dataStreams.getFirst()->getSampleRate(), params.sampleRate);
        EXPECT_EQ(dataStreams.getFirst()->getDescription(), "description");

        for (int i = 0; i < dataStreams.size(); i++)
            EXPECT_EQ(dataStreams.getFirst()->getName(), "FakeSourceNode" + String(i));
    }

    void VerifyContinuousChannels(const FakeSourceNodeParams& params) const
    {
        EXPECT_EQ(continuousChannels.size(), params.streams);

        for (int i = 0; i < continuousChannels.size(); i++)
        {
            EXPECT_EQ(continuousChannels[i]->getName(), "CH" + String(i));
            EXPECT_EQ(continuousChannels[i]->getBitVolts(), params.bitVolts);
            EXPECT_EQ(continuousChannels[i]->getChannelType(), ContinuousChannel::Type::ELECTRODE);
            EXPECT_EQ(continuousChannels[i]->getDescription(), String(i));
        }
    }

    void VerityEventChannels(const FakeSourceNodeParams& params) const
    {
        EXPECT_EQ(eventChannels.size(), params.streams);

        for (int i = 0; i < eventChannels.size(); i++)
        {
            EXPECT_EQ(eventChannels[i]->getType(), EventChannel::Type::TTL);
            EXPECT_EQ(eventChannels[i]->getName(), "TTL");
            EXPECT_EQ(eventChannels[i]->getDescription(), "TTL");
            EXPECT_EQ(eventChannels[i]->getEventMetadataCount(), params.metadataSizeBytes > 0 ? 1 : 0);
        }
    }
};

class GenericProcessorUnitTests : public testing::Test
{
protected:
    void SetUp() override
    {
        mProcessorTester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder(FakeSourceNodeParams{}));
        processor = mProcessorTester->createProcessor<MockProcessor>(Plugin::Processor::Type::SINK);
        //mDestProcessor = std::make_unique<MockProcessor> ("FakeDestinationNode");
        //mProcessor->setDestNode(mDestProcessor.get());
    }

    void TearDown() override
    {
        delete processor;
    }

protected:
    MockProcessor* processor;
    std::unique_ptr<MockProcessor> mDestProcessor;
    std::unique_ptr<ProcessorTester> mProcessorTester;
    FakeSourceNodeParams mParams;
};

/*
Generic Processors copy Data Streams from upstream processors. 
This data is saved internally by Generic Processors so that incoming data can be contextualized. 
This test verifies that this copying of Data Streams populates the necessary members within a Generic Processor.
*/
TEST_F (GenericProcessorUnitTests, CopyStreams)
{
    processor->update();
    
    processor->VerifyDataStreams(mParams);
    processor->VerifyContinuousChannels(mParams);
    processor->VerityEventChannels(mParams);
}

/*
Generic Processors can retrieve the source node that is connected to them.
*/
TEST_F(GenericProcessorUnitTests, GetSourceNode)
{
    EXPECT_EQ(processor->getSourceNode()->getName(), "FakeSourceNode");
}

/*
Generic Processors can retrieve the destination node that is connected to them.
*/
TEST_F(GenericProcessorUnitTests, GetDestinationNode)
{
    EXPECT_EQ(processor->getDestNode()->getName(), "FakeDestinationNode");
}

/*
Generic Processors can retrieve their name.
*/
TEST_F(GenericProcessorUnitTests, GetName)
{
    EXPECT_EQ(processor->getName(), "MockProcessor");
}

/*
Generic Processors can be configured to generate timestamps.
*/
TEST_F (GenericProcessorUnitTests, GeneratesTimestamps)
{
    EXPECT_TRUE(processor->generatesTimestamps());
}

/*
Generic Processors ccan get and set their node ID.
*/
TEST_F (GenericProcessorUnitTests, GetSetNodeId)
{
    processor->setNodeId(1);
    EXPECT_EQ(processor->getNodeId(), 1);
}

/*
Generic Processors can add boolean parameters.
*/
TEST_F(GenericProcessorUnitTests, AddBooleanParameter)
{
    String name = "param";
    String displayName = "param";
    String description = "param";

    processor->addBooleanParameter (Parameter::PROCESSOR_SCOPE, name, displayName, description, true);
    EXPECT_EQ (processor->getStreamParameter ("param")->getName(), name);
    EXPECT_EQ (processor->getStreamParameter ("param")->getDisplayName(), displayName);
    EXPECT_EQ (processor->getStreamParameter ("param")->getDescription(), description);
}