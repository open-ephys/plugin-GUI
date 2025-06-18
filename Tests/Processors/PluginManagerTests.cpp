#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <Processors/PluginManager/PluginManager.h>
class PluginManagerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        File arduinoOutputDir = File::getSpecialLocation (File::currentExecutableFile).getParentDirectory();

#ifdef JUCE_LINUX
        arduinoOutputDir = arduinoOutputDir.getChildFile ("../ArduinoOutput");
        files =
            arduinoOutputDir.findChildFiles (File::findFiles, false, "ArduinoOutput.*", File::FollowSymlinks::no);
#else
        arduinoOutputDir = arduinoOutputDir.getChildFile ("../../ArduinoOutput");
        files =
            arduinoOutputDir.findChildFiles (File::findFiles, true, "ArduinoOutput.*", File::FollowSymlinks::no);
#endif

        ASSERT_GE (files.size(), 1) << "Arduino Output plugin not found in " << arduinoOutputDir.getFullPathName();

        String path = files[0].getFullPathName();

        pluginManager.loadPlugin (path);
    }

    PluginManager pluginManager;

private:
    Array<File> files;
};

/*
The Plugin Manager should load the DLL without warning or errors. 
The Plugin Manager should record library information from the DLL that will be verified.
*/
TEST_F (PluginManagerTest, PluginLoading)
{
    EXPECT_EQ (pluginManager.getNumProcessors(), 1);
    EXPECT_EQ (pluginManager.getLibraryName (0), "Arduino Output");
}

/*
Find the processor information from the Plugin Manager and verify the processor can be created.
*/
TEST_F (PluginManagerTest, PluginCreation)
{
    Plugin::ProcessorInfo processorInfo = pluginManager.getProcessorInfo (0);

    EXPECT_EQ (String (processorInfo.name), "Arduino Output");
    EXPECT_EQ (processorInfo.type, Plugin::Processor::SINK);
    EXPECT_NE (processorInfo.creator, nullptr);
}

TEST_F (PluginManagerTest, getLibraryIndexFromPlugin)
{
    Plugin::Type type = Plugin::Type::PROCESSOR;

    EXPECT_EQ (pluginManager.getLibraryIndexFromPlugin (type, 0), 0);
}

TEST_F (PluginManagerTest, removePlugin)
{
    auto libName = pluginManager.getLibraryName (0);

    ASSERT_EQ (libName, "Arduino Output");

    pluginManager.removePlugin (libName);

    EXPECT_EQ (pluginManager.getNumProcessors(), 0);
}