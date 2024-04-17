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
#include "juce_ETW_windows.h"
#include "juce_DirectX_windows.h"
#include "juce_Direct2DHelpers_windows.cpp"

#endif

namespace juce
{
    namespace direct2d
    {

        //==============================================================================
        //
        // Device context and transform
        //

        struct DeviceContext
        {
            HRESULT createHwndRenderTarget(HWND hwnd)
            {
                HRESULT hr = S_OK;

                if (hwndRenderTarget == nullptr)
                {
                    SharedResourcePointer<DirectX> directX;

                    D2D1_SIZE_U size{ 1, 1 };

                    D2D1_RENDER_TARGET_PROPERTIES renderTargetProps{};
                    renderTargetProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
                    renderTargetProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;

                    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRenderTargetProps{};
                    hwndRenderTargetProps.hwnd = hwnd;
                    hwndRenderTargetProps.pixelSize = size;
                    hwndRenderTargetProps.presentOptions = D2D1_PRESENT_OPTIONS_IMMEDIATELY | D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS;
                    hr = directX->direct2D.getFactory()->CreateHwndRenderTarget(&renderTargetProps,
                        &hwndRenderTargetProps,
                        hwndRenderTarget.resetAndGetPointerAddress());
                }

                return hr;
            }

            void resetTransform()
            {
                context->SetTransform(D2D1::IdentityMatrix());
                transform = {};
            }

            void setTransform(AffineTransform newTransform)
            {
                context->SetTransform(transformToMatrix(newTransform));
                transform = newTransform;
            }

            void release()
            {
                hwndRenderTarget = nullptr;
                context = nullptr;
            }

            ComSmartPtr<ID2D1DeviceContext1> context;
            ComSmartPtr<ID2D1HwndRenderTarget> hwndRenderTarget;
            AffineTransform                 transform;
        };

        //==============================================================================
        //
        // Direct2D bitmap
        //

        struct Metrics;
        class Direct2DBitmap
        {
        public:
            static Direct2DBitmap fromImage(Image const& image, ID2D1DeviceContext1* deviceContext, Image::PixelFormat outputFormat)
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(MetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

                jassert(outputFormat == Image::ARGB || outputFormat == Image::SingleChannel);

                TRACE_LOG_D2D_PAINT_CALL(etw::createDirect2DBitmapFromImage, etw::graphicsKeyword);

                //
                // Calling Image::convertedToFormat could cause unchecked recursion since convertedToFormat
                // calls Graphics::drawImageAt which calls Direct2DGraphicsContext::drawImage which calls this function...
                //
                // Use a software image for the conversion instead so the Graphics::drawImageAt call doesn't go
                // through the Direct2D renderer
                //
                // Be sure to explicitly set the DPI to 96.0 for the image; otherwise it will default to the screen DPI
                // and may be scaled incorrectly
                //
                Image convertedImage = SoftwareImageType{}.convert(image).convertedToFormat(outputFormat);
                Image::BitmapData bitmapData{ convertedImage, Image::BitmapData::readWrite };

                D2D1_BITMAP_PROPERTIES1 bitmapProperties{};
                bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
                bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
                bitmapProperties.dpiX = USER_DEFAULT_SCREEN_DPI;
                bitmapProperties.dpiY = USER_DEFAULT_SCREEN_DPI;

                switch (outputFormat)
                {
                case Image::RGB:
                    bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
                    break;

                case Image::SingleChannel:
                    bitmapProperties.pixelFormat.format = DXGI_FORMAT_A8_UNORM;
                    break;

                case Image::UnknownFormat:
                    jassertfalse;
                    break;

                case Image::ARGB:
                default:
                    break;
                }

                D2D1_SIZE_U size = { (UINT32)image.getWidth(), (UINT32)image.getHeight() };

                Direct2DBitmap direct2DBitmap;
                deviceContext->CreateBitmap(size, bitmapData.data, (UINT32) bitmapData.lineStride, bitmapProperties, direct2DBitmap.bitmap.resetAndGetPointerAddress());
                return direct2DBitmap;
            }

