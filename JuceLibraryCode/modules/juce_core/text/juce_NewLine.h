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
/** This class is used for represent a new-line character sequence.

    To write a new-line to a stream, you can use the predefined 'newLine' variable, e.g.
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode

    The exact character sequence that will be used for the new-line can be set and
    retrieved with OutputStream::setNewLineString() and OutputStream::getNewLineString().

    @tags{Core}
*/
class JUCE_API  NewLine
{
public:
    /** Returns the default new-line sequence that the library uses.
        @see OutputStream::setNewLineString()
    */
    static const char* getDefault() noexcept        { return "\r\n"; }

    /** Returns the default new-line sequence that the library uses.
        @see getDefault()
    */
    operator String() const                         { return getDefault(); }

    /** Returns the default new-line sequence that the library uses.
        @see OutputStream::setNewLineString()
    */
    operator StringRef() const noexcept             { return getDefault(); }
};

//==============================================================================
/** A predefined object representing a new-line, which can be written to a string or stream.

    To write a new-line to a stream, you can use the predefined 'newLine' variable like this:
    @code
    myOutputStream << "Hello World" << newLine << newLine;
    @endcode
*/
extern NewLine newLine;

//==============================================================================
/** Writes a new-line sequence to a string.
    You can use the predefined object 'newLine' to invoke this, e.g.
    @code
    myString << "Hello World" << newLine << newLine;
    @endcode
*/
inline String& operator<< (String& string1, const NewLine&) { return string1 += NewLine::getDefault(); }
inline String& operator+= (String& s1, const NewLine&)      { return s1 += NewLine::getDefault(); }

inline String operator+ (const NewLine&, const NewLine&)    { return String (NewLine::getDefault()) + NewLine::getDefault(); }
inline String operator+ (String s1, const NewLine&)         { return s1 += NewLine::getDefault(); }
inline String operator+ (const NewLine&, const char* s2)    { return String (NewLine::getDefault()) + s2; }

} // namespace juce
