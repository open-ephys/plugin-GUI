#include "gtest/gtest.h"

#include <Processors/Synchronizer/Synchronizer.h>
#include <array>

class SynchronizerTests : public testing::Test
{
protected:
    void SetUp() override
    {
        synchronizer.addDataStream ("main", sampleRate, 0, false);
        synchronizer.addDataStream (streamKey, sampleRate, 0, false);
        synchronizer.setMainDataStream ("main");
    }

    void tick()
    {
        // Exercise the real callback without starting a background timer or
        // depending on wall-clock timing.
        static_cast<HighResolutionTimer&> (synchronizer).hiResTimerCallback();
    }

    void addBarcode (uint32_t seconds, int64 startSample, bool valid = true)
    {
        // Four UART bytes: LOW start bit, eight LSB-first data bits, HIGH stop
        // bit. The timestamps used here leave the last data bit LOW, so the
        // last rising edge is at 39 ms (within the decoder's timing tolerance).
        std::array<bool, 40> bits {};
        for (int byte = 0; byte < 4; ++byte)
        {
            for (int bit = 0; bit < 8; ++bit)
                bits[byte * 10 + 1 + bit] = (seconds & (uint32_t (1) << (byte * 8 + bit))) != 0;
            bits[byte * 10 + 9] = true;
        }

        bool previousState = true;
        const int samplesPerBit = valid ? 30 : 60;
        for (int bit = 0; bit < int (bits.size()); ++bit)
        {
            if (bits[bit] != previousState)
            {
                synchronizer.addEvent (streamKey, 0, startSample + bit * samplesPerBit, bits[bit]);
                previousState = bits[bit];
            }
        }
    }

    void expectMapping (int64 sample, double timestamp)
    {
        ASSERT_TRUE (synchronizer.isHarpStream (streamKey));
        ASSERT_TRUE (synchronizer.isStreamSynced (streamKey));
        EXPECT_NEAR (synchronizer.convertSampleNumberToTimestamp (streamKey, sample), timestamp, 1.0e-9);
        EXPECT_NEAR (synchronizer.convertSampleNumberToTimestamp (streamKey, sample + sampleRate),
                     timestamp + 1.0, 1.0e-9);
    }

    static constexpr int sampleRate = 30000;
    const String streamKey = "harp";
    Synchronizer synchronizer;
};

TEST_F (SynchronizerTests, DecodesAllBarcodesQueuedBetweenCallbacks)
{
    // Each following barcode closes the preceding one.
    for (int i = 0; i < 4; ++i)
        addBarcode (1000 + i, i * sampleRate);

    tick();
    expectMapping (0, 1000.0);

    // Repeated callbacks must not reprocess already-consumed barcodes.
    for (int i = 0; i < 10; ++i)
        tick();
    expectMapping (4 * sampleRate, 1004.0);
}

TEST_F (SynchronizerTests, RequiresThreeValidBarcodes)
{
    for (int i = 0; i < 3; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
        EXPECT_FALSE (synchronizer.isStreamSynced (streamKey));
    }

    addBarcode (1003, 3 * sampleRate);
    tick();
    expectMapping (0, 1000.0);
}

TEST_F (SynchronizerTests, DetectsHarpAfterMoreThanFiveEmptyCallbacks)
{
    for (int i = 0; i < 10; ++i)
        tick();

    for (int i = 0; i < 4; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
    }
    expectMapping (0, 1000.0);
}

TEST_F (SynchronizerTests, UpdatesRateAfterIdleCallbacksRatherThanKeepingAStaleMapping)
{
    for (int i = 0; i < 4; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
    }
    expectMapping (0, 1000.0);

    for (int i = 0; i < 10; ++i)
        tick();

    // Slightly change the sample spacing. A stale mapping would now be wrong
    // by 1 ms at barcode 1004, even if the synchronized flag remained true.
    for (int i = 4; i < 7; ++i)
        addBarcode (1000 + i, 3 * sampleRate + (i - 3) * 30030);
    tick();
    ASSERT_TRUE (synchronizer.isStreamSynced (streamKey));
    EXPECT_NEAR (synchronizer.convertSampleNumberToTimestamp (streamKey, 120030), 1004.0, 1.0e-9);
}

TEST_F (SynchronizerTests, SynchronizesMainStreamToHarp)
{
    synchronizer.setMainDataStream (streamKey);
    for (int i = 0; i < 4; ++i)
        addBarcode (1000 + i, i * sampleRate);
    tick();
    expectMapping (0, 1000.0);
}

