#include "gtest/gtest.h"

#include <JuceHeader.h>

class MockChangeListener : public ChangeListener
{
public:
    // Inherited via ChangeListener
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        ++numCalls;
    }

public:
    int numCalls = 0;
};

class ChangeBroadcasterTests : public testing::Test
{
protected:
    void SetUp() override
    {
        MessageManager::getInstance();
    }

    void TearDown() override
    {
        MessageManager::deleteInstance();
    }
};

TEST_F (ChangeBroadcasterTests, RemoveChangeListener)
{
    ChangeBroadcaster broadcaster;
    MockChangeListener listener;

    broadcaster.addChangeListener (&listener);
    broadcaster.removeChangeListener (&listener);

    broadcaster.sendSynchronousChangeMessage();

    EXPECT_EQ(listener.numCalls, 0);
}

TEST_F (ChangeBroadcasterTests, RemoveAllChangeListeners)
{
    ChangeBroadcaster broadcaster;
    MockChangeListener listener1;
    MockChangeListener listener2;

    broadcaster.addChangeListener (&listener1);
    broadcaster.addChangeListener (&listener2);
    broadcaster.removeAllChangeListeners();

    broadcaster.sendSynchronousChangeMessage();

    EXPECT_EQ(listener1.numCalls, 0);
    EXPECT_EQ(listener2.numCalls, 0);
}

TEST_F (ChangeBroadcasterTests, SendSynchronousChangeMessage)
{
    ChangeBroadcaster broadcaster;
    MockChangeListener listener;

    broadcaster.addChangeListener (&listener);
    broadcaster.sendSynchronousChangeMessage();

    EXPECT_EQ(listener.numCalls, 1);
}

TEST_F (ChangeBroadcasterTests, DispatchPendingMessages)
{
    ChangeBroadcaster broadcaster;
    MockChangeListener listener;

    broadcaster.addChangeListener (&listener);
    broadcaster.sendChangeMessage();
    broadcaster.dispatchPendingMessages();

    EXPECT_EQ(listener.numCalls, 1);
}