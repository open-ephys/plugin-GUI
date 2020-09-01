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
#include "Processors/PluginManager/OpenEphysPlugin.h"

class GenericEditor;
class GenericProcessor;
class SpikeChannel;
class SpikeEvent;

namespace CoreServices
{
/** Issues a signal chain update, useful for propagating new channel settings */
PLUGIN_API void updateSignalChain(GenericEditor* source);

/** Returns true is the GUI is recording */
PLUGIN_API bool getRecordingStatus();

/** Activated or deactivates recording */
PLUGIN_API void setRecordingStatus(bool enable);

/** Returns true if the GUI is acquiring data */
PLUGIN_API bool getAcquisitionStatus();

/** Activates or deactivates data acquisition */
PLUGIN_API void setAcquisitionStatus(bool enable);

/** Sends a string to the message bar */
PLUGIN_API void sendStatusMessage(const String& text);

/** Sends a string to the message bar */
PLUGIN_API void sendStatusMessage(const char* text);

/** Highlights an editor */
PLUGIN_API void highlightEditor(GenericEditor* ed);

/** Gets the timestamp selected on the MessageCenter interface
Defaults to the first hardware timestamp source or the software one if
no hardware timestamping is present*/
PLUGIN_API juce::int64 getGlobalTimestamp();

/** Gets the sample rate selected on the MessageCenter interface
Defaults to the dsmple rate of the first hardware source or 
the software high resolution timer if no hardware source is present*/
PLUGIN_API float getGlobalSampleRate();

/** Gets the full id of the node generating global timestamps.
Returns 0 if timestamps are provided by the software high resolution timer */
PLUGIN_API uint32 getGlobalTimestampSourceFullId();

/** Gets the software timestamp based on a high resolution timer aligned to the start of each processing block */
PLUGIN_API juce::int64 getSoftwareTimestamp();

/** Gets the ticker frequency of the software timestamp clock*/
PLUGIN_API float getSoftwareSampleRate();

/** Set new recording directory */
PLUGIN_API void setRecordingDirectory(String dir);

PLUGIN_API File getRecordingDirectory();

/** Create new recording directory */
PLUGIN_API void createNewRecordingDir();

/** Manually set the text to be prepended to the recording directory */
PLUGIN_API void setPrependTextToRecordingDir(String text);

/** Manually set the text to be appended to the recording directory */
PLUGIN_API void setAppendTextToRecordingDir(String text);

//** Get array of available record engines
PLUGIN_API std::vector<RecordEngineManager*> getAvailableRecordEngines();

/** Gets the ID fo the selected Record Engine*/
PLUGIN_API String getSelectedRecordEngineId();

/** Sets a specific RecordEngine to be used based on its id. 
Return true if there is an engine with the specified ID and it's possible to
change the current engine or false otherwise. */
PLUGIN_API bool setSelectedRecordEngineId(String id);

PLUGIN_API int getSelectedRecordEngineIdx();

namespace RecordNode
{
/** Forces creation of new directory on recording */
PLUGIN_API void createNewrecordingDir();

/** Gets the current recording directories and status information */
PLUGIN_API File getRecordingPath();
PLUGIN_API int getRecordingNumber();
PLUGIN_API int getExperimentNumber();
PLUGIN_API bool getRecordThreadStatus();

/* Spike related methods. See record engine documentation */

PLUGIN_API void writeSpike(const SpikeEvent* spike, const SpikeChannel* chan);
PLUGIN_API void registerSpikeSource(GenericProcessor* processor);
PLUGIN_API int addSpikeElectrode(const SpikeChannel* elec);

};

PLUGIN_API const char* getApplicationResource(const char* name, int& size);
    
/** Gets the default directory for user-initiated file saving/loading */
PLUGIN_API File getDefaultUserSaveDirectory();

/** Gets the save directory for GUI-related file saving/loading */
PLUGIN_API File getSavedStateDirectory();

/** Gets the GUI version */
PLUGIN_API String getGUIVersion();

};




#endif  // CORESERVICES_H_INCLUDED
