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

#ifndef __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__
#define __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

/**

  User interface for the PulsePalOutput.

  @see PulsePalOutput

*/

class ChannelTriggerInterface;
class PulsePal;
class PulsePalOutput;

class UtilityButton;

class PulsePalOutputEditor : public GenericEditor

{
public:
    PulsePalOutputEditor(GenericProcessor* parentNode, PulsePal* pp, bool useDefaultParameterEditors);
    virtual ~PulsePalOutputEditor();

private:

    OwnedArray<ChannelTriggerInterface> channelTriggerInterfaces;

    PulsePal* pulsePal;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PulsePalOutputEditor);

};


class ChannelTriggerInterface : public Component,
    public Button::Listener,
    public ComboBox::Listener
{
public:
    ChannelTriggerInterface(PulsePal*, PulsePalOutput*, int channelNum);
    ~ChannelTriggerInterface();

    void paint(Graphics& g);

    void buttonClicked(Button* button);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

private:

    int channelNumber;
    String name;

    bool isEnabled;

    PulsePal* pulsePal;
    PulsePalOutput* processor;

    ScopedPointer<UtilityButton> triggerButton;
    ScopedPointer<ComboBox> comboBox;

};



#endif  // __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__
