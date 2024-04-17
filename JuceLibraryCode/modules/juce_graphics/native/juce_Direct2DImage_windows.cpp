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
#include "juce_DirectX_windows.h"
#include "juce_Direct2DImage_windows.h"

#endif

namespace juce
{

    //==============================================================================
    //
    // Direct2D pixel data
    //

    Direct2DPixelData::Direct2DPixelData(Image::PixelFormat formatToUse,
        direct2d::DPIScalableArea<int> area_,
        bool clearImage_,
        DirectX::DXGI::Adapter::Ptr adapter_)
        : ImagePixelData(formatToUse,
            area_.getDeviceIndependentWidth(),
            area_.getDeviceIndependentHeight()),
        deviceIndependentClipArea(area_.withZeroOrigin().getDeviceIndependentArea()),
        imageAdapter(adapter_),
        bitmapArea(area_.withZeroOrigin()),
        pixelStride((formatToUse == Image::SingleChannel) ? 1 : 4),
        lineStride((pixelStride* jmax(1, width) + 3) & ~3),
        clearImage(clearImage_)
    {
        createAdapterBitmap();

        directX->dxgi.adapters.listeners.add(this);
    }

    Direct2DPixelData::Direct2DPixelData(ReferenceCountedObjectPtr<Direct2DPixelData> source_,
        Rectangle<int> clipArea_,
        DirectX::DXGI::Adapter::Ptr adapter_)
        : ImagePixelData(source_->pixelFormat, clipArea_.getWidth(), clipArea_.getHeight()),
        deviceIndependentClipArea(clipArea_ + source_->deviceIndependentClipArea.getPosition()),
        imageAdapter(adapter_),
        bitmapArea(source_->bitmapArea.withZeroOrigin()),
        pixelStride(source_->pixelStride),
        lineStride(source_->lineStride),
        clearImage(false),
        adapterBitmap(source_->adapterBitmap)
    {
        createAdapterBitmap();

        directX->dxgi.adapters.listeners.add(this);
    }

    Direct2DPixelData::Direct2DPixelData(Image::PixelFormat formatToUse,
        direct2d::DPIScalableArea<int> area_,
        bool clearImage_,
        ID2D1Bitmap1* d2d1Bitmap,
        DirectX::DXGI::Adapter::Ptr adapter_)
        : ImagePixelData(formatToUse,
            area_.getDeviceIndependentWidth(),
            area_.getDeviceIndependentHeight()),
        deviceIndependentClipArea(area_.getDeviceIndependentArea()),
        imageAdapter(adapter_),
        bitmapArea(area_.withZeroOrigin()),
        pixelStride(4),
        lineStride((pixelStride* jmax(1, width) + 3) & ~3),
        clearImage(clearImage_)
    {
        if (!imageAdapter || !imageAdapter->direct2DDevice)
        {
            imageAdapter = directX->dxgi.adapters.getDefaultAdapter();
        }

        deviceResources.create(imageAdapter, getDPIScalingFactor());

        adapterBitmap.setD2D1Bitmap(d2d1Bitmap);

        directX->dxgi.adapters.listeners.add(this);
    }

    Direct2DPixelData::~Direct2DPixelData()
    {
        directX->dxgi.adapters.listeners.remove(this);
    }

    bool Direct2DPixelData::isValid() const noexcept
    {
        return imageAdapter && imageAdapter->direct2DDevice && adapterBitmap.getD2D1Bitmap() != nullptr;
    }

    void Direct2DPixelData::createAdapterBitmap()
    {
        if (!imageAdapter || !imageAdapter->direct2DDevice)
            imageAdapter = directX->dxgi.adapters.getDefaultAdapter();

        deviceResources.create(imageAdapter, getDPIScalingFactor());
        adapterBitmap.create(deviceResources.deviceContext.context, pixelFormat, bitmapArea, lineStride, clearImage);
    }

    void Direct2DPixelData::release()
    {
        if (adapterBitmap.getD2D1Bitmap())
        {
            listeners.call(&Listener::imageDataBeingDeleted, this);
        }

        imageAdapter = nullptr;
        deviceResources.release();
        adapterBitmap.release();

        for (auto bitmap : mappableBitmaps)
        {
            bitmap->release();
        }
    }

