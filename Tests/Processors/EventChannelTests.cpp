#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class EventChannelTests : public testing::Test
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

        // Create an Event Channel
        eventChannel = std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL,
                "Event Channel",
                "Event Channel Description",
                "event.channel.identifier",
                dataStream.get(),
                8,
                EventChannel::BinaryDataType::UINT8_ARRAY,
                0
            }
        );
    }

protected:
    std::unique_ptr<EventChannel> eventChannel;
    std::unique_ptr<DataStream> dataStream;
};

/*
Event Channel should return the correct type when initialized
with a specific type
*/
TEST_F(EventChannelTests, GetType)
{
    EXPECT_EQ(eventChannel->getType(), EventChannel::Type::TTL);
}

/*
Event Channel should return the correct name when initialized
with a specific name
*/
TEST_F(EventChannelTests, GetName)
{
    EXPECT_EQ(eventChannel->getName(), "Event Channel");
}

/*
Event Channel should return the correct description when initialized
with a specific description
*/
TEST_F(EventChannelTests, GetDescription)
{
    EXPECT_EQ(eventChannel->getDescription(), "Event Channel Description");
}

/*
Event Channel should return the correct identifier when initialized
with a specific identifier
*/
TEST_F(EventChannelTests, GetIdentifier)
{
    EXPECT_EQ(eventChannel->getIdentifier(), "event.channel.identifier");
}

/*
Event Channel should return the correct binary data type when initialized
with a specific binary data type
*/
TEST_F(EventChannelTests, GetBinaryDataType)
{
    EXPECT_EQ(eventChannel->getBinaryDataType(), EventChannel::BinaryDataType::UINT8_ARRAY);
}

/*
Event Channel should return the correct size of the event payload
when initialized with a specific size
*/
TEST_F(EventChannelTests, GetLength)
{
    EXPECT_EQ(eventChannel->getLength(), 10);
}

/*
Event Channel should return the correct size of the event payload in bytes.
*/
TEST_F(EventChannelTests, GetDataSize)
{
    EXPECT_EQ(eventChannel->getDataSize(), 10);
}

/*
Event Channel should return the correct TTL size.
*/
TEST_F(EventChannelTests, GetTTLBits)
{
    EXPECT_EQ(eventChannel->getMaxTTLBits(), 8);
}

/*
Event Channel should get the correct size in bytes of an element depending on the type.
*/
TEST_F(EventChannelTests, GetBinaryDataTypeSize)
{
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::INT8_ARRAY), sizeof(int8));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::UINT8_ARRAY), sizeof(uint8));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::INT16_ARRAY), sizeof(int16));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::UINT16_ARRAY), sizeof(uint16));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::INT32_ARRAY), sizeof(int32));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::UINT32_ARRAY), sizeof(uint32));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::INT64_ARRAY), sizeof(int64));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::UINT64_ARRAY), sizeof(uint64));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::FLOAT_ARRAY), sizeof(float));
    EXPECT_EQ(EventChannel::getBinaryDataTypeSize(EventChannel::BinaryDataType::DOUBLE_ARRAY), sizeof(double));
}

/*
* Event Channel should get the equivalent metadata type of the event channel.
*/
TEST_F(EventChannelTests, GetEquivalentMetadataType)
{
    EXPECT_EQ(EventChannel::getEquivalentMetadataType(*eventChannel), MetadataDescriptor::UINT8);
}

/*
* Event Channel should get the equivalent metadata type of the event channel.
*/
TEST_F(EventChannelTests, GetEquivalentMetadataTypeConst)
{
    EXPECT_EQ(eventChannel->getEquivalentMetadataType(), MetadataDescriptor::UINT8);
}

/*
* Event Channel should correctly set and get the label for a particular line.
*/
TEST_F(EventChannelTests, SetLineLabel)
{
    eventChannel->setLineLabel(0, "Line 0");
    EXPECT_EQ(eventChannel->getLineLabel(0), "Line 0");
}

/*
* Event Channel should correctly set and get the state for a particular line.
*/
TEST_F(EventChannelTests, SetLineState)
{
    eventChannel->setLineState(0, true);
    EXPECT_EQ(eventChannel->getTTLWord(), 1);
}

/*
* Event Channel should get the TTL word.
*/
TEST_F(EventChannelTests, GetTTLWord)
{
    EXPECT_EQ(eventChannel->getTTLWord(), 0);
}