#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

/*
 	struct Settings
	{
		Type type; 

		String name;
		String description;
		String identifier;
		
		float bitVolts;

		DataStream* stream;
	};
*/
class ContinuousChannelTests : public testing::Test
{
protected:
    void SetUp() override
    {
        dataStream = std::make_unique<DataStream>(
            DataStream::Settings{
                "Data Stream",
                "Data Stream Description",
                "Data Stream Identifier",
            }
        );

        // Create a Continuous Channel
        continuousChannel = std::make_unique<ContinuousChannel>(
            ContinuousChannel::Settings{
                ContinuousChannel::Type::ELECTRODE,
                "Continuous Channel",
                "Continuous Channel Description",
                "Continuous Channel Identifier",
                0.0f,
                dataStream.get()
            }
        );
    }

protected:
    std::unique_ptr<ContinuousChannel> continuousChannel;
    std::unique_ptr<DataStream> dataStream;
};

/*
Continuous Channel should be able to set the bitVolts value for the channel
and also retrieve the bitVolts value for the channel.
*/
TEST_F(ContinuousChannelTests, SetGetBitVolts)
{
    // Set the bitVolts value for the channel
    continuousChannel->setBitVolts(0.5f);

    // Verify that the bitVolts value was set successfully
    EXPECT_EQ(continuousChannel->getBitVolts(), 0.5f);
}

/*
Continuous Channel should be able to set the unit string 
and also retrieve the unit string.
*/
TEST_F(ContinuousChannelTests, SetGetUnits)
{
    // Set the unit string
    continuousChannel->setUnits("mV");

    // Verify that the unit string was set successfully
    EXPECT_EQ(continuousChannel->getUnits(), "mV");
}

/*
Continuous Channel should be able to get the channel type.
*/
TEST_F(ContinuousChannelTests, GetChannelType)
{
    EXPECT_EQ(continuousChannel->getChannelType(), ContinuousChannel::Type::ELECTRODE);
}