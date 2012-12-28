/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef __SPIKEDETECTOREDITOR_H_F0BD2DD9__
#define __SPIKEDETECTOREDITOR_H_F0BD2DD9__


#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

class TriangleButton;
class UtilityButton;

/**

  Used to select individual electrodes within a multichannel electrode.

  @see SpikeDetectorEditor.

*/

class ElectrodeButton : public Button
{
public:
    ElectrodeButton(int chan_) : Button("Electrode"), chan(chan_) 
    {
        setClickingTogglesState(true);
        //setRadioGroupId(299);
        setToggleState(true, false);
    }
    ~ElectrodeButton() {}

    int getChannelNum() {return chan;}
    void setChannelNum(int i) {chan = i;}
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    int chan;
};

/**

  Utility button for the SpikeDetectorEditor.

  @see SpikeDetectorEditor

*/

class ElectrodeEditorButton : public Button
{
public:
    ElectrodeEditorButton(const String& name_, Font font_) : Button("Electrode Editor"),
        name(name_), font(font_)
    {
        if (name.equalsIgnoreCase("edit") || name.equalsIgnoreCase("monitor"))
            setClickingTogglesState(true);
    }
    ~ElectrodeEditorButton() {}
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    const String name;

    Font font;

};

/**

  Used to change the spike detection threshold.

  @see SpikeDetectorEditor

*/

class ThresholdSlider : public Slider
{
public: 
    ThresholdSlider(Font f);
    ~ThresholdSlider() {}

    void setActive(bool);

    void setValues(Array<double>);

private:
    void paint(Graphics& g);

    Path makeRotaryPath(double, double, double);

    Font font;

    bool isActive;

    Array<double> valueArray;

};

/**

  User interface for the SpikeDetector processor.

  Allows the user to add single electrodes, stereotrodes, or tetrodes.

  Parameters of individual channels, such as channel mapping, threshold,
  and enabled state, can be edited.

  @see SpikeDetector

*/

class SpikeDetectorEditor : public GenericEditor,
                            public Label::Listener,
                            public ComboBox::Listener

{
public:
	SpikeDetectorEditor (GenericProcessor* parentNode);
	virtual ~SpikeDetectorEditor();
    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);
    void comboBoxChanged(ComboBox* comboBox);
    void sliderEvent(Slider* slider);

    void channelChanged(int chan);

private:	

    void drawElectrodeButtons(int);

    void refreshElectrodeList();
	
    ComboBox* electrodeTypes;
    ComboBox* electrodeList;
    Label* numElectrodes;
    Label* thresholdLabel;
    TriangleButton* upButton;
    TriangleButton* downButton;
    UtilityButton* plusButton;

    ThresholdSlider* thresholdSlider;

    OwnedArray<ElectrodeButton> electrodeButtons;
    Array<ElectrodeEditorButton*> electrodeEditorButtons;

    bool addElectrode(int nChans);
    void removeElectrode(int index);
    void editElectrode(int index, int chan, int newChan);

    int lastId;
    bool isPlural;

    Font font;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDetectorEditor);

};




#endif  // __SPIKEDETECTOREDITOR_H_F0BD2DD9__
