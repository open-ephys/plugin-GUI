#include <stdio.h>

#include "gtest/gtest.h"

#include "../SpikeDisplayNode/SpikeDisplayNode.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class SpikeDetectorNodeTests : public ProcessorTest {
protected:
    SpikeDetectorNodeTests() : ProcessorTest() {
    }

    ~SpikeDetectorNodeTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


