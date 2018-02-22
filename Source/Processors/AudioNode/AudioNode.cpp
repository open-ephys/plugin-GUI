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
    : GenericProcessor("Audio Node"), audioEditor(0), volume(0.00001f), noiseGateLevel(0.0f)
{

    settings.numInputs = 4096;
    settings.numOutputs = 2;

    // 128 inputs, 2 outputs (left and right channel)
    setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

    nextAvailableChannel = 2; // keep first two channels empty

    tempBuffer = new AudioSampleBuffer(16, 1024);

}


AudioNode::~AudioNode()
{
}

AudioProcessorEditor* AudioNode::createEditor()
{

    audioEditor = new AudioEditor(this);

    return audioEditor;

}

void AudioNode::resetConnections()
{

    nextAvailableChannel = 2; // start connections at channel 2
    wasConnected = false;

    dataChannelArray.clear();

}

void AudioNode::updateBufferSize()
{
    //AudioEditor* editor = (AudioEditor*) getEditor();
    audioEditor->updateBufferSizeText();

}

void AudioNode::setChannel(const DataChannel* ch)
{

	int channelNum = getDataChannelIndex(ch->getSourceIndex(), ch->getSourceNodeID(), ch->getSubProcessorIdx());

    std::cout << "Audio node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);
}

void AudioNode::setChannelStatus(const DataChannel* chan, bool status)
{

    setChannel(chan); // add 2 to account for 2 output channels

    enableCurrentChannel(status);

}

void AudioNode::enableCurrentChannel(bool state)
{

    if (state)
    {
        setParameter(100, 0.0f);
    }
    else
    {
        setParameter(-100, 0.0f);
    }
}


void AudioNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{


    int channelIndex = getNextChannel(false);

    setPlayConfigDetails(channelIndex+1,0,44100.0,128);

    auto dataChannel = sourceNode->getDataChannel(chan);
    auto dataChannelCopy = new DataChannel(*dataChannel);
    dataChannelCopy->setMonitored(dataChannel->isMonitored());
    
    dataChannelArray.add(dataChannelCopy);

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
    else if (parameterIndex == 100)
    {

		dataChannelArray[currentChannel]->setMonitored(true);

    }
    else if (parameterIndex == -100)
    {

		dataChannelArray[currentChannel]->setMonitored(false);
    }

}

void AudioNode::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{


    // std::cout << "Processor sample rate: " << getSampleRate() << std::endl;
    // std::cout << "Audio card sample rate: " << sampleRate_ << std::endl;
    // std::cout << "Samples per block: " << estimatedSamplesPerBlock << std::endl;
	if (sampleRate_ != destBufferSampleRate || estimatedSamplesPerBlock != estimatedSamples)
	{
		destBufferSampleRate = sampleRate_;
		estimatedSamples = estimatedSamplesPerBlock;
		recreateBuffers();
	}

}

void AudioNode::recreateBuffers()
{
	numSamplesExpected.clear();
    samplesInBackupBuffer.clear();
    samplesInOverflowBuffer.clear();
    ratio.clear();
    filters.clear();
    bufferA.clear();
    bufferB.clear();
    bufferSwap.clear();

    for (int i = 0; i < dataChannelArray.size(); i++)
    {
        // processor sample rate divided by sound card sample rate
        numSamplesExpected.add((int)(dataChannelArray[i]->getSampleRate()/destBufferSampleRate*float(estimatedSamples)) + 1);
        samplesInBackupBuffer.add(0);
        samplesInOverflowBuffer.add(0);
        sourceBufferSampleRate.add(dataChannelArray[i]->getSampleRate());

        filters.add(new Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 1> (1024));

        ratio.add(float(numSamplesExpected[i])/float(estimatedSamples));
        updateFilter(i);

        bufferA.add(new AudioSampleBuffer(1,10000));
        bufferB.add(new AudioSampleBuffer(1,10000));
        bufferSwap.add(false);

    }

//    tempBuffer->setSize(getNumInputs(), 4096);
    tempBuffer->setSize(1, 4096);
}

bool AudioNode::enable()
{
	recreateBuffers();
	return true;
}

void AudioNode::updateFilter(int i)
{

    double cutoffFreq = (ratio[i] > 1.0) ? 2 * destBufferSampleRate  // downsample
                        : destBufferSampleRate / 2; // upsample

    double sampleFreq = (ratio[i] > 1.0) ? sourceBufferSampleRate[i] // downsample
                        : destBufferSampleRate;  // upsample

    Dsp::Params params;
    params[0] = sampleFreq; // sample rate
    params[1] = cutoffFreq; // cutoff frequency
    params[2] = 1.25; //Q //

    filters[i]->setParams(params);

}

