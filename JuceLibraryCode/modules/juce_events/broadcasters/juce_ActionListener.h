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
    Interface class for delivery of events that are sent by an ActionBroadcaster.

    @see ActionBroadcaster, ChangeListener

    @tags{Events}
*/
class JUCE_API  ActionListener
{
public:
    /** Destructor. */
    virtual ~ActionListener() = default;

    /** Overridden by your subclass to receive the callback.

        @param message  the string that was specified when the event was triggered
                        by a call to ActionBroadcaster::sendActionMessage()
    */
    virtual void actionListenerCallback (const String& message) = 0;
};

} // namespace juce
