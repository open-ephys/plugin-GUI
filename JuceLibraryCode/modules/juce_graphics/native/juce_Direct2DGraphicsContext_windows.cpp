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
#include "juce_ETW_windows.h"

#endif

namespace juce
{
#if JUCE_DIRECT2D_METRICS
    JUCE_IMPLEMENT_SINGLETON(direct2d::MetricsHub)
#endif

    struct Direct2DGraphicsContext::SavedState
    {
    private:
        Direct2DGraphicsContext& owner;

        //==============================================================================
        //
        // PushedLayer represents a Direct2D clipping or transparency layer
        //
        // D2D layers have to be pushed into the device context. Every push has to be
        // matched with a pop.
        //
        // D2D has special layers called "axis aligned clip layers" which clip to an
        // axis-aligned rectangle. Pushing an axis-aligned clip layer must be matched
        // with a call to deviceContext->PopAxisAlignedClip() in the reverse order
        // in which the layers were pushed.
        //
        // So if the pushed layer stack is built like this:
        //
        // PushLayer()
        // PushLayer()
        // PushAxisAlignedClip()
        // PushLayer()
        //
        // the layer stack must be popped like this:
        //
        // PopLayer()
        // PopAxisAlignedClip()
        // PopLayer()
        // PopLayer()
        //
        // PushedLayer, PushedAxisAlignedClipLayer, and LayerPopper all exist just to unwind the
        // layer stack accordingly.
        //
        //using PopLayer = void (*) (ID2D1DeviceContext*);
        //std::stack<PopLayer> pushedLayers;
        enum
        {
            popLayerFlag,
            popAxisAlignedLayerFlag
        };
        std::vector<int> pushedLayers;

        ID2D1Brush* currentBrush = nullptr;
        ComSmartPtr<ID2D1SolidColorBrush>& colourBrush; // reference to shared colour brush
        ComSmartPtr<ID2D1BitmapBrush>            bitmapBrush;
        ComSmartPtr<ID2D1LinearGradientBrush>    linearGradient;
        ComSmartPtr<ID2D1RadialGradientBrush>    radialGradient;

    public:
        //
        // Constructor for first stack entry
        //
        SavedState(Direct2DGraphicsContext& owner_, Rectangle<int> frameSize_, ComSmartPtr<ID2D1SolidColorBrush>& colourBrush_, DirectX::DXGI::Adapter::Ptr& adapter_, direct2d::DeviceResources& deviceResources_)
            : owner(owner_),
            colourBrush(colourBrush_),
            adapter(adapter_),
            deviceResources(deviceResources_),
            deviceSpaceClipList(frameSize_)
        {
            currentBrush = colourBrush;

            pushedLayers.reserve(32);
        }

        //
        // Constructor for subsequent entries
        //
        SavedState(SavedState const* const previousState_) :
            owner(previousState_->owner),
            currentBrush(previousState_->currentBrush),
            colourBrush(previousState_->colourBrush),
            bitmapBrush(previousState_->bitmapBrush),
            linearGradient(previousState_->linearGradient),
            radialGradient(previousState_->radialGradient),
            currentTransform(previousState_->currentTransform),
            adapter(previousState_->adapter),
            deviceResources(previousState_->deviceResources),
            deviceSpaceClipList(previousState_->deviceSpaceClipList),
            font(previousState_->font),
            fillType(previousState_->fillType),
            interpolationMode(previousState_->interpolationMode)
        {
            pushedLayers.reserve(32);
        }

        ~SavedState()
        {
            jassert(pushedLayers.empty());
            clearFill();
        }

        void pushLayer(const D2D1_LAYER_PARAMETERS& layerParameters)
        {
            //
            // Clipping and transparency are all handled by pushing Direct2D layers. The SavedState creates an internal stack
            // of Layer objects to keep track of how many layers need to be popped.
            //
            // Pass nullptr for the PushLayer layer parameter to allow Direct2D to manage the layers (Windows 8 or later)
            //
            deviceResources.deviceContext.context->PushLayer(layerParameters, nullptr);
            pushedLayers.emplace_back(popLayerFlag);
        }

        void pushGeometryClipLayer(ComSmartPtr<ID2D1Geometry> geometry)
        {
            if (geometry != nullptr)
            {
                pushLayer(D2D1::LayerParameters(D2D1::InfiniteRect(), geometry));
            }
        }

        void pushTransformedRectangleGeometryClipLayer(ComSmartPtr<ID2D1RectangleGeometry> geometry, AffineTransform const& transform)
        {
            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(owner.metrics, pushGeometryLayerTime)

            jassert(geometry != nullptr);
            auto layerParameters = D2D1::LayerParameters(D2D1::InfiniteRect(), geometry);
            layerParameters.maskTransform = direct2d::transformToMatrix(transform);
            pushLayer(layerParameters);
        }

        void pushAliasedAxisAlignedClipLayer(Rectangle<int> r)
        {
            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(owner.metrics, pushAliasedAxisAlignedLayerTime)

            deviceResources.deviceContext.context->PushAxisAlignedClip(direct2d::rectangleToRectF(r), D2D1_ANTIALIAS_MODE_ALIASED);
            pushedLayers.emplace_back(popAxisAlignedLayerFlag);
        }

        void pushTransparencyLayer(float opacity)
        {
            pushLayer({ D2D1::InfiniteRect(), nullptr, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE, D2D1::IdentityMatrix(), opacity, {}, {} });
        }

        void popLayers()
        {
            while (! pushedLayers.empty())
            {
                popTopLayer();
            }
        }

        void popTopLayer()
        {
            if (! pushedLayers.empty())
            {
                if (pushedLayers.back() == popLayerFlag)
                {
                    deviceResources.deviceContext.context->PopLayer();
                }
                else
                {
                    deviceResources.deviceContext.context->PopAxisAlignedClip();
                }

                pushedLayers.pop_back();
            }
        }

        void setFont(const Font& newFont)
        {
            font = newFont;
        }

        void setOpacity(float newOpacity)
        {
            fillType.setOpacity(newOpacity);
        }

        void clearFill()
        {
            linearGradient = nullptr;
            radialGradient = nullptr;
            bitmapBrush = nullptr;
            currentBrush = nullptr;
        }

        //
        // Translate a JUCE FillType to a Direct2D brush
        //
        void updateCurrentBrush()
        {
            if (fillType.isColour())
            {
                //
                // Reuse the same colour brush
                //
                currentBrush = (ID2D1Brush*)colourBrush;
            }
            else if (fillType.isTiledImage())
            {
                if (fillType.image.isNull())
                {
                    return;
                }

                ComSmartPtr<ID2D1Bitmap1> d2d1Bitmap;

                if (auto direct2DPixelData = dynamic_cast<Direct2DPixelData*> (fillType.image.getPixelData()))
                {
                    d2d1Bitmap = direct2DPixelData->getAdapterD2D1Bitmap(adapter);
                }

                if (! d2d1Bitmap || d2d1Bitmap->GetPixelFormat().format != DXGI_FORMAT_B8G8R8A8_UNORM)
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(direct2d::MetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

                    d2d1Bitmap = direct2d::Direct2DBitmap::fromImage(fillType.image, deviceResources.deviceContext.context, Image::ARGB).getD2D1Bitmap();
                }

                if (d2d1Bitmap)
                {
                    D2D1_BRUSH_PROPERTIES brushProps = { fillType.getOpacity(), direct2d::transformToMatrix(fillType.transform) };
                    auto                  bmProps = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);
                    auto hr = deviceResources.deviceContext.context->CreateBitmapBrush(d2d1Bitmap,
                        bmProps,
                        brushProps,
                        bitmapBrush.resetAndGetPointerAddress());
                    jassert(SUCCEEDED(hr));
                    if (SUCCEEDED(hr))
                    {
                        currentBrush = bitmapBrush;
                    }
                }
            }
            else if (fillType.isGradient())
            {
                if (fillType.gradient->isRadial)
                {
                    deviceResources.radialGradientCache.get(*fillType.gradient, deviceResources.deviceContext.context, radialGradient, owner.metrics.get());
                    currentBrush = radialGradient;
                }
                else
                {
                    deviceResources.linearGradientCache.get(*fillType.gradient, deviceResources.deviceContext.context, linearGradient, owner.metrics.get());
                    currentBrush = linearGradient;
                }
            }

