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

#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include <stdio.h>

using namespace LfpViewer;


LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor  ("LFP Viewer")
    , displayGain       (1)
    , bufferLength      (10.0f)
    , abstractFifo      (100)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    displayBuffer = new AudioSampleBuffer (8, 100);

    const int heapSize = 5000;
    arrayOfOnes = new float[heapSize];
    for (int n = 0; n < heapSize; ++n)
    {
        arrayOfOnes[n] = 1;
    }

	subprocessorToDraw = 0;
	numSubprocessors = -1;
	numChannelsInSubprocessor = 0;
	updateSubprocessorsFlag = true;
}


LfpDisplayNode::~LfpDisplayNode()
{
    delete[] arrayOfOnes;
}


AudioProcessorEditor* LfpDisplayNode::createEditor()
{
    editor = new LfpDisplayEditor (this, true);
    return editor;
}


void LfpDisplayNode::updateSettings()
{

    std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;

	numChannelsInSubprocessor = 0;
	int totalSubprocessors = 0;
	int currentSubprocessor = -1;

	for (int i = 0; i < getNumInputs(); i++)
	{
		int channelSubprocessor = getDataChannel(i)->getSubProcessorIdx();

		if (currentSubprocessor != channelSubprocessor)
		{
			totalSubprocessors++;
			currentSubprocessor = channelSubprocessor;
		}

		if (channelSubprocessor == subprocessorToDraw)
		{
			numChannelsInSubprocessor++;
			subprocessorSampleRate = getDataChannel(i)->getSampleRate();
		}
	}

	std::cout << "Re-setting num inputs on LfpDisplayNode to " << numChannelsInSubprocessor << std::endl;
	std::cout << "Sample rate = " << subprocessorSampleRate << std::endl;

    channelForEventSource.clear();
    eventSourceNodes.clear();
    ttlState.clear();

	for (int i = 0; i < eventChannelArray.size(); ++i)
	{
		uint32 sourceID = getChannelSourceID(eventChannelArray[i]);
		if (!eventSourceNodes.contains(sourceID))
		{
			eventSourceNodes.add(sourceID);

		}
	}

    numEventChannels = eventSourceNodes.size();

    std::cout << "Found " << numEventChannels << " event channels." << std::endl;

    for (int i = 0; i < eventSourceNodes.size(); ++i)
    {
		std::cout << "Adding channel " << numChannelsInSubprocessor + i << " for event source node " << eventSourceNodes[i] << std::endl;

		channelForEventSource[eventSourceNodes[i]] = numChannelsInSubprocessor + i;
        ttlState[eventSourceNodes[i]] = 0;
    }

    displayBufferIndex.clear();
	displayBufferIndex.insertMultiple(0, 0, numChannelsInSubprocessor + numEventChannels);
    
    // update the editor's subprocessor selection display and sample rate
	LfpDisplayEditor * ed = (LfpDisplayEditor*)getEditor();
	ed->updateSubprocessorSelectorOptions();
	numSubprocessors = totalSubprocessors;
}

uint32 LfpDisplayNode::getChannelSourceID(const EventChannel* event) const
{
	int metaDataIndex = event->findMetaData(MetaDataDescriptor::UINT16, 3, "source.channel.identifier.full");
	if (metaDataIndex < 0)
	{
		return getProcessorFullId(event->getSourceNodeID(), event->getSubProcessorIdx());
	}
	uint16 values[3];
	event->getMetaDataValue(metaDataIndex)->getValue(static_cast<uint16*>(values));
	return getProcessorFullId(values[1], values[2]);
}

void LfpDisplayNode::setSubprocessor(int sp)
{

	subprocessorToDraw = sp;
	std::cout << "LfpDisplayNode setting subprocessor to " << sp << std::endl;
	updateSubprocessorsFlag = false;
	
}

int LfpDisplayNode::getNumSubprocessorChannels()
{
	return numChannelsInSubprocessor;
}

float LfpDisplayNode::getSubprocessorSampleRate()
{
	return subprocessorSampleRate;
}

bool LfpDisplayNode::resizeBuffer()
{
	int nSamples = (int)subprocessorSampleRate * bufferLength;
	int nInputs = numChannelsInSubprocessor;

	std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << ", event channels: " << numEventChannels << std::endl;

	if (nSamples > 0 && nInputs > 0)
	{
		abstractFifo.setTotalSize(nSamples);
		displayBuffer->setSize(nInputs + numEventChannels, nSamples); // add extra channels for TTLs

		return true;
	}
	else
	{
		return false;
	}
	
}


bool LfpDisplayNode::enable()
{

	if (resizeBuffer())
	{
		LfpDisplayEditor* editor = (LfpDisplayEditor*)getEditor();
		editor->enable();
		return true;
	}
	else
	{
		return false;
	}

}


bool LfpDisplayNode::disable()
{
    LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
    editor->disable();
    return true;
}


void LfpDisplayNode::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);
    //
    //Sets Parameter in parameters array for processor
    parameters[parameterIndex]->setValue (newValue, currentChannel);

    //std::cout << "Saving Parameter from " << currentChannel << ", channel ";

    LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
    if (ed->canvas != 0)
        ed->canvas->setParameter (parameterIndex, newValue);
}


