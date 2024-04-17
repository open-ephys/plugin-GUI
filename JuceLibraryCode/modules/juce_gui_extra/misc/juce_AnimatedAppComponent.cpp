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

AnimatedAppComponent::AnimatedAppComponent()
{
    setOpaque (true);
}

void AnimatedAppComponent::setFramesPerSecond (int framesPerSecondIn)
{
    jassert (0 < framesPerSecond && framesPerSecond < 1000);
    framesPerSecond = framesPerSecondIn;
    updateSync();
}

void AnimatedAppComponent::updateSync()
{
    if (useVBlank)
    {
        stopTimer();

        if (vBlankAttachment.isEmpty())
            vBlankAttachment = { this, [this] { timerCallback(); } };
    }
    else
    {
        vBlankAttachment = {};

        const auto interval = 1000 / framesPerSecond;

        if (getTimerInterval() != interval)
            startTimer (interval);
    }
}

void AnimatedAppComponent::setSynchroniseToVBlank (bool syncToVBlank)
{
    useVBlank = syncToVBlank;
    updateSync();
}

int AnimatedAppComponent::getMillisecondsSinceLastUpdate() const noexcept
{
    return (int) (Time::getCurrentTime() - lastUpdateTime).inMilliseconds();
}

void AnimatedAppComponent::timerCallback()
{
    ++totalUpdates;
    update();
    repaint();
    lastUpdateTime = Time::getCurrentTime();
}

} // namespace juce
