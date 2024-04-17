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

static BOOL CALLBACK enumAlwaysOnTopWindows (HWND hwnd, LPARAM lParam)
{
    if (IsWindowVisible (hwnd))
    {
        DWORD processID = 0;
        GetWindowThreadProcessId (hwnd, &processID);

        if (processID == GetCurrentProcessId())
        {
            WINDOWINFO info{};

            if (GetWindowInfo (hwnd, &info)
                 && (info.dwExStyle & WS_EX_TOPMOST) != 0)
            {
                *reinterpret_cast<bool*> (lParam) = true;
                return FALSE;
            }
        }
    }

    return TRUE;
}

bool WindowUtils::areThereAnyAlwaysOnTopWindows()
{
    bool anyAlwaysOnTopFound = false;
    EnumWindows (&enumAlwaysOnTopWindows, (LPARAM) &anyAlwaysOnTopFound);
    return anyAlwaysOnTopFound;
}

} // namespace juce
