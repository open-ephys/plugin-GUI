#include <stdio.h>

#include "gtest/gtest.h"

#include "../LfpDisplayNode.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class LfpDisplayNodeTests : public ProcessorTest {
protected:
    LfpDisplayNodeTests() : ProcessorTest() {
    }

    ~LfpDisplayNodeTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


