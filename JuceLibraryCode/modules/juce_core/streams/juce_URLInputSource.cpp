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

URLInputSource::URLInputSource (const URL& url)
    : u (url)
{
}

URLInputSource::URLInputSource (URL&& url)
    : u (std::move (url))
{
}

URLInputSource::~URLInputSource()
{
}

InputStream* URLInputSource::createInputStream()
{
    return u.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)).release();
}

InputStream* URLInputSource::createInputStreamFor (const String& relatedItemPath)
{
    auto sub = u.getSubPath();
    auto parent = sub.containsChar (L'/') ? sub.upToLastOccurrenceOf ("/", false, false)
                                          : String();

    return u.withNewSubPath (parent)
            .getChildURL (relatedItemPath)
            .createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress))
            .release();
}

int64 URLInputSource::hashCode() const
{
    return u.toString (true).hashCode64();
}

} // namespace juce
