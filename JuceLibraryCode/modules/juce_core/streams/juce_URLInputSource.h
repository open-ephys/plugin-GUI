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
    A type of InputSource that represents a URL.

    @see InputSource

    @tags{Core}
*/
class JUCE_API  URLInputSource     : public InputSource
{
public:
    //==============================================================================
    /** Creates a URLInputSource for a url. */
    URLInputSource (const URL& url);

    /** Move constructor which will move the URL into the InputSource.

        This is useful when the url carries any security credentials.
    */
    URLInputSource (URL&& url);

    /** Destructor. */
    ~URLInputSource() override;

    InputStream* createInputStream() override;
    InputStream* createInputStreamFor (const String& relatedItemPath) override;
    int64 hashCode() const override;

private:
    //==============================================================================
    const URL u;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (URLInputSource)
};

}
