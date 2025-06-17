/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
#include "../AccessClass.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"
#include <stdio.h>

#include "../CoreServices.h"
#include "../Utils/Utils.h"

AudioComponent::AudioComponent() : isPlaying (false)
{
    AccessClass::setAudioComponent (this);

    bool initialized = false;
    while (! initialized)
    {
        // if this is nonempty, we got an error
        String error = deviceManager.initialise (0, // numInputChannelsNeeded
                                                 2, // numOutputChannelsNeeded
                                                 0, // *savedState (XmlElement)
                                                 true, // selectDefaultDeviceOnFailure
                                                 String(), // preferred device
                                                 0); // preferred device setup options

        if (error == String())
        {
            initialized = true;
        }
        else
        {
            String titleMessage = String ("Audio device initialization error");
            String contentMessage = String ("There was a problem initializing the audio device:\n" + error);
            // this uses a bool since there are only two options
            // also, omitting parameters works fine, even though the docs don't show defaults
            bool retryButtonClicked = AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                                                    titleMessage,
                                                                    contentMessage,
                                                                    String ("Retry"),
                                                                    String ("Quit"));

            if (! retryButtonClicked) // quit button clicked
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
        String titleMessage = String ("No audio device found");
        String contentMessage = String ("Couldn't find an audio device. ") + String ("Perhaps some other program has control of the default one.");
        AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                     titleMessage,
                                     contentMessage);
        JUCEApplication::quit();
    }

    String devName = aIOd->getName();

    LOGC ("Audio device name: ", devName);

    AudioDeviceManager::AudioDeviceSetup setup;
    setup = deviceManager.getAudioDeviceSetup();

    setup.bufferSize = 1024; /// larger buffer = fewer empty blocks, but longer latencies
    setup.useDefaultInputChannels = false;
    setup.inputChannels = 0;
    setup.useDefaultOutputChannels = true;
    setup.outputChannels = 2;
    setup.sampleRate = 44100.0;

    String msg = deviceManager.setAudioDeviceSetup (setup, false);

    if (msg.isNotEmpty())
        LOGE (msg);

    String devType = deviceManager.getCurrentAudioDeviceType();
    LOGC ("Audio device type: ", devType);

    float sr = setup.sampleRate;
    int buffSize = setup.bufferSize;
    String oDN = setup.outputDeviceName;
    BigInteger oC = setup.outputChannels;

    LOGC ("Audio output channels: ", oC.toInteger());
    LOGC ("Audio device sample rate: ", sr);
    LOGC ("Audio device buffer size: ", buffSize);
    std::cout << std::endl;

    graphPlayer = std::make_unique<AudioProcessorPlayer>();
}

AudioComponent::~AudioComponent()
{
    if (callbacksAreActive())
        endCallbacks();
}

int AudioComponent::getBufferSize()
{
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    return setup.bufferSize;
}