            updateColourBrush();
        }

        void updateColourBrush()
        {
            if (colourBrush && fillType.isColour())
            {
                auto colour = direct2d::colourToD2D(fillType.colour);
                colourBrush->SetColor(colour);
            }
        }

        enum BrushTransformFlags
        {
            noTransforms = 0,
            applyWorldTransform = 1,
            applyInverseWorldTransform = 2,
            applyFillTypeTransform = 4,
            applyWorldAndFillTypeTransforms = applyFillTypeTransform | applyWorldTransform
        };
        ID2D1Brush* getBrush(int flags = applyWorldAndFillTypeTransforms)
        {
            if (fillType.isInvisible())
            {
                return nullptr;
            }

            if (fillType.isGradient() || fillType.isTiledImage())
            {
                Point<float> translation;
                AffineTransform transform;

                if (flags != 0)
                {
                    if ((flags & BrushTransformFlags::applyWorldTransform) != 0)
                    {
                        if (currentTransform.isOnlyTranslated)
                        {
                            translation = currentTransform.offset.toFloat();
                        }
                        else
                        {
                            transform = currentTransform.getTransform();
                        }
                    }

                    if ((flags & BrushTransformFlags::applyFillTypeTransform) != 0)
                    {
                        if (fillType.transform.isOnlyTranslation())
                        {
                            translation += Point<float>(fillType.transform.getTranslationX(), fillType.transform.getTranslationY());
                        }
                        else
                        {
                            transform = transform.followedBy(fillType.transform);
                        }
                    }

                    if ((flags & BrushTransformFlags::applyInverseWorldTransform) != 0)
                    {
                        if (currentTransform.isOnlyTranslated)
                        {
                            translation -= currentTransform.offset.toFloat();
                        }
                        else
                        {
                            transform = transform.followedBy(currentTransform.getTransform().inverted());
                        }
                    }
                }

                if (fillType.isGradient())
                {
                    auto p1 = fillType.gradient->point1;
                    auto p2 = fillType.gradient->point2;
                    p1 += translation;
                    p2 += translation;

                    if (fillType.gradient->isRadial)
                    {
                        radialGradient->SetCenter({ p1.x, p1.y });
                        float radius = p2.getDistanceFrom(p1);
                        radialGradient->SetRadiusX(radius);
                        radialGradient->SetRadiusY(radius);
                    }
                    else
                    {
                        linearGradient->SetStartPoint({ p1.x, p1.y });
                        linearGradient->SetEndPoint({ p2.x, p2.y });
                    }
                }

                currentBrush->SetTransform(direct2d::transformToMatrix(transform));
                currentBrush->SetOpacity(fillType.getOpacity());
            }

            return currentBrush;
        }

        bool doesRectangleIntersectClipList(Rectangle<int> r) const noexcept
        {
            return deviceSpaceClipList.intersects(r);
        }

        bool doesRectangleIntersectClipList(Rectangle<float> r) const noexcept
        {
            for (auto const& clipRectangle : deviceSpaceClipList)
            {
                if (clipRectangle.toFloat().intersects(r))
                {
                    return true;
                }
            }

            return false;
        }

        struct TranslationOrTransform : public RenderingHelpers::TranslationOrTransform
        {
            bool isAxisAligned() const noexcept
            {
                return isOnlyTranslated || (complexTransform.mat01 == 0.0f && complexTransform.mat10 == 0.0f);
            }

#if JUCE_DEBUG
            String toString()
            {
                String s;
                s << "Offset " << offset.toString() << newLine;
                s << "Transform " << complexTransform.mat00 << " " << complexTransform.mat01 << " " << complexTransform.mat02 << " / ";
                s << "          " << complexTransform.mat10 << " " << complexTransform.mat11 << " " << complexTransform.mat12 << newLine;
                return s;
            }
#endif
        } currentTransform;

        DirectX::DXGI::Adapter::Ptr& adapter;
        direct2d::DeviceResources& deviceResources;
        RectangleList<int> deviceSpaceClipList;

        Font font;

        FillType fillType;