    int64 Direct2DPixelData::getEffectImageHash() const noexcept
    {
        auto hash = 0xd2d000000000000ll;
        hash |= width;
        hash |= (int64)height << 24;
        return hash;
    }

    ReferenceCountedObjectPtr<Direct2DPixelData> Direct2DPixelData::fromDirect2DBitmap(ID2D1Bitmap1* const bitmap,
        direct2d::DPIScalableArea<int> area)
    {
        Direct2DPixelData::Ptr pixelData = new Direct2DPixelData{ Image::ARGB, area, false };
        pixelData->adapterBitmap.setD2D1Bitmap(bitmap);
        return pixelData;
    }

    std::unique_ptr<LowLevelGraphicsContext> Direct2DPixelData::createLowLevelContext()
    {
        sendDataChangeMessage();

        createAdapterBitmap();

        auto bitmap = getAdapterD2D1Bitmap(imageAdapter);
        jassert(bitmap);

        auto context = std::make_unique<Direct2DImageContext>(imageAdapter);
        context->startFrame(bitmap, getDPIScalingFactor());
        context->clipToRectangle(deviceIndependentClipArea);
        context->setOrigin(deviceIndependentClipArea.getPosition());
        return context;
    }

    ID2D1Bitmap1* Direct2DPixelData::getAdapterD2D1Bitmap(DirectX::DXGI::Adapter::Ptr adapter)
    {
        jassert(adapter && adapter->direct2DDevice);

        if (!imageAdapter || !imageAdapter->direct2DDevice || imageAdapter->direct2DDevice != adapter->direct2DDevice)
        {
            release();

            imageAdapter = adapter;

            createAdapterBitmap();
        }

        jassert(imageAdapter && imageAdapter->direct2DDevice);
        jassert(adapter->direct2DDevice == imageAdapter->direct2DDevice);

        return adapterBitmap.getD2D1Bitmap();
    }

    void Direct2DPixelData::initialiseBitmapData(Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode)
    {
        TRACE_LOG_D2D_IMAGE_MAP_DATA;

        x += deviceIndependentClipArea.getX();
        y += deviceIndependentClipArea.getY();

        //
        // Use a mappable Direct2D bitmap to read the contents of the bitmap from the CPU back to the CPU
        //
        // Mapping the bitmap to the CPU means this class can read the pixel data, but the mappable bitmap
        // cannot be a render target
        //
        // So - the Direct2D image low-level graphics context allocates two bitmaps - the adapter bitmap and the mappable bitmap.
        // initialiseBitmapData copies the contents of the adapter bitmap to the mappable bitmap, then maps that mappable bitmap to the
        // CPU.
        //
        // Ultimately the data releaser copies the bitmap data from the CPU back to the GPU
        //
        // Adapter bitmap -> mappable bitmap -> mapped bitmap data -> adapter bitmap
        //
        bitmap.size = 0;
        bitmap.pixelFormat = pixelFormat;
        bitmap.pixelStride = pixelStride;
        bitmap.data = nullptr;

        auto mappableBitmap = new MappableBitmap{};

        if (auto sourceBitmap = getAdapterD2D1Bitmap(imageAdapter))
        {
            mappableBitmap->createAndMap(sourceBitmap,
                pixelFormat,
                Rectangle<int>{ x, y, width, height },
                deviceResources.deviceContext.context,
                deviceIndependentClipArea,
                getDPIScalingFactor(),
                lineStride);
            mappableBitmaps.add(mappableBitmap);
        }

        bitmap.lineStride = (int)mappableBitmap->mappedRect.pitch;
        bitmap.data = mappableBitmap->mappedRect.bits;
        bitmap.size = (size_t)mappableBitmap->mappedRect.pitch * (size_t)height;

        if (pixelFormat == Image::RGB && bitmap.data)
        {
            //
            // Direct2D doesn't support RGB formats, but some legacy code assumes that the pixel stride is 3.
            // Convert the ARGB data to RGB
            // The alpha mode should be set to D2D1_ALPHA_MODE_IGNORE, so the alpha value can be skipped.
            //
            mappableBitmap->rgbProxyImage = SoftwareImageType{}.convertFromBitmapData(bitmap);
            mappableBitmap->rgbProxyBitmapData = std::make_unique<Image::BitmapData>(mappableBitmap->rgbProxyImage, mode);

            //
            // Change the bitmap data to refer to the RGB proxy
            //
            bitmap.pixelStride = mappableBitmap->rgbProxyBitmapData->pixelStride;
            bitmap.lineStride = mappableBitmap->rgbProxyBitmapData->lineStride;
            bitmap.data = mappableBitmap->rgbProxyBitmapData->data;
            bitmap.size = mappableBitmap->rgbProxyBitmapData->size;
        }

        auto bitmapDataScaledArea = direct2d::DPIScalableArea<int>::fromDeviceIndependentArea({ bitmap.width, bitmap.height }, getDPIScalingFactor());
        bitmap.width = bitmapDataScaledArea.getPhysicalArea().getWidth();
        bitmap.height = bitmapDataScaledArea.getPhysicalArea().getHeight();

        bitmap.dataReleaser = std::make_unique<Direct2DBitmapReleaser>(*this, mappableBitmap, mode);

        if (mode != Image::BitmapData::readOnly) sendDataChangeMessage();
    }

