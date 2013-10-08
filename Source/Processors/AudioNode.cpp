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
    : GenericProcessor("Audio Node"), audioEditor(0), volume(0.00001f), overflowBuffer(2,10000)
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
    overflowBufferIndex = 0;

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

    int totalSamples = nSamples + overflowBufferIndex;
    
    if (channelPointers.size() > 0)
    {

        // fill the beginning of the buffer from the overflowBuffer
        
        buffer.addFrom(0,       // destination channel
               0,           // destination start sample
               overflowBuffer,      // source
               0,           // source channel
               0,           // source start sample
               overflowBufferIndex, //  number of samples
               1.0f       // gain to apply
              );

        buffer.addFrom(1,       // destination channel
               0,           // destination start sample
               overflowBuffer,      // source
               1,           // source channel
               0,           // source start sample
               overflowBufferIndex, //  number of samples
               1.0f       // gain to apply
              );

        for (int i = 2; i < buffer.getNumChannels(); i++)
        {

            if (channelPointers[i-2]->isMonitored)
            {
                gain = volume/(float(0x7fff) * channelPointers[i-2]->bitVolts);

                if (totalSamples <= numSamplesExpected)
                {
                    
                    // we don't have any overflow

                    buffer.addFrom(0,       // destination channel
                               overflowBufferIndex,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               nSamples, //  number of samples
                               gain       // gain to apply
                              );

                    buffer.addFrom(1,       // destination channel
                               overflowBufferIndex,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               nSamples, //  number of samples
                               gain       // gain to apply
                              );

                    overflowBufferIndex = 0;
                    overflowBuffer.clear();

                    int overflowSamples = 0;

                    // std::cout << "Total samples: " << totalSamples << std::endl;
                    // std::cout << "Samples expected: " << numSamplesExpected << std::endl;
                    // std::cout << "Overflow samples: " << overflowSamples << std::endl;

                } else {
                {

                    // first copy to the actual buffer

                    int overflowSamples = totalSamples - numSamplesExpected;

                    // std::cout << "Total samples: " << totalSamples << std::endl;
                    // std::cout << "Samples expected: " << numSamplesExpected << std::endl;
                    // std::cout << "Overflow samples: " << overflowSamples << std::endl;

                     buffer.addFrom(0,       // destination channel
                               overflowBufferIndex,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               nSamples - overflowSamples, //  number of samples
                               gain       // gain to apply
                              );

                    buffer.addFrom(1,       // destination channel
                               overflowBufferIndex,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               0,           // source start sample
                               nSamples - overflowSamples, //  number of samples
                               gain       // gain to apply
                              );

                    // now copy the rest to the overflowBuffer

                    
                    overflowBuffer.clear();

                     overflowBuffer.addFrom(0,       // destination channel
                               0,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               nSamples - overflowSamples,           // source start sample
                               overflowSamples, //  number of samples
                               gain       // gain to apply
                              );

                    overflowBuffer.addFrom(1,       // destination channel
                               0,           // destination start sample
                               buffer,      // source
                               i,           // source channel
                               nSamples - overflowSamples,           // source start sample
                               overflowSamples, //  number of samples
                               gain       // gain to apply
                              );

                    overflowBufferIndex = overflowSamples;

                }                

            }

        }
    }
}

    nSamples = numSamplesExpected;

}
