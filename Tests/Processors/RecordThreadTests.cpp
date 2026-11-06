#include "gtest/gtest.h"

#include <Processors/RecordNode/RecordNode.h>
#include <TestFixtures.h>
#include <Utils/OpenEphysHttpServer.h>
#include <algorithm>
#include <atomic>
#include <thread>
#include <tuple>

namespace
{
class CapturingRecordEngine : public RecordEngine
{
public:
    explicit CapturingRecordEngine (bool batchWrites, int channels = 3)
        : engineId (batchWrites ? "BINARY" : "LEGACY"), samples (channels), timestamps (channels)
    {
    }

    String getEngineId() const override { return engineId; }
    void openFiles (File, int, int) override {}
    void closeFiles() override
    {
        ++closeCalls;
        closing = true;
        while (! allowClose.load())
            Thread::sleep (1);
        closed = true;
    }
    void writeEvent (int, const EventPacket&) override { ++eventsWritten; }
    void writeSpike (int, const Spike*) override { ++spikesWritten; }
    void writeTimestampSyncText (uint64, int64, float, String) override { ++eventsWritten; }

    void writeContinuousData (int channel, int, const float* data, const double* times, int count) override
    {
        while (! allowWrite.load())
            Thread::sleep (1);
        EXPECT_LE (count, BLOCK_DEFAULT_MAX_WRITE_SAMPLES);
        samples[channel].insert (samples[channel].end(), data, data + count);
        timestamps[channel].insert (timestamps[channel].end(), times, times + count);
    }

    void writeContinuousDataBatch (const int* channels,
                                   const int* realChannels,
                                   const float* const* data,
                                   const double* times,
                                   int numChannels,
                                   int count,
                                   int) override
    {
        ++batchWrites;
        for (int channel = 0; channel < numChannels; ++channel)
            writeContinuousData (channels[channel], realChannels[channel], data[channel], times, count);
    }

    const String engineId;
    std::vector<std::vector<float>> samples;
    std::vector<std::vector<double>> timestamps;
    int batchWrites = 0;
    int eventsWritten = 0;
    int spikesWritten = 0;
    std::atomic<bool> closed { false };
    std::atomic<bool> closing { false };
    std::atomic<bool> allowClose { true };
    std::atomic<bool> allowWrite { true };
    std::atomic<int> closeCalls { 0 };
};

class RecordThreadTests : public testing::Test
{
protected:
    void SetUp() override
    {
        tester = std::make_unique<ProcessorTester> (
            TestSourceNodeBuilder (FakeSourceNodeParams { 2, 30000.0f, 1.0f, 3 }),
            ProcessorTesterMode::FullApp,
            showControlPanel());
        node = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    }

    void fillQueue (DataQueue& queue, int channels, int count, int64 startSample)
    {
        AudioBuffer<float> data (channels, 1024);
        Array<int> sourceChannels;
        for (int channel = 0; channel < channels; ++channel)
            sourceChannels.add (channel);

        for (int written = 0; written < count;)
        {
            const int blockSamples = std::min (1024, count - written);
            for (int channel = 0; channel < channels; ++channel)
                for (int sample = 0; sample < blockSamples; ++sample)
                    data.setSample (channel, sample, float (startSample + written + sample + channel * 1000000));

            queue.writeSynchronizedTimestamps (double (startSample + written), 1.0, 0, blockSamples);
            queue.writeAllChannels (data, sourceChannels, blockSamples, startSample + written);
            written += blockSamples;
        }
    }

    void advanceQueue (DataQueue& queue, int channels)
    {
        std::vector<CircularBufferIndexes> dataIndexes (channels), timestampIndexes (1);
        for (int block = 0; block < DATA_BUFFER_NBLOCKS - 1; ++block)
        {
            fillQueue (queue, channels, 1024, block * 1024);
            int64 sampleNumber = 0;
            ASSERT_TRUE (queue.startRead (dataIndexes, timestampIndexes, sampleNumber, 0, 1024));
            queue.stopRead();
        }
    }

    std::unique_ptr<ProcessorTester> tester;
    RecordNode* node;

