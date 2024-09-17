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

    String path = 
        File::getCurrentWorkingDirectory().
        getChildFile("../Resources/ArduinoOutput.dll").
        getFullPathName();

    pluginManager.loadPlugin(path);

    EXPECT_EQ(pluginManager.getLibraryName(0), "Arduino Output");
}

/*
The Plugin Manager will first load a Plugin DLL. 
A Plugin Description will be passed to the Plugin Manager, 
which will output an instance of the Processor by 
locating the associated Library Info object containing the Processor's constructor.
*/
TEST(PluginManagerTest, PluginCreation)
{
    PluginManager pluginManager;

    String path =
        File::getCurrentWorkingDirectory().
        getChildFile("../Resources/ArduinoOutput.dll").
        getFullPathName();

    int idx = pluginManager.loadPlugin(path) - 1;
    Plugin::ProcessorInfo processorInfo = pluginManager.getProcessorInfo(idx);

    EXPECT_EQ(String(processorInfo.name), "Arduino Output");
    EXPECT_EQ(processorInfo.type, Plugin::Processor::SINK);
}

TEST(PluginManagerTest, getLibraryIndexFromPlugin)
{
    PluginManager pluginManager;

    String path =
        File::getCurrentWorkingDirectory().
        getChildFile("../Resources/ArduinoOutput.dll").
        getFullPathName();

    int idx = pluginManager.loadPlugin(path) - 1;
    Plugin::Type type = Plugin::Type::PROCESSOR;

    EXPECT_EQ(pluginManager.getLibraryIndexFromPlugin(type, idx), 0);
}

TEST(PluginManagerTest, removePlugin)
{
    PluginManager pluginManager;

    String path =
        File::getCurrentWorkingDirectory().
        getChildFile("../Resources/ArduinoOutput.dll").
        getFullPathName();

    int idx = pluginManager.loadPlugin(path) - 1;
    Plugin::ProcessorInfo processorInfo = pluginManager.getProcessorInfo(idx);

    pluginManager.removePlugin(processorInfo.name);

    EXPECT_EQ(pluginManager.getNumProcessors(), 0);
}