void AudioNode::process(AudioSampleBuffer& buffer)
{
    float gain;
    int valuesNeeded = buffer.getNumSamples(); // samples needed to fill out the buffer

    //std::cout << "Buffer size: " << buffer.getNumChannels() << std::endl;

    // clear the left and right channels
    buffer.clear(0,0,buffer.getNumSamples());
    buffer.clear(1,0,buffer.getNumSamples());

    if (1)
    {

        AudioSampleBuffer* overflowBuffer;
        AudioSampleBuffer* backupBuffer;

        if (dataChannelArray.size() > 0) // we have some channels
        {

//            tempBuffer->clear();

            for (int i = 0; i < buffer.getNumChannels()-2; i++) // cycle through them all
            {
                
                if (dataChannelArray[i]->isMonitored())
                {
                    tempBuffer->clear();
                    //std::cout << "Processing channel " << i << std::endl;

                    if (!bufferSwap[i])
                    {
                        overflowBuffer = bufferA[i];
                        backupBuffer = bufferB[i];

                        bufferSwap.set(i,true);
                    }
                    else
                    {
                        overflowBuffer = bufferB[i];
                        backupBuffer = bufferA[i];

                        bufferSwap.set(i,false);
                    }

                    backupBuffer->clear();

                    samplesInOverflowBuffer.set(i,samplesInBackupBuffer[i]); // size of buffer after last round
                    samplesInBackupBuffer.set(i,0);

                    int orphanedSamples = 0;

                    // 1. copy overflow buffer

                    int samplesToCopyFromOverflowBuffer =
                        ((samplesInOverflowBuffer[i] <= numSamplesExpected[i]) ?
                         samplesInOverflowBuffer[i] :
                         numSamplesExpected[i]);

                    //std::cout << "Copying " << samplesToCopyFromOverflowBuffer << " samples from overflow buffer of " << samplesInOverflowBuffer[i] << " samples." << std::endl;

                    if (samplesToCopyFromOverflowBuffer > 0) // need to re-add samples from backup buffer
                    {

                        tempBuffer->addFrom(0,    // destination channel
                                            0,                // destination start sample
                                            *overflowBuffer,  // source
                                            0,                // source channel
                                            0,                // source start sample
                                            samplesToCopyFromOverflowBuffer,    // number of samples
                                            1.0f              // gain to apply
                                           );

                        int leftoverSamples = samplesInOverflowBuffer[i] - samplesToCopyFromOverflowBuffer;

                        // std::cout << "Samples remaining in overflow buffer: " << leftoverSamples << std::endl;

                        if (leftoverSamples > 0) // move remaining samples to the backup buffer
                        {

                            backupBuffer->addFrom(0, // destination channel
                                                  0,                     // destination start sample
                                                  *overflowBuffer,       // source
                                                  0,                     // source channel
                                                  samplesToCopyFromOverflowBuffer,         // source start sample
                                                  leftoverSamples,       // number of samples
                                                  1.0f                   // gain to apply
                                                 );
                        }

                        samplesInBackupBuffer.set(i,leftoverSamples);
                    }

                    gain = volume/(float(0x7fff) * dataChannelArray[i]->getBitVolts());
                    // Data are floats in units of microvolts, so dividing by bitVolts and 0x7fff (max value for 16b signed)
                    // rescales to between -1 and +1. Audio output starts So, maximum gain applied to maximum data would be 10.

                    int remainingSamples = numSamplesExpected[i] - samplesToCopyFromOverflowBuffer;

                    int samplesAvailable = getNumSourceSamples(dataChannelArray[i]->getSourceNodeID(), dataChannelArray[i]->getSubProcessorIdx());

                    int samplesToCopyFromIncomingBuffer = ((remainingSamples <= samplesAvailable) ?
                                                           remainingSamples :
                                                           samplesAvailable);

                    //std::cout << "Copying " << samplesToCopyFromIncomingBuffer << " samples from incoming buffer of " << samplesAvailable << " samples." << std::endl;


                    if (samplesToCopyFromIncomingBuffer > 0)
                    {

                        //tempBuffer->addFrom(i,       // destination channel
                        tempBuffer->addFrom(0,       // destination channel
                                            samplesToCopyFromOverflowBuffer,           // destination start sample
                                            buffer,      // source
                                            i+2,           // source channel (add 2 to account for output channels)
                                            0,           // source start sample
                                            samplesToCopyFromIncomingBuffer, //  number of samples
                                            gain       // gain to apply
                                           );

                        //if (destBufferPos == 0)
                        //  std::cout << "Temp buffer 0 value: " << *tempBuffer->getReadPointer(i,0) << std::endl;

                    }

                    orphanedSamples = samplesAvailable - samplesToCopyFromIncomingBuffer;

                    // std::cout << "Samples remaining in incoming buffer: " << orphanedSamples << std::endl;

                    if (orphanedSamples > 0 && (samplesInBackupBuffer[i] + orphanedSamples < backupBuffer->getNumSamples()))
                    {

                        backupBuffer->addFrom(0,                            // destination channel
                                              samplesInBackupBuffer[i],     // destination start sample
                                              buffer,                       // source
                                              i+2,                          // source channel (add 2 to account for output channels)
                                              remainingSamples,             // source start sample
                                              orphanedSamples,              //  number of samples
                                              gain                          // gain to apply
                                             );

                        samplesInBackupBuffer.set(i, samplesInBackupBuffer[i] + orphanedSamples);

                    }

                    // now that our tempBuffer is ready, we can filter it and copy it into the
                    // original buffer

                    //std::cout << "Ratio = " << ratio[i] << ", gain = " << gain << std::endl;
                    //std::cout << "Values needed = " << valuesNeeded << ", channel = " << i << std::endl;

                    if (ratio[i] > 1.00001)
                    {
                        // pre-apply filter before downsampling
                        float* ptr = tempBuffer->getWritePointer(i);
                        filters[i]->process(numSamplesExpected[i], &ptr);
                    }

                    // initialize variables
                    int sourceBufferPos = 0;
                    int sourceBufferSize = numSamplesExpected[i];
                    float subSampleOffset = 0.0;
                    int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

                    int destBufferPos;

                    // code modified from "juce_ResamplingAudioSource.cpp":

                    for (destBufferPos = 0; destBufferPos < valuesNeeded; destBufferPos++)
                    {
                        float gain = 1.0;
                        float alpha = (float) subSampleOffset;
                        float invAlpha = 1.0f - alpha;

                        // std::cout << "Copying sample " << sourceBufferPos << std::endl;

                        buffer.addFrom(0,    // destChannel
                                       destBufferPos,  // destSampleOffset
                                       *tempBuffer,     // source
                                       //i,    // sourceChannel
                                       0,       // sourceChannel
                                       sourceBufferPos,// sourceSampleOffset
                                       1,        // number of samples
                                       invAlpha*gain);      // gain to apply to source

                        buffer.addFrom(0,    // destChannel
                                       destBufferPos,   // destSampleOffset
                                       *tempBuffer,     // source
                                       //i,      // sourceChannel
                                       0,      // sourceChannel
                                       nextPos,      // sourceSampleOffset
                                       1,        // number of samples
                                       alpha*gain);       // gain to apply to source

                        // if (destBufferPos == 0)
                        //std::cout << "Output buffer 0 value: " << *buffer.getReadPointer(i+2,destBufferPos) << std::endl;

                        subSampleOffset += ratio[i];

                        while (subSampleOffset >= 1.0)
                        {
                            if (++sourceBufferPos >= sourceBufferSize)
                                sourceBufferPos = 0;

                            nextPos = (sourceBufferPos + 1) % sourceBufferSize;
                            subSampleOffset -= 1.0;
                        }
                    }

                    if (ratio[i] < 0.99999)
                    {
                        // apply the filter after upsampling
                        float* ptr = buffer.getWritePointer(0);
                        filters[i]->process(destBufferPos, &ptr);
                    }

                    // now copy the channel into the output zone

                    // buffer.addFrom(0,    // destChannel
                    //                0,  // destSampleOffset
                    //                buffer,     // source
                    //                 i+2,    // sourceChannel
                    //                 0,// sourceSampleOffset
                    //                 valuesNeeded,        // number of samples
                    //                 1.0);      // gain to apply to source

                } // if channelPointers[i]->isMonitored
            } // end cycling through channels

            // Simple implementation of a "noise gate" on audio output
            expander.process(buffer.getWritePointer(0), // expand the left channel
                             buffer.getNumSamples());

            // copy the signal into the right channel (no stereo audio yet!)
            buffer.addFrom(1,    // destChannel
                           0,  // destSampleOffset
                           buffer,     // source
                           0,    // sourceChannel
                           0,// sourceSampleOffset
                           valuesNeeded,        // number of samples
                           1.0);      // gain to apply to source
        }
    }
}


void AudioNode::updateRecordChannelIndexes()
{
	//Keep the nodeIDs of the original processor from each channel comes from
	updateChannelIndexes(false);
}

// ==========================================================

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

    std::cout << "Threshold set to " << threshold << std::endl;
    std::cout << "transfer_B set to " << transfer_B << std::endl;
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
