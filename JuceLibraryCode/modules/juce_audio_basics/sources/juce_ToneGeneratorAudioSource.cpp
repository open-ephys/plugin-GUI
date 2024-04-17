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

ToneGeneratorAudioSource::ToneGeneratorAudioSource()
    : frequency (1000.0),
      sampleRate (44100.0),
      currentPhase (0.0),
      phasePerSample (0.0),
      amplitude (0.5f)
{
}

ToneGeneratorAudioSource::~ToneGeneratorAudioSource()
{
}

//==============================================================================
void ToneGeneratorAudioSource::setAmplitude (const float newAmplitude)
{
    amplitude = newAmplitude;
}

void ToneGeneratorAudioSource::setFrequency (const double newFrequencyHz)
{
    frequency = newFrequencyHz;
    phasePerSample = 0.0;
}

//==============================================================================
void ToneGeneratorAudioSource::prepareToPlay (int /*samplesPerBlockExpected*/, double rate)
{
    currentPhase = 0.0;
    phasePerSample = 0.0;
    sampleRate = rate;
}

void ToneGeneratorAudioSource::releaseResources()
{
}

void ToneGeneratorAudioSource::getNextAudioBlock (const AudioSourceChannelInfo& info)
{
    if (approximatelyEqual (phasePerSample, 0.0))
        phasePerSample = MathConstants<double>::twoPi / (sampleRate / frequency);

    for (int i = 0; i < info.numSamples; ++i)
    {
        const float sample = amplitude * (float) std::sin (currentPhase);
        currentPhase += phasePerSample;

        for (int j = info.buffer->getNumChannels(); --j >= 0;)
            info.buffer->setSample (j, info.startSample + i, sample);
    }
}

} // namespace juce
