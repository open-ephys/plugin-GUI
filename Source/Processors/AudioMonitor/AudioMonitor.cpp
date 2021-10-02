/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "AudioMonitor.h"
#include "AudioMonitorEditor.h"
#include <stdio.h>


AudioMonitor::AudioMonitor()
    : GenericProcessor ("Audio Monitor"), isMuted(false), audioOutput(BOTH)
{
    setProcessorType (PROCESSOR_TYPE_AUDIO_MONITOR);

    tempBuffer = std::make_unique<AudioSampleBuffer>(16, 1024);
}


AudioMonitor::~AudioMonitor()
{

}


AudioProcessorEditor* AudioMonitor::createEditor()
{
    editor = std::make_unique<AudioMonitorEditor>(this, true);

    return editor.get();
}


void AudioMonitor::updateSettings()
{

	int ch = 0;
    dataChannelStates.clear();

	while (ch < continuousChannels.size())
	{
        dataChannelStates.push_back(false);

        ch++;
    }
    
    // Set Number of outputs to be 2 more than inputs
    audioChannels.add(continuousChannels.getFirst());
    audioChannels.add(continuousChannels.getFirst());

    updatePlaybackBuffer();
}


void AudioMonitor::resetConnections()
{
    GenericProcessor::resetConnections();

    updatePlaybackBuffer();
}


void AudioMonitor::updatePlaybackBuffer()
{
	setPlayConfigDetails(getNumInputs(), getNumOutputs() + 2, 44100.0, 128);
}


void AudioMonitor::setChannelStatus(int chan, bool status)
{

    setCurrentChannel(chan);

     if (status)
    {
        setParameter(100, 0.0f);
    }
    else
    {
        setParameter(-100, 0.0f);
    }

}


void AudioMonitor::setParameter (int parameterIndex, float newValue)
{
    // change output to left channel, right channel, or both
    if (parameterIndex == 1)
    {
        audioOutput = LEFT;
    }
    else if (parameterIndex == 2)
    {
        audioOutput = BOTH;
    }
    else if (parameterIndex == 3)
    {
        audioOutput = RIGHT;
    }

    // Mute on/off
    else if (parameterIndex == 4)
    {
        isMuted = (newValue == 0.0f) ? true : false;
    }

    //channel enable/disable
    else if (parameterIndex == 100)
    {

		dataChannelStates[currentChannel] = true;

    }
    else if (parameterIndex == -100)
    {

		dataChannelStates[currentChannel] = false;
    }
}


void AudioMonitor::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{
    //LOGDD("Processor sample rate: ", getSampleRate());
    LOGDD("Audio card sample rate: ", sampleRate_);
    LOGDD("Samples per block: ", estimatedSamplesPerBlock);
	if (sampleRate_ != destBufferSampleRate || estimatedSamplesPerBlock != estimatedSamples)
	{
		destBufferSampleRate = sampleRate_;
		estimatedSamples = estimatedSamplesPerBlock;
		recreateBuffers();
	}

}


void AudioMonitor::recreateBuffers()
{
	numSamplesExpected.clear();
    samplesInBackupBuffer.clear();
    samplesInOverflowBuffer.clear();
    ratio.clear();
    filters.clear();
    bufferA.clear();
    bufferB.clear();
    bufferSwap.clear();

    for (int i = 0; i < continuousChannels.size(); i++)
    {
        if(dataChannelStates.at(i))
        {
            // processor sample rate divided by sound card sample rate
            numSamplesExpected.emplace(i, (int)(continuousChannels[i]->getSampleRate()/destBufferSampleRate*float(estimatedSamples)) + 1);
            samplesInBackupBuffer.emplace(i, 0);
            samplesInOverflowBuffer.emplace(i, 0);
            sourceBufferSampleRate.emplace(i, continuousChannels[i]->getSampleRate());

            filters.emplace(i, std::make_unique<Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 1>> (1024));

            ratio.emplace(i, float(numSamplesExpected[i])/float(estimatedSamples));
            updateFilter(i);

            bufferA.emplace(i, std::make_unique<AudioBuffer<float>>(1,10000));
            bufferB.emplace(i, std::make_unique<AudioBuffer<float>>(1,10000));
            bufferSwap.emplace(i, false);
        }

    }

    tempBuffer->setSize(1, 4096);
}


void AudioMonitor::updateFilter(int i)
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


bool AudioMonitor::startAcquisition()
{
	recreateBuffers();
	return true;
}


