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
#include <math.h>
#include <stdio.h>

#include "gtest/gtest.h"

#include "../CommonAvgRef.h"
#include <ModelApplication.h>
#include <ModelProcessors.h>
#include <ProcessorHeaders.h>
#include <TestFixtures.h>

class CommonAverageRefTests : public testing::Test
{
protected:
    void SetUp() override
    {
        numChannels = 2;
        tester = std::make_unique<ProcessorTester> (TestSourceNodeBuilder (FakeSourceNodeParams {
            numChannels,
            sampleRate,
            1.0,
        }));
        processor = tester->createProcessor<CommonAverageRef> (Plugin::Processor::FILTER);
        ASSERT_EQ (processor->getNumDataStreams(), 1);
        streamId = processor->getDataStreams()[0]->getStreamId();
    }

    /** Buffer for the input signal */
    std::unique_ptr<AudioBuffer<float>> signal;

    /** Prints out all samples in the buffer */
    void dumpAllSamples()
    {
        for (int i = 0; i < signal->getNumChannels(); i++)
        {
            for (int j = 0; j < signal->getNumSamples(); j++)
            {
                std::cout << signal->getSample (i, j) << std::endl;
            }
        }
    }

    /** Checks that all samples in a channel are equal (almost equal) to a given value */
    bool checkSamplesEqual (int chan, float expected)
    {
        for (int j = 0; j < signal->getNumSamples(); j++)
        {
            float diff = fabs (signal->getSample (chan, j) - expected);
            if (std::isgreater (diff, 0.01f)) // Allow a small tolerance
                return false;
        }
        return true;
    }

    Array<float> generateSineWave (float frequency, float amplitude, int numSamples, float sampleRate)
    {
        Array<float> signal;
        signal.resize (numSamples);

        LOGD ("Sample rate: ", sampleRate, ", Num Samples: ", numSamples);

        const float angularFreq = 2.0 * M_PI * frequency;
        for (int i = 0; i < numSamples; i++)
        {
            const float t = i / sampleRate;
            const float value = std::sin (angularFreq * t);
            signal.set (i, amplitude * value);
        }

        return signal;
    }

    CommonAverageRef* processor;
    int numChannels;
    uint16 streamId;
    std::unique_ptr<ProcessorTester> tester;
    float sampleRate = 30000.0;
};

TEST_F (CommonAverageRefTests, ContructorTest)
{
    ASSERT_EQ (processor->getDisplayName(), "Common Avg Ref");
}

TEST_F (CommonAverageRefTests, SineWaveTest)
{
    const float sampleRate = 30000;
    const int bufferSize = 50;

    processor->update();

    // Set 1st channel as reference and second as affected in every stream
    for (auto stream : processor->getDataStreams())
    {
        auto referenceChans = (MaskChannelsParameter*) stream->getParameter ("reference");
        referenceChans->currentValue = Array<var> ({ 0 });

        auto affectedChans = (MaskChannelsParameter*) stream->getParameter ("affected");
        affectedChans->currentValue = Array<var> ({ 1 });
    }

    Array<float> sineData = generateSineWave (150.0, 1.0, bufferSize, sampleRate);

    signal = std::make_unique<AudioBuffer<float>> (2, bufferSize);

    for (auto stream : processor->getDataStreams())
    {
        // Add positive sine wave data to first two channel in each dataStream
        signal->copyFrom (0, 0, sineData.data(), bufferSize, 1.0f);
        signal->copyFrom (1, 0, sineData.data(), bufferSize, 1.0f);

        AccessClass::ExternalProcessorAccessor::injectNumSamples (processor,
                                                                  stream->getStreamId(),
                                                                  bufferSize);
    }

    processor->process (*(signal.get()));

    // check that signal is common average referenced
    ASSERT_TRUE (checkSamplesEqual (1, 0.0f));
}

TEST_F (CommonAverageRefTests, GainZeroLeavesAffectedUnchanged)
{
    const int bufferSize = 8;
    processor->update();

    for (auto stream : processor->getDataStreams())
    {
        auto referenceChans = (MaskChannelsParameter*) stream->getParameter ("reference");
        referenceChans->currentValue = Array<var> ({ 0 });
        auto affectedChans = (MaskChannelsParameter*) stream->getParameter ("affected");
        affectedChans->currentValue = Array<var> ({ 1 });
        auto gainParam = (FloatParameter*) stream->getParameter ("gain");
        gainParam->currentValue = 0.0f;
    }

    signal = std::make_unique<AudioBuffer<float>> (2, bufferSize);
    for (int i = 0; i < bufferSize; ++i)
    {
        signal->setSample (0, i, 1.0f); // reference
        signal->setSample (1, i, 2.0f); // affected
    }

    for (auto stream : processor->getDataStreams())
    {
        AccessClass::ExternalProcessorAccessor::injectNumSamples (processor, stream->getStreamId(), bufferSize);
    }

    processor->process (*(signal.get()));
    ASSERT_TRUE (checkSamplesEqual (1, 2.0f));
}

TEST_F (CommonAverageRefTests, GainHalfSubtractsHalfReference)
{
    const int bufferSize = 8;
    processor->update();

    for (auto stream : processor->getDataStreams())
    {
        auto referenceChans = (MaskChannelsParameter*) stream->getParameter ("reference");
        referenceChans->currentValue = Array<var> ({ 0 });
        auto affectedChans = (MaskChannelsParameter*) stream->getParameter ("affected");
        affectedChans->currentValue = Array<var> ({ 1 });
        auto gainParam = (FloatParameter*) stream->getParameter ("gain");
        gainParam->currentValue = 50.0f;
    }

    signal = std::make_unique<AudioBuffer<float>> (2, bufferSize);
    for (int i = 0; i < bufferSize; ++i)
    {
        signal->setSample (0, i, 1.0f); // reference
        signal->setSample (1, i, 2.0f); // affected
    }

    for (auto stream : processor->getDataStreams())
    {
        AccessClass::ExternalProcessorAccessor::injectNumSamples (processor, stream->getStreamId(), bufferSize);
    }

    processor->process (*(signal.get()));
    // 2.0 - 0.5*1.0 = 1.5
    ASSERT_TRUE (checkSamplesEqual (1, 1.5f));
}

TEST_F (CommonAverageRefTests, GainFullSubtractsFullReference)
{
    const int bufferSize = 8;
    processor->update();

    for (auto stream : processor->getDataStreams())
    {
        auto referenceChans = (MaskChannelsParameter*) stream->getParameter ("reference");
        referenceChans->currentValue = Array<var> ({ 0 });
        auto affectedChans = (MaskChannelsParameter*) stream->getParameter ("affected");
        affectedChans->currentValue = Array<var> ({ 1 });
        auto gainParam = (FloatParameter*) stream->getParameter ("gain");
        gainParam->currentValue = 100.0f;
    }

    signal = std::make_unique<AudioBuffer<float>> (2, bufferSize);
    for (int i = 0; i < bufferSize; ++i)
    {
        signal->setSample (0, i, 1.0f); // reference
        signal->setSample (1, i, 2.0f); // affected
    }

    for (auto stream : processor->getDataStreams())
    {
        AccessClass::ExternalProcessorAccessor::injectNumSamples (processor, stream->getStreamId(), bufferSize);
    }

    processor->process (*(signal.get()));
    // 2.0 - 1.0*1.0 = 1.0
    ASSERT_TRUE (checkSamplesEqual (1, 1.0f));
}
