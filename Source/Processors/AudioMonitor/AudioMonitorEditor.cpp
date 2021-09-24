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

static const Colour COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));
static const Colour COLOUR_ACCENT  (Colour::fromRGB (3, 169, 244));

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
    
    selectedChansLabel = std::make_unique<Label>("selected channels", "Channels:");
    selectedChansLabel->setBounds(15, 35, 60, 20);
    addAndMakeVisible(selectedChansLabel.get());
    
    channelSelectButton = std::make_unique<UtilityButton>("C", Font ("Small Text", 14, Font::plain));
    channelSelectButton->setBounds(80, 35, 20, 20);
	channelSelectButton->addListener(this);
	addAndMakeVisible(channelSelectButton.get());

    muteButton = std::make_unique<MonitorMuteButton>();
    muteButton->addListener (this);
    muteButton->setToggleState (false, dontSendNotification);
    muteButton->setBounds(130, 35, 20, 20);
    addAndMakeVisible (muteButton.get());

    spikeChan = std::make_unique<ComboBox>("Spike Channels");
    spikeChan->setBounds(20, 100, 140, 20);

    for (int i = 0; i < audioMonitor->getTotalSpikeChannels() ; i++)
	{
		spikeChan->addItem(audioMonitor->getSpikeChannel(i)->getName(), i + 1);
	}
    
    spikeChan->setTextWhenNoChoicesAvailable("No spike channels");
    spikeChan->setTextWhenNothingSelected("Select a Spike Channel");

	spikeChan->addListener(this);
	addAndMakeVisible(spikeChan.get());

    TextButton* leftButton = new TextButton ("Left", "Output to left channel only");
    leftButton->setClickingTogglesState (true);
    leftButton->setToggleState (true, dontSendNotification);
    leftButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    leftButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    leftButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    leftButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    TextButton* bothButton  = new TextButton ("Both", "Output to both channels");
    bothButton->setClickingTogglesState (true);
    bothButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    bothButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    bothButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    bothButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    TextButton* rightButton  = new TextButton ("Right", "Output to right channel only");
    rightButton->setClickingTogglesState (true);
    rightButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    rightButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    rightButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    rightButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    outputChannelButtonManager = std::make_unique<LinearButtonGroupManager>();
    outputChannelButtonManager->addButton (leftButton);
    outputChannelButtonManager->addButton (bothButton);
    outputChannelButtonManager->addButton (rightButton);
    outputChannelButtonManager->setRadioButtonMode (true);
    outputChannelButtonManager->setButtonListener (this);
    outputChannelButtonManager->setButtonsLookAndFeel (m_materialButtonLookAndFeel);
    outputChannelButtonManager->setColour (ButtonGroupManager::backgroundColourId,   Colours::white);
    outputChannelButtonManager->setColour (ButtonGroupManager::outlineColourId,      Colour (0x0));
    outputChannelButtonManager->setColour (LinearButtonGroupManager::accentColourId, COLOUR_ACCENT);
    outputChannelButtonManager->setBounds (20, 70, 140, 20);
    addAndMakeVisible (outputChannelButtonManager.get());

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
        auto* channelSelector = new PopupChannelSelector(this, channelStates);
        channelSelector->setMaximumSelectableChannels(4);
        channelSelector->setChannelButtonColour(Colour(0, 174, 239));
    
        CallOutBox& myBox
            = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), channelSelectButton->getScreenBounds(), nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);
        
    }

    else if (button == muteButton.get())
    {
        if (muteButton->getToggleState())
        {
            audioMonitor->setParameter (2, 0.0f);
            LOGD("Mute on.");
        }
        else
        {
            audioMonitor->setParameter (2, 100.0f);
            LOGD("Mute off.");
        }
    }
}


void AudioMonitorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    const String buttonName = buttonThatWasClicked->getName().toLowerCase();

    if (buttonName.startsWith ("left"))
    {
        
    }
    else if (buttonName.startsWith ("both"))
    {
       
    }

    else if (buttonName.startsWith ("right"))
    {
       
    }

    GenericEditor::buttonClicked (buttonThatWasClicked);
}


void AudioMonitorEditor::channelStateChanged(Array<int> activeChannels)
{    
    std::cout << "[Audio Monitor] Selected Channels: (";

    for(int i = 0; i < channelStates.size(); i++)
    {
        if(activeChannels.contains(i+1) && !channelStates.at(i))
        {
            std::cout << i+1 << ", ";
            channelStates.at(i) = true;
            audioMonitor->setChannelStatus(i, true);
        }
        else if(!activeChannels.contains(i+1) && channelStates.at(i))
        {
            channelStates.at(i) = false;
            audioMonitor->setChannelStatus(i, false);
        }
    }

    std::cout << ")" << std::endl;

    audioMonitor->dataChannelStates = channelStates;
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