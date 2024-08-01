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
        mStream = std::make_unique<DataStream>(
            DataStream::Settings{
                "Data Stream",
                "Data Stream Description",
                "Data Stream Identifier",
            }
        );

        // Create a Continuous Channel
        mContinuousChannel = std::make_unique<ContinuousChannel>(
            ContinuousChannel::Settings{
                ContinuousChannel::Type::ELECTRODE,
                "Continuous Channel",
                "Continuous Channel Description",
                "Continuous Channel Identifier",
                0.0f,
                mStream.get()
            }
        );
    }

protected:
    std::unique_ptr<ContinuousChannel> mContinuousChannel;
    std::unique_ptr<DataStream> mStream;
};

/*
Continuous Channel should be able to set the bitVolts value for the channel
and also retrieve the bitVolts value for the channel.
*/
TEST_F(ContinuousChannelTests, SetGetBitVolts)
{
    // Set the bitVolts value for the channel
    mContinuousChannel->setBitVolts(0.5f);

    // Verify that the bitVolts value was set successfully
    EXPECT_EQ(mContinuousChannel->getBitVolts(), 0.5f);
}

/*
Continuous Channel should be able to set the unit string 
and also retrieve the unit string.
*/
TEST_F(ContinuousChannelTests, SetGetUnits)
{
    // Set the unit string
    mContinuousChannel->setUnits("mV");

    // Verify that the unit string was set successfully
    EXPECT_EQ(mContinuousChannel->getUnits(), "mV");
}

/*
Continuous Channel should be able to get the channel type.
*/
TEST_F(ContinuousChannelTests, GetChannelType)
{
    EXPECT_EQ(mContinuousChannel->getChannelType(), ContinuousChannel::Type::ELECTRODE);
}