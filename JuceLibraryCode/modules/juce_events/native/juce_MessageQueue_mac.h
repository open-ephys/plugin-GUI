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
/* An internal message pump class used in OSX and iOS. */
class MessageQueue
{
public:
    MessageQueue()
    {
       #if JUCE_IOS
        runLoop = CFRunLoopGetCurrent();
       #else
        runLoop = CFRunLoopGetMain();
       #endif

        CFRunLoopSourceContext sourceContext;
        zerostruct (sourceContext); // (can't use "= { 0 }" on this object because it's typedef'ed as a C struct)
        sourceContext.info = this;
        sourceContext.perform = runLoopSourceCallback;
        runLoopSource.reset (CFRunLoopSourceCreate (kCFAllocatorDefault, 1, &sourceContext));
        CFRunLoopAddSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
    }

    ~MessageQueue() noexcept
    {
        CFRunLoopRemoveSource (runLoop, runLoopSource.get(), kCFRunLoopCommonModes);
        CFRunLoopSourceInvalidate (runLoopSource.get());
    }

    void post (MessageManager::MessageBase* const message)
    {
        messages.add (message);
        wakeUp();
    }

private:
    ReferenceCountedArray<MessageManager::MessageBase, CriticalSection> messages;
    CFRunLoopRef runLoop;
    CFUniquePtr<CFRunLoopSourceRef> runLoopSource;

    void wakeUp() noexcept
    {
        CFRunLoopSourceSignal (runLoopSource.get());
        CFRunLoopWakeUp (runLoop);
    }

    bool deliverNextMessage()
    {
        const MessageManager::MessageBase::Ptr nextMessage (messages.removeAndReturn (0));

        if (nextMessage == nullptr)
            return false;

        JUCE_AUTORELEASEPOOL
        {
            JUCE_TRY
            {
                nextMessage->messageCallback();
            }
            JUCE_CATCH_EXCEPTION
        }

        return true;
    }

    void runLoopCallback() noexcept
    {
        for (int i = 4; --i >= 0;)
            if (! deliverNextMessage())
                return;

        wakeUp();
    }

    static void runLoopSourceCallback (void* info) noexcept
    {
        static_cast<MessageQueue*> (info)->runLoopCallback();
    }
};

} // namespace juce
