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


#ifndef __AUDIOMONITOREDITOR_H__
#define __AUDIOMONITOREDITOR_H__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../Editors/PopupChannelSelector.h"
#include "../../Source/UI/Utils/LinearButtonGroupManager.h"
#include "../../Source/UI/LookAndFeel/MaterialButtonLookAndFeel.h"
#include "AudioMonitor.h"

class AudioMonitor;

/**
  Toggles audio output on and off.

  @see AudioMonitor, AudioMonitorEditor

*/
class MonitorMuteButton : public ImageButton
{
public:
    MonitorMuteButton();
    ~MonitorMuteButton();
};


/**

  User interface for the "AudioMonitor" source node.

*/

class AudioMonitorEditor : public GenericEditor, ComboBox::Listener
{
public:
    AudioMonitorEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~AudioMonitorEditor();

    void buttonEvent (Button* button) override;

    // Button::Listener method
    void buttonClicked (Button* buttonThatWasClicked) override;

    void comboBoxChanged(ComboBox*); 

    void saveCustomParameters (XmlElement*) override;
    void loadCustomParameters (XmlElement*) override;

	void startAcquisition() override;
	void stopAcquisition()  override;

    void channelStateChanged(Array<int> activeChannels) override;

    std::vector<bool> channelStates;

private:

    AudioMonitor* audioMonitor;
    
    std::unique_ptr<juce::Button> channelSelectButton;

    std::unique_ptr<juce::Label> selectedChansLabel;

    std::unique_ptr<MonitorMuteButton> muteButton;

    std::unique_ptr<juce::ComboBox> spikeChan;

    OwnedArray<ChannelButton> channelButtons;

    std::unique_ptr<LinearButtonGroupManager> outputChannelButtonManager;

    std::shared_ptr<MaterialButtonLookAndFeel> m_materialButtonLookAndFeel;

    bool editable;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioMonitorEditor);
};



#endif  // __AUDIOMONITOREDITOR_H__
