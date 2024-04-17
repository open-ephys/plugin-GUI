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

#if JUCE_ETW_TRACELOGGING

struct ETWEventProvider
{
    ETWEventProvider();
    ~ETWEventProvider();
};

#endif

class Direct2DGraphicsContext : public LowLevelGraphicsContext
{
public:
    Direct2DGraphicsContext();
    ~Direct2DGraphicsContext() override;

    //==============================================================================
    bool isVectorDevice() const override
    {
        return false;
    }

    void setOrigin (Point<int>) override;
    void addTransform (const AffineTransform&) override;
    float getPhysicalPixelScaleFactor() override;
    bool clipToRectangle (const Rectangle<int>&) override;
    bool clipToRectangleList (const RectangleList<int>&) override;
    void excludeClipRectangle (const Rectangle<int>&) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;
    bool clipRegionIntersects (const Rectangle<int>&) override;
    Rectangle<int> getClipBounds() const override;
    bool isClipEmpty() const override;

    //==============================================================================
    void saveState() override;
    void restoreState() override;
    void beginTransparencyLayer (float opacity) override;
    void endTransparencyLayer() override;

    //==============================================================================
    void setFill (const FillType&) override;
    void setOpacity (float) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) override;

    //==============================================================================
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRect (const Rectangle<float>&) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    void drawLine (const Line<float>&) override;
    void setFont (const Font&) override;
    const Font& getFont() override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;
    bool drawTextLayout (const AttributedString&, const Rectangle<float>&) override;


    //==============================================================================
    //
    // These methods are not part of the standard LowLevelGraphicsContext; they
    // were added because Direct2D supports these drawing primitives
    //
    // Standard LLGC only supports drawing one glyph at a time; it's much more
    // efficient to pass an entire run of glyphs to the device context
    //
    bool drawLineWithThickness (const Line<float>&, float) override;

    bool drawEllipse (Rectangle<float> area, float lineThickness) override;
    bool fillEllipse (Rectangle<float> area) override;

    bool drawRect (const Rectangle<float>&, float) override;
    bool drawPath (const Path&, const PathStrokeType& strokeType, const AffineTransform&) override;

    bool drawRoundedRectangle (Rectangle<float> area, float cornerSize, float lineThickness) override;
    bool fillRoundedRectangle (Rectangle<float> area, float cornerSize) override;

    bool supportsGlyphRun() override
    {
        return true;
    }
    void drawPositionedGlyphRun (Array<PositionedGlyph> const& glyphs,
                       int                           startIndex,
                       int                           numGlyphs,
                       const AffineTransform&        transform,
                       Rectangle<float>              underlineArea) override;
    void drawTextLayoutGlyphRun(Array<TextLayout::Glyph> const&, Font const&, const AffineTransform&) override;


    //==============================================================================
    bool startFrame();
    void endFrame();

    void setPhysicalPixelScaleFactor(float scale_);

    virtual Image createSnapshot(Rectangle<int> /*areaDIPs*/)
    {
        return {};
    }

    virtual Image createSnapshot()
    {
        return {};
    }

    direct2d::Metrics::Ptr metrics;

    //==============================================================================
    //
    // Min & max frame sizes; same as Direct3D texture size limits
    //
    static int constexpr minFrameSize = 1;
    static int constexpr maxFrameSize = 16384;

    //==============================================================================
protected:
    struct SavedState;
    SavedState* currentState = nullptr;
    RectangleList<int> pendingDeviceSpaceClipList;

    struct Pimpl;
    virtual Pimpl* getPimpl() const noexcept = 0;

    void resetPendingClipList();
    void applyPendingClipList();
    virtual void clearTargetBuffer() = 0;
    void drawGlyphCommon (int numGlyphs, Font const& font, const AffineTransform& transform, Rectangle<float> underlineArea);

    struct ScopedTransform
    {
        ScopedTransform(Pimpl& pimpl_, SavedState* state_);
        ScopedTransform(Pimpl& pimpl_, SavedState* state_, const AffineTransform& transform);
        ~ScopedTransform();

        Pimpl& pimpl;
        SavedState* state = nullptr;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DGraphicsContext)
};

} // namespace juce
