#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

class MidiBufferTests : public testing::Test
{
protected:
    void SetUp() override
    {

    }

    void TearDown() override
    {

    }

protected:
};

/*
The Midi Buffer encodes Metadata as raw byte data and provides an iterator for processing separate blocks of data. 
This test verifies that multiple Metadata objects inserted into the Midi Buffer are recoverable.
*/
TEST_F(MidiBufferTests, ReadWrite)
{
    

    EventPacket packet();
}