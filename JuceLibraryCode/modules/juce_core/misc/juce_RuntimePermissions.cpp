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

#if ! JUCE_ANDROID  // We currently don't request runtime permissions on any other platform
                    // than Android, so this file contains a dummy implementation for those.
                    // This may change in the future.

void RuntimePermissions::request (PermissionID, Callback callback)   { callback (true); }
bool RuntimePermissions::isRequired (PermissionID) { return false; }
bool RuntimePermissions::isGranted (PermissionID) { return true; }

#endif

} // namespace juce
