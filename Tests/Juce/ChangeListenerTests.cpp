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

class ChangeListenerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        broadcaster.addChangeListener (&listener);
    }

    void TearDown() override
    {
        broadcaster.removeChangeListener (&listener);
    }

protected:
    ChangeBroadcaster broadcaster;
    MockChangeListener listener;
};

TEST_F (ChangeListenerTest, ChangeListenerCallback)
{
    broadcaster.sendSynchronousChangeMessage();
    
    EXPECT_EQ (listener.numCalls, 1);
}