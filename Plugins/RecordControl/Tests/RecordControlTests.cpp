#include <stdio.h>

#include "gtest/gtest.h"

#include "../RecordControl.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class RecordControlTests : public ProcessorTest {
protected:
    RecordControlTests() : ProcessorTest() {
    }

    ~RecordControlTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