    ImagePixelData::Ptr Direct2DPixelData::clone()
    {
        auto sourceArea = direct2d::DPIScalableArea<int>::fromDeviceIndependentArea(deviceIndependentClipArea, getDPIScalingFactor());
        sourceArea.clipToPhysicalArea(bitmapArea.getPhysicalArea());

        auto clone = new Direct2DPixelData{ pixelFormat,
            sourceArea,
            false,
            imageAdapter };

        D2D1_POINT_2U destinationPoint{ 0, 0 };
        auto sourceRectU = sourceArea.getPhysicalAreaD2DRectU();
        auto sourceD2D1Bitmap = getAdapterD2D1Bitmap(imageAdapter);
        auto destinationD2D1Bitmap = clone->getAdapterD2D1Bitmap(imageAdapter);
        if (sourceD2D1Bitmap && destinationD2D1Bitmap)
        {
            auto hr = destinationD2D1Bitmap->CopyFromBitmap(&destinationPoint, sourceD2D1Bitmap, &sourceRectU);
            jassertquiet(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
            {
                return clone;
            }
        }

        return nullptr;
    }

    ImagePixelData::Ptr Direct2DPixelData::clip(Rectangle<int> sourceArea)
    {
        sourceArea = sourceArea.getIntersection({ width, height });

        return new Direct2DPixelData{ this,
            sourceArea,
            imageAdapter };
    }

    float Direct2DPixelData::getDPIScalingFactor() const noexcept
    {
        return bitmapArea.getDPIScalingFactor();
    }

    std::optional<Image> Direct2DPixelData::applyGaussianBlurEffect(float radius, [[maybe_unused]] int frameNumber)
    {
        SCOPED_TRACE_EVENT(etw::nativeDropShadow, frameNumber, etw::graphicsKeyword);

        ComSmartPtr<ID2D1Effect> effect;

        if (deviceResources.deviceContext.context)
        {
            deviceResources.deviceContext.context->CreateEffect(CLSID_D2D1GaussianBlur, effect.resetAndGetPointerAddress());
            if (effect)
            {
                effect->SetInput(0, getAdapterD2D1Bitmap(imageAdapter));
                effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, radius / 8.0f);
                return applyNativeEffect(effect);
            }
        }

        return {};
    }

    std::optional<Image> Direct2DPixelData::applyNativeEffect(ID2D1Effect* effect)
    {
        auto hash = getEffectImageHash();
        auto outputImage = ImageCache::getFromHashCode(hash);
        if (outputImage.isNull())
        {
            outputImage = Image{ Image::ARGB, width,height, false };
            ImageCache::addImageToCache(outputImage, hash);
        }

        applyNativeEffect(effect, outputImage);

        return outputImage;
    }

