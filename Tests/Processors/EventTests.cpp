#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <unordered_map>

class MockProcessor : public GenericProcessor
{
public:
    MockProcessor() :
        GenericProcessor("MockProcessor")
    {}

    // Inherited via GenericProcessor
    void process(AudioBuffer<float>& continuousBuffer) override
    {}
};

class FakeEvent : public Event
{
public:
    FakeEvent(const EventChannel* channelInfo, int64 sampleNumber, double timestamp = -1.0) :
        Event(channelInfo, sampleNumber, timestamp)
    {}

    void serialize(void* destinationBuffer, size_t bufferSize) const override
    {}
};

class EventTests : public testing::Test
{
protected:
    void SetUp() override
    {   
        // Create a DataStream
        DataStream::Settings dataStreamSettings{
            "DataStream",
            "description",
            "identifier",
            20000.0f
        };

        mDataStream = std::make_unique<DataStream>(dataStreamSettings);

        // Create a ContinuousChannel and add it to the DataStream
        ContinuousChannel::Settings continuousChannelSettings{
            ContinuousChannel::Type::ELECTRODE,
            "CH0",
            "0",
            "identifier",
            1.0f,
            mDataStream.get()
        };

        mContinuousChannel = std::make_unique<ContinuousChannel>(continuousChannelSettings);
        mDataStream->addChannel(mContinuousChannel.get());

        // Set the source node id for the data stream
        mNodeId = 0;
        mDataStream->setNodeId(mNodeId);

        // Create two EventChannels and add them to the DataStream
        mEventChannel.emplace("TTL", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL, "TTL", "Event", "identifier.ttl", mDataStream.get()
            }));
        mEventChannel.emplace("Text", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TEXT, "Text", "Event", "identifier.text", mDataStream.get()
            }));
        mEventChannel.emplace("Event", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL, "Event", "Event", "identifier.event", mDataStream.get()
            }));

        float data[] = { 0, 1, 2 ,3 };

        mEventChannel.emplace("Binary", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::CUSTOM, 
                "Binary", 
                "Event", 
                "identifier.binary",
                mDataStream.get(), 
                8,
                EventChannel::BinaryDataType::FLOAT_ARRAY, 
                sizeof(data) / sizeof(data[0])
            }));

        // Add the processor to the EventChannels
        processor = std::make_unique<MockProcessor>();
        processor->setNodeId(mNodeId);

        for (const auto& channel : mEventChannel)
            channel.second->addProcessor(processor.get());

        // Create Events
        mEvent = new FakeEvent(mEventChannel["Event"].get(), 0);
        mTTLEvent = TTLEvent::createTTLEvent(mEventChannel["TTL"].get(), 0, 0, true);
        mTextEvent = TextEvent::createTextEvent(mEventChannel["Text"].get(), 0, "Text");

        mBinaryEvent = BinaryEvent::createBinaryEvent<float>(mEventChannel["Binary"].get(), 0, data, sizeof(data));
    }

protected:
    std::unique_ptr<MockProcessor> processor;
    std::unique_ptr<DataStream> mDataStream;
    std::unique_ptr<ContinuousChannel> mContinuousChannel;
    std::unordered_map<String, std::unique_ptr<EventChannel>> mEventChannel;
    EventPtr mEvent;
    TTLEventPtr mTTLEvent;
    TextEventPtr mTextEvent;
    BinaryEventPtr mBinaryEvent;

    int mNodeId;
};

/*
Event should return the correct event type.
*/
TEST_F(EventTests, GetEventType)
{
    EXPECT_EQ(mEvent->getEventType(), EventChannel::Type::TTL);
    EXPECT_EQ(mTTLEvent->getEventType(), EventChannel::Type::TTL);
    EXPECT_EQ(mTextEvent->getEventType(), EventChannel::Type::TEXT);
    EXPECT_EQ(mBinaryEvent->getEventType(), EventChannel::Type::CUSTOM);
}

/*
Event should return the correct EventChannel info object.
*/
TEST_F(EventTests, GetEventChannelInfo)
{
    EXPECT_EQ(mEvent->getChannelInfo(), mEventChannel["Event"].get());
    EXPECT_EQ(mTTLEvent->getChannelInfo(), mEventChannel["TTL"].get());
    EXPECT_EQ(mTextEvent->getChannelInfo(), mEventChannel["Text"].get());
    EXPECT_EQ(mBinaryEvent->getChannelInfo(), mEventChannel["Binary"].get());
}

/*
Events should should correctly be serialized and deserialized.
*/
TEST_F(EventTests, SerializeDeserializeEvent)
{
    // TTLEvent
    {
        // Serialize the event
        size_t size = mTTLEvent->getChannelInfo()->getDataSize() +
            mTTLEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
        HeapBlock<uint8> buffer(size);

        mTTLEvent->serialize(buffer, size);

        {
            // Deserialize the event
            auto deserializedEvent = TTLEvent::deserialize(buffer.getData(), mTTLEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(mTTLEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }

        {
            EventPacket packet(buffer, size);

            // Deserialize the event
            auto deserializedEvent = TTLEvent::deserialize(packet, mTTLEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(mTTLEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }
    }

    // TextEvent
    {
        // Serialize the event
        size_t size = mTextEvent->getChannelInfo()->getDataSize() +
            mTextEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
        HeapBlock<uint8> buffer(size);

        mTextEvent->serialize(buffer, size);

        {
            // Deserialize the event
            auto deserializedEvent = TextEvent::deserialize(buffer.getData(), mTextEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(mTextEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }

        {
            EventPacket packet(buffer, size);

            // Deserialize the event
            auto deserializedEvent = TextEvent::deserialize(packet, mTextEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(mTextEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }
    }
}

/*
TTLEvent should return the correct state.
*/
TEST_F(EventTests, GetState)
{
    EXPECT_EQ(mTTLEvent->getState(), true);
}

/*
TTLEvent should return the correct line.
*/
TEST_F(EventTests, GetLine)
{
    EXPECT_EQ(mTTLEvent->getLine(), 0);
}

/*
TTLEvent should return the correct word.
*/
TEST_F(EventTests, GetWord)
{
    EXPECT_EQ(mTTLEvent->getWord(), true);
}

/*
TextEvent should return the correct text.
*/
TEST_F(EventTests, GetText)
{
    EXPECT_EQ(mTextEvent->getText(), "Text");
}

/*
BinaryEvent should return the correct type.
*/
TEST_F(EventTests, GetBinaryType)
{
    EXPECT_EQ(mBinaryEvent->getBinaryType(), EventChannel::BinaryDataType::FLOAT_ARRAY);
}

/*
BinaryEvent should return the correct data.
*/
TEST_F(EventTests, GetBinaryDataPointer)
{
    const float* data = static_cast<const float*>(mBinaryEvent->getBinaryDataPointer());

    EXPECT_EQ(data[0], 0);
    EXPECT_EQ(data[1], 1);
    EXPECT_EQ(data[2], 2);
    EXPECT_EQ(data[3], 3);
}