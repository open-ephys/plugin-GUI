#include "gtest/gtest.h"

#include <DataThreadHeaders.h>

/*
The Read/Write test will verify the Audio Buffer’s fundamental ability to store and retrieve data. 
Size parameters will passed that will define the number of channels and samples the buffer should hold. 
Data will then be written to the buffer using the buffer’s write pointers. 
The written data will be verified through the buffer’s read pointers.
*/
TEST (AudioBufferTest, ReadWrite)
{
    // Create an Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer (1, 10);

    // Write data to the buffer
    for (int channel = 0; channel < audioBuffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer.getNumSamples(); ++sample)
            audioBuffer.setSample (channel, sample, sample);
    }

    // Verify that the data was written successfully
    for (int channel = 0; channel < audioBuffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer.getNumSamples(); ++sample)
            EXPECT_EQ (audioBuffer.getSample (channel, sample), sample);
    }
}

/*
The Audio Buffer defines an explicit method for copying data from a different Audio Buffer to itself. 
This test will verify that an empty buffer can successfully copy data from an Audio Buffer with existing data.
*/
TEST (AudioBufferTest, MakeCopy)
{
    // Create a Data Buffer with 1 channel and 10 samples
    constexpr int numItems = 10;
    DataBuffer dataBuffer (1, numItems + 1);

    {
        // Add data to the buffer
        float data[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int64 sampleNumbers[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double timestamps[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint64 eventCodes[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        dataBuffer.addToBuffer (data, sampleNumbers, timestamps, eventCodes, numItems);
    }

    // Create an Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer (1, numItems);

    // Copy data from the Data Buffer to the Audio Buffer
    int64 sampleNumbers[numItems];
    double timestamps[numItems];
    uint64 eventCodes[numItems];
    std::optional<int64> timestampSampleIndex;

    dataBuffer.readAllFromBuffer (audioBuffer, sampleNumbers, timestamps, eventCodes, numItems, &timestampSampleIndex);

    // Create a second Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer2 (1, numItems);

    // Copy data from the first Audio Buffer to the second Audio Buffer
    audioBuffer2.makeCopyOf (audioBuffer);

    // Verify that the data was copied successfully
    for (int channel = 0; channel < audioBuffer2.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer2.getNumSamples(); ++sample)
            EXPECT_EQ (audioBuffer2.getSample (channel, sample), sample);
    }
}

/*
Audio Buffers can append samples for a channel from a different Audio Buffer to itself. 
This test will verify that channel data can successfully be appended from an Audio Buffer with existing data.
*/
TEST (AudioBufferTest, AddFrom)
{
    // Create a Data Buffer with 1 channel and 10 samples
    constexpr int numItems = 10;
    DataBuffer dataBuffer (1, numItems + 1);

    {
        // Add data to the buffer
        float data[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int64 sampleNumbers[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double timestamps[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint64 eventCodes[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        dataBuffer.addToBuffer (data, sampleNumbers, timestamps, eventCodes, numItems);
    }

    // Create an Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer (1, numItems);

    // Copy data from the Data Buffer to the Audio Buffer
    int64 sampleNumbers[numItems];
    double timestamps[numItems];
    uint64 eventCodes[numItems];
    std::optional<int64> timestampSampleIndex;

    dataBuffer.readAllFromBuffer (audioBuffer, sampleNumbers, timestamps, eventCodes, numItems, &timestampSampleIndex);

    AudioBuffer<float> audioBuffer2(1, numItems);
    audioBuffer2.clear();

    // Append data from the first Audio Buffer to the second Audio Buffer
    audioBuffer2.addFrom (0, 0, audioBuffer, 0, 0, numItems);

    // Verify that the data was appended successfully
    for (int channel = 0; channel < audioBuffer2.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer2.getNumSamples(); ++sample)
            EXPECT_EQ (audioBuffer2.getSample (channel, sample), sample);
    }
}

/*
Audio Buffers can copy samples for a channel from a different Audio Buffer to itself. 
This test will verify that channel data can successfully be copied from an Audio Buffer with existing data.
*/
TEST (AudioBufferTest, CopyFrom)
{
    // Create a Data Buffer with 1 channel and 10 samples
    constexpr int numItems = 10;
    DataBuffer dataBuffer (1, numItems + 1);

    {
        // Add data to the buffer
        float data[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int64 sampleNumbers[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        double timestamps[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        uint64 eventCodes[numItems] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

        dataBuffer.addToBuffer (data, sampleNumbers, timestamps, eventCodes, numItems);
    }

    // Create an Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer (1, numItems);

    // Copy data from the Data Buffer to the Audio Buffer
    int64 sampleNumbers[numItems];
    double timestamps[numItems];
    uint64 eventCodes[numItems];
    std::optional<int64> timestampSampleIndex;

    dataBuffer.readAllFromBuffer (audioBuffer, sampleNumbers, timestamps, eventCodes, numItems, &timestampSampleIndex);

    // Create a second Audio Buffer with 1 channel and 10 samples
    AudioBuffer<float> audioBuffer2 (1, numItems);

    // Copy data from the first Audio Buffer to the second Audio Buffer
    audioBuffer2.copyFrom (0, 0, audioBuffer, 0, 0, numItems);

    // Verify that the data was copied successfully
    for (int channel = 0; channel < audioBuffer2.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < audioBuffer2.getNumSamples(); ++sample)
            EXPECT_EQ (audioBuffer2.getSample (channel, sample), sample);
    }
}