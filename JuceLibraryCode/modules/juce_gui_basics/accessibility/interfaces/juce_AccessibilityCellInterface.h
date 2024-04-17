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

/** An abstract interface which represents a UI element that supports a cell interface.

    This typically represents a single cell inside of a UI element which implements an
    AccessibilityTableInterface.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityCellInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityCellInterface() = default;

    /** Returns the indentation level for the cell. */
    virtual int getDisclosureLevel() const = 0;

    /** Returns the AccessibilityHandler of the table which contains the cell. */
    virtual const AccessibilityHandler* getTableHandler() const = 0;

    /** Returns a list of the accessibility elements that are disclosed by this element, if any. */
    virtual std::vector<const AccessibilityHandler*> getDisclosedRows() const { return {}; }
};

} // namespace juce
