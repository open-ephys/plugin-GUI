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
#ifndef __LfpTriggeredAverageCAVCAS_H_B711873A__
#define __LfpTriggeredAverageCAVCAS_H_B711873A__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "LfpTriggeredAverageNode.h"
#include "../Visualization/Visualizer.h"

class LfpTriggeredAverageNode;

class LfpTriggeredAverageTimescale;
class LfpTriggeredAverageDisplay;
class LfpTriggeredAverageChannelDisplay;
class LfpTriggeredAverageChannelDisplayInfo;
class LfpTriggeredAverageEventInterface;

/**

  Displays multiple channels of continuous data.

  @see LfpTriggeredAverageNode, LfpTriggeredAverageDisplayEditor

*/

class LfpTriggeredAverageCanvas : public Visualizer,
    public ComboBox::Listener

{
public:
    LfpTriggeredAverageCanvas(LfpTriggeredAverageNode* n);
    ~LfpTriggeredAverageCanvas();

    void beginAnimation();
    void endAnimation();

    void refreshState();

    void update();

    void setParameter(int, float);
    void setParameter(int, int, int, float) {}

    void paint(Graphics& g);

    void refresh();

    void resized();

    int getChannelHeight();

    int getNumChannels();

    float getXCoord(int chan, int samp);
    float getYCoord(int chan, int samp);

    int screenBufferIndex;
    int lastScreenBufferIndex;

    void comboBoxChanged(ComboBox* cb);

    void saveVisualizerParameters(XmlElement* xml);

    void loadVisualizerParameters(XmlElement* xml);

    //void scrollBarMoved(ScrollBar *scrollBarThatHasMoved, double newRangeStart);

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw, there is a similar switch for ach ch display;
    static const int leftmargin=50; // left margin for lfp plots (so the ch number text doesnt overlap)

private:

    float sampleRate;
    float timebase;
    float displayGain;
    float timeOffset;
    //int spread ; // vertical spacing between channels

    static const int MAX_N_CHAN = 256;  // maximum number of channels
    static const int MAX_N_SAMP = 5000; // maximum display size in pixels
    //float waves[MAX_N_CHAN][MAX_N_SAMP*2]; // we need an x and y point for each sample

    LfpTriggeredAverageNode* processor;
    AudioSampleBuffer* displayBuffer;
    AudioSampleBuffer* screenBuffer;
    MidiBuffer* eventBuffer;

    ScopedPointer<LfpTriggeredAverageTimescale> timescale;
    ScopedPointer<LfpTriggeredAverageDisplay> display;
    ScopedPointer<Viewport> viewport;

    ScopedPointer<ComboBox> timebaseSelection;
    ScopedPointer<ComboBox> rangeSelection;
    ScopedPointer<ComboBox> spreadSelection;

    StringArray voltageRanges;
    StringArray timebases;
    StringArray spreads; // option for vertical spacing between channels

    OwnedArray<LfpTriggeredAverageEventInterface> LfpTriggeredAverageEventInterfaces;

    void refreshScreenBuffer();
    void updateScreenBuffer();

    int displayBufferIndex;
    int displayBufferSize;

    int scrollBarThickness;

    int nChans;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpTriggeredAverageCanvas);

};

class LfpTriggeredAverageTimescale : public Component
{
public:
    LfpTriggeredAverageTimescale(LfpTriggeredAverageCanvas*);
    ~LfpTriggeredAverageTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    LfpTriggeredAverageCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

class LfpTriggeredAverageDisplay : public Component
{
public:
    LfpTriggeredAverageDisplay(LfpTriggeredAverageCanvas*, Viewport*);
    ~LfpTriggeredAverageDisplay();

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

    void setChannelHeight(int r);
    int getChannelHeight();


    bool setEventDisplayState(int ch, bool state);
    bool getEventDisplayState(int ch);


    Array<Colour> channelColours;

    Array<LfpTriggeredAverageChannelDisplay*> channels;
    Array<LfpTriggeredAverageChannelDisplayInfo*> channelInfo;

    bool eventDisplayEnabled[8];

private:
    int numChans;

    int totalHeight;

    LfpTriggeredAverageCanvas* canvas;
    Viewport* viewport;

    float range;

};

class LfpTriggeredAverageChannelDisplay : public Component
{
public:
    LfpTriggeredAverageChannelDisplay(LfpTriggeredAverageCanvas*, LfpTriggeredAverageDisplay*, int channelNumber);
    ~LfpTriggeredAverageChannelDisplay();

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

    bool fullredraw; // used to indicate that a full redraw is required. is set false after each full redraw

protected:

    LfpTriggeredAverageCanvas* canvas;
    LfpTriggeredAverageDisplay* display;

    bool isSelected;

    int chan;

    String name;

    Font channelFont;

    Colour lineColour;

    int channelOverlap;
    int channelHeight;
    float channelHeightFloat;

    float range;

};

class LfpTriggeredAverageChannelDisplayInfo : public LfpTriggeredAverageChannelDisplay
{
public:
    LfpTriggeredAverageChannelDisplayInfo(LfpTriggeredAverageCanvas*, LfpTriggeredAverageDisplay*, int channelNumber);

    void paint(Graphics& g);

};

class LfpTriggeredAverageEventInterface : public Component,
    public Button::Listener
{
public:
    LfpTriggeredAverageEventInterface(LfpTriggeredAverageDisplay*, LfpTriggeredAverageCanvas*, int chNum);
    ~LfpTriggeredAverageEventInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);

    void checkEnabledState();

    bool isEnabled;

private:

    int channelNumber;

    LfpTriggeredAverageDisplay* display;
    LfpTriggeredAverageCanvas* canvas;

    ScopedPointer<UtilityButton> chButton;

};


#endif  // __LfpTriggeredAverageCAVCAS_H_B711873A__
