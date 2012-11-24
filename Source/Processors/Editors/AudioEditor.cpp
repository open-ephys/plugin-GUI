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

#include "AudioEditor.h"
#include "../../Audio/AudioComponent.h"


MuteButton::MuteButton()
	: ImageButton ("MuteButton")
{


	Image offimage = ImageCache::getFromMemory (BinaryData::muteoff_png, BinaryData::muteoff_pngSize);
	Image onimage = ImageCache::getFromMemory (BinaryData::muteon_png, BinaryData::muteon_pngSize);

	setImages(false, true, true,
			offimage, 1.0f, Colours::white.withAlpha(0.0f),
			offimage, 1.0f, Colours::black.withAlpha(0.0f),
			onimage, 1.0f, Colours::white.withAlpha(0.0f));

	setClickingTogglesState(true);
}

MuteButton::~MuteButton()
{
}

AudioWindowButton::AudioWindowButton()
	: Button ("AudioWindowButton")
{
	setClickingTogglesState(true);

	MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    Typeface::Ptr typeface = new CustomTypeface(mis);
    font = Font(typeface);
    font.setHeight(12);
}

AudioWindowButton::~AudioWindowButton()
{
}

void AudioWindowButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	if (getToggleState())
		g.setColour(Colours::yellow);
	else
		g.setColour(Colours::black);

	g.setFont(font);
	//g.drawSingleLineText(" AUDIO",0,0);
	g.drawSingleLineText("AUDIO",0,15);
}

AudioEditor::AudioEditor (AudioNode* owner) 
	: AudioProcessorEditor (owner), lastValue(1.0f), acw(0)

{

	muteButton = new MuteButton();
	muteButton->addListener(this);
	muteButton->setToggleState(false,false);
	addAndMakeVisible(muteButton);

	audioWindowButton = new AudioWindowButton();
	audioWindowButton->addListener(this);
	audioWindowButton->setToggleState(false,false);
	addAndMakeVisible(audioWindowButton);

	volumeSlider = new Slider ("High-Cut Slider");
	volumeSlider->setRange(0,100,1);
	volumeSlider->addListener(this);
	volumeSlider->setTextBoxStyle(Slider::NoTextBox,
								false, 0, 0);
	addAndMakeVisible(volumeSlider);

	//acw = new AudioConfigurationWindow(getAudioComponent()->deviceManager, (Button*) audioWindowButton);

}

AudioEditor::~AudioEditor()
{
	deleteAllChildren();
	deleteAndZero(acw);
}

void AudioEditor::resized()
{
	muteButton->setBounds(0,0,30,25);
	volumeSlider->setBounds(35,0,100,getHeight());
	audioWindowButton->setBounds(140,0,200,getHeight());
}

bool AudioEditor::keyPressed (const KeyPress& key)
{
	//std::cout << name << " received " << key.getKeyCode() << std::endl;
}


void AudioEditor::buttonClicked(Button* button)
{
	if (button == muteButton)
	{

		if(muteButton->getToggleState()) {
			lastValue = volumeSlider->getValue();
			getAudioProcessor()->setParameter(1,0.0f);
			std::cout << "Mute on." << std::endl;
	    } else {
	    	getAudioProcessor()->setParameter(1,lastValue);
	    	std::cout << "Mute off." << std::endl;
	    }
	} else if (button == audioWindowButton)
	{
		if (audioWindowButton->getToggleState())
		{
			if (acw == 0) {
				
				// AudioComponent* audioComponent = getAudioComponent();
				// audioComponent->restartDevice();

				// if (audioComponent != 0) {
					acw = new AudioConfigurationWindow(getAudioComponent()->deviceManager, (Button*) audioWindowButton);
					acw->setUIComponent(getUIComponent());
				//}
			}

			getAudioComponent()->restartDevice();
			acw->setVisible(true);

		} else {

			acw->setVisible(false);
			//deleteAndZero(acw);
			getAudioComponent()->stopDevice();
		}
	}

}

void AudioEditor::sliderValueChanged(Slider* slider)
{
	getAudioProcessor()->setParameter(1,slider->getValue());
}

void AudioEditor::paint (Graphics& g)
{
	//g.setColour(Colours::grey);
	// g.fillRect(1,1,getWidth()-2,getHeight()-2);
}



AudioConfigurationWindow::AudioConfigurationWindow(AudioDeviceManager& adm, Button* cButton)
	: DocumentWindow ("Audio Settings", 
					  Colours::red, 
					  DocumentWindow::closeButton),
	  controlButton(cButton)

{
	centreWithSize(360,300);
	setUsingNativeTitleBar(true);
	setResizable(false,false);

	//std::cout << "Audio CPU usage:" << adm.getCpuUsage() << std::endl;

	AudioDeviceSelectorComponent* adsc = new AudioDeviceSelectorComponent 
								(adm,
								 0, // minAudioInputChannels
								 2, // maxAudioInputChannels
								 0, // minAudioOutputChannels
								 2, // maxAudioOutputChannels
								 false, // showMidiInputOptions
								 false, // showMidiOutputSelector
								 false, // showChannelsAsStereoPairs
								 false); // hideAdvancedOptionsWithButton

	adsc->setBounds(0,0,450,240);

	setContentComponent (adsc, true, true);
	setVisible(false);
	//setContentComponentSize(getWidth(), getHeight());
}

AudioConfigurationWindow::~AudioConfigurationWindow()
{
	setContentComponent (0);
	//eleteAndZero(deviceManager);
//	deleteAndZero (deviceSelector);
}

void AudioConfigurationWindow::closeButtonPressed()
{
	controlButton->setToggleState(false,false);
	getAudioComponent()->stopDevice();
	setVisible(false);
}

void AudioConfigurationWindow::resized()
{
	//deviceSelector->setBounds (8, 8, getWidth() - 16, getHeight() - 16);
}

void AudioConfigurationWindow::paint(Graphics& g)
{
	g.fillAll(Colours::darkgrey);
}
