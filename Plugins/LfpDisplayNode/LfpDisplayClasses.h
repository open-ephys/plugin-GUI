// LfpDisplayClasses.h - part of LfpDisplayNode
#ifndef LFPDISPLAYCLASSES_H
#define LFPDISPLAYCLASSES_H

namespace LfpViewer {
    constexpr int MAX_N_CHAN = 16;
    constexpr int MAX_N_SAMP_PER_PIXEL = 100;
    constexpr int CHANNEL_TYPES = 3;
    enum SplitLayouts {SINGLE = 1, TWO_VERT, THREE_VERT, TWO_HORZ, THREE_HORZ};

    class LfpDisplayNode;
    class LfpDisplayCanvas;
    class LfpDisplaySplitter;
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
    class ChannelColourScheme;
    class DefaultColourScheme;
    class MonochromaticColourScheme;
    class GradientColourScheme;

};
#endif
