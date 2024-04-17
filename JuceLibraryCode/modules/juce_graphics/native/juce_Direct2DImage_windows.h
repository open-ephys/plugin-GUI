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

struct ID2D1Bitmap1;

namespace juce
{

    //==============================================================================
    //
    // DPIScalableArea keeps track of an area for a window or a bitmap both in
    // terms of device-independent pixels and physical pixels, as well as the DPI
    // scaling factor.
    //

    namespace direct2d
    {
        template <class valueType>
        class DPIScalableArea
        {
        public:
            static DPIScalableArea fromDeviceIndependentArea(Rectangle<valueType> dipArea, float dpiScalingFactor)
            {
                DPIScalableArea scalableArea;
                scalableArea.deviceIndependentArea = dipArea;
                scalableArea.dpiScalingFactor = dpiScalingFactor;

                //
                // These need to round to the nearest integer, so use roundToInt instead of the standard Rectangle methods
                //
                Rectangle<float> physicalArea = dipArea.toFloat() * dpiScalingFactor;
                scalableArea.physicalArea =
                {
                    roundToInt(physicalArea.getX()),
                    roundToInt(physicalArea.getY()),
                    roundToInt(physicalArea.getWidth()),
                    roundToInt(physicalArea.getHeight())
                };

                return scalableArea;
            }

            static DPIScalableArea fromPhysicalArea(Rectangle<valueType> physicalArea, float dpiScalingFactor)
            {
                DPIScalableArea scalableArea;
                scalableArea.dpiScalingFactor = dpiScalingFactor;
                scalableArea.physicalArea = physicalArea;

                //
                // These need to round to the nearest integer, so use roundToInt instead of the standard Rectangle methods
                //
                Rectangle<float> dipArea = physicalArea.toFloat() / dpiScalingFactor;
                scalableArea.deviceIndependentArea =
                {
                    roundToInt(dipArea.getX()),
                    roundToInt(dipArea.getY()),
                    roundToInt(dipArea.getWidth()),
                    roundToInt(dipArea.getHeight())
                };

                return scalableArea;
            }

            bool isEmpty() const noexcept
            {
                return deviceIndependentArea.isEmpty();
            }

            float getDPIScalingFactor() const noexcept
            {
                return dpiScalingFactor;
            }

            auto getDeviceIndependentArea() const noexcept
            {
                return deviceIndependentArea;
            }

            auto getPhysicalArea() const noexcept
            {
                return physicalArea;
            }

            D2D1_RECT_U getPhysicalAreaD2DRectU() const noexcept
            {
                return { (UINT32)physicalArea.getX(), (UINT32)physicalArea.getY(), (UINT32)physicalArea.getRight(), (UINT32)physicalArea.getBottom() };
            }

            D2D_SIZE_U getPhysicalAreaD2DSizeU() const noexcept
            {
                return { (UINT32)physicalArea.getWidth(), (UINT32)physicalArea.getHeight() };
            }

            valueType getDeviceIndependentWidth() const noexcept { return deviceIndependentArea.getWidth(); }

            valueType getDeviceIndependentHeight() const noexcept { return deviceIndependentArea.getHeight(); }

            void clipToPhysicalArea(Rectangle<valueType> clipArea)
            {
                *this = fromPhysicalArea(physicalArea.getIntersection(clipArea), dpiScalingFactor);
            }

            DPIScalableArea withZeroOrigin() const noexcept
            {
                return DPIScalableArea
                {
                    deviceIndependentArea.withZeroOrigin(),
                    physicalArea.withZeroOrigin(),
                    dpiScalingFactor
                };
            }

        private:
            DPIScalableArea() = default;

            DPIScalableArea(Rectangle<valueType> deviceIndependentArea_,
                Rectangle<valueType> physicalArea_,
                float dpiScalingFactor_) :
                deviceIndependentArea(deviceIndependentArea_),
                physicalArea(physicalArea_),
                dpiScalingFactor(dpiScalingFactor_)
            {
            }

