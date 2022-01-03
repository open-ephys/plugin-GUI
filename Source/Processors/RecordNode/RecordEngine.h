/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2016 Open Ephys

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

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../../Utils/Utils.h"

#include <map>

//Handy macros for setParameter
#define boolParameter(i,v) if ((parameter.id == i) && (parameter.type == EngineParameter::BOOL)) \
        v = parameter.boolParam.value
#define intParameter(i,v) if ((parameter.id == i) && (parameter.type == EngineParameter::INT)) \
        v = parameter.intParam.value
#define floatParameter(i,v) if ((parameter.id == i) && (parameter.type == EngineParameter::FLOAT)) \
        v = parameter.floatParam.value
#define strParameter(i,v) if ((parameter.id == i) && (parameter.type == EngineParameter::STR)) \
        v = parameter.strParam.value
#define multiParameter(i,v) if ((parameter.id == i) && (parameter.type == EngineParameter::MULTI)) \
        v = parameter.multiParam.value

struct RecordProcessorInfo
{
	int processorId;
	String processorName;
	Array<int> recordedChannels; //Indexes of the recorded channels. From 0-maxRecordChannels, not 0-totalChannels
};

struct EngineParameter;

class RecordNode;
class RecordEngineManager;


class PLUGIN_API RecordEngine
{
public:

	/** Constructor */
	RecordEngine();

	/** Destructor */
	virtual ~RecordEngine() { }

	/** Returns the unique identifier of the Record Engine */
	virtual String getEngineId() const = 0;

	/** Sets the pointer to the RecordEngineManager (called by ControlPanel) */
	void registerManager(RecordEngineManager* engineManager);


	/** ----- CALLED AT START OF ACQUISITION ---- */

	/** Sets RecordEngine parameters (if available) at the start of acquisition*/
	void configureEngine();

	/** Called by configureEngine() */
	virtual void setParameter(EngineParameter& parameter) { }

	/** Called just before acquisition starts (does nothing by default) */
	virtual void startAcquisition();


	/** ----- CALLED AT START OF RECORDING ---- */
	
	/** Sets the pointer to Record Node for this engine*/
	void registerRecordNode(RecordNode* node);

	/** Called when a new recording starts, to clean all channel data before registering the processors */
	virtual void resetChannels() { }

	/** Called prior to opening files, to set the map between recorded channels and actual channel numbers */
	void setChannelMapping(const Array<int>& channels, 
						   const Array<int>& chanProcessor, 
						   const Array<int>& chanOrder, 
						   OwnedArray<RecordProcessorInfo>& processors);

	/** Called if the directory has changed since the last recording */
	virtual void directoryChanged();

	/** Called when recording starts to open all needed files */
	virtual void openFiles(File rootFolder, int experimentNumber, int recordingNumber) = 0;


	/** ----- CALLED DURING RECORDING ---- */

	/** Called at the start of every write block */
	void updateTimestamps(const Array<int64>& timestamps, int channel = -1);

	/** Called by the record thread before it starts writing the channels to disk */
	virtual void startChannelBlock(bool lastBlock);

	/** Write continuous data for a channel. The raw buffer pointer is passed for speed, care must be taken to only read the specified number of bytes. */
	virtual void writeData(int writeChannel, int realChannel, const float* buffer, int size) = 0;

	/** Write continuous data for a channel with synchronized float timestamps */
	virtual void writeSynchronizedData(int writeChannel, int realChannel, const float* dataBuffer, const double* ftsBuffer, int size) = 0;

	/** Called by the record thread after it has written a channel block */
	virtual void endChannelBlock(bool lastBlock);

	/** Write a single event to disk.  */
	virtual void writeEvent(int eventChannel, const MidiMessage& event) = 0;

	/** Handle the timestamp sync text messages*/
	virtual void writeTimestampSyncText(uint64 streamId, int64 timestamp, float sourceSampleRate, String text) = 0;

	/** Write a spike to disk */
	virtual void writeSpike(int electrodeIndex, const Spike* spike) = 0;


	/** ----- CALLED WHEN RECORDING STOPS ---- */

