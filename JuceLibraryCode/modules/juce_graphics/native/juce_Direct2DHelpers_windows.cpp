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

    #include <windows.h>
    #include <juce_core/juce_core.h>
    #include <juce_graphics/juce_graphics.h>
    #include <d2d1_3.h>
    #include <d3d11_1.h>
    #include <dwrite.h>
    #include <dcomp.h>
    #include <evntrace.h>
    #include <TraceLoggingProvider.h>

#endif

namespace juce
{

namespace direct2d
{

//==============================================================================
//
// Utility routines to convert between Win32 and JUCE types
//
// Some of these are duplicated in juce_Windowing_windows.cpp; could do
// some D.R.Y.
//

template <typename Type>
D2D1_RECT_F rectangleToRectF (const Rectangle<Type>& r)
{
    return { (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom() };
}

template <typename Type>
RECT rectangleToRECT (const Rectangle<Type>& r)
{
    return { r.getX(), r.getY(), r.getRight(), r.getBottom() };
}

template <typename Type>
Rectangle<int> RECTToRectangle (RECT const& r)
{
    return Rectangle<int>::leftTopRightBottom (r.left, r.top, r.right, r.bottom);
}

static D2D1_COLOR_F colourToD2D (Colour c)
{
    return { c.getFloatRed(), c.getFloatGreen(), c.getFloatBlue(), c.getFloatAlpha() };
}

//==============================================================================
//
// Scoped I2D1Multithread
//

class ScopedMultithread
{
public:
    ScopedMultithread(ID2D1Multithread* multithread_) :
        multithread(multithread_)
    {
        multithread_->Enter();
    }

    ~ScopedMultithread()
    {
        multithread->Leave();
    }

private:
    ComSmartPtr<ID2D1Multithread> multithread;
};

//==============================================================================
//
// Convert a JUCE Path to a D2D Geometry
//

static void pathToGeometrySink (const Path& path, ID2D1GeometrySink* sink, const AffineTransform& transform, D2D1_FIGURE_BEGIN figureMode)
{
    //
    // Every call to BeginFigure must have a matching call to EndFigure. But - the Path does not necessarily
    // have matching startNewSubPath and closePath markers. The figureStarted flag indicates if an extra call
    // to BeginFigure or EndFigure is needed during the iteration loop or when exiting this function.
    //
    Path::Iterator it (path);
    bool           figureStarted = false;

    while (it.next())
    {
        switch (it.elementType)
        {
            case Path::Iterator::cubicTo:
            {
                jassert (figureStarted);

                transform.transformPoint (it.x1, it.y1);
                transform.transformPoint (it.x2, it.y2);
                transform.transformPoint (it.x3, it.y3);

                sink->AddBezier ({ { it.x1, it.y1 }, { it.x2, it.y2 }, { it.x3, it.y3 } });
                break;
            }

            case Path::Iterator::lineTo:
            {
                jassert (figureStarted);

                transform.transformPoint (it.x1, it.y1);
                sink->AddLine ({ it.x1, it.y1 });
                break;
            }

            case Path::Iterator::quadraticTo:
            {
                jassert (figureStarted);

                transform.transformPoint (it.x1, it.y1);
                transform.transformPoint (it.x2, it.y2);
                sink->AddQuadraticBezier ({ { it.x1, it.y1 }, { it.x2, it.y2 } });
                break;
            }

            case Path::Iterator::closePath:
            {
                if (figureStarted)
                {
                    sink->EndFigure (D2D1_FIGURE_END_CLOSED);
                    figureStarted = false;
                }
                break;
            }

            case Path::Iterator::startNewSubPath:
            {
                if (figureStarted)
                {
                    sink->EndFigure (D2D1_FIGURE_END_OPEN);
                }

                transform.transformPoint (it.x1, it.y1);
                sink->BeginFigure ({ it.x1, it.y1 }, figureMode);

                figureStarted = true;
                break;
            }
        }
    }

    if (figureStarted)
    {
        sink->EndFigure (D2D1_FIGURE_END_OPEN);
    }
}

static void pathToGeometrySink(const Path& path, ID2D1GeometrySink* sink, D2D1_FIGURE_BEGIN figureMode)
{
    Path::Iterator it(path);
    bool           figureStarted = false;

    while (it.next())
    {
        switch (it.elementType)
        {
        case Path::Iterator::cubicTo:
        {
            jassert(figureStarted);
            sink->AddBezier({ { it.x1, it.y1 }, { it.x2, it.y2 }, { it.x3, it.y3 } });
            break;
        }

        case Path::Iterator::lineTo:
        {
            jassert(figureStarted);
            sink->AddLine({ it.x1, it.y1 });
            break;
        }

        case Path::Iterator::quadraticTo:
        {
            jassert(figureStarted);
            sink->AddQuadraticBezier({ { it.x1, it.y1 }, { it.x2, it.y2 } });
            break;
        }

        case Path::Iterator::closePath:
        {
            if (figureStarted)
            {
                sink->EndFigure(D2D1_FIGURE_END_CLOSED);
                figureStarted = false;
            }
            break;
        }

        case Path::Iterator::startNewSubPath:
        {
            if (figureStarted)
            {
                sink->EndFigure(D2D1_FIGURE_END_OPEN);
            }
            sink->BeginFigure({ it.x1, it.y1 }, figureMode);
            figureStarted = true;
            break;
        }
        }
    }

    if (figureStarted)
    {
        sink->EndFigure(D2D1_FIGURE_END_OPEN);
    }
}

static D2D1::Matrix3x2F transformToMatrix (const AffineTransform& transform)
{
    return { transform.mat00, transform.mat10, transform.mat01, transform.mat11, transform.mat02, transform.mat12 };
}

static D2D1_POINT_2F pointTransformed (int x, int y, const AffineTransform& transform)
{
    auto xf = (float) x;
    auto yf = (float) y;
    transform.transformPoint (xf, yf);
    return { (FLOAT) xf, (FLOAT) yf };
}

static void rectToGeometrySink (const Rectangle<int>& rect, ID2D1GeometrySink* sink, const AffineTransform& transform, D2D1_FIGURE_BEGIN figureMode)
{
    sink->BeginFigure (pointTransformed (rect.getX(), rect.getY(), transform), figureMode);
    sink->AddLine (pointTransformed (rect.getRight(), rect.getY(), transform));
    sink->AddLine (pointTransformed (rect.getRight(), rect.getBottom(), transform));
    sink->AddLine (pointTransformed (rect.getX(), rect.getBottom(), transform));
    sink->EndFigure (D2D1_FIGURE_END_CLOSED);
}

//
// ScopedGeometryWithSink creates an ID2D1PathGeometry object with an open sink.
// D.R.Y. for rectListToPathGeometry, and pathToPathGeometry
//
// Ensures that sink->Close is called
//
struct ScopedGeometryWithSink
{
    ScopedGeometryWithSink (ID2D1Factory* factory, D2D1_FILL_MODE fillMode)
    {
        auto hr = factory->CreatePathGeometry (geometry.resetAndGetPointerAddress());
        if (SUCCEEDED (hr))
        {
            hr = geometry->Open (sink.resetAndGetPointerAddress());
            if (SUCCEEDED (hr))
            {
                sink->SetFillMode (fillMode);
            }
        }
    }

    ~ScopedGeometryWithSink()
    {
        if (sink != nullptr)
        {
            auto hr = sink->Close();
            jassertquiet (SUCCEEDED (hr));
        }
    }

    ComSmartPtr<ID2D1PathGeometry> geometry;
    ComSmartPtr<ID2D1GeometrySink> sink;
};

static ComSmartPtr<ID2D1Geometry> rectListToPathGeometry (ID2D1Factory* factory,
                                                          const RectangleList<int>& clipRegion,
                                                          const AffineTransform& transform,
                                                          D2D1_FILL_MODE fillMode,
                                                          D2D1_FIGURE_BEGIN figureMode,
                                                          [[maybe_unused]]Metrics* metrics)
{
    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createGeometryTime)
    ScopedGeometryWithSink objects { factory, fillMode };

    if (objects.sink != nullptr)
    {
        for (int i = clipRegion.getNumRectangles(); --i >= 0;)
            direct2d::rectToGeometrySink (clipRegion.getRectangle (i), objects.sink, transform, figureMode);

        return { (ID2D1Geometry*) objects.geometry };
    }

    return nullptr;
}

static ComSmartPtr<ID2D1Geometry> pathToPathGeometry(ID2D1Factory* factory,
    const Path& path,
    const AffineTransform& transform,
    D2D1_FIGURE_BEGIN figureMode,
    [[maybe_unused]] Metrics* metrics)
{
    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createGeometryTime)
    ScopedGeometryWithSink objects { factory, path.isUsingNonZeroWinding() ? D2D1_FILL_MODE_WINDING : D2D1_FILL_MODE_ALTERNATE };

