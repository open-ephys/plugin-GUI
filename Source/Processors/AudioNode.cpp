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
    overflowBuffer(2,10000),
    overflowOverflowBuffer(2,10000)
{

    settings.numInputs = 2048;
    settings.numOutputs = 2;

    // 128 inputs, 2 outputs (left and right channel)
    setPlayConfigDetails(getNumInputs(), getNumOutputs(), 44100.0, 128);

    //leftChan.clear();
    //rightChan.clear();

    nextAvailableChannel = 2; // keep first two channels empty

    //overflowBuffer.setSize(2, 10000); // 2 channels x 10000 samples

    numSamplesExpected = 1024;
    overflowBufferEndIndex = 0;
    overflowBufferStartIndex = 0;
    overflowSamples = 0;

}


AudioNode::~AudioNode()
{



}

AudioProcessorEditor* AudioNode::createEditor()
{

    audioEditor = new AudioEditor(this);
    //audioEditor->setUIComponent(getUIComponent());
    //audioEditor->updateBufferSizeText();

   // setEditor(editor);

    return audioEditor;

}

void AudioNode::resetConnections()
{

    nextAvailableChannel = 2; // start connections at channel 2
    wasConnected = false;

    channelPointers.clear();

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

    //setCurrentChannel(nextAvailableChannel);

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

    //if (chan != getProcessorGraph()->midiChannelIndex)
    //{

    int channelIndex = getNextChannel(false);

    setPlayConfigDetails(channelIndex+1,0,44100.0,128);

    channelPointers.add(sourceNode->channels[chan]);

    //} else {

    // Can't monitor events at the moment!
    //	}

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

        // add current channel
        // if (!leftChan.contains(currentChannel))
        // {
        // 	leftChan.add(currentChannel);
        // 	rightChan.add(currentChannel);
        // }
    }
    else if (parameterIndex == -100)
    {

        channelPointers[currentChannel]->isMonitored = false;
        // remove current channel
        // if (leftChan.contains(currentChannel))
        // {
        // 	leftChan.remove(leftChan.indexOf(currentChannel));
        // 	rightChan.remove(rightChan.indexOf(currentChannel));
        // }
    }

}

void AudioNode::prepareToPlay(double sampleRate_, int estimatedSamplesPerBlock)
{


    std::cout << "Processor sample rate: " << getSampleRate() << std::endl;
    std::cout << "Audio card sample rate: " << sampleRate_ << std::endl;
    std::cout << "Samples per block: " << estimatedSamplesPerBlock << std::endl;

    numSamplesExpected = (int) (getSampleRate()/sampleRate_*float(estimatedSamplesPerBlock)); 
    // processor sample rate divided by sound card sample rate

    overflowBufferStartIndex = 0;
    overflowBufferEndIndex = 0;
    overflowSamples = 0;
}