	/** Called when recording stops to close all files and do all the necessary cleanups */
	virtual void closeFiles() = 0;

	
	

protected:

	/** Gets the number of processors being recorded */
	int getNumRecordedProcessors() const;

	/** Gets the processor info structure for a recorded processor */
	const RecordProcessorInfo& getProcessorInfo(int processor) const;

	/** Functions to access RecordNode arrays and utilities */
	RecordNode* recordNode;

	/** Gets the specified channel from the channel array stored in RecordNode */
	const ContinuousChannel* getContinuousChannel(int index) const;

	/** Gets the specified event channel from the channel array stored in RecordNode */
	const EventChannel* getEventChannel(int index) const;

	const EventChannel* getEventChannelFromStreamId(int streamId) const;

	/** Gets the specified channel group info structure from the array stored in RecordNode */
	const SpikeChannel* getSpikeChannel(int index) const;

	/** Generate a Matlab-compatible datestring */
	String generateDateString() const;

	/** Gets the current block's first timestamp for a given recorded channel */
	int64 getTimestamp(int channel) const;

	/** Gets the actual channel number from a recorded channel index */
	int getRealChannel(int channel) const;

	/** Gets the number of recorded channels */
	int getNumRecordedContinuousChannels() const;

	/** Gets the number of recorded event channels
	(right now all channels are recorded) */
	int getNumRecordedEventChannels() const;

	/** Gets the number of recorded spike channels
	(right now all channels are recorded) */
	int getNumRecordedSpikeChannels() const;

	/** Gets the recorded processor index for a recorded channel index
	*/
	int getProcessorFromChannel(int channel) const;

	/** Gets the recorded channel index inside a specific processor for a written channel
	*/
	int getChannelNumInProc(int channel) const;

	/** Gets the last created settings.xml in text form. Should be called at file opening to get the latest version.
	Since the string will be large, returns a const reference. It should never be const_casted.
	*/
	const String& getLatestSettingsXml() const;

private:

	Array<int64> timestamps;
	Array<int> channelMap;
	Array<int> chanProcessorMap;
	Array<int> chanOrderMap;

	RecordEngineManager* manager;
	OwnedArray<RecordProcessorInfo> recordProcessors;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordEngine);
};

typedef RecordEngine* (*EngineCreator)();

struct PLUGIN_API EngineParameter
{
public:
	enum EngineParameterType { STR, INT, FLOAT, BOOL, MULTI };

	EngineParameter(EngineParameterType paramType,
		int paramId,
		String paramName,
		var defaultValue,
		var min = 0,
		var max = 100);

	void restoreDefault();

	union
	{
		struct
		{
			int min;
			int max;
			int value;
		} intParam;

		struct
		{
			float min;
			float max;
			float value;
		} floatParam;

		struct
		{
			bool value;
		} boolParam;

		struct
		{
			int value;
		} multiParam;
	};

	//Strings can't be inside an union. This means wasting a bit of memory, but adds more safety than using char*
	struct
	{
		String value;
	} strParam;

	const EngineParameterType type;
	const String name;
	const int id;


private:
	var def;
};


class EngineConfigWindow;
class PLUGIN_API RecordEngineManager
{
public:
	RecordEngineManager(String engineID, String engineName, EngineCreator creatorFunc);
	~RecordEngineManager();

	void addParameter(EngineParameter* param);

	RecordEngine* instantiateEngine();
	void toggleConfigWindow();
	bool isWindowOpen() const;

	void saveParametersToXml(XmlElement* xml);
	void loadParametersFromXml(XmlElement* xml);

	EngineParameter& getParameter(int index);
	int getNumParameters() const;

	String getID()   const;
	String getName() const;

	static int getNumOfBuiltInEngines();
	static RecordEngineManager* createBuiltInEngineManager(int index);


private:
	EngineCreator creator;

	String id;
	String name;

	OwnedArray<EngineParameter> parameters;
	ScopedPointer<EngineConfigWindow> window;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordEngineManager);
};

template<class T>
RecordEngine* engineFactory()
{
	return new T;
}

#endif  // RECORDENGINE_H_INCLUDED
