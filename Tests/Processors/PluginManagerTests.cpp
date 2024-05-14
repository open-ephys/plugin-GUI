#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <Processors/PluginManager/PluginManager.h>

/*
The Plugin Manager will be passed an absolute path to a DLL containing a Plugin library. 
The Plugin Manager should load the DLL without warning or errors. 
The Plugin Manager should record library information from the DLL that will be verified.
*/
TEST(PluginManagerTest, PluginLoading)
{
    PluginManager pluginManager;

    pluginManager.loadPlugin("C:\\Users\\benja\\source\\repos\\iebl\\cpp\\build\\plugin-GUI\\Debug\\plugins\\ArduinoOutput.dll");

    EXPECT_EQ(pluginManager.getLibraryName(0), "Arduino Output");
    EXPECT_EQ(pluginManager.getLibraryVersion(0), "0.7.0");
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