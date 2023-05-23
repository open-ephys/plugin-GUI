#include "gtest/gtest.h"
#include "../../Source/UI/DataViewport.h"
#include "../../Source/UI/InfoLabel.h"

class DataViewportTests : public ::testing::Test {
protected:
    void SetUp() override {
        MessageManager::deleteInstance();
        MessageManager::getInstance();

        // Holds a lock for the test; necessary to prevent a bunch of assertion failures in debug
        lock = std::make_unique<MessageManagerLock>();
        viewport = std::make_unique<DataViewport>();
    }

    void TearDown() override {
        lock.reset();
    }

    std::unique_ptr<DataViewport> viewport;
    std::unique_ptr<MessageManagerLock> lock;
};

TEST_F(DataViewportTests, TestAddTabsAndSwitchTabs) {
    ASSERT_EQ(viewport->getNumTabs(), 0);
    ASSERT_EQ(viewport->getTabNames().size(), 0);

    auto info_label = std::make_unique<InfoLabel>();
    viewport->addTabToDataViewport("Info", info_label.get());
    ASSERT_EQ(viewport->getNumTabs(), 1);
    ASSERT_EQ(viewport->getTabNames().size(), 1);
    ASSERT_EQ(viewport->getTabNames()[0], juce::String("Info"));
    ASSERT_EQ(viewport->getCurrentTabIndex(), 0);

    auto info_label2 = std::make_unique<InfoLabel>();
    viewport->addTabToDataViewport("Info2", info_label.get());
    ASSERT_EQ(viewport->getNumTabs(), 2);
    ASSERT_EQ(viewport->getTabNames().size(), 2);
    ASSERT_EQ(viewport->getTabNames()[0], juce::String("Info"));
    ASSERT_EQ(viewport->getTabNames()[1], juce::String("Info2"));

    ASSERT_EQ(viewport->getCurrentTabIndex(), 1);
    viewport->setCurrentTabIndex(0);
    ASSERT_EQ(viewport->getCurrentTabIndex(), 0);
    viewport->setCurrentTabIndex(1);
    ASSERT_EQ(viewport->getCurrentTabIndex(), 1);
}
