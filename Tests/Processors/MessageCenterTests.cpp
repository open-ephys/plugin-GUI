#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <Processors/MessageCenter/MessageCenter.h>
#include <memory>

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

class MessageCenterTests : public testing::Test
{
protected:
    void SetUp() override
    {
        mMessageCenter = std::make_unique<MessageCenter>();
    }

protected:
    std::unique_ptr<MessageCenter> mMessageCenter;
};

TEST_F(MessageCenterTests, Constructor)
{
    EXPECT_EQ(mMessageCenter->getName(), "Message Center");
}

TEST_F(MessageCenterTests, AddSpecialProcessorChannels)
{
    mMessageCenter->addSpecialProcessorChannels();
    EXPECT_EQ(mMessageCenter->getDataStreams().size(), 1);
    EXPECT_EQ(mMessageCenter->getEventChannels().size(), 1);
}

TEST_F(MessageCenterTests, GetMessageChannel)
{
    EXPECT_EQ(mMessageCenter->getMessageChannel(), nullptr);
}

TEST_F(MessageCenterTests, GetMessageStream)
{
    EXPECT_EQ(mMessageCenter->getMessageStream(), nullptr);
}

TEST_F(MessageCenterTests, AddOutgoingMessage)
{
    GTEST_SKIP() << "Need headless support for MessageCenter";

    mMessageCenter->addOutgoingMessage("Test Message", 100);
    EXPECT_EQ(mMessageCenter->getSavedMessages().size(), 1);

    const auto& savedMsg = mMessageCenter->getSavedMessages()[0];
    EXPECT_EQ(savedMsg, "Test Message");
}

TEST_F(MessageCenterTests, AddSavedMessage)
{
    GTEST_SKIP() << "Need headless support for MessageCenter";

    mMessageCenter->addSavedMessage("Test Message");
    EXPECT_EQ(mMessageCenter->getSavedMessages().size(), 1);

    const auto& savedMsg = mMessageCenter->getSavedMessages()[0];
    EXPECT_EQ(savedMsg, "Test Message");
}

TEST_F(MessageCenterTests, ClearSavedMessages)
{
    GTEST_SKIP() << "Need headless support for MessageCenter";

    mMessageCenter->addSavedMessage("Test Message");
    EXPECT_EQ(mMessageCenter->getSavedMessages().size(), 1);

    mMessageCenter->clearSavedMessages();
    EXPECT_EQ(mMessageCenter->getSavedMessages().size(), 0);
}