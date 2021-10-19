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

#ifndef __GENERICPROCESSOR_H_1F469DAF__
#define __GENERICPROCESSOR_H_1F469DAF__

#include <JuceHeader.h>

#include "GenericProcessorBase.h"

#include "../Parameter/Parameter.h"
#include "../../CoreServices.h"
#include "../PluginManager/PluginClass.h"
#include "../../Processors/Dsp/LinearSmoothedValueAtomic.h"
#include "../../Processors/PluginManager/PluginIDs.h"

#include "../Settings/ContinuousChannel.h"
#include "../Settings/EventChannel.h"
#include "../Settings/SpikeChannel.h"
#include "../Settings/DataStream.h"
#include "../Settings/InfoObject.h"

#include "../Events/Event.h"

#include <time.h>
#include <stdio.h>
#include <map>
#include <unordered_map>

class EditorViewport;
class DataViewport;
class UIComponent;
class GenericEditor;
class Parameter;
class GenericProcessorBase;
class GenericEditor;

class ConfigurationObject;
class ProcessorInfoObject;
class DeviceInfo;

class Spike;

class LatencyMeter;

using namespace Plugin;

namespace AccessClass
{
	class ExternalProcessorAccessor;
};

/**
    Abstract base class for creating processors.

    All processors must be derived from this class, and must provide an
    implementation of the process() method.

    @see ProcessorGraph, GenericEditor, SourceNode, FilterNode, LfpDisplayNode
*/
class PLUGIN_API GenericProcessor   : public GenericProcessorBase
                                    , public PluginClass
									//, public ChannelCreationIndices
{
	friend AccessClass::ExternalProcessorAccessor;
    friend class RecordEngine;
    friend class MessageCenter;
    friend class ProcessorGraph;
    friend class GenericEditor;
    friend class Splitter;
    friend class Merger;

public:
    /** Constructor (sets the processor's name). */
    GenericProcessor (const String& name_);

    /** Destructor. */
    virtual ~GenericProcessor();

    // --------------------------------------------
    //    PROCESS METHOD -- must be implemented
    // --------------------------------------------

    /** This method defines a processor's functionality.

        This is the most important function for each
        processor, as it determines how it creates, modifies, or responds to incoming data
        streams. Rather than use the default JUCE processBlock() method, processBlock()
        automatically calls process() in order to add the 'nSamples' variable to indicate
        the number of samples in the current buffer.
    */
    virtual void process (AudioBuffer<float>& continuousBuffer) = 0;

    /** Allows parameters to change while acquisition is active. If the user wants
    to change ANY variables that are used within the process() method, this must
    be done through setParameter(). */
    void setParameter(int parameterIndex, float newValue);

    // --------------------------------------------
    //    QUERYING INFO ABOUT THIS PROCESSOR
    // --------------------------------------------

    /** Pointer to a processor's immediate source node.*/
    GenericProcessor* sourceNode;

    /** Pointer to a processor's immediate destination.*/
    GenericProcessor* destNode;

    /** Returns a pointer to the processor immediately preceding a given processor in the signal chain. */
    GenericProcessor* getSourceNode() const;

    /** Returns a pointer to the processor immediately following a given processor in the signal chain. */
    GenericProcessor* getDestNode() const;

    /** Returns a pointer to the processor's editor. */
    GenericEditor* getEditor() const;

    /** Returns the sample rate for a given data stream.*/
    virtual float getSampleRate(int streamIndex) const;

    /** Returns the default sample rate, in case a processor has no source (or is itself a source).*/
    virtual float getDefaultSampleRate() const;

    /** Returns the total number of inputs to a processor.*/
    int getNumInputs() const;

    /** Returns the total number of outputs from a processor.*/
    int getNumOutputs() const;

    /** Returns the total number of data streams handled by this processor.*/
    int getNumDataStreams() const;

	/** Returns the number of outputs from a specific data stream.*/
	int getNumOutputsForStream(int streamIndex) const;

    /** Returns the unique integer ID for a processor. */
    int getNodeId() const;

    /** Sets the unique integer ID for a processor. */
    void setNodeId (int id);

    // --------------------------------------------
    //       CREATING THE EDITOR
    // --------------------------------------------

    /** Creates a GenericEditor.*/
    virtual AudioProcessorEditor* createEditor() override;

    // --------------------------------------------
    //         BUILDING THE SIGNAL CHAIN
    // --------------------------------------------

    /** Returns the next available continuous channel (and increments the channel if the input is set to 'true'). */
    virtual int getNextChannel(bool increment);

    /** Resets all inter-processor connections prior to the start of data acquisition.*/
    virtual void resetConnections();

    /** Sets the current channel (for purposes of updating parameters).*/
    virtual void setCurrentChannel(int chan);

    /** Sets the input or output of a splitter or merger.*/
    virtual void switchIO (int);

    /** Switches the input or output of a splitter or merger.*/
    virtual void switchIO();

    /** Sets the input to a merger a given processor.*/
    virtual void setPathToProcessor (GenericProcessor* p);

    /** Sets a processor's source node.*/
    virtual void setSourceNode (GenericProcessor* sn);

    /** Sets a processor's destination node.*/
    virtual void setDestNode (GenericProcessor* dn);

    /** Sets one of two possible source nodes for a merger.*/
    virtual void setMergerSourceNode (GenericProcessor* sn);

    /** Sets one of two possible source nodes for a splitter.*/
    virtual void setSplitterDestNode (GenericProcessor* dn);

    /** Returns trus if a processor generates its own timestamps, false otherwise.*/
    virtual bool generatesTimestamps() const;

    /** Returns true if a processor is a filter processor, false otherwise. */
    bool isFilter() const;

    /** Returns true if a processor is a source, false otherwise.*/
    bool isSource() const;

    /** Returns true if a processor is a sink, false otherwise.*/
    bool isSink() const;

    /** Returns true if a processor is a splitter, false otherwise.*/
    bool isSplitter() const;

    /** Returns true if a processor is a merger, false otherwise.*/
    bool isMerger() const;

     /** Returns true if a processor is a audio monitor, false otherwise.*/
    bool isAudioMonitor() const;

    /** Returns true if a processor is a utility (non-merger or splitter), false otherwise.*/
    bool isUtility() const;

    /** Returns true if a processor is a record node, false otherwise. */
    bool isRecordNode() const;

    /** Returns true if a processor is able to send its output to a given processor.

        Ideally, this should always return true, but there may be special cases
        when this is not possible.
    */
    virtual bool canSendSignalTo (GenericProcessor*) const;

    // --------------------------------------------
    //     ACQ / RECORD STATUS NOTIFICATIONS
    // --------------------------------------------

    /** Called immediately prior to the start of data acquisition, once all processors in the signal chain have
        indicated they are ready to process data.
         
         Returns: true if processor is ready to acquire, false otherwise
     */
    virtual bool startAcquisition();

    /** Called immediately after the end of data acquisition.*/
    virtual bool stopAcquisition();

    /** Called from whenever recording has started. */
    virtual void startRecording();

    /** Called from whenever recording has stopped. */
    virtual void stopRecording();

    /** Indicates whether or not a processor is currently enabled (i.e., able to process data). */
    //virtual bool isEnabled() const;

    /** Indicates whether a source node is connected to a processor (used for mergers).*/
    virtual bool stillHasSource() const { return true; }

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a boolean parameter, which will later be accessed by name*/
    void addBooleanParameter(const String& name,
        const String& description,
        bool defaultValue,
        bool deactivateDuringAcquisition = false,
        bool isGlobal = false);

    /** Adds an integer parameter, which will later be accessed by name*/
    void addIntParameter(const String& name,
        const String& description,
        int defaultValue,
        int minValue,
        int maxValue,
        bool deactivateDuringAcquisition = false,
        bool isGlobal = false);

    /** Adds an float parameter, which will later be accessed by name*/
    void addFloatParameter(const String& name,
        const String& description,
        float defaultValue,
        float minValue,
        float maxValue,
        float stepSize,
        bool deactivateDuringAcquisition = false,
        bool isGlobal = false);

    /** Adds a selected channels parameter, which will later be accessed by name*/
    void addSelectedChannelsParameter(const String& name,
        const String& description,
        int maxSelectedChannels = INT_MAX,
        bool deactivateDuringAcquisition = false, 
        bool isGlobal = false);

    /** Adds a categorical parameter, which will later be accessed by name*/
    void addCategoricalParameter(const String& name,
        const String& description,
        StringArray categories,
        int defaultIndex,
        bool deactivateDuringAcquisition = false,
        bool isGlobal = false);

    /** Returns a pointer to a Parameter object for a given name*/
    Parameter* getParameter(const uint16 streamId, const String& parameterName);

    /** Returns a pointer to a Parameter object for a given name.*/
    Parameter* getParameter(const String& parameterName);

    /** Returns the actual value for a parameter for a given name*/
    var getParameterValue(const uint16 streamId, const String& parameterName);

    /** Returns the index of the parameter for a given name.*/
    int getParameterIndex(const String& parameterName);

    /** Returns a global parameter*/
    Parameter* getGlobalParameter(const String& parameterName);

    /** Returns a global parameter value*/
    var getGlobalParameterValue(const String& parameterName);

    /** Initiates parameter value update */
    void parameterChangeRequest(Parameter*);

    /** Called when a parameter value is updated*/
    virtual void parameterValueChanged(Parameter*) { }

    // BUFFER ACCESS

    /** Returns a pointer to the processor's internal continuous buffer, if it exists. */
    virtual AudioBuffer<float>* getContinuousBuffer() const;

    /** Returns a pointer to the processor's internal event buffer, if it exists. */
    virtual MidiBuffer* getEventBuffer() const;

    /** Returns an array of this processor's event channels*/
    Array<const EventChannel*> getEventChannels();

    int nextAvailableChannel;

    /** Variable used to orchestrate saving the ProcessorGraph. */
    int saveOrder;

    /** Variable used to orchestrate loading the ProcessorGraph. */
    int loadOrder;

    /** The channel that will be updated the next time a parameter is changed. */
    int currentChannel;

    // --------------------------------------------
    //     UPDATING SETTINGS
    // --------------------------------------------

    /** Method for updating settings, called by ProcessorGraph.*/
    void update();

    // --------------------------------------------
    //     LOADING / SAVING SETTINGS
    // --------------------------------------------

    /** Save generic settings to XML (called by all processors).*/
    void saveToXml (XmlElement* parentElement);

    /** Load generic settings from XML (called by all processors). */
    void loadFromXml();

    /** Saving generic settings for each channel (called by all processors). */
    void saveChannelParametersToXml(XmlElement* parentElement, InfoObject* channelObject);

    /** Load generic parameters for each channel (called by all processors). */
    void loadChannelParametersFromXml(XmlElement* channelElement, InfoObject::Type type);

    // --------------------------------------------
   //     SAVING + LOADING SETTINGS
   // --------------------------------------------

   /** Saving custom settings to XML. */
    virtual void saveCustomParametersToXml(XmlElement* parentElement);

    /** Saving custom settings for each channel. */
    virtual void saveCustomChannelParametersToXml(XmlElement* channelElement, InfoObject* channel);

    /** Load custom settings from XML*/
    virtual void loadCustomParametersFromXml();

    /** Load custom parameters for each channel. */
    virtual void loadCustomChannelParametersFromXml(XmlElement* channelElement, InfoObject::Type type);

    /** Holds loaded parameters */
    XmlElement* parametersAsXml;

    // --------------------------------------------
    //     ACCESSING CHANNEL INFO OBJECTS
    // --------------------------------------------

    const ContinuousChannel* getContinuousChannel(int globalIndex) const;

    int getIndexOfMatchingChannel(const ContinuousChannel* channel) const;

    int getIndexOfMatchingChannel(const EventChannel* channel) const;

    int getIndexOfMatchingChannel(const SpikeChannel* channel) const;

    const EventChannel* getEventChannel(int globalIndex) const;

    const SpikeChannel* getSpikeChannel(int globalIndex) const;

	const ContinuousChannel* getContinuousChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const;

	const EventChannel* getEventChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const;

    const EventChannel* getMessageChannel() const;

	const SpikeChannel* getSpikeChannel(uint16 processorId, uint16 streamId, uint16 localIndex) const;

    DataStream* getDataStream(uint16 streamId) const;

    virtual Array<const DataStream*> getStreamsForDestNode(GenericProcessor* processor);

    Array<const DataStream*> getDataStreams() const;

	const ConfigurationObject* getConfigurationObject(int index) const;

	int getTotalContinuousChannels() const;
	
	int getTotalEventChannels() const;

	int getTotalSpikeChannels() const;

	int getTotalConfigurationObjects() const;

    PluginProcessorType getProcessorType() const;

    String getDisplayName() { return m_name; }

    void updateDisplayName(String name);

	//juce::int64 getLastProcessedsoftwareTime() const;

	//static uint32 getProcessorFullId(uint16 processorId, uint16 streamIdx);

	//static uint16 getNodeIdFromFullId(uint32 fullId);

	//static uint16 getStreamFromFullId(uint32 fullid);

	class PLUGIN_API DefaultEventInfo
	{
	public:
		DefaultEventInfo();
		DefaultEventInfo(EventChannel::Type type, unsigned int nChans, unsigned int length, float SampleRate);
		EventChannel::Type type{ EventChannel::INVALID };
		unsigned int nChannels{ 0 };
		unsigned int length{ 0 };
		float sampleRate{ 44100 };
		String name;
		String description;
		String identifier;
	};

    /** Determines whether the processor's editor appears colored or grayed out*/
    bool isEnabled;

protected:

    // --------------------------------------------
    //     SAMPLES + TIMESTAMPS
    // --------------------------------------------

    /** Used to get the number of samples in a given buffer, for a given channel. */
    uint32 getNumSamples(int channelNumber) const;

    /** Used to get the timestamp for a given buffer, for a given channel. */
    juce::uint64 getTimestamp(int channelNumber) const;

    /** Used to get the number of samples a specific source generates.
    Look by full source ID.
    @see GenericProcessor::getProcessorFullId(uint16,uint16) */
    uint32 getNumSourceSamples(uint16 streamId) const;

    /** Used to get the current timestamp of a specific source.
    Look by source ID and subprocessor index */
    juce::uint64 getSourceTimestamp(uint16 streamId) const;

	/** Used to set the timestamp for a given buffer, for a given DataStream. */
	void setTimestampAndSamples(juce::uint64 timestamp, uint32 nSamples, uint16 streamId);

    // --------------------------------------------
    //     HANDLING EVENTS AND MESSAGES
    // --------------------------------------------

    /** Handles a configuration message sent to this processor, while acquisition is not active.*/
    virtual String handleConfigMessage(String msg);

	/** Can be called by processors that need to respond to incoming events.
	Set respondToSpikes to true if the processor should also search for spikes*/
	virtual int checkForEvents(bool respondToSpikes = false);

	/** Makes it easier for processors to respond to incoming events, such as TTLs. Called if checkForEvents() returns true. */
	virtual void handleEvent(const EventChannel* eventInfo, const EventPacket& packet, int samplePosition = 0);

	/** Makes it easier for processors to respond to incoming spikes. Called if checkForEvents(true) returns true. */
	virtual void handleSpike(const SpikeChannel* spikeInfo, const EventPacket& packet, int samplePosition = 0);

	/** Responds to TIMESTAMP_SYNC_TEXT system events, in case a processor needs to listen to them (useful for the record node) */
	virtual void handleTimestampSyncTexts(const EventPacket& event);

	/** Returns the default number of datachannels outputs for a specific type and a specific subprocessor
	Called by createDataChannels(). It is not needed to implement if createDataChannels() is overriden */
	//virtual int getDefaultNumDataOutputs(ContinuousChannel::Type type, int subProcessorIdx = 0) const;

	/** Returns info about the default events a specific subprocessor generates.
	Called by createEventChannels(). It is not needed to implement if createEventChannels() is overriden */
	virtual void getDefaultEventInfo(Array<DefaultEventInfo>& events, int subProcessorIdx = 0) const;

    /** Sets whether processor will have behaviour like Source, Sink, Splitter, Utility or Merge */
    void setProcessorType (PluginProcessorType processorType);

    // --------------------------------------------
    //     ADDING SPIKES AND EVENTS
    // --------------------------------------------

    /** Create a simple TTL event channel with 8 bits - must be called during updateSettings() */
    void addTTLChannel(String name);

    /** Must be called during the process() method */
    void flipTTLBit(int sampleIndex, int bit);

    /** Must be called during the process() method */
    bool getTTLBit(int bit);

    /** Add an Event to the processing buffer */
	void addEvent(const Event* event, int sampleNum);

    /** Sends a TEXT event to all other processors, via the MessageCenter, while acquisition is active.
    If recording is active, this message will be recorded*/
    void broadcastMessage(String msg);

    /** Add a spike by SpikeChannel index */
	void addSpike(int channelIndex, const Spike* event, int sampleNum);

    /** Add a spike on the specified SpikeChanel */
	void addSpike(const SpikeChannel* channel, const Spike* event, int sampleNum);

    // --------------------------------------------
    //     UPDATING SETTINGS
    // --------------------------------------------

	/** Custom method for updating settings, called automatically by update() after creating the info objects.*/
	virtual void updateSettings();

    /** Holds information about continuous channels handled by this processor */
    OwnedArray<ContinuousChannel> continuousChannels;

    /** Holds information about event channels handled by this processor */
    OwnedArray<EventChannel> eventChannels;

    /** Holds information about spike channels handled by this processor */
    OwnedArray<SpikeChannel> spikeChannels;

    /** Holds additional configuration information*/
    OwnedArray<ConfigurationObject> configurationObjects;

    /** Holds information about data streams handled by this processor*/
    OwnedArray<DataStream> dataStreams;

    /** Copies DataStream settings from a source processor*/
    void copyDataStreamSettings(const DataStream*);

    /** Updates the data channel map objects*/
	void updateChannelIndexMaps();

    /** Holds info about this processor.*/
    std::unique_ptr<ProcessorInfoObject> processorInfo;

    /** Holds info about available devices.*/
    OwnedArray<DeviceInfo> devices;

    /** When set to false, this disables the sending of sample counts through the event buffer (used by Mergers and Splitters). */
    bool sendSampleCount;

    /** Pointer to the processor's editor. */
    std::unique_ptr<GenericEditor> editor;

    /** An array of default parameters for this processor.*/
    OwnedArray<Parameter> availableParameters;

    /** An array of global parameters for this processor.*/
    Array<Parameter*> globalParameters;

    /** An array of parameters for each stream that the user can modify.*/
    OwnedArray<Parameter> parameters;

    /** Used to quickly access parameters by name*/
    std::map<uint16, std::map<String, Parameter*>> parameterMap;

    /** Used to quickly access parameters by name*/
     std::map<String, Parameter*> globalParameterMap;


private:

    /** Clears the settings arrays.*/
    void clearSettings();

    /** Map between stream IDs and buffer sample counts. */
	std::map<uint16, uint32> numSamples;

    /** Map between stream IDs and buffer timestamps. */
	std::map<uint16, juce::int64> timestamps;

    /** Map between stream IDs and start time of process callbacks. */
    std::map<uint16, juce::int64> processStartTimes;

    /** First software timestamp of process() callback. */
	juce::int64 m_initialProcessTime;

    /** Built-in method for creating continuous channels. */
	void createDataChannelsByType(ContinuousChannel::Type type);

	/** Each processor has a unique integer ID that can be used to identify it.*/
	int nodeId;

    /** Automatically extracts the number of samples in the buffer, then
    calls the process(), where custom actions take place.*/
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages);

    /** Extracts sample counts and timestamps from the MidiBuffer. */
    int processEventBuffer();

    /** The type of the processor. */
    PluginProcessorType m_processorType;

    /** The name of the processor.*/
    String m_name;

    /** For getInputChannelName() and getOutputChannelName() */
    static const String m_unusedNameString;

    bool m_paramsWereLoaded;

	std::map<int,bool> m_needsToSendTimestampMessages;

	MidiBuffer* m_currentMidiBuffer;

    typedef std::unordered_map<uint16, 
        std::unordered_map<uint16, 
        std::unordered_map<uint16, 
        ContinuousChannel*>>> ContinuousChannelIndexMap;

    typedef std::unordered_map<uint16,
        std::unordered_map<uint16,
        std::unordered_map<uint16,
        EventChannel*>>> EventChannelIndexMap;

    typedef std::unordered_map<uint16,
        std::unordered_map<uint16,
        std::unordered_map<uint16,
        SpikeChannel*>>> SpikeChannelIndexMap;

    typedef std::unordered_map<uint16, DataStream*> DataStreamMap;

    ContinuousChannelIndexMap continuousChannelMap;
    EventChannelIndexMap eventChannelMap;
    SpikeChannelIndexMap spikeChannelMap;

    DataStreamMap dataStreamMap;

    Parameter* currentParameter;

    EventChannel* ttlEventChannel;
    Array<bool> ttlBitStates;

    bool wasConnected;

    std::unique_ptr<LatencyMeter> latencyMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);
};

class LatencyMeter
{
public:
    LatencyMeter(GenericProcessor* processor);

    void setLatestLatency(std::map<uint16, juce::int64>& processStartTimes);

    void update(Array<const DataStream*>);

private:
    int counter;

    std::map<uint16, Array<int>> latencies;
    GenericProcessor* processor;
};


#endif  // __GENERICPROCESSOR_H_1F469DAF__
