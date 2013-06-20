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

#include <stdio.h>
#include "ChannelMappingNode.h"
#include "Editors/ChannelMappingEditor.h"



ChannelMappingNode::ChannelMappingNode()
    : GenericProcessor("Channel Map"), channelBuffer(1,10000)
{
    referenceArray.resize(1024); // make room for 1024 channels
    channelArray.resize(1024);

    for (int i = 0; i < referenceArray.size(); i++)
    {
        referenceArray.set(i, -1);
        channelArray.set(i, i);
    }

}

ChannelMappingNode::~ChannelMappingNode()
{

}

AudioProcessorEditor* ChannelMappingNode::createEditor()
{
    editor = new ChannelMappingEditor(this, true);

    std::cout << "Creating editor." << std::endl;

    return editor;
}



void ChannelMappingNode::updateSettings()
{
    if (getNumInputs() > 0)
        channelBuffer.setSize(getNumInputs(), 10000);

}


void ChannelMappingNode::setParameter(int parameterIndex, float newValue)
{

    if (parameterIndex == 1)
    {
        referenceArray.set(currentChannel, (int) newValue);
    }
    else
    {
        channelArray.set(currentChannel, (int) newValue);
    }

}

void ChannelMappingNode::process(AudioSampleBuffer& buffer,
                                 MidiBuffer& midiMessages,
                                 int& nSamples)
{

    // use copy constructor to set the data to refer to
    channelBuffer = buffer;

    // copy it back into the buffer according to the channel mapping
    buffer.clear();

    for (int i = 0; i < buffer.getNumChannels(); i++)
    {
        buffer.addFrom(i, // destChannel
                       0, // destStartSample
                       channelBuffer, // source
                       channelArray[i], // sourceChannel
                       0, // sourceStartSample
                       nSamples, // numSamples
                       1.0f // gain to apply to source (positive for original signal)
                      );

    }

    // now do the referencing
    for (int i = 0; i < buffer.getNumChannels(); i++)
    {

        if (referenceArray[i] > -1)
        {

            buffer.addFrom(i, // destChannel
                           0, // destStartSample
                           channelBuffer, // source
                           referenceArray[i], // sourceChannel
                           0, // sourceStartSample
                           nSamples, // numSamples
                           -1.0f // gain to apply to source (negative for reference)
                          );
        }

    }

}

