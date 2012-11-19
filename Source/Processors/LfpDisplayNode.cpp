/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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
#include "Visualization/LfpDisplayCanvas.h"
#include <stdio.h>

LfpDisplayNode::LfpDisplayNode()
	: GenericProcessor("LFP Viewer"),
	  bufferLength(5.0f), displayBufferIndex(0), displayGain(1),
	  abstractFifo(100), ttlState(0)

{
	displayBuffer = new AudioSampleBuffer(8, 100);
	eventBuffer = new MidiBuffer();

	Array<var> timeBaseValues;
	timeBaseValues.add(1);
	timeBaseValues.add(2);
	timeBaseValues.add(5);
	timeBaseValues.add(10);

	parameters.add(Parameter("timebase",timeBaseValues, 0, 0));//true);//a,0);

	Array<var> displayGainValues;
	displayGainValues.add(1);
	displayGainValues.add(2);
	displayGainValues.add(4);
	displayGainValues.add(8);

	parameters.add(Parameter("display gain",displayGainValues, 0, 1));//true);//a,0);

	arrayOfOnes = new float[5000];

	for (int n = 0; n < 5000; n++)
	{
		arrayOfOnes[n] = 1;
	}

}

LfpDisplayNode::~LfpDisplayNode()
{
	//deleteAndZero(displayBuffer);
	//deleteAndZero(eventBuffer);
}

AudioProcessorEditor* LfpDisplayNode::createEditor()
{

	editor = new LfpDisplayEditor(this);	
	return editor;

}

void LfpDisplayNode::updateSettings()
{
	std::cout << "Setting num inputs on LfpDisplayNode to " << getNumInputs() << std::endl;
}

bool LfpDisplayNode::resizeBuffer()
{
	int nSamples = (int) getSampleRate()*bufferLength;
	int nInputs = getNumInputs();

	std::cout << "Resizing buffer. Samples: " << nSamples << ", Inputs: " << nInputs << std::endl;

	if (nSamples > 0 && nInputs > 0)
	{
		abstractFifo.setTotalSize(nSamples);
		displayBuffer->setSize(nInputs+1, nSamples); // add an extra channel for TTLs
		return true;
	} else {
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
	} else {
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
	LfpDisplayEditor* ed = (LfpDisplayEditor*) getEditor();
	if (ed->canvas != 0)
		ed->canvas->setParameter(parameterIndex, newValue);
}

void LfpDisplayNode::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
	if (eventType == TTL)
	{
		uint8* dataptr = event.getRawData();

    	int eventNodeId = *(dataptr+1);
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
    	} else {
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
    	} else {

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


	} else if (eventType == TIMESTAMP)
	{

		uint8* dataptr = event.getRawData();

    	int eventNodeId = *(dataptr+1);
    	int eventId = *(dataptr+2);
    	int eventChannel = *(dataptr+3);
    	
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

void LfpDisplayNode::initializeEventChannel()
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
    	} else {

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

void LfpDisplayNode::process(AudioSampleBuffer &buffer, MidiBuffer &events, int& nSamples)
{
	// 1. place any new samples into the displayBuffer
	//std::cout << "Display node sample count: " << nSamples << std::endl; ///buffer.getNumSamples() << std::endl;

	totalSamples = nSamples;
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

	} else {

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

	//std::cout << *displayBuffer->getSampleData(displayBuffer->getNumChannels()-1, 
	//										  displayBufferIndex-1) << std::endl;


	///// failed attempt to use abstractFifo:

	// int start1, size1, start2, size2;

	// abstractFifo.prepareToWrite(nSamples, start1, size1, start2, size2);

	// if (size1 > 0)
	// {
	// 	for (int chan = 0; chan < buffer.getNumChannels(); chan++)
	// 	{	
	// 		displayBuffer->copyFrom(chan,  			// destChannel
	// 						    start1, 			// destStartSample
	// 						    buffer, 			// source
	// 						    chan, 				// source channel
	// 						    0,					// source start sample
	// 						    size1); 			// numSamples

	// 	}

	// 	displayBufferIndex += size1;
	// }

	// if (size2 > 0)
	// {
	// 	for (int chan = 0; chan < buffer.getNumChannels(); chan++)
	// 	{	
	// 		displayBuffer->copyFrom(chan,  			// destChannel
	// 						    start2, 			// destStartSample
	// 						    buffer, 			// source
	// 						    chan, 				// source channel
	// 						    size1,				// source start sample
	// 						    size2); 			// numSamples

	// 	}

	// 	displayBufferIndex = size2;
	// }

	// std::cout << displayBufferIndex << std::endl;

	// abstractFifo.finishedWrite(size1 + size2);


}

