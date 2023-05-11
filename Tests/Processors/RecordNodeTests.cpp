#include <stdio.h>

#include "gtest/gtest.h"

#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class RecordNodeTests : public ProcessorTest {
protected:
    RecordNodeTests() : ProcessorTest(1, 150) {
    }

    ~RecordNodeTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};

TEST_F(RecordNodeTests, ParameterTest) {
    GTEST_SKIP();
}

TEST_F(RecordNodeTests, DataIntegrityTest) {
    GTEST_SKIP();
}
