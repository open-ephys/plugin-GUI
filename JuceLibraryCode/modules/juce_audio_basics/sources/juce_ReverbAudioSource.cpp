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

ReverbAudioSource::ReverbAudioSource (AudioSource* const inputSource, const bool deleteInputWhenDeleted)
   : input (inputSource, deleteInputWhenDeleted),
     bypass (false)
{
    jassert (inputSource != nullptr);
}

ReverbAudioSource::~ReverbAudioSource() {}

void ReverbAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    const ScopedLock sl (lock);
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);
    reverb.setSampleRate (sampleRate);
}

void ReverbAudioSource::releaseResources() {}

void ReverbAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    const ScopedLock sl (lock);

    input->getNextAudioBlock (bufferToFill);

    if (! bypass)
    {
        float* const firstChannel = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);

        if (bufferToFill.buffer->getNumChannels() > 1)
        {
            reverb.processStereo (firstChannel,
                                  bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample),
                                  bufferToFill.numSamples);
        }
        else
        {
            reverb.processMono (firstChannel, bufferToFill.numSamples);
        }
    }
}

void ReverbAudioSource::setParameters (const Reverb::Parameters& newParams)
{
    const ScopedLock sl (lock);
    reverb.setParameters (newParams);
}

void ReverbAudioSource::setBypassed (bool b) noexcept
{
    if (bypass != b)
    {
        const ScopedLock sl (lock);
        bypass = b;
        reverb.reset();
    }
}

} // namespace juce
