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

#ifdef __INTELLISENSE__

    #define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
    #define JUCE_WINDOWS                    1

    #include <d2d1_3.h>
    #include <d3d11_1.h>
    #include <dcomp.h>
    #include <dwrite.h>
    #include <juce_core/juce_core.h>
    #include <juce_graphics/juce_graphics.h>
    #include <windows.h>

#endif

namespace juce
{

namespace
{
    static String getLocalisedName (IDWriteLocalizedStrings* names)
    {
        jassert (names != nullptr);

        uint32 index = 0;
        BOOL exists = false;
        [[maybe_unused]] auto hr = names->FindLocaleName (L"en-us", &index, &exists);

        if (! exists)
            index = 0;

        uint32 length = 0;
        hr = names->GetStringLength (index, &length);

        HeapBlock<wchar_t> name (length + 1);
        hr = names->GetString (index, name, length + 1);

        return static_cast<const wchar_t*> (name);
    }

    static String getFontFamilyName (IDWriteFontFamily* family)
    {
        jassert (family != nullptr);
        ComSmartPtr<IDWriteLocalizedStrings> familyNames;
        [[maybe_unused]] auto hr = family->GetFamilyNames (familyNames.resetAndGetPointerAddress());
        jassert (SUCCEEDED (hr));
        return getLocalisedName (familyNames);
    }

    static String getFontFaceName (IDWriteFont* font)
    {
        jassert (font != nullptr);
        ComSmartPtr<IDWriteLocalizedStrings> faceNames;
        [[maybe_unused]] auto hr = font->GetFaceNames (faceNames.resetAndGetPointerAddress());
        jassert (SUCCEEDED (hr));
        return getLocalisedName (faceNames);
    }

    inline Point<float> convertPoint (D2D1_POINT_2F p) noexcept   { return Point<float> ((float) p.x, (float) p.y); }
}

//==============================================================================
class WindowsDirectWriteTypeface final : public Typeface
{
public:
    WindowsDirectWriteTypeface (const Font& font, IDWriteFontCollection* fontCollection)
        : Typeface (font.getTypefaceName(), font.getTypefaceStyle())
    {
        jassert (fontCollection != nullptr);

        uint32 fontIndex = 0;
        [[maybe_unused]] auto hr = fontCollection->FindFamilyName (font.getTypefaceName().toWideCharPointer(), &fontIndex, &fontFound);

        if (! fontFound)
            fontIndex = 0;

        // Get the font family using the search results
        // Fonts like: Times New Roman, Times New Roman Bold, Times New Roman Italic are all in the same font family
        ComSmartPtr<IDWriteFontFamily> dwFontFamily;
        hr = fontCollection->GetFontFamily (fontIndex, dwFontFamily.resetAndGetPointerAddress());
        if (!dwFontFamily || FAILED(hr))
            return;

        // Get a specific font in the font family using typeface style
        {
            for (int i = (int) dwFontFamily->GetFontCount(); --i >= 0;)
            {
                hr = dwFontFamily->GetFont ((UINT32) i, dwFont.resetAndGetPointerAddress());

                if (i == 0)
                    break;

                ComSmartPtr<IDWriteLocalizedStrings> faceNames;
                hr = dwFont->GetFaceNames (faceNames.resetAndGetPointerAddress());

                if (font.getTypefaceStyle() == getLocalisedName (faceNames))
                    break;
            }

            jassert (dwFont != nullptr);
            hr = dwFont->CreateFontFace (dwFontFace.resetAndGetPointerAddress());
        }

        initializeFromFontFace();
    }

