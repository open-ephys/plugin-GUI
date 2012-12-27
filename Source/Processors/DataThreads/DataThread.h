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

#ifndef __DATATHREAD_H_C454F4DB__
#define __DATATHREAD_H_C454F4DB__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "DataBuffer.h"

class SourceNode;

/**

  Abstract base class for a data input thread owned by the SourceNode.

  To communicate with input sources that may have a different clock as the
  data acquisition callbacks, it's most efficient to use a separate thread.
  The DataThread class makes it easy to create threads that interact with
  new data sources, such as an FPGA, an Arduino, or a network stream.

  @see SourceNode

*/

class DataThread : public Thread
{

public:

	DataThread(SourceNode* sn);
	~DataThread();

    /** Calls 'updateBuffer()' continuously while the thread is being run.*/
	void run();

    /** Returns the address of the DataBuffer that the input source will fill.*/
	DataBuffer* getBufferAddress();

    /** Fills the DataBuffer with incoming data. This is the most important 
    method for each DataThread.*/
	virtual bool updateBuffer() = 0;
    
    /** Experimental method used for testing data sources that can deliver outputs.*/
    virtual void setOutputHigh() {}

    /** Experimental method used for testing data sources that can deliver outputs.*/
    virtual void setOutputLow() {}

	ScopedPointer<DataBuffer> dataBuffer;

    /** Returns true if the data source is connected, false otherwise.*/
	virtual bool foundInputSource() = 0;

    /** Initializes data transfer.*/
	virtual bool startAcquisition() = 0;

    /** Stops data transfer.*/
	virtual bool stopAcquisition() = 0;

    /** Returns the number of continuous channels the data source can provide.*/
	virtual int getNumChannels() = 0;

    /** Returns the sample rate of the data source.*/
	virtual float getSampleRate() = 0;

    /** Returns the volts per bit of the data source.*/
    virtual float getBitVolts() = 0;

    /** Returns the number of event channels of the data source.*/
    virtual int getNumEventChannels() {return 0;}

	SourceNode* sn;

    int16 eventCode;
    uint64 timestamp;

    Time timer;
    
    /** Returns a pointer to the data input device, in case other processors
    need to communicate with it.*/
    virtual void* getDevice() {return 0;}

private:



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataThread);


};


#endif  // __DATATHREAD_H_C454F4DB__
