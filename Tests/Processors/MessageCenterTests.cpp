#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <Processors/MessageCenter/MessageCenter.h>
#include <memory>

class MessageCenterTests : public testing::Test
{
protected:
    void SetUp() override
    {
        messageCenter = std::make_unique<MessageCenter>();
    }

protected:
    std::unique_ptr<MessageCenter> messageCenter;
};

TEST_F(MessageCenterTests, Constructor)
{
    EXPECT_EQ(messageCenter->getName(), "Message Center");
}

TEST_F(MessageCenterTests, AddSpecialProcessorChannels)
{
    messageCenter->addSpecialProcessorChannels();
    EXPECT_EQ(messageCenter->getDataStreams().size(), 1);
    EXPECT_EQ(messageCenter->getEventChannels().size(), 1);
}

TEST_F(MessageCenterTests, GetMessageChannel)
{
    EXPECT_EQ(messageCenter->getMessageChannel(), nullptr);
}

TEST_F(MessageCenterTests, GetMessageStream)
{
    EXPECT_EQ(messageCenter->getMessageStream(), nullptr);
}

TEST_F(MessageCenterTests, AddOutgoingMessage)
{
    messageCenter->addOutgoingMessage("Test Message", 100);
    EXPECT_EQ(messageCenter->getSavedMessages().size(), 1);

    const auto& savedMsg = messageCenter->getSavedMessages()[0];
    EXPECT_EQ(savedMsg, "Test Message");
}

TEST_F(MessageCenterTests, AddSavedMessage)
{
    messageCenter->addSavedMessage("Test Message");
    EXPECT_EQ(messageCenter->getSavedMessages().size(), 1);

    const auto& savedMsg = messageCenter->getSavedMessages()[0];
    EXPECT_EQ(savedMsg, "Test Message");
}

TEST_F(MessageCenterTests, ClearSavedMessages)
{
    messageCenter->addSavedMessage("Test Message");
    EXPECT_EQ(messageCenter->getSavedMessages().size(), 1);

    messageCenter->clearSavedMessages();
    EXPECT_EQ(messageCenter->getSavedMessages().size(), 0);
}