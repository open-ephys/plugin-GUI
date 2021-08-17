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

#include "AudioMonitorEditor.h"


MonitorMuteButton::MonitorMuteButton()
    : ImageButton ("MuteButton")
{
    Image offimage = ImageCache::getFromMemory  (BinaryData::muteoff_png, BinaryData::muteoff_pngSize);
    Image onimage  = ImageCache::getFromMemory  (BinaryData::muteon_png,  BinaryData::muteon_pngSize);

    setImages (false, true, true,
               offimage, 1.0f, Colours::black,
               offimage, 1.0f, Colours::black.withAlpha (0.0f),
               onimage,  1.0f, Colours::darkgrey);

    setClickingTogglesState (true);

    setTooltip ("Mute audio");
}


MonitorMuteButton::~MonitorMuteButton()
{
}


AudioMonitorEditor::AudioMonitorEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors = true)
    : GenericEditor (parentNode, useDefaultParameterEditors),
      editable(true)
{
    audioMonitor = static_cast<AudioMonitor*>(parentNode);
    
    channelSelectButton = std::make_unique<UtilityButton>("C", Font ("Small Text", 14, Font::plain));
    channelSelectButton->setBounds(20, 35, 20, 20);
	channelSelectButton->addListener(this);
	addAndMakeVisible(channelSelectButton.get());

    muteButton = std::make_unique<MonitorMuteButton>();
    muteButton->addListener (this);
    muteButton->setToggleState (false, dontSendNotification);
    muteButton->setBounds(120, 35, 20, 20);
    addAndMakeVisible (muteButton.get());

    spikeChan = std::make_unique<ComboBox>("Spike Channels");
    spikeChan->setBounds(20, 80, 140, 20);

    for (int i = 0; i < audioMonitor->getTotalSpikeChannels() ; i++)
	{
		spikeChan->addItem(audioMonitor->getSpikeChannel(i)->getName(), i + 1);
	}
    
    spikeChan->setTextWhenNoChoicesAvailable("No spike channels");
    spikeChan->setTextWhenNothingSelected("Select a Spike Channel");

	spikeChan->addListener(this);
	addAndMakeVisible(spikeChan.get());

    desiredWidth = 180;
}


AudioMonitorEditor::~AudioMonitorEditor()
{
}


void AudioMonitorEditor::buttonEvent (Button* button)
{
    if (button == channelSelectButton.get())
    {
        channelStates = audioMonitor->dataChannelStates;
        auto* channelSelector = new PopupChannelSelector(channelStates, editable);
    
        CallOutBox& myBox
            = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        
    }

    else if (button == muteButton.get())
    {
        if (muteButton->getToggleState())
        {
            getAudioProcessor()->setParameter (1,0.0f);
            LOGD("Mute on.");
        }
        else
        {
            getAudioProcessor()->setParameter (1,50.0f);
            LOGD("Mute off.");
        }
    }
}

void AudioMonitorEditor::comboBoxChanged(ComboBox* cb)
{

}

void AudioMonitorEditor::startAcquisition()
{
    editable = false;
}


void AudioMonitorEditor::stopAcquisition()
{
    editable = true;
}


void AudioMonitorEditor::saveCustomParameters (XmlElement* xml)
{

}


void AudioMonitorEditor::loadCustomParameters (XmlElement* xml)
{

}