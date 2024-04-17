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

class Direct2DHwndContext : public Direct2DGraphicsContext
{
public:
    Direct2DHwndContext (void* windowHandle, float dpiScalingFactor, bool opaque);
    ~Direct2DHwndContext() override;

    void* getHwnd() const noexcept;
    void handleShowWindow();
    void setWindowAlpha (float alpha);

    void startResizing();
    void setSize (int width, int height);
    void updateSize();
    void finishResizing();

    void addDeferredRepaint (Rectangle<int> deferredRepaint);
    void addInvalidWindowRegionToDeferredRepaints();

    Image createSnapshot(Rectangle<int> deviceIndependentArea) override;
    Image createSnapshot() override;

    static Colour getBackgroundTransparencyKeyColour() noexcept
    {
        return Colour { 0xff000001 };
    }

private:
    struct HwndPimpl;
    std::unique_ptr<HwndPimpl> pimpl;

    Pimpl* getPimpl() const noexcept override;
    void clearTargetBuffer() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Direct2DHwndContext)
};

namespace direct2d
{

//==============================================================================
//
// DPI scaling and snapping
//
// Repaint areas are specified in units of points (device-independent pixels).
//
// If DPI scaling is > 100%, then the scaled repaint area may not align on a physical pixel boundary. The D2D
// device context can clip and paint to sub-pixel boundaries, but the swap chain dirty rectangles are always
// aligned to physical pixel boundaries. The scaled repaint area may be smaller than the dirty rectangle, leaving
// a small unpainted gap between the clipping region and the dirty rectangle.
//
// For example - say deferredRepaint is x:1, y:3, w:97, h:99 and that DPI scaling is 125%.
// When scaled, the physical area in the swap chain buffer will be:
//
// { 9, 11, 97, 99 } * 1.25 == { 11.25, 13.75, 121.25, 123.75 }
//
// The component will only fill this subpixel region in the swap chain, but the swap chain dirty rectangle
// will be { 11, 13, 122, 124 }, meaning that the dirty rectangle will not be fully painted.
//
// The unscaled deferred repaint area needs to be extended so that the unscaled deferred repaint area. In the example
// above, the { 9, 11, 97, 99 } unscaled region would be expanded to { 8, 8, 100, 100 }, which scales up nicely
// to { 10, 10, 125, 125 }
//
// The trick is therefore to find the smallest amount to expand a given repaint area
//
class PhysicalPixelSnapper
{
public:
    void setDPIScaleFactor(float dpiScaleFactor)
    {
        //
        // The goal here is to find the minimum snap amount to expand the deferred repaint areas based on the DPI
        // scale factor. Instead of a percentage, think of the DPI scale factor as a fractional integer ratio:
        //
        // 100% == 100/100
        // 125% == 125/100
        // 250% == 250/100
        //
        // Simplify the ratio; the denominator of the simplified ratio is the minimum snap amount.
        //
        // 100% == 100/100 == 1/1 == snap to nearest multiple of 1
        // 125% == 125/100 == 5/4 == snap to nearest multiple of 4
        // 250% == 250/100 == 5/2 == snap to nearest multiple of 2
        //
        // Windows allows the user to manually specify the DPI scale factor to the nearest 1%, so round the DPI
        // scaling factor to the nearest 1/128th and simplify the ratio.
        //
        // For example: DPI scaling 150%
        //      snappedDpiScalingFactor = 1.5
        //      greatestCommonDenominator = gdc( 1.5 * 128, 128) = 64
        //      repaintAreaPixelSnap = 128 / 64 = 2
        //      deferredRepaint will be expanded to snap to the next multiple of 2
        //
        // DPI scaling of 225%
        //      snappedDpiScalingFactor = 2.25
        //      greatestCommonDenominator = gdc( 2.25 * 128, 128) = 32
        //      repaintAreaPixelSnap = 128 / 32 = 4
        //      deferredRepaint will be expanded to snap to the next multiple of 4
        //
        // DPI scaling of 301%
        //      snappedDpiScalingFactor = 3.0078125
        //      greatestCommonDenominator = gdc( 3.0078125 * 128, 128) = 1
        //      repaintAreaPixelSnap = 128 / 1 = 128
        //      deferredRepaint will be expanded to snap to the next multiple of 128
        //
        // For the typical scaling factors, the deferred repaint area will be only slightly expanded to the nearest
        // multiple of 4. The more offbeat scaling factors will be less efficient and require more painting.
        //
        snappedDpiScalingFactor =
            (float)roundToInt(dpiScaleFactor * dpiScalingIntConversionFactor) / float{ dpiScalingIntConversionFactor };

        auto greatestCommonDenominator = std::gcd(roundToInt(float{ dpiScalingIntConversionFactor } *snappedDpiScalingFactor),
            dpiScalingIntConversionFactor);
        pixelSnap = dpiScalingIntConversionFactor / greatestCommonDenominator;
    }

    float getDPIScaleFactor() const noexcept
    {
        return snappedDpiScalingFactor;
    }

    Rectangle<int> snapRectangle(Rectangle<int> const rectangle)
    {
        //
        // rectangle is in points; expand rectangle so that when it is scaled, it aligns to a physical pixel boundary
        //
        auto snapMask = ~(pixelSnap - 1);
        return Rectangle<int>::leftTopRightBottom(rectangle.getX() & snapMask,
            rectangle.getY() & snapMask,
            (rectangle.getRight() + pixelSnap - 1) & snapMask,
            (rectangle.getBottom() + pixelSnap - 1) & snapMask);
    }

private:
    float                     snappedDpiScalingFactor = 1.0f;
    static constexpr int      dpiScalingIntConversionShift = 7;
    static constexpr int      dpiScalingIntConversionFactor = 1 << dpiScalingIntConversionShift;
    int                       pixelSnap = dpiScalingIntConversionFactor;
};

}

} // namespace juce
