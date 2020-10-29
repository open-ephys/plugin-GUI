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


#include "AudioComponent.h"
#include <stdio.h>


#include "../Utils/Utils.h"

AudioComponent::AudioComponent() : isPlaying(false)
{
    bool initialized = false;
    while (!initialized)
    {
        // if this is nonempty, we got an error
        String error = deviceManager.initialise(0,  // numInputChannelsNeeded
            2,  // numOutputChannelsNeeded
            0,  // *savedState (XmlElement)
            true, // selectDefaultDeviceOnFailure
            String::empty, // preferred device
            0); // preferred device setup options

        if (error == String::empty)
        {
            initialized = true;
        }
        else
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

            if (!retryButtonClicked) // quit button clicked
            {
                JUCEApplication::quit();
                break;
            }
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


    LOGD("Got audio device.");

    String devName = aIOd->getName();

    LOGD("Audio device name: ", devName);

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
    LOGD("Audio device type: ", devType);

    float sr = setup.sampleRate;
    int buffSize = setup.bufferSize;
    String oDN = setup.outputDeviceName;
    BigInteger oC = setup.outputChannels;

    LOGD("Audio output channels: ", oC.toInteger());
    LOGD("Audio device sample rate: ", sr);
    LOGD("Audio device buffer size: ", buffSize);

    graphPlayer = new AudioProcessorPlayer();

    stopDevice(); // reduces the amount of background processing when
    // device is not in use


}

AudioComponent::~AudioComponent()
{

    if (callbacksAreActive())
        endCallbacks();

}

int AudioComponent::getBufferSize()
{
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup(setup);

    return setup.bufferSize;
}

int AudioComponent::getBufferSizeMs()
{
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup(setup);

    return int(float(setup.bufferSize)/setup.sampleRate*1000);
}

void AudioComponent::connectToProcessorGraph(AudioProcessorGraph* processorGraph)
{

    graphPlayer->setProcessor(processorGraph);

}

void AudioComponent::disconnectProcessorGraph()
{

    graphPlayer->setProcessor(0);

}

bool AudioComponent::callbacksAreActive()
{
    return isPlaying;
}

void AudioComponent::restartDevice()
{
    deviceManager.restartLastAudioDevice();

}

void AudioComponent::stopDevice()
{

    //deviceManager.closeAudioDevice();
}

void AudioComponent::beginCallbacks()
{

    if (!isPlaying)
    {

        //const MessageManagerLock mmLock;
        // MessageManagerLock mml (Thread::getCurrentThread());

        // if (mml.lockWasGained())
        // {
        //LOGDD("AUDIO COMPONENT GOT THAT LOCK!");
        // } else {
        //LOGDD("AUDIO COMPONENT COULDN'T GET THE LOCK...RETURNING.");
        //     return;
        // }

        //     MessageManager* mm = MessageManager::getInstance();

        //     if (mm->isThisTheMessageThread())
        //LOGDD("THIS IS THE MESSAGE THREAD -- AUDIO COMPONENT");
        //     else
        //LOGDD("NOT THE MESSAGE THREAD -- AUDIO COMPONENT");



        restartDevice();

        int64 ms = Time::getCurrentTime().toMilliseconds();

        while (Time::getCurrentTime().toMilliseconds() - ms < 100)
        {
            // pause to let things finish up

        }


        LOGD("Adding audio callback.");
        deviceManager.addAudioCallback(graphPlayer);
        isPlaying = true;
    }
    else
    {
        LOGD("beginCallbacks was called while acquisition was active.");
    }

    //int64 ms = Time::getCurrentTime().toMilliseconds();

    //while(Time::getCurrentTime().toMilliseconds() - ms < 100)
    //{
    // pause to let things finish up

    // }

}

void AudioComponent::endCallbacks()
{

    const MessageManagerLock mmLock; // add a lock to prevent crashes

    MessageManagerLock mml (Thread::getCurrentThread());

    if (mml.lockWasGained())
    {
        LOGDD("AUDIO COMPONENT GOT THAT LOCK!");
    }

    MessageManager* mm = MessageManager::getInstance();

    if (mm->isThisTheMessageThread())
    {
        LOGDD("THIS IS THE MESSAGE THREAD -- AUDIO COMPONENT");
    }
    else
    {
        LOGDD("NOT THE MESSAGE THREAD -- AUDIO COMPONENT");
    }


    LOGD("Removing audio callback.");
    deviceManager.removeAudioCallback(graphPlayer);
    isPlaying = false;

    stopDevice();

    int64 ms = Time::getCurrentTime().toMilliseconds();

    while (Time::getCurrentTime().toMilliseconds() - ms < 50)
    {
        // pause to let things finish up

    }


}

void AudioComponent::saveStateToXml(XmlElement* parent)
{
    // JUCE's audioState XML format (includes all info)
    ScopedPointer<XmlElement> audioState = deviceManager.createStateXml();

    if (audioState != nullptr)
    {
        parent->addChildElement(audioState.release());
    }

    // Save type, buffer size, and sample rate as attributes separately
    // in case part of the audioState is invalid later, like the specific device names.
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup(setup);

    parent->setAttribute("sampleRate", setup.sampleRate);
    parent->setAttribute("bufferSize", setup.bufferSize);
    parent->setAttribute("deviceType", deviceManager.getCurrentAudioDeviceType());
}

void AudioComponent::loadStateFromXml(XmlElement* parent)
{
    forEachXmlChildElement(*parent, child)
    {
        if (!child->isTextElement())
        {
            deviceManager.closeAudioDevice(); // necessary to ensure correct device type gets created

            String error = deviceManager.initialise(
                0,              // numInputChannelsNeeded
                2,              // numOutputChannelsNeeded
                child,          // savedState
                true,           // selectDefaultDeviceOnFailure
                String::empty,  // preferred device
                nullptr);       // preferred device setup options

            if (error.isEmpty())
            {
                break;
            }
        }
    }

    // Now the important parameters separately, as a backup (in case the devices have different names or something)
    String deviceType = parent->getStringAttribute("deviceType", String::empty);
    if (!deviceType.isEmpty())
    {
        deviceManager.setCurrentAudioDeviceType(deviceType, true);
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup(setup);

    double sampleRate = parent->getDoubleAttribute("sampleRate");
    if (sampleRate > 0)
    {
        setup.sampleRate = sampleRate;
    }

    int bufferSize = parent->getIntAttribute("bufferSize");
    if (bufferSize > 16 && bufferSize < 6000)
    {
        setup.bufferSize = bufferSize;
    }
    else
    {
    LOGD("Buffer size out of range.");
    }

    deviceManager.setAudioDeviceSetup(setup, true);
}