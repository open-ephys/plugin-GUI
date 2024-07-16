#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class FakeGenericProcessor : public GenericProcessor
{
public:
    FakeGenericProcessor() : 
        GenericProcessor("FakeGenericProcessor") 
    {}

    ~FakeGenericProcessor() noexcept override = default;

    void process(AudioBuffer<float>& continuousBuffer) override
    {
        for (int i = 0; i < continuousBuffer.getNumChannels(); i++)
        {
            for (int j = 0; j < continuousBuffer.getNumSamples(); j++)
            {
                float currentValue = continuousBuffer.getSample(i, j);
                float newValue = currentValue + 1;

                continuousBuffer.setSample(i, j, newValue);
            }
        }
    }

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
            EXPECT_EQ(eventChannels[i]->getEventMetadataCount(), params.metadata_size_bytes > 0 ? 1 : 0);
        }
    }
};

class GenericProcessorTests : public testing::Test
{
protected:
    void SetUp() override
    {
        mProcessorTester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder(FakeSourceNodeParams{}));
        mProcessor = mProcessorTester->CreateProcessor<FakeGenericProcessor>(Plugin::Processor::Type::SINK);
    }

    void TearDown() override
    {
        
    }

protected:
    FakeGenericProcessor* mProcessor;
    std::unique_ptr<ProcessorTester> mProcessorTester;
    FakeSourceNodeParams mParams;
};

/*
Generic processors implement custom functionality to manipulate data stored in an incoming Audio Buffer 
and Midi Buffer (each iteration of these buffers is a block). 
This test verifies that the Generic Processor interface can successfully modify the Audio Buffer through this interface.
*/
TEST_F(GenericProcessorTests, DataIntegrity)
{
    AudioBuffer<float> inputBuffer(2, 10);

    for (int i = 0; i < inputBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < inputBuffer.getNumSamples(); j++)
            inputBuffer.setSample(i, j, i + j);
    }

    mProcessor->process(inputBuffer);

    for (int i = 0; i < inputBuffer.getNumChannels(); i++)
    {
        for (int j = 0; j < inputBuffer.getNumSamples(); j++)
        {
            float value = inputBuffer.getSample(i, j);
            float expectedValue = i + j + 1;

            EXPECT_EQ(value, expectedValue);
        }
    }
}

/*
Generic Processors copy Data Streams from upstream processors. 
This data is saved internally by Generic Processors so that incoming data can be contextualized. 
This test verifies that this copying of Data Streams populates the necessary members within a Generic Processor.
*/
TEST_F(GenericProcessorTests, CopyStreams)
{
    mProcessor->update();
    
    mProcessor->VerifyDataStreams(mParams);
    mProcessor->VerifyContinuousChannels(mParams);
    mProcessor->VerityEventChannels(mParams);
}