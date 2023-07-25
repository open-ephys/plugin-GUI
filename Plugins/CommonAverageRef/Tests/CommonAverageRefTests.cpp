#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include "gtest/gtest.h"

#include "../CommonAverageRef.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>
class CommonAverageRefTests : public ::testing::Test  {
protected:
    void SetUp() override {
        num_channels = 2;
        tester = std::make_unique<ProcessorTester>(TestSourceNodeBuilder
                                                   (FakeSourceNodeParams{
            num_channels,
            sample_rate_,
            1.0,
        }));
        processor = tester->Create<CommonAverageRef>(Plugin::Processor::FILTER);
        ASSERT_EQ(processor->getNumDataStreams(), 1);
        stream_id = processor->getDataStreams()[0]->getStreamId();
    }

    void TearDown() override {

    }

     /** Buffer for the input signal */
    std::unique_ptr<AudioBuffer<float>> signal;

    /** Prints out all samples in the buffer */
    void dumpAllSamples()
    {
        for (int i = 0; i < signal -> getNumChannels(); i++)
        {
            for (int j = 0; j < signal -> getNumSamples(); j++)
            {
                std::cout << signal -> getSample(i, j) << std::endl;
            }
        }
    }

    /** Checks that all samples in a channel are equal (almost equal) to zero */
     bool checkSamplesZero(int chan)
     {
         for (auto stream : processor->getDataStreams())
         {
             for (int j = 0; j < signal -> getNumSamples(); j++)
             {
                 float diff = fabs(signal->getSample(chan, j) - 0.0f);
                 if (std::isgreater(diff, 1e-5))
                     return false;
             }
         }
        
         return true;
     }
    
    Array<float> generateSineWave(float frequency, float amplitude, int numSamples, float sampleRate)
       {
           
           Array<float> signal;
           signal.resize(numSamples);

           LOGD("Sample rate: ", sampleRate, ", Num Samples: ", numSamples);

           const float angularFreq = 2.0 * M_PI * frequency;
           for (int i = 0; i < numSamples; i++)
           {
               const float t = i / sampleRate;
               const float value = std::sin(angularFreq * t);
               signal.set(i, amplitude * value);
           }

           return signal;
       }
    
    CommonAverageRef *processor;
    int num_channels;
    uint16 stream_id;
    std::unique_ptr<ProcessorTester> tester;
    float sample_rate_ = 30000.0;
};

TEST_F(CommonAverageRefTests, ContructorTest) {
    ASSERT_EQ(processor -> getDisplayName(), "Common Avg Ref");
}

TEST_F(CommonAverageRefTests, CommonAverageTest) {

    const float sampleRate = 30000;
    const int bufferSize = 50;

    processor->update();

    // Set 1st channel as reference and second as affected in every stream
    for (auto stream : processor->getDataStreams())
    {
        auto referenceChans = (SelectedChannelsParameter *)stream->getParameter("Reference");
        referenceChans->currentValue = Array<var>({0});

        auto affectedChans = (SelectedChannelsParameter *)stream->getParameter("Affected");
        affectedChans->currentValue = Array<var>({1});
    }

    Array<float> posSine = generateSineWave(150.0, 1.0, bufferSize, sampleRate);
    Array<float> negSine = generateSineWave(150.0, -1.0, bufferSize, sampleRate);
    
    signal = std::make_unique<AudioBuffer<float>>(2, bufferSize);
    
    for (auto stream: processor->getDataStreams())
    {
        // Add positive sine wave data to first channel in each dataStream
        signal->copyFrom(0, 0, posSine.data(), bufferSize, 0.0f);

        // Add negative sine wave data to second channel in each dataStream
        signal->copyFrom(1, 0, negSine.data(), bufferSize, 0.0f);
    
        AccessClass::ExternalProcessorAccessor::injectNumSamples(processor,
                    stream->getStreamId(),
                    bufferSize);
    }

    processor->process(*(signal.get()));

    // check that signal is common average referenced
    ASSERT_TRUE(checkSamplesZero(1));
}


