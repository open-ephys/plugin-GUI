#include "gtest/gtest.h"

#include <Processors/FileReader/BinaryFileSource/BinaryFileSource.h>
#include <Processors/FileReader/FileReader.h>
#include <TestFixtures.h>

class TestPlaybackFileSource : public FileSource
{
public:
    TestPlaybackFileSource (std::vector<int64> samples, int64 numSamples = 163444) : eventSamples (std::move (samples))
    {
        infoArray.add ({ "test", {}, numSamples, 40000.0f, 0 });
        setActiveRecord (0);
    }

    bool open (File) override { return true; }
    void fillRecordInfo() override {}
    void updateActiveRecord (int) override {}
    void seekTo (int64 sampleNumber) override { position = sampleNumber; }

    int readData (float* buffer, int numSamples) override
    {
        const int count = int (jmin (int64 (numSamples), getActiveNumSamples() - position));
        for (int sample = 0; sample < count; ++sample)
            buffer[sample] = float (position + sample);
        position += count;
        return count;
    }

    void processEventData (EventInfo& events, int64 start, int64 stop) override
    {
        queries.emplace_back (start, stop);
        for (const auto sample : eventSamples)
        {
            if (sample >= start && sample < stop)
            {
                events.channels.push_back (0);
                events.channelStates.push_back (1);
                events.sampleNumbers.push_back (sample);
                events.text.emplace_back();
            }
        }
    }

    std::vector<std::pair<int64, int64>> queries;

private:
    int64 position = 0;
    std::vector<int64> eventSamples;
};

class TestBinaryFileSource : public BinarySource::BinaryFileSource
{
public:
    TestBinaryFileSource()
    {
        infoArray.add ({ "test", {}, 163444, 40000.0f, 0 });
        setActiveRecord (0);
        currentStream = "test";

        EventInfo events;
        events.channels = { 1, 1, 1, 1 };
        events.channelStates = { 1, 0, 1, 0 };
        events.sampleNumbers = { 0, 10, 163440, 163443 };
        events.text.resize (4);
        eventInfoMap[currentStream] = events;
    }

    void updateActiveRecord (int) override {}
};

TEST (BinaryFileSourceTests, BinaryEventsIncludeLastSampleAtExclusiveFileEnd)
{
    TestBinaryFileSource source;
    EventInfo events;
    source.processEventData (events, 163440, 163444);

    EXPECT_EQ (events.sampleNumbers, (std::vector<int64> { 163440, 163443 }));
}

TEST (BinaryFileSourceTests, BinaryEventsUseHalfOpenRanges)
{
    TestBinaryFileSource source;
    EventInfo events;
    source.processEventData (events, 0, 10);

    EXPECT_EQ (events.sampleNumbers, (std::vector<int64> { 0 }));
}

TEST (BinaryFileSourceTests, BinaryEventsRespectSelectedEnd)
{
    TestBinaryFileSource source;
    EventInfo events;
    source.processEventData (events, 10, 163440);

    EXPECT_EQ (events.sampleNumbers, (std::vector<int64> { 10 }));
}

class FileReaderTests : public testing::Test
{
protected:
    void SetUp() override
    {
        tester = std::make_unique<ProcessorTester> (TestSourceNodeBuilder (FakeSourceNodeParams {}));
        reader = std::make_unique<FileReader>();
        reader->setHeadlessMode (true);
        reader->setNodeId (101);
        reader->registerParameters();
        reader->input = std::make_unique<TestBinaryFileSource>();
        reader->currentSampleRate = 40000.0f;
        reader->startSample = 0;
        reader->stopSample = 163444;
    }

    void TearDown() override
    {
        reader.reset();
        tester.reset();
    }

    void preparePlayback (int64 start, int64 stop, int64 position, int blockSize,
                          std::vector<int64> events)
    {
        reader->input = std::make_unique<TestPlaybackFileSource> (std::move (events));
        reader->startSample = start;
        reader->stopSample = stop;
        reader->currentSample = position;
        reader->playbackSamplePos.set (position);
        reader->currentNumChannels = 1;
        reader->m_sysSampleRate = 40000.0f;
        reader->m_bufferSize = blockSize;
        reader->resetBufferSchedule();
        reader->input->seekTo (position);
        reader->bufferA.malloc (blockSize * BUFFER_WINDOW_CACHE_SIZE);
        reader->bufferB.malloc (blockSize * BUFFER_WINDOW_CACHE_SIZE);
        reader->readAndFillBufferCache (reader->bufferA, reader->bufferAInfo);
        reader->readAndFillBufferCache (reader->bufferB, reader->bufferBInfo);
        reader->readBuffer = &reader->bufferA;
        reader->readBufferInfo = &reader->bufferAInfo;

        reader->setProcessorType (Processor::Type::SOURCE);
        reader->dataStreams.add (new DataStream ({ "test", "test", "test", 40000.0f }));
        reader->dataStreams[0]->addProcessor (reader.get());
        reader->eventChannels.add (new EventChannel ({ EventChannel::Type::TTL, "TTL", "test", "test", reader->dataStreams[0] }));
        reader->eventChannels[0]->addProcessor (reader.get());
    }

    std::vector<std::pair<int64, int64>> getQueries() const
    {
        return static_cast<TestPlaybackFileSource*> (reader->input.get())->queries;
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& events)
    {
        static_cast<AudioProcessor*> (reader.get())->processBlock (buffer, events);
    }

    void seekTo (int64 sampleNumber)
    {
        reader->setCurrentSample (sampleNumber);
        reader->readAndFillBufferCache (reader->bufferA, reader->bufferAInfo);
        reader->readAndFillBufferCache (reader->bufferB, reader->bufferBInfo);
    }