TEST_F (SynchronizerTests, RemainsSynchronizedAcrossBatchedAndEmptyCallbacks)
{
    for (int i = 0; i < 4; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
    }
    expectMapping (0, 1000.0);

    for (int i = 4; i < 100; i += 2)
    {
        addBarcode (1000 + i, i * sampleRate);
        addBarcode (1001 + i, (i + 1) * sampleRate);
        tick();
        for (int empty = 0; empty < 6; ++empty)
            tick();
        expectMapping (i * sampleRate, 1000.0 + i);
    }
}

TEST_F (SynchronizerTests, SkipsMalformedBarcodeWithoutLosingSynchronization)
{
    for (int i = 0; i < 4; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
    }
    expectMapping (0, 1000.0);

    addBarcode (1004, 4 * sampleRate, false);
    addBarcode (1005, 5 * sampleRate);
    tick();
    expectMapping (5 * sampleRate, 1005.0);

    addBarcode (1006, 6 * sampleRate);
    addBarcode (1007, 7 * sampleRate);
    tick();
    expectMapping (7 * sampleRate, 1007.0);
}

TEST_F (SynchronizerTests, AcceptsGapsThatMatchElapsedSampleTime)
{
    addBarcode (1000, 0);
    addBarcode (1002, 2 * sampleRate);
    addBarcode (1005, 5 * sampleRate);
    addBarcode (1006, 6 * sampleRate);
    tick();
    expectMapping (6 * sampleRate, 1006.0);
}

TEST_F (SynchronizerTests, PartialBarcodeDoesNotBecomeARateBaseline)
{
    synchronizer.addEvent (streamKey, 0, 0, false);
    synchronizer.addEvent (streamKey, 0, 30, true);

    for (int i = 1; i < 5; ++i)
        addBarcode (1000 + i, i * sampleRate);
    tick();
    expectMapping (sampleRate, 1001.0);
}

TEST_F (SynchronizerTests, RecoversAfterBackwardsHarpClock)
{
    for (int i = 0; i < 4; ++i)
    {
        addBarcode (1000 + i, i * sampleRate);
        tick();
    }
    expectMapping (0, 1000.0);

    addBarcode (900, 4 * sampleRate);
    addBarcode (901, 5 * sampleRate);
    tick();
    EXPECT_FALSE (synchronizer.isStreamSynced (streamKey));
    EXPECT_DOUBLE_EQ (synchronizer.convertSampleNumberToTimestamp (streamKey, 5 * sampleRate), -1.0);

    addBarcode (902, 6 * sampleRate);
    addBarcode (903, 7 * sampleRate);
    tick();
    expectMapping (4 * sampleRate, 900.0);
}

TEST_F (SynchronizerTests, RecoversAfterRepeatedHarpTimestamp)
{
    addBarcode (1000, 0);
    addBarcode (1000, sampleRate);
    addBarcode (1001, 2 * sampleRate);
    tick();
    EXPECT_FALSE (synchronizer.isStreamSynced (streamKey));

    addBarcode (1002, 3 * sampleRate);
    addBarcode (1003, 4 * sampleRate);
    tick();
    expectMapping (sampleRate, 1000.0);
}

TEST_F (SynchronizerTests, RejectsOutOfRangeRateAndRecovers)
{
    // Valid 1-ms bits, but start-to-start spacing implies a 40 kHz clock
    // rather than the declared 30 kHz clock.
    for (int i = 0; i < 4; ++i)
        addBarcode (1000 + i, i * 40000);
    tick();
    EXPECT_FALSE (synchronizer.isStreamSynced (streamKey));

    for (int i = 4; i < 8; ++i)
        addBarcode (1000 + i, 120000 + (i - 3) * sampleRate);
    tick();
    expectMapping (120000, 1003.0);
}

TEST_F (SynchronizerTests, EstimatesNonNominalRateFromOriginalBaseline)
{
    constexpr int actualRate = 30003;
    for (int i = 0; i < 40; ++i)
    {
        addBarcode (1000 + i, i * actualRate);
        if (i % 4 == 3)
            tick();
    }
    ASSERT_TRUE (synchronizer.isStreamSynced (streamKey));
    EXPECT_NEAR (synchronizer.convertSampleNumberToTimestamp (streamKey, 40 * actualRate), 1040.0, 1.0e-9);
}

TEST_F (SynchronizerTests, ResetDiscardsPendingAndValidatedHistory)
{
    for (int i = 0; i < 4; ++i)
        addBarcode (1000 + i, i * sampleRate);
    tick();
    expectMapping (0, 1000.0);
    addBarcode (1004, 4 * sampleRate);

    synchronizer.reset();
    tick();
    EXPECT_FALSE (synchronizer.isHarpStream (streamKey));
    EXPECT_FALSE (synchronizer.isStreamSynced (streamKey));

    for (int i = 0; i < 4; ++i)
        addBarcode (2000 + i, i * sampleRate);
    tick();
    expectMapping (0, 2000.0);
}
