#ifndef TESTFIXTURES_H
#define TESTFIXTURES_H

#include "gtest/gtest.h"

class ProcessorTest : public ::testing::Test {
protected:
    ProcessorTest(int channels, int sampleRate) {
        sn = std::make_unique<FakeSourceNode>(channels, sampleRate);
        ac = std::make_unique<StubAccessClass>();
    }

    ~ProcessorTest() override {
    }

    void SetUp() override {

        ac->addMessageCenter();
        sn->addTestDataStreams();
        sn->update();

    }

    void TearDown() override {

    }
    std::unique_ptr<FakeSourceNode> sn;
    std::unique_ptr<StubAccessClass> ac;

};


#endif