            void createBitmap(ID2D1DeviceContext1* deviceContext,
                Image::PixelFormat format,
                D2D_SIZE_U size,
                int lineStride,
                float dpiScaleFactor,
                D2D1_BITMAP_OPTIONS options)
            {
                TRACE_LOG_D2D_PAINT_CALL(etw::createDirect2DBitmap, etw::graphicsKeyword);

                if (! bitmap)
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(MetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

#if JUCE_DEBUG
                    //
                    // Verify that the GPU can handle a bitmap of this size
                    //
                    // If you need a bitmap larger than this, you'll need to either split it up into multiple bitmaps
                    // or use a software image (see SoftwareImageType).
                    //
                    auto maxBitmapSize = deviceContext->GetMaximumBitmapSize();
                    jassert(size.width <= maxBitmapSize && size.height <= maxBitmapSize);
#endif

                    D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
                    bitmapProperties.dpiX = dpiScaleFactor * USER_DEFAULT_SCREEN_DPI;
                    bitmapProperties.dpiY = bitmapProperties.dpiX;
                    bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
                    bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    bitmapProperties.bitmapOptions = options;

                    switch (format)
                    {
                    case Image::RGB:
                        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
                        break;

                    case Image::SingleChannel:
                        bitmapProperties.pixelFormat.format = DXGI_FORMAT_A8_UNORM;
                        break;

                    case Image::UnknownFormat:
                        jassertfalse;
                        break;

                    case Image::ARGB:
                    default:
                        break;
                    }

                    deviceContext->CreateBitmap(
                        size,
                        nullptr,
                        (UINT32) lineStride,
                        bitmapProperties,
                        bitmap.resetAndGetPointerAddress());
                }
            }

            void setD2D1Bitmap(ID2D1Bitmap1* bitmap_)
            {
                bitmap = bitmap_;
            }

            ID2D1Bitmap1* getD2D1Bitmap() const noexcept
            {
                return bitmap;
            }

            void release()
            {
                bitmap = nullptr;
            }

        protected:
            ComSmartPtr<ID2D1Bitmap1> bitmap;
        };


        //==============================================================================
        //
        // Geometry caching
        //

        class GeometryCache
        {
        public:
            virtual ~GeometryCache()
            {
                release();
            }

            void release()
            {
                hashMap.clear();
            }

        protected:

            static float findGeometryFlatteningTolerance(float scaleFactor)
            {
                //
                // Could use D2D1::ComputeFlatteningTolerance here, but that requires defining NTDDI_VERSION and it doesn't do anything special.
                //
                // Direct2D default flattening tolerance is 0.25
                //
                return 0.25f / scaleFactor;
            }

            //==============================================================================
            //
            // Caching
            //
            struct CachedGeometryRealisation : public ReferenceCountedObject
            {
                CachedGeometryRealisation(uint64 hash_) :
                    hash(hash_)
                {
                }

                CachedGeometryRealisation(CachedGeometryRealisation const& other) :
                    hash(other.hash),
                    pathModificationCount(other.pathModificationCount),
                    geometryRealisation(other.geometryRealisation)
                {
                }

                CachedGeometryRealisation(CachedGeometryRealisation&& other)  noexcept :
                    hash(other.hash),
                    pathModificationCount(other.pathModificationCount),
                    geometryRealisation(other.geometryRealisation)
                {
                }

                ~CachedGeometryRealisation()
                {
                    clear();
                }

                void clear()
                {
                    hash = 0;
                    geometryRealisation = nullptr;
                }

                uint64 hash = 0;
                int pathModificationCount = 0;
                ComSmartPtr<ID2D1GeometryRealization> geometryRealisation;

                using Ptr = ReferenceCountedObjectPtr<CachedGeometryRealisation>;
            };

            class HashMap
            {
            public:
                ~HashMap()
                {
                    clear();
                }

                void clear()
                {
                    lruCache.clear();
                }

                auto size() const noexcept
                {
                    return lruCache.size();
                }

                CachedGeometryRealisation::Ptr getCachedGeometryRealisation(uint64 hash)
                {
                    trim();

                    if (auto entry = lruCache.get(hash))
                    {
                        return entry;
                    }

                    CachedGeometryRealisation::Ptr cachedGeometryRealisation = new CachedGeometryRealisation{ hash };
                    lruCache.set(hash, cachedGeometryRealisation);
                    return cachedGeometryRealisation;
                }

                void trim()
                {
                    //
                    // Remove any expired entries
                    //
                    while (lruCache.size() > maxNumCacheEntries)
                    {
                        TRACE_LOG_D2D_PAINT_CALL(etw::releaseGeometryRealization, etw::graphicsKeyword);

                        lruCache.popBack();
                    }
                }

            private:
                static int constexpr maxNumCacheEntries = 128;

                direct2d::LeastRecentlyUsedCache<uint64, CachedGeometryRealisation::Ptr> lruCache;
            } hashMap;
        };

