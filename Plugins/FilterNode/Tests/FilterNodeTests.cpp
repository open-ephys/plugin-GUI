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
class FilterNodeTests : public ProcessorTest {
protected:
    FilterNodeTests() : ProcessorTest(1, 150) {
        uut = std::make_unique<FilterNode>(true);
    }

    ~FilterNodeTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
        uut->setSourceNode(sn.get());
        uut->update();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }
    
    std::unique_ptr<FilterNode> uut;
    std::unique_ptr<AudioBuffer<float>> signal;
    
    void buildSineWave(int frequency){
        const DataStream* stream = uut -> getDataStreams().getFirst();
        signal = std::make_unique<AudioBuffer<float>>(stream->getContinuousChannels().size(), frequency);
        for(int i = 0; i < frequency; i++){
            signal -> addSample(0, i, 10*sin((2*3.1415*i)/frequency));
        }
    }
    
    void setHighCut(float value){
        Parameter* highCut = uut->getParameter("high_cut");
        highCut -> currentValue = value;
        uut->parameterChangeRequest(highCut);
    }
    
    void setLowCut(float value){
        uint16 streamId = uut->getDataStreams().getFirst()->getStreamId();
        Parameter* lowCut = uut->getParameter("low_cut");
        lowCut -> currentValue = value;
        uut->parameterChangeRequest(lowCut);
    }
    
    void dumpAllSamples() {
            for(int i = 0; i < signal -> getNumSamples(); i++) {
                for(int j = 0; j < signal -> getNumChannels(); j++) {
                    std::cout << signal -> getSample(j, i) << std::endl;
                }
            }
    }

};

TEST_F(FilterNodeTests, ParametersTest) {
    GTEST_SKIP();
}

TEST_F(FilterNodeTests, DataIntegrityTest) {
    GTEST_SKIP();
}
