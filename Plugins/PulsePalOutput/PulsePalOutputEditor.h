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

#ifndef __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__
#define __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__

#include <VisualizerEditorHeaders.h>

/**

  User interface for the PulsePalOutput.

  @see PulsePalOutput

*/

class ChannelTriggerInterface;
class PulsePal;
class PulsePalOutput;

class UtilityButton;

class PulsePalOutputEditor : public VisualizerEditor

{
public:
    PulsePalOutputEditor(GenericProcessor* parentNode, PulsePal* pp, bool useDefaultParameterEditors);
    virtual ~PulsePalOutputEditor();
    void updateSettings();
    Visualizer* createNewCanvas();

private:
    OwnedArray<ChannelTriggerInterface> channelTriggerInterfaces;
    PulsePal* pulsePal;
    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

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
    /**
     * @brief updateSources checks sources available from the Pulse Pal processor
     *        and updates the trigger and gate comboboxes
     */
    void updateSources();

    void setTriggerChannel(int chan);
    void setGateChannel(int chan);
    int getTriggerChannel();
    int getGateChannel();

    void buttonClicked(Button* button);
    void comboBoxChanged(ComboBox* comboBoxThatHasChanged);

private:
    PulsePal* pulsePal;
    PulsePalOutput* processor;

    int m_triggerSelected;
    int m_gateSelected;
    bool isEnabled;
    int channelNumber;
    String name;

    ScopedPointer<UtilityButton> triggerButton;
    ScopedPointer<ComboBox> triggerSelector;
    ScopedPointer<ComboBox> gateSelector;
};



#endif  // __PULSEPALOUTPUTEDITOR_H_BB5F0ECC__
