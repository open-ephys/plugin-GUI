//
//  FilterNode_Tests.cpp
//  FilterNode_tests
//
//  Created by Allen Munk on 3/15/23.
//

#include <stdio.h>

#include "gtest/gtest.h"

#include "../FilterNode.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>
#include <cmath>

/**
 
 Class for testing the Filter Node
 
 */
class FilterNodeTests : public ProcessorTest {
protected:
    
    /** Constructor */
    FilterNodeTests() : ProcessorTest() {
        uut = std::make_unique<FilterNode>(true);
    }

    /** Destructor */
    ~FilterNodeTests() override {
    }

    /** Called before running tests*/
    void SetUp() override {
        ProcessorTest::SetUp();
        uut->setSourceNode(sn.get()); // sets source node
    }

    /** Called after running tests */
    void TearDown() override {
        ProcessorTest::TearDown();
    }
    
    /** Pointer to the Filter Node object */
    std::unique_ptr<FilterNode> uut;
    
    /** Buffer for the input signal */
    std::unique_ptr<AudioBuffer<float>> signal;
    
    /** Generates a sine wave at a specified frequency */
    void buildSineWave(float frequency, float amplitude, int numSamples)
    {
        
        int totalChannels = uut->getTotalNumInputChannels();
        
        signal = std::make_unique<AudioBuffer<float>>(totalChannels, numSamples);
        
        int channelOffset = 0;
        
        for (auto stream : uut->getDataStreams())
        {
            const float sampleRate = stream->getSampleRate();
            const int numChannels = stream->getChannelCount();
            LOGD("Sample rate: ", sampleRate, ", Num Channels: ", numChannels);

            const float angularFreq = 2.0 * M_PI * frequency;
            
            for (int ch = 0; ch < numChannels; ch++)
            {
                for (int i = 0; i < numSamples; i++)
                {
                    const float t = i / sampleRate;
                    const float value = std::sin(angularFreq * t);
                    signal -> setSample(ch + channelOffset, i, amplitude * value);
                }
            }
            
            channelOffset += numChannels;
        }
        
        
    }
    
    /** Sets the filter high cut*/
    void setHighCut(float value)
    {
        for (auto stream : uut->getDataStreams())
        {
            Parameter* highCut = stream->getParameter("high_cut");
            highCut -> currentValue = value;
            uut->parameterValueChanged(highCut);
        }
        
    }
    
    /** Sets the filter low cut */
    void setLowCut(float value)
    {
        for (auto stream : uut->getDataStreams())
        {
            Parameter* lowCut = stream->getParameter("low_cut");
            lowCut -> currentValue = value;
            uut->parameterValueChanged(lowCut);
        }
        
    }
    
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
    
    /** Checks that all samples are less than a given value */
    bool checkSamplesLessThan(float maxValue)
    {
        for (int i = 0; i < signal -> getNumChannels(); i++)
        {
            for (int j = 0; j < signal -> getNumSamples(); j++)
            {
                if (signal -> getSample(i, j) > maxValue)
                    return false;
            }
        }
        
        return true;
    }

};

TEST_F(FilterNodeTests, ContructorTest) {
    ASSERT_EQ(uut -> getDisplayName(), "Bandpass Filter");
}

TEST_F(FilterNodeTests, CutoffTest) {
    
    clearInputStreams();
    LOGD("Adding first input stream");
    addInputStream(10, 30000);
    LOGD("Adding second input stream");
    addInputStream(10, 30000);
    uut->update();
    
    const int bufferSize = 25;
    buildSineWave(150, 1.0, bufferSize); // fill one data buffer

    setLowCut(1);
    setHighCut(10);
    
    for (auto stream: uut->getDataStreams())
    {
        AccessClass::ExternalProcessorAccessor::injectNumSamples(uut.get(),
                    stream->getStreamId(),
                    bufferSize);
    }
    
    
    //dumpAllSamples();
    uut->process(*(signal.get()));
    //dumpAllSamples();
    // check that signal is filtered
    ASSERT_TRUE(checkSamplesLessThan(1e-3));
    
}