    if (objects.sink != nullptr)
    {
        direct2d::pathToGeometrySink (path, objects.sink, transform, figureMode);

        return { (ID2D1Geometry*) objects.geometry };
    }

    return nullptr;
}

static ComSmartPtr<ID2D1Geometry> pathToPathGeometry(ID2D1Factory* factory, const Path& path, D2D1_FIGURE_BEGIN figureMode, [[maybe_unused]] Metrics* metrics)
{
    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createGeometryTime)
    ScopedGeometryWithSink objects{ factory, path.isUsingNonZeroWinding() ? D2D1_FILL_MODE_WINDING : D2D1_FILL_MODE_ALTERNATE };

    if (objects.sink != nullptr)
    {
        direct2d::pathToGeometrySink(path, objects.sink, figureMode);

        return { (ID2D1Geometry*)objects.geometry };
    }

    return nullptr;
}

static ComSmartPtr<ID2D1StrokeStyle1> pathStrokeTypeToStrokeStyle(ID2D1Factory1* factory, const PathStrokeType& strokeType)
{
    // JUCE JointStyle   ID2D1StrokeStyle
    // ---------------   ----------------
    // mitered           D2D1_LINE_JOIN_MITER
    // curved            D2D1_LINE_JOIN_ROUND
    // beveled           D2D1_LINE_JOIN_BEVEL
    //
    // JUCE EndCapStyle  ID2D1StrokeStyle
    // ----------------  ----------------
    // butt              D2D1_CAP_STYLE_FLAT
    // square            D2D1_CAP_STYLE_SQUARE
    // rounded           D2D1_CAP_STYLE_ROUND
    //
    auto lineJoin = D2D1_LINE_JOIN_MITER;
    switch (strokeType.getJointStyle())
    {
    case PathStrokeType::JointStyle::mitered:
        // already set
        break;

    case PathStrokeType::JointStyle::curved: lineJoin = D2D1_LINE_JOIN_ROUND; break;

    case PathStrokeType::JointStyle::beveled: lineJoin = D2D1_LINE_JOIN_BEVEL; break;

    default:
        // invalid EndCapStyle
        jassertfalse;
        break;
    }

    auto capStyle = D2D1_CAP_STYLE_FLAT;
    switch (strokeType.getEndStyle())
    {
    case PathStrokeType::EndCapStyle::butt:
        // already set
        break;

    case PathStrokeType::EndCapStyle::square: capStyle = D2D1_CAP_STYLE_SQUARE; break;

    case PathStrokeType::EndCapStyle::rounded: capStyle = D2D1_CAP_STYLE_ROUND; break;

    default:
        // invalid EndCapStyle
        jassertfalse;
        break;
    }

    D2D1_STROKE_STYLE_PROPERTIES1 strokeStyleProperties
    {
        capStyle,
        capStyle,
        capStyle,
        lineJoin,
        strokeType.getStrokeThickness(),
        D2D1_DASH_STYLE_SOLID,
        0.0f,
        D2D1_STROKE_TRANSFORM_TYPE_NORMAL
    };
    ComSmartPtr<ID2D1StrokeStyle1> strokeStyle;
    factory->CreateStrokeStyle(strokeStyleProperties,
        nullptr,
        0,
        strokeStyle.resetAndGetPointerAddress());

    return strokeStyle;
}

