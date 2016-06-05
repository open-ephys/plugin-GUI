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

#include "LfpTriggeredAverageNode.h"
#include "LfpTriggeredAverageCanvas.h"
#include <stdio.h>

LfpTriggeredAverageNode::LfpTriggeredAverageNode()
    : GenericProcessor      ("LFP Trig. Avg.")
    , displayBufferIndex    (0)
    , displayGain           (1)
    , bufferLength          (5.0f)
    , abstractFifo          (100)
    , ttlState              (0)
{
    setProcessorType (PROCESSOR_TYPE_SINK);

    std::cout << " LfpTriggeredAverageNode Constructor" << std::endl;
    displayBuffer = new AudioSampleBuffer (8, 100);
    eventBuffer = new MidiBuffer;

    arrayOfOnes = new float[5000];

    for (int n = 0; n < 5000; ++n)
    {
        arrayOfOnes[n] = 1;
    }
}


LfpTriggeredAverageNode::~LfpTriggeredAverageNode()
{
}


AudioProcessorEditor* LfpTriggeredAverageNode::createEditor()
{
    editor = new LfpTriggeredAverageEditor (this, true);
    return editor;
}


void LfpTriggeredAverageNode::updateSettings()
{
    std::cout << "Setting num inputs on LfpTriggeredAverageNode to " << getNumInputs() << std::endl;
}


bool LfpTriggeredAverageNode::resizeBuffer()
{
    const int nSamples = (int) getSampleRate() * bufferLength;
    const int nInputs = getNumInputs();

    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

    if (nSamples > 0 && nInputs > 0)
    {
        abstractFifo.setTotalSize (nSamples);
        displayBuffer->setSize (nInputs + 1, nSamples); // add an extra channel for TTLs

        return true;
    }
    else
    {
        return false;
    }
}


bool LfpTriggeredAverageNode::enable()
{
    if (resizeBuffer())
    {
        LfpTriggeredAverageEditor* editor = (LfpTriggeredAverageEditor*) getEditor();
        editor->enable();

        return true;
    }
    else
    {
        return false;
    }
}


bool LfpTriggeredAverageNode::disable()
{
    LfpTriggeredAverageEditor* editor = (LfpTriggeredAverageEditor*) getEditor();
    editor->disable();

    return true;
}


void LfpTriggeredAverageNode::setParameter (int parameterIndex, float newValue)
{
    editor->updateParameterButtons (parameterIndex);

    //Sets Parameter in parameters array for processor
    Parameter* parameterPointer = parameters.getRawDataPointer();
    parameterPointer += parameterIndex;
    parameterPointer->setValue (newValue, currentChannel);

    LfpTriggeredAverageEditor* ed = (LfpTriggeredAverageEditor*) getEditor();
    if (ed->canvas != 0)
        ed->canvas->setParameter (parameterIndex, newValue);
}


void LfpTriggeredAverageNode::handleEvent (int eventType, MidiMessage& event)
{
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        const int eventId         = *(dataptr + 2);
        const int eventChannel    = *(dataptr + 3);
        const int eventTime       = event.getTimeStamp();
        const int samplesLeft     = totalSamples - eventTime;
        const int bufferIndex     = (displayBufferIndex + eventTime);// % displayBuffer->getNumSamples();

        if (eventId == 1)
        {
            ttlState |= (1L << eventChannel);
        }
        else
        {
            ttlState &= ~(1L << eventChannel);
        }

        if (samplesLeft + bufferIndex < displayBuffer->getNumSamples())
        {
            displayBuffer->copyFrom (displayBuffer->getNumChannels() - 1,  // destChannel
                                     bufferIndex,                          // destStartSample
                                     arrayOfOnes,                          // source
                                     samplesLeft,                          // numSamples
                                     float (ttlState));                    // gain
        }
        else
        {
            const int block2Size = (samplesLeft + bufferIndex) % displayBuffer->getNumSamples();
            const int block1Size = samplesLeft - block2Size;

            displayBuffer->copyFrom (displayBuffer->getNumChannels() - 1,  // destChannel
                                     bufferIndex,                          // destStartSample
                                     arrayOfOnes,                          // source
                                     block1Size,                           // numSamples
                                     float (ttlState));                    // gain

            displayBuffer->copyFrom (displayBuffer->getNumChannels() - 1,  // destChannel
                                     0,                                    // destStartSample
                                     arrayOfOnes,                          // source
                                     block2Size,                           // numSamples
                                     float (ttlState));                    // gain
        }
    }
    else if (eventType == TIMESTAMP)
    {
        const uint8* dataptr = event.getRawData();

        // update the timestamp for the current buffer:
        memcpy (&bufferTimestamp, dataptr + 4, 4);
    }
}


void LfpTriggeredAverageNode::initializeEventChannel()
{
    if (displayBufferIndex + totalSamples < displayBuffer->getNumSamples())
    {
        displayBuffer->copyFrom (displayBuffer->getNumChannels() - 1,   // destChannel
                                 displayBufferIndex,                    // destStartSample
                                 arrayOfOnes,                           // source
                                 totalSamples,                          // numSamples
                                 float (ttlState));                     // gain
    }
    else
    {
        const int block2Size = (displayBufferIndex + totalSamples) % displayBuffer->getNumSamples();
        const int block1Size = totalSamples - block2Size;

        displayBuffer->copyFrom(displayBuffer->getNumChannels() - 1,    // destChannel
                                displayBufferIndex,                     // destStartSample
                                arrayOfOnes,                            // source
                                block1Size,                             // numSamples
                                float (ttlState));                      // gain

        displayBuffer->copyFrom (displayBuffer->getNumChannels() - 1,   // destChannel
                                 0,                                     // destStartSample
                                 arrayOfOnes,                           // source
                                 block2Size,                            // numSamples
                                 float (ttlState));                     // gain
    }
}


void LfpTriggeredAverageNode::process (AudioSampleBuffer& buffer, MidiBuffer& events)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    const int nSamples = 100;

    totalSamples = nSamples; //nSamples;
    displayBufferIndexEvents = displayBufferIndex;

    initializeEventChannel();

    checkForEvents (events); // update timestamp, see if we got any TTL events

    const int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex;

    if (nSamples < samplesLeft)
    {
        for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
        {
            displayBuffer->copyFrom (chan,                  // destChannel
                                     displayBufferIndex,    // destStartSample
                                     buffer,                // source
                                     chan,                  // source channel
                                     0,                     // source start sample
                                     nSamples);             // numSamples
        }

        displayBufferIndex += (nSamples);
    }
    else
    {
        const int extraSamples = nSamples - samplesLeft;

        for (int chan = 0; chan < buffer.getNumChannels(); ++chan)
        {
            displayBuffer->copyFrom (chan,                  // destChannel
                                     displayBufferIndex,    // destStartSample
                                     buffer,                // source
                                     chan,                  // source channel
                                     0,                     // source start sample
                                     samplesLeft);          // numSamples

            displayBuffer->copyFrom (chan,                  // destChannel
                                     0,                     // destStartSample
                                     buffer,                // source
                                     chan,                  // source channel
                                     samplesLeft,           // source start sample
                                     extraSamples);         // numSamples
        }

        displayBufferIndex = extraSamples;
    }
}

