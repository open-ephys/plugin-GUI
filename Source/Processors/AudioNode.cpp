/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


#include "AudioNode.h"
#include "Channel.h"

AudioNode::AudioNode()
    : GenericProcessor("Audio Node"), audioEditor(0), volume(0.00001f), 
    bufferA(2,10000),
    bufferB(2,10000)
{

    settings.numInputs = 2048;
    settings.numOutputs = 2;

    // 128 inputs, 2 outputs (left and right channel)
    setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

    nextAvailableChannel = 2; // keep first two channels empty

    numSamplesExpected = 1024;

    samplesInOverflowBuffer = 0;
    samplesInBackupBuffer = 0;

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

    channelPointers.clear();

    samplesInOverflowBuffer = 0;
    samplesInBackupBuffer = 0;

}

void AudioNode::updateBufferSize()
{
    //AudioEditor* editor = (AudioEditor*) getEditor();
    audioEditor->updateBufferSizeText();
    
}

void AudioNode::setChannel(Channel* ch)
{

    int channelNum = channelPointers.indexOf(ch);

    std::cout << "Audio node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);
}

void AudioNode::setChannelStatus(Channel* chan, bool status)
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

    channelPointers.add(sourceNode->channels[chan]);

}

void AudioNode::setParameter(int parameterIndex, float newValue)
{
    // change left channel, right channel, or volume
    if (parameterIndex == 1)
    {
        // volume level
        volume = newValue*0.1f;

    }
    else if (parameterIndex == 100)
    {

        channelPointers[currentChannel]->isMonitored = true;

    }
    else if (parameterIndex == -100)
    {

        channelPointers[currentChannel]->isMonitored = false;
    }

}

void AudioNode::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{


    std::cout << "Processor sample rate: " << getSampleRate() << std::endl;
    std::cout << "Audio card sample rate: " << sampleRate_ << std::endl;
    std::cout << "Samples per block: " << estimatedSamplesPerBlock << std::endl;

    numSamplesExpected = (int) (getSampleRate()/sampleRate_*float(estimatedSamplesPerBlock)) + 1; 
    // processor sample rate divided by sound card sample rate

    samplesInBackupBuffer = 0;
    samplesInOverflowBuffer = 0;

    bufferA.clear();
    bufferB.clear();

    bufferSwap = false;
}

void AudioNode::process(AudioSampleBuffer& buffer,
                        MidiBuffer& midiMessages,
                        int& nSamples)
{
    float gain;

    // clear the left and right channels
    buffer.clear(0,0,buffer.getNumSamples());
    buffer.clear(1,0,buffer.getNumSamples());

    if (1)
    {

        AudioSampleBuffer* overflowBuffer;
        AudioSampleBuffer* backupBuffer;

        if (!bufferSwap)
        {
            overflowBuffer = &bufferA;
            backupBuffer = &bufferB;
            bufferSwap = true;
        } else {
            overflowBuffer = &bufferB;
            backupBuffer = &bufferA;
            bufferSwap = false;
        }

        backupBuffer->clear();
        
        samplesInOverflowBuffer = samplesInBackupBuffer; // size of buffer after last round
        samplesInBackupBuffer = 0;

        int samplesToCopy = 0;
        int orphanedSamples = 0;

        if (channelPointers.size() > 0)
        {
            bool copiedBuffer = false;

            for (int i = 2; i < buffer.getNumChannels(); i++)
            {

                if (channelPointers[i-2]->isMonitored)
                {

                    if (!copiedBuffer)
                    {
                        // 1. copy overflow buffer

                        samplesToCopy = ((samplesInOverflowBuffer <= numSamplesExpected) ? 
                                                  samplesInOverflowBuffer :
                                                  numSamplesExpected);

                      //  std::cout << " " << std::endl;
                      //  std::cout << "Copying " << samplesToCopy << " samples from overflow buffer of " << samplesInOverflowBuffer << " samples." << std::endl;

                        if (samplesToCopy > 0)
                        {

                            buffer.addFrom(0,    // destination channel
                               0,                // destination start sample
                               *overflowBuffer,  // source
                               0,                // source channel
                               0,                // source start sample
                               samplesToCopy,    // number of samples
                               1.0f              // gain to apply
                              );

                            buffer.addFrom(1,       // destination channel
                               0,           // destination start sample
                               *overflowBuffer,      // source
                               1,           // source channel
                               0,           // source start sample
                               samplesToCopy, //  number of samples
                               1.0f       // gain to apply
                              );


                            int leftoverSamples = samplesInOverflowBuffer - samplesToCopy;

                       //     std::cout << "Samples remaining in overflow buffer: " << leftoverSamples << std::endl;

                            if (leftoverSamples > 0)
                            {

                                // move remaining samples to the backup buffer

                                 backupBuffer->addFrom(0, // destination channel
                                   0,                     // destination start sample
                                   *overflowBuffer,       // source
                                   0,                     // source channel
                                   samplesToCopy,         // source start sample
                                   leftoverSamples,       // number of samples
                                   1.0f                   // gain to apply
                                  );

                                backupBuffer->addFrom(1,  // destination channel
                                   0,                     // destination start sample
                                   *overflowBuffer,        // source
                                   1,                     // source channel
                                   samplesToCopy,         // source start sample
                                   leftoverSamples,       // number of samples
                                   1.0f                   // gain to apply
                                  );

                            }

                            samplesInBackupBuffer = leftoverSamples;

                        }

                        copiedBuffer = true;

                    } // copying buffer

                    gain = volume/(float(0x7fff) * channelPointers[i-2]->bitVolts);

                    int remainingSamples = numSamplesExpected - samplesToCopy;

                  //  std::cout << "Copying " << remainingSamples << " samples from incoming buffer of " << nSamples << " samples." << std::endl;

                    int samplesToCopy2 = ((remainingSamples <= nSamples) ? 
                                                  remainingSamples :
                                                  nSamples);

                    if (samplesToCopy2 > 0)
                    {

                        buffer.addFrom(0,       // destination channel
                                   samplesToCopy,           // destination start sample
                                   buffer,      // source
                                   i,           // source channel
                                   0,           // source start sample
                                   remainingSamples, //  number of samples
                                   gain       // gain to apply
                                  );

                        buffer.addFrom(1,       // destination channel
                                   samplesToCopy,           // destination start sample
                                   buffer,      // source
                                   i,           // source channel
                                   0,           // source start sample
                                   remainingSamples, //  number of samples
                                   gain       // gain to apply
                                  );
                    }

                    orphanedSamples = nSamples - samplesToCopy2;

                   // std::cout << "Samples remaining in incoming buffer: " << orphanedSamples << std::endl;


                    if (orphanedSamples > 0)
                    {
                          backupBuffer->addFrom(0,       // destination channel
                                   samplesInBackupBuffer,           // destination start sample
                                   buffer,      // source
                                   i,           // source channel
                                   remainingSamples,           // source start sample
                                   orphanedSamples, //  number of samples
                                   gain       // gain to apply
                                  );

                        backupBuffer->addFrom(0,       // destination channel
                                   samplesInBackupBuffer,           // destination start sample
                                   buffer,      // source
                                   i,           // source channel
                                   remainingSamples,           // source start sample
                                   orphanedSamples, //  number of samples
                                   gain       // gain to apply
                                  );

                    }

                }
            }

        }

        samplesInBackupBuffer += orphanedSamples;
        nSamples = numSamplesExpected;
   
    }
}