void AudioMonitor::process (AudioSampleBuffer& buffer)
{

    float gain;
    int valuesNeeded = buffer.getNumSamples(); // samples needed to fill out the buffer

    LOGDD("Buffer size: ", buffer.getNumChannels());

    // clear the left and right channels
    buffer.clear(0,0,buffer.getNumSamples());
    buffer.clear(1,0,buffer.getNumSamples());

    if (!isMuted)
    {

        AudioSampleBuffer* overflowBuffer;
        AudioSampleBuffer* backupBuffer;

        int nInputs = continuousChannels.size();
        if (nInputs > 0) // we have some channels
        {

//            tempBuffer->clear();

            for (int i = 0; i < nInputs; i++) // cycle through them all
            {

                if (dataChannelStates.at(i))
                {
                    tempBuffer->clear();
                    LOGDD("Processing channel ", i);

                    if (!bufferSwap[i])
                    {
                        overflowBuffer = bufferA[i].get();
                        backupBuffer = bufferB[i].get();

                        bufferSwap[i] = true;
                    }
                    else
                    {
                        overflowBuffer = bufferB[i].get();
                        backupBuffer = bufferA[i].get();

                        bufferSwap[i] = false;
                    }

                    backupBuffer->clear();

                    samplesInOverflowBuffer[i] = samplesInBackupBuffer[i]; // size of buffer after last round
                    samplesInBackupBuffer[i] = 0;

                    int orphanedSamples = 0;

                    // 1. copy overflow buffer

                    int samplesToCopyFromOverflowBuffer =
                        ((samplesInOverflowBuffer[i] <= numSamplesExpected[i]) ?
                         samplesInOverflowBuffer[i] :
                         numSamplesExpected[i]);

                    LOGDD("Copying ", samplesToCopyFromOverflowBuffer, " samples from overflow buffer of ", samplesInOverflowBuffer[i], " samples.");

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

                        LOGDD("Samples remaining in overflow buffer: ", leftoverSamples);

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

                        samplesInBackupBuffer[i] = leftoverSamples;
                    }

                    // TODO: GAIN
                    //gain = volume/(float(0x7fff) * dataChannelArray[i]->getBitVolts());
                    gain = 1.0;
                    // Data are floats in units of microvolts, so dividing by bitVolts and 0x7fff (max value for 16b signed)
                    // rescales to between -1 and +1. Audio output starts So, maximum gain applied to maximum data would be 10.

                    int remainingSamples = numSamplesExpected[i] - samplesToCopyFromOverflowBuffer;

                    int samplesAvailable = getNumSourceSamples(continuousChannels[i]->getStreamId());

                    int samplesToCopyFromIncomingBuffer = ((remainingSamples <= samplesAvailable) ?
                                                           remainingSamples :
                                                           samplesAvailable);

                    LOGDD("Copying ", samplesToCopyFromIncomingBuffer, " samples from incoming buffer of ", samplesAvailable, " samples.");


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
                      //  LOGDD("Temp buffer 0 value: ", *tempBuffer->getReadPointer(i,0));

                    }

                    orphanedSamples = samplesAvailable - samplesToCopyFromIncomingBuffer;

                    LOGDD("Samples remaining in incoming buffer: ", orphanedSamples);

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

                        samplesInBackupBuffer[i] = samplesInBackupBuffer[i] + orphanedSamples;

                    }

                    // now that our tempBuffer is ready, we can filter it and copy it into the
                    // original buffer

                    LOGDD("Ratio = ", ratio[i], ", gain = ", gain);
                    LOGDD("Values needed = ", valuesNeeded, ", channel = ", i);

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

                        LOGDD("Copying sample ", sourceBufferPos);

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
                        LOGDD("Output buffer 0 value: ", *buffer.getReadPointer(i+2,destBufferPos));

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

                } // if channelPointers[i]->isMonitored
            } // end cycling through channels

            if(audioOutput == BOTH || audioOutput == RIGHT)
            {
                // copy the signal into the right channel
                buffer.addFrom(1,    // destChannel
                            0,  // destSampleOffset
                            buffer,     // source
                            0,    // sourceChannel
                            0,// sourceSampleOffset
                            valuesNeeded,        // number of samples
                            1.0);      // gain to apply to source
                
                // if Right only, clear left channel
                if(audioOutput == RIGHT)
                    buffer.clear(0,0,buffer.getNumSamples());
            }

        }
    }

}
