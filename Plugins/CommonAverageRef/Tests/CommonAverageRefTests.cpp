#include <stdio.h>

#include "gtest/gtest.h"

#include "../CommonAverageRef.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class CommonAverageRefTests : public ProcessorTest {
protected:
    CommonAverageRefTests() : ProcessorTest() {
        uut = std::make_unique<CommonAverageRef>();
    }

    ~CommonAverageRefTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
        uut->setSourceNode(sn.get());
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

    std::unique_ptr<CommonAverageRef> uut;

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
         for (auto stream : uut->getDataStreams())
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

};

TEST_F(CommonAverageRefTests, ContructorTest) {
    ASSERT_EQ(uut -> getDisplayName(), "Common Avg Ref");
}

TEST_F(CommonAverageRefTests, CommonAverageTest) {

    clearInputStreams();

    const float sampleRate = 30000;
    const int bufferSize = 50;

    // create and add an input stream with 2 channels
    addInputStream(2, sampleRate);
    uut->update();

    // Set 1st channel as reference and second as affected in every stream
    for (auto stream : uut->getDataStreams())
    {
        auto referenceChans = (SelectedChannelsParameter *)stream->getParameter("Reference");
        referenceChans->currentValue = Array<var>({0});

        auto affectedChans = (SelectedChannelsParameter *)stream->getParameter("Affected");
        affectedChans->currentValue = Array<var>({1});
    }

    Array<float> posSine = generateSineWave(150.0, 1.0, bufferSize, sampleRate);
    Array<float> negSine = generateSineWave(150.0, -1.0, bufferSize, sampleRate);
    
    signal = std::make_unique<AudioBuffer<float>>(2, bufferSize);
    
    for (auto stream: uut->getDataStreams())
    {
        // Add positive sine wave data to first channel in each dataStream
        signal->copyFrom(0, 0, posSine.data(), bufferSize, 0.0f);

        // Add negative sine wave data to second channel in each dataStream
        signal->copyFrom(1, 0, negSine.data(), bufferSize, 0.0f);
    
        AccessClass::ExternalProcessorAccessor::injectNumSamples(uut.get(),
                    stream->getStreamId(),
                    bufferSize);
    }

    uut->process(*(signal.get()));

    // check that signal is common average referenced
    ASSERT_TRUE(checkSamplesZero(1));
}


