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

        dataStream = std::make_unique<DataStream>(dataStreamSettings);

        // Create a ContinuousChannel and add it to the DataStream
        ContinuousChannel::Settings continuousChannelSettings{
            ContinuousChannel::Type::ELECTRODE,
            "CH0",
            "0",
            "identifier",
            1.0f,
            dataStream.get()
        };

        continuousChannel = std::make_unique<ContinuousChannel>(continuousChannelSettings);
        dataStream->addChannel(continuousChannel.get());

        // Set the source node id for the data stream
        nodeId = 0;
        dataStream->setNodeId(nodeId);

        // Create two EventChannels and add them to the DataStream
        eventChannel.emplace("TTL", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL, "TTL", "Event", "identifier.ttl", dataStream.get()
            }));
        eventChannel.emplace("Text", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TEXT, "Text", "Event", "identifier.text", dataStream.get()
            }));
        eventChannel.emplace("Event", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::TTL, "Event", "Event", "identifier.event", dataStream.get()
            }));

        float data[] = { 0, 1, 2 ,3 };

        eventChannel.emplace("Binary", std::make_unique<EventChannel>(
            EventChannel::Settings{
                EventChannel::Type::CUSTOM, 
                "Binary", 
                "Event", 
                "identifier.binary",
                dataStream.get(), 
                8,
                EventChannel::BinaryDataType::FLOAT_ARRAY, 
                sizeof(data) / sizeof(data[0])
            }));

        // Add the processor to the EventChannels
        processor = std::make_unique<MockProcessor>();
        processor->setNodeId(nodeId);

        for (const auto& channel : eventChannel)
            channel.second->addProcessor(processor.get());

        // Create Events
        event = new FakeEvent(eventChannel["Event"].get(), 0);
        ttlEvent = TTLEvent::createTTLEvent(eventChannel["TTL"].get(), 0, 0, true);
        textEvent = TextEvent::createTextEvent(eventChannel["Text"].get(), 0, "Text");
        binaryEvent = BinaryEvent::createBinaryEvent<float>(eventChannel["Binary"].get(), 0, data, sizeof(data));
    }

protected:
    std::unique_ptr<MockProcessor> processor;
    std::unique_ptr<DataStream> dataStream;
    std::unique_ptr<ContinuousChannel> continuousChannel;
    std::unordered_map<String, std::unique_ptr<EventChannel>> eventChannel;
    EventPtr event;
    TTLEventPtr ttlEvent;
    TextEventPtr textEvent;
    BinaryEventPtr binaryEvent;

    int nodeId;
};

/*
Event should return the correct event type.
*/
TEST_F(EventTests, GetEventType)
{
    EXPECT_EQ(event->getEventType(), EventChannel::Type::TTL);
    EXPECT_EQ(ttlEvent->getEventType(), EventChannel::Type::TTL);
    EXPECT_EQ(textEvent->getEventType(), EventChannel::Type::TEXT);
    EXPECT_EQ(binaryEvent->getEventType(), EventChannel::Type::CUSTOM);
}

/*
Event should return the correct EventChannel info object.
*/
TEST_F(EventTests, GetEventChannelInfo)
{
    EXPECT_EQ(event->getChannelInfo(), eventChannel["Event"].get());
    EXPECT_EQ(ttlEvent->getChannelInfo(), eventChannel["TTL"].get());
    EXPECT_EQ(textEvent->getChannelInfo(), eventChannel["Text"].get());
    EXPECT_EQ(binaryEvent->getChannelInfo(), eventChannel["Binary"].get());
}

/*
Events should should correctly be serialized and deserialized.
*/
TEST_F(EventTests, SerializeDeserializeEvent)
{
    // TTLEvent
    {
        // Serialize the event
        size_t size = ttlEvent->getChannelInfo()->getDataSize() +
            ttlEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
        HeapBlock<uint8> buffer(size);

        ttlEvent->serialize(buffer, size);

        {
            // Deserialize the event
            auto deserializedEvent = TTLEvent::deserialize(buffer.getData(), ttlEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(ttlEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }

        {
            EventPacket packet(buffer, size);

            // Deserialize the event
            auto deserializedEvent = TTLEvent::deserialize(packet, ttlEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(ttlEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }
    }

    // TextEvent
    {
        // Serialize the event
        size_t size = textEvent->getChannelInfo()->getDataSize() +
            textEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
        HeapBlock<uint8> buffer(size);

        textEvent->serialize(buffer, size);

        {
            // Deserialize the event
            auto deserializedEvent = TextEvent::deserialize(buffer.getData(), textEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(textEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }

        {
            EventPacket packet(buffer, size);

            // Deserialize the event
            auto deserializedEvent = TextEvent::deserialize(packet, textEvent->getChannelInfo());

            // Check that the deserialized event is the same as the original event
            EXPECT_EQ(textEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
        }
    }
}

/*
TTLEvent should return the correct state.
*/
TEST_F(EventTests, GetState)
{
    EXPECT_EQ(ttlEvent->getState(), true);
}

/*
TTLEvent should return the correct line.
*/
TEST_F(EventTests, GetLine)
{
    EXPECT_EQ(ttlEvent->getLine(), 0);
}

/*
TTLEvent should return the correct word.
*/
TEST_F(EventTests, GetWord)
{
    EXPECT_EQ(ttlEvent->getWord(), true);
}

/*
TextEvent should return the correct text.
*/
TEST_F(EventTests, GetText)
{
    EXPECT_EQ(textEvent->getText(), "Text");
}

/*
BinaryEvent should return the correct type.
*/
TEST_F(EventTests, GetBinaryType)
{
    EXPECT_EQ(binaryEvent->getBinaryType(), EventChannel::BinaryDataType::FLOAT_ARRAY);
}

/*
BinaryEvent should return the correct data.
*/
TEST_F(EventTests, GetBinaryDataPointer)
{
    const float* data = static_cast<const float*>(binaryEvent->getBinaryDataPointer());

    EXPECT_EQ(data[0], 0);
    EXPECT_EQ(data[1], 1);
    EXPECT_EQ(data[2], 2);
    EXPECT_EQ(data[3], 3);
}