    void Direct2DPixelData::applyNativeEffect(ID2D1Effect* effect, Image& destImage)
    {
        auto outputPixelData = dynamic_cast<Direct2DPixelData*>(destImage.getPixelData());
        if (auto outputDataContext = outputPixelData->deviceResources.deviceContext.context)
        {
            outputDataContext->SetTarget(outputPixelData->getAdapterD2D1Bitmap(imageAdapter));
            outputDataContext->BeginDraw();
            outputDataContext->Clear();
            outputDataContext->DrawImage(effect);
            outputDataContext->EndDraw();
            outputDataContext->SetTarget(nullptr);
        }
    }

    std::unique_ptr<ImageType> Direct2DPixelData::createType() const
    {
        return std::make_unique<NativeImageType>(getDPIScalingFactor());
    }

    void Direct2DPixelData::adapterCreated(DirectX::DXGI::Adapter::Ptr adapter)
    {
        if (!imageAdapter || imageAdapter->uniqueIDMatches(adapter))
        {
            release();

            imageAdapter = adapter;
        }
    }

    void Direct2DPixelData::adapterRemoved(DirectX::DXGI::Adapter::Ptr adapter)
    {
        if (imageAdapter && imageAdapter->uniqueIDMatches(adapter))
        {
            release();
        }
    }

    Direct2DPixelData::Direct2DBitmapReleaser::Direct2DBitmapReleaser(Direct2DPixelData& pixelData_, ReferenceCountedObjectPtr<MappableBitmap> mappableBitmap_, Image::BitmapData::ReadWriteMode mode_)
        : pixelData(pixelData_),
        mappableBitmap(mappableBitmap_),
        mode(mode_)
    {
    }

    Direct2DPixelData::Direct2DBitmapReleaser::~Direct2DBitmapReleaser()
    {
        TRACE_LOG_D2D_IMAGE_UNMAP_DATA;

        mappableBitmap->unmap(pixelData.adapterBitmap.getD2D1Bitmap(), mode);
        pixelData.mappableBitmaps.removeObject(mappableBitmap);
    }

    //==============================================================================
    //
    // Direct2D native image type
    //

    ImagePixelData::Ptr NativeImageType::create(Image::PixelFormat format, int width, int height, bool clearImage) const
    {
        SharedResourcePointer<DirectX> directX;
        if (!directX->dxgi.isReady())
        {
            //
            // Make sure the DXGI factory exists
            //
            // The caller may be trying to create an Image from a static variable; if this is a DLL, then this is
            // probably called from DllMain. You can't create a DXGI factory from DllMain, so fall back to a
            // software image.
            //
            return new SoftwarePixelData{ format, width, height, clearImage };
        }

        auto area = direct2d::DPIScalableArea<int>::fromDeviceIndependentArea({ width, height }, scaleFactor);
        return new Direct2DPixelData{ format, area, clearImage };
    }

    //==============================================================================
    //
    // Unit test
    //

#if JUCE_UNIT_TESTS

