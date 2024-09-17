#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class MockProcessor : public GenericProcessor
{
public:
    MockProcessor() : 
        GenericProcessor("MockProcessor", true) 
    {
        setProcessorType(Processor::Type::SINK);
    }

    void process (AudioBuffer<float>& continuousBuffer) override
    {}

    void verifyDataStreams(const FakeSourceNodeParams& params) const
    {
        EXPECT_EQ(dataStreams.size(), params.streams);
        EXPECT_EQ(dataStreams.getFirst()->getChannelCount(), params.channels);
        EXPECT_EQ(dataStreams.getFirst()->getSampleRate(), params.sampleRate);
        EXPECT_EQ(dataStreams.getFirst()->getDescription(), "description");

        for (int i = 0; i < dataStreams.size(); i++)
            EXPECT_EQ(dataStreams.getFirst()->getName(), "FakeSourceNode" + String(i));
    }

    void verifyContinuousChannels(const FakeSourceNodeParams& params) const
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

    void verifyEventChannels(const FakeSourceNodeParams& params) const
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

class GenericProcessorTests : public testing::Test
{
protected:
    void SetUp() override
    {
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder(FakeSourceNodeParams{}));
        processor = tester->createProcessor<MockProcessor>(Processor::Type::SINK);
    }

protected:
    MockProcessor* processor;
    std::unique_ptr<ProcessorTester> tester;
    FakeSourceNodeParams params;
};

/*
Generic Processors copy Data Streams from upstream processors. 
This data is saved internally by Generic Processors so that incoming data can be contextualized. 
This test verifies that this copying of Data Streams populates the necessary members within a Generic Processor.
*/
TEST_F (GenericProcessorTests, UnitTest_CopyStreams)
{
    processor->update();
    
    processor->verifyDataStreams(params);
    processor->verifyContinuousChannels(params);
    processor->verifyEventChannels(params);
}

/*
Generic Processors can retrieve the source node that is connected to them.
*/
TEST_F (GenericProcessorTests, UnitTest_GetSourceNode)
{
    EXPECT_TRUE(processor->getSourceNode());
}

/*
Generic Processors can retrieve the destination node that is connected to them.
*/
TEST_F (GenericProcessorTests, UnitTest_GetDestinationNode)
{
    EXPECT_FALSE(processor->getDestNode());
}

/*
Generic Processors can retrieve their name.
*/
TEST_F (GenericProcessorTests, UnitTest_GetName)
{
    EXPECT_EQ(processor->getName(), "MockProcessor");
}

/*
Generic Processors can be configured to generate timestamps.
*/
TEST_F (GenericProcessorTests, UnitTest_GeneratesTimestamps)
{
    EXPECT_FALSE(processor->generatesTimestamps());
}

/*
Generic Processors ccan get and set their node ID.
*/
TEST_F (GenericProcessorTests, UnitTest_GetSetNodeId)
{
    processor->setNodeId(1);
    EXPECT_EQ(processor->getNodeId(), 1);
}

/*
Generic Processors can add boolean parameters.
*/
TEST_F (GenericProcessorTests, UnitTest_AddBooleanParameter)
{
    String name = "param";
    String displayName = "param";
    String description = "param";

    processor->addBooleanParameter (Parameter::PROCESSOR_SCOPE, name, displayName, description, true);
    EXPECT_EQ (processor->getParameter ("param")->getName(), name);
    EXPECT_EQ (processor->getParameter ("param")->getDisplayName(), displayName);
    EXPECT_EQ (processor->getParameter ("param")->getDescription(), description);
}