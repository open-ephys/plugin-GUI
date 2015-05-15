/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef CORESERVICES_H_INCLUDED
#define CORESERVICES_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class GenericEditor;
class SpikeObject;
class GenericProcessor;
struct SpikeRecordInfo;

namespace CoreServices
{
void updateSignalChain(GenericEditor* source);
bool getRecordingStatus();
void setRecordingStatus(bool enable);
void sendStatusMessage(String& text);
void sendStatusMessage(const char* text);
void highlightEditor(GenericEditor* ed);
int64 getGlobalTimestamp();

namespace RecordNode
{
void createNewrecordingDir();
File getRecordingPath();
void writeSpike(SpikeObject& spike, int electrodeIndex);
void registerSpikeSource(GenericProcessor* processor);
int addSpikeElectrode(SpikeRecordInfo* elec);
};

};




#endif  // CORESERVICES_H_INCLUDED
