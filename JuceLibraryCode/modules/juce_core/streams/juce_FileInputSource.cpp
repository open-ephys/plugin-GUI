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

FileInputSource::FileInputSource (const File& f, bool useFileTimeInHash)
    : file (f), useFileTimeInHashGeneration (useFileTimeInHash)
{
}

FileInputSource::~FileInputSource()
{
}

InputStream* FileInputSource::createInputStream()
{
    return file.createInputStream().release();
}

InputStream* FileInputSource::createInputStreamFor (const String& relatedItemPath)
{
    return file.getSiblingFile (relatedItemPath).createInputStream().release();
}

int64 FileInputSource::hashCode() const
{
    int64 h = file.hashCode();

    if (useFileTimeInHashGeneration)
        h ^= file.getLastModificationTime().toMilliseconds();

    return h;
}

} // namespace juce
