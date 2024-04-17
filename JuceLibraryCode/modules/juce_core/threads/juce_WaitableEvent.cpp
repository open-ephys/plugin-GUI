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

WaitableEvent::WaitableEvent (bool manualReset) noexcept
    : useManualReset (manualReset)
{
}

bool WaitableEvent::wait (double timeOutMilliseconds) const
{
    std::unique_lock<std::mutex> lock (mutex);

    if (! triggered)
    {
        if (timeOutMilliseconds < 0.0)
        {
            condition.wait (lock, [this] { return triggered == true; });
        }
        else
        {
            if (! condition.wait_for (lock, std::chrono::duration<double, std::milli> { timeOutMilliseconds },
                                      [this] { return triggered == true; }))
            {
                return false;
            }
        }
    }

    if (! useManualReset)
        reset();

    return true;
}

void WaitableEvent::signal() const
{
    std::lock_guard<std::mutex> lock (mutex);

    triggered = true;
    condition.notify_all();
}

void WaitableEvent::reset() const
{
    triggered = false;
}

} // namespace juce
