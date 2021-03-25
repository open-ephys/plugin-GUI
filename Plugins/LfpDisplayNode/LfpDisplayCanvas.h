/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef __LFPDISPLAYCANVAS_H__
#define __LFPDISPLAYCANVAS_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
namespace LfpViewer {

class LfpDisplaySplitter;

#pragma  mark - LfpDisplayCanvas -
//==============================================================================
/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/
    
class LfpDisplayCanvas : public Visualizer,
                         public KeyListener,
                         public ComboBox::Listener
{
public:

    LfpDisplayCanvas(LfpDisplayNode* n, SplitLayouts sl);
    ~LfpDisplayCanvas();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float) {}

    void paint(Graphics& g);

    void refresh();
    void resized();

    void comboBoxChanged(ComboBox* cb);

    void setLayout(SplitLayouts);

    void toggleOptionsDrawer(bool);

    int getTotalSplits();

    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);

    bool keyPressed(const KeyPress& key);
    bool keyPressed(const KeyPress& key, Component* orig);

    bool optionsDrawerIsOpen;

private:

    LfpDisplayNode* processor;

    OwnedArray<LfpDisplaySplitter> displaySplits;

    OwnedArray<LfpDisplayOptions> optionsList;

    ScopedPointer<LfpDisplayOptions> options;

    ScopedPointer<ComboBox> displaySelection;
    ScopedPointer<Label> displayLabel;

    SplitLayouts selectedLayout;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayCanvas);

};


class LfpDisplaySplitter : public Component,
                           public ComboBoxListener
{
public:
    LfpDisplaySplitter(LfpDisplayNode* node, LfpDisplayCanvas* canvas, int id);
    ~LfpDisplaySplitter();

    void paint(Graphics& g);

    void beginAnimation();

        /** Resizes the LfpDisplay to the size required to fit all channels that are being
        drawn to the screen.
        
        @param respectViewportPosition  (optional) if true, viewport automatically
                                        scrolls to maintain view prior to resizing
     */
    
    void refreshSplitterState();

    void processorUpdate();
    
    void resized();
    void refresh();

    void resizeToChannels(bool respectViewportPosition = false);

    int getChannelHeight();
    
    float channelOverlapFactor;

    float histogramParameterA;
    float histogramParameterB;

    /** Returns the number of both continuous and event channels tracked by the canvas */
    int getNumChannels();
    /** Returns the number of channels NOT hidden for display */
    int getNumChannelsVisible();
    bool getInputInvertedState();
    
    /** Returns a bool describing whether the spike raster functionality is enabled */
    bool getDisplaySpikeRasterizerState();
    
    bool getDrawMethodState();
    
    int getChannelSampleRate(int channel);
    
    /** Delegates a samplerate for drawing to the LfpDisplay referenced by this canvas */
    void setDrawableSampleRate(float samplerate);
    
    /** Returns the subprocessor index of the given channel */
    int getChannelSubprocessorIdx(int channel);
    
    /** Fetch list of input subprocessors from LfpDisplayNode */
    void setInputSubprocessors();

    /** Delegates a subprocessor for drawing to the LfpDisplay referenced by this
        this canvas */
    void setDrawableSubprocessor(uint32 sp);

    const float getXCoord(int chan, int samp);
    const float getYCoord(int chan, int samp);
    
    std::array<float, MAX_N_SAMP_PER_PIXEL> getSamplesPerPixel(int chan, int px);
    const int getSampleCountPerPixel(int px);
    
    const float getYCoordMin(int chan, int samp);
    const float getYCoordMean(int chan, int samp);
    const float getYCoordMax(int chan, int samp);

    float getMean(int chan);
    float getStd(int chan);

    Array<int> screenBufferIndex;
    Array<int> lastScreenBufferIndex;

    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for each display;
    static const int leftmargin = 50; // left margin for lfp plots (so the ch number text doesnt overlap)

    Array<bool> isChannelEnabled;
    
    bool  drawClipWarning; // optinally draw (subtle) warning if data is clipped in display
    bool  drawSaturationWarning; // optionally raise hell if the actual data is saturating
    
    int nChans;
    //int nChansVisible; // the number of channels NOT hidden for display
    int splitID; //split display ID

    float timebase;

    void redraw();

    void handleSpaceKeyPauseEvent();

    /** Respond to user's subprocessor selection */
    void comboBoxChanged(ComboBox *cb);

	DataChannel::DataChannelTypes selectedChannelType;

    ScopedPointer<LfpViewport> viewport;
    ScopedPointer<LfpTimescale> timescale;
    ScopedPointer<LfpDisplay> lfpDisplay;
    LfpDisplayOptions* options;


private:

    LfpDisplayNode* processor;

    float sampleRate;

    float displayGain;
    float timeOffset;

	uint32 drawableSubprocessor;
	float displayedSampleRate;
    
    std::shared_ptr<AudioSampleBuffer> displayBuffer; // sample wise data buffer for display
    ScopedPointer<AudioSampleBuffer> screenBuffer; // subsampled buffer- one int per pixel

    //'define 3 buffers for min mean and max for better plotting of spikes
    // not pretty, but 'AudioSampleBuffer works only for channels X samples
    ScopedPointer<AudioSampleBuffer> screenBufferMin; // like screenBuffer but holds min/mean/max values per pixel
    ScopedPointer<AudioSampleBuffer> screenBufferMean; // like screenBuffer but holds min/mean/max values per pixel
    ScopedPointer<AudioSampleBuffer> screenBufferMax; // like screenBuffer but holds min/mean/max values per pixel

    MidiBuffer* eventBuffer;

    ScopedPointer<ComboBox> subprocessorSelection;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    Array<int> displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;

	void resizeSamplesPerPixelBuffer(int numChannels);
    std::vector<std::array<std::array<float, MAX_N_SAMP_PER_PIXEL>, MAX_N_SAMP>> samplesPerPixel;

    int sampleCountPerPixel[MAX_N_SAMP];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplaySplitter);

    
};

}; // namespace
#endif
