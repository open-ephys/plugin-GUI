/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2020 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PARAMETER_HELPERS__
#define __PARAMETER_HELPERS__

#include <JuceHeader.h>

/** Returns the string reporesentation of an array. */
template<typename ValueType>
static String convertArrayToString (const Array<ValueType>& sourceArray)
{
    // Allow to convert only the arrays which store arithmetic types
    //if (! std::is_arithmetic<ValueType>::value)
    //{
    //    DBG ("Not Arithmetic");
    //    return String::empty;
    //}

    String stringRepr;
    for (auto value: sourceArray)
        stringRepr += value.toString() + ",";

    // Some hardcode - remove last coma at the end of the string
    stringRepr = stringRepr.dropLastCharacters (1);

    return stringRepr;
}


/** Creates and returns the array created from given string. Could be dangerous. */
template <typename ValueType>
static Array<var> createArrayFromString (const String& arrayString, const String& breakCharacters, const String& quoteCharacters = String::empty)
{
    // Allow to convert only the arrays which store arithmetic types
    if (! std::is_arithmetic<ValueType>::value)
       return Array<var> {};

    StringArray valuesStr;
    valuesStr.addTokens (arrayString, breakCharacters, quoteCharacters);

    const bool isBool   = std::is_same<ValueType, bool>::value;
    const bool isInt    = std::is_same<ValueType, int>::value;
    const bool isLong   = std::is_same<ValueType, long>::value;
    const bool isFloat  = std::is_same<ValueType, float>::value;
    const bool isDouble = std::is_same<ValueType, double>::value;

    Array<var> resultArray;
    for (const auto& line: valuesStr)
    {
        if (isBool)
            resultArray.add (line.getFloatValue());
        else if (isInt)
            resultArray.add (line.getIntValue());
        else if (isLong)
            resultArray.add (line.getLargeIntValue());
        else if (isFloat)
            resultArray.add (line.getFloatValue());
        else if (isDouble)
            resultArray.add (line.getDoubleValue());
        // If any another type - try to convert to int
        else
            resultArray.add (line.getIntValue());
    }

    return resultArray;
}

#endif // __PARAMETER_HELPERS__