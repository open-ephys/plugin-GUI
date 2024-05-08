#include "gtest/gtest.h"

#include <JuceHeader.h>

/*
Continuous Data and Metadata are pushed to the Data Buffer. 
This data can then be copied to an Audio Buffer. 
The Data Buffer will attempt to copy the maximum number of samples, 
dependent on the number of samples within the Data Buffer and the size of the Audio Buffer. 
This test verifies that the Data Buffer can successfully perform this copy.
*/
TEST(DataBufferTest, ReadWrite)
{
    
}