    virtual bool showControlPanel() const { return false; }

    bool waitUntil (std::function<bool()> predicate)
    {
        const auto deadline = Time::getMillisecondCounterHiRes() + 5000.0;
        while (! predicate() && Time::getMillisecondCounterHiRes() < deadline)
            MessageManager::getInstance()->runDispatchLoopUntil (10);
        return predicate();
    }

    void startWithEngine (CapturingRecordEngine& engine)
    {
        node->recordThread->setEngine (&engine);
        tester->startAcquisition (true, true);
        EXPECT_TRUE (node->getRecordingStatus());
        EXPECT_TRUE (CoreServices::getAcquisitionStatus());
        EXPECT_TRUE (CoreServices::getRecordingStatus());
    }

    void expectModeChangesBlocked()
    {
        for (const bool state : { false, true })
        {
            CoreServices::setAcquisitionStatus (state);
            CoreServices::setRecordingStatus (state);
            tester->controlPanel->setAcquisitionState (state);
            tester->controlPanel->setRecordingState (state, true);
        }
        auto* playButton = dynamic_cast<Button*> (tester->controlPanel->getPlayButton());
        auto* recordButton = dynamic_cast<Button*> (tester->controlPanel->getRecordButton());
        playButton->setToggleState (false, sendNotificationSync);
        recordButton->setToggleState (false, sendNotificationSync);
        EXPECT_TRUE (playButton->getToggleState());
        EXPECT_TRUE (recordButton->getToggleState());

        EXPECT_TRUE (CoreServices::getAcquisitionStatus());
        EXPECT_TRUE (CoreServices::getRecordingStatus());
        EXPECT_FALSE (tester->controlPanel->getPlayButton()->isEnabled());
        EXPECT_FALSE (tester->controlPanel->getRecordButton()->isEnabled());
    }

    void finishAndRestoreEngine (CapturingRecordEngine& engine)
    {
        engine.allowClose = true;
        EXPECT_TRUE (waitUntil ([this] { return ! tester->controlPanel->isFinalizingRecording(); }));
        tester->stopAcquisition();
        node->recordThread->setEngine (node->recordEngine.get());
    }

