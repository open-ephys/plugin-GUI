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

#include "AudioMonitor.h"
#include "AudioMonitorEditor.h"
#include <stdio.h>

AudioMonitorSettings::AudioMonitorSettings()
    : numChannels (0),
      sampleRate (0),
      destBufferSampleRate (44100.0f),
      estimatedSamples (1024)
{
}

void AudioMonitorSettings::createFilters (int nChans, float sampleRate_)
{
    numChannels = nChans;
    sampleRate = sampleRate_;

    bandpassfilters.clear();
    antialiasingfilters.clear();

    Dsp::Params bandpassParams;
    bandpassParams[0] = sampleRate; // sample rate
    bandpassParams[1] = 2; // order
    bandpassParams[2] = (7000 + 100) / 2; // center frequency
    bandpassParams[3] = 7000 - 100; // bandwidth

    for (int n = 0; n < numChannels; ++n)
    {
        bandpassfilters.add (new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass // design type
                                                           <2>, // order
                                                           1, // number of channels (must be const)
                                                           Dsp::DirectFormII> (1)); // realization

        bandpassfilters.getLast()->setParams (bandpassParams);

        antialiasingfilters.add (new Dsp::SmoothedFilterDesign<Dsp::RBJ::Design::LowPass, 1> (1024));
    }

    updateAntiAliasingFilterParameters();
}

void AudioMonitorSettings::updateAntiAliasingFilterParameters()
{
    Dsp::Params antialiasingParams;
    antialiasingParams[0] = destBufferSampleRate; // sample rate
    antialiasingParams[1] = destBufferSampleRate / 2; // cutoff frequency
    antialiasingParams[2] = 1.25; //Q //

    for (int i = 0; i < numChannels; ++i)
    {
        antialiasingfilters[i]->setParams (antialiasingParams);
    }
}

void AudioMonitorSettings::setOutputSampleRate (double outputSampleRate, double estimatedSamplesPerBlock)
{
    destBufferSampleRate = outputSampleRate;
    estimatedSamples = estimatedSamplesPerBlock;

    numSamplesExpected = (sampleRate / destBufferSampleRate) * estimatedSamples;
    ratio = sampleRate / destBufferSampleRate;

    samplesInBackupBuffer.clear();
    samplesInOverflowBuffer.clear();

    bufferA.clear();
    bufferB.clear();
    bufferSwap.clear();

    for (int i = 0; i < numChannels; i++)
    {
        samplesInBackupBuffer.emplace (i, 0.0f);
        samplesInOverflowBuffer.emplace (i, 0.0f);

        bufferA.emplace (i, std::make_unique<AudioBuffer<float>> (1, (int) destBufferSampleRate));
        bufferB.emplace (i, std::make_unique<AudioBuffer<float>> (1, (int) destBufferSampleRate));
        bufferSwap.emplace (i, false);
    }

    updateAntiAliasingFilterParameters();
}

AudioMonitor::AudioMonitor()
    : GenericProcessor ("Audio Monitor")
{
    tempBuffer = std::make_unique<AudioSampleBuffer> (1, 4096);
}

void AudioMonitor::registerParameters()
{
    addBooleanParameter (
        Parameter::PROCESSOR_SCOPE,
        "mute_audio",
        "Mute audio",
        "Mute audio for this Audio Monitor",
        false);

    addCategoricalParameter (
        Parameter::PROCESSOR_SCOPE,
        "audio_output",
        "Audio output",
        "Select L/R or both",
        { "LEFT", "BOTH", "RIGHT" },
        1);

    addSelectedChannelsParameter (
        Parameter::STREAM_SCOPE,
        "channels",
        "Channels",
        "Channels to monitor");

    addCategoricalParameter (
        Parameter::STREAM_SCOPE,
        "spike_channel",
        "Spike Channel",
        "Select the spike channel. This will automatically select relevant channels to monitor.",
        { "No spike channel" },
        0);
}

AudioProcessorEditor* AudioMonitor::createEditor()
{
    editor = std::make_unique<AudioMonitorEditor> (this);

    return editor.get();
}