//==============================================================================
//
// UpdateRegion extracts the invalid region for a window
//
// UpdateRegion is used to service WM_PAINT to add the invalid region of a window to
// deferredRepaints. UpdateRegion marks the region as valid, and the region should be painted on the
// next vblank.
//
// This is similar to the invalid region update in HWNDComponentPeer::handlePaintMessage()
//

class UpdateRegion
{
public:
    ~UpdateRegion()
    {
        clear();
    }

    void findRECTAndValidate (HWND windowHandle)
    {
        numRect = 0;

        auto regionHandle = CreateRectRgn (0, 0, 0, 0);
        if (regionHandle)
        {
            auto regionType = GetUpdateRgn (windowHandle, regionHandle, false);
            if (regionType == SIMPLEREGION || regionType == COMPLEXREGION)
            {
                auto regionDataBytes = GetRegionData (regionHandle, (DWORD) block.getSize(), (RGNDATA*) block.getData());
                if (regionDataBytes > block.getSize())
                {
                    block.ensureSize (regionDataBytes);
                    regionDataBytes = GetRegionData (regionHandle, (DWORD) block.getSize(), (RGNDATA*) block.getData());
                }

                if (regionDataBytes > 0)
                {
                    auto header = (RGNDATAHEADER const* const) block.getData();
                    if (header->iType == RDH_RECTANGLES)
                    {
                        numRect = header->nCount;
                    }
                }
            }

            if (numRect > 0)
            {
                ValidateRgn (windowHandle, regionHandle);
            }
            else
            {
                ValidateRect (windowHandle, nullptr);
            }

            DeleteObject (regionHandle);
            regionHandle = nullptr;

            return;
        }

        ValidateRect (windowHandle, nullptr);
    }

