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

namespace juce::detail
{

/* Keeps track of the active top level window. */
class TopLevelWindowManager  : private Timer,
                               private DeletedAtShutdown
{
public:
    TopLevelWindowManager() = default;

    ~TopLevelWindowManager() override
    {
        clearSingletonInstance();
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (TopLevelWindowManager)

    static void checkCurrentlyFocusedTopLevelWindow()
    {
        if (auto* wm = TopLevelWindowManager::getInstanceWithoutCreating())
            wm->checkFocusAsync();
    }

    void checkFocusAsync()
    {
        startTimer (10);
    }

    void checkFocus()
    {
        startTimer (jmin (1731, getTimerInterval() * 2));

        auto* newActive = findCurrentlyActiveWindow();

        if (newActive != currentActive)
        {
            currentActive = newActive;

            for (int i = windows.size(); --i >= 0;)
                if (auto* tlw = windows[i])
                    tlw->setWindowActive (isWindowActive (tlw));

            Desktop::getInstance().triggerFocusCallback();
        }
    }

    bool addWindow (TopLevelWindow* const w)
    {
        windows.add (w);
        checkFocusAsync();

        return isWindowActive (w);
    }

    void removeWindow (TopLevelWindow* const w)
    {
        checkFocusAsync();

        if (currentActive == w)
            currentActive = nullptr;

        windows.removeFirstMatchingValue (w);

        if (windows.isEmpty())
            deleteInstance();
    }

    Array<TopLevelWindow*> windows;

private:
    TopLevelWindow* currentActive = nullptr;

    void timerCallback() override
    {
        checkFocus();
    }

    bool isWindowActive (TopLevelWindow* const tlw) const
    {
        return (tlw == currentActive
                || tlw->isParentOf (currentActive)
                || tlw->hasKeyboardFocus (true))
               && tlw->isShowing();
    }

    TopLevelWindow* findCurrentlyActiveWindow() const
    {
        if (Process::isForegroundProcess())
        {
            auto* focusedComp = Component::getCurrentlyFocusedComponent();
            auto* w = dynamic_cast<TopLevelWindow*> (focusedComp);

            if (w == nullptr && focusedComp != nullptr)
                w = focusedComp->findParentComponentOfClass<TopLevelWindow>();

            if (w == nullptr)
                w = currentActive;

            if (w != nullptr && w->isShowing())
                return w;
        }

        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE (TopLevelWindowManager)
};

JUCE_IMPLEMENT_SINGLETON (TopLevelWindowManager)

} // namespace juce::detail