    int overflowBuffer (RecordNode* recordNode)
    {
        AudioBuffer<float> data (6, 1024);
        data.clear();
        for (int block = 0; block < DATA_BUFFER_NBLOCKS * 10; ++block)
        {
            tester->processBlock (recordNode, data);
            if (std::any_of (recordNode->fifoUsage.begin(), recordNode->fifoUsage.end(),
                             [] (const auto& stream) { return stream.second > 0.9f; }))
                return (block + 1) * data.getNumSamples();
        }
        ADD_FAILURE() << "The stalled recording writer did not reach the overflow threshold";
        return 0;
    }
};

class RecordThreadShutdownTests : public RecordThreadTests,
                                  public testing::WithParamInterface<std::tuple<int, bool, bool>>
{
};

TEST_P (RecordThreadShutdownTests, DrainsAllStreamsBeforeClosing)
{
    const auto [queuedSamples, wrapAround, batchWrites] = GetParam();
    CapturingRecordEngine engine (batchWrites);
    OwnedArray<DataQueue> queues;
    queues.add (new DataQueue (1024, DATA_BUFFER_NBLOCKS));
    queues.add (nullptr);
    queues.add (new DataQueue (1024, DATA_BUFFER_NBLOCKS));
    queues[0]->setChannelCount (2);
    queues[2]->setChannelCount (1);
    queues[0]->setTimestampStreamCount (1);
    queues[2]->setTimestampStreamCount (1);
    if (wrapAround)
    {
        advanceQueue (*queues[0], 2);
        advanceQueue (*queues[2], 1);
    }

    const int64 startSample = wrapAround ? (DATA_BUFFER_NBLOCKS - 1) * 1024 : 0;
    const int otherSamples = queuedSamples / 3;
    fillQueue (*queues[0], 2, queuedSamples, startSample);
    fillQueue (*queues[2], 1, otherSamples, startSample);
    EventMsgQueue events (16);
    SpikeMsgQueue spikes (16);
    RecordThread thread (node, &engine);
    thread.setChannelMap (Array<int> { 0, 1, 4 });
    thread.setTimestampChannelMap (Array<int> { 0, 0, 2 });
    thread.setQueuePointers (&queues, &events, &spikes);
    thread.setFirstBlockFlag (true);

    // Skip the normal loop so every queued sample must be written by shutdown.
    thread.signalThreadShouldExit();
    thread.run();

    EXPECT_TRUE (engine.closed);
    EXPECT_DOUBLE_EQ (thread.getDrainProgress(), 1.0);
    EXPECT_EQ (queues[0]->getNumSamplesReady(), 0);
    EXPECT_EQ (queues[2]->getNumSamplesReady(), 0);
    EXPECT_EQ (engine.batchWrites > 0, batchWrites && queuedSamples > 0);
    for (int channel = 0; channel < 3; ++channel)
    {
        const int expectedSamples = channel < 2 ? queuedSamples : otherSamples;
        ASSERT_EQ (engine.samples[channel].size(), expectedSamples);
        ASSERT_EQ (engine.timestamps[channel].size(), expectedSamples);
        for (int sample = 0; sample < expectedSamples; ++sample)
        {
            ASSERT_EQ (engine.samples[channel][sample], float (startSample + sample + (channel == 1 ? 1000000 : 0)));
            ASSERT_EQ (engine.timestamps[channel][sample], double (startSample + sample));
        }
    }
}

TEST_F (RecordThreadTests, DrainsEventsAndSpikesWithoutContinuousData)
{
    CapturingRecordEngine engine (false);
    OwnedArray<DataQueue> queues;
    for (int stream = 0; stream < 3; ++stream)
        queues.add (nullptr);
    EventMsgQueue events (BLOCK_MAX_WRITE_EVENTS + 2);
    SpikeMsgQueue spikes (BLOCK_MAX_WRITE_SPIKES + 2);
    HeapBlock<char> eventData;
    const auto eventSize = SystemEvent::fillTimestampSyncTextData (eventData, node, 0, 0, 0.0, true);
    const EventPacket event (eventData.getData(), int (eventSize));
    for (int i = 0; i < BLOCK_MAX_WRITE_EVENTS + 1; ++i)
        events.addEvent (event, i);

    SpikeChannel spikeChannel (SpikeChannel::Settings { SpikeChannel::SINGLE, "spike", "", "", Array<int> { 0 }, 0, 1 });
    Spike::Buffer spikeBuffer (&spikeChannel);
    spikeBuffer.set (0, 0.0f);
    const auto spike = Spike::createSpike (&spikeChannel, 0, Array<float> { 1.0f }, spikeBuffer);
    ASSERT_NE (spike, nullptr);
    for (int i = 0; i < BLOCK_MAX_WRITE_SPIKES + 1; ++i)
        spikes.addEvent (*spike, i);

    RecordThread thread (node, &engine);
    thread.setChannelMap (Array<int>());
    thread.setTimestampChannelMap (Array<int>());
    thread.setQueuePointers (&queues, &events, &spikes);
    thread.signalThreadShouldExit();
    thread.run();

    EXPECT_TRUE (engine.closed);
    EXPECT_EQ (events.getRemainingEvents(), 0);
    EXPECT_EQ (spikes.getRemainingEvents(), 0);
    EXPECT_EQ (engine.eventsWritten, BLOCK_MAX_WRITE_EVENTS + 1);
    EXPECT_EQ (engine.spikesWritten, BLOCK_MAX_WRITE_SPIKES + 1);
}

TEST_F (RecordThreadTests, HeadlessStopKeepsStatusActiveAndMessageThreadResponsive)
{
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);
    tester->controlPanel->stopAcquisition();

    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());
    EXPECT_TRUE (node->getRecordingStatus());
    EXPECT_DOUBLE_EQ (node->recordThread->getDrainProgress(), 0.95);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    expectModeChangesBlocked();
    bool messageDelivered = false;
    MessageManager::callAsync ([&messageDelivered] { messageDelivered = true; });
    EXPECT_TRUE (waitUntil ([&messageDelivered] { return messageDelivered; }));
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());

    std::thread modeRequest ([] {
        CoreServices::setAcquisitionStatus (true);
        CoreServices::setRecordingStatus (true);
    });
    modeRequest.join();

    finishAndRestoreEngine (engine);
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    EXPECT_FALSE (CoreServices::getAcquisitionStatus());
    EXPECT_TRUE (engine.closed);
}

