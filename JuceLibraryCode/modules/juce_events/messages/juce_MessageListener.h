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
    MessageListener subclasses can post and receive Message objects.

    @see Message, MessageManager, ActionListener, ChangeListener

    @tags{Events}
*/
class JUCE_API  MessageListener
{
public:
    //==============================================================================
    MessageListener() noexcept;

    /** Destructor. */
    virtual ~MessageListener();

    //==============================================================================
    /** This is the callback method that receives incoming messages.

        This is called by the MessageManager from its dispatch loop.

        @see postMessage
    */
    virtual void handleMessage (const Message& message) = 0;

    //==============================================================================
    /** Sends a message to the message queue, for asynchronous delivery to this listener
        later on.

        This method can be called safely by any thread.

        @param message      the message object to send - this will be deleted
                            automatically by the message queue, so make sure it's
                            allocated on the heap, not the stack!
        @see handleMessage
    */
    void postMessage (Message* message) const;

private:
    WeakReference<MessageListener>::Master masterReference;
    friend class WeakReference<MessageListener>;
};

} // namespace juce
