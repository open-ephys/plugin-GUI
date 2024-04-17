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

std::unique_ptr<ScopedMessageBoxInterface> ScopedMessageBoxInterface::create (const MessageBoxOptions& options)
{
    // On Linux, we re-use the AlertWindow rather than using a platform-specific dialog.
    // For consistency with the NativeMessageBox on other platforms, the result code must
    // match the button index, hence this adapter.
    class MessageBox final : public ScopedMessageBoxInterface
    {
    public:
        explicit MessageBox (const MessageBoxOptions& options)
            : inner (detail::AlertWindowHelpers::create (options)),
              numButtons (options.getNumButtons()) {}

        void runAsync (std::function<void (int)> fn) override
        {
            inner->runAsync ([fn, n = numButtons] (int result)
                             {
                                 fn (map (result, n));
                             });
        }

        int runSync() override
        {
            return map (inner->runSync(), numButtons);
        }

        void close() override
        {
            inner->close();
        }

    private:
        static int map (int button, int numButtons) { return (button + numButtons - 1) % numButtons; }

        std::unique_ptr<ScopedMessageBoxInterface> inner;
        int numButtons = 0;
    };

    return std::make_unique<MessageBox> (options);
}

} // namespace juce::detail