        D2D1_INTERPOLATION_MODE interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SavedState)
    };

    //==============================================================================

    struct Direct2DGraphicsContext::Pimpl : public DirectX::DXGI::AdapterListener
    {
    protected:
        Direct2DGraphicsContext& owner;
        SharedResourcePointer<DirectX> directX;
        SharedResourcePointer<DirectWrite> directWrite;
        float                                   dpiScalingFactor = 1.0f;
        RectangleList<int> paintAreas;

        DirectX::DXGI::Adapter::Ptr adapter;
        direct2d::DeviceResources              deviceResources;

        std::stack<std::unique_ptr<Direct2DGraphicsContext::SavedState>> savedClientStates;

        virtual HRESULT prepare()
        {
            if (!deviceResources.canPaint(adapter))
            {
                if (auto hr = deviceResources.create(adapter, dpiScalingFactor); FAILED(hr))
                {
                    return hr;
                }
            }

            return S_OK;
        }

        virtual void teardown()
        {
            deviceResources.release();
        }

        virtual ID2D1Image* getDeviceContextTarget() = 0;

        virtual void updatePaintAreas() = 0;

        virtual bool checkPaintReady()
        {
            return deviceResources.canPaint(adapter);
        }

    public:
        Pimpl(Direct2DGraphicsContext& owner_, bool opaque_)
            : owner(owner_), opaque(opaque_)
        {
            setTargetAlpha(1.0f);

            directX->dxgi.adapters.listeners.add(this);
        }

        ~Pimpl() override
        {
            directX->dxgi.adapters.listeners.remove(this);

            popAllSavedStates();

            teardown();
        }

        void setTargetAlpha(float alpha)
        {
            backgroundColor = direct2d::colourToD2D(Colours::black.withAlpha(opaque ? targetAlpha : 0.0f));
            targetAlpha = alpha;
        }

        virtual void clearBackground()
        {
            deviceResources.deviceContext.context->Clear(backgroundColor);
        }

        virtual SavedState* startFrame()
        {
            prepare();

            //
            // Anything to paint?
            //
            updatePaintAreas();
            auto paintBounds = paintAreas.getBounds();
            if (!getFrameSize().intersects(paintBounds) || paintBounds.isEmpty())
            {
                return nullptr;
            }

            //
            // Is Direct2D ready to paint?
            //
            if (!checkPaintReady())
            {
                return nullptr;
            }

#if JUCE_DIRECT2D_METRICS
            owner.metrics->startFrame();
#endif

            TRACE_EVENT_INT_RECT_LIST(etw::startD2DFrame, owner.llgcFrameNumber, paintAreas, etw::direct2dKeyword)

            //
            // Init device context transform
            //
            deviceResources.deviceContext.resetTransform();

            //
            // Start drawing
            //
            deviceResources.deviceContext.context->SetTarget(getDeviceContextTarget());
            deviceResources.deviceContext.context->BeginDraw();

            //
            // Init the save state stack and return the first saved state
            //
            return pushFirstSavedState(paintBounds);
        }

        virtual HRESULT finishFrame()
        {
            //
            // Fully pop the state stack
            //
            popAllSavedStates();

            //
            // Finish drawing
            //
            // SetTarget(nullptr) so the device context doesn't hold a reference to the swap chain buffer
            //
            HRESULT hr = S_OK;
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(owner.metrics, endDrawDuration)

                SCOPED_TRACE_EVENT(etw::endDraw, owner.llgcFrameNumber, etw::direct2dKeyword);

                hr = deviceResources.deviceContext.context->EndDraw();
                deviceResources.deviceContext.context->SetTarget(nullptr);
            }

            jassert(SUCCEEDED(hr));

            if (FAILED(hr))
            {
                teardown();
            }

#if JUCE_DIRECT2D_METRICS
            owner.metrics->finishFrame();
#endif

            return hr;
        }

        virtual void setScaleFactor(float scale_)
        {
            dpiScalingFactor = scale_;
        }

        float getScaleFactor() const
        {
            return dpiScalingFactor;
        }

        SavedState* getCurrentSavedState() const
        {
            return savedClientStates.size() > 0 ? savedClientStates.top().get() : nullptr;
        }

        SavedState* pushFirstSavedState(Rectangle<int> initialClipRegion)
        {
            jassert(savedClientStates.size() == 0);

            savedClientStates.push(
                std::make_unique<SavedState>(owner, initialClipRegion, deviceResources.colourBrush, adapter, deviceResources));

            return getCurrentSavedState();
        }

        SavedState* pushSavedState()
        {
            jassert(savedClientStates.size() > 0);

            savedClientStates.push(std::make_unique<SavedState>(savedClientStates.top().get()));

            return getCurrentSavedState();
        }

        SavedState* popSavedState()
        {
            savedClientStates.top()->popLayers();
            savedClientStates.pop();

            return getCurrentSavedState();
        }

        void popAllSavedStates()
        {
            while (savedClientStates.size() > 0)
            {
                popSavedState();
            }
        }

        DirectX::DXGI::Adapter& getAdapter() const noexcept
        {
            return *adapter;
        }

        inline ID2D1DeviceContext1* getDeviceContext() const noexcept
        {
            return deviceResources.deviceContext.context;
        }

        auto const& getPaintAreas() const noexcept
        {
            return paintAreas;
        }

        virtual Rectangle<int> getFrameSize() = 0;

        void setDeviceContextTransform(AffineTransform transform)
        {
            deviceResources.deviceContext.setTransform(transform);
        }

        void resetDeviceContextTransform()
        {
            deviceResources.deviceContext.setTransform({});
        }

        auto getDirect2DFactory()
        {
            return directX->direct2D.getFactory();
        }

        auto getDirectWriteFactory()
        {
            return directWrite->getFactory();
        }

        auto getFontCollection()
        {
            return directWrite->getFontCollection();
        }

        auto& getFilledGeometryCache()
        {
            return deviceResources.filledGeometryCache;
        }

        auto& getStrokedGeometryCache()
        {
            return deviceResources.strokedGeometryCache;
        }

        void adapterCreated(DirectX::DXGI::Adapter::Ptr newAdapter) override
        {
            if (!adapter || adapter->uniqueIDMatches(newAdapter))
            {
                teardown();

                adapter = newAdapter;
            }
        }

        void adapterRemoved(DirectX::DXGI::Adapter::Ptr expiringAdapter) override
        {
            if (adapter && adapter->uniqueIDMatches(expiringAdapter))
            {
                teardown();

                adapter = nullptr;
            }
        }

        auto getRectangleListSpriteBatch() noexcept
        {
            return deviceResources.rectangleListSpriteBatch.get();
        }

        direct2d::DirectWriteGlyphRun       glyphRun;
        bool                                opaque = true;
        float                               targetAlpha = 1.0f;
        D2D1_COLOR_F                        backgroundColor{};

    private:
        HWND                                hwnd = nullptr;

#if JUCE_DIRECT2D_METRICS
        int64 paintStartTicks = 0;
