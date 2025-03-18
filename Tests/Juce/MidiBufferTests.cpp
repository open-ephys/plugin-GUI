#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

class MockProcessor : public GenericProcessor
{
public:
    MockProcessor() :
        GenericProcessor("MockProcessor")
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

        // Set the source node id for the data stream
        nodeId = 0;
        dataStream->setNodeId(nodeId);

        eventChannel = std::make_unique<EventChannel>(EventChannel::Settings{
                EventChannel::Type::TTL, "TTL", "Event", "identifier.ttl", dataStream.get()
            });

        // Add the processor to the EventChannels
        processor = std::make_unique<MockProcessor>();
        processor->setNodeId(nodeId);

        dataStream->addProcessor (processor.get());
        continuousChannel->addProcessor (processor.get());
        eventChannel->addProcessor(processor.get());

        event = TTLEvent::createTTLEvent(eventChannel.get(), 0, 0, true);
    }

protected:
    TTLEventPtr event;
    std::unique_ptr<MockProcessor> processor;
    std::unique_ptr<DataStream> dataStream;
    std::unique_ptr<ContinuousChannel> continuousChannel;
    std::unique_ptr<EventChannel> eventChannel;

    int nodeId;
};

/*
The Midi Buffer encodes Metadata as raw byte data and provides an iterator for processing separate blocks of data. 
This test verifies that multiple Metadata objects inserted into the Midi Buffer are recoverable.
*/
TEST_F(MidiBufferTests, ReadWrite)
{
    size_t size = event->getChannelInfo()->getDataSize() + 
        event->getChannelInfo()->getTotalEventMetadataSize() + 
        EVENT_BASE_SIZE;
    HeapBlock<uint8> buffer (size);

    EventPacket packet (buffer, size);

    MidiBuffer midiBuffer;
    
    midiBuffer.addEvent (packet, 0);
    midiBuffer.addEvent (packet, 0);

    for(auto it = midiBuffer.begin(); it != midiBuffer.end(); ++it)
    {
        auto metaData = *it;

        auto expectedMetaData = packet.getRawData();
        auto expectedMetaDataLength = packet.getRawDataSize();

        EXPECT_EQ (metaData.numBytes, expectedMetaDataLength);
        
        for (int i = 0; i < expectedMetaDataLength; ++i)
        {
			EXPECT_EQ (metaData.data[i], expectedMetaData[i]);
		}
    }
}

/*
Clear the Midi Buffer and verify that the buffer is empty.
*/
TEST_F(MidiBufferTests, Clear)
{
    size_t size = event->getChannelInfo()->getDataSize() + 
        event->getChannelInfo()->getTotalEventMetadataSize() + 
        EVENT_BASE_SIZE;
    HeapBlock<uint8> buffer (size);

    EventPacket packet (buffer, size);

    MidiBuffer midiBuffer;
    
    midiBuffer.addEvent (packet, 0);
    midiBuffer.addEvent (packet, 0);

    midiBuffer.clear();

    EXPECT_TRUE(midiBuffer.isEmpty());
}

/*
Adding an event to the Midi buffer should correctly increment the number of events in the buffer.
*/ 
TEST_F(MidiBufferTests, AddEvent)
{
    size_t size = event->getChannelInfo()->getDataSize() + 
        event->getChannelInfo()->getTotalEventMetadataSize() + 
        EVENT_BASE_SIZE;
    HeapBlock<uint8> buffer (size);

    EventPacket packet (buffer, size);

    MidiBuffer midiBuffer;
    
    midiBuffer.addEvent (packet, 0);
    midiBuffer.addEvent (packet, 0);

    EXPECT_EQ(midiBuffer.getNumEvents(), 2);
}

/*
Should be able to add events from another buffer to the Midi Buffer.
*/
TEST_F(MidiBufferTests, AddBuffer)
{
    size_t size = event->getChannelInfo()->getDataSize() + 
        event->getChannelInfo()->getTotalEventMetadataSize() + 
        EVENT_BASE_SIZE;
    HeapBlock<uint8> buffer (size);

    EventPacket packet (buffer, size);

    MidiBuffer midiBuffer;
    
    midiBuffer.addEvent (packet, 0);
    midiBuffer.addEvent (packet, 0);

    MidiBuffer midiBuffer2;
    midiBuffer2.addEvent (packet, 0);
    midiBuffer2.addEvent (packet, 0);

    midiBuffer.addEvents(midiBuffer2, 0, size, 0);

    EXPECT_EQ(midiBuffer.getNumEvents(), 4);
}