void LfpDisplayNode::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition)
{
    if (Event::getEventType(event) == EventChannel::TTL)
    {
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
        
        //int eventNodeId = *(dataptr+1);
        const int eventId = ttl->getState() ? 1 : 0;
        const int eventChannel = ttl->getChannel();
        const int eventTime = samplePosition;
        const uint32 eventSourceNodeId = getChannelSourceID(eventInfo);
        
		//std::cout << "Received event on channel " << eventChannel << std::endl;
		//std::cout << "Copying to channel " << channelForEventSource[eventSourceNodeId] << std::endl;
        
        const int chan          = channelForEventSource[eventSourceNodeId];
        const int index         = (displayBufferIndex[chan] + eventTime) % displayBuffer->getNumSamples();
        const int samplesLeft   = displayBuffer->getNumSamples() - index;
        const int nSamples = getNumSourceSamples(eventSourceNodeId) - eventTime;
        
        if (eventId == 1)
        {
            ttlState[eventSourceNodeId] |= (1L << eventChannel);
        }
        else
        {
            ttlState[eventSourceNodeId] &= ~(1L << eventChannel);
        }
        
        if (nSamples < samplesLeft)
        {
            displayBuffer->copyFrom (channelForEventSource[eventSourceNodeId],  // destChannel
                                     index,                               // destStartSample
                                     arrayOfOnes,                               // source
                                     nSamples,                             // numSamples
                                     float (ttlState[eventSourceNodeId]));      // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;
            
            displayBuffer->copyFrom (channelForEventSource[eventSourceNodeId],  // destChannel
                                     index,                               // destStartSample
                                     arrayOfOnes,                               // source
                                     samplesLeft,                                // numSamples
                                     float (ttlState[eventSourceNodeId]));      // gain
            
            displayBuffer->copyFrom (channelForEventSource[eventSourceNodeId],  // destChannel
                                     0,                                         // destStartSample
                                     arrayOfOnes,                               // source
                                     extraSamples,                                // numSamples
                                     float (ttlState[eventSourceNodeId]));      // gain
        }
        
        //         std::cout << "Received event from " << eventSourceNodeId
        //                   << " on channel " << eventChannel
        //                   << " with value " << eventId
        //                   << " at timestamp " << event.getTimeStamp() << std::endl;
    }
}


void LfpDisplayNode::initializeEventChannels()
{

	//std::cout << "Initializing events..." << std::endl;

    for (int i = 0; i < eventSourceNodes.size(); ++i)
    {
        const int chan          = channelForEventSource[eventSourceNodes[i]];
        const int index         = displayBufferIndex[chan];
        const int samplesLeft   = displayBuffer->getNumSamples() - index;
		const int nSamples = getNumSourceSamples(eventSourceNodes[i]);

		//std::cout << chan << " " << index << " " << samplesLeft << " " << nSamples << std::endl;
        
        if (nSamples < samplesLeft)
        {

            displayBuffer->copyFrom (chan,                                      // destChannel
                                     index,                                     // destStartSample
                                     arrayOfOnes,                               // source
                                     nSamples,                                  // numSamples
                                     float (ttlState[eventSourceNodes[i]]));    // gain
        }
        else
        {
            int extraSamples = nSamples - samplesLeft;

            displayBuffer->copyFrom (chan,                                      // destChannel
                                     index,                                     // destStartSample
                                     arrayOfOnes,                               // source
                                     samplesLeft,                               // numSamples
                                     float (ttlState[eventSourceNodes[i]]));    // gain

            displayBuffer->copyFrom (chan,                                      // destChannel
                                     0,                                         // destStartSample
                                     arrayOfOnes,                               // source
                                     extraSamples,                              // numSamples
                                     float (ttlState[eventSourceNodes[i]]));    // gain
        }
    }
}

void LfpDisplayNode::finalizeEventChannels()
{
    for (int i = 0; i < eventSourceNodes.size(); ++i)
    {
        const int chan          = channelForEventSource[eventSourceNodes[i]];
        const int index         = displayBufferIndex[chan];
        const int samplesLeft   = displayBuffer->getNumSamples() - index;
        const int nSamples = getNumSourceSamples(eventSourceNodes[i]);
        
        int newIdx = 0;
        
        if (nSamples < samplesLeft)
        {
            newIdx = index + nSamples;
        }
        else
        {
            newIdx = nSamples - samplesLeft;
        }
        
        displayBufferIndex.set(chan, newIdx);
    }
}


void LfpDisplayNode::process (AudioSampleBuffer& buffer)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

	if (true)
	{
		ScopedLock displayLock(displayMutex);

		if (true)
		{
			initializeEventChannels();
			checkForEvents(); // see if we got any TTL events
			finalizeEventChannels();
		}

		if (true)
		{
			int channelIndex = -1;

			for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
			{
				if (getDataChannel(chan)->getSubProcessorIdx() == subprocessorToDraw)
				{
					channelIndex++;
					const int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex[channelIndex];
					const int nSamples = getNumSamples(chan);

					if (nSamples < samplesLeft)
					{
						displayBuffer->copyFrom(channelIndex,                      // destChannel
							displayBufferIndex[channelIndex],  // destStartSample
							buffer,                    // source
							chan,                      // source channel
							0,                         // source start sample
							nSamples);                 // numSamples

						displayBufferIndex.set(channelIndex, displayBufferIndex[channelIndex] + nSamples);
					}
					else
					{
						const int extraSamples = nSamples - samplesLeft;

						displayBuffer->copyFrom(channelIndex,                      // destChannel
							displayBufferIndex[channelIndex],  // destStartSample
							buffer,                    // source
							chan,                      // source channel
							0,                         // source start sample
							samplesLeft);              // numSamples

						displayBuffer->copyFrom(channelIndex,                      // destChannel
							0,                         // destStartSample
							buffer,                    // source
							chan,                      // source channel
							samplesLeft,               // source start sample
							extraSamples);             // numSamples

						displayBufferIndex.set(channelIndex, extraSamples);
					}
				}
			}
		}
	}
}