        class FilledGeometryCache : public GeometryCache
        {
        public:
            ~FilledGeometryCache() override = default;

            ID2D1GeometryRealization* getGeometryRealisation(const Path& path,
                ID2D1Factory2* factory,
                ID2D1DeviceContext1* deviceContext,
                float dpiScaleFactor,
                [[maybe_unused]] int frameNumber,
                direct2d::Metrics* metrics)
            {
                if (path.getModificationCount() == 0 || !path.isCacheEnabled())
                {
                    return nullptr;
                }

                auto flatteningTolerance = findGeometryFlatteningTolerance(dpiScaleFactor);
                auto hash = calculatePathHash(path, flatteningTolerance);

                if (auto cachedGeometry = hashMap.getCachedGeometryRealisation(hash))
                {
                    if (cachedGeometry->geometryRealisation && cachedGeometry->pathModificationCount != path.getModificationCount())
                    {
                        cachedGeometry->geometryRealisation = nullptr;
                        cachedGeometry->pathModificationCount = 0;
                    }

                    if (cachedGeometry->geometryRealisation)
                    {
                        TRACE_LOG_D2D_PAINT_CALL(etw::filledGeometryRealizationCacheHit, frameNumber);
                    }

                    else
                    {
                        if (auto geometry = direct2d::pathToPathGeometry(factory, path, D2D1_FIGURE_BEGIN_FILLED, metrics))
                        {
                            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createFilledGRTime);

                            auto hr = deviceContext->CreateFilledGeometryRealization(geometry, flatteningTolerance, cachedGeometry->geometryRealisation.resetAndGetPointerAddress());
                            switch (hr)
                            {
                            case S_OK:
                                cachedGeometry->pathModificationCount = path.getModificationCount();

                                TRACE_LOG_D2D_PAINT_CALL(etw::filledGeometryRealizationCreated, frameNumber);
                                break;

                            case E_OUTOFMEMORY:
                                return nullptr;
                            }
                        }
                    }

                    return cachedGeometry->geometryRealisation;
                }

                return nullptr;
            }

        private:

            //==============================================================================
            //
            // Hashing
            //

            uint64 calculatePathHash(Path const& path, float flatteningTolerance)
            {
                return DefaultHashFunctions::generateHash(reinterpret_cast<uint8 const*>(&flatteningTolerance), sizeof(flatteningTolerance), path.getUniqueID());
            }

        };

        class StrokeGeometryCache : public GeometryCache
        {
        public:
            ~StrokeGeometryCache() override = default;

            ID2D1GeometryRealization* getGeometryRealisation(const Path& path,
                const PathStrokeType& strokeType,
                ID2D1Factory2* factory,
                ID2D1DeviceContext1* deviceContext,
                float xScaleFactor,
                float yScaleFactor,
                float dpiScaleFactor,
                [[maybe_unused]] int frameNumber,
                Metrics* metrics)
            {
                if (path.getModificationCount() == 0 || !path.isCacheEnabled())
                {
                    return nullptr;
                }

                auto flatteningTolerance = findGeometryFlatteningTolerance(dpiScaleFactor * (xScaleFactor + yScaleFactor) * 0.5f);
                auto hash = calculatePathHash(path, strokeType, flatteningTolerance, xScaleFactor, yScaleFactor);

                if (auto cachedGeometry = hashMap.getCachedGeometryRealisation(hash))
                {
                    if (cachedGeometry->geometryRealisation && cachedGeometry->pathModificationCount != path.getModificationCount())
                    {
                        cachedGeometry->geometryRealisation = nullptr;
                        cachedGeometry->pathModificationCount = 0;
                    }

                    if (cachedGeometry->geometryRealisation)
                    {
                        TRACE_LOG_D2D_PAINT_CALL(etw::strokedGeometryRealizationCacheHit, frameNumber);
                    }
                    else
                    {
                        auto transform = AffineTransform::scale(xScaleFactor, yScaleFactor, path.getBounds().getX(), path.getBounds().getY());
                        if (auto geometry = direct2d::pathToPathGeometry(factory, path, transform, D2D1_FIGURE_BEGIN_HOLLOW, metrics))
                        {
                            if (auto strokeStyle = direct2d::pathStrokeTypeToStrokeStyle(factory, strokeType))
                            {
                                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createStrokedGRTime);

                                //
                                // For stroked paths, the transform will affect the thickness of the path as well
                                // as the dimensions of the path. Divide the stroke thickness by the scale factor
                                // of the transform to compensate
                                //
                                auto hr = deviceContext->CreateStrokedGeometryRealization(geometry,
                                    flatteningTolerance,
                                    strokeType.getStrokeThickness(),
                                    strokeStyle,
                                    cachedGeometry->geometryRealisation.resetAndGetPointerAddress());

                                switch (hr)
                                {
                                case S_OK:
                                    cachedGeometry->pathModificationCount = path.getModificationCount();

                                    TRACE_LOG_D2D_PAINT_CALL(etw::strokedGeometryRealizationCreated, frameNumber);
                                    break;

                                case E_OUTOFMEMORY:
                                    return nullptr;
                                }
                            }
                        }
                    }

                    return cachedGeometry->geometryRealisation;
                }

