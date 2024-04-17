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

LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (const Image& image)
    : RenderingHelpers::StackBasedLowLevelGraphicsContext<RenderingHelpers::SoftwareRendererSavedState>
        (new RenderingHelpers::SoftwareRendererSavedState (image, image.getBounds()))
{
#if JUCE_ETW_TRACELOGGING
    TRACE_LOG_PAINT_CALL(etw::startGDIImage, llgcFrameNumber, etw::softwareRendererKeyword);
#endif
}

LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (const Image& image, Point<int> origin,
                                                                    const RectangleList<int>& initialClip)
    : RenderingHelpers::StackBasedLowLevelGraphicsContext<RenderingHelpers::SoftwareRendererSavedState>
        (new RenderingHelpers::SoftwareRendererSavedState (image, initialClip, origin))
{
#if JUCE_ETW_TRACELOGGING
    TRACE_EVENT_INT_RECT_LIST(etw::startGDIFrame, llgcFrameNumber, initialClip, etw::softwareRendererKeyword);
#endif
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer()
{
#if JUCE_ETW_TRACELOGGING
    TRACE_LOG_PAINT_CALL(etw::endGDIFrame, llgcFrameNumber, etw::softwareRendererKeyword);
#endif
}

} // namespace juce