TEST_F (RecordThreadTests, HttpStopReturnsActiveModeAndRejectsConflictingModes)
{
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);

    httplib::Response stopping;
    OpenEphysHttpServer::handleStatusModeChange ("IDLE", stopping);
    EXPECT_EQ (stopping.status, 200);
    auto status = json::parse (stopping.body);
    EXPECT_EQ (status["mode"], "RECORD");
    EXPECT_EQ (status["finalizing"], true);
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    for (const auto* mode : { "IDLE", "ACQUIRE", "RECORD" })
    {
        httplib::Response conflict;
        OpenEphysHttpServer::handleStatusModeChange (mode, conflict);
        EXPECT_EQ (conflict.status, 409);
        auto activeStatus = json::parse (conflict.body);
        EXPECT_EQ (activeStatus["mode"], "RECORD");
        EXPECT_EQ (activeStatus["finalizing"], true);
        EXPECT_TRUE (activeStatus.contains ("error"));
    }

    finishAndRestoreEngine (engine);
    httplib::Response stopped;
    OpenEphysHttpServer::handleStatusModeChange ("IDLE", stopped);
    EXPECT_EQ (stopped.status, 200);
    EXPECT_EQ (json::parse (stopped.body)["mode"], "IDLE");
    EXPECT_EQ (json::parse (stopped.body)["finalizing"], false);
}

TEST_F (RecordThreadTests, RecordOnlyStopKeepsAcquisitionActiveAndAllowsAnotherRecording)
{
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);
    tester->controlPanel->setRecordingState (false);
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    expectModeChangesBlocked();
    engine.allowClose = true;
    EXPECT_TRUE (waitUntil ([this] { return ! tester->controlPanel->isFinalizingRecording(); }));
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    EXPECT_TRUE (CoreServices::getAcquisitionStatus());
    EXPECT_FALSE (node->getRecordingStatus());

    CoreServices::setRecordingStatus (true);
    EXPECT_TRUE (waitUntil ([this] { return node->getRecordingStatus(); }));
    EXPECT_DOUBLE_EQ (node->recordThread->getDrainProgress(), 0.0);
    tester->stopAcquisition();
    node->recordThread->setEngine (node->recordEngine.get());
}

TEST_F (RecordThreadTests, KeepsModeActiveUntilEveryRecordNodeFinishes)
{
    auto* secondNode = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    CapturingRecordEngine firstEngine (false, 6), secondEngine (false, 6);
    secondEngine.allowClose = false;
    secondNode->recordThread->setEngine (&secondEngine);
    startWithEngine (firstEngine);
    tester->controlPanel->stopAcquisition();
    EXPECT_TRUE (waitUntil ([&] { return firstEngine.closed.load() && secondEngine.closing.load(); }));
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());
    EXPECT_TRUE (CoreServices::getRecordingStatus());
    EXPECT_TRUE (CoreServices::getAcquisitionStatus());

    secondEngine.allowClose = true;
    finishAndRestoreEngine (firstEngine);
    secondNode->recordThread->setEngine (secondNode->recordEngine.get());
    EXPECT_TRUE (secondEngine.closed);
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    EXPECT_FALSE (CoreServices::getAcquisitionStatus());
}

