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
}