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
struct SpikeObject;
class GenericProcessor;
struct SpikeRecordInfo;

namespace CoreServices
{
/** Issues a signal chain update, useful for propagating new channel settings */
void updateSignalChain(GenericEditor* source);

/** Returns true is the GUI is recording */
bool getRecordingStatus();

/** Activated or deactivates recording */
void setRecordingStatus(bool enable);

/** Returns true if the GUI is acquiring data */
bool getAcquisitionStatus();

/** Activates or deactivates data acquisition */
void setAcquisitionStatus(bool enable);

/** Sends a string to the message bar */
void sendStatusMessage(const String& text);

/** Sends a string to the message bar */
void sendStatusMessage(const char* text);

/** Highlights an editor */
void highlightEditor(GenericEditor* ed);

/** Gets the timestamp selected on the MessageCenter interface
Defaults to the first hardware timestamp source or the software one if
no hardware timestamping is present*/
int64 getGlobalTimestamp();

/** Gets the software timestamp based on a high resolution timer aligned to the start of each processing block */
int64 getSoftwareTimestamp();

/** Set new recording directory */
void setRecordingDirectory(String dir);

/** Create new recording directory */
void createNewRecordingDir();

/** Manually set the text to be prepended to the recording directory */
void setPrependTextToRecordingDir(String text);

/** Manually set the text to be appended to the recording directory */
void setAppendTextToRecordingDir(String text);

namespace RecordNode
{
/** Forces creation of new directory on recording */
void createNewrecordingDir();

/** Gets the current recording directory */
File getRecordingPath();

/* Spike related methods. See record engine documentation */

void writeSpike(SpikeObject& spike, int electrodeIndex);
void registerSpikeSource(GenericProcessor* processor);
int addSpikeElectrode(SpikeRecordInfo* elec);
};

};




#endif  // CORESERVICES_H_INCLUDED
