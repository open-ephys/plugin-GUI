#include <gtest/gtest.h>

#include <JuceHeader.h>
#include <thread>

class MockTimer : public Timer
{
public:
    // Inherited via Timer
    void timerCallback() override
    {
        ++callbackCalled;
    }

public:
    int callbackCalled = 0;
};

class TimerTests : public testing::Test
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

protected:
    MockTimer timer;
};

TEST_F (TimerTests, StartTimer)
{
    // Start the timer with an interval of 100 milliseconds
    timer.startTimer (100);

    // Assert that the timer was running
    ASSERT_TRUE (timer.isTimerRunning());

    // Stop the timer
    timer.stopTimer();

    // Assert that the timer was stopped
    ASSERT_FALSE (timer.isTimerRunning());
}

TEST_F (TimerTests, GetTimerInterval)
{
    // Start the timer with an interval of 100 milliseconds
    timer.startTimer (100);

    // Get the timer interval
    int interval = timer.getTimerInterval();

    // Stop the timer
    timer.stopTimer();

    // Assert that the timer interval is correct
    ASSERT_EQ (interval, 100);
}

TEST_F (TimerTests, IsTimerRunning)
{
    // Start the timer with an interval of 100 milliseconds
    timer.startTimer (100);

    // Check if the timer is running
    bool isRunning = timer.isTimerRunning();

    // Stop the timer
    timer.stopTimer();

    // Assert that the timer is running
    ASSERT_TRUE (isRunning);
}

TEST_F (TimerTests, StopTimer)
{
    // Start the timer with an interval of 100 milliseconds
    timer.startTimer (100);

    // Stop the timer
    timer.stopTimer();

    // Assert that the timer is not running
    ASSERT_FALSE (timer.isTimerRunning());
}