                return nullptr;
            }

        private:

            uint64 calculatePathHash(Path const& path, PathStrokeType const& strokeType, float flatteningTolerance, float xScaleFactor, float yScaleFactor)
            {
                struct
                {
                    int xScaleFactor, yScaleFactor, flatteningTolerance, strokeThickness;
                    PathStrokeType::JointStyle jointStyle;
                    PathStrokeType::EndCapStyle endStyle;
                } extraHashData;

                //
                // Round the various float value to 128ths to avoid floating point hash differences
                //
                extraHashData.xScaleFactor = roundToInt(xScaleFactor * 128.0f);
                extraHashData.yScaleFactor = roundToInt(yScaleFactor * 128.0f);
                extraHashData.flatteningTolerance = roundToInt(flatteningTolerance * 128.0f);
                extraHashData.strokeThickness = roundToInt(strokeType.getStrokeThickness() * 128.0f);
                extraHashData.jointStyle = strokeType.getJointStyle();
                extraHashData.endStyle = strokeType.getEndStyle();

                return DefaultHashFunctions::generateHash(reinterpret_cast<uint8 const*>(&extraHashData), sizeof(extraHashData), path.getUniqueID());
            }
        };


        //==============================================================================
        //
        // Colour gradient caching
        //

        template<class BrushType>
        class ColourGradientCache
        {
        public:
            ~ColourGradientCache()
            {
                release();
            }

            void release()
            {
                gradientMap.clear();
            }

            void get(ColourGradient const&, ID2D1DeviceContext1*, ComSmartPtr<BrushType>&, Metrics* metrics);


        protected:
            uint64 calculateGradientHash(ColourGradient const& gradient)
            {
                return gradient.getHash();
            }

            void makeGradientStopCollection(ColourGradient const& gradient,
                ID2D1DeviceContext1* deviceContext,
                ComSmartPtr<ID2D1GradientStopCollection>& gradientStops,
                [[maybe_unused]] Metrics* metrics) const noexcept
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createGradientTime);

                const int numColors = gradient.getNumColours();

                HeapBlock<D2D1_GRADIENT_STOP> stops{ numColors };

                for (int i = numColors - 1; i >= 0; --i)
                {
                    stops[i].color = direct2d::colourToD2D(gradient.getColour(i));
                    stops[i].position = (FLOAT)gradient.getColourPosition(i);
                }

                deviceContext->CreateGradientStopCollection(stops.getData(), (UINT32)numColors, gradientStops.resetAndGetPointerAddress());
            }

            class HashMap
            {
            public:
                ~HashMap()
                {
                    clear();
                }

                void clear()
                {
                    lruCache.clear();
                }

                auto size() const noexcept
                {
                    return lruCache.size();
                }

                void getCachedBrush(uint64 hash, ComSmartPtr<BrushType>& brush)
                {
                    trim();

                    brush = lruCache.get(hash);
                }

                void store(uint64 hash, ComSmartPtr<BrushType>& brush)
                {
                    lruCache.set(hash, brush);
                }

                void trim()
                {
                    //
                    // Remove any expired entries
                    //
                    while (lruCache.size() > maxNumCacheEntries)
                    {
                        TRACE_LOG_D2D_PAINT_CALL(etw::releaseGradient, etw::graphicsKeyword);

                        lruCache.popBack();
                    }
                }

            private:
                static int constexpr maxNumCacheEntries = 128;

                direct2d::LeastRecentlyUsedCache<uint64, ComSmartPtr<BrushType>> lruCache;
            };

            HashMap gradientMap;
        };

        template<>
        void ColourGradientCache<ID2D1LinearGradientBrush>::get(ColourGradient const& gradient, ID2D1DeviceContext1* deviceContext, ComSmartPtr<ID2D1LinearGradientBrush>& brush, Metrics* metrics)
        {
            jassert(!gradient.isRadial);

            //
            // Already cached?
            //
            const auto p1 = gradient.point1;
            const auto p2 = gradient.point2;

            auto hash = calculateGradientHash(gradient);
            if (gradientMap.getCachedBrush(hash, brush); brush != nullptr)
            {
                return;
            }

            //
            // Make and store a new gradient brush
            //
            ComSmartPtr<ID2D1GradientStopCollection> gradientStops;
            makeGradientStopCollection(gradient, deviceContext, gradientStops, metrics);

            D2D1_BRUSH_PROPERTIES brushProps = { 1.0f, D2D1::IdentityMatrix() };
            const auto linearGradientBrushProperties = D2D1::LinearGradientBrushProperties({ p1.x, p1.y }, { p2.x, p2.y });

            deviceContext->CreateLinearGradientBrush(linearGradientBrushProperties,
                brushProps,
                gradientStops,
                brush.resetAndGetPointerAddress());

            gradientMap.store(hash, brush);
        }

        template<>
        void ColourGradientCache<ID2D1RadialGradientBrush>::get(ColourGradient const& gradient, ID2D1DeviceContext1* deviceContext, ComSmartPtr<ID2D1RadialGradientBrush>& brush, Metrics* metrics)
        {
            jassert(gradient.isRadial);

            //
            // Already cached?
            //
            const auto p1 = gradient.point1;
            const auto p2 = gradient.point2;
            const auto r = p1.getDistanceFrom(p2);

            auto hash = calculateGradientHash(gradient);
            if (gradientMap.getCachedBrush(hash, brush); brush != nullptr)
            {
                return;
            }

            //
            // Make and store a new gradient brush
            //
            ComSmartPtr<ID2D1GradientStopCollection> gradientStops;
            makeGradientStopCollection(gradient, deviceContext, gradientStops, metrics);

            D2D1_BRUSH_PROPERTIES brushProps = { 1.0F, D2D1::IdentityMatrix() };
            const auto radialGradientBrushProperties = D2D1::RadialGradientBrushProperties({ p1.x, p1.y }, {}, r, r);

            deviceContext->CreateRadialGradientBrush(radialGradientBrushProperties,
                brushProps,
                gradientStops,
                brush.resetAndGetPointerAddress());

            gradientMap.store(hash, brush);
        }

        //==============================================================================
        //
        // Sprite batch
        //

        class RectangleListSpriteBatch
        {
        public:
            RectangleListSpriteBatch() = default;
            ~RectangleListSpriteBatch()
            {
                release();
            }

            void release()
            {
                whiteRectangle = nullptr;
                spriteBatches.clear();
                destinations.free();
                destinationsCapacity = 0;
            }

            void fillRectangles(ID2D1DeviceContext1* deviceContext,
                const RectangleList<float>& rectangles,
                Colour const colour,
                std::function<Rectangle<float>(Rectangle<float> const r)> transformRectangle,
                [[maybe_unused]] Metrics* metrics)
            {
                if (rectangles.isEmpty())
                {
                    return;
                }

                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, spriteBatchTime)

                auto numRectanglesPainted = 0;
                while (numRectanglesPainted < rectangles.getNumRectangles())
                {
                    auto numRectanglesRemaining = rectangles.getNumRectangles() - numRectanglesPainted;
                    auto spriteBatchSize = isPowerOfTwo(numRectanglesRemaining) ? numRectanglesRemaining : (nextPowerOfTwo(numRectanglesRemaining) >> 1);

                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, spriteBatchSetupTime);

                        if (destinationsCapacity < (size_t)spriteBatchSize)
                        {
                            destinations.calloc(spriteBatchSize);
                            destinationsCapacity = (size_t)spriteBatchSize;
                        }

                        auto destination = destinations.getData();

                        for (int i = numRectanglesPainted; i < numRectanglesPainted + spriteBatchSize; ++i)
                        {
                            auto r = rectangles.getRectangle(i);
                            r = transformRectangle(r);
                            *destination = direct2d::rectangleToRectF(r);
                            ++destination;
                        }
                    }

                    if (!whiteRectangle)
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createSpriteSourceTime);

                        auto hr = deviceContext->CreateCompatibleRenderTarget(D2D1_SIZE_F{ (float)rectangleSize, (float)rectangleSize },
                            D2D1_SIZE_U{ rectangleSize, rectangleSize },
                            D2D1_PIXEL_FORMAT{ DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
                            whiteRectangle.resetAndGetPointerAddress());
                        if (FAILED(hr))
                        {
                            return;
                        }

                        whiteRectangle->BeginDraw();
                        whiteRectangle->Clear(D2D1_COLOR_F{ 1.0f, 1.0f, 1.0f, 1.0f });
                        whiteRectangle->EndDraw();
                    }

                    ComSmartPtr<ID2D1Bitmap> bitmap;
                    if (auto hr = whiteRectangle->GetBitmap(bitmap.resetAndGetPointerAddress()); SUCCEEDED(hr))
                    {
                        ComSmartPtr<ID2D1DeviceContext3> deviceContext3;
                        if (hr = deviceContext->QueryInterface<ID2D1DeviceContext3>(deviceContext3.resetAndGetPointerAddress()); SUCCEEDED(hr))
                        {
                            auto d2dColour = direct2d::colourToD2D(colour);

                            auto spriteBatch = spriteBatches.get((uint32)spriteBatchSize);
                            if (!spriteBatch)
                            {
                                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, createSpriteBatchTime);

                                if (hr = deviceContext3->CreateSpriteBatch(spriteBatch.resetAndGetPointerAddress()); FAILED(hr))
                                {
                                    return;
                                }

                                spriteBatches.set((uint32)spriteBatchSize, spriteBatch);
                            }

                            auto setCount = jmin((uint32)spriteBatchSize, spriteBatch->GetSpriteCount());
                            auto addCount = (uint32)spriteBatchSize > setCount ? (uint32)spriteBatchSize - setCount : 0;

                            if (setCount != 0)
                            {
                                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, setSpritesTime);

                                spriteBatch->SetSprites(0, setCount, destinations.getData(), nullptr, &d2dColour, nullptr, sizeof(D2D1_RECT_F), 0, 0, 0);
                            }

                            if (addCount != 0)
                            {
                                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, addSpritesTime);

                                spriteBatch->AddSprites(addCount, destinations.getData() + setCount, nullptr, &d2dColour, nullptr, sizeof(D2D1_RECT_F), 0, 0, 0);
                            }

                            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawSpritesTime);

                            deviceContext3->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                            deviceContext3->DrawSpriteBatch(spriteBatch, bitmap);
                            deviceContext3->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                        }
                    }

                    numRectanglesPainted += spriteBatchSize;
                }

                while (spriteBatches.size() > maxCacheSize)
                {
                    spriteBatches.popBack();
                }
            }

        private:
            static constexpr uint32 rectangleSize = 32;
            ComSmartPtr<ID2D1BitmapRenderTarget> whiteRectangle;
            HeapBlock<D2D1_RECT_F> destinations;
            size_t destinationsCapacity = 0;
            static constexpr size_t maxCacheSize = 8;
            direct2d::LeastRecentlyUsedCache<uint32, ComSmartPtr<ID2D1SpriteBatch>> spriteBatches;
        };

        //==============================================================================
        //
        // Device resources
        //

        class DeviceResources
        {
        public:
            DeviceResources() = default;

            ~DeviceResources()
            {
                release();
            }

            //
            // Create a Direct2D device context for a DXGI adapter
            //
            HRESULT create(DirectX::DXGI::Adapter::Ptr adapter, double dpiScalingFactor)
            {
                HRESULT hr = S_OK;

                jassert(adapter);

                if (adapter->direct2DDevice == nullptr)
                {
                    SharedResourcePointer<DirectX> directX;
                    if (hr = adapter->createDirect2DResources(directX->direct2D.getFactory()); FAILED(hr)) return hr;
                }

                if (deviceContext.context == nullptr)
                {
                    hr = adapter->direct2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
                        deviceContext.context.resetAndGetPointerAddress());
                    if (FAILED(hr)) return hr;
                }

                deviceContext.context->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
                deviceContext.context->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

                float dpi = (float)(USER_DEFAULT_SCREEN_DPI * dpiScalingFactor);
                deviceContext.context->SetDpi(dpi, dpi);

                if (colourBrush == nullptr)
                {
                    hr = deviceContext.context->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f),
                        colourBrush.resetAndGetPointerAddress());
                    jassertquiet(SUCCEEDED(hr));
                }

                if (rectangleListSpriteBatch == nullptr)
                {
                    ComSmartPtr<ID2D1DeviceContext3> deviceContext3;
                    if (hr = deviceContext.context->QueryInterface<ID2D1DeviceContext3>(deviceContext3.resetAndGetPointerAddress()); SUCCEEDED(hr))
                    {
                        rectangleListSpriteBatch = std::make_unique<RectangleListSpriteBatch>();
                    }
                }

                return hr;
            }

            void release()
            {
                rectangleListSpriteBatch = nullptr;
                linearGradientCache.release();
                radialGradientCache.release();
                filledGeometryCache.release();
                strokedGeometryCache.release();
                colourBrush = nullptr;
                deviceContext.release();
            }

            bool canPaint(DirectX::DXGI::Adapter::Ptr adapter)
            {
                return adapter->direct2DDevice != nullptr && deviceContext.context != nullptr && colourBrush != nullptr;
            }

            DeviceContext                     deviceContext;
            ComSmartPtr<ID2D1SolidColorBrush> colourBrush;
            FilledGeometryCache               filledGeometryCache;
            StrokeGeometryCache               strokedGeometryCache;
            ColourGradientCache<ID2D1LinearGradientBrush> linearGradientCache;
            ColourGradientCache<ID2D1RadialGradientBrush> radialGradientCache;
            std::unique_ptr<RectangleListSpriteBatch> rectangleListSpriteBatch;
        };

        //==============================================================================
        //
        // Swap chain
        //

        class SwapChain
        {
        public:
            SwapChain() = default;

            ~SwapChain()
            {
                release();
            }

            HRESULT create(HWND hwnd, Rectangle<int> size, DirectX::DXGI::Adapter::Ptr adapter)
            {
                if (!chain && hwnd)
                {
                    SharedResourcePointer<DirectX> directX;
                    auto dxgiFactory = directX->dxgi.getFactory();

                    if (dxgiFactory == nullptr || adapter->direct3DDevice == nullptr)
                    {
                        return E_FAIL;
                    }

                    HRESULT hr = S_OK;

                    buffer = nullptr;
                    chain = nullptr;

                    //
                    // Make the waitable swap chain
                    //
                    // Create the swap chain with premultiplied alpha support for transparent windows
                    //
                    DXGI_SWAP_CHAIN_DESC1 swapChainDescription = {};
                    swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
                    swapChainDescription.Width = (UINT) size.getWidth();
                    swapChainDescription.Height = (UINT) size.getHeight();
                    swapChainDescription.SampleDesc.Count = 1;
                    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    swapChainDescription.BufferCount = bufferCount;
                    swapChainDescription.SwapEffect = swapEffect;
                    swapChainDescription.Flags = swapChainFlags;

                    swapChainDescription.Scaling = DXGI_SCALING_STRETCH;
                    swapChainDescription.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
                    hr = dxgiFactory->CreateSwapChainForComposition(adapter->direct3DDevice,
                        &swapChainDescription,
                        nullptr,
                        chain.resetAndGetPointerAddress());
                    jassert(SUCCEEDED(hr));

                    if (SUCCEEDED(hr))
                    {
                        //
                        // Get the waitable swap chain presentation event and set the maximum frame latency
                        //
                        ComSmartPtr<IDXGISwapChain2> chain2;
                        chain.QueryInterface<IDXGISwapChain2>(chain2);
                        if (chain2)
                        {
                            swapChainEvent.emplace(chain2->GetFrameLatencyWaitableObject());
                            if (swapChainEvent->getHandle() == INVALID_HANDLE_VALUE)
                                return E_NOINTERFACE;

                            hr = chain2->SetMaximumFrameLatency(1);
                            if (SUCCEEDED(hr))
                            {
                                state = State::chainAllocated;
                            }
                        }
                    }
                    else
                    {
                        return E_NOINTERFACE;
                    }

                    return hr;
                }

                return S_OK;
            }

            HRESULT createBuffer(ID2D1DeviceContext* const deviceContext)
            {
                if (deviceContext && chain && !buffer)
                {
                    ComSmartPtr<IDXGISurface> surface;
                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wlanguage-extension-token")
                    auto hr = chain->GetBuffer(0, __uuidof (surface), reinterpret_cast<void**> (surface.resetAndGetPointerAddress()));
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                    if (SUCCEEDED(hr))
                    {
                        D2D1_BITMAP_PROPERTIES1 bitmapProperties = {};
                        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
                        bitmapProperties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
                        bitmapProperties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;

                        hr = deviceContext->CreateBitmapFromDxgiSurface(surface, bitmapProperties, buffer.resetAndGetPointerAddress());
                        jassert(SUCCEEDED(hr));

                        if (SUCCEEDED(hr))
                        {
                            state = State::bufferAllocated;
                        }
                    }

                    return hr;
                }

                return S_OK;
            }

            void release()
            {
                buffer = nullptr;
                chain = nullptr;
                state = State::idle;
            }

            bool canPaint()
            {
                return chain != nullptr && buffer != nullptr && state >= State::bufferAllocated;
            }

            HRESULT resize(Rectangle<int> newSize, float dpiScalingFactor, ID2D1DeviceContext* const deviceContext)
            {
                if (chain)
                {
                    auto scaledSize = newSize * dpiScalingFactor;
                    scaledSize =
                        scaledSize.getUnion({ Direct2DGraphicsContext::minFrameSize, Direct2DGraphicsContext::minFrameSize })
                        .getIntersection({ Direct2DGraphicsContext::maxFrameSize, Direct2DGraphicsContext::maxFrameSize });

                    buffer = nullptr;
                    state = State::chainAllocated;

                    auto dpi = USER_DEFAULT_SCREEN_DPI * dpiScalingFactor;
                    deviceContext->SetDpi(dpi, dpi);

                    auto hr = chain->ResizeBuffers(0, (UINT) scaledSize.getWidth(), (UINT) scaledSize.getHeight(), DXGI_FORMAT_B8G8R8A8_UNORM, swapChainFlags);
                    if (SUCCEEDED(hr))
                    {
                        hr = createBuffer(deviceContext);
                    }

                    if (FAILED(hr))
                    {
                        release();
                    }

                    return hr;
                }

                return E_FAIL;
            }

            Rectangle<int> getSize() const
            {
                if (buffer)
                {
                    auto size = buffer->GetPixelSize();
                    return { (int)size.width, (int)size.height };
                }

                return {};
            }

            DXGI_SWAP_EFFECT const                     swapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
            UINT const                                 bufferCount = 2;
            uint32 const                               swapChainFlags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
            uint32 const                               presentSyncInterval = 1;
            uint32 const                               presentFlags = 0;
            ComSmartPtr<IDXGISwapChain1>               chain;
            ComSmartPtr<ID2D1Bitmap1>                  buffer;
            std::optional<ScopedEvent> swapChainEvent;

            enum class State
            {
                idle,
                chainAllocated,
                bufferAllocated,
                bufferFilled
            };
            State state = State::idle;
        };

        //==============================================================================
        //
        // DirectComposition
        //
        // Using DirectComposition enables transparent windows and smoother window
        // resizing
        //
        // This class builds a simple DirectComposition tree that ultimately contains
        // the swap chain
        //