    //
    // Alternate constructor for WindowsDirectWriteTypeface to create the typeface from TTF data in memory
    //
    WindowsDirectWriteTypeface (const void* data, size_t dataSize) :
        Typeface({}, {}) // set the typeface name & style as empty initially
    {
        //
        // Get the DirectWrite font family for the raw data
        //
        auto fontFamily = directWrite->getFontFamilyForRawData(data, dataSize);
        if (fontFamily == nullptr)
        {
            return;
        }

        //
        // Get the JUCE typeface name from the DirectWrite font family
        //
        {
            ComSmartPtr<IDWriteLocalizedStrings> familyNames;
            auto hr = fontFamily->GetFamilyNames(familyNames.resetAndGetPointerAddress());
            if (FAILED(hr))
            {
                return;
            }

            name = getLocalisedName(familyNames);
        }

        //
        // Get the JUCE typeface style from the DirectWrite font and get the font face
        //
        // Only supports one font per family
        //
        {
            auto hr = fontFamily->GetFont(0, dwFont.resetAndGetPointerAddress());
            if (FAILED(hr))
            {
                return;
            }

            ComSmartPtr<IDWriteLocalizedStrings> faceNames;
            hr = dwFont->GetFaceNames(faceNames.resetAndGetPointerAddress());
            if (FAILED(hr))
            {
                return;
            }

            fontFound = true;

            style = getLocalisedName(faceNames);

            hr = dwFont->CreateFontFace(dwFontFace.resetAndGetPointerAddress());
            if (FAILED(hr))
            {
                return;
            }
        }

        initializeFromFontFace();
    }

    bool loadedOk() const noexcept          { return dwFontFace != nullptr; }
    BOOL isFontFound() const noexcept       { return fontFound; }

    float getAscent() const                 override { return ascent; }
    float getDescent() const                override { return 1.0f - ascent; }
    float getHeightToPointsFactor() const   override { return heightToPointsFactor; }

    float getStringWidth (const String& text) override
    {
        auto textUTF32 = text.toUTF32();
        auto len = textUTF32.length();

        HeapBlock<UINT16> glyphIndices (len);
        dwFontFace->GetGlyphIndices (textUTF32, (UINT32) len, glyphIndices);

        HeapBlock<DWRITE_GLYPH_METRICS> dwGlyphMetrics (len);
        dwFontFace->GetDesignGlyphMetrics (glyphIndices, (UINT32) len, dwGlyphMetrics, false);

        float x = 0;

        for (size_t i = 0; i < len; ++i)
            x += (float) dwGlyphMetrics[i].advanceWidth / (float) designUnitsPerEm;

        return x * unitsToHeightScaleFactor;
    }

    void getGlyphPositions (const String& text, Array<int>& resultGlyphs, Array<float>& xOffsets) override
    {
        xOffsets.add (0);

        auto textUTF32 = text.toUTF32();
        auto len = textUTF32.length();

        HeapBlock<UINT16> glyphIndices (len);
        dwFontFace->GetGlyphIndices (textUTF32, (UINT32) len, glyphIndices);
        HeapBlock<DWRITE_GLYPH_METRICS> dwGlyphMetrics (len);
        dwFontFace->GetDesignGlyphMetrics (glyphIndices, (UINT32) len, dwGlyphMetrics, false);

        float x = 0;

        for (size_t i = 0; i < len; ++i)
        {
            x += (float) dwGlyphMetrics[i].advanceWidth / (float) designUnitsPerEm;
            xOffsets.add (x * unitsToHeightScaleFactor);
            resultGlyphs.add (glyphIndices[i]);
        }
    }

    bool getOutlineForGlyph (int glyphNumber, Path& path) override
    {
        jassert (path.isEmpty());  // we might need to apply a transform to the path, so this must be empty
        auto glyphIndex = (UINT16) glyphNumber;
        ComSmartPtr<PathGeometrySink> pathGeometrySink (new PathGeometrySink());

        dwFontFace->GetGlyphRunOutline (1024.0f, &glyphIndex, nullptr, nullptr,
                                        1, false, false, pathGeometrySink);
        path = pathGeometrySink->path;

        if (! pathTransform.isIdentity())
            path.applyTransform (pathTransform);

        return true;
    }

    IDWriteFont* getIDWriteFont() const noexcept { return dwFont; }
    IDWriteFontFace* getIDWriteFontFace() const noexcept    { return dwFontFace; }

