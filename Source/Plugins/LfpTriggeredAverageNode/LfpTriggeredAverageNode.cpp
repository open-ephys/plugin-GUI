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

#include "LfpTriggeredAverageNode.h"
#include "LfpTriggeredAverageCanvas.h"
#include <stdio.h>

LfpTriggeredAverageNode::LfpTriggeredAverageNode()
    : GenericProcessor("LFP Trig. Avg."),
      displayBufferIndex(0), displayGain(1), bufferLength(5.0f),
      abstractFifo(100), ttlState(0)
{
    std::cout << " LfpTriggeredAverageNode Constructor" << std::endl;
    displayBuffer = new AudioSampleBuffer(8, 100);
    eventBuffer = new MidiBuffer();

    arrayOfOnes = new float[5000];

    for (int n = 0; n < 5000; n++)
    {
        arrayOfOnes[n] = 1;
    }

}

LfpTriggeredAverageNode::~LfpTriggeredAverageNode()
{

}

AudioProcessorEditor* LfpTriggeredAverageNode::createEditor()
{

    editor = new LfpTriggeredAverageEditor(this, true);
    return editor;

}

void LfpTriggeredAverageNode::updateSettings()
{
    std::cout << "Setting num inputs on LfpTriggeredAverageNode to " << getNumInputs() << std::endl;
}

bool LfpTriggeredAverageNode::resizeBuffer()
{
    int nSamples = (int) getSampleRate()*bufferLength;
    int nInputs = getNumInputs();

    std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

    if (nSamples > 0 && nInputs > 0)
    {
        abstractFifo.setTotalSize(nSamples);
        displayBuffer->setSize(nInputs+1, nSamples); // add an extra channel for TTLs
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

void LfpTriggeredAverageNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    //Sets Parameter in parameters array for processor
    Parameter* parameterPointer=parameters.getRawDataPointer();
    parameterPointer=parameterPointer+parameterIndex;
    parameterPointer->setValue(newValue, currentChannel);

    //std::cout << "Saving Parameter from " << currentChannel << ", channel ";

    LfpTriggeredAverageEditor* ed = (LfpTriggeredAverageEditor*) getEditor();
    if (ed->canvas != 0)
        ed->canvas->setParameter(parameterIndex, newValue);
}

void LfpTriggeredAverageNode::handleEvent(int eventType, MidiMessage& event)
{
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);
        int eventTime = event.getTimeStamp();

        int samplesLeft = totalSamples - eventTime;

        //	std::cout << "Received event from " << eventNodeId << ", channel "
        //	          << eventChannel << ", with ID " << eventId << std::endl;
        //
        int bufferIndex = (displayBufferIndex + eventTime);// % displayBuffer->getNumSamples();

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

            //	std::cout << bufferIndex << " " << samplesLeft << " " << ttlState << std::endl;

            displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                    bufferIndex,		// destStartSample
                                    arrayOfOnes, 		// source
                                    samplesLeft, 		// numSamples
                                    float(ttlState));   // gain
        }
        else
        {

            int block2Size = (samplesLeft + bufferIndex) % displayBuffer->getNumSamples();
            int block1Size = samplesLeft - block2Size;

            //std::cout << "OVERFLOW." << std::endl;

            //std::cout << bufferIndex << " " << block1Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                    bufferIndex,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block1Size, 		// numSamples
                                    float(ttlState));   // gain

            //std::cout << 0 << " " << block2Size << " " << ttlState << std::endl;

            displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                    0,		// destStartSample
                                    arrayOfOnes, 		// source
                                    block2Size, 		// numSamples
                                    float(ttlState));   // gain


        }


        // 	std::cout << "ttlState: " << ttlState << std::endl;

        // std::cout << "Received event from " << eventNodeId <<
        //              " on channel " << eventChannel <<
        //             " with value " << eventId <<
        //             " at timestamp " << event.getTimeStamp() << std::endl;


    }
    else if (eventType == TIMESTAMP)
    {

        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        // int eventId = *(dataptr+2);
        // int eventChannel = *(dataptr+3);

        // update the timestamp for the current buffer:
        memcpy(&bufferTimestamp, dataptr+4, 4);



        //   	double timeInSeconds = double(ts)/Time::getHighResolutionTicksPerSecond();
        //   	//int64 timestamp = ts[0] << 32 +
        //   	//				  ts[1] << 16 +
        //   	//				  ts[2] << 8 +
        //   	//				  ts[3];
        //   	//memcpy(ts, dataptr+4, 1);

        //   	std::cout << "Time in seconds is " << timeInSeconds << std::endl;

        // // std::cout << "Received event from " << eventNodeId <<
        //    	//              " on channel " << eventChannel <<
        //    	 //             " with value " << eventId <<
        //    	 //             " for time: " << ts << std::endl;
    }
}

void LfpTriggeredAverageNode::initializeEventChannel()
{
    if (displayBufferIndex + totalSamples < displayBuffer->getNumSamples())
    {

        //	std::cout << getNumInputs()+1 << " " << displayBufferIndex << " " << totalSamples << " " << ttlState << std::endl;
        //
        displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                displayBufferIndex,		// destStartSample
                                arrayOfOnes, 		// source
                                totalSamples, 		// numSamples
                                float(ttlState));   // gain
    }
    else
    {

        int block2Size = (displayBufferIndex + totalSamples) % displayBuffer->getNumSamples();
        int block1Size = totalSamples - block2Size;

        // std::cout << "OVERFLOW." << std::endl;

        // std::cout << bufferIndex << " " << block1Size << " " << ttlState << std::endl;

        displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                displayBufferIndex,		// destStartSample
                                arrayOfOnes, 		// source
                                block1Size, 		// numSamples
                                float(ttlState));   // gain
        // std::cout << 0 << " " << block2Size << " " << ttlState << std::endl;

        displayBuffer->copyFrom(displayBuffer->getNumChannels()-1,  // destChannel
                                0,		// destStartSample
                                arrayOfOnes, 		// source
                                block2Size, 		// numSamples
                                float(ttlState));   // gain


    }
}

void LfpTriggeredAverageNode::process(AudioSampleBuffer& buffer, MidiBuffer& events)
{
    // 1. place any new samples into the displayBuffer
    //std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

    int nSamples = 100;

    totalSamples = nSamples; //nSamples;
    displayBufferIndexEvents = displayBufferIndex;

    initializeEventChannel();

    checkForEvents(events); // update timestamp, see if we got any TTL events

    int samplesLeft = displayBuffer->getNumSamples() - displayBufferIndex;

    if (nSamples < samplesLeft)
    {

        for (int chan = 0; chan < buffer.getNumChannels(); chan++)
        {
            displayBuffer->copyFrom(chan,  			// destChannel
                                    displayBufferIndex, // destStartSample
                                    buffer, 			// source
                                    chan, 				// source channel
                                    0,					// source start sample
                                    nSamples); 			// numSamples

        }
        displayBufferIndex += (nSamples);

    }
    else
    {

        int extraSamples = nSamples - samplesLeft;

        for (int chan = 0; chan < buffer.getNumChannels(); chan++)
        {
            displayBuffer->copyFrom(chan,  				// destChannel
                                    displayBufferIndex, // destStartSample
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
        }

        displayBufferIndex = extraSamples;
    }



}

