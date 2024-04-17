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

#if JUCE_WEB_BROWSER || DOXYGEN

bool WebBrowserComponent::pageAboutToLoad ([[maybe_unused]] const String& newURL)             { return true; }
void WebBrowserComponent::pageFinishedLoading ([[maybe_unused]] const String& url)            {}
bool WebBrowserComponent::pageLoadHadNetworkError ([[maybe_unused]] const String& errorInfo)  { return true; }
void WebBrowserComponent::windowCloseRequest()                                                {}
void WebBrowserComponent::newWindowAttemptingToLoad ([[maybe_unused]] const String& newURL)   {}

#endif

} // namespace juce
