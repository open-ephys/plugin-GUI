/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

void AudioIODeviceCallback::audioDeviceIOCallbackWithContext ([[maybe_unused]] const float* const* inputChannelData,
                                                              [[maybe_unused]] int numInputChannels,
                                                              [[maybe_unused]] float* const* outputChannelData,
                                                              [[maybe_unused]] int numOutputChannels,
                                                              [[maybe_unused]] int numSamples,
                                                              [[maybe_unused]] const AudioIODeviceCallbackContext& context) {}

//==============================================================================
AudioIODevice::AudioIODevice (const String& deviceName, const String& deviceTypeName)
    : name (deviceName), typeName (deviceTypeName)
{
}

AudioIODevice::~AudioIODevice() {}

void AudioIODeviceCallback::audioDeviceError (const String&)    {}
bool AudioIODevice::setAudioPreprocessingEnabled (bool)         { return false; }
bool AudioIODevice::hasControlPanel() const                     { return false; }
int  AudioIODevice::getXRunCount() const noexcept               { return -1; }

bool AudioIODevice::showControlPanel()
{
    jassertfalse;    // this should only be called for devices which return true from
                     // their hasControlPanel() method.
    return false;
}

} // namespace juce