    float getUnitsToHeightScaleFactor() const noexcept      { return unitsToHeightScaleFactor; }

private:
    SharedResourcePointer<DirectWrite> directWrite;
    ComSmartPtr<IDWriteFont> dwFont;
    ComSmartPtr<IDWriteFontFace> dwFontFace;
    float unitsToHeightScaleFactor = 1.0f, heightToPointsFactor = 1.0f, ascent = 0;
    int designUnitsPerEm = 0;
    AffineTransform pathTransform;
    BOOL fontFound = false;

    //
    // D.R.Y. since this code is common to both constructors
    //
    void initializeFromFontFace()
    {
        if (dwFontFace != nullptr)
        {
            DWRITE_FONT_METRICS dwFontMetrics;
            dwFontFace->GetMetrics(&dwFontMetrics);

            // All Font Metrics are in design units so we need to get designUnitsPerEm value
            // to get the metrics into Em/Design Independent Pixels
            designUnitsPerEm = dwFontMetrics.designUnitsPerEm;

            ascent = std::abs((float)dwFontMetrics.ascent);
            auto totalSize = ascent + std::abs((float)dwFontMetrics.descent);
            ascent /= totalSize;
            unitsToHeightScaleFactor = (float)designUnitsPerEm / totalSize;

            auto tempDC = GetDC(nullptr);
            auto dpi = (float)(GetDeviceCaps(tempDC, LOGPIXELSX) + GetDeviceCaps(tempDC, LOGPIXELSY)) / 2.0f;
            heightToPointsFactor = (dpi / (float)GetDeviceCaps(tempDC, LOGPIXELSY)) * unitsToHeightScaleFactor;
            ReleaseDC(nullptr, tempDC);

            auto pathAscent = (1024.0f * dwFontMetrics.ascent) / (float)designUnitsPerEm;
            auto pathDescent = (1024.0f * dwFontMetrics.descent) / (float)designUnitsPerEm;
            auto pathScale = 1.0f / (std::abs(pathAscent) + std::abs(pathDescent));
            pathTransform = AffineTransform::scale(pathScale);
        }
    }

    struct PathGeometrySink  : public ComBaseClassHelper<IDWriteGeometrySink>
    {
        PathGeometrySink() : ComBaseClassHelper (0) {}

        void STDMETHODCALLTYPE AddBeziers (const D2D1_BEZIER_SEGMENT* beziers, UINT beziersCount) noexcept override
        {
            for (UINT i = 0; i < beziersCount; ++i)
                path.cubicTo (convertPoint (beziers[i].point1),
                              convertPoint (beziers[i].point2),
                              convertPoint (beziers[i].point3));
        }

        void STDMETHODCALLTYPE AddLines (const D2D1_POINT_2F* points, UINT pointsCount) noexcept override
        {
            for (UINT i = 0; i < pointsCount; ++i)
                path.lineTo (convertPoint (points[i]));
        }

        void STDMETHODCALLTYPE BeginFigure (D2D1_POINT_2F startPoint, D2D1_FIGURE_BEGIN) noexcept override
        {
            path.startNewSubPath (convertPoint (startPoint));
        }

        void STDMETHODCALLTYPE EndFigure (D2D1_FIGURE_END figureEnd) noexcept override
        {
            if (figureEnd == D2D1_FIGURE_END_CLOSED)
                path.closeSubPath();
        }

        void STDMETHODCALLTYPE SetFillMode (D2D1_FILL_MODE fillMode) noexcept override
        {
            path.setUsingNonZeroWinding (fillMode == D2D1_FILL_MODE_WINDING);
        }

        void STDMETHODCALLTYPE SetSegmentFlags (D2D1_PATH_SEGMENT) noexcept override {}
        JUCE_COMRESULT Close() noexcept override  { return S_OK; }

        Path path;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathGeometrySink)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsDirectWriteTypeface)
};

} // namespace juce
