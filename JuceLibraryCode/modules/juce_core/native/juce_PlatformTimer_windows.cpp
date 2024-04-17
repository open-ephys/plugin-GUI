/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class PlatformTimer final
{
public:
    explicit PlatformTimer (PlatformTimerListener& ptl)
        : listener { ptl } {}

    void startTimer (int newIntervalMs)
    {
        jassert (newIntervalMs > 0);

        const auto callback = [] (UINT, UINT, DWORD_PTR context, DWORD_PTR, DWORD_PTR)
        {
            reinterpret_cast<PlatformTimerListener*> (context)->onTimerExpired();
        };

        timerId = timeSetEvent ((UINT) newIntervalMs, 1, callback, (DWORD_PTR) &listener, TIME_PERIODIC | TIME_CALLBACK_FUNCTION);
        intervalMs = timerId != 0 ? newIntervalMs : 0;
    }

    void cancelTimer()
    {
        jassert (timerId != 0);

        timeKillEvent (timerId);
        timerId = 0;
        intervalMs = 0;
    }

    int getIntervalMs() const
    {
        return intervalMs;
    }

private:
    PlatformTimerListener& listener;
    UINT timerId { 0 };
    int intervalMs { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTimer)
    JUCE_DECLARE_NON_MOVEABLE (PlatformTimer)
};

} // namespace juce