TEST_F (RecordThreadTests, OverflowStopsBufferingBeforeMessageDispatchAndAllowsRestart)
{
    CapturingRecordEngine engine (false, 6);
    engine.allowWrite = false;
    engine.allowClose = false;
    startWithEngine (engine);
    tester->audioComponent->endCallbacks();
    const int recordingNumber = node->getRecordingNumber();
    int queuedSamples = 0;
    std::thread producer ([&] { queuedSamples = overflowBuffer (node); });
    producer.join();

    const auto usageAtOverflow = node->fifoUsage;
    AudioBuffer<float> data (6, 1024);
    data.clear();
    for (int block = 0; block < 20; ++block)
        tester->processBlock (node, data);
    EXPECT_EQ (node->fifoUsage, usageAtOverflow);
    EXPECT_TRUE (node->getRecordingStatus());
    EXPECT_FALSE (tester->controlPanel->isFinalizingRecording());

    EXPECT_TRUE (waitUntil ([this] { return tester->controlPanel->isFinalizingRecording(); }));
    EXPECT_EQ (node->getRecordingNumber(), recordingNumber + 1);
    EXPECT_EQ (tester->controlPanel->findChildWithID ("recordingFinalizationOverlay"), nullptr);
    expectModeChangesBlocked();
    engine.allowWrite = true;
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    engine.allowClose = true;
    EXPECT_TRUE (waitUntil ([this] { return ! tester->controlPanel->isFinalizingRecording(); }));
    EXPECT_EQ (engine.closeCalls.load(), 1);
    for (const auto& samples : engine.samples)
        EXPECT_EQ (samples.size(), queuedSamples);
    EXPECT_TRUE (CoreServices::getAcquisitionStatus());
    EXPECT_FALSE (CoreServices::getRecordingStatus());

    const auto samplesBeforeRestart = engine.samples[0].size();
    CoreServices::setRecordingStatus (true);
    EXPECT_TRUE (waitUntil ([this] { return node->getRecordingStatus(); }));
    tester->processBlock (node, data);
    EXPECT_FALSE (tester->controlPanel->isFinalizingRecording());
    tester->stopAcquisition();
    EXPECT_GT (engine.samples[0].size(), samplesBeforeRestart);
    node->recordThread->setEngine (node->recordEngine.get());
}

class RecordThreadFinalizationOverlayTests : public RecordThreadTests
{
protected:
    void SetUp() override
    {
        RecordThreadTests::SetUp();
        tester->customLookAndFeel->setUsingNativeAlertWindows (false);
        tester->controlPanel->setSize (900, 32);
        tester->controlPanel->setVisible (true);
    }

    bool showControlPanel() const override { return true; }
};

TEST_F (RecordThreadFinalizationOverlayTests, ShowsProgressImmediatelyWithoutModalWindow)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    auto* status = dynamic_cast<Label*> (overlay->findChildWithID ("recordingFinalizationStatus"));
    auto* progressBar = dynamic_cast<ProgressBar*> (overlay->findChildWithID ("recordingFinalizationProgress"));
    ASSERT_NE (status, nullptr);
    ASSERT_NE (progressBar, nullptr);
    auto accessibility = progressBar->createAccessibilityHandler();
    auto* value = accessibility->getValueInterface();
    ASSERT_NE (value, nullptr);
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);
    tester->controlPanel->stopAcquisition();
    EXPECT_TRUE (overlay->isVisible());
    EXPECT_TRUE (status->getText().containsIgnoreCase ("finalizing"));
    EXPECT_FALSE (status->getText().containsIgnoreCase ("buffer full"));
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    expectModeChangesBlocked();

    EXPECT_EQ (tester->controlPanel->getComponentAt (tester->controlPanel->getLocalBounds().getCentre()), overlay);
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    EXPECT_TRUE (waitUntil ([value] { return value->getCurrentValue() == 0.95; }));
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);

    finishAndRestoreEngine (engine);
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_DOUBLE_EQ (value->getCurrentValue(), 1.0);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
}

TEST_F (RecordThreadFinalizationOverlayTests, FastFinalizationRemovesOverlayWithoutModalWindow)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    EXPECT_FALSE (overlay->isVisible());
    CapturingRecordEngine engine (false, 6);
    startWithEngine (engine);
    tester->controlPanel->stopAcquisition();
    EXPECT_TRUE (overlay->isVisible());
    EXPECT_TRUE (waitUntil ([this] {
        EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
        return ! tester->controlPanel->isFinalizingRecording();
    }));
    EXPECT_TRUE (engine.closed);
    EXPECT_FALSE (CoreServices::getAcquisitionStatus());
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    node->recordThread->setEngine (node->recordEngine.get());
}