void AudioMonitor::updateSettings()
{
    updatePlaybackBuffer();

    settings.update (getDataStreams());

    for (auto stream : dataStreams)
    {
        // create filters for each channel
        settings[stream->getStreamId()]->createFilters (stream->getChannelCount(),
                                                        stream->getSampleRate());

        // update the spike channel parameter with the available spike channels
        Array<String> spikeChannelNames;
        spikeChannelNames.add ("No spike channel");
        for (auto spikeChan : stream->getSpikeChannels())
            spikeChannelNames.add (spikeChan->getName());

        CategoricalParameter* spikeChanParam = (CategoricalParameter*) stream->getParameter ("spike_channel");
        spikeChanParam->setCategories (spikeChannelNames);

        parameterValueChanged (stream->getParameter ("spike_channel"));
    }
}

void AudioMonitor::resetConnections()
{
    GenericProcessor::resetConnections();

    updatePlaybackBuffer();
}

void AudioMonitor::updatePlaybackBuffer()
{
    setPlayConfigDetails (getNumInputs(), getNumOutputs() + 2, 44100.0, 128);
}

void AudioMonitor::prepareToPlay (double sampleRate_, int estimatedSamplesPerBlock)
{
    for (auto stream : dataStreams)
    {
        settings[stream->getStreamId()]->setOutputSampleRate (sampleRate_, estimatedSamplesPerBlock);
    }
}

void AudioMonitor::parameterValueChanged (Parameter* param)
{
    if (param->getName().equalsIgnoreCase ("spike_channel"))
    {
        DataStream* stream = getDataStream (param->getStreamId());

        int selectedIndex = (int) param->getValue();

        Parameter* chansParam = stream->getParameter ("channels");

        if (selectedIndex > 0)
        {
            Array<int> selectedChannels = stream->getSpikeChannels()[selectedIndex - 1]->localChannelIndexes;

            Array<var> inds;

            for (int ch : selectedChannels)
            {
                inds.add (ch);
            }

            chansParam->currentValue = inds;
        }
        else
        {
            chansParam->currentValue = Array<var>();
        }

        chansParam->valueChanged();
    }
}

void AudioMonitor::handleBroadcastMessage (const String& msg, const int64 messageTimeMilliseconds)
{
    LOGC ("Audio Monitor received message: ", msg);

    StringArray parts = StringArray::fromTokens (msg, " ", "");

    if (parts[0].equalsIgnoreCase ("AUDIO"))
    {
        if (parts.size() > 1)
        {
            String command = parts[1];

            if (command.equalsIgnoreCase ("SELECT"))
            {
                if (parts.size() == 3)
                {
                    String command = parts[2];

                    if (command.equalsIgnoreCase ("NONE"))
                    {
                        Array<var> ch;

                        for (auto stream : getDataStreams())
                            stream->getParameter ("channels")->setNextValue (ch);
                    }
                }
                else if (parts.size() >= 4)
                {
                    uint16 streamId = parts[2].getIntValue();

                    DataStream* stream = getDataStream (streamId);

                    if (stream != nullptr)
                    {
                        Array<var> ch;
                        int channelCount = 0;

                        while (channelCount < (parts.size() - 3))
                        {
                            int localChannel = parts[channelCount + 3].getIntValue() - 1;

                            if (localChannel >= 0 && localChannel < stream->getContinuousChannels().size())
                            {
                                ch.add (localChannel);
                            }

                            channelCount++;
                        }

                        if (ch.size() > 0)
                        {
                            stream->getParameter ("channels")->setNextValue (ch);
                        }
                    }
                }
            }
        }
    }
}

void AudioMonitor::setSelectedStream (uint16 streamId)
{
    selectedStream = streamId;
}

