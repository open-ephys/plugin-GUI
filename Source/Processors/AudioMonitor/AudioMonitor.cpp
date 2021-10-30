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
    : GenericProcessor ("Audio Monitor") //, isMuted(false), audioOutput(BOTH)
{

    tempBuffer = std::make_unique<AudioSampleBuffer>(16, 1024);
    
    addBooleanParameter(Parameter::GLOBAL_SCOPE,
                        String("mute_audio"),
                        "Mute audio for this Audio Monitor",
                        false);
    
    addCategoricalParameter(Parameter::GLOBAL_SCOPE,
                            String("audio_output"),
                            "Select L/R or both",
                            { "LEFT", "BOTH", "RIGHT" },
                            1);
    
    addSelectedChannelsParameter(Parameter::STREAM_SCOPE,
                                 String("selected_channels"),
                                 "Channels to monitor",
                                 4);

    for (int i = 0; i < MAX_CHANNELS; i++)
    {
        
        filters.add (new Dsp::SmoothedFilterDesign
                         <Dsp::Butterworth::Design::BandPass    // design type
                         <2>,                                   // order
                         1,                                     // number of channels (must be const)
                         Dsp::DirectFormII> (1));               // realization
    }
}


AudioMonitor::~AudioMonitor()
{

}


AudioProcessorEditor* AudioMonitor::createEditor()
{
    editor = std::make_unique<AudioMonitorEditor>(this);

    return editor.get();
}


void AudioMonitor::updateSettings()
{
    updatePlaybackBuffer();
}


void AudioMonitor::resetConnections()
{
    GenericProcessor::resetConnections();

    updatePlaybackBuffer();
}


void AudioMonitor::updatePlaybackBuffer()
{
	setPlayConfigDetails(getNumInputs(), getNumOutputs() + 2, 48000.0, 128);
}

/*Array<int> AudioMonitor::getMonitoredChannels()
{
    return activeChannels;
    
}

void AudioMonitor::setMonitoredChannels(Array<int> channelsToMonitor)
{
    setParameter(-100, 0.0f); // clear channels
    
    for (auto ch : channelsToMonitor)
        setParameter(100, ch);
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

        LOGD("Adding channel ", newValue);
        activeChannels.add(int(newValue));
        updateFilter(activeChannels.indexOf(int(newValue)));

    } else if (parameterIndex == -100)
    {
        LOGD("Clearing active channels.");
        activeChannels.clear();
    }
}*/


void AudioMonitor::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{
    //("Processor sample rate: ", getSampleRate());
    LOGD("Audio card sample rate: ", sampleRate_);
    LOGD("Samples per block: ", estimatedSamplesPerBlock);
    
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
    sourceBufferSampleRate.clear();
    
    ratio.clear();
    
    for (int i = 0; i < getNumInputs(); i++)
    {
        numSamplesExpected.emplace(i, (int)(continuousChannels[i]->getSampleRate()/destBufferSampleRate*float(estimatedSamples)) + 1);
        sourceBufferSampleRate.emplace(i, continuousChannels[i]->getSampleRate());
        ratio.emplace(i, float(numSamplesExpected[i])/float(estimatedSamples));
        std::cout << "Ratio " << i << ": " << ratio[i] << std::endl;
    }

    samplesInBackupBuffer.clear();
    samplesInOverflowBuffer.clear();
    
    bufferA.clear();
    bufferB.clear();
    bufferSwap.clear();
    
    for (int i = 0; i < MAX_CHANNELS; i++)
    {

        //
        samplesInBackupBuffer.emplace(i, 0);
        samplesInOverflowBuffer.emplace(i, 0);

        bufferA.emplace(i, std::make_unique<AudioBuffer<float>>(1,10000));
        bufferB.emplace(i, std::make_unique<AudioBuffer<float>>(1,10000));
        bufferSwap.emplace(i, false);

    }

    tempBuffer->setSize(1, 4096);
}


void AudioMonitor::updateFilter(int i)
{
    // NEEDS TO BE UPDATED FOR STREAM-BASED PROCESSING
    
    Dsp::Params params;
    params[0] = continuousChannels[0]->getSampleRate(); // sample rate
    params[1] = 2;                          // order
    params[2] = (7000 + 300) / 2;     // center frequency
    params[3] = 7000 - 300;           // bandwidth

    filters[i]->setParams (params);

    /*double cutoffFreq = (ratio[i] > 1.0) ? 2 * destBufferSampleRate  // downsample
                        : destBufferSampleRate / 2; // upsample

    double sampleFreq = (ratio[i] > 1.0) ? sourceBufferSampleRate[i] // downsample
                        : destBufferSampleRate;  // upsample

    Dsp::Params params;
    params[0] = sampleFreq; // sample rate
    params[1] = cutoffFreq; // cutoff frequency
    params[2] = 1.25; //Q //

    filters[i]->setParams(params);*/

}


bool AudioMonitor::startAcquisition()
{
	return true;
}