TEST_F (RecordThreadFinalizationOverlayTests, HttpStopCanSuppressOverlay)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);
    httplib::Response response;
    OpenEphysHttpServer::handleStatusModeChange ("ACQUIRE", response);
    EXPECT_EQ (response.status, 200);
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    finishAndRestoreEngine (engine);
}

TEST_F (RecordThreadFinalizationOverlayTests, RecordOnlyStopKeepsAcquisitionActive)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    CapturingRecordEngine engine (false, 6);
    engine.allowClose = false;
    startWithEngine (engine);
    tester->controlPanel->setRecordingState (false);
    EXPECT_TRUE (overlay->isVisible());
    expectModeChangesBlocked();
    engine.allowClose = true;
    EXPECT_TRUE (waitUntil ([this] { return ! tester->controlPanel->isFinalizingRecording(); }));
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_TRUE (CoreServices::getAcquisitionStatus());
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    finishAndRestoreEngine (engine);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
}

TEST_F (RecordThreadFinalizationOverlayTests, MultipleOverflowsWarnImmediatelyWhileAllWritersDrain)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    auto* status = dynamic_cast<Label*> (overlay->findChildWithID ("recordingFinalizationStatus"));
    auto* progressBar = dynamic_cast<ProgressBar*> (overlay->findChildWithID ("recordingFinalizationProgress"));
    ASSERT_NE (status, nullptr);
    ASSERT_NE (progressBar, nullptr);
    auto accessibility = progressBar->createAccessibilityHandler();
    auto* value = accessibility->getValueInterface();
    ASSERT_NE (value, nullptr);
    auto* secondNode = tester->createProcessor<RecordNode> (Plugin::Processor::RECORD_NODE);
    CapturingRecordEngine firstEngine (false, 6), secondEngine (false, 6);
    firstEngine.allowWrite = false;
    secondEngine.allowWrite = false;
    secondEngine.allowClose = false;
    secondNode->recordThread->setEngine (&secondEngine);
    startWithEngine (firstEngine);
    tester->audioComponent->endCallbacks();
    const int firstQueuedSamples = overflowBuffer (node);
    const int secondQueuedSamples = overflowBuffer (secondNode);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);

    // Race an explicit acquisition stop against the pending overflow notification.
    tester->controlPanel->stopAcquisition();
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    EXPECT_TRUE (overlay->isVisible());
    EXPECT_TRUE (status->getText().containsIgnoreCase ("buffer full"));
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() > 0; }));
    auto* warningWindow = dynamic_cast<AlertWindow*> (Component::getCurrentlyModalComponent());
    EXPECT_NE (warningWindow, nullptr);
    if (warningWindow != nullptr)
        EXPECT_EQ (warningWindow->getName(), "Record Buffer Warning");
    EXPECT_FALSE (firstEngine.closed);
    EXPECT_FALSE (secondEngine.closed);
    firstEngine.allowWrite = true;
    secondEngine.allowWrite = true;
    EXPECT_TRUE (waitUntil ([&] { return firstEngine.closed.load() && secondEngine.closing.load(); }));
    EXPECT_TRUE (waitUntil ([value] { return value->getCurrentValue() == 0.975; }));
    EXPECT_TRUE (overlay->isVisible());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 1);
    expectModeChangesBlocked();

    secondEngine.allowClose = true;
    EXPECT_TRUE (waitUntil ([this] { return ! tester->controlPanel->isFinalizingRecording(); }));
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() > 0; }));
    EXPECT_TRUE (secondEngine.closed);
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_EQ (firstEngine.closeCalls.load(), 1);
    EXPECT_EQ (secondEngine.closeCalls.load(), 1);
    for (const auto& samples : firstEngine.samples)
        EXPECT_EQ (samples.size(), firstQueuedSamples);
    for (const auto& samples : secondEngine.samples)
        EXPECT_EQ (samples.size(), secondQueuedSamples);
    EXPECT_FALSE (CoreServices::getAcquisitionStatus());
    EXPECT_FALSE (CoreServices::getRecordingStatus());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 1);
    if (warningWindow != nullptr)
    {
        EXPECT_EQ (warningWindow->getName(), "Record Buffer Warning");
        warningWindow->exitModalState (0);
    }
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() == 0; }));
    MessageManager::getInstance()->runDispatchLoopUntil (100);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    node->recordThread->setEngine (node->recordEngine.get());
    secondNode->recordThread->setEngine (secondNode->recordEngine.get());
}