#define JUCE_EARLY_EXIT(expr) \
    JUCE_BLOCK_WITH_FORCED_SEMICOLON ( if (const auto hr = (expr); ! SUCCEEDED (hr)) return hr; )

        class CompositionTree
        {
        public:
            HRESULT create(IDXGIDevice* const dxgiDevice, HWND hwnd, IDXGISwapChain1* const swapChain)
            {
                if (compositionDevice != nullptr)
                    return S_OK;

                if (dxgiDevice == nullptr)
                    return S_FALSE;

                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE("-Wlanguage-extension-token")
                JUCE_EARLY_EXIT(DCompositionCreateDevice(dxgiDevice,
                    __uuidof (IDCompositionDevice),
                    reinterpret_cast<void**> (compositionDevice.resetAndGetPointerAddress())));
                JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                JUCE_EARLY_EXIT(compositionDevice->CreateTargetForHwnd(hwnd, FALSE, compositionTarget.resetAndGetPointerAddress()));
                JUCE_EARLY_EXIT(compositionDevice->CreateVisual(compositionVisual.resetAndGetPointerAddress()));
                JUCE_EARLY_EXIT(compositionTarget->SetRoot(compositionVisual));
                JUCE_EARLY_EXIT(compositionVisual->SetContent(swapChain));
                JUCE_EARLY_EXIT(compositionDevice->Commit());
                return S_OK;
            }

            void release()
            {
                compositionVisual = nullptr;
                compositionTarget = nullptr;
                compositionDevice = nullptr;
            }

            bool canPaint() const
            {
                return compositionVisual != nullptr;
            }

        private:
            ComSmartPtr<IDCompositionDevice> compositionDevice;
            ComSmartPtr<IDCompositionTarget> compositionTarget;
            ComSmartPtr<IDCompositionVisual> compositionVisual;
        };

#undef JUCE_EARLY_EXIT

    } // namespace direct2d

} // namespace juce
