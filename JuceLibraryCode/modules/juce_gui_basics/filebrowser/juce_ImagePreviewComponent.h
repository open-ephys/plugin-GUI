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
    A simple preview component that shows thumbnails of image files.

    @see FileChooserDialogBox, FilePreviewComponent

    @tags{GUI}
*/
class JUCE_API  ImagePreviewComponent  : public FilePreviewComponent,
                                         private Timer
{
public:
    //==============================================================================
    /** Creates an ImagePreviewComponent. */
    ImagePreviewComponent();

    /** Destructor. */
    ~ImagePreviewComponent() override;

    //==============================================================================
    /** @internal */
    void selectedFileChanged (const File& newSelectedFile) override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void timerCallback() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    File fileToLoad;
    Image currentThumbnail;
    String currentDetails;

    void getThumbSize (int& w, int& h) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagePreviewComponent)
};

} // namespace juce
