/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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


#include <EditorHeaders.h>

class TriangleButton;
class UtilityButton;



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
    SpikeDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~SpikeDetectorEditor();
    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);
    void comboBoxChanged(ComboBox* comboBox);
    void sliderEvent(Slider* slider);

    void channelChanged (int channel, bool newState) override;

    bool addElectrode(int nChans, int electrodeID = 0);
    void removeElectrode(int index);

    void checkSettings();
    void refreshElectrodeList();

private:

    void drawElectrodeButtons(int);

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

    void editElectrode(int index, int chan, int newChan);

    int lastId;
    bool isPlural;

    Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDetectorEditor);

};




#endif  // __SPIKEDETECTOREDITOR_H_F0BD2DD9__
