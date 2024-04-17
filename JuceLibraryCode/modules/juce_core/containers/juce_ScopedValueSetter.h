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
/**
    Helper class providing an RAII-based mechanism for temporarily setting and
    then re-setting a value.

    E.g. @code
    int x = 1;

    {
        ScopedValueSetter setter (x, 2);

        // x is now 2
    }

    // x is now 1 again

    {
        ScopedValueSetter setter (x, 3, 4);

        // x is now 3
    }

    // x is now 4
    @endcode

    @tags{Core}
*/
template <typename ValueType>
class ScopedValueSetter
{
public:
    /** Creates a ScopedValueSetter that will immediately change the specified value to the
        given new value, and will then reset it to its original value when this object is deleted.
    */
    ScopedValueSetter (ValueType& valueToSet,
                       ValueType newValue)
        : value (valueToSet),
          originalValue (valueToSet)
    {
        valueToSet = newValue;
    }

    /** Creates a ScopedValueSetter that will immediately change the specified value to the
        given new value, and will then reset it to be valueWhenDeleted when this object is deleted.
    */
    ScopedValueSetter (ValueType& valueToSet,
                       ValueType newValue,
                       ValueType valueWhenDeleted)
        : value (valueToSet),
          originalValue (valueWhenDeleted)
    {
        valueToSet = newValue;
    }

    ~ScopedValueSetter()
    {
        value = originalValue;
    }

private:
    //==============================================================================
    ValueType& value;
    const ValueType originalValue;

    JUCE_DECLARE_NON_COPYABLE (ScopedValueSetter)
};

} // namespace juce
