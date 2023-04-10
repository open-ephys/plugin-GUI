#include <stdio.h>

#include "gtest/gtest.h"

#include "../ArduinoOutput.h"
#include <ProcessorHeaders.h>
#include <ModelProcessors.h>
#include <ModelApplication.h>
#include <TestFixtures.h>

class ArduinoOutputTests : public ProcessorTest {
protected:
    ArduinoOutputTests() : ProcessorTest() {
    }

    ~ArduinoOutputTests() override {
    }

    void SetUp() override {
        ProcessorTest::SetUp();
    }

    void TearDown() override {
        ProcessorTest::TearDown();

    }

};