TEST_F (RecordThreadFinalizationOverlayTests, DismissingOverflowWarningDoesNotCancelFinalization)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    CapturingRecordEngine engine (false, 6);
    engine.allowWrite = false;
    engine.allowClose = false;
    startWithEngine (engine);
    tester->audioComponent->endCallbacks();
    overflowBuffer (node);
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() > 0; }));
    EXPECT_TRUE (overlay->isVisible());
    EXPECT_TRUE (tester->controlPanel->isFinalizingRecording());
    auto* warningWindow = dynamic_cast<AlertWindow*> (Component::getCurrentlyModalComponent());
    EXPECT_NE (warningWindow, nullptr);
    if (warningWindow != nullptr)
        warningWindow->exitModalState (0);
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() == 0; }));
    tester->controlPanel->requestRecordBufferOverflowStop();
    MessageManager::getInstance()->runDispatchLoopUntil (100);
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    EXPECT_TRUE (overlay->isVisible());
    expectModeChangesBlocked();
    engine.allowWrite = true;
    EXPECT_TRUE (waitUntil ([&engine] { return engine.closing.load(); }));
    finishAndRestoreEngine (engine);
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 0);
    EXPECT_EQ (engine.closeCalls.load(), 1);
}

TEST_F (RecordThreadFinalizationOverlayTests, FastOverflowWarningDoesNotStopNextRecording)
{
    auto* overlay = tester->controlPanel->findChildWithID ("recordingFinalizationOverlay");
    ASSERT_NE (overlay, nullptr);
    CapturingRecordEngine engine (false, 6);
    engine.allowWrite = false;
    startWithEngine (engine);
    tester->audioComponent->endCallbacks();
    overflowBuffer (node);
    engine.allowWrite = true;

    EXPECT_TRUE (waitUntil ([] { return ! CoreServices::getRecordingStatus(); }));
    EXPECT_TRUE (engine.closed);
    EXPECT_TRUE (CoreServices::getAcquisitionStatus());
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() > 0; }));
    auto* warningWindow = dynamic_cast<AlertWindow*> (Component::getCurrentlyModalComponent());
    EXPECT_NE (warningWindow, nullptr);
    if (warningWindow != nullptr)
        EXPECT_EQ (warningWindow->getName(), "Record Buffer Warning");

    CoreServices::setRecordingStatus (true);
    EXPECT_TRUE (waitUntil ([this] { return node->getRecordingStatus(); }));
    MessageManager::getInstance()->runDispatchLoopUntil (100);
    EXPECT_TRUE (CoreServices::getRecordingStatus());
    EXPECT_FALSE (tester->controlPanel->isFinalizingRecording());
    EXPECT_FALSE (overlay->isVisible());
    EXPECT_EQ (Component::getNumCurrentlyModalComponents(), 1);
    if (warningWindow != nullptr)
        warningWindow->exitModalState (0);
    EXPECT_TRUE (waitUntil ([] { return Component::getNumCurrentlyModalComponents() == 0; }));
    EXPECT_TRUE (CoreServices::getRecordingStatus());
    EXPECT_FALSE (tester->controlPanel->isFinalizingRecording());
    tester->stopAcquisition();
    EXPECT_EQ (engine.closeCalls.load(), 2);
    node->recordThread->setEngine (node->recordEngine.get());
}

INSTANTIATE_TEST_SUITE_P (
    QueuedData,
    RecordThreadShutdownTests,
    testing::Combine (testing::Values (0, 127, 4096, 4097, DATA_BUFFER_NBLOCKS * 1024 - 1),
                      testing::Bool(),
                      testing::Bool()));
}