void AudioNode::process(AudioSampleBuffer& buffer,
                        MidiBuffer& midiMessages,
                        int& nSamples)
{
    float gain;
    //std::cout << "Audio node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    // clear the left and right channels
    buffer.clear(0,0,buffer.getNumSamples());
    buffer.clear(1,0,buffer.getNumSamples());

    if (overflowSamples > 1024)
    {
        std::cout << "Clearing overflow buffer!" << std::endl;
        // just forget about it
        overflowBuffer.clear();
        overflowSamples = 0;
    }

    int totalAvailableSamples = nSamples + overflowSamples;
    int leftoverSamples = 0;
    int samplesFromOverflowBuffer = 0;
    int remainingSamples = 0;

   // jassert (sourceStartSample >= 0 && sourceStartSample + numSamples <= source.size);

    if (channelPointers.size() > 0)
    {

        // 2. now copy from the incoming buffer

        bool copiedBuffer = false;

        for (int i = 2; i < buffer.getNumChannels(); i++)
        {

            if (channelPointers[i-2]->isMonitored)
            {

                if (!copiedBuffer)
                {
                    // 1. copy overflow buffer

                    samplesFromOverflowBuffer = ((overflowSamples <= numSamplesExpected) ? 
                                              overflowSamples :
                                              numSamplesExpected);

                    std::cout << " " << std::endl;
                    std::cout << "Copying " << samplesFromOverflowBuffer << " samples from overflow buffer." << std::endl;

                    if (samplesFromOverflowBuffer > 0)
                    {

                        buffer.addFrom(0,   // destination channel
                           0,               // destination start sample
                           overflowBuffer,  // source
                           0,               // source channel
                           0,               // source start sample
                           samplesFromOverflowBuffer, //  number of samples
                           1.0f             // gain to apply
                          );

                        buffer.addFrom(1,       // destination channel
                           0,           // destination start sample
                           overflowBuffer,      // source
                           1,           // source channel
                           0,           // source start sample
                           samplesFromOverflowBuffer, //  number of samples
                           1.0f       // gain to apply
                          );

                        overflowBuffer.clear(0, samplesFromOverflowBuffer);

                        std::cout << "Samples remaining in overflow buffer: " << overflowSamples - samplesFromOverflowBuffer << std::endl;


                        if (samplesFromOverflowBuffer < overflowSamples)
                        {

                            // move remaining samples to the front of the buffer
                            // have to double-copy because copying from a buffer into itself is not allowed

                             overflowOverflowBuffer.addFrom(0,   // destination channel
                               0,               // destination start sample
                               overflowBuffer,  // source
                               0,               // source channel
                               samplesFromOverflowBuffer,               // source start sample
                               overflowSamples - samplesFromOverflowBuffer,   //  number of samples
                               1.0f             // gain to apply
                              );

                            overflowOverflowBuffer.addFrom(1,   // destination channel
                               0,               // destination start sample
                               overflowBuffer,  // source
                               1,               // source channel
                               samplesFromOverflowBuffer,               // source start sample
                               overflowSamples - samplesFromOverflowBuffer,   //  number of samples
                               1.0f             // gain to apply
                              );

                            overflowBuffer.addFrom(0,   // destination channel
                               0,               // destination start sample
                               overflowOverflowBuffer,  // source
                               0,               // source channel
                               0,               // source start sample
                               overflowSamples - samplesFromOverflowBuffer,   //  number of samples
                               1.0f             // gain to apply
                              );

                            overflowBuffer.addFrom(1,   // destination channel
                               0,               // destination start sample
                               overflowOverflowBuffer,  // source
                               1,               // source channel
                               0,               // source start sample
                               overflowSamples - samplesFromOverflowBuffer,   //  number of samples
                               1.0f             // gain to apply
                              );

                            overflowOverflowBuffer.clear();

                        }

                        overflowSamples = overflowSamples - samplesFromOverflowBuffer;

                    }

                    int remainingSamples = numSamplesExpected - samplesFromOverflowBuffer;

                    std::cout << "Copying " << remainingSamples << " samples from incoming buffer of " << nSamples << " samples" << std::endl;

                    leftoverSamples = nSamples - remainingSamples;

                    if (leftoverSamples < 0)
                    {
                        leftoverSamples = nSamples;
                    }

                    jassert (leftoverSamples >= 0);

                    std::cout << "Samples remaining in incoming buffer: " << leftoverSamples << std::endl;

                    copiedBuffer = true;
                }

                gain = volume/(float(0x7fff) * channelPointers[i-2]->bitVolts);

                if (remainingSamples > 0)
                {

                    buffer.addFrom(0,       // destination channel
                               samplesFromOverflowBuffer,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               remainingSamples, //  number of samples
                               gain       // gain to apply
                              );

                    buffer.addFrom(1,       // destination channel
                               samplesFromOverflowBuffer,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               remainingSamples, //  number of samples
                               gain       // gain to apply
                              );
                }

                if (leftoverSamples > 0)
                {
                      overflowBuffer.addFrom(0,       // destination channel
                               overflowSamples,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               remainingSamples,           // source start sample
                               leftoverSamples, //  number of samples
                               gain       // gain to apply
                              );

                    overflowBuffer.addFrom(0,       // destination channel
                               overflowSamples,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               remainingSamples,           // source start sample
                               leftoverSamples, //  number of samples
                               gain       // gain to apply
                              );

                }

            }
        }



        overflowSamples += leftoverSamples;

    }

    nSamples = numSamplesExpected;

}
