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


#ifndef __SWRDETECTOREDITOR_H_136829C6__
#define __SWRDETECTOREDITOR_H_136829C6__

#include <EditorHeaders.h>

class DetectorInterface;
class SWRDetector;
class ElectrodeButton;

/**

User interface for the SWRDetector processor.

@see SWRDetector

*/

class SWRDetectorEditor : public GenericEditor,
    public ComboBox::Listener,
    public Label::Listener
{
public:
    SWRDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~SWRDetectorEditor();

    void buttonEvent(Button* button);
    void labelTextChanged(juce::Label* label) override;

    void comboBoxChanged(ComboBox* c);

    void updateSettings();

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    void setDefaults(double lowCut, double highCut);

private:

    ScopedPointer<ComboBox> detectorSelector;
    ScopedPointer<UtilityButton> plusButton;

    void addDetector();

    OwnedArray<DetectorInterface> interfaces;

    int previousChannelCount;

    Array<Colour> backgroundColours;

    String lastThresholdConstString;
    String lastThresholdTimeString;

    ScopedPointer<Label> thresholdConstLabel;
    ScopedPointer<Label> eventStimulationTimeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SWRDetectorEditor);

};

class DetectorInterface : public Component,
    public ComboBox::Listener,
    public Button::Listener
{
public:
    DetectorInterface(SWRDetector*, Colour, int);
    ~DetectorInterface();

    void paint(Graphics& g);

    void comboBoxChanged(ComboBox*);
    void buttonClicked(Button*);

    void updateChannels(int);

    void setInputChan(int);
    void setOutputChan(int);
    void setGateChan(int);

    int getInputChan();
    int getOutputChan();
    int getGateChan();

private:
    Colour backgroundColour;

    Font font;

    int idNum;

    SWRDetector* processor;

    OwnedArray<ElectrodeButton> phaseButtons;

    ScopedPointer<ComboBox> inputSelector;
    ScopedPointer<ComboBox> gateSelector;
    ScopedPointer<ComboBox> outputSelector;
};


#endif  // __SWRDETECTOREDITOR_H_136829C6__
