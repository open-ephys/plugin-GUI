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
        mTester = std::make_unique<DataThreadTester>(TestSourceNodeBuilder(FakeSourceNodeParams{}));
        mProcessor = mTester->CreateDataThread<FakeDataThread>();
    }

    void TearDown() override
    {}

protected:
    std::unique_ptr<DataThreadTester> mTester;
    FakeDataThread* mProcessor;
};

TEST_F(DataThreadTests, DataIntegrity)
{

}

TEST_F(DataThreadTests, ConfigureAcquisition)
{

}