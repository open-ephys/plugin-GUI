#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "gtest/gtest.h"

#define _USE_MATH_DEFINES
#include <math.h>

class ProcessorTest : public ::testing::Test {
protected:
    ProcessorTest() {
        sn = std::make_unique<FakeSourceNode>();
        ac = std::make_unique<StubAccessClass>();
    }

    ~ProcessorTest() override {
    }

    void SetUp() override {

        ac->addMessageCenter();
    }

    void TearDown() override
    {

    }
    
    void clearInputStreams()
    {
        sn->clearStreams();
        sn->update();
    }
    
    void addInputStream(int numChannels, float sampleRate)
    {
        sn->addTestDataStream(numChannels, sampleRate);
        sn->update();
    }

    /** Generates a sine wave at a specified frequency & amplitude */
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
    
    std::unique_ptr<FakeSourceNode> sn;
    std::unique_ptr<StubAccessClass> ac;

};


#endif
