/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>

#include "gtest/gtest.h"

#include "../BandpassFilter.h"
#include <ModelApplication.h>
#include <ModelProcessors.h>
#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class BandpassFilterTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        numChannels = 8;
        tester = std::make_unique<ProcessorTester> (TestSourceNodeBuilder (FakeSourceNodeParams {
            numChannels,
            sampleRate,
            1.0,
        }));
        processor = tester->createProcessor<BandpassFilter> (Plugin::Processor::FILTER);
        ASSERT_EQ (processor->getNumDataStreams(), 1);
        streamId = processor->getDataStreams()[0]->getStreamId();

        setLowCut (1.0);
        setHighCut (6000.0);
    }

    AudioBuffer<float> buildSineWave (float amplitude, float signal_frequency, int num_samples) const
    {
        AudioBuffer<float> signal (numChannels, num_samples);
        for (int i = 0; i < num_samples; i++)
        {
            for (int chidx = 0; chidx < numChannels; ++chidx)
            {
                signal.setSample (
                    chidx, i, amplitude * (float) sin (2 * M_PI * signal_frequency * ((float) i / sampleRate)));
            }
        }
        return signal;
    }

    void setLowCut (float value, bool assert_set = true)
    {
        Parameter* param = processor->getDataStream (streamId)->getParameter ("low_cut");
        param->setNextValue (value);
        if (assert_set)
        {
            ASSERT_EQ (getLowCut(), value);
        }
    }

    void setHighCut (float value, bool assert_set = true)
    {
        Parameter* param = processor->getDataStream (streamId)->getParameter ("high_cut");
        param->setNextValue (value);
        if (assert_set)
        {
            ASSERT_EQ (getHighCut(), value);
        }
    }

    float getLowCut()
    {
        return processor->getDataStream (streamId)->getParameter ("low_cut")->getValue();
    }

    float getHighCut()
    {
        return processor->getDataStream (streamId)->getParameter ("high_cut")->getValue();
    }

    double computeAmplitude (const float* data, int numSamples, double inputFrequency) const
    {
        // Perform correlation with sine and cosine waveforms.
        const double k = 2 * M_PI * inputFrequency / sampleRate; // precalculate for speed
        double meanI = 0.0;
        double meanQ = 0.0;
        for (int t = 0; t < numSamples; ++t)
        {
            meanI += data[t] * cos (k * t);
            meanQ += data[t] * -1.0 * sin (k * t);
        }
        meanI /= (double) numSamples;
        meanQ /= (double) numSamples;

        double realComponent = 2.0 * meanI;
        double imagComponent = 2.0 * meanQ;

        return sqrt (realComponent * realComponent + imagComponent * imagComponent);
    }

    BandpassFilter* processor;
    int numChannels;
    uint16 streamId;
    std::unique_ptr<ProcessorTester> tester;
    float sampleRate = 30000.0;
};

TEST_F (BandpassFilterTests, TestRespectsLowHighCutSetting)
{
    setLowCut (10.0, false);
    ASSERT_FLOAT_EQ (getLowCut(), 10.0f);

    setHighCut (6000.0, false);
    ASSERT_FLOAT_EQ (getLowCut(), 10.0f);
    ASSERT_FLOAT_EQ (getHighCut(), 6000.0f);

    // Trying to set a high_cut *under* low_cut is ignored:
    setHighCut (1.0, false);
    ASSERT_FLOAT_EQ (getLowCut(), 10.0f);
    ASSERT_FLOAT_EQ (getHighCut(), 6000.0f);

    // Same with an invalid low cut; it's ignored
    setHighCut (900.0f, false);
    setLowCut (950.0f, false);
    ASSERT_FLOAT_EQ (getLowCut(), 10.0f);
    ASSERT_FLOAT_EQ (getHighCut(), 900.0f);
}

TEST_F (BandpassFilterTests, Test_SignalInPassband)
{
    setLowCut (0.5);
    setHighCut (4000.0);

    int numSamples = (int) (sampleRate * 5);

    auto inputFrequency = 1000.0f;
    auto inputAmplitude = 10.0f;
    auto inputBuffer = buildSineWave (inputAmplitude, inputFrequency, numSamples);
    for (int chidx = 0; chidx < numChannels; ++chidx)
    {
        auto inputAmplitudeCalculated = computeAmplitude (
            inputBuffer.getReadPointer (chidx), numSamples, inputFrequency);
        // Sanity check our ComputeAmplitude function and our BuildSineWave function; it's within 0.1% of expected
        ASSERT_NEAR (inputAmplitudeCalculated, inputAmplitude, 0.1 * inputAmplitude / 100);
    }
    auto outputBuffer = tester->processBlock (processor, inputBuffer);

    for (int chidx = 0; chidx < numChannels; ++chidx)
    {
        auto outputAmplitude = computeAmplitude (
            outputBuffer.getReadPointer (chidx), numSamples, inputFrequency);
        auto ampDiff = (std::abs) (inputAmplitude - outputAmplitude);
        // The attenutation was < 1% of the input amplitude
        ASSERT_LE (ampDiff, inputAmplitude / 100.0);
    }
}

TEST_F (BandpassFilterTests, Test_SignalBelowPassband)
{
    setLowCut (100, true);
    setHighCut (4000.0, true);

    int numSamples = (int) (sampleRate * 5);

    auto inputFrequency = 5.0f;
    auto inputAmplitude = 10.0f;
    auto inputBuffer = buildSineWave (inputAmplitude, inputFrequency, numSamples);
    auto outputBuffer = tester->processBlock (processor, inputBuffer);

    for (int chidx = 0; chidx < numChannels; ++chidx)
    {
        auto outputAmplitude = computeAmplitude (
            outputBuffer.getReadPointer (chidx), numSamples, inputFrequency);
        auto ampDiff = (std::abs) (inputAmplitude - outputAmplitude);
        // The attenutation was severe, > 95% of the input amplitude
        ASSERT_GE (ampDiff, 0.95 * inputAmplitude);
    }
}

TEST_F (BandpassFilterTests, Test_SignalAbovePassband)
{
    setLowCut (100, true);
    setHighCut (1000.0, true);

    int numSamples = (int) (sampleRate * 5);

    auto inputFrequency = 5000.0f;
    auto inputAmplitude = 10.0f;
    auto inputBuffer = buildSineWave (inputAmplitude, inputFrequency, numSamples);
    auto outputBuffer = tester->processBlock (processor, inputBuffer);

    for (int chidx = 0; chidx < numChannels; ++chidx)
    {
        auto outputAmplitude = computeAmplitude (
            outputBuffer.getReadPointer (chidx), numSamples, inputFrequency);
        auto ampDiff = (std::abs) (inputAmplitude - outputAmplitude);
        // The attenutation was severe, > 95% of the input amplitude
        ASSERT_GE (ampDiff, 0.95 * inputAmplitude);
    }
}
