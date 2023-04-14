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

#define _USE_MATH_DEFINES
#include <math.h>


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
    const float sampleRate = 30000;
    LOGD("Adding first input stream");
    addInputStream(10, sampleRate);
    LOGD("Adding second input stream");
    addInputStream(10, sampleRate);
    uut->update();
    
    const int bufferSize = 25;
    int totalChans = uut->getTotalNumInputChannels();
    signal = std::make_unique<AudioBuffer<float>>(totalChans, bufferSize);
    Array<float> sineWave = generateSineWave(150, 1.0, bufferSize, sampleRate);
    
    setLowCut(1);
    setHighCut(10);
    
    int channelOffset = 0;
    for (auto stream: uut->getDataStreams())
    {
        // Add sine wave data to each channel in each buffer
        const int numChans = stream->getChannelCount();
        for(int i = 0; i < numChans; i++)
        {
            signal->addFrom(i + numChans, 0, sineWave.data(), bufferSize, 1.0f);
        }
        channelOffset += numChans;

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
