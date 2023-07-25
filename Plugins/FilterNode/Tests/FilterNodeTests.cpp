//
//  FilterNode_Tests.cpp
//  FilterNode_tests
//
//  Created by Allen Munk on 3/15/23.
//

#define _USE_MATH_DEFINES
#include <cmath>

#include <stdio.h>

#include "gtest/gtest.h"

#include "../FilterNode.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>


class FilterNodeTests :  public ::testing::Test {
protected:
    void SetUp() override {
        num_channels = 8;
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            num_channels,
            sample_rate_,
            1.0,
        }));
        processor = tester->Create<FilterNode>(Plugin::Processor::FILTER);
        ASSERT_EQ(processor->getNumDataStreams(), 1);
        stream_id = processor->getDataStreams()[0]->getStreamId();

        SetLowCut(1.0);
        SetHighCut(6000.0);
    }

    AudioBuffer<float> BuildSineWave(float amplitude, float signal_frequency, int num_samples) const {
        AudioBuffer<float> signal(num_channels, num_samples);
        for (int i = 0; i < num_samples; i++) {
            for (int chidx = 0; chidx < num_channels; ++chidx) {
                signal.setSample(
                    chidx, i, amplitude * (float) sin(2 * M_PI * signal_frequency * ((float) i / sample_rate_)));
            }
        }
        return signal;
    }

    void SetLowCut(float value, bool assert_set = true) {
        Parameter *param = processor->getDataStream(stream_id)->getParameter("low_cut");
        param->setNextValue(value);
        processor->parameterValueChanged(param);
        processor->updateSettings();
        if (assert_set) {
            ASSERT_EQ(GetLowCut(), value);
        }
    }

    void SetHighCut(float value, bool assert_set = true) {
        Parameter *param = processor->getDataStream(stream_id)->getParameter("high_cut");
        param->setNextValue(value);
        processor->parameterValueChanged(param);
        processor->updateSettings();
        if (assert_set) {
            ASSERT_EQ(GetHighCut(), value);
        }
    }

    float GetLowCut() {
        return processor->getDataStream(stream_id)->getParameter("low_cut")->getValue();
    }

    float GetHighCut() {
        return processor->getDataStream(stream_id)->getParameter("high_cut")->getValue();
    }

    double ComputeAmplitude(const float *data, int num_samples, double input_frequency) {
        // Perform correlation with sine and cosine waveforms.
        const double k = 2 * M_PI * input_frequency / sample_rate_;  // precalculate for speed
        double meanI = 0.0;
        double meanQ = 0.0;
        for (int t = 0; t < num_samples; ++t) {
            meanI += data[t] * cos(k * t);
            meanQ += data[t] * -1.0 * sin(k * t);
        }
        meanI /= (double) num_samples;
        meanQ /= (double) num_samples;

        double realComponent = 2.0 * meanI;
        double imagComponent = 2.0 * meanQ;

        return sqrt(realComponent * realComponent + imagComponent * imagComponent);
    }

    FilterNode *processor;
    int num_channels;
    uint16 stream_id;
    std::unique_ptr<ProcessorTester> tester;
    float sample_rate_ = 30000.0;
};

TEST_F(FilterNodeTests, TestRespectsLowHighCutSetting) {
    SetLowCut(10.0, false);
    ASSERT_FLOAT_EQ(GetLowCut(), 10.0f);

    SetHighCut(10000.0, false);
    ASSERT_FLOAT_EQ(GetLowCut(), 10.0f);
    ASSERT_FLOAT_EQ(GetHighCut(), 6000.0f);

    // Trying to set a high_cut *under* low_cut is ignored:
    SetHighCut(1.0, false);
    ASSERT_FLOAT_EQ(GetLowCut(), 10.0f);
    ASSERT_FLOAT_EQ(GetHighCut(), 6000.0f);

    // Same with an invalid low cut; it's ignored
    SetLowCut(20000.0f, false);
    ASSERT_FLOAT_EQ(GetLowCut(), 600.0f);
    ASSERT_FLOAT_EQ(GetHighCut(), 6000.0f);
}

TEST_F(FilterNodeTests, Test_SignalInPassband) {
    SetLowCut(0.5);
    SetHighCut(4000.0);

    int num_samples = (int) (sample_rate_ * 5);

    auto input_frequency = 1000.0f;
    auto input_amplitude = 10.0f;
    auto input_buffer = BuildSineWave(input_amplitude, input_frequency, num_samples);
    for (int chidx = 0; chidx < num_channels; ++chidx) {
        auto input_amplitude_calculated = ComputeAmplitude(
            input_buffer.getReadPointer(chidx), num_samples, input_frequency);
        // Sanity check our ComputeAmplitude function and our BuildSineWave function; it's within 0.1% of expected
        ASSERT_NEAR(input_amplitude_calculated, input_amplitude, 0.1 * input_amplitude/100);
    }
    auto output_buffer = tester->ProcessBlock(processor, input_buffer);

    for (int chidx = 0; chidx < num_channels; ++chidx) {
        auto output_amplitude = ComputeAmplitude(
            output_buffer.getReadPointer(chidx), num_samples, input_frequency);
        auto amp_diff = (std::abs)(input_amplitude - output_amplitude);
        // The attenutation was < 1% of the input amplitude
        ASSERT_LE(amp_diff, input_amplitude / 100.0);
    }
}

TEST_F(FilterNodeTests, Test_SignalBelowPassband) {
    SetLowCut(100, true);
    SetHighCut(4000.0, true);

    int num_samples = (int) (sample_rate_ * 5);

    auto input_frequency = 5.0f;
    auto input_amplitude = 10.0f;
    auto input_buffer = BuildSineWave(input_amplitude, input_frequency, num_samples);
    auto output_buffer = tester->ProcessBlock(processor, input_buffer);

    for (int chidx = 0; chidx < num_channels; ++chidx) {
        auto output_amplitude = ComputeAmplitude(
            output_buffer.getReadPointer(chidx), num_samples, input_frequency);
        auto amp_diff = (std::abs)(input_amplitude - output_amplitude);
        // The attenutation was severe, > 95% of the input amplitude
        ASSERT_GE(amp_diff, 0.95 * input_amplitude);
    }
}

TEST_F(FilterNodeTests, Test_SignalAbovePassband) {
    SetLowCut(100, true);
    SetHighCut(1000.0, true);

    int num_samples = (int) (sample_rate_ * 5);

    auto input_frequency = 5000.0f;
    auto input_amplitude = 10.0f;
    auto input_buffer = BuildSineWave(input_amplitude, input_frequency, num_samples);
    auto output_buffer = tester->ProcessBlock(processor, input_buffer);

    for (int chidx = 0; chidx < num_channels; ++chidx) {
        auto output_amplitude = ComputeAmplitude(
            output_buffer.getReadPointer(chidx), num_samples, input_frequency);
        auto amp_diff = (std::abs)(input_amplitude - output_amplitude);
        // The attenutation was severe, > 95% of the input amplitude
        ASSERT_GE(amp_diff, 0.95 * input_amplitude);
    }
}

