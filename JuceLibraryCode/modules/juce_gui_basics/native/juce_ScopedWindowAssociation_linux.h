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

extern XContext windowHandleXContext;

/*  Attaches a pointer to a given window, so that it can be retrieved with XFindContext on
    the windowHandleXContext.
*/
class ScopedWindowAssociation
{
public:
    ScopedWindowAssociation() = default;

    ScopedWindowAssociation (void* associatedIn, Window windowIn)
        : associatedPointer ([&]() -> void*
          {
              if (associatedIn == nullptr)
                  return nullptr;

              // If you hit this, there's already a pointer associated with this window.
              const auto display = XWindowSystem::getInstance()->getDisplay();
              jassert (! getAssociatedPointer (display, windowIn).has_value());

              if (X11Symbols::getInstance()->xSaveContext (display,
                                                           static_cast<XID> (windowIn),
                                                           windowHandleXContext,
                                                           unalignedPointerCast<XPointer> (associatedIn)) != 0)
              {
                  jassertfalse;
                  return nullptr;
              }

              return associatedIn;
          }()),
          window (static_cast<XID> (windowIn)) {}

    ScopedWindowAssociation (const ScopedWindowAssociation&) = delete;
    ScopedWindowAssociation& operator= (const ScopedWindowAssociation&) = delete;

    ScopedWindowAssociation (ScopedWindowAssociation&& other) noexcept
        : associatedPointer (std::exchange (other.associatedPointer, nullptr)), window (other.window) {}

    ScopedWindowAssociation& operator= (ScopedWindowAssociation&& other) noexcept
    {
        ScopedWindowAssociation { std::move (other) }.swap (*this);
        return *this;
    }

    ~ScopedWindowAssociation() noexcept
    {
        if (associatedPointer == nullptr)
            return;

        const auto display = XWindowSystem::getInstance()->getDisplay();
        const auto ptr = getAssociatedPointer (display, window);

        if (! ptr.has_value())
        {
            // If you hit this, something else has cleared this association before we were able to.
            jassertfalse;
            return;
        }

        jassert (unalignedPointerCast<XPointer> (associatedPointer) == *ptr);

        if (X11Symbols::getInstance()->xDeleteContext (display, window, windowHandleXContext) != 0)
            jassertfalse;
    }

    bool isValid() const { return associatedPointer != nullptr; }

private:
    static std::optional<XPointer> getAssociatedPointer (Display* display, Window window)
    {
        XPointer ptr{};

        if (X11Symbols::getInstance()->xFindContext (display, window, windowHandleXContext, &ptr) != 0)
            return std::nullopt;

        return ptr;
    }

    void swap (ScopedWindowAssociation& other) noexcept
    {
        std::swap (other.associatedPointer, associatedPointer);
        std::swap (other.window, window);
    }

    void* associatedPointer = nullptr;
    XID window{};
};

} // namespace juce
