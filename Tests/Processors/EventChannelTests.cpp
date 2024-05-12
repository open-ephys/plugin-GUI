#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class EventChannelTests : public testing::Test
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

        // Create an Event Channel
        mEventChannel = std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL,
                "Event Channel",
                "Event Channel Description",
                "Event Channel Identifier",
                mStream.get(),
                8,
                EventChannel::BinaryDataType::UINT8_ARRAY,
                0
            }
        );
    }

    void TearDown() override
    {

    }

protected:
    std::unique_ptr<EventChannel> mEventChannel;
    std::unique_ptr<DataStream> mStream;
};

/*
Event Channel should return the correct type when initialized
with a specific type
*/
TEST_F(EventChannelTests, GetType)
{
    EXPECT_EQ(mEventChannel->getType(), EventChannel::Type::TTL);
}

/*
Event Channel should return the correct binary data type when initialized
with a specific binary data type
*/
TEST_F(EventChannelTests, GetBinaryDataType)
{
    EXPECT_EQ(mEventChannel->getBinaryDataType(), EventChannel::BinaryDataType::UINT8_ARRAY);
}

/*
Event Channel should return the correct size of the event payload
when initialized with a specific size
*/
TEST_F(EventChannelTests, GetLength)
{
    EXPECT_EQ(mEventChannel->getLength(), 10);
}

/*
Event Channel should return the correct size of the event payload in bytes.
*/
TEST_F(EventChannelTests, GetDataSize)
{
    EXPECT_EQ(mEventChannel->getDataSize(), 10);
}

/*
Event Channel should return the correct TTL size.
*/
TEST_F(EventChannelTests, GetTTLBits)
{
    EXPECT_EQ(mEventChannel->getMaxTTLBits(), 8);
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
    EXPECT_EQ(EventChannel::getEquivalentMetadataType(*mEventChannel), MetadataDescriptor::UINT8);
}

/*
* Event Channel should get the equivalent metadata type of the event channel.
*/
TEST_F(EventChannelTests, GetEquivalentMetadataTypeConst)
{
    EXPECT_EQ(mEventChannel->getEquivalentMetadataType(), MetadataDescriptor::UINT8);
}

/*
* Event Channel should correctly set and get the label for a particular line.
*/
TEST_F(EventChannelTests, SetLineLabel)
{
    mEventChannel->setLineLabel(0, "Line 0");
    EXPECT_EQ(mEventChannel->getLineLabel(0), "Line 0");
}

/*
* Event Channel should correctly set and get the state for a particular line.
*/
TEST_F(EventChannelTests, SetLineState)
{
    mEventChannel->setLineState(0, true);
    EXPECT_EQ(mEventChannel->getTTLWord(), 1);
}

/*
* Event Channel should get the TTL word.
*/
TEST_F(EventChannelTests, GetTTLWord)
{
    EXPECT_EQ(mEventChannel->getTTLWord(), 0);
}