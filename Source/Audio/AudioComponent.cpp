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
	
	initialise(0,  // numInputChannelsNeeded
			   2,  // numOutputChannelsNeeded
			   0,  // *savedState (XmlElement)
			   true, // selectDefaultDeviceOnFailure
			   String::empty, // preferred device
			   0); // preferred device setup options
	
	AudioIODevice* aIOd = getCurrentAudioDevice();

	std::cout << "Got audio device." << std::endl;

	String devType = getCurrentAudioDeviceType();
	String devName = aIOd->getName();
	
	std::cout << std::endl << "Audio device name: " << devName << std::endl;

	AudioDeviceManager::AudioDeviceSetup setup;
	getAudioDeviceSetup(setup);

	setup.bufferSize = 2048; /// larger buffer = fewer empty blocks, but longer latencies
	setup.useDefaultInputChannels = false;
	setup.inputChannels = 0;
	setup.useDefaultOutputChannels = true;
	setup.outputChannels = 2;
	setup.sampleRate = 44100.0;

	String msg = setAudioDeviceSetup(setup, false);

	devType = getCurrentAudioDeviceType();
	std::cout << "Audio device type: " << devType << std::endl;

	float sr = setup.sampleRate;
	int buffSize = setup.bufferSize;
	String oDN = setup.outputDeviceName;
	BigInteger oC = setup.outputChannels;

	std::cout << "Audio output channels: " <<  oC.toInteger() << std::endl;
	std::cout << "Audio device sample rate: " <<  sr << std::endl;
	std::cout << "Audio device buffer size: " << buffSize << std::endl << std::endl;

	graphPlayer = new AudioProcessorPlayer();

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

void AudioComponent::beginCallbacks() {
	
	std::cout << std::endl << "Adding audio callback." << std::endl;
	addAudioCallback(graphPlayer);
	isPlaying = true;

}

void AudioComponent::endCallbacks() {
	
	std::cout << std::endl << "Removing audio callback." << std::endl;
	removeAudioCallback(graphPlayer);
	isPlaying = false;

}