void AudioComponent::setBufferSize (int bufferSize)
{
    if (callbacksAreActive())
    {
        CoreServices::sendStatusMessage ("Cannot set buffer size while acquisition is active.");
        return;
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    setup.bufferSize = bufferSize;

    deviceManager.setAudioDeviceSetup (setup, true);

    CoreServices::sendStatusMessage ("Set buffer size to " + String (deviceManager.getAudioDeviceSetup().bufferSize) + " samples.");
}

int AudioComponent::getBufferSizeMs()
{
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    return int (float (setup.bufferSize) / setup.sampleRate * 1000);
}

int AudioComponent::getSampleRate()
{
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    return setup.sampleRate;
}

void AudioComponent::setSampleRate (int sampleRate)
{
    if (callbacksAreActive())
    {
        CoreServices::sendStatusMessage ("Cannot set sample rate while acquisition is active.");
        return;
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    setup.sampleRate = sampleRate;

    deviceManager.setAudioDeviceSetup (setup, true);

    CoreServices::sendStatusMessage ("Set sample rate to " + String (deviceManager.getAudioDeviceSetup().sampleRate) + " Hz.");
}

String AudioComponent::getDeviceName()
{
    return deviceManager.getAudioDeviceSetup().outputDeviceName;
}

void AudioComponent::setDeviceName (String deviceName)
{
    if (callbacksAreActive())
    {
        CoreServices::sendStatusMessage ("Cannot set device name while acquisition is active.");
        return;
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    setup.outputDeviceName = deviceName;

    deviceManager.setAudioDeviceSetup (setup, true);

    CoreServices::sendStatusMessage ("Set device name to " + deviceName);
}

String AudioComponent::getDeviceType()
{
    return deviceManager.getCurrentAudioDeviceType();
}

void AudioComponent::setDeviceType (String deviceType)
{
    if (callbacksAreActive())
    {
        CoreServices::sendStatusMessage ("Cannot set device type while acquisition is active.");
        return;
    }

    deviceManager.setCurrentAudioDeviceType (deviceType, true);

    CoreServices::sendStatusMessage ("Set device type to " + String (deviceManager.getCurrentDeviceTypeObject()->getTypeName()));
}

Array<double> AudioComponent::getAvailableSampleRates()
{
    return deviceManager.getCurrentAudioDevice()->getAvailableSampleRates();
}

Array<int> AudioComponent::getAvailableBufferSizes()
{
    return deviceManager.getCurrentAudioDevice()->getAvailableBufferSizes();
}

void AudioComponent::connectToProcessorGraph (AudioProcessorGraph* processorGraph)
{
    graphPlayer->setProcessor (processorGraph);
}

void AudioComponent::disconnectProcessorGraph()
{
    graphPlayer->setProcessor (0);
}

bool AudioComponent::callbacksAreActive()
{
    return isPlaying;
}

bool AudioComponent::checkForDevice()
{
    if (deviceManager.getCurrentAudioDevice() == nullptr)
    {
        AudioDeviceManager::AudioDeviceSetup ads = deviceManager.getAudioDeviceSetup();
        if (ads.outputDeviceName.isEmpty() && ads.inputDeviceName.isEmpty())
            return false;
    }

    return true;
}

bool AudioComponent::restartDevice()
{
    deviceManager.restartLastAudioDevice();

    //if (deviceManager.getCurrentAudioDevice() != nullptr)
    //{

    //    return true;
    //} else {
    //   LOGD("Could not find audio device.");
    //    return false;
    // }

    return true;
}

void AudioComponent::stopDevice()
{
    deviceManager.closeAudioDevice();
}

bool AudioComponent::beginCallbacks()
{
    if (! isPlaying)
    {
        if (restartDevice())
        {
            int64 ms = Time::getCurrentTime().toMilliseconds();

            while (Time::getCurrentTime().toMilliseconds() - ms < 100)
            {
                // pause to let device initialize
            }

            LOGD ("Adding audio callback.");
            deviceManager.addAudioCallback (graphPlayer.get());
            isPlaying = true;
            return true;
        }
        else
        {
            LOGE ("No audio device found. Cannot start callbacks.");
        }
    }
    else
    {
        LOGE ("beginCallbacks was called while acquisition was active.");
    }

    return false;
}

void AudioComponent::endCallbacks()
{
    LOGD ("Removing audio callback.");
    deviceManager.removeAudioCallback (graphPlayer.get());
    isPlaying = false;
}

void AudioComponent::saveStateToXml (XmlElement* parent)
{
    // JUCE's audioState XML format (includes all info)
    std::unique_ptr<XmlElement> audioState = deviceManager.createStateXml();

    if (audioState != nullptr)
    {
        parent->addChildElement (audioState.release());
    }

    // Save type, buffer size, and sample rate as attributes separately
    // in case part of the audioState is invalid later, like the specific device names.
    AudioDeviceManager::AudioDeviceSetup setup;
    deviceManager.getAudioDeviceSetup (setup);

    parent->setAttribute ("sampleRate", setup.sampleRate);
    parent->setAttribute ("bufferSize", setup.bufferSize);
    parent->setAttribute ("deviceType", deviceManager.getCurrentAudioDeviceType());
}

void AudioComponent::loadStateFromXml (XmlElement* parent)
{
    for (auto* child : parent->getChildIterator())
    {
        if (! child->isTextElement())
        {
            deviceManager.closeAudioDevice(); // necessary to ensure correct device type gets created

            String error = deviceManager.initialise (
                0, // numInputChannelsNeeded
                2, // numOutputChannelsNeeded
                child, // savedState
                true, // selectDefaultDeviceOnFailure
                String(), // preferred device
                nullptr); // preferred device setup options

            if (error.isEmpty())
            {
                break;
            }
        }
    }

    // Now the important parameters separately, as a backup (in case the devices have different names or something)
    String deviceType = parent->getStringAttribute ("deviceType", String());
    if (! deviceType.isEmpty())
    {
        deviceManager.setCurrentAudioDeviceType (deviceType, true);
    }

    AudioDeviceManager::AudioDeviceSetup setup;
    setup = deviceManager.getAudioDeviceSetup();

    double sampleRate = parent->getDoubleAttribute ("sampleRate");
    if (sampleRate > 0)
    {
        setup.sampleRate = sampleRate;
    }

    int bufferSize = parent->getIntAttribute ("bufferSize");
    if (bufferSize > 16 && bufferSize < 6000)
    {
        setup.bufferSize = bufferSize;
    }
    else
    {
        LOGE ("Buffer size out of range.");
    }

    String error = deviceManager.setAudioDeviceSetup (setup, true);

    if (! error.isEmpty())
    {
        LOGE ("Error loading audio device setup: " + error);
    }
}