#endif

        JUCE_DECLARE_WEAK_REFERENCEABLE(Pimpl)
    };

    //==============================================================================
    Direct2DGraphicsContext::Direct2DGraphicsContext() = default;

    Direct2DGraphicsContext::~Direct2DGraphicsContext() = default;

    bool Direct2DGraphicsContext::startFrame()
    {
        auto pimpl = getPimpl();

        if (currentState = pimpl->startFrame(); currentState != nullptr)
        {
            if (auto deviceContext = pimpl->getDeviceContext())
            {
                resetPendingClipList();

                clipToRectangleList(pimpl->getPaintAreas());

                //
                // Clear the buffer *after* setting the clip region
                //
                clearTargetBuffer();

                //
                // Init font & brush
                //
                setFont(currentState->font);
                currentState->updateCurrentBrush();
            }

            return true;
        }

        return false;
    }

    void Direct2DGraphicsContext::endFrame()
    {
        getPimpl()->finishFrame();

        currentState = nullptr;
    }

    void Direct2DGraphicsContext::setOrigin(Point<int> o)
    {
        applyPendingClipList();

        currentState->currentTransform.setOrigin(o);
    }

    void Direct2DGraphicsContext::addTransform(const AffineTransform& transform)
    {
        //
        // The pending clip list is based on the transform stored in currentState, so apply the pending clip list before adding the transform
        //
        applyPendingClipList();

        currentState->currentTransform.addTransform(transform);

        resetPendingClipList();
    }

    bool Direct2DGraphicsContext::clipToRectangle(const Rectangle<int>& r)
    {
        auto const& transform = currentState->currentTransform;
        auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

        SCOPED_TRACE_EVENT_INT_RECT(etw::clipToRectangle, llgcFrameNumber, r, etw::direct2dKeyword)

        //
        // The renderer needs to keep track of the aggregate clip rectangles in order to correctly report the
        // clip region to the caller. The renderer also needs to push Direct2D clip layers to the device context
        // to perform the actual clipping. The reported clip region will not necessarily match the Direct2D clip region
        // if the clip region is transformed, or the clip region is an image or a path.
        //
        // Pushing Direct2D clip layers is expensive and there's no need to clip until something is actually drawn.
        // So - pendingDeviceSpaceClipList is a list of the areas that need to actually be clipped. Each fill or
        // draw method then applies any pending clip areas before drawing.
        //
        // Also - calling ID2D1DeviceContext::SetTransform is expensive, so check the current transform to see
        // if the renderer can pre-transform the clip rectangle instead.
        //
        if (transform.isOnlyTranslated)
        {
            //
            // The current transform is only a translation, so save a few cycles by just adding the
            // offset instead of transforming the rectangle; the software renderer does something similar.
            //
            auto translatedR = r + transform.offset;
            deviceSpaceClipList.clipTo(translatedR);

            pendingDeviceSpaceClipList.clipTo(translatedR);
        }
        else if (transform.isAxisAligned())
        {
            //
            // The current transform will translate and scale the rectangle, but the clipped area will
            // still be axis aligned. Pre-transform the rectangle and clip to the transformed rectangle
            // instead of calling ID2D1DeviceContext::SetTransform
            //
            auto transformedR = transform.transformed(r);
            deviceSpaceClipList.clipTo(transformedR);

            pendingDeviceSpaceClipList.clipTo (transformedR);
        }
        else
        {
            deviceSpaceClipList = getPimpl()->getFrameSize();

            //
            // The current transform is too complex to pre-transform the rectangle, so just add the
            // rectangle to the clip list. The renderer will need to call ID2D1DeviceContext::SetTransform
            // before applying the clip layer.
            //
            pendingDeviceSpaceClipList.clipTo (r);
        }

        return !isClipEmpty();
    }

    bool Direct2DGraphicsContext::clipToRectangleList(const RectangleList<int>& newClipList)
    {
        SCOPED_TRACE_EVENT_INT_RECT_LIST(etw::clipToRectangleList, llgcFrameNumber, newClipList, etw::direct2dKeyword)

        auto const& transform = currentState->currentTransform;
        auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

        //
        // This works a lot like clipToRect
        //

        //
        // Just one rectangle?
        //
        if (newClipList.getNumRectangles() == 1)
        {
            return clipToRectangle(newClipList.getRectangle(0));
        }

        if (transform.isIdentity())
        {
            deviceSpaceClipList.clipTo(newClipList);

            pendingDeviceSpaceClipList.clipTo(newClipList);
        }
        else if (currentState->currentTransform.isOnlyTranslated)
        {
            RectangleList<int> offsetList(newClipList);
            offsetList.offsetAll(transform.offset);
            deviceSpaceClipList.clipTo(offsetList);

            pendingDeviceSpaceClipList.clipTo(offsetList);
        }
        else if (transform.isAxisAligned())
        {
            RectangleList<int> scaledList;

            for (auto& i : newClipList)
                scaledList.add(transform.transformed(i));

            deviceSpaceClipList.clipTo(scaledList);

            pendingDeviceSpaceClipList.clipTo(scaledList);
        }
        else
        {
            deviceSpaceClipList = getPimpl()->getFrameSize();

            pendingDeviceSpaceClipList.clipTo(newClipList);
        }

        return !isClipEmpty();
    }

    void Direct2DGraphicsContext::excludeClipRectangle(const Rectangle<int>& userSpaceExcludedRectangle)
    {
        auto const& transform = currentState->currentTransform;
        auto& deviceSpaceClipList = currentState->deviceSpaceClipList;

        SCOPED_TRACE_EVENT_INT_RECT(etw::excludeClipRectangle, llgcFrameNumber, userSpaceExcludedRectangle, etw::direct2dKeyword)

        // Remove the excluded rectangle from the clip list
        // As always, try to avoid setting ID2D1DeviceContext::SetTransform
        auto frameSize = getPimpl()->getFrameSize();

        if (transform.isOnlyTranslated)
        {
            // Just a translation; pre-translate the exclusion area
            auto translatedR = transform.translated(userSpaceExcludedRectangle.toFloat()).toNearestIntEdges();
            if (!translatedR.contains(frameSize))
            {
                deviceSpaceClipList.subtract(translatedR);
                pendingDeviceSpaceClipList.subtract(translatedR);
            }
        }
        else if (transform.isAxisAligned())
        {
            // Just a scale + translation; pre-transform the exclusion area
            auto transformedR = transform.transformed(userSpaceExcludedRectangle.toFloat()).toNearestIntEdges();
            if (!transformedR.contains(frameSize))
            {
                deviceSpaceClipList.subtract(transformedR);
                pendingDeviceSpaceClipList.subtract(transformedR.reduced(1));
            }
        }
        else
        {
            deviceSpaceClipList = frameSize;

            pendingDeviceSpaceClipList.subtract(userSpaceExcludedRectangle);
        }
    }

    void Direct2DGraphicsContext::clipToPath(const Path& path, const AffineTransform& transform)
    {
        SCOPED_TRACE_EVENT(etw::clipToPath, llgcFrameNumber, etw::direct2dKeyword);

        applyPendingClipList();

        //
        // Set the clip list to the full size of the frame to match
        // the software renderer
        //
        auto pathTransform = currentState->currentTransform.getTransformWith(transform);
        auto transformedBounds = path.getBounds().transformedBy(pathTransform);
        currentState->deviceSpaceClipList.clipTo(transformedBounds.toNearestIntEdges());

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            currentState->pushGeometryClipLayer(
                direct2d::pathToPathGeometry(getPimpl()->getDirect2DFactory(), path, pathTransform, D2D1_FIGURE_BEGIN_FILLED, metrics.get()));
        }
    }

    void Direct2DGraphicsContext::clipToImageAlpha(const Image& sourceImage, const AffineTransform& transform)
    {
        SCOPED_TRACE_EVENT(etw::clipToImageAlpha, llgcFrameNumber, etw::direct2dKeyword);

        if (sourceImage.isNull())
        {
            return;
        }

        applyPendingClipList();

        //
        // Put a rectangle clip layer under the image clip layer
        //
        // The D2D bitmap brush will extend past the boundaries of sourceImage, so clip
        // to the sourceImage bounds
        //
        auto brushTransform = currentState->currentTransform.getTransformWith(transform);
        {
            D2D1_RECT_F sourceImageRectF = direct2d::rectangleToRectF(sourceImage.getBounds());
            ComSmartPtr<ID2D1RectangleGeometry> geometry;
            getPimpl()->getDirect2DFactory()->CreateRectangleGeometry(sourceImageRectF, geometry.resetAndGetPointerAddress());
            if (geometry)
            {
                currentState->pushTransformedRectangleGeometryClipLayer(geometry, brushTransform);
            }
        }

        //
        // Set the clip list to the full size of the frame to match
        // the software renderer
        //
        currentState->deviceSpaceClipList = getPimpl()->getFrameSize();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            //
            // Is this a Direct2D image already?
            //
            ComSmartPtr<ID2D1Bitmap> d2d1Bitmap;

            if (auto direct2DPixelData = dynamic_cast<Direct2DPixelData*> (sourceImage.getPixelData()))
            {
                d2d1Bitmap = direct2DPixelData->getAdapterD2D1Bitmap(getPimpl()->getAdapter());
            }

            if (! d2d1Bitmap)
            {
                //
                // Convert sourceImage to single-channel alpha-only maskImage
                //
                direct2d::Direct2DBitmap direct2DBitmap = direct2d::Direct2DBitmap::fromImage(sourceImage, deviceContext, Image::SingleChannel);
                d2d1Bitmap = direct2DBitmap.getD2D1Bitmap();
            }

            if (d2d1Bitmap)
            {
                //
                // Make a transformed bitmap brush using the bitmap
                //
                // As usual, apply the current transform first *then* the transform parameter
                //
                ComSmartPtr<ID2D1BitmapBrush> brush;
                auto                          matrix = direct2d::transformToMatrix(brushTransform);
                D2D1_BRUSH_PROPERTIES         brushProps = { 1.0f, matrix };

                auto bitmapBrushProps = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP);
                auto hr = deviceContext->CreateBitmapBrush(d2d1Bitmap, bitmapBrushProps, brushProps, brush.resetAndGetPointerAddress());

                if (SUCCEEDED(hr))
                {
                    //
                    // Push the clipping layer onto the layer stack
                    //
                    // Don't set maskTransform in the LayerParameters struct; that only applies to geometry clipping
                    // Do set the contentBounds member, transformed appropriately
                    //
                    auto layerParams = D2D1::LayerParameters();
                    auto transformedBounds = sourceImage.getBounds().toFloat().transformedBy(brushTransform);
                    layerParams.contentBounds = direct2d::rectangleToRectF(transformedBounds);
                    layerParams.opacityBrush = brush;

                    currentState->pushLayer(layerParams);
                }
            }
        }
    }

    bool Direct2DGraphicsContext::clipRegionIntersects(const Rectangle<int>& r)
    {
        if (currentState->currentTransform.isOnlyTranslated)
        {
            return currentState->deviceSpaceClipList.intersectsRectangle(currentState->currentTransform.translated(r));
        }

        return currentState->deviceSpaceClipList.intersectsRectangle(currentState->currentTransform.transformed(r));
    }

    Rectangle<int> Direct2DGraphicsContext::getClipBounds() const
    {
        return currentState->currentTransform.deviceSpaceToUserSpace(currentState->deviceSpaceClipList.getBounds());
    }

    bool Direct2DGraphicsContext::isClipEmpty() const
    {
        return getClipBounds().isEmpty();
    }

    void Direct2DGraphicsContext::saveState()
    {
        SCOPED_TRACE_EVENT(etw::saveState, llgcFrameNumber, etw::direct2dKeyword);

        applyPendingClipList();

        currentState = getPimpl()->pushSavedState();
    }

    void Direct2DGraphicsContext::restoreState()
    {
        SCOPED_TRACE_EVENT(etw::restoreState, llgcFrameNumber, etw::direct2dKeyword);

        currentState = getPimpl()->popSavedState();

        currentState->updateColourBrush();
        jassert(currentState);

        resetPendingClipList();
    }

    void Direct2DGraphicsContext::beginTransparencyLayer(float opacity)
    {
        SCOPED_TRACE_EVENT(etw::beginTransparencyLayer, llgcFrameNumber, etw::direct2dKeyword);

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            currentState->pushTransparencyLayer(opacity);
        }
    }

    void Direct2DGraphicsContext::endTransparencyLayer()
    {
        SCOPED_TRACE_EVENT(etw::endTransparencyLayer, llgcFrameNumber, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            currentState->popTopLayer();
        }
    }

    void Direct2DGraphicsContext::setFill(const FillType& fillType)
    {
        SCOPED_TRACE_EVENT(etw::setFill, llgcFrameNumber, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            currentState->fillType = fillType;
            currentState->updateCurrentBrush();
        }
    }

    void Direct2DGraphicsContext::setOpacity(float newOpacity)
    {
        SCOPED_TRACE_EVENT(etw::setOpacity, llgcFrameNumber, etw::direct2dKeyword);

        currentState->setOpacity(newOpacity);
        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            currentState->updateCurrentBrush();
        }
    }

    void Direct2DGraphicsContext::setInterpolationQuality(Graphics::ResamplingQuality quality)
    {
        switch (quality)
        {
        case Graphics::ResamplingQuality::lowResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
            break;

        case Graphics::ResamplingQuality::mediumResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_LINEAR;
            break;

        case Graphics::ResamplingQuality::highResamplingQuality:
            currentState->interpolationMode = D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC;
            break;
        }
    }

    void Direct2DGraphicsContext::fillRect(const Rectangle<int>& r, bool replaceExistingContents)
    {
        applyPendingClipList();

        if (replaceExistingContents)
        {
            SCOPED_TRACE_EVENT_INT_RECT(etw::fillRectReplace,llgcFrameNumber, r, etw::direct2dKeyword);

            clipToRectangle(r);
            getPimpl()->clearBackground();
            currentState->popTopLayer();
        }

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = r + currentState->currentTransform.offset; currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTranslatedRectTime)

                        deviceContext->FillRectangle(direct2d::rectangleToRectF(translatedR), brush);
                    }
                }
                return;
            }

            if (currentState->currentTransform.isAxisAligned())
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto transformedR = currentState->currentTransform.transformed(r); currentState->doesRectangleIntersectClipList(transformedR))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillAxisAlignedRectTime)

                        deviceContext->FillRectangle(direct2d::rectangleToRectF(transformedR), brush);
                    }
                }
                return;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                if (auto transformedR = currentState->currentTransform.transformed(r); currentState->doesRectangleIntersectClipList(transformedR))
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTransformedRectTime)

                    ScopedTransform scopedTransform{ *getPimpl(), currentState };
                    deviceContext->FillRectangle(direct2d::rectangleToRectF(r), brush);
                }
            }
        }
    }

    void Direct2DGraphicsContext::fillRect(const Rectangle<float>& r)
    {
        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::fillRect, llgcFrameNumber, r, etw::direct2dKeyword);

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = r + currentState->currentTransform.offset.toFloat(); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTranslatedRectTime)

                        deviceContext->FillRectangle(direct2d::rectangleToRectF(translatedR), brush);
                    }
                }
                return;
            }

            if (currentState->currentTransform.isAxisAligned())
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto transformedR = currentState->currentTransform.transformed(r); currentState->doesRectangleIntersectClipList(transformedR))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillAxisAlignedRectTime)

                        deviceContext->FillRectangle(direct2d::rectangleToRectF(transformedR), brush);
                    }
                }
                return;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                if (auto transformedR = currentState->currentTransform.transformed(r); currentState->doesRectangleIntersectClipList(transformedR))
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTransformedRectTime)

                    ScopedTransform scopedTransform{ *getPimpl(), currentState };
                    deviceContext->FillRectangle(direct2d::rectangleToRectF(r), brush);
                }
            }
        }
    }

    void Direct2DGraphicsContext::fillRectList(const RectangleList<float>& list)
    {
        auto const& transform = currentState->currentTransform;

        applyPendingClipList();

        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillRectListTime)

        SCOPED_TRACE_EVENT_FLOAT_RECT_LIST(etw::fillRectList, llgcFrameNumber, list, etw::direct2dKeyword | etw::spriteKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            //
            // Use a sprite batch? Requires Windows 10 or later.
            //
            if (currentState->fillType.isColour())
            {
                if (auto rectangleListSpriteBatch = getPimpl()->getRectangleListSpriteBatch())
                {
                    if (transform.isOnlyTranslated)
                    {
                        auto translateRectangle = [&](Rectangle<float> const& r) -> Rectangle<float>
                            {
                                return transform.translated(r);
                            };

                        rectangleListSpriteBatch->fillRectangles(deviceContext,
                            list,
                            currentState->fillType.colour,
                            translateRectangle,
                            metrics.get());
                        return;
                    }

                    if (transform.isAxisAligned())
                    {
                        auto transformRectangle = [&](Rectangle<float> const& r) -> Rectangle<float>
                            {
                                return transform.transformed(r);
                            };

                        rectangleListSpriteBatch->fillRectangles(deviceContext,
                            list,
                            currentState->fillType.colour,
                            transformRectangle,
                            metrics.get());
                        return;
                    }

                    auto checkRectangleWithoutTransforming = [&](Rectangle<float> const& r) -> Rectangle<float>
                        {
                            return transform.transformed(r);
                        };

                    ScopedTransform scopedTransform{ *getPimpl(), currentState };
                    rectangleListSpriteBatch->fillRectangles(deviceContext,
                        list,
                        currentState->fillType.colour,
                        checkRectangleWithoutTransforming,
                        metrics.get());
                    return;
                }
            }

            //
            // Call FillRectangle for each rectangle
            //
            if (transform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    for (auto const& r : list)
                    {
                        if (auto translatedR = transform.translated(r); currentState->doesRectangleIntersectClipList(translatedR))
                        {
                            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTranslatedRectTime)

                            deviceContext->FillRectangle(direct2d::rectangleToRectF(translatedR), brush);
                        }
                    }
                }
                return;
            }

            if (transform.isAxisAligned())
            {
                if (auto brush = currentState->getBrush())
                {
                    for (auto const& r : list)
                    {
                        if (auto transformedR = transform.transformed(r); currentState->doesRectangleIntersectClipList(transformedR))
                        {
                            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillAxisAlignedRectTime)

                            deviceContext->FillRectangle(direct2d::rectangleToRectF(transformedR), brush);
                        }
                    }
                }
                return;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                {
                    ScopedTransform scopedTransform{ *getPimpl(), currentState };
                    for (auto const& r : list)
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillTransformedRectTime)

                        if (auto transformedR = transform.transformed(r);  currentState->doesRectangleIntersectClipList(transformedR))
                        {
                            deviceContext->FillRectangle(direct2d::rectangleToRectF(r), brush);
                        }
                    }
                }
            }
        }
    }

    bool Direct2DGraphicsContext::drawRect(const Rectangle<float>& r, float lineThickness)
    {
        applyPendingClipList();

        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::drawRect, llgcFrameNumber, r, etw::direct2dKeyword);

        //
        // ID2D1DeviceContext::DrawRectangle centers the stroke around the edges of the specified rectangle, but
        // the software renderer contains the stroke within the rectangle
        //
        // To match the software renderer, reduce the rectangle by half the stroke width
        //
        if (r.getWidth() * 0.5f < lineThickness || r.getHeight() * 0.5f < lineThickness)
        {
            return false;
        }

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            auto reducedR = r.reduced(lineThickness * 0.5f);

            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = currentState->currentTransform.translated(reducedR); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        deviceContext->DrawRectangle(direct2d::rectangleToRectF(translatedR), brush, lineThickness);
                    }
                }
                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                deviceContext->DrawRectangle(direct2d::rectangleToRectF(reducedR), brush, lineThickness);
            }
        }

        return true;
    }

    void Direct2DGraphicsContext::fillPath(const Path& p, const AffineTransform& transform)
    {
        SCOPED_TRACE_EVENT(etw::fillPath, llgcFrameNumber, etw::direct2dKeyword);

        if (p.isEmpty())
        {
            return;
        }

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (auto brush = currentState->getBrush())
            {
                auto factory = getPimpl()->getDirect2DFactory();

                //
                // Use a cached geometry realisation?
                //
                if (auto geometryRealisation = getPimpl()->getFilledGeometryCache().getGeometryRealisation(p,
                    factory,
                    deviceContext,
                    getPhysicalPixelScaleFactor(),
                    llgcFrameNumber,
                    metrics.get()))
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGRTime)

                    ScopedTransform scopedTransform{ *getPimpl(), currentState, transform };
                    deviceContext->DrawGeometryRealization(geometryRealisation, brush);
                    return;
                }

                //
                // Create and fill the geometry
                //
                if (auto geometry = direct2d::pathToPathGeometry(factory, p, currentState->currentTransform.getTransformWith(transform), D2D1_FIGURE_BEGIN_FILLED, metrics.get()))
                {
                    JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, fillGeometryTime)

                    deviceContext->FillGeometry(geometry, brush);
                }
            }
        }
    }

    bool Direct2DGraphicsContext::drawPath(const Path& p, const PathStrokeType& strokeType, const AffineTransform& transform)
    {
        SCOPED_TRACE_EVENT(etw::drawPath, llgcFrameNumber, etw::direct2dKeyword);

        if (p.isEmpty())
        {
            return true;
        }

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (auto brush = currentState->getBrush())
            {
                auto factory = getPimpl()->getDirect2DFactory();

                //
                // Use a cached geometry realisation?
                //
                if (auto pathBounds = p.getBounds(); !pathBounds.isEmpty())
                {
                    auto transformedPathBounds = p.getBoundsTransformed(transform);
                    float xScale = transformedPathBounds.getWidth() / pathBounds.getWidth();
                    float yScale = transformedPathBounds.getHeight() / pathBounds.getHeight();
                    if (auto geometryRealisation = getPimpl()->getStrokedGeometryCache().getGeometryRealisation(p,
                        strokeType,
                        factory,
                        deviceContext,
                        xScale,
                        yScale,
                        getPhysicalPixelScaleFactor(),
                        llgcFrameNumber,
                        metrics.get()))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGRTime)

                        ScopedTransform scopedTransform{ *getPimpl(),
                            currentState,
                            AffineTransform::scale(1.0f / xScale, 1.0f / yScale, pathBounds.getX(), pathBounds.getY()).followedBy(transform) };
                        deviceContext->DrawGeometryRealization(geometryRealisation, brush);
                        return true;
                    }
                }

                //
                // Create and draw a geometry
                //
                if (auto geometry = direct2d::pathToPathGeometry(factory, p, transform, D2D1_FIGURE_BEGIN_HOLLOW, metrics.get()))
                {
                    if (auto strokeStyle = direct2d::pathStrokeTypeToStrokeStyle(factory, strokeType))
                    {
                        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGeometryTime)

                        ScopedTransform scopedTransform{ *getPimpl(), currentState };
                        deviceContext->DrawGeometry(geometry, brush, strokeType.getStrokeThickness(), strokeStyle);
                    }
                }
            }
        }

        return true;
    }

    void Direct2DGraphicsContext::drawImage(const Image& image, const AffineTransform& transform)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawImageTime)

        SCOPED_TRACE_EVENT(etw::drawImage, llgcFrameNumber, etw::direct2dKeyword);

        if (image.isNull())
        {
            return;
        }

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            //
            // Is this a Direct2D image already with the correct format?
            //
            ComSmartPtr<ID2D1Bitmap1> d2d1Bitmap;
            Rectangle<int> imageClipArea;

            if (auto direct2DPixelData = dynamic_cast<Direct2DPixelData*> (image.getPixelData()))
            {
                d2d1Bitmap = direct2DPixelData->getAdapterD2D1Bitmap(getPimpl()->getAdapter());
                imageClipArea = direct2DPixelData->deviceIndependentClipArea;
            }

            if (!d2d1Bitmap || d2d1Bitmap->GetPixelFormat().format != DXGI_FORMAT_B8G8R8A8_UNORM)
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(direct2d::MetricsHub::getInstance()->imageContextMetrics, createBitmapTime);

                d2d1Bitmap = direct2d::Direct2DBitmap::fromImage(image, deviceContext, Image::ARGB).getD2D1Bitmap();
                imageClipArea = image.getBounds();
            }

            if (d2d1Bitmap)
            {
                auto sourceRectF = direct2d::rectangleToRectF(imageClipArea);

                auto imageTransform = currentState->currentTransform.getTransformWith(transform);

               if (imageTransform.isOnlyTranslation())
                {
                    auto destinationRect = direct2d::rectangleToRectF(imageClipArea.toFloat() + Point<float>{ imageTransform.getTranslationX(), imageTransform.getTranslationY() });

                    deviceContext->DrawBitmap(d2d1Bitmap,
                        &destinationRect,
                        currentState->fillType.getOpacity(),
                        currentState->interpolationMode,
                        &sourceRectF,
                        {});

                    return;
                }

                ScopedTransform scopedTransform{ *getPimpl(), currentState, transform };
                deviceContext->DrawBitmap(d2d1Bitmap,
                    nullptr,
                    currentState->fillType.getOpacity(),
                    currentState->interpolationMode,
                    &sourceRectF,
                    {});
            }
        }
    }

    void Direct2DGraphicsContext::drawLine(const Line<float>& line)
    {
        drawLineWithThickness(line, 1.0f);
    }

    bool Direct2DGraphicsContext::drawLineWithThickness(const Line<float>& line, float lineThickness)
    {
        SCOPED_TRACE_EVENT_FLOAT_XYWH(etw::drawLine, llgcFrameNumber, line.getStartX(), line.getStartY(), line.getEndX(), line.getEndY(), etw::direct2dKeyword);

        applyPendingClipList();

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    auto offset = currentState->currentTransform.offset.toFloat();
                    auto start = line.getStart() + offset;
                    auto end = line.getEnd() + offset;
                    deviceContext->DrawLine(D2D1::Point2F(start.x, start.y), D2D1::Point2F(end.x, end.y), brush, lineThickness);
                }
                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                deviceContext->DrawLine(D2D1::Point2F(line.getStartX(), line.getStartY()),
                    D2D1::Point2F(line.getEndX(), line.getEndY()),
                    brush,
                    lineThickness);
            }
        }

        return true;
    }

    void Direct2DGraphicsContext::setFont(const Font& newFont)
    {
        SCOPED_TRACE_EVENT(etw::setFont, llgcFrameNumber, etw::direct2dKeyword);

        currentState->setFont(newFont);
    }

    const Font& Direct2DGraphicsContext::getFont()
    {
        return currentState->font;
    }

    void Direct2DGraphicsContext::drawGlyph(int glyphNumber, const AffineTransform& transform)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGlyphRunTime)

        SCOPED_TRACE_EVENT(etw::drawGlyph, llgcFrameNumber, etw::direct2dKeyword);

        getPimpl()->glyphRun.glyphIndices[0] = (uint16)glyphNumber;
        getPimpl()->glyphRun.glyphOffsets[0] = {};

        drawGlyphCommon(1, currentState->font, transform, {});
    }

    bool Direct2DGraphicsContext::drawTextLayout(const AttributedString& text, const Rectangle<float>& area)
    {
        SCOPED_TRACE_EVENT(etw::drawTextLayout, llgcFrameNumber, etw::direct2dKeyword);

        applyPendingClipList();

        auto deviceContext = getPimpl()->getDeviceContext();
        auto directWriteFactory = getPimpl()->getDirectWriteFactory();
        auto fontCollection = getPimpl()->getFontCollection();

        auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform);

        if (deviceContext && directWriteFactory && fontCollection && brush)
        {
            ScopedTransform scopedTransform{ *getPimpl(), currentState };
            auto translatedArea = area;
            auto textLayout =
                DirectWriteTypeLayout::createDirectWriteTextLayout(text, translatedArea, *directWriteFactory, *fontCollection, *deviceContext);
            if (textLayout)
            {
                deviceContext->DrawTextLayout(D2D1::Point2F(translatedArea.getX(), translatedArea.getY()),
                    textLayout,
                    brush,
                    D2D1_DRAW_TEXT_OPTIONS_NONE);
            }
        }

        return true;
    }

    float Direct2DGraphicsContext::getPhysicalPixelScaleFactor()
    {
        return getPimpl()->getScaleFactor();
    }

    void Direct2DGraphicsContext::setPhysicalPixelScaleFactor(float scale_)
    {
        getPimpl()->setScaleFactor(scale_);
    }

    bool Direct2DGraphicsContext::drawRoundedRectangle(Rectangle<float> area, float cornerSize, float lineThickness)
    {
        applyPendingClipList();

        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::drawRoundedRectangle, llgcFrameNumber, area, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = currentState->currentTransform.translated(area); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        D2D1_ROUNDED_RECT roundedRect{ direct2d::rectangleToRectF(translatedR), cornerSize, cornerSize };

                        deviceContext->DrawRoundedRectangle(roundedRect, brush, lineThickness);
                    }
                }
                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                D2D1_ROUNDED_RECT roundedRect{ direct2d::rectangleToRectF(area), cornerSize, cornerSize };
                deviceContext->DrawRoundedRectangle(roundedRect, brush, lineThickness);
            }
        }

        return true;
    }

    bool Direct2DGraphicsContext::fillRoundedRectangle(Rectangle<float> area, float cornerSize)
    {
        applyPendingClipList();

        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::fillRoundedRectangle, llgcFrameNumber, area, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = currentState->currentTransform.translated(area); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        D2D1_ROUNDED_RECT roundedRect{ direct2d::rectangleToRectF(translatedR), cornerSize, cornerSize };

                        deviceContext->FillRoundedRectangle(roundedRect, brush);
                    }
                }
                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                D2D1_ROUNDED_RECT roundedRect{ direct2d::rectangleToRectF(area), cornerSize, cornerSize };
                deviceContext->FillRoundedRectangle(roundedRect, brush);
            }
        }

        return true;
    }

    bool Direct2DGraphicsContext::drawEllipse(Rectangle<float> area, float lineThickness)
    {
        applyPendingClipList();

        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::drawEllipse, llgcFrameNumber, area, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = currentState->currentTransform.translated(area); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        area = currentState->currentTransform.translated(area);
                        auto centre = area.getCentre();

                        D2D1_ELLIPSE ellipse{ { centre.x, centre.y}, area.proportionOfWidth(0.5f), area.proportionOfHeight(0.5f) };
                        deviceContext->DrawEllipse(ellipse, brush, lineThickness);
                    }
                }

                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                auto         centre = area.getCentre();
                D2D1_ELLIPSE ellipse{ { centre.x, centre.y }, area.proportionOfWidth(0.5f), area.proportionOfHeight(0.5f) };
                deviceContext->DrawEllipse(ellipse, brush, lineThickness, nullptr);
            }
        }

        return true;
    }

    bool Direct2DGraphicsContext::fillEllipse(Rectangle<float> area)
    {
        applyPendingClipList();

        SCOPED_TRACE_EVENT_FLOAT_RECT(etw::fillEllipse, llgcFrameNumber, area, etw::direct2dKeyword);

        if (auto deviceContext = getPimpl()->getDeviceContext())
        {
            if (currentState->currentTransform.isOnlyTranslated)
            {
                if (auto brush = currentState->getBrush())
                {
                    if (auto translatedR = currentState->currentTransform.translated(area); currentState->doesRectangleIntersectClipList(translatedR))
                    {
                        area = currentState->currentTransform.translated(area);
                        auto centre = area.getCentre();

                        D2D1_ELLIPSE ellipse{ { centre.x, centre.y}, area.proportionOfWidth(0.5f), area.proportionOfHeight(0.5f) };
                        deviceContext->FillEllipse(ellipse, brush);
                    }
                }
                return true;
            }

            if (auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform))
            {
                ScopedTransform scopedTransform{ *getPimpl(), currentState };
                auto         centre = area.getCentre();
                D2D1_ELLIPSE ellipse{ { centre.x, centre.y }, area.proportionOfWidth(0.5f), area.proportionOfHeight(0.5f) };
                deviceContext->FillEllipse(ellipse, brush);
            }
        }

        return true;
    }

    void Direct2DGraphicsContext::drawPositionedGlyphRun(Array<PositionedGlyph> const& glyphs,
        int                           startIndex,
        int                           numGlyphs,
        const AffineTransform& transform,
        Rectangle<float>              underlineArea)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGlyphRunTime)

        SCOPED_TRACE_EVENT(etw::drawGlyphRun, llgcFrameNumber, etw::direct2dKeyword);

        if (currentState->fillType.isInvisible())
        {
            return;
        }

        if (numGlyphs > 0 && (startIndex + numGlyphs) <= glyphs.size())
        {
            //
            // Fill the array of glyph indices and offsets
            //
            // Each glyph should have the same font
            //
            getPimpl()->glyphRun.ensureStorageAllocated(numGlyphs);

            auto const& firstGlyph = glyphs[startIndex];
            auto const& font = firstGlyph.getFont();
            auto        fontHorizontalScale = font.getHorizontalScale();
            auto        inverseHScale = fontHorizontalScale > 0.0f ? 1.0f / fontHorizontalScale : 1.0f;

            auto indices = getPimpl()->glyphRun.glyphIndices.getData();
            auto offsets = getPimpl()->glyphRun.glyphOffsets.getData();

            int numGlyphsToDraw = 0;
            for (int sourceIndex = 0; sourceIndex < numGlyphs; ++sourceIndex)
            {
                auto const& glyph = glyphs[sourceIndex + startIndex];
                if (!glyph.isWhitespace())
                {
                    indices[numGlyphsToDraw] = (UINT16)glyph.getGlyphNumber();
                    offsets[numGlyphsToDraw] = {
                        glyph.getLeft() * inverseHScale,
                        -glyph.getBaselineY()
                    }; // note the essential minus sign before the baselineY value; negative offset goes down, positive goes up (opposite from JUCE)
                    jassert(getPimpl()->glyphRun.glyphAdvances[numGlyphsToDraw] == 0.0f);
                    jassert(glyph.getFont() == font);
                    ++numGlyphsToDraw;
                }
            }

            drawGlyphCommon(numGlyphsToDraw, font, transform, underlineArea);
        }
    }

    void Direct2DGraphicsContext::drawTextLayoutGlyphRun(Array<TextLayout::Glyph> const& glyphs, const Font& font, const AffineTransform& transform)
    {
        JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(metrics, drawGlyphRunTime)

        SCOPED_TRACE_EVENT(etw::drawGlyphRun, llgcFrameNumber, etw::direct2dKeyword);

        if (currentState->fillType.isInvisible() || glyphs.size() <= 0)
        {
            return;
        }

        //
        // Fill the array of glyph indices and offsets
        //
        getPimpl()->glyphRun.ensureStorageAllocated(glyphs.size());

        auto        fontHorizontalScale = font.getHorizontalScale();
        auto        inverseHScale = fontHorizontalScale > 0.0f ? 1.0f / fontHorizontalScale : 1.0f;

        auto indices = getPimpl()->glyphRun.glyphIndices.getData();
        auto offsets = getPimpl()->glyphRun.glyphOffsets.getData();

        int numGlyphsToDraw = 0;
        for (auto const& glyph : glyphs)
        {
            indices[numGlyphsToDraw] = (UINT16)glyph.glyphCode;
            offsets[numGlyphsToDraw] =
            {
                glyph.anchor.x * inverseHScale,
                -glyph.anchor.y
            }; // note the essential minus sign before the baselineY value; negative offset goes down, positive goes up (opposite from JUCE)
            jassert(getPimpl()->glyphRun.glyphAdvances[numGlyphsToDraw] == 0.0f);
            ++numGlyphsToDraw;
        }

        drawGlyphCommon(numGlyphsToDraw, font, transform, {});
    }

    void Direct2DGraphicsContext::resetPendingClipList()
    {
        auto frameSize = getPimpl()->getFrameSize();
        auto& transform = currentState->currentTransform;

        if (!transform.isOnlyTranslated && !transform.isAxisAligned())
        {
            frameSize = frameSize.transformedBy(transform.getTransform().inverted());
        }

        pendingDeviceSpaceClipList = frameSize;
    }

    void Direct2DGraphicsContext::applyPendingClipList()
    {
        auto& transform = currentState->currentTransform;

        // Clip if the pending clip list is not empty and smaller than the frame size
        if (!pendingDeviceSpaceClipList.containsRectangle(getPimpl()->getFrameSize()) && !pendingDeviceSpaceClipList.isEmpty())
        {
            if (pendingDeviceSpaceClipList.getNumRectangles() == 1 && (transform.isOnlyTranslated || transform.isAxisAligned()))
            {
                auto r = pendingDeviceSpaceClipList.getRectangle(0);
                currentState->pushAliasedAxisAlignedClipLayer(r);
            }
            else
            {
                auto clipTransform = transform.isOnlyTranslated || transform.isAxisAligned() ? AffineTransform{} : transform.getTransform();
                if (auto clipGeometry = direct2d::rectListToPathGeometry(getPimpl()->getDirect2DFactory(),
                    pendingDeviceSpaceClipList,
                    clipTransform,
                    D2D1_FILL_MODE_WINDING,
                    D2D1_FIGURE_BEGIN_FILLED,
                    metrics.get()))
                {
                    currentState->pushGeometryClipLayer(clipGeometry);
                }
            }
        }

        resetPendingClipList();
    }

    void Direct2DGraphicsContext::drawGlyphCommon(int numGlyphs,
        Font const& font,
        const AffineTransform& transform,
        Rectangle<float> underlineArea)
    {
        auto deviceContext = getPimpl()->getDeviceContext();
        if (!deviceContext)
        {
            return;
        }

        auto dwriteFontFace = direct2d::DirectWriteFontFace::fromFont(font);
        if (dwriteFontFace.fontFace == nullptr)
        {
            return;
        }

        auto brush = currentState->getBrush(SavedState::BrushTransformFlags::applyFillTypeTransform);
        if (! brush)
        {
            return;
        }

        applyPendingClipList();

        //
        // Draw the glyph run
        //
        {
            D2D1_POINT_2F baselineOrigin = { 0.0f, 0.0f };

            auto scaledTransform = AffineTransform::scale(dwriteFontFace.fontHorizontalScale, 1.0f).followedBy(transform);
            auto glyphRunTransform = scaledTransform.followedBy(currentState->currentTransform.getTransform());
            bool onlyTranslated = glyphRunTransform.isOnlyTranslation();

            if (onlyTranslated)
            {
                baselineOrigin = { glyphRunTransform.getTranslationX(), glyphRunTransform.getTranslationY() };
            }
            else
            {
                getPimpl()->setDeviceContextTransform(glyphRunTransform);
            }

            DWRITE_GLYPH_RUN directWriteGlyphRun;
            directWriteGlyphRun.fontFace = dwriteFontFace.fontFace;
            directWriteGlyphRun.fontEmSize = dwriteFontFace.getEmSize();
            directWriteGlyphRun.glyphCount = (UINT32)numGlyphs;
            directWriteGlyphRun.glyphIndices = getPimpl()->glyphRun.glyphIndices.getData();
            directWriteGlyphRun.glyphAdvances = getPimpl()->glyphRun.glyphAdvances.getData();
            directWriteGlyphRun.glyphOffsets = getPimpl()->glyphRun.glyphOffsets.getData();
            directWriteGlyphRun.isSideways = FALSE;
            directWriteGlyphRun.bidiLevel = 0;

            deviceContext->DrawGlyphRun(baselineOrigin, &directWriteGlyphRun, brush);

            //
            // Draw the underline
            //
            if (!underlineArea.isEmpty())
            {
                fillRect(underlineArea);
            }

            if (!onlyTranslated)
            {
                getPimpl()->resetDeviceContextTransform();
            }
        }
    }

    Direct2DGraphicsContext::ScopedTransform::ScopedTransform(Pimpl& pimpl_, SavedState* state_) :
        pimpl(pimpl_),
        state(state_)
    {
        pimpl.setDeviceContextTransform(state_->currentTransform.getTransform());
    }

    Direct2DGraphicsContext::ScopedTransform::ScopedTransform(Pimpl& pimpl_, SavedState* state_, const AffineTransform& transform) :
        pimpl(pimpl_),
        state(state_)
    {
        pimpl.setDeviceContextTransform(state_->currentTransform.getTransformWith(transform));
    }

    Direct2DGraphicsContext::ScopedTransform::~ScopedTransform()
    {
        pimpl.resetDeviceContextTransform();
    }

} // namespace juce
