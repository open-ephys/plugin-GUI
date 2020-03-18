// LfpDisplayClasses.h - part of LfpDisplayNode
#ifndef LFPDISPLAYCLASSES_H
#define LFPDISPLAYCLASSES_H

namespace LfpViewer {
    constexpr int MAX_N_CHAN = 2048;
    constexpr int MAX_N_SAMP = 5000;
    constexpr int MAX_N_SAMP_PER_PIXEL = 100;
    constexpr int CHANNEL_TYPES = 3;

    class LfpDisplayNode;
    class LfpDisplayCanvas;
    class ShowHideOptionsButton;
    class LfpDisplayOptions;
    class LfpTimescale;
    class LfpDisplay;
    class LfpChannelDisplay;
    class LfpChannelDisplayInfo;
    class EventDisplayInterface;
    class LfpViewport;
    class LfpBitmapPlotterInfo;
    class LfpBitmapPlotter;
    class PerPixelBitmapPlotter;
    class SupersampledBitmapPlotter;
    class LfpChannelColourScheme;
    class LfpDefaultColourScheme;
    class LfpMonochromaticColourScheme;
    class LfpGradientColourScheme;

};
#endif
