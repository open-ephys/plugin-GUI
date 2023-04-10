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
    }

    ~CommonAverageRefTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


