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

/**
    Objects of this type can be used to programmatically close message boxes.

    @see NativeMessageBox::showScopedAsync(), AlertWindow::showScopedAsync()

    @tags{GUI}
*/
class ScopedMessageBox
{
public:
    /** @internal */
    explicit ScopedMessageBox (std::shared_ptr<detail::ScopedMessageBoxImpl>);

    /** Constructor */
    ScopedMessageBox();

    /** Destructor */
    ~ScopedMessageBox() noexcept;

    /** Move constructor */
    ScopedMessageBox (ScopedMessageBox&&) noexcept;

    /** Move assignment operator */
    ScopedMessageBox& operator= (ScopedMessageBox&&) noexcept;

    /** Closes the message box, if it is currently showing.

        This is also called automatically during ~ScopedMessageBox. This is useful if you want
        to display a message corresponding to a particular view, and hide the message automatically
        when the view is hidden. This situation commonly arises when displaying messages in plugin
        editors.
    */
    void close();

private:
    std::shared_ptr<detail::ScopedMessageBoxImpl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedMessageBox)
};

} // namespace juce
