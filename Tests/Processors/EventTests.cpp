#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class EventTests : testing::Test
{
protected:
    void SetUp() override
    {
        mProcessor = std::make_unique<GenericProcessor>();
    }

    void TearDown() override
    {

    }

protected:
    std::unique_ptr<GenericProcessor> mProcessor;
};

/*
Events should should correctly be serialized and deserialized.
*/
TEST(EventTests, SerializeEvent)
{
    // Create a DataStream
    DataStream::Settings dataStreamSettings{
        "DataStream",
        "description",
        "identifier",
        20000.0f
    };

    DataStream* dataStream = new DataStream(dataStreamSettings);

    // Create a ContinuousChannel and add it to the DataStream
    ContinuousChannel::Settings continuousChannelSettings{
        ContinuousChannel::Type::ELECTRODE,
        "CH0",
        "0",
        "identifier",
        1.0f,
        dataStream
    };

    ContinuousChannel* continuousChannel = new ContinuousChannel(continuousChannelSettings);
    dataStream->addChannel(continuousChannel);

    // Set the source node id for the data stream
    dataStream->setNodeId(0);

    // Create a TTL event
    EventChannel::Settings settings{
        EventChannel::Type::TTL,
        "TTL",
        "TTL",
        "identifier.ttl.events",
        dataStream
    };

    EventChannel* ttlChannel = new EventChannel(settings);

    auto ttlEvent = TTLEvent::createTTLEvent(ttlChannel, 0, 0, true);

    // Serialize the event
    size_t ttlSize = ttlEvent->getChannelInfo()->getDataSize() +
        ttlEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
    HeapBlock<uint8> ttlBuffer(ttlSize);
    ttlEvent->serialize(ttlBuffer, ttlSize);

    // Deserialize the event
    auto deserializedEvent = TTLEvent::deserialize(ttlBuffer.getData(), ttlEvent->getChannelInfo());

    // Check that the deserialized event is the same as the original event
    EXPECT_EQ(ttlEvent->getChannelInfo(), deserializedEvent->getChannelInfo());
}