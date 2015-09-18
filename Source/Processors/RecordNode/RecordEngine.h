/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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
#include "../Channel/Channel.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Visualization/SpikeObject.h"

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

struct SpikeRecordInfo
{
    String name;
    int numChannels;
    int sampleRate;

    int recordIndex;
};

struct EngineParameter;
class RecordNode;
class RecordEngineManager;

class RecordEngine
{
public:
    RecordEngine();
    virtual ~RecordEngine();
    virtual String getEngineID() =0;

    /** All the public methods (except registerManager) are called by RecordNode:
    When acquisition starts (in the specified order):
    	1-resetChannels
    	2-registerProcessor, addChannel, registerSpikeSource, addspikeelectrode
    	3-configureEngine (which calls setParameter)
    	3-startAcquisition
    During acquisition:
    	updateTimeStamps
    When recording starts (in the specified order):
    	1-directoryChanged (if needed)
    	2-openFiles
    During recording:
    	writeData, writeEvent, writeSpike
    When recording stops:
    	closeFiles
    */

    /** Called for registering parameters
    */
    virtual void setParameter(EngineParameter& parameter);

    /** Called when recording starts to open all needed files
    */
    virtual void openFiles(File rootFolder, int experimentNumber, int recordingNumber) = 0;

    /** Called when recording stops to close all files
    	and do all the necessary cleanups
    */
    virtual void closeFiles() = 0;

    /** Write continuous data.
    	This method gets the full data buffer, it must query getRecordState for
    	each registered channel to determine which channels to actually write to disk.
        The number of samples to write will be found in the numSamples object.
    */
    virtual void writeData(AudioSampleBuffer& buffer) = 0;

    /** Write a single event to disk.
    */
    virtual void writeEvent(int eventType, MidiMessage& event, int samplePosition) = 0;

    /** Called when acquisition starts once for each processor that might record continuous data
    */
    virtual void registerProcessor(GenericProcessor* processor);

    /** Called after registerProcessor, once for each output
    	channel of the processor
    */
    virtual void addChannel(int index, Channel* chan) = 0;

    /** Called when acquisition starts once for each processor that might record spikes
    */
    virtual void registerSpikeSource(GenericProcessor* processor);

    /** Called after registerSpikesource, once for each channel group
    */
    virtual void addSpikeElectrode(int index, SpikeRecordInfo* elec) = 0;

    /** Write a spike to disk
    */
    virtual void writeSpike(const SpikeObject& spike, int electrodeIndex) = 0;

    /** Called when a new acquisition starts, to clean all channel data
    	before registering the processors
    */
    virtual void resetChannels();

    /** Called every time a new timestamp event is received
    */
    void updateTimestamps(std::map<uint8, int64>* timestamps);

    /** Called every time a new numSamples event is received */
    void updateNumSamples(std::map<uint8, int>* numSamples);

    /** Called after all channels and spike groups have been registered,
    	just before acquisition starts
    */
    virtual void startAcquisition();

    /** Called when the recording directory changes during an acquisition
    */
    virtual void directoryChanged();


    void registerManager(RecordEngineManager* engineManager);
    void configureEngine();

protected:
    /** Functions to access RecordNode arrays and utilities
    */

    /** Gets the specified channel from the channel array stored in RecordNode
    */
    Channel* getChannel(int index);

    /** Gets the specified channel group info structure from the array stored in RecordNode
    */
    SpikeRecordInfo* getSpikeElectrode(int index);

    /** Generate a Matlab-compatible datestring
    */
    String generateDateString();

    std::map<uint8, int>* numSamples;
    std::map<uint8, int64>* timestamps;

private:
    RecordEngineManager* manager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordEngine);
};

typedef RecordEngine* (*EngineCreator)();

struct EngineParameter
{
public:
    enum EngineParameterType {STR, INT, FLOAT, BOOL};
    EngineParameter(EngineParameterType paramType, int paramId, String paramName, var defaultValue, var min = 0, var max = 100);
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
class RecordEngineManager
{
public:
    RecordEngineManager(String engineID, String engineName, EngineCreator creatorFunc);
    ~RecordEngineManager();
    void addParameter(EngineParameter* param);

    RecordEngine* instantiateEngine();
    void toggleConfigWindow();
    bool isWindowOpen();

    void saveParametersToXml(XmlElement* xml);
    void loadParametersFromXml(XmlElement* xml);

    EngineParameter& getParameter(int index);
    int getNumParameters();

    String getID();
    String getName();

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

#endif  // RECORDENGINE_H_INCLUDED
