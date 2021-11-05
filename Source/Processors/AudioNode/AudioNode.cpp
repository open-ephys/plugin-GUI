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

#include <cmath>

#include "AudioNode.h"

AudioNode::AudioNode()
    : GenericProcessor("Audio Node"), audioEditor(nullptr), volume(0.00001f), noiseGateLevel(0.0f),
    connectedProcessors(0)
{

    resetConnections();

}


AudioNode::~AudioNode()
{
    std::cout << "AUDIO NODE BEING DELETED." << std::endl;
}

AudioProcessorEditor* AudioNode::createEditor()
{

    audioEditor = std::make_unique<AudioEditor>(this);

    return audioEditor.get();

}

void AudioNode::resetConnections()
{

    connectedProcessors = 0;
    
    updatePlaybackBuffer();
}

void AudioNode::registerProcessor(const GenericProcessor* sourceNode)
{
    connectedProcessors++;
}

void AudioNode::updateBufferSize()
{
    //AudioEditor* editor = (AudioEditor*) getEditor();
    audioEditor->updateBufferSizeText();

}


void AudioNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{
    nextAvailableChannel++;
    //auto sourceChannel = sourceNode->getAudioChannel(chan);
    //auto sourceChannelCopy = new ContinuousChannel(*sourceChannel);
    
    //continuousChannels.set(chan, sourceChannelCopy);

}

void AudioNode::updatePlaybackBuffer()
{
	setPlayConfigDetails(2, 2, 44100.0, 128);
}

void AudioNode::setParameter(int parameterIndex, float newValue)
{
    // change left channel, right channel, or volume
    if (parameterIndex == 1)
    {
        // volume level
        volume = newValue*0.1f;

    }
    else if (parameterIndex == 2)
    {
        // noiseGateLevel level

        expander.setThreshold(newValue); // in microVolts

    }
}

bool AudioNode::startAcquisition()
{
	return true;
}

void AudioNode::process(AudioBuffer<float>& buffer)
{

    if (connectedProcessors > 0)
    {
        float gain;

        int valuesNeeded = buffer.getNumSamples(); // samples needed to fill out the buffer

        int nInputs = buffer.getNumChannels();

        if (nInputs > 0) // we have some channels
        {

            for (int i = 0; i < nInputs; i++) // cycle through them all
            {
                gain = volume / (float(0x7fff) * 0.2);

                // Data are floats in units of microvolts, so dividing by bitVolts and 0x7fff (max value for 16b signed)
                // rescales to between -1 and +1. Audio output starts So, maximum gain applied to maximum data would be 10.

                buffer.applyGain(i, 0, valuesNeeded, gain);

                // Simple implementation of a "noise gate" on audio output
                expander.process(buffer.getWritePointer(i), // expand the left/right channel
                    buffer.getNumSamples());
            } // end cycling through channels

        }
    }
    else {
        buffer.clear();
    }
    
}


Expander::Expander()
{
    threshold = 1.f;
    output = 1.f;

    env = 0.f;
    gain = 1.f;

    setAttack(1.0f);
    setRelease(1.0f);
    setRatio(1.2); // ratio > 1.0 will decrease gain below threshold
}

void Expander::setThreshold(float value)
{
    threshold = value;
    transfer_B = output * pow(threshold, -transfer_A);

    LOGD("Threshold set to ", threshold);
    LOGD("transfer_B set to ", transfer_B);
}


void Expander::setRatio(float value)
{
    transfer_A = value - 1.f;
    transfer_B = output * pow(threshold, -transfer_A);
}


void Expander::setAttack(float value)
{
    attack = exp(-1.f/value);
}


void Expander::setRelease(float value)
{
    release = exp(-1.f/value);
    envelope_decay = exp(-4.f/value); /* = exp(-1/(0.25*value)) */
}


void Expander::process(float* sampleData, int numSamples)
{
    float det, transfer_gain;

    for (int i = 0; i < numSamples; i++)
    {
        det = fabs(sampleData[i]);
        det += 10e-30f; /* add tiny DC offset (-600dB) to prevent denormals */

        env = det >= env ? det : det + envelope_decay*(env-det);

        transfer_gain = env < threshold ? pow(env, transfer_A) * transfer_B : output;

        gain = transfer_gain < gain ?
               transfer_gain + attack * (gain - transfer_gain) :
               transfer_gain + release * (gain - transfer_gain);

        sampleData[i] = sampleData[i] * gain;
    }
}
