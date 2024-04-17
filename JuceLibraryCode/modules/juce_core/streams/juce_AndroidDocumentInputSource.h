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
    An InputSource backed by an AndroidDocument.

    @see InputSource, AndroidDocument

    @tags{Core}
*/
class JUCE_API  AndroidDocumentInputSource   : public InputSource
{
public:
    //==============================================================================
    /** Creates a new AndroidDocumentInputSource, backed by the provided document.
    */
    explicit AndroidDocumentInputSource (const AndroidDocument& doc)
        : document (doc) {}

    //==============================================================================
    /** Returns a new InputStream to read this item.

        @returns            an inputstream that the caller will delete, or nullptr if
                            the document can't be opened.
    */
    InputStream* createInputStream() override
    {
        return document.createInputStream().release();
    }

    /** @internal

        An AndroidDocument doesn't use conventional filesystem paths.
        Use the member functions of AndroidDocument to locate relative items.

        @param relatedItemPath  the relative pathname of the resource that is required
        @returns            an input stream if relatedItemPath was empty, otherwise
                            nullptr.
    */
    InputStream* createInputStreamFor (const String& relatedItemPath) override
    {
        return relatedItemPath.isEmpty() ? document.createInputStream().release() : nullptr;
    }

    /** Returns a hash code that uniquely represents this item.
    */
    int64 hashCode() const override
    {
        return document.getUrl().toString (true).hashCode64();
    }

private:
    AndroidDocument document;
};

} // namespace juce