            Rectangle<valueType> deviceIndependentArea;
            Rectangle<valueType> physicalArea;
            float dpiScalingFactor = 1.0f;
        };
    }

class Direct2DPixelData : public ImagePixelData, public DirectX::DXGI::AdapterListener
{
public:
    Direct2DPixelData(Image::PixelFormat formatToUse, direct2d::DPIScalableArea<int> area_, bool clearImage_, DirectX::DXGI::Adapter::Ptr adapter_ = nullptr);
    Direct2DPixelData(ReferenceCountedObjectPtr<Direct2DPixelData> source_, Rectangle<int> clipArea_, DirectX::DXGI::Adapter::Ptr adapter_ = nullptr);
    Direct2DPixelData(Image::PixelFormat formatToUse, direct2d::DPIScalableArea<int> area_, bool clearImage_, ID2D1Bitmap1* d2d1Bitmap, DirectX::DXGI::Adapter::Ptr adapter_ = nullptr);

    static ReferenceCountedObjectPtr<Direct2DPixelData> fromDirect2DBitmap(ID2D1Bitmap1* const bitmap, direct2d::DPIScalableArea<int> area);

    ~Direct2DPixelData() override;

    bool isValid() const noexcept override;

    ID2D1Bitmap1* getAdapterD2D1Bitmap(DirectX::DXGI::Adapter::Ptr adapter);

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override;

    void initialiseBitmapData(Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override;

    ImagePixelData::Ptr clone() override;

    ImagePixelData::Ptr clip(Rectangle<int> clipArea) override;

    float getDPIScalingFactor() const noexcept;

    std::optional<Image> applyGaussianBlurEffect(float radius, int frameNumber) override;

    std::unique_ptr<ImageType> createType() const override;

    using Ptr = ReferenceCountedObjectPtr<Direct2DPixelData>;

    Rectangle<int> const deviceIndependentClipArea;

    void adapterCreated(DirectX::DXGI::Adapter::Ptr adapter) override;
    void adapterRemoved(DirectX::DXGI::Adapter::Ptr adapter) override;

private:
    class AdapterBitmap : public direct2d::Direct2DBitmap
    {
    public:
        void create(ID2D1DeviceContext1* deviceContext_,
            Image::PixelFormat format,
            direct2d::DPIScalableArea<int> area_,
            int lineStride_,
            bool clearImage_)
        {
            if (!bitmap)
            {
                createBitmap(deviceContext_,
                    format,
                    area_.getPhysicalAreaD2DSizeU(),
                    lineStride_,
                    area_.getDPIScalingFactor(),
                    D2D1_BITMAP_OPTIONS_TARGET);

                //
                // The bitmap may be slightly too large due
                // to DPI scaling, so fill it with transparent black
                //
                if (bitmap && clearImage_)
                {
                    deviceContext_->SetTarget(bitmap);
                    deviceContext_->BeginDraw();
                    deviceContext_->Clear();
                    deviceContext_->EndDraw();
                    deviceContext_->SetTarget(nullptr);
                }

            }
        }
    };

    class MappableBitmap : public direct2d::Direct2DBitmap, public ReferenceCountedObject
    {
    public:
        MappableBitmap() = default;
        ~MappableBitmap()
        {
            jassert(mappedRect.bits == nullptr);
        }

        ID2D1Bitmap* createAndMap(ID2D1Bitmap1* sourceBitmap,
            Image::PixelFormat format,
            Rectangle<int> sourceRectangle,
            ID2D1DeviceContext1* deviceContext_,
            Rectangle<int> deviceIndependentClipArea_,
            float dpiScaleFactor,
            int lineStride_)
        {
            createBitmap(deviceContext_,
                format,
                sourceBitmap->GetPixelSize(),
                lineStride_,
                dpiScaleFactor,
                D2D1_BITMAP_OPTIONS_CPU_READ | D2D1_BITMAP_OPTIONS_CANNOT_DRAW);

            if (bitmap && mappedRect.bits == nullptr)
            {
                JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(direct2d::MetricsHub::getInstance()->imageContextMetrics, mapBitmapTime)

                D2D1_POINT_2U destPoint{ 0, 0 };
                sourceRectangle = sourceRectangle.getIntersection(deviceIndependentClipArea_);
                auto scaledSourceRect = direct2d::DPIScalableArea<int>::fromDeviceIndependentArea(sourceRectangle, dpiScaleFactor);
                auto sourceRectU = scaledSourceRect.getPhysicalAreaD2DRectU();

                //
                // Copy from the painted adapter bitmap to the mappable bitmap
                //
                if (auto hr = bitmap->CopyFromBitmap(&destPoint, sourceBitmap, &sourceRectU); FAILED(hr))
                {
                    return nullptr;
                }

                //
                // Map the mappable bitmap to CPU memory; ID2D1Bitmap::Map will allocate memory and populate mappedRect
                //
                mappedRect = {};
                if (auto hr = bitmap->Map(D2D1_MAP_OPTIONS_READ, &mappedRect); FAILED(hr))
                {
                    return nullptr;
                }
            }

            return bitmap;
        }

        void unmap(ID2D1Bitmap1* adapterD2D1Bitmap, Image::BitmapData::ReadWriteMode mode)
        {
            JUCE_D2DMETRICS_SCOPED_ELAPSED_TIME(direct2d::MetricsHub::getInstance()->imageContextMetrics, unmapBitmapTime)

            //
            // Unmap the mappable bitmap if it was mapped
            //
            // If the mappable bitmap was mapped, copy the mapped bitmap data to the adapter bitmap
            //
            if (mappedRect.bits && bitmap)
            {
                if (adapterD2D1Bitmap && mode != Image::BitmapData::readOnly)
                {
                    if (rgbProxyImage.isValid())
                    {
                        //
                        // Convert the RGB data back to ARGB and then copy the ARGB data to the
                        // GPU
                        //
                        auto argbProxyImage = rgbProxyImage.convertedToFormat(Image::ARGB);
                        Image::BitmapData argbProxyBitmapData{ argbProxyImage, Image::BitmapData::readOnly };

                        D2D1_RECT_U rect{ 0, 0, (uint32)argbProxyImage.getWidth(), (uint32)argbProxyImage.getHeight() };
                        adapterD2D1Bitmap->CopyFromMemory(&rect, argbProxyBitmapData.data, (uint32)argbProxyBitmapData.lineStride);
                    }
                    else
                    {
                        //
                        // Copy data to the GPU
                        //
                        auto size = bitmap->GetPixelSize();

                        D2D1_RECT_U rect{ 0, 0, size.width, size.height };
                        adapterD2D1Bitmap->CopyFromMemory(&rect, mappedRect.bits, mappedRect.pitch);
                    }
                }

                rgbProxyBitmapData = nullptr;
                rgbProxyImage = {};

                bitmap->Unmap();
            }

            mappedRect = {};
        }

        D2D1_MAPPED_RECT          mappedRect{};
        Image rgbProxyImage;
        std::unique_ptr<Image::BitmapData> rgbProxyBitmapData;

        using Ptr = ReferenceCountedObjectPtr<MappableBitmap>;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappableBitmap)
    };

