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

#ifndef __FILEREADERTHREAD_H_82594504__
#define __FILEREADERTHREAD_H_82594504__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../../JuceLibraryCode/JuceHeader.h"

#include <stdio.h>
#include <time.h>
#include "DataThread.h"

/**

  --UNDER CONSTRUCTION--

  Similar to FileReader, but works within its own thread.

  @see DataThread, FileReader

*/


class SourceNode;

class FileReaderThread : public DataThread

{
public:
	FileReaderThread(SourceNode* sn);
	~FileReaderThread();

	bool foundInputSource();// {return true;}
	bool startAcquisition();
	bool stopAcquisition();
	int getNumChannels();// {return 16;}
	float getSampleRate();// {return 40000.0;}
    float getBitVolts();
	
private:

	int samplesPerBlock;

    int lengthOfInputFile;

    int playHead;

    FILE* input;

	//FileInputStream* input;

	float thisSample[16];
    int16 readBuffer[1600];

    int bufferSize;

	bool updateBuffer();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileReaderThread);
};



#endif  // __FILEREADERTHREAD_H_82594504__
