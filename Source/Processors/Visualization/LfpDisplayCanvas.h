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
#ifndef __LFPDISPLAYCANVAS_H_B711873A__
#define __LFPDISPLAYCANVAS_H_B711873A__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../LfpDisplayNode.h"
#include "Visualizer.h"

class LfpDisplayNode;

class LfpTimescale;
class LfpDisplay;
class LfpChannelDisplay;
class LfpChannelDisplayInfo;
class EventDisplayInterface;

/**

  Displays multiple channels of continuous data.

  @see LfpDisplayNode, LfpDisplayEditor

*/

class LfpDisplayCanvas : public Visualizer,
    public ComboBox::Listener,
    public Button::Listener,
    public KeyListener

{
public:
    LfpDisplayCanvas(LfpDisplayNode* n);
    ~LfpDisplayCanvas();

    void beginAnimation();
    void endAnimation();

    void refreshState();
    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float) {}

	void setRangeSelection(float range); // set range selection combo box to correct value if it has been changed by scolling etc.
	void setSpreadSelection(int spread); // set spread selection combo box to correct value if it has been changed by scolling etc.

    void paint(Graphics& g);

    void refresh();

    void resized();

    int getChannelHeight();

    int getNumChannels();
    bool getInputInvertedState();
    bool getDrawMethodState();

    const float getXCoord(int chan, int samp);
    const float getYCoord(int chan, int samp);

    const float getYCoordMin(int chan, int samp);
    const float getYCoordMean(int chan, int samp);
    const float getYCoordMax(int chan, int samp);

    int screenBufferIndex;
    int lastScreenBufferIndex;

    void comboBoxChanged(ComboBox* cb);
    void buttonClicked(Button* button);

    void saveVisualizerParameters(XmlElement* xml);
    void loadVisualizerParameters(XmlElement* xml);

    bool keyPressed(const KeyPress& key);
    bool keyPressed(const KeyPress& key, Component* orig);


    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for ach ch display;
    static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

    Array<bool> isChannelEnabled;

	int nChans;

private:

    float sampleRate;
    float timebase;
    float displayGain;
    float timeOffset;
    //int spread ; // vertical spacing between channels


    static const int MAX_N_CHAN = 256;  // maximum number of channels
    static const int MAX_N_SAMP = 5000; // maximum display size in pixels
    //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

    LfpDisplayNode* processor;
    AudioSampleBuffer* displayBuffer; // sample wise data buffer for display
    AudioSampleBuffer* screenBuffer; // subsampled buffer- one int per pixel

    //'define 3 buffers for min mean and max for better plotting of spikes
    // not pretty, but 'AudioSampleBuffer works only for channels X samples
    AudioSampleBuffer* screenBufferMin; // like screenBuffer but holds min/mean/max values per pixel
    AudioSampleBuffer* screenBufferMean; // like screenBuffer but holds min/mean/max values per pixel
    AudioSampleBuffer* screenBufferMax; // like screenBuffer but holds min/mean/max values per pixel

    MidiBuffer* eventBuffer;

    ScopedPointer<LfpTimescale> timescale;
    ScopedPointer<LfpDisplay> lfpDisplay;
    ScopedPointer<Viewport> viewport;

    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> rangeSelection;
    ScopedPointer<ComboBox> spreadSelection;
    ScopedPointer<ComboBox> colorGroupingSelection;
    ScopedPointer<UtilityButton> invertInputButton;
    ScopedPointer<UtilityButton> drawMethodButton;
    ScopedPointer<UtilityButton> pauseButton;

    StringArray voltageRanges;
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels
    StringArray colorGroupings; // option for coloring every N channels the same

    OwnedArray<EventDisplayInterface> eventDisplayInterfaces;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    int displayBufferIndex;
    int displayBufferSize;

	int scrollBarThickness;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayCanvas);

};

class LfpTimescale : public Component
{
public:
    LfpTimescale(LfpDisplayCanvas*);
    ~LfpTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    LfpDisplayCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

class LfpDisplay : public Component
{
public:
    LfpDisplay(LfpDisplayCanvas*, Viewport*);
    ~LfpDisplay();

    void setNumChannels(int numChannels);
    int getNumChannels();

    int getTotalHeight();

    void paint(Graphics& g);

    void refresh();

    void resized();

    void mouseDown(const MouseEvent& event);
    void mouseWheelMove(const MouseEvent&  event, const MouseWheelDetails&   wheel) ;


    void setRange(float range);
    int getRange();

    void setChannelHeight(int r, bool resetSingle = true);
    int getChannelHeight();
    void setInputInverted(bool);
    void setDrawMethod(bool);

    void setColors();

    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);

    int getColorGrouping();
    void setColorGrouping(int i);

    void setEnabledState(bool, int);
    bool getEnabledState(int);
    void enableChannel(bool, int);

	bool getSingleChannelState();

    Array<Colour> channelColours;

    Array<LfpChannelDisplay*> channels;
    Array<LfpChannelDisplayInfo*> channelInfo;

    bool eventDisplayEnabled[8];
    bool isPaused; // simple pause function, skips screen bufer updates

private:
	void toggleSingleChannel(int chan);
	int singleChan;

    int numChans;

    int totalHeight;

    int colorGrouping;

    LfpDisplayCanvas* canvas;
    Viewport* viewport;

    float range;


};

class LfpChannelDisplay : public Component
{
public:
    LfpChannelDisplay(LfpDisplayCanvas*, LfpDisplay*, int channelNumber);
    ~LfpChannelDisplay();

    void paint(Graphics& g);

    void select();
    void deselect();

    void setName(String);

    void setColour(Colour c);

    void setChannelHeight(int);
    int getChannelHeight();

    void setChannelOverlap(int);
    int getChannelOverlap();

    void setRange(float range);
    int getRange();

    void setInputInverted(bool);
    void setCanBeInverted(bool);

    void setDrawMethod(bool);

    void setEnabledState(bool);
    bool getEnabledState()
    {
        return isEnabled;
    }

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    LfpDisplayCanvas* canvas;
    LfpDisplay* display;

    bool isSelected;

    int chan;

    String name;

    Font channelFont;

    Colour lineColour;

    int channelOverlap;
    int channelHeight;
    float channelHeightFloat;

    float range;

    bool isEnabled;
    bool inputInverted;
    bool canBeInverted;
    bool drawMethod;

};

class LfpChannelDisplayInfo : public LfpChannelDisplay,
    public Button::Listener
{
public:
    LfpChannelDisplayInfo(LfpDisplayCanvas*, LfpDisplay*, int channelNumber);

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void resized();

    void setEnabledState(bool);

private:

    ScopedPointer<UtilityButton> enableButton;

};

class EventDisplayInterface : public Component,
    public Button::Listener
{
public:
    EventDisplayInterface(LfpDisplay*, LfpDisplayCanvas*, int chNum);
    ~EventDisplayInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

    bool isEnabled;

private:

    int channelNumber;

    LfpDisplay* display;
    LfpDisplayCanvas* canvas;

    ScopedPointer<UtilityButton> chButton;

};


#endif  // __LFPDISPLAYCANVAS_H_B711873A__
