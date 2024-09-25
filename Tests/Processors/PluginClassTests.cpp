#include "gtest/gtest.h"

#include <ProcessorHeaders.h>
#include <TestFixtures.h>

TEST(PluginClassTests, constructor)
{
    PluginClass pluginClass;
    EXPECT_EQ(pluginClass.getLibName(), "");
    EXPECT_EQ(pluginClass.getPluginName(), "");
    EXPECT_EQ(pluginClass.getLibVersion(), "");
    EXPECT_EQ(pluginClass.getPluginType(), Plugin::INVALID);
}

TEST(PluginClassTests, setPluginData)
{
    PluginClass pluginClass;
    pluginClass.setPluginData(Plugin::DATA_THREAD, 0);
    EXPECT_EQ(pluginClass.getPluginType(), Plugin::DATA_THREAD);
}

TEST(PluginClassTests, getLibName)
{
    PluginClass pluginClass;
    pluginClass.setPluginData(Plugin::DATA_THREAD, 0);
    EXPECT_EQ(pluginClass.getLibName(), "DataThread");
}

TEST(PluginClassTests, getPluginName)
{
    PluginClass pluginClass;
    pluginClass.setPluginData(Plugin::DATA_THREAD, 0);
    EXPECT_EQ(pluginClass.getPluginName(), "DataThread");
}

TEST(PluginClassTests, getLibVersion)
{
    PluginClass pluginClass;
    pluginClass.setPluginData(Plugin::DATA_THREAD, 0);
    EXPECT_EQ(pluginClass.getLibVersion(), "1.0.0");
}

TEST(PluginClassTests, getIndex)
{
    PluginClass pluginClass;
    pluginClass.setPluginData(Plugin::DATA_THREAD, 0);
    EXPECT_EQ(pluginClass.getIndex(), 0);
}