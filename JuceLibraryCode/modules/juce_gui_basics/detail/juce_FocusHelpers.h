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

struct FocusHelpers
{
    FocusHelpers() = delete;

    static int getOrder (const Component* c)
    {
        auto order = c->getExplicitFocusOrder();
        return order > 0 ? order : std::numeric_limits<int>::max();
    }

    template <typename FocusContainerFn>
    static void findAllComponents (Component* parent,
                                   std::vector<Component*>& components,
                                   FocusContainerFn isFocusContainer)
    {
        if (parent == nullptr || parent->getNumChildComponents() == 0)
            return;

        std::vector<Component*> localComponents;

        for (auto* c : parent->getChildren())
            if (c->isVisible() && c->isEnabled())
                localComponents.push_back (c);

        const auto compareComponents = [&] (const Component* a, const Component* b)
        {
            const auto getComponentOrderAttributes = [] (const Component* c)
            {
                return std::make_tuple (getOrder (c),
                                        c->isAlwaysOnTop() ? 0 : 1,
                                        c->getY(),
                                        c->getX());
            };

            return getComponentOrderAttributes (a) < getComponentOrderAttributes (b);
        };

        // This will sort so that they are ordered in terms of explicit focus,
        // always on top, left-to-right, and then top-to-bottom.
        std::stable_sort (localComponents.begin(), localComponents.end(), compareComponents);

        for (auto* c : localComponents)
        {
            components.push_back (c);

            if (! (c->*isFocusContainer)())
                findAllComponents (c, components, isFocusContainer);
        }
    }

    enum class NavigationDirection { forwards, backwards };

    template <typename FocusContainerFn>
    static Component* navigateFocus (Component* current,
                                     Component* focusContainer,
                                     NavigationDirection direction,
                                     FocusContainerFn isFocusContainer)
    {
        if (focusContainer != nullptr)
        {
            std::vector<Component*> components;
            findAllComponents (focusContainer, components, isFocusContainer);

            const auto iter = std::find (components.cbegin(), components.cend(), current);

            if (iter == components.cend())
                return nullptr;

            switch (direction)
            {
                case NavigationDirection::forwards:
                    if (iter != std::prev (components.cend()))
                        return *std::next (iter);

                    break;

                case NavigationDirection::backwards:
                    if (iter != components.cbegin())
                        return *std::prev (iter);

                    break;
            }
        }

        return nullptr;
    }
};

} // namespace juce::detail
