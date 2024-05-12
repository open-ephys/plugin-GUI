#include "gtest/gtest.h"

#include <ProcessorHeaders.h>

/*
The Plugin Manager will be passed an absolute path to a DLL containing a Plugin library. 
The Plugin Manager should load the DLL without warning or errors. 
The Plugin Manager will should record library information from the DLL that will be verified.
*/
TEST(PluginManagerTest, PluginLoading)
{
    EXPECT_EQ(1, 1);
}

/*
The Plugin Manager will first load a Plugin DLL. 
A Plugin Description will be passed to the Plugin Manager, 
which will output an instance of the Processor by 
locating the associated Library Info object containing the Processor’s constructor.
*/
TEST(PluginManagerTest, PluginCreation)
{
    EXPECT_EQ(1, 1);
}