#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

class MockChannelInfoObject : public ChannelInfoObject
{
public:
    MockChannelInfoObject (InfoObject::Type type, DataStream* stream) : ChannelInfoObject (type, stream)
    {
    }

    ~MockChannelInfoObject() noexcept override = default;

    DataStream* getStream()
    {
        return stream;
    }
};

TEST (ChannelInfoObjectTests, GetSampleRate)
{
    // Arrange
    float expectedSampleRate = 44100.0f;
    DataStream::Settings settings;
    settings.sample_rate = expectedSampleRate;
    DataStream stream(settings);
    ChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, &stream);

    // Act
    float actualSampleRate = channel.getSampleRate();

    // Assert
    EXPECT_EQ (expectedSampleRate, actualSampleRate);
}

TEST (ChannelInfoObjectTests, GetStreamId)
{
    // Arrange
    DataStream::Settings settings;
    DataStream stream (settings);
    ChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, &stream);
    uint16_t expectedStreamId = stream.getStreamId();

    // Act
    uint16_t actualStreamId = channel.getStreamId();

    // Assert
    EXPECT_EQ (expectedStreamId, actualStreamId);
}

TEST (ChannelInfoObjectTests, GetStreamName)
{
    // Arrange
    DataStream::Settings settings {
        "Channel 1"
    };
    DataStream stream (settings);
    ChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, &stream);
    std::string expectedStreamName = "Channel 1";

    // Act
    String actualStreamName = channel.getStreamName();

    // Assert
    EXPECT_EQ (expectedStreamName, actualStreamName);
}

TEST (ChannelInfoObjectTests, SetDataStream_AddsToStream)
{
    // Arrange
    DataStream stream ({});
    MockChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, nullptr);

    // Act
    channel.setDataStream (&stream, true);

    // Assert
    EXPECT_EQ (&stream, channel.getStream());
}

TEST (ChannelInfoObjectTests, SetDataStream_DoesNotAddToStream)
{
    // Arrange
    DataStream stream ({});
    MockChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, nullptr);

    // Act
    channel.setDataStream (&stream, false);

    // Assert
    EXPECT_TRUE (channel.getStream()->getContinuousChannels().isEmpty());
}

TEST (ChannelInfoObjectTests, IsRecorded_DefaultValueIsFalse)
{
    // Arrange
    DataStream stream ({});
    ChannelInfoObject channel (InfoObject::Type::CONTINUOUS_CHANNEL, &stream);

    // Act
    bool isRecorded = channel.isRecorded;

    // Assert
    EXPECT_FALSE (isRecorded);
}