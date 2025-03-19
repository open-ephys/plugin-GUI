#include "gtest/gtest.h"

#include <DataThreadHeaders.h>
#include <TestFixtures.h>

class FakeDataThread : public DataThread
{
public:
    FakeDataThread(SourceNode* sn) :
        DataThread(sn)
    {}

    ~FakeDataThread() noexcept override = default;

    bool updateBuffer() override
    {
        return false;
    }

    bool foundInputSource() override
    {
        return false;
    }

    bool startAcquisition() override
    {
        return true;
    }

    bool stopAcquisition() override
    {
        return true;
    }

    void updateSettings(
        OwnedArray<ContinuousChannel>* continuousChannels,
        OwnedArray<EventChannel>* eventChannels,
        OwnedArray<SpikeChannel>* spikeChannels,
        OwnedArray<DataStream>* sourceStreams,
        OwnedArray<DeviceInfo>* devices,
        OwnedArray<ConfigurationObject>* configurationObjects) override
    {}
};

class DataThreadTests : public testing::Test
{
protected:
    void SetUp() override
    {
        tester = std::make_unique<DataThreadTester>(TestSourceNodeBuilder(FakeSourceNodeParams{}));
        processor = tester->createDataThread<FakeDataThread>();
    }

    AudioBuffer<float> createBuffer(float startingValue, float step, int numChannels, int numSamples)
    {
        AudioBuffer<float> inputBuffer(numChannels, numSamples);

        // in microvolts
        float curValue = startingValue;

        for (int chidx = 0; chidx < numChannels; chidx++)
        {
            for (int sample_idx = 0; sample_idx < numSamples; sample_idx++)
            {
                inputBuffer.setSample(chidx, sample_idx, curValue);
                curValue += step;
            }
        }

        return inputBuffer;
    }

    void writeBlock(AudioBuffer<float>& buffer)
    {
        auto audioProcessor = (AudioProcessor*)tester->getSourceNode();
        auto dataStreams = tester->getSourceNode()->getDataStreams();

        ASSERT_EQ(dataStreams.size(), 1);

        auto streamId = dataStreams[0]->getStreamId();
        HeapBlock<char> data;
        size_t dataSize = SystemEvent::fillTimestampAndSamplesData(
            data,
            tester->getSourceNode(),
            streamId,
            currentSampleIndex,
            0,
            buffer.getNumSamples(),
            0);

        MidiBuffer eventBuffer;
        eventBuffer.addEvent(data, dataSize, 0);

        auto originalBuffer = buffer;
        audioProcessor->processBlock(buffer, eventBuffer);

        // Assert the buffer hasn't changed after process()
        ASSERT_EQ(buffer.getNumSamples(), originalBuffer.getNumSamples());
        ASSERT_EQ(buffer.getNumChannels(), originalBuffer.getNumChannels());

        for (int chidx = 0; chidx < buffer.getNumChannels(); chidx++)
        {
            for (int sampleIdx = 0; sampleIdx < buffer.getNumSamples(); ++sampleIdx)
                ASSERT_EQ(buffer.getSample(chidx, sampleIdx), originalBuffer.getSample(chidx, sampleIdx));
        }

        currentSampleIndex += buffer.getNumSamples();
    }

protected:
    std::unique_ptr<DataThreadTester> tester;
    FakeDataThread* processor;
    int64_t currentSampleIndex = 0;
};

TEST_F(DataThreadTests, DataIntegrity)
{
    tester->startAcquisition(false);

    int numSamples = 100;
    auto inputBuffer = createBuffer(1000.0, 20.0, 5, numSamples);
    writeBlock(inputBuffer);

    tester->stopAcquisition();
}