#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "gtest/gtest.h"

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
    
    std::unique_ptr<FakeSourceNode> sn;
    std::unique_ptr<StubAccessClass> ac;

};


#endif
