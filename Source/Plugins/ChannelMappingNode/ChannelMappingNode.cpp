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

#include <stdio.h>
#include "ChannelMappingNode.h"
#include "ChannelMappingEditor.h"



ChannelMappingNode::ChannelMappingNode()
    : GenericProcessor("Channel Map"), channelBuffer(1,10000)
{
    referenceArray.resize(1024); // make room for 1024 channels
    channelArray.resize(1024);

    for (int i = 0; i < referenceArray.size(); i++)
    {
        channelArray.set(i, i);
        referenceArray.set(i,-1);
        enabledChannelArray.set(i,true);
    }
    for (int i = 0; i < NUM_REFERENCES; i++)
    {
        referenceChannels.set(i, -1);
    }

}

ChannelMappingNode::~ChannelMappingNode()
{

}

AudioProcessorEditor* ChannelMappingNode::createEditor()
{
    editor = new ChannelMappingEditor(this, true);

    //std::cout << "Creating editor." << std::endl;

    return editor;
}



void ChannelMappingNode::updateSettings()
{
    if (getNumInputs() > 0)
        channelBuffer.setSize(getNumInputs(), 10000);

	if (editorIsConfigured)
	{
	    OwnedArray<Channel> oldChannels;
		oldChannels.swapWith(channels);
	    channels.clear();
		Array<bool> recordStates;

	    settings.numOutputs = 0;

	    for (int i = 0; i < getNumInputs(); i++)
	    {
			if ((enabledChannelArray[channelArray[i]]) && (channelArray[i] < oldChannels.size()))
	        {
				oldChannels[channelArray[i]]->mappedIndex = settings.numOutputs;
	            channels.add(oldChannels[channelArray[i]]);
				recordStates.add(oldChannels[i]->getRecordState());
	            settings.numOutputs++;
	        }

	    }
		oldChannels.clearQuick(false);
		for (int i = 0; i < settings.numOutputs; i++)
		{
			channels[i]->setRecordState(recordStates[i]);
		}
	}
}


void ChannelMappingNode::setParameter(int parameterIndex, float newValue)
{

    if (parameterIndex == 1)
    {
        referenceArray.set(currentChannel, (int) newValue);
    }
    else if (parameterIndex == 2)
    {
		referenceChannels.set((int)newValue, currentChannel);
    }
    else if (parameterIndex == 3)
    {
        enabledChannelArray.set(currentChannel, (newValue != 0) ? true : false);
    }
    else if (parameterIndex == 4)
    {
        editorIsConfigured = (newValue != 0) ? true : false;
    }
    else
    {
        channelArray.set(currentChannel, (int) newValue);
    }

}

void ChannelMappingNode::process(AudioSampleBuffer& buffer,
                                 MidiBuffer& midiMessages)
{
    int j=0;
    int i=0;
    int realChan;

    // use copy constructor to set the data to refer to
    channelBuffer = buffer;

   // buffer.clear();

    while (j < settings.numOutputs)
    {
        realChan = channelArray[i];
        if ((realChan < channelBuffer.getNumChannels()) && (enabledChannelArray[realChan]))
        {
            // copy it back into the buffer according to the channel mapping
            buffer.copyFrom(j, // destChannel
                           0, // destStartSample
                           channelBuffer.getReadPointer(realChan), // source
                           getNumSamples(j), // numSamples
                           1.0f // gain to apply to source (positive for original signal)
                          );

            // now do the referencing
            if ((referenceArray[realChan] > -1) && (referenceChannels[referenceArray[realChan]] > -1)
                && (referenceChannels[referenceArray[realChan]] < channelBuffer.getNumChannels()))
            {
                buffer.addFrom(j, // destChannel
                               0, // destStartSample
                               channelBuffer, // source
							   channels[referenceChannels[referenceArray[realChan]]]->index-1, // sourceChannel
                               0, // sourceStartSample
                               getNumSamples(j), // numSamples
                               -1.0f // gain to apply to source (negative for reference)
                              );
            }
            j++;
        }
        i++;

    }

}

