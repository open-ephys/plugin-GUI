#include "gtest/gtest.h"

#include "Audio/AudioComponent.h"

TEST (AudioComponentTests, FindsClosestBufferSizeForDuration)
{
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration (
                   { 128, 512, 1024, 2048 },
                   44100.0,
                   23.0),
               1024);

    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration (
                   { 512, 1024, 1152, 2048 },
                   48000.0,
                   23.0),
               1152);

    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration (
                   { 4096, 512, 2048, 1024 },
                   96000.0,
                   23.0),
               2048);
}

TEST (AudioComponentTests, PrefersLargerBufferWhenDistancesAreEqual)
{
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration (
                   { 10, 20 },
                   1000.0,
                   15.0),
               20);
}

TEST (AudioComponentTests, ReturnsZeroWithoutAValidChoice)
{
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration ({}, 48000.0, 23.0), 0);
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration ({ 0, -128 }, 48000.0, 23.0), 0);
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration ({ 1024 }, 0.0, 23.0), 0);
    EXPECT_EQ (AudioComponent::findClosestBufferSizeForDuration ({ 1024 }, 48000.0, 0.0), 0);
}
