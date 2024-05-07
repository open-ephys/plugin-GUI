#include <stdio.h>

#include "gtest/gtest.h"

#include "../PhaseDetector.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class PhaseDetectorTests : public ProcessorTest {
protected:
    PhaseDetectorTests() : ProcessorTest(1, 150) {
    }

    ~PhaseDetectorTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