    class Direct2DImageUnitTest final : public UnitTest
    {
    public:
        Direct2DImageUnitTest()
            : UnitTest("Direct2DImageUnitTest", UnitTestCategories::graphics)
        {
            compareFunctions[{Image::RGB, Image::RGB}] = [](uint8* rgb1, uint8* rgb2)
                {
                    return rgb1[0] == rgb2[0] &&
                        rgb1[1] == rgb2[1] &&
                        rgb1[2] == rgb2[2];
                };

            compareFunctions[{Image::RGB, Image::ARGB}] = [](uint8* rgb, uint8* argb)
                {
                    //
                    // Compare bytes directly to avoid alpha premultiply issues
                    //
                    return rgb[0] == argb[0] &&  // blue
                        rgb[1] == argb[1] && // green
                        rgb[2] == argb[2]; // red
                };

            compareFunctions[{Image::RGB, Image::SingleChannel}] = [](uint8*, uint8* singleChannel)
                {
                    return *singleChannel == 0xff;
                };


            compareFunctions[{Image::ARGB, Image::RGB}] = [](uint8* argb, uint8* rgb)
                {
                    //
                    // Compare bytes directly to avoid alpha premultiply issues
                    //
                    return argb[0] == rgb[0] &&
                        argb[1] == rgb[1] &&
                        argb[2] == rgb[2];
                };

            compareFunctions[{Image::ARGB, Image::ARGB}] = [](uint8* argb1, uint8* argb2)
                {
                    return *reinterpret_cast<uint32*>(argb1) == *reinterpret_cast<uint32*>(argb2);
                };

            compareFunctions[{Image::ARGB, Image::SingleChannel}] = [](uint8* argb, uint8* singleChannel)
                {
                    return argb[3] = *singleChannel;
                };


            compareFunctions[{Image::SingleChannel, Image::RGB}] = [](uint8* singleChannel, uint8* rgb)
                {
                    auto alpha = *singleChannel;
                    return rgb[0] == alpha &&
                        rgb[1] == alpha &&
                        rgb[2] == alpha;
                };

            compareFunctions[{Image::SingleChannel, Image::ARGB}] = [](uint8* singleChannel, uint8* argb)
                {
                    return *singleChannel = argb[3];
                };

            compareFunctions[{Image::SingleChannel, Image::SingleChannel}] = [](uint8* singleChannel1, uint8* singleChannel2)
                {
                    return *singleChannel1 == *singleChannel2;
                };
        }

        void runTest() override
        {
            beginTest("Direct2DImageUnitTest");

            for (auto format : formats)
            {
                compareSameFormat(format);
            }

            testFormatConversion();
        }

        Rectangle<int> randomRectangleWithin(Rectangle<int> container) const noexcept
        {
            auto x = getRandom().nextInt(container.getWidth() - 2);
            auto y = getRandom().nextInt(container.getHeight() - 2);
            auto w = getRandom().nextInt(container.getHeight() - x);
            auto h = getRandom().nextInt(container.getWidth() - y);
            h = jmax(h, 1);
            w = jmax(w, 1);
            return Rectangle<int>{ x, y, w, h };
        }

        void compareSameFormat(Image::PixelFormat format)
        {
            auto softwareImage = Image{ SoftwareImageType{}.create(format, 100, 100, true) };
            {
                Graphics g{ softwareImage };
                g.fillCheckerBoard(softwareImage.getBounds().toFloat(), 21.0f, 21.0f, makeRandomColor(), makeRandomColor());
            }

            auto direct2DImage = NativeImageType{}.convert(softwareImage);

            compareImages(softwareImage, direct2DImage, compareFunctions[{ softwareImage.getFormat(), direct2DImage.getFormat()} ]);
            checkReadWriteModes(softwareImage);
            checkReadWriteModes(direct2DImage);
        }

        void compareImages(Image& image1, Image& image2, std::function<bool(uint8*, uint8*)> compareBytes)
        {
            {
                //
                // BitmapData width & height should match
                //
                Rectangle<int> area = randomRectangleWithin(image1.getBounds());
                Image::BitmapData data1{ image1, area.getX(), area.getY(), area.getWidth(), area.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };
                Image::BitmapData data2{ image2, area.getX(), area.getY(), area.getWidth(), area.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };

                expect(data1.width == data2.width);
                expect(data1.height == data2.height);
            }

            {
                //
                // Bitmap data should match after ImageType::convert
                //
                Image::BitmapData data1{ image1, Image::BitmapData::ReadWriteMode::readOnly };
                Image::BitmapData data2{ image2, Image::BitmapData::ReadWriteMode::readOnly };

                for (int y = 0; y < data1.height; ++y)
                {
                    auto line1 = data1.getLinePointer(y);
                    auto line2 = data2.getLinePointer(y);

                    for (int x = 0; x < data1.width; ++x)
                    {
                        expect(compareBytes(line1, line2), "Failed comparing format " + String{ image1.getFormat() } + " to " + String{ image2.getFormat() });

                        line1 += data1.pixelStride;
                        line2 += data2.pixelStride;
                    }
                }
            }

            {
                //
                // Subsection data should match
                //
                // Should be able to have two different BitmapData objects simultaneously for the same source image
                //
                Rectangle<int> area1 = randomRectangleWithin(image1.getBounds());
                Rectangle<int> area2 = randomRectangleWithin(image1.getBounds());
                Image::BitmapData data1{ image1, Image::BitmapData::ReadWriteMode::readOnly };
                Image::BitmapData data2a{ image2, area1.getX(), area1.getY(), area1.getWidth(), area1.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };
                Image::BitmapData data2b{ image2, area2.getX(), area2.getY(), area2.getWidth(), area2.getHeight(), Image::BitmapData::ReadWriteMode::readOnly };

                auto compareSubsection = [&](Image::BitmapData& subsection1, Image::BitmapData& subsection2, Rectangle<int> area)
                {
                    for (int y = 0; y < area.getHeight(); ++y)
                    {
                        auto line1 = subsection1.getLinePointer(y + area.getY());
                        auto line2 = subsection2.getLinePointer(y);

                        for (int x = 0; x < area.getWidth(); ++x)
                        {
                            expect(compareBytes(line1 + (x + area.getX()) * subsection1.pixelStride, line2 + x * subsection2.pixelStride));
                        }
                    }
                };

                compareSubsection(data1, data2a, area1);
                compareSubsection(data1, data2b, area2);
            }
        }

