#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class FakeProcessor : public GenericProcessor
{
public:
    FakeProcessor() :
        GenericProcessor("FakeProcessor")
    {}

    // Inherited via GenericProcessor
    void process(AudioBuffer<float>& continuousBuffer) override
    {
    }
};

class EventTests : public testing::Test
{
protected:
    void SetUp() override
    {
        mProcessor = std::make_unique<FakeProcessor>();

        mNodeId = 0;
        mProcessor->setNodeId(mNodeId);
    }

    void TearDown() override
    {

    }

protected:
    std::unique_ptr<FakeProcessor> mProcessor;
    int mNodeId;
};

/*
Events should should correctly be serialized and deserialized.
*/
TEST_F(EventTests, SerializeDeserializeEvent)
{
    // Create a DataStream
    DataStream::Settings dataStreamSettings{
        "DataStream",
        "description",
        "identifier",
        20000.0f
    };

    std::unique_ptr<DataStream> dataStream = std::make_unique<DataStream>(dataStreamSettings);

    // Create a ContinuousChannel and add it to the DataStream
    ContinuousChannel::Settings continuousChannelSettings{
        ContinuousChannel::Type::ELECTRODE,
        "CH0",
        "0",
        "identifier",
        1.0f,
        dataStream.get()
    };

    std::unique_ptr<ContinuousChannel> continuousChannel = std::make_unique<ContinuousChannel>(continuousChannelSettings);
    dataStream->addChannel(continuousChannel.get());

    // Set the source node id for the data stream
    dataStream->setNodeId(mNodeId);

    // Create a TTL event
    EventChannel::Settings settings{
        EventChannel::Type::TTL,
        "TTL",
        "TTL",
        "identifier.ttl.events",
        dataStream.get()
    };

    std::unique_ptr<EventChannel> ttlChannel = std::make_unique<EventChannel>(settings);
    ttlChannel->addProcessor(mProcessor.get());

    auto ttlEvent = TTLEvent::createTTLEvent(ttlChannel.get(), 0, 0, true);

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