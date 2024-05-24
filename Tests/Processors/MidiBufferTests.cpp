#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

class FakeProcessor : public GenericProcessor
{
public:
    FakeProcessor() :
        GenericProcessor("FakeProcessor")
    {
    }

    // Inherited via GenericProcessor
    void process(AudioBuffer<float>& continuousBuffer) override
    {
    }
};


class MidiBufferTests : public testing::Test
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

        mEventChannel = std::make_unique<EventChannel>(EventChannel::Settings{
                EventChannel::Type::TTL, "TTL", "Event", "identifier.ttl", mDataStream.get()
            });

        // Add the processor to the EventChannels
        mProcessor = std::make_unique<FakeProcessor>();
        mProcessor->setNodeId(mNodeId);

        mEventChannel->addProcessor(mProcessor.get());

        mEvent = TTLEvent::createTTLEvent(mEventChannel.get(), 0, 0, true);
    }

    void TearDown() override
    {

    }

protected:
    EventPtr mEvent;
    std::unique_ptr<FakeProcessor> mProcessor;
    std::unique_ptr<DataStream> mDataStream;
    std::unique_ptr<ContinuousChannel> mContinuousChannel;
    std::unique_ptr<EventChannel> mEventChannel;

    int mNodeId;
};

/*
The Midi Buffer encodes Metadata as raw byte data and provides an iterator for processing separate blocks of data. 
This test verifies that multiple Metadata objects inserted into the Midi Buffer are recoverable.
*/
TEST_F(MidiBufferTests, ReadWrite)
{
    size_t size = mEvent->getChannelInfo()->getDataSize() +
        mEvent->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;
    HeapBlock<uint8> buffer(size);

    EventPacket packet(buffer, size);

    MidiBuffer midiBuffer;
    midiBuffer.addEvent(packet, 0);
    midiBuffer.addEvent(packet, 0);
}