    void clear()
    {
        numRect = 0;
    }

    uint32 getNumRECT() const
    {
        return numRect;
    }

    RECT* getRECTArray()
    {
        auto header = (RGNDATAHEADER const* const) block.getData();
        return (RECT*) (header + 1);
    }

    static void forwardInvalidRegionToParent (HWND childHwnd)
    {
        auto regionHandle = CreateRectRgn (0, 0, 0, 0);
        if (regionHandle)
        {
            GetUpdateRgn (childHwnd, regionHandle, false);
            ValidateRgn (childHwnd, regionHandle);
            InvalidateRgn (GetParent (childHwnd), regionHandle, FALSE);
            DeleteObject (regionHandle);
        }
    }

private:
    MemoryBlock block { 1024 };
    uint32      numRect = 0;
};


//==============================================================================
//
// Wrapper for a DirectWrite font face along with relevant font info
//

struct DirectWriteFontFace
{
    ComSmartPtr<IDWriteFontFace> fontFace;
    float                        fontHeight               = 0.0f;
    float                        fontHeightToEmSizeFactor = 0.0f;
    float                        fontHorizontalScale      = 0.0f;

    float getEmSize() const noexcept
    {
        return fontHeight * fontHeightToEmSizeFactor;
    }

    void clear()
    {
        fontFace = nullptr;
    }

    static DirectWriteFontFace fromFont(Font const& font)
    {
        ReferenceCountedObjectPtr<WindowsDirectWriteTypeface> typeface =
            dynamic_cast<WindowsDirectWriteTypeface*> (font.getTypefacePtr().get());
        if (typeface)
        {
            return { typeface->getIDWriteFontFace(), font.getHeight(), typeface->getUnitsToHeightScaleFactor(), font.getHorizontalScale() };
        }

        return {};
    }
};

//==============================================================================
//
// Heap storage for a DirectWrite glyph run
//

class DirectWriteGlyphRun
{
public:
    DirectWriteGlyphRun()
    {
        ensureStorageAllocated (16);
    }

    void ensureStorageAllocated (int capacityNeeded)
    {
        if (capacityNeeded > glyphCapacity)
        {
            glyphCapacity = capacityNeeded;
            glyphIndices.realloc (capacityNeeded);
            glyphAdvances.realloc (capacityNeeded);
            glyphOffsets.realloc (capacityNeeded);

            glyphAdvances.clear (capacityNeeded);
        }
    }

    int                            glyphCapacity = 0;
    HeapBlock<UINT16>              glyphIndices;
    HeapBlock<float>               glyphAdvances;
    HeapBlock<DWRITE_GLYPH_OFFSET> glyphOffsets;
};

class ScopedEvent
{
public:
    explicit ScopedEvent (HANDLE handle_)
        : handle (handle_)
    {
    }

    ScopedEvent()
        : handle (CreateEvent (nullptr, FALSE, FALSE, nullptr))
    {
    }

    HANDLE getHandle() const noexcept
    {
        return handle.get();
    }

private:
    struct Destructor
    {
        void operator() (HANDLE h) const
        {
            if (h != nullptr)
                CloseHandle (h);
        }
    };

    std::unique_ptr<std::remove_pointer_t<HANDLE>, Destructor> handle;
};


//==============================================================================
//
// LRU cache for geometry caching
//

template <typename KeyType, typename ValueType>
class LeastRecentlyUsedCache
{
public:
    void clear()
    {
        list.clear();
        map.clear();
    }

    bool contains(KeyType const& key)
    {
        return map.find(key) != map.end();
    }

    void set(KeyType const& key, ValueType const& value)
    {
        //
        // Replace existing entry
        //
        if (auto iterator = map.find(key); iterator != map.end())
        {
            iterator->second->second = value;
            return;
        }

        //
        // Add a new entry at the front of the LRU list
        //
        list.emplace_front(std::pair{ key, value });
        map.emplace(key, list.begin());
    }

