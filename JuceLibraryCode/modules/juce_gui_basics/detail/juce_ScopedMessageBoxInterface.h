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

/*
    Instances of this type can show and dismiss a message box.

    This is an interface rather than a concrete type so that platforms can pick an implementation at
    runtime if necessary.
*/
struct ScopedMessageBoxInterface
{
    virtual ~ScopedMessageBoxInterface() = default;

    /*  Shows the message box.

        When the message box exits normally, it should send the result to the passed-in function.
        The passed-in function is safe to call from any thread at any time.
    */
    virtual void runAsync (std::function<void (int)>) = 0;

    /*  Shows the message box and blocks. */
    virtual int runSync() = 0;

    /*  Forcefully closes the message box.

        This will be called when the message box handle has fallen out of scope.
        If the message box has already been closed by the user, this shouldn't do anything.
    */
    virtual void close() = 0;

    /*  Implemented differently for each platform. */
    static std::unique_ptr<ScopedMessageBoxInterface> create (const MessageBoxOptions& options);
};

} // namespace juce::detail
