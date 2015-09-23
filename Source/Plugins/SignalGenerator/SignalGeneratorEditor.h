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

#ifndef __SIGNALGENERATOREDITOR_H_841A7078__
#define __SIGNALGENERATOREDITOR_H_841A7078__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

class FilterViewport;
class WaveformSelector;

/**

  User interface for the SignalGenerator.

  Allows the user to edit the waveform type, amplitude, frequency, and phase of individual channels.

  @see SignalGenerator

*/

class SignalGeneratorEditor : public GenericEditor,
    public Label::Listener
{
public:
    SignalGeneratorEditor(GenericProcessor* parentNode, bool useDefaultParameters);
    virtual ~SignalGeneratorEditor();
    void sliderEvent(Slider* slider);
    void buttonEvent(Button* button);
    void labelTextChanged(Label* label);

private:

    Label* numChannelsLabel;
    TriangleButton* upButton;
    TriangleButton* downButton;

    Slider* amplitudeSlider;
    Slider* frequencySlider;
    Slider* phaseSlider;

    Array<WaveformSelector*> waveformSelectors;

    enum wvfrm
    {
        SINE, SQUARE, SAW, TRIANGLE, NOISE
    };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignalGeneratorEditor);

};

/**

  Customized buttons for selecting different waveforms.

  Currently supports triangle wave, sine wave, square wave, saw wave, and noise.

  @see SignalGeneratorEditor

*/

class WaveformSelector : public Button
{
public:
    WaveformSelector(int type_);
    ~WaveformSelector() {}
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    int type;

    Image neutral;
    Image neutralOver;
    Image selected;
    Image selectedOver;

};


#endif  // __SIGNALGENERATOREDITOR_H_841A7078__