void AudioMonitor::process (AudioBuffer<float>& buffer)
{
    int valuesNeeded = buffer.getNumSamples(); // samples needed to fill the complete buffer

    int totalBufferChannels = buffer.getNumChannels();

    // clear the left and right channels (last two channels)
    buffer.clear (totalBufferChannels - 2, 0, buffer.getNumSamples());
    buffer.clear (totalBufferChannels - 1, 0, buffer.getNumSamples());

    if (! getParameter ("mute_audio")->getValue())
    {
        for (auto stream : dataStreams)
        {
            if (stream->getStreamId() == selectedStream)
            {
                auto streamSettings = settings[selectedStream];
                AudioSampleBuffer* overflowBuffer;
                AudioSampleBuffer* backupBuffer;

                Array<var>* activeChannels = stream->getParameter ("channels")->getValue().getArray();

                for (int i = 0; i < activeChannels->size(); i++)
                {
                    int localIndex = (int) activeChannels->getReference (i);

                    int globalIndex = getDataStream (selectedStream)->getContinuousChannels()[localIndex]->getGlobalIndex();

                    tempBuffer->clear();

                    if (! streamSettings->bufferSwap[localIndex])
                    {
                        overflowBuffer = streamSettings->bufferA[localIndex].get();
                        backupBuffer = streamSettings->bufferB[localIndex].get();

                        streamSettings->bufferSwap[localIndex] = true;
                    }
                    else
                    {
                        overflowBuffer = streamSettings->bufferB[localIndex].get();
                        backupBuffer = streamSettings->bufferA[localIndex].get();

                        streamSettings->bufferSwap[localIndex] = false;
                    }

                    backupBuffer->clear();

                    streamSettings->samplesInOverflowBuffer[localIndex] = streamSettings->samplesInBackupBuffer[localIndex]; // size of buffer after last round
                    streamSettings->samplesInBackupBuffer[localIndex] = 0;

                    double orphanedSamples = 0;

                    // 1. copy overflow buffer

                    double samplesToCopyFromOverflowBuffer =
                        ((streamSettings->samplesInOverflowBuffer[localIndex] <= streamSettings->numSamplesExpected)
                             ? streamSettings->samplesInOverflowBuffer[localIndex]
                             : streamSettings->numSamplesExpected);

                    // LOGD("Number of samples to copy: ", samplesToCopyFromOverflowBuffer);

                    //std::cout << "Copying from overflow buffer: " << samplesToCopyFromOverflowBuffer << std::endl;

                    if (samplesToCopyFromOverflowBuffer > 0) // need to re-add samples from backup buffer
                    {
                        tempBuffer->addFrom (0, // destination channel
                                             0, // destination start sample
                                             *overflowBuffer, // source
                                             0, // source channel
                                             0, // source start sample
                                             (int) samplesToCopyFromOverflowBuffer, // number of samples
                                             1.0f // gain to apply
                        );

                        double leftoverSamples = streamSettings->samplesInOverflowBuffer[localIndex] - samplesToCopyFromOverflowBuffer;

                        //std::cout << "Copying to backup buffer: " << leftoverSamples << std::endl;

                        if (leftoverSamples > 0) // move remaining samples to the backup buffer
                        {
                            backupBuffer->addFrom (0, // destination channel
                                                   0, // destination start sample
                                                   *overflowBuffer, // source
                                                   0, // source channel
                                                   (int) samplesToCopyFromOverflowBuffer, // source start sample
                                                   (int) leftoverSamples, // number of samples
                                                   1.0f // gain to apply
                            );
                        }

                        streamSettings->samplesInBackupBuffer[localIndex] = leftoverSamples;
                    }

                    double remainingSamples = double (streamSettings->numSamplesExpected) - samplesToCopyFromOverflowBuffer;

                    double samplesAvailable = double (getNumSamplesInBlock (selectedStream));

                    //std::cout << "Remaining samples: " << remainingSamples << std::endl;
                    //std::cout << "Samples available: " << samplesAvailable << std::endl;

                    double samplesToCopyFromIncomingBuffer =
                        ((remainingSamples <= samplesAvailable) ? remainingSamples : samplesAvailable);

                    //std::cout << "Copying from incoming buffer: " << samplesToCopyFromIncomingBuffer << std::endl;

                    if (samplesToCopyFromIncomingBuffer > 0)
                    {
                        tempBuffer->addFrom (0, // destination channel
                                             (int) samplesToCopyFromOverflowBuffer, // destination start sample
                                             buffer, // source
                                             globalIndex, // source channel
                                             0, // source start sample
                                             (int) samplesToCopyFromIncomingBuffer, //  number of samples
                                             1.0f // gain to apply
                        );
                    }

                    orphanedSamples = samplesAvailable - samplesToCopyFromIncomingBuffer;

                    //std::cout << "Orphaned samples: " << orphanedSamples << std::endl;

                    if (orphanedSamples > 0
                        && (streamSettings->samplesInBackupBuffer[localIndex] + orphanedSamples < backupBuffer->getNumSamples()))
                    {
                        backupBuffer->addFrom (0, // destination channel
                                               streamSettings->samplesInBackupBuffer[localIndex], // destination start sample
                                               buffer, // source
                                               globalIndex, // source channel
                                               (int) remainingSamples, // source start sample
                                               (int) orphanedSamples, //  number of samples
                                               1.0f // gain to apply
                        );

                        streamSettings->samplesInBackupBuffer[localIndex] += orphanedSamples;
                    }

                    //std::cout << "Total copied: " << samplesToCopyFromOverflowBuffer + samplesToCopyFromIncomingBuffer << std::endl;

                    // now that our tempBuffer is ready, we can filter it and copy it into the
                    // original buffer
                    float* ptr = tempBuffer->getWritePointer (0);

                    int totalCopied = int (samplesToCopyFromOverflowBuffer + samplesToCopyFromIncomingBuffer);

                    if (totalCopied == 0)
                        continue;

                    streamSettings->bandpassfilters[localIndex]->process (totalCopied, &ptr);

                    /*if (i == 0)
                    {
                        std::cout << "np.array([";
                        for (int j = 0; j < totalCopied; j++)
                        {
                            std::cout << *(tempBuffer->getReadPointer(0, j)) << ", ";
                        }
                        
                        std::cout << "])";
                        std::cout << std::endl;
                        std::cout << "------------- " << std::endl;
                    }*/

                    // initialize variables
                    int sourceBufferPos = 0;
                    int sourceBufferSize = totalCopied;
                    double subSampleOffset = 0.0;
                    int nextPos = (sourceBufferPos + 1) % sourceBufferSize;

                    double destBufferPos;
                    int targetChannel;

                    //std::cout << "Ratio: " << ratio[globalIndex] << std::endl;

                    if (int (getParameter ("audio_output")->getValue()) == 0 || int (getParameter ("audio_output")->getValue()) == 1)
                        targetChannel = totalBufferChannels - 2;
                    else
                        targetChannel = totalBufferChannels - 1;

                    // code modified from "juce_ResamplingAudioSource.cpp":
                    for (destBufferPos = 0; destBufferPos < valuesNeeded; destBufferPos++)
                    {
                        float alpha = (float) subSampleOffset;
                        float invAlpha = 1.0f - alpha;

                        buffer.addFrom (targetChannel, // destChannel
                                        destBufferPos, // destSampleOffset
                                        *tempBuffer, // source
                                        0, // sourceChannel
                                        sourceBufferPos, // sourceSampleOffset
                                        1, // number of samples
                                        invAlpha); // gain to apply to source

                        buffer.addFrom (targetChannel, // destChannel
                                        destBufferPos, // destSampleOffset
                                        *tempBuffer, // source
                                        0, // sourceChannel
                                        nextPos, // sourceSampleOffset
                                        1, // number of samples
                                        alpha); // gain to apply to source

                        subSampleOffset += streamSettings->ratio;

                        while (subSampleOffset >= 1.0)
                        {
                            //if (++sourceBufferPos >= sourceBufferSize)
                            //    sourceBufferPos = 0;

                            ++sourceBufferPos;
                            nextPos = (sourceBufferPos + 1); //% sourceBufferSize;

                            if (nextPos >= sourceBufferSize)
                                nextPos = sourceBufferPos;

                            subSampleOffset -= 1.0;
                        }
                    }

                    //std::cout << "Source buffer pos: " << sourceBufferPos << std::endl;

                    //std::cout << "After upsampling: " << valuesNeeded << std::endl;

                    //std::cout << std::endl;

                    //ptr = buffer.getWritePointer(targetChannel);
                    //antialiasingfilters[i]->process(destBufferPos, &ptr);

                    /*if (i == 0)
                    {
                        std::cout << "np.array([";
                        for (int j = 0; j < valuesNeeded; j++)
                        {
                            std::cout << *(buffer.getReadPointer(targetChannel, j)) << ", ";
                        }
                        
                        std::cout << "])";
                        std::cout << std::endl;
                        std::cout << "------------- " << std::endl;
                    }*/

                } // end cycling through channels

                if (int (getParameter ("audio_output")->getValue()) == 1)
                {
                    // copy the signal into the right channel
                    buffer.addFrom (totalBufferChannels - 1, // destChannel
                                    0, // destSampleOffset
                                    buffer, // source
                                    totalBufferChannels - 2, // sourceChannel
                                    0, // sourceSampleOffset
                                    valuesNeeded, // number of samples
                                    1.0); // gain to apply to source
                }

            } // stream is selected

        } // loop through streams

    } // not muted

} // process
