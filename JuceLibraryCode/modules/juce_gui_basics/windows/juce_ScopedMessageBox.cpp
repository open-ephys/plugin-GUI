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
ScopedMessageBox::ScopedMessageBox() = default;

ScopedMessageBox::ScopedMessageBox (std::shared_ptr<detail::ScopedMessageBoxImpl> i)
    : impl (std::move (i)) {}

ScopedMessageBox::~ScopedMessageBox() noexcept
{
    close();
}

ScopedMessageBox::ScopedMessageBox (ScopedMessageBox&& other) noexcept
    : impl (std::exchange (other.impl, nullptr)) {}

ScopedMessageBox& ScopedMessageBox::operator= (ScopedMessageBox&& other) noexcept
{
    ScopedMessageBox temp (std::move (other));
    std::swap (temp.impl, impl);
    return *this;
}

void ScopedMessageBox::close()
{
    if (impl != nullptr)
        impl->close();

    impl.reset();
}

} // namespace juce
