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

struct LinuxEventLoopInternal
{
    /**
        @internal

        Receives notifications when a fd callback is added or removed.

        This is useful for VST3 plugins that host other VST3 plugins. In this scenario, the 'inner'
        plugin may want to register additional file descriptors on top of those registered by the
        'outer' plugin. In order for this to work, the outer plugin must in turn pass this request
        on to the host, to indicate that the outer plugin wants to receive FD callbacks for both the
        inner and outer plugin.
    */
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void fdCallbacksChanged() = 0;
    };

    /** @internal */
    static void registerLinuxEventLoopListener (Listener&);
    /** @internal */
    static void deregisterLinuxEventLoopListener (Listener&);
    /** @internal */
    static void invokeEventLoopCallbackForFd (int);
    /** @internal */
    static std::vector<int> getRegisteredFds();
};

} // namespace juce
