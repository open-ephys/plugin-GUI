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

#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include <stdio.h>

LfpDisplayNode::LfpDisplayNode()
    : GenericProcessor("LFP Viewer"),
      displayGain(1), bufferLength(5.0f),
      abstractFifo(100)
{
    //std::cout << " LFPDisplayNodeConstructor" << std::endl;
    displayBuffer = new AudioSampleBuffer(8, 100);

    arrayOfOnes = new float[5000];

    for (int n = 0; n < 5000; n++)
    {
        arrayOfOnes[n] = 1;
    }

}

LfpDisplayNode::~LfpDisplayNode()
{

}

AudioProcessorEditor* LfpDisplayNode::createEditor()
{

    editor = new LfpDisplayEditor(this, true);
    return editor;

}

void LfpDisplayNode::updateSettings()
{
    // std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;

    channelForEventSource.clear();
    eventSourceNodes.clear();
    ttlState.clear();

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if (!eventSourceNodes.contains(eventChannels[i]->sourceNodeId))
        {
            eventSourceNodes.add(eventChannels[i]->sourceNodeId);

        }
    }

    numEventChannels = eventSourceNodes.size();

    for (int i = 0; i < eventSourceNodes.size(); i++)
    {
        channelForEventSource[eventSourceNodes[i]] = getNumInputs() + i;
        ttlState[eventSourceNodes[i]] = 0;
        Channel* eventChan = new Channel(this, getNumInputs() + i, EVENT_CHANNEL);
        eventChan->sourceNodeId = eventSourceNodes[i];
        channels.add(eventChan); // add a channel for event data for each source node
    }

    displayBufferIndex.clear();
    displayBufferIndex.insertMultiple(0, 0, getNumInputs() + numEventChannels);

}

bool LfpDisplayNode::resizeBuffer()
{
    int nSamples = (int) getSampleRate()*bufferLength;
    int nInputs = getNumInputs();

    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

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
        LfpDisplayEditor* editor = (LfpDisplayEditor*) getEditor();
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

void LfpDisplayNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    //Sets Parameter in parameters array for processor
    Parameter* parameterPointer = parameters.getRawDataPointer();
    parameterPointer = parameterPointer+parameterIndex;
    parameterPointer->setValue(newValue, currentChannel);

    //std::cout << "Saving Parameter from " << currentChannel << ", channel ";

    LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
    if (ed->canvas != 0)
        ed->canvas->setParameter(parameterIndex, newValue);
}

void LfpDisplayNode::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        int eventSourceNode = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);
        int eventTime = event.getTimeStamp();

        int samplesLeft = totalSamples - eventTime;

        //	std::cout << "Received event from " << eventNodeId << ", channel "
        //	          << eventChannel << ", with ID " << eventId << std::endl;
        //
        int bufferIndex = (displayBufferIndex[channelForEventSource[eventSourceNode]] + eventTime);// % displayBuffer->getNumSamples();

        if (eventId == 1)
        {
            ttlState[eventSourceNode] |= (1L << eventChannel);
        }
        else
        {
            ttlState[eventSourceNode] &= ~(1L << eventChannel);
        }

        if (samplesLeft + bufferIndex < displayBuffer->getNumSamples())
        {

            //	std::cout << bufferIndex << " " << samplesLeft << " " << ttlState << std::endl;

            displayBuffer->copyFrom(channelForEventSource[eventSourceNode],  // destChannel
                                    bufferIndex,		// destStartSample
                                    arrayOfOnes, 		// source
                                    samplesLeft, 		// numSamples
                                    float(ttlState[eventSourceNode]));   // gain
        }
        else
        {

            int block2Size = (samplesLeft + bufferIndex) % displayBuffer->getNumSamples();
            int block1Size = samplesLeft - block2Size;

            //std::cout << "OVERFLOW." << std::endl;

            //std::cout << bufferIndex << " " << block1Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(channelForEventSource[eventSourceNode],  // destChannel
                                    bufferIndex,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block1Size, 		// numSamples
                                    float(ttlState[eventSourceNode]));   // gain

            //std::cout << 0 << " " << block2Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(channelForEventSource[eventSourceNode],  // destChannel
                                    0,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block2Size, 		// numSamples
                                    float(ttlState[eventSourceNode]));   // gain


        }


        // 	std::cout << "ttlState: " << ttlState << std::endl;

        // std::cout << "Received event from " << eventNodeId <<
        //              " on channel " << eventChannel <<
        //             " with value " << eventId <<
        //             " at timestamp " << event.getTimeStamp() << std::endl;


    }

}

void LfpDisplayNode::initializeEventChannels()
{

    for (int i = 0; i < eventSourceNodes.size(); i++)
    {

        int chan = channelForEventSource[eventSourceNodes[i]];
        int index = displayBufferIndex[chan];

        totalSamples = getNumSamples(getNumInputs()+i);

        if (index + totalSamples < displayBuffer->getNumSamples())
        {

            //	std::cout << getNumInputs()+1 << " " << displayBufferIndex << " " << totalSamples << " " << ttlState << std::endl;
            //
            displayBuffer->copyFrom(chan,  // destChannel
                                    index,		// destStartSample
                                    arrayOfOnes, 		// source
                                    totalSamples, 		// numSamples
                                    float(ttlState[eventSourceNodes[i]]));   // gain
        }
        else
        {

            int block2Size = (displayBufferIndex[eventSourceNodes[i]] + totalSamples) % displayBuffer->getNumSamples();
            int block1Size = totalSamples - block2Size;

            // std::cout << "OVERFLOW." << std::endl;
            // std::cout << bufferIndex << " " << block1Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(chan,    // destChannel
                                    index,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block1Size, 		// numSamples
                                    float(ttlState[eventSourceNodes[i]]));   // gain
            // std::cout << 0 << " " << block2Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(chan,  // destChannel
                                    0,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block2Size, 		// numSamples
                                    float(ttlState[eventSourceNodes[i]]));   // gain


        }
    }   
}

void LfpDisplayNode::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    initializeEventChannels();

    checkForEvents(events); // update timestamp, see if we got any TTL events

    for (int i = 0; i < eventSourceNodes.size(); i++)
    {
        int originalBufferIndex = displayBufferIndex[eventSourceNodes[i]];
        displayBufferIndex.set(channelForEventSource[eventSourceNodes[i]], 
                                originalBufferIndex + getNumSamples(getNumInputs()+i)); 
    }

    for (int chan = 0; chan < buffer.getNumChannels(); chan++)
    {
         int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex[chan];
         int nSamples = getNumSamples(chan);

        if (nSamples < samplesLeft)
        {

            displayBuffer->copyFrom(chan,  			// destChannel
                                    displayBufferIndex[chan], // destStartSample
                                    buffer, 			// source
                                    chan, 				// source channel
                                    0,					// source start sample
                                    nSamples); 			// numSamples
        
            displayBufferIndex.set(chan, displayBufferIndex[chan] + nSamples);
        }
        else
        {

            int extraSamples = nSamples - samplesLeft;

            displayBuffer->copyFrom(chan,  				// destChannel
                                    displayBufferIndex[chan], // destStartSample
                                        buffer, 			// source
                                        chan, 				// source channel
                                        0,					// source start sample
                                        samplesLeft); 		// numSamples

                displayBuffer->copyFrom(chan,
                                        0,
                                        buffer,
                                        chan,
                                        samplesLeft,
                                        extraSamples);

            displayBufferIndex.set(chan, extraSamples);
        }
    }

}

