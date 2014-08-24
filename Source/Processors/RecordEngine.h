/*
 ------------------------------------------------------------------
 
 This file is part of the Open Ephys GUI
 Copyright (C) 2013 Florian Franzen
 
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

#ifndef RECORDENGINE_H_INCLUDED
#define RECORDENGINE_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Channel.h"
#include "GenericProcessor.h"

class RecordNode;

class RecordEngine
{
public:
	RecordEngine(RecordNode* rn);
	~RecordEngine();

	virtual void openFiles(File rootfolder) =0;
	virtual void closeFiles() =0;
	virtual void writeData(AudioSampleBuffer& buffer, int nSamples, int64 timestamp) =0;
	virtual void writeEvent(MidiMessage& event, int samplePosition) =0;
	virtual void addChannel(int index, Channel* chan) =0;
	virtual void registerProcessor(GenericProcessor* processor);
	virtual void clearConnections();

protected:
	Channel* getChannel(int index);

private:
	RecordNode* recordNode;
	

};



#endif  // RECORDENGINE_H_INCLUDED
