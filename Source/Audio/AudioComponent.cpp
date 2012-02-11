/*
  ==============================================================================

    AudioComponent.cpp
    Created: 7 May 2011 1:35:05pm
    Author:  jsiegle

  ==============================================================================
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

