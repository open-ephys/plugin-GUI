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

    class Direct2DImageContext : public Direct2DGraphicsContext
    {
    public:
        /** Creates a context to render into an image. */
        Direct2DImageContext(DirectX::DXGI::Adapter::Ptr adapter_);

        ~Direct2DImageContext() override;

        void startFrame(ID2D1Bitmap1* bitmap, float dpiScaleFactor);

    private:
        struct ImagePimpl;
        std::unique_ptr<ImagePimpl> pimpl;

        Pimpl* getPimpl() const noexcept override;
        void clearTargetBuffer() override;

        static int nextFrameNumber;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Direct2DImageContext)
    };

} // namespace juce
