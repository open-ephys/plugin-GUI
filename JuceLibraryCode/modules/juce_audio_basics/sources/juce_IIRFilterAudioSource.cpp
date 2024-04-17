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

IIRFilterAudioSource::IIRFilterAudioSource (AudioSource* const inputSource,
                                            const bool deleteInputWhenDeleted)
    : input (inputSource, deleteInputWhenDeleted)
{
    jassert (inputSource != nullptr);

    for (int i = 2; --i >= 0;)
        iirFilters.add (new IIRFilter());
}

IIRFilterAudioSource::~IIRFilterAudioSource()  {}

//==============================================================================
void IIRFilterAudioSource::setCoefficients (const IIRCoefficients& newCoefficients)
{
    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->setCoefficients (newCoefficients);
}

void IIRFilterAudioSource::makeInactive()
{
    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->makeInactive();
}

//==============================================================================
void IIRFilterAudioSource::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    input->prepareToPlay (samplesPerBlockExpected, sampleRate);

    for (int i = iirFilters.size(); --i >= 0;)
        iirFilters.getUnchecked (i)->reset();
}

void IIRFilterAudioSource::releaseResources()
{
    input->releaseResources();
}

void IIRFilterAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    input->getNextAudioBlock (bufferToFill);

    const int numChannels = bufferToFill.buffer->getNumChannels();

    while (numChannels > iirFilters.size())
        iirFilters.add (new IIRFilter (*iirFilters.getUnchecked (0)));

    for (int i = 0; i < numChannels; ++i)
        iirFilters.getUnchecked (i)
            ->processSamples (bufferToFill.buffer->getWritePointer (i, bufferToFill.startSample),
                              bufferToFill.numSamples);
}

} // namespace juce
