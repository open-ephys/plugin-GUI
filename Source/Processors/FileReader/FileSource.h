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

#ifndef FILESOURCE_H_INCLUDED
#define FILESOURCE_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"

class FileSource
{
public:
	FileSource();
	~FileSource();

	int getNumRecords();
	String getRecordName(int index);

	int getActiveRecord();
	void setActiveRecord(int index);

	float getRecordSampleRate(int index);
	int getRecordNumChannels(int index);

	float getActiveSampleRate();
	int getActiveNumChannels();

	bool OpenFile(File file);
	bool fileIsOpened();
	
	/** Reads nSamples from the file, returns true if the end was reached**/
	virtual bool readData(int16** startAddr, int& nSamples) =0;

	virtual void seekTo(int64 sample) =0;

protected:
	struct RecordInfo
	{
		String name;
		int numChannels;
		float sampleRate;
	};
	Array<RecordInfo> infoArray;

	bool fileOpened;
	int numRecords;
	int activeRecord;

private:
	virtual bool Open(File file)=0;
	virtual void fillRecordInfo()=0;
	virtual void updateActiveRecord()=0;

	

};



#endif  // FILESOURCE_H_INCLUDED
