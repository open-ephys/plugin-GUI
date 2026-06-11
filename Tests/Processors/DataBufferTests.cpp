#include "gtest/gtest.h"

#include <DataThreadHeaders.h>

/*
Continuous Data and Metadata are pushed to the Data Buffer.
This data can then be copied to an Audio Buffer.
The Data Buffer will attempt to copy the maximum number of samples,
dependent on the number of samples within the Data Buffer and the size of the Audio Buffer.
This test verifies that the Data Buffer can successfully perform this copy.
*/
TEST(DataBufferTest, CopyToAudioBuffer)
{
    // Create a Data Buffer with 1 channel and 10 samples
    constexpr int numItems = 10;
    DataBuffer dataBuffer(1, numItems + 1);

    {
        // Add data to the buffer
        float data[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int64 sampleNumbers[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double timestamps[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint64 eventCodes[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        dataBuffer.addToBuffer(data, sampleNumbers, timestamps, eventCodes, numItems);
    }

    // Create an Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer(1, numItems);

    // Copy data from the Data Buffer to the Audio Buffer
    int64 sampleNumbers[numItems];
    double timestamps[numItems];
    uint64 eventCodes[numItems];

    dataBuffer.readAllFromBuffer(audioBuffer, sampleNumbers, timestamps, eventCodes, numItems);

    // Verify that the data was copied successfully
    for (int channel = 0; channel < audioBuffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer.getNumSamples(); ++sample)
            EXPECT_EQ(audioBuffer.getSample(channel, sample), sample);
    }

    for (int sample = 0; sample < numItems; ++sample)
    {
        EXPECT_EQ(timestamps[sample], sample);
        EXPECT_EQ(eventCodes[sample], (uint64) sample);
    }
}

TEST(DataBufferTest, CopyToAudioBufferAcrossWrap)
{
    constexpr int bufferSize = 11;
    constexpr int firstWriteSize = 8;
    constexpr int secondWriteSize = 6;
    constexpr int firstReadSize = 5;

    DataBuffer dataBuffer (1, bufferSize);

    {
        float data[firstWriteSize] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        int64 sampleNumbers[firstWriteSize] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        double timestamps[firstWriteSize] = { 0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5 };
        uint64 eventCodes[firstWriteSize] = { 10, 11, 12, 13, 14, 15, 16, 17 };

        EXPECT_EQ (dataBuffer.addToBuffer (data, sampleNumbers, timestamps, eventCodes, firstWriteSize), firstWriteSize);
    }

    {
        AudioBuffer<float> discardBuffer (1, firstReadSize);
        int64 sampleNumber = -1;
        double timestamps[firstReadSize] = { -1, -1, -1, -1, -1 };
        uint64 eventCodes[firstReadSize] = { 0, 0, 0, 0, 0 };

        EXPECT_EQ (dataBuffer.readAllFromBuffer (discardBuffer, &sampleNumber, timestamps, eventCodes, firstReadSize), firstReadSize);
    }

    {
        float data[secondWriteSize] = { 100, 101, 102, 103, 104, 105 };
        int64 sampleNumbers[secondWriteSize] = { 8, 9, 10, 11, 12, 13 };
        double timestamps[secondWriteSize] = { 4.0, 4.5, 5.0, 5.5, 6.0, 6.5 };
        uint64 eventCodes[secondWriteSize] = { 18, 19, 20, 21, 22, 23 };

        EXPECT_EQ (dataBuffer.addToBuffer (data, sampleNumbers, timestamps, eventCodes, secondWriteSize), secondWriteSize);
    }

    constexpr int expectedReadSize = firstWriteSize - firstReadSize + secondWriteSize;
    AudioBuffer<float> audioBuffer (1, expectedReadSize);
    int64 startSampleNumber = -1;
    double timestamps[expectedReadSize] = {};
    uint64 eventCodes[expectedReadSize] = {};

    EXPECT_EQ (dataBuffer.readAllFromBuffer (audioBuffer, &startSampleNumber, timestamps, eventCodes, expectedReadSize), expectedReadSize);
    EXPECT_EQ (startSampleNumber, 5);

    const float expectedSamples[expectedReadSize] = { 5, 6, 7, 100, 101, 102, 103, 104, 105 };
    const double expectedTimestamps[expectedReadSize] = { 2.5, 3.0, 3.5, 4.0, 4.5, 5.0, 5.5, 6.0, 6.5 };
    const uint64 expectedEventCodes[expectedReadSize] = { 15, 16, 17, 18, 19, 20, 21, 22, 23 };

    for (int sample = 0; sample < expectedReadSize; ++sample)
    {
        EXPECT_EQ (audioBuffer.getSample (0, sample), expectedSamples[sample]);
        EXPECT_EQ (timestamps[sample], expectedTimestamps[sample]);
        EXPECT_EQ (eventCodes[sample], expectedEventCodes[sample]);
    }
}