void AudioMonitor::process (AudioBuffer<float>& buffer)
{

    float gain;
    
    int valuesNeeded = buffer.getNumSamples(); // samples needed to fill the complete buffer
    
    int totalBufferChannels = buffer.getNumChannels();

    // clear the left and right channels (last two channels)
    buffer.clear(totalBufferChannels - 2, 0, buffer.getNumSamples());
    buffer.clear(totalBufferChannels - 1, 0, buffer.getNumSamples());

    if (!getParameter("mute_audio")->getValue())
    {

        for (auto stream : getDataStreams())
        {

            AudioSampleBuffer* overflowBuffer;
            AudioSampleBuffer* backupBuffer;

            Array<var> activeChannels = getParameter(stream->getStreamId(), "selected_channels")->getValue();

            for (int i = 0; i < activeChannels.size(); i++)
            {

                int channelIndex = stream->getContinuousChannels()[activeChannels[i]]->getGlobalIndex();

                tempBuffer->clear();

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

                // LOGD("Overflow samples: ", samplesInOverflowBuffer[i]);

                int orphanedSamples = 0;

                // 1. copy overflow buffer

                int samplesToCopyFromOverflowBuffer =
                    ((samplesInOverflowBuffer[i] <= numSamplesExpected[channelIndex]) ?
                        samplesInOverflowBuffer[i] :
                        numSamplesExpected[channelIndex]);

                // LOGD("Number of samples to copy: ", samplesToCopyFromOverflowBuffer);

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

                //LOGD("Leftover samples: ", samplesInBackupBuffer[i]);

                // TODO: GAIN
                //gain = volume/(float(0x7fff) * dataChannelArray[i]->getBitVolts());
                gain = 1.0;
                // Data are floats in units of microvolts, so dividing by bitVolts and 0x7fff (max value for 16b signed)
                // rescales to between -1 and +1. Audio output starts So, maximum gain applied to maximum data would be 10.

                int remainingSamples = numSamplesExpected[channelIndex] - samplesToCopyFromOverflowBuffer;

                // NEED TO UPDATE THIS!!!
                int samplesAvailable = getNumSourceSamples(continuousChannels[0]->getStreamId());

                int samplesToCopyFromIncomingBuffer = ((remainingSamples <= samplesAvailable) ?
                    remainingSamples :
                    samplesAvailable);

                //LOGDD("Copying ", samplesToCopyFromIncomingBuffer, " samples from incoming buffer of ", samplesAvailable, " samples.");

                //LOGD("Incoming samples: ", samplesAvailable);

                if (samplesToCopyFromIncomingBuffer > 0)
                {

                    tempBuffer->addFrom(0,       // destination channel
                        samplesToCopyFromOverflowBuffer,           // destination start sample
                        buffer,      // source
                        i,           // source channel
                        0,           // source start sample
                        samplesToCopyFromIncomingBuffer, //  number of samples
                        gain       // gain to apply
                    );

                }

                orphanedSamples = samplesAvailable - samplesToCopyFromIncomingBuffer;

                //LOGD("Orphaned samples: ", orphanedSamples);

                //LOGDD("Samples remaining in incoming buffer: ", orphanedSamples);

                if (orphanedSamples > 0 && (samplesInBackupBuffer[i] + orphanedSamples < backupBuffer->getNumSamples()))
                {

                    backupBuffer->addFrom(0,                            // destination channel
                        samplesInBackupBuffer[i],     // destination start sample
                        buffer,                       // source
                        i,                            // source channel
                        remainingSamples,             // source start sample
                        orphanedSamples,              //  number of samples
                        gain                          // gain to apply
                    );

                    samplesInBackupBuffer[i] = samplesInBackupBuffer[i] + orphanedSamples;

                }

                // now that our tempBuffer is ready, we can filter it and copy it into the
                // original buffer

               // LOGD("Ratio = ", ratio[channelIndex], ", gain = ", gain);
               // LOGD("Values needed = ", valuesNeeded, ", channel = ", channelIndex);

                float* ptr = tempBuffer->getWritePointer(0);
                filters[i]->process(numSamplesExpected[channelIndex], &ptr);

                if (ratio[i] > 1.00001)
                {
                    // pre-apply filter before downsampling
                    //float* ptr = tempBuffer->getWritePointer(i);
                    //filters[i]->process(numSamplesExpected[i], &ptr);
                }

                // initialize variables
                int sourceBufferPos = 0;
                int sourceBufferSize = numSamplesExpected[i];
                float subSampleOffset = 0.0;
                int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

                int destBufferPos;

                // code modified from "juce_ResamplingAudioSource.cpp":

                int targetChannel;

                if (int(getParameter("audio_output")->getValue()) == 0 || int(getParameter("audio_output")->getValue()) == 2)
                    targetChannel = totalBufferChannels - 2;
                else
                    targetChannel = totalBufferChannels - 1;

                for (destBufferPos = 0; destBufferPos < valuesNeeded; destBufferPos++)
                {
                    float gain = 1.0;
                    float alpha = (float)subSampleOffset;
                    float invAlpha = 1.0f - alpha;

                    //LOGDD("Copying sample ", sourceBufferPos);

                    buffer.addFrom(targetChannel,    // destChannel
                        destBufferPos,  // destSampleOffset
                        *tempBuffer,     // source
                        0,       // sourceChannel
                        sourceBufferPos,// sourceSampleOffset
                        1,        // number of samples
                        invAlpha * gain);      // gain to apply to source

                    buffer.addFrom(targetChannel,    // destChannel
                        destBufferPos,   // destSampleOffset
                        *tempBuffer,     // source
                        0,      // sourceChannel
                        nextPos,      // sourceSampleOffset
                        1,        // number of samples
                        alpha * gain);       // gain to apply to source

         // if (destBufferPos == 0)
         //LOGDD("Output buffer 0 value: ", *buffer.getReadPointer(i,destBufferPos));

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
                    //float* ptr = buffer.getWritePointer(totalBufferChannels - 2);
                    //filters[i]->process(destBufferPos, &ptr);
                }

            } // end cycling through channels

            if (int(getParameter("audio_output")->getValue()) == 2)
            {
                // copy the signal into the right channel
                buffer.addFrom(totalBufferChannels - 1,    // destChannel
                    0,  // destSampleOffset
                    buffer,     // source
                    totalBufferChannels - 2,    // sourceChannel
                    0,// sourceSampleOffset
                    valuesNeeded,        // number of samples
                    1.0);      // gain to apply to source

            }
        }

    }

}