    void setFileLength (int64 numSamples)
    {
        reader->input = std::make_unique<TestPlaybackFileSource> (std::vector<int64> {}, numSamples);
        reader->stopSample = numSamples;
    }

    int64 getBlockStartSample() const
    {
        return reader->getFirstSampleNumberForBlock (reader->dataStreams[0]->getStreamId());
    }

    std::vector<std::pair<int, int64>> getEvents (const MidiBuffer& buffer) const
    {
        std::vector<std::pair<int, int64>> events;
        for (const auto metadata : buffer)
        {
            if (EventBase::getBaseType (metadata.data) == EventBase::PROCESSOR_EVENT)
            {
                auto event = TTLEvent::deserialize (metadata.data, reader->getEventChannels()[0]);
                events.emplace_back (metadata.samplePosition, event->getSampleNumber());
            }
        }
        return events;
    }

    std::unique_ptr<ProcessorTester> tester;
    std::unique_ptr<FileReader> reader;
};

TEST_F (FileReaderTests, DisplayedFileEndPreservesAllSamples)
{
    reader->getParameter ("end_time")->setNextValue ("00:00:04.086", false);

    EXPECT_EQ (reader->getPlaybackStop(), 163444);
    EXPECT_EQ (reader->getCurrentNumTotalSamples(), 163444);
}

TEST_F (FileReaderTests, EarlierEndTimeRemainsMillisecondBased)
{
    reader->getParameter ("end_time")->setNextValue ("00:00:04.000", false);

    EXPECT_EQ (reader->getPlaybackStop(), 160000);
}

TEST_F (FileReaderTests, LongDurationConversionsKeepSamplePrecision)
{
    EXPECT_EQ (reader->samplesToMilliseconds (16777279), 419431u);
    EXPECT_EQ (reader->millisecondsToSamples (3355445), 134217800);
}

TEST_F (FileReaderTests, SubMillisecondFilePreservesAllSamples)
{
    setFileLength (4);
    reader->getParameter ("end_time")->setNextValue ("00:00:00.000", false);

    EXPECT_EQ (reader->getPlaybackStop(), 4);
}

TEST_F (FileReaderTests, FullFileWrapKeepsContinuousDataAndEventsAligned)
{
    preparePlayback (0, 163444, 163440, 8, { 0, 2, 163440, 163443 });
    AudioBuffer<float> buffer (1, 8);
    MidiBuffer events;
    processBlock (buffer, events);

    const std::vector<float> expected { 163440, 163441, 163442, 163443, 0, 1, 2, 3 };
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        EXPECT_EQ (buffer.getSample (0, sample), expected[sample]);
    EXPECT_EQ (getQueries(), (std::vector<std::pair<int64, int64>> { { 163440, 163444 }, { 0, 4 } }));
    EXPECT_EQ (getEvents (events), (std::vector<std::pair<int, int64>> { { 0, 0 }, { 3, 3 }, { 4, 4 }, { 6, 6 } }));
    EXPECT_EQ (reader->getPlayheadPosition(), 4);
}

TEST_F (FileReaderTests, ShortSelectedRangeCanWrapMultipleTimesInOneBlock)
{
    preparePlayback (10, 14, 12, 10, { 0, 10, 11, 13, 14 });
    AudioBuffer<float> buffer (1, 10);
    MidiBuffer events;
    processBlock (buffer, events);

    const std::vector<float> expected { 12, 13, 10, 11, 12, 13, 10, 11, 12, 13 };
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        EXPECT_EQ (buffer.getSample (0, sample), expected[sample]);
    EXPECT_EQ (getQueries(), (std::vector<std::pair<int64, int64>> { { 12, 14 }, { 10, 14 }, { 10, 14 } }));
    EXPECT_EQ (getEvents (events), (std::vector<std::pair<int, int64>> { { 1, 1 }, { 2, 2 }, { 3, 3 }, { 5, 5 }, { 6, 6 }, { 7, 7 }, { 9, 9 } }));
    EXPECT_EQ (reader->getPlayheadPosition(), 10);
}

TEST_F (FileReaderTests, OutputSampleNumbersContinueAcrossLoops)
{
    preparePlayback (0, 4, 0, 4, { 0, 3 });
    AudioBuffer<float> buffer (1, 4);
    MidiBuffer firstEvents;
    processBlock (buffer, firstEvents);
    EXPECT_EQ (getBlockStartSample(), 0);
    EXPECT_EQ (getEvents (firstEvents), (std::vector<std::pair<int, int64>> { { 0, 0 }, { 3, 3 } }));

    MidiBuffer secondEvents;
    processBlock (buffer, secondEvents);
    EXPECT_EQ (getBlockStartSample(), 4);
    EXPECT_EQ (getEvents (secondEvents), (std::vector<std::pair<int, int64>> { { 0, 4 }, { 3, 7 } }));
    EXPECT_EQ (reader->getPlayheadPosition(), 0);
}

TEST_F (FileReaderTests, SeekingDoesNotResetOutputSampleNumbers)
{
    preparePlayback (10, 14, 10, 2, { 10, 13 });
    AudioBuffer<float> buffer (1, 2);
    MidiBuffer firstEvents;
    processBlock (buffer, firstEvents);

    seekTo (13);
    MidiBuffer secondEvents;
    processBlock (buffer, secondEvents);

    EXPECT_EQ (buffer.getSample (0, 0), 13.0f);
    EXPECT_EQ (buffer.getSample (0, 1), 10.0f);
    EXPECT_EQ (getBlockStartSample(), 2);
    EXPECT_EQ (getEvents (secondEvents), (std::vector<std::pair<int, int64>> { { 0, 2 }, { 1, 3 } }));
    EXPECT_EQ (reader->getPlayheadPosition(), 11);
}