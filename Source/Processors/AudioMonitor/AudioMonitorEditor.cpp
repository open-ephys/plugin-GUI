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

MonitorMuteButton::MonitorMuteButton(Parameter* param) : ParameterEditor(param)
{

    muteButton = new ImageButton("Mute Button");

    Image offimage = ImageCache::getFromMemory  (BinaryData::muteoff_png, BinaryData::muteoff_pngSize);
    Image onimage  = ImageCache::getFromMemory  (BinaryData::muteon_png,  BinaryData::muteon_pngSize);

    muteButton->setImages (false, true, true,
               offimage, 1.0f, Colours::black,
               offimage, 1.0f, Colours::black.withAlpha (0.0f),
               onimage,  1.0f, Colours::darkgrey);

    muteButton->setClickingTogglesState (true);

    muteButton->setTooltip ("Mute audio");

    muteButton->addListener(this);
    muteButton->setToggleState(false, false);

    addAndMakeVisible(muteButton);

    setBounds(0, 0, 20, 20);
}

void MonitorMuteButton::buttonClicked(Button*)
{
    param->setNextValue(muteButton->getToggleState());
}

void MonitorMuteButton::updateView()
{
    if (param != nullptr)
        muteButton->setToggleState(param->getValue(), dontSendNotification);
}

void MonitorMuteButton::resized()
{

    muteButton->setBounds(0, 0, 20, 20);
}


AudioOutputSelector::AudioOutputSelector(Parameter* param) : ParameterEditor(param)
{

    leftButton = new TextButton("Left", "Output to left channel only");
    leftButton->setClickingTogglesState(true);
    leftButton->setToggleState(false, dontSendNotification);
    leftButton->setColour(TextButton::buttonColourId, Colour(0x0));
    leftButton->setColour(TextButton::buttonOnColourId, Colour(0x0));
    leftButton->setColour(TextButton::textColourOffId, COLOUR_PRIMARY);
    leftButton->setColour(TextButton::textColourOnId, COLOUR_ACCENT);

    bothButton = new TextButton("Both", "Output to both channels");
    bothButton->setClickingTogglesState(true);
    bothButton->setToggleState(true, dontSendNotification);
    bothButton->setColour(TextButton::buttonColourId, Colour(0x0));
    bothButton->setColour(TextButton::buttonOnColourId, Colour(0x0));
    bothButton->setColour(TextButton::textColourOffId, COLOUR_PRIMARY);
    bothButton->setColour(TextButton::textColourOnId, COLOUR_ACCENT);

    rightButton = new TextButton("Right", "Output to right channel only");
    rightButton->setClickingTogglesState(true);
    rightButton->setToggleState(false, dontSendNotification);
    rightButton->setColour(TextButton::buttonColourId, Colour(0x0));
    rightButton->setColour(TextButton::buttonOnColourId, Colour(0x0));
    rightButton->setColour(TextButton::textColourOffId, COLOUR_PRIMARY);
    rightButton->setColour(TextButton::textColourOnId, COLOUR_ACCENT);

    outputChannelButtonManager = std::make_unique<LinearButtonGroupManager>();
    outputChannelButtonManager->addButton(leftButton);
    outputChannelButtonManager->addButton(bothButton);
    outputChannelButtonManager->addButton(rightButton);
    outputChannelButtonManager->setRadioButtonMode(true);
    outputChannelButtonManager->setButtonListener(this);
    outputChannelButtonManager->setButtonsLookAndFeel(m_materialButtonLookAndFeel.get());
    outputChannelButtonManager->setColour(ButtonGroupManager::backgroundColourId, Colours::white);
    outputChannelButtonManager->setColour(ButtonGroupManager::outlineColourId, Colour(0x0));
    outputChannelButtonManager->setColour(LinearButtonGroupManager::accentColourId, COLOUR_ACCENT);
    addAndMakeVisible(outputChannelButtonManager.get());

    setBounds(0, 0, 140, 20);
}


void AudioOutputSelector::buttonClicked(Button* buttonThatWasClicked)
{

    const String buttonName = buttonThatWasClicked->getName().toLowerCase();

    if (buttonName.startsWith("left"))
    {
        param->setNextValue(0);
        LOGD("Left channel only");
    }
    else if (buttonName.startsWith("both"))
    {
        param->setNextValue(2);
        LOGD("Both channels");
    }

    else if (buttonName.startsWith("right"))
    {
        param->setNextValue(1);
        LOGD("Right channel only");
    }
}

void AudioOutputSelector::updateView()
{
    if (param != nullptr)
    {

        if (int(param->getValue()) == 0)
        {
            leftButton->setToggleState(true, dontSendNotification);
        }
        else if (int(param->getValue()) == 1)
        {
            rightButton->setToggleState(true, dontSendNotification);
        }
        else {
            bothButton->setToggleState(true, dontSendNotification);
        }
    }
       
}

void AudioOutputSelector::resized()
{
    outputChannelButtonManager->setBounds(0, 0, 140, 20);
}

AudioMonitorEditor::AudioMonitorEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)
{
    audioMonitor = static_cast<AudioMonitor*>(parentNode);
    
    addSelectedChannelsParameterEditor("selected_channels", 10, 35);

    Parameter* muteParam = parentNode->getGlobalParameter("mute_audio");
    addCustomParameterEditor(new MonitorMuteButton(muteParam), 135, 35);

    Parameter* outputParam = parentNode->getGlobalParameter("audio_output");
    addCustomParameterEditor(new AudioOutputSelector(outputParam), 20, 65);

    spikeChan = std::make_unique<ComboBox>("Spike Channels");
    spikeChan->setBounds(20, 100, 140, 20);

    for (int i = 0; i < audioMonitor->getTotalSpikeChannels() ; i++)
	{
		spikeChan->addItem(audioMonitor->getSpikeChannel(i)->getName(), i + 1);
	}
    
    spikeChan->setTextWhenNoChoicesAvailable("No spike channels");
    spikeChan->setTextWhenNothingSelected("Select a Spike Channel");

	//spikeChan->addListener(this);
	addAndMakeVisible(spikeChan.get());

    desiredWidth = 180;
}