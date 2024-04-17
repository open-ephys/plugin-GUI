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

void Logger::outputDebugString (const String& text)
{
    char* data = text.toUTF8().getAddress();
    const size_t length = CharPointer_UTF8::getBytesRequiredFor (text.getCharPointer());
    const size_t chunkSize = 1023;

    size_t position = 0;
    size_t numToRead = jmin (chunkSize, length);

    while (numToRead > 0)
    {
        __android_log_print (ANDROID_LOG_INFO, "JUCE", "%s", data + position);

        position += numToRead;
        numToRead = jmin (chunkSize, length - position);
    }
}

} // namespace juce
