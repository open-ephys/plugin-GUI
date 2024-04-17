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

//==============================================================================
/**
    These enums are used in various classes to indicate whether a notification
    event should be sent out.
*/
enum NotificationType
{
    dontSendNotification = 0,   /**< No notification message should be sent. */
    sendNotification = 1,       /**< Requests a notification message, either synchronous or not. */
    sendNotificationSync,       /**< Requests a synchronous notification. */
    sendNotificationAsync,      /**< Requests an asynchronous notification. */
};

} // namespace juce