    class Direct2DBitmapReleaser : public Image::BitmapData::BitmapDataReleaser
    {
    public:
        Direct2DBitmapReleaser(Direct2DPixelData& pixelData_, ReferenceCountedObjectPtr<MappableBitmap> mappableBitmap_, Image::BitmapData::ReadWriteMode mode_);
        ~Direct2DBitmapReleaser() override;

    private:
        Direct2DPixelData& pixelData;
        ReferenceCountedObjectPtr<MappableBitmap> mappableBitmap;
        Image::BitmapData::ReadWriteMode mode;
    };

    void createAdapterBitmap();
    void release();

    int64 getEffectImageHash() const noexcept;
    std::optional<Image> applyNativeEffect(ID2D1Effect* effect);
    void applyNativeEffect(ID2D1Effect* effect, Image& destImage);

    SharedResourcePointer<DirectX> directX;
    DirectX::DXGI::Adapter::Ptr imageAdapter;
    direct2d::DeviceResources deviceResources;
    direct2d::DPIScalableArea<int> bitmapArea;
    const int                 pixelStride, lineStride;
    bool const                clearImage;
    AdapterBitmap adapterBitmap;
    ReferenceCountedArray<MappableBitmap> mappableBitmaps;

    JUCE_LEAK_DETECTOR(Direct2DPixelData)
};

} // namespace juce
