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

class VBlankThread : private Thread,
                     private AsyncUpdater
{
public:
    using VBlankListener = ComponentPeer::VBlankListener;

    VBlankThread (ComSmartPtr<IDXGIOutput> out,
                  HMONITOR mon,
                  VBlankListener& listener)
        : Thread ("VBlankThread"),
          output (out),
          monitor (mon)
    {
        listeners.push_back (listener);
        startThread (Priority::highest);
    }

    ~VBlankThread() override
    {
        stopThread (-1);
        cancelPendingUpdate();
    }

    void updateMonitor()
    {
        monitor = getMonitorFromOutput (output);
    }

    HMONITOR getMonitor() const noexcept { return monitor; }

    void addListener (VBlankListener& listener)
    {
        listeners.push_back (listener);
    }

    bool removeListener (const VBlankListener& listener)
    {
        auto it = std::find_if (listeners.cbegin(),
                                listeners.cend(),
                                [&listener] (const auto& l) { return &(l.get()) == &listener; });

        if (it != listeners.cend())
        {
            listeners.erase (it);
            return true;
        }

        return false;
    }

    bool hasNoListeners() const noexcept
    {
        return listeners.empty();
    }

    bool hasListener (const VBlankListener& listener) const noexcept
    {
        return std::any_of (listeners.cbegin(),
                            listeners.cend(),
                            [&listener] (const auto& l) { return &(l.get()) == &listener; });
    }

    static HMONITOR getMonitorFromOutput (ComSmartPtr<IDXGIOutput> output)
    {
        DXGI_OUTPUT_DESC desc = {};
        return (FAILED (output->GetDesc (&desc)) || ! desc.AttachedToDesktop)
               ? nullptr
               : desc.Monitor;
    }

private:
    //==============================================================================
    void run() override
    {
        while (! threadShouldExit())
        {
            if (output->WaitForVBlank() == S_OK)
            {
                TRACE_LOG_JUCE_VBLANK_THREAD_EVENT;

                triggerAsyncUpdate();
            }
            else
            {
                Thread::sleep (1);
            }
        }
    }

    void handleAsyncUpdate() override
    {
        TRACE_LOG_JUCE_VBLANK_CALL_LISTENERS;

        for (auto& listener : listeners)
            listener.get().onVBlank();
    }

    //==============================================================================
    ComSmartPtr<IDXGIOutput> output;
    HMONITOR monitor = nullptr;
    std::vector<std::reference_wrapper<VBlankListener>> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankThread)
    JUCE_DECLARE_NON_MOVEABLE (VBlankThread)
};

//==============================================================================
class VBlankDispatcher : public DeletedAtShutdown
{
public:
    using VBlankListener = ComponentPeer::VBlankListener;

    void updateDisplay (VBlankListener& listener, HMONITOR monitor)
    {
        if (monitor == nullptr)
        {
            removeListener (listener);
            return;
        }

        auto threadWithListener = threads.end();
        auto threadWithMonitor  = threads.end();

        for (auto it = threads.begin(); it != threads.end(); ++it)
        {
            if ((*it)->hasListener (listener))
                threadWithListener = it;

            if ((*it)->getMonitor() == monitor)
                threadWithMonitor = it;

            if (threadWithListener != threads.end()
                && threadWithMonitor != threads.end())
            {
                if (threadWithListener == threadWithMonitor)
                    return;

                (*threadWithMonitor)->addListener (listener);

                // This may invalidate iterators, so be careful!
                removeListener (threadWithListener, listener);
                return;
            }
        }

        if (threadWithMonitor != threads.end())
        {
            (*threadWithMonitor)->addListener (listener);
            return;
        }

        if (threadWithListener != threads.end())
            removeListener (threadWithListener, listener);

        SharedResourcePointer<DirectX> directX;
        for (const auto& adapter : directX->dxgi.adapters.getAdapterArray())
        {
            UINT i = 0;
            ComSmartPtr<IDXGIOutput> output;

            while (adapter->dxgiAdapter->EnumOutputs (i, output.resetAndGetPointerAddress()) != DXGI_ERROR_NOT_FOUND)
            {
                if (VBlankThread::getMonitorFromOutput (output) == monitor)
                {
                    threads.emplace_back (std::make_unique<VBlankThread> (output, monitor, listener));
                    return;
                }

                ++i;
            }
        }
    }

    void removeListener (const VBlankListener& listener)
    {
        for (auto it = threads.begin(); it != threads.end(); ++it)
            if (removeListener (it, listener))
                return;
    }

    void reconfigureDisplays()
    {
        SharedResourcePointer<DirectX> directX;
        directX->dxgi.adapters.updateAdapters();

        for (auto& thread : threads)
            thread->updateMonitor();

        threads.erase (std::remove_if (threads.begin(),
                                       threads.end(),
                                       [] (const auto& thread) { return thread->getMonitor() == nullptr; }),
                       threads.end());
    }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED (VBlankDispatcher, false)

private:
    //==============================================================================
    using Threads = std::vector<std::unique_ptr<VBlankThread>>;

    VBlankDispatcher()
    {
        reconfigureDisplays();
    }

    ~VBlankDispatcher() override
    {
        threads.clear();
        clearSingletonInstance();
    }

    // This may delete the corresponding thread and invalidate iterators,
    // so be careful!
    bool removeListener (Threads::iterator it, const VBlankListener& listener)
    {
        if ((*it)->removeListener (listener))
        {
            if ((*it)->hasNoListeners())
                threads.erase (it);

            return true;
        }

        return false;
    }

    //==============================================================================
    Threads threads;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VBlankDispatcher)
    JUCE_DECLARE_NON_MOVEABLE (VBlankDispatcher)
};

JUCE_IMPLEMENT_SINGLETON (VBlankDispatcher)

} // namespace juce
