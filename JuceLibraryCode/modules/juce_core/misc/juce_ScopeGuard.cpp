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

ErasedScopeGuard::ErasedScopeGuard (std::function<void()> d)
    : detach (std::move (d)) {}

ErasedScopeGuard::ErasedScopeGuard (ErasedScopeGuard&& other) noexcept
    : detach (std::exchange (other.detach, nullptr)) {}

ErasedScopeGuard& ErasedScopeGuard::operator= (ErasedScopeGuard&& other) noexcept
{
    ErasedScopeGuard token { std::move (other) };
    std::swap (token.detach, detach);
    return *this;
}

ErasedScopeGuard::~ErasedScopeGuard() noexcept
{
    reset();
}

void ErasedScopeGuard::reset()
{
    if (auto d = std::exchange (detach, nullptr))
        d();
}

void ErasedScopeGuard::release()
{
    detach = nullptr;
}

} // namespace juce