    void set (KeyType const& key, ValueType&& value)
    {
        //
        // Replace existing entry
        //
        if (auto iterator = map.find(key); iterator != map.end())
        {
            iterator->second->second = value;
            return;
        }

        //
        // Add a new entry at the front of the LRU list
        //
        list.emplace_front(std::pair{ key, value });
        map.emplace(key, list.begin());
    }

    ValueType get (KeyType const& key)
    {
        if (auto iterator = map.find(key); iterator != map.end())
        {
            //
            // Found a match; move the key to the front of LRU list
            //
            list.splice(list.begin(), list, iterator->second);
            return iterator->second->second;
        }

        return {};
    }

    void erase (KeyType const& key)
    {
        if (auto iterator = map.find(key); iterator != map.end())
        {
            //
            // Remove the entry from the LRU list
            //
            list.erase(iterator->second);
            map.erase(iterator);
        }
    }

    ValueType back() const
    {
        if (list.empty())
        {
            return {};
        }

        return list.back().second;
    }

    void popBack()
    {
        if (list.empty())
        {
            return;
        }

        map.erase(list.back().first);
        list.pop_back();
    }

    auto size() const noexcept
    {
        jassert(map.size() == list.size());
        return map.size();
    }

private:
    using ListType = std::list<std::pair<KeyType, ValueType>>;
    ListType list;
    std::unordered_map<KeyType, typename ListType::iterator> map;
};

#if JUCE_UNIT_TESTS

class LeastRecentlyUsedCacheTests final : public UnitTest
{
public:
    LeastRecentlyUsedCacheTests()
        : UnitTest("LeastRecentlyUsedCache", UnitTestCategories::containers)
    {}

    void runTest() override
    {
        beginTest("LeastRecentlyUsedCache");

        {
            LeastRecentlyUsedCache<int, String> cache;
            int numTestStrings = 100;
            std::vector<int> hashes;
            for (int i = 0; i < numTestStrings; ++i)
            {
                String testString = "String " + String{ i };
                auto hash = testString.hashCode();
                hashes.emplace_back(hash);
                cache.set(hash, testString);
            }

            for (int i = 0; i < 1000; ++i)
            {
                auto hashIndex = getRandom().nextInt((int)hashes.size());
                auto hash = hashes[static_cast<size_t>(hashIndex)];

                auto value = cache.get(hash);
                expect(value.isNotEmpty());
                expect(value == "String " + String{ hashIndex });
            }

            for (int i = 0; i < 1000; ++i)
            {
                auto hashIndex = getRandom().nextInt((int)hashes.size());
                auto hash = hashes[static_cast<size_t>(hashIndex)];

                cache.erase(hash);
                expect(!cache.contains(hash));
            }
        }

        {
            LeastRecentlyUsedCache<int, float> cache;

            for (int i = 0; i < 10; ++i)
            {
                cache.set(i, static_cast<float>(i) * -1.0f);
            }

            for (int i = 9; i >= 0; --i)
            {
                auto value = cache.get(i);
                expect(cache.contains(i));
                expect(approximatelyEqual(value, static_cast<float>(i) * -1.0f));
            }

            for (int i = 9; i >= 0; --i)
            {
                auto backValue = cache.back();
                auto expectedValue = static_cast<float>(i) * -1.0f;
                expect(cache.contains(i));
                expect(approximatelyEqual(backValue, expectedValue));

                cache.popBack();
            }

            expect(cache.size() == 0);
        }
    }
};

static LeastRecentlyUsedCacheTests leastRecentlyUsedCacheTests;

#endif

} // namespace direct2d

#if JUCE_ETW_TRACELOGGING

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wgnu-zero-variadic-macro-arguments")
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wmissing-prototypes")
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wc++98-compat-extra-semi")
TRACELOGGING_DEFINE_PROVIDER (JUCE_ETW_TRACELOGGING_PROVIDER_HANDLE,
                              "JuceEtwTraceLogging",
                              // {6A612E78-284D-4DDB-877A-5F521EB33132}
                              (0x6a612e78, 0x284d, 0x4ddb, 0x87, 0x7a, 0x5f, 0x52, 0x1e, 0xb3, 0x31, 0x32));
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

ETWEventProvider::ETWEventProvider()
{
    auto hr = TraceLoggingRegister (JUCE_ETW_TRACELOGGING_PROVIDER_HANDLE);
    ignoreUnused (hr);
    jassert (SUCCEEDED (hr));
}

ETWEventProvider::~ETWEventProvider()
{
    TraceLoggingUnregister (JUCE_ETW_TRACELOGGING_PROVIDER_HANDLE);
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif

} // namespace juce