        void checkReadWriteModes(Image& image)
        {
            //
            // Check read and write modes
            //
            int x = getRandom().nextInt(image.getWidth());
            auto writeColor = makeRandomColor().withAlpha(1.0f);
            auto expectedColor = writeColor;
            switch (image.getFormat())
            {
            case Image::SingleChannel:
            {
                auto alpha = writeColor.getAlpha();
                expectedColor = Colour{ alpha, alpha, alpha, alpha };
                break;
            }

            case Image::RGB:
            case Image::ARGB:
                break;

            case Image::UnknownFormat:
            default:
                jassertfalse;
                break;
            }

            {
                Image::BitmapData data{ image, Image::BitmapData::ReadWriteMode::writeOnly };

                for (int y = 0; y < data.height; ++y)
                {
                    data.setPixelColour(x, y, writeColor);
                }
            }

            {
                Image::BitmapData data{ image, Image::BitmapData::ReadWriteMode::readOnly };

                for (int y = 0; y < data.height; ++y)
                {
                    auto color = data.getPixelColour(x, y);
                    expect(color == expectedColor);
                }
            }
        }

        void testFormatConversion()
        {
            for (auto sourceFormat : formats)
            {
                for (auto destFormat : formats)
                {
                    auto softwareStartImage = Image{ SoftwareImageType{}.create(sourceFormat, 100, 100, true) };
                    {
                        Graphics g{ softwareStartImage };
                        g.fillCheckerBoard(softwareStartImage.getBounds().toFloat(), 21.0f, 21.0f, makeRandomColor(), makeRandomColor());
                    }
                    auto convertedSoftwareImage = softwareStartImage.convertedToFormat(destFormat);

                    compareImages(softwareStartImage, convertedSoftwareImage, compareFunctions[{sourceFormat, destFormat}]);

                    auto direct2DImage = NativeImageType{}.convert(softwareStartImage);

                    compareImages(softwareStartImage, direct2DImage, compareFunctions[{ sourceFormat, sourceFormat }]);

                    auto convertedDirect2DImage = direct2DImage.convertedToFormat(destFormat);

                    compareImages(softwareStartImage, convertedDirect2DImage, compareFunctions[{ sourceFormat, destFormat }]);
                }
            }
        }

        Colour makeRandomColor()
        {
            auto random = getRandom();
            uint8 red = (uint8)random.nextInt(255);
            uint8 green = (uint8)random.nextInt(255);
            uint8 blue = (uint8)random.nextInt(255);
            uint8 alpha = (uint8)random.nextInt(255);
            return Colour{ red, green, blue, alpha };
        }

        std::array<Image::PixelFormat, 3> const formats{ Image::RGB, Image::ARGB, Image::SingleChannel };
        std::map<std::pair<Image::PixelFormat, Image::PixelFormat>, std::function<bool(uint8*, uint8*)>> compareFunctions;
    };

    static Direct2DImageUnitTest direct2DImageUnitTest;

#endif

} // namespace juce
