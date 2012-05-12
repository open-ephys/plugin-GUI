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

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "DataBuffer.h"

/**

  Abstract base class for a data input thread owned by the SourceNode.

  @see SourceNode

*/


class SourceNode;


class DataThread : public Thread
{

public:

	DataThread(SourceNode* sn);
	~DataThread();

	void run();

	DataBuffer* getBufferAddress();

	virtual bool updateBuffer() = 0;

	ScopedPointer<DataBuffer> dataBuffer;

	virtual bool foundInputSource() = 0;
	virtual bool startAcquisition() = 0;
	virtual bool stopAcquisition() = 0;
	virtual int getNumChannels() = 0;
	virtual float getSampleRate() = 0;
    virtual float getBitVolts() = 0;

	SourceNode* sn;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataThread);


};


#endif  // __DATATHREAD_H_C454F4DB__
