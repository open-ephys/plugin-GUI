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


#include "AudioComponent.h"
#include <stdio.h>

AudioComponent::AudioComponent() : isPlaying(false)
{
	// if this is nonempty, we got an error
	String error = deviceManager.initialise(0,  // numInputChannelsNeeded
						2,  // numOutputChannelsNeeded
						0,  // *savedState (XmlElement)
						true, // selectDefaultDeviceOnFailure
						String::empty, // preferred device
						0); // preferred device setup options
	if (error != String::empty)
	{
		String titleMessage = String("Audio device initialization error");
		String contentMessage = String("There was a problem initializing the audio device:\n" + error);
		// this uses a bool since there are only two options
		// also, omitting parameters works fine, even though the docs don't show defaults
		bool retryButtonClicked = AlertWindow::showOkCancelBox(AlertWindow::QuestionIcon,
								       titleMessage,
								       contentMessage,
								       String("Retry"),
								       String("Quit"));

		if (retryButtonClicked)
		{
			// as above
			error = deviceManager.initialise(0, 2, 0, true, String::empty, 0);
		} else { // quit button clicked
			JUCEApplication::quit();
		}
	}
							    
							    
	AudioIODevice* aIOd = deviceManager.getCurrentAudioDevice();

	// the error string doesn't tell you if there's no audio device found...
	if (aIOd == 0)
	{
	  String titleMessage = String("No audio device found");
	  String contentMessage = String("Couldn't find an audio device. ") +
	    String("Perhaps some other program has control of the default one.");
	  AlertWindow::showMessageBox(AlertWindow::InfoIcon,
				      titleMessage,
				      contentMessage);
	  JUCEApplication::quit();
	}
					   

	std::cout << "Got audio device." << std::endl;

	String devName = aIOd->getName();
	
	std::cout << std::endl << "Audio device name: " << devName << std::endl;

	AudioDeviceManager::AudioDeviceSetup setup;
	deviceManager.getAudioDeviceSetup(setup);

	setup.bufferSize = 1024; /// larger buffer = fewer empty blocks, but longer latencies
	setup.useDefaultInputChannels = false;
	setup.inputChannels = 0;
	setup.useDefaultOutputChannels = true;
	setup.outputChannels = 2;
	setup.sampleRate = 44100.0;

	String msg = deviceManager.setAudioDeviceSetup(setup, false);

	String devType = deviceManager.getCurrentAudioDeviceType();
	std::cout << "Audio device type: " << devType << std::endl;

	float sr = setup.sampleRate;
	int buffSize = setup.bufferSize;
	String oDN = setup.outputDeviceName;
	BigInteger oC = setup.outputChannels;

	std::cout << "Audio output channels: " <<  oC.toInteger() << std::endl;
	std::cout << "Audio device sample rate: " <<  sr << std::endl;
	std::cout << "Audio device buffer size: " << buffSize << std::endl << std::endl;

	graphPlayer = new AudioProcessorPlayer();

	stopDevice(); // reduces the amount of background processing when
				  // device is not in use

}

AudioComponent::~AudioComponent() {
	
	if (callbacksAreActive())
		endCallbacks();

	deleteAndZero(graphPlayer);

}

void AudioComponent::connectToProcessorGraph(AudioProcessorGraph* processorGraph)
{
	
	graphPlayer->setProcessor(processorGraph);

}

void AudioComponent::disconnectProcessorGraph()
{
	
	graphPlayer->setProcessor(0);

}

bool AudioComponent::callbacksAreActive() {
	return isPlaying;
}

void AudioComponent::restartDevice()
{
	deviceManager.restartLastAudioDevice();

}

void AudioComponent::stopDevice()
{

	deviceManager.closeAudioDevice();
}

void AudioComponent::beginCallbacks() {
	
	restartDevice();

	std::cout << std::endl << "Adding audio callback." << std::endl;
	deviceManager.addAudioCallback(graphPlayer);
	isPlaying = true;

}

void AudioComponent::endCallbacks() {
	
	std::cout << std::endl << "Removing audio callback." << std::endl;
	deviceManager.removeAudioCallback(graphPlayer);
	isPlaying = false;

	stopDevice();

}

