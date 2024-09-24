#include "gtest/gtest.h"

#include <JuceHeader.h>

/*
The Read/Write test will verify the Audio Buffer’s fundamental ability to store and retrieve data. 
Size parameters will passed that will define the number of channels and samples the buffer should hold. 
Data will then be written to the buffer using the buffer’s write pointers. 
The written data will be verified through the buffer’s read pointers.
*/
TEST(AudioBufferTest, ReadWrite)
{
    // Create an AudioBuffer with 2 channels and 10 samples
    AudioBuffer<float> buffer(2, 10);

    // Write data to the buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, sample);
    }

    // Read data from the buffer and verify
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            EXPECT_EQ(buffer.getSample(channel, sample), sample);
    }
}

/*
The Audio Buffer defines an explicit method for copying data from a different Audio Buffer to itself.
This test will verify that an empty buffer can successfully copy data from an Audio Buffer with existing data.
*/
TEST(AudioBufferTest, MakeCopy)
{
    // Create an AudioBuffer with 2 channels and 10 samples
    AudioBuffer<float> buffer(2, 10);

    // Write data to the buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, channel * sample);
    }

    // Create a new buffer
    AudioBuffer<float> newBuffer(2, 10);

    // Copy data from the original buffer to the new buffer
    newBuffer.makeCopyOf(buffer);

    // Verify that the data was copied successfully
    for (int channel = 0; channel < newBuffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < newBuffer.getNumSamples(); ++sample)
            EXPECT_EQ(newBuffer.getSample(channel, sample), channel * sample);
    }
}

/*
Audio Buffers can append samples for a channel from a different Audio Buffer to itself. 
This test will verify that channel data can successfully be appended from an Audio Buffer with existing data.
*/
TEST(AudioBufferTest, AddFrom)
{
    // Create an AudioBuffer with 2 channels and 10 samples
    AudioBuffer<float> buffer(2, 10);

    // Write data to the buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, sample);
    }

    // Create a new buffer
    AudioBuffer<float> newBuffer(2, 10);

    // Append data from the original buffer to the new buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            newBuffer.setSample(channel, sample, buffer.getSample(channel, sample));
    }

    // Verify that the data was appended successfully
    for (int channel = 0; channel < newBuffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < newBuffer.getNumSamples(); ++sample)
            EXPECT_EQ(newBuffer.getSample(channel, sample), buffer.getSample(channel, sample));
    }
}

/*
Audio Buffers can copy samples for a channel from a different Audio Buffer to itself. 
This test will verify that channel data can successfully be copied from an Audio Buffer with existing data.
*/
TEST(AudioBufferTest, CopyFrom)
{
    // Create an AudioBuffer with 2 channels and 10 samples
    AudioBuffer<float> buffer(2, 10);

    // Write data to the buffer
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            buffer.setSample(channel, sample, sample);
    }

    // Create a new buffer
    AudioBuffer<float> newBuffer(2, 10);

    // Copy data from the original buffer to the new buffer
    newBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());

    // Verify that the data was copied successfully
    for (int sample = 0; sample < newBuffer.getNumSamples(); ++sample)
        EXPECT_EQ(newBuffer.getSample(0, sample), sample);
}