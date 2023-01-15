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
#include "../Events/Spike.h"

#include <time.h>
#include <stdio.h>
#include <map>
#include <unordered_map>
#include <limits>

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
    GenericProcessor (const String& name);

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
    virtual void setParameter(int parameterIndex, float newValue) override;

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
    virtual void startRecording() { }

    /** Called from whenever recording has stopped. */
    virtual void stopRecording() { }

    /** Indicates whether a source node is connected to a processor (used for mergers).*/
    virtual bool stillHasSource() const { return true; }

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a boolean parameter, which will later be accessed by name*/
    void addBooleanParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        bool defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an integer parameter, which will later be accessed by name*/
    void addIntParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        int defaultValue,
        int minValue,
        int maxValue,
        bool deactivateDuringAcquisition = false);
    
    /** Adds a string parameter, which will later be accessed by name*/
    void addStringParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        String defaultValue,
        bool deactivateDuringAcquisition = false);

    /** Adds an float parameter, which will later be accessed by name*/
    void addFloatParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        float defaultValue,
        float minValue,
        float maxValue,
        float stepSize,
        bool deactivateDuringAcquisition = false);

    /** Adds a selected channels parameter, which will later be accessed by name*/
    void addSelectedChannelsParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        int maxSelectedChannels = std::numeric_limits<int>::max(),
        bool deactivateDuringAcquisition = false);
    
    /** Adds a mask channels parameter, which will later be accessed by name*/
    void addMaskChannelsParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        bool deactivateDuringAcquisition = false);

    /** Adds a categorical parameter, which will later be accessed by name*/
    void addCategoricalParameter(Parameter::ParameterScope scope,
        const String& name,
        const String& description,
        StringArray categories,
        int defaultIndex,
        bool deactivateDuringAcquisition = false);

    /** Returns a pointer to a global Parameter*/
    Parameter* getParameter(const String& parameterName);
    
    /** Returns a pointer to a Parameter object for a given stream (0 = no stream)*/
    Parameter* getParameter(const uint16 streamId, const String& parameterName);

    /** Returns a pointer to a Parameter object for a given event channel.*/
    Parameter* getParameter(EventChannel* channelInfo, const String& parameterName);
    
    /** Returns a pointer to a Parameter object for a given continuous channel.*/
    Parameter* getParameter(ContinuousChannel* channelInfo, const String& parameterName);
    
    /** Returns a pointer to a Parameter object for a given spike channel.*/
    Parameter* getParameter(SpikeChannel* channelInfo, const String& parameterName);
    
    /** Returns a list of global parameters for this processor*/
    Array<Parameter*> getParameters();
    
    /** Returns a list of parameters for a given stream within this processor*/
    Array<Parameter*> getParameters(uint16 streamId);
    
    /** Returns a list of parameters for a given event channel within this processor*/
    Array<Parameter*> getParameters(EventChannel* channelInfo);
    
    /** Returns a list of parameters for a given continuous channel within this processor*/
    Array<Parameter*> getParameters(ContinuousChannel* channelInfo);
    
    /** Returns a list of parameters for a given spike channel within this processor*/
    Array<Parameter*> getParameters(SpikeChannel* channelInfo);

    /** Initiates parameter value update */
    void parameterChangeRequest(Parameter*);

    /** Called when a parameter value is updated, to allow plugin-specific responses*/
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

    /** Allows the processor to load its default state after the editor is created.

        The "signalChainIsLoading" flag can be used to prevent the processor from
        performing unnecessary steps before it receives information
        about its previous settings.
    */
    virtual void initialize(bool signalChainIsLoading) { }

    /** Method for updating settings, called by ProcessorGraph.*/
    void update();

    // --------------------------------------------
    //     LOADING / SAVING SETTINGS
    // --------------------------------------------

    /** Save generic settings to XML (called by all processors).*/
    void saveToXml (XmlElement* parentElement);

    /** Load generic settings from XML (called by all processors). */
    void loadFromXml();


    // --------------------------------------------
   //     SAVING + LOADING SETTINGS
   // --------------------------------------------

   /** Saving custom settings to XML. */
    virtual void saveCustomParametersToXml(XmlElement* parentElement);

    /** Load custom settings from XML*/
    virtual void loadCustomParametersFromXml(XmlElement* customParamsXml);

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

    uint16 findSimilarStream(int sourceNodeId, String name, float sample_rate, bool sourceNodeIdMustMatch = false);

    virtual Array<const DataStream*> getStreamsForDestNode(GenericProcessor* processor);

    Array<const DataStream*> getDataStreams() const;

	const ConfigurationObject* getConfigurationObject(int index) const;

	int getTotalContinuousChannels() const;
	
	int getTotalEventChannels() const;

	int getTotalSpikeChannels() const;

	int getTotalConfigurationObjects() const;

    Plugin::Processor::Type getProcessorType() const;
    
    String getProcessorTypeString() const;
    
    static Plugin::Processor::Type typeFromString(String typeName);

    String getDisplayName() { return m_name; }

    void updateDisplayName(String name);

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

    /** Determines whether the processor is ready to start acquisition
        If set to false, the editor will be greyed out
    */
    bool isEnabled;
    
    /** Pointer to the processor's editor. */
    std::unique_ptr<GenericEditor> editor;
    
    
    /** Sets whether processor will have behaviour like Source, Sink, Splitter, Utility or Merge */
    void setProcessorType (Plugin::Processor::Type processorType);

protected:

    // --------------------------------------------
    //     SAMPLES + TIMESTAMPS
    // --------------------------------------------

    /** Used to get the number of samples available in a current block, for a given stream */
    uint32 getNumSamplesInBlock(uint16 streamId) const;

    /** Used to get the current sample number for a given stream */
    int64 getFirstSampleNumberForBlock(uint16 streamId) const;
    
    /** Used to get the current timestamp for a given stream.*/
    double getFirstTimestampForBlock(uint16 streamId) const;

	/** Used to set the timestamp for a given buffer, for a given DataStream. */
	void setTimestampAndSamples(int64 startSampleForBlock,
                                double startTimestampForBlock,
                                uint32 nSamples,
                                uint16 streamId);
    
    // --------------------------------------------
    //     CHANNEL INDEXING
    // --------------------------------------------
    int getGlobalChannelIndex(uint16 streamId, int localIndex) const;

    // --------------------------------------------
    //     HANDLING EVENTS AND MESSAGES
    // --------------------------------------------

    /** Handles a configuration message sent to this processor, while acquisition is not active.*/
    virtual String handleConfigMessage(String msg);
    
    /** Handles a configuration message sent to this processor, while acquisition is not active.*/
    virtual void handleBroadcastMessage(String msg) { }

	/** Can be called by processors that need to respond to incoming events.
	Set respondToSpikes to true if the processor should also search for spikes*/
	virtual int checkForEvents(bool respondToSpikes = false);

	/** Allows processors to respond to incoming TTL events; called by checkForEvents() */
	virtual void handleTTLEvent(TTLEventPtr event) { }

	/** Allows processors to respond to incoming spikes; called by checkForEvents(true) */
	virtual void handleSpike(SpikePtr spike) { }

	/** Returns info about the default events a specific subprocessor generates.
	Called by createEventChannels(). It is not needed to implement if createEventChannels() is overriden */
	virtual void getDefaultEventInfo(Array<DefaultEventInfo>& events, int subProcessorIdx = 0) const;


    // --------------------------------------------
    //     ADDING SPIKES AND EVENTS
    // --------------------------------------------

    /** Add an event (usually a TTLEventPtr) to the processing buffer */
    void addEvent(const Event* event, int sampleNum);

    /** Sends a TEXT event to all other processors, via the MessageCenter, while acquisition is active.
        If recording is active, this message will be recorded */
    void broadcastMessage(String msg);

    /** Add a Spike event to the outgoing buffer */
    void addSpike(const Spike* event);

    /// OPTIONAL HELPER FUNCTIONS ///

    /** Create a simple TTL event channel with 8 lines on the first incoming data stream
        -- Must be called during updateSettings() --
    */
    void addTTLChannel(String name);

    /** Flip the state of one of the TTL lines 
        -- Must be called during the process() method --
     */
    void flipTTLState(int sampleIndex, int lineIndex);

    /** Set the state of one of the TTL lines
        -- Must be called during the process() method --
     */
    void setTTLState(int sampleIndex, int lineIndex, bool state);

    /** Get the state of one of the TTL lines
        -- Must be called during the process() method --
     */
    bool getTTLState(int lineIndex);

    // --------------------------------------------
    //     UPDATING SETTINGS
    // --------------------------------------------

	/** Custom method for updating settings, called automatically by update() after creating the info objects.*/
	virtual void updateSettings() { }

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
    int copyDataStreamSettings(const DataStream*, int continuousChannelGlobalIndex);

    /** Finds the parameters that most likely belong to this stream */
    int findMatchingStreamParameters(DataStream* stream);
    
    /** Sets whether or not a given stream is enabled*/
    void setStreamEnabled(uint16 streamId, bool isEnabled);

    /** Updates the data channel map objects*/
	void updateChannelIndexMaps();

    /** Holds info about this processor.*/
    std::unique_ptr<ProcessorInfoObject> processorInfo;

    /** Holds info about available devices.*/
    OwnedArray<DeviceInfo> devices;

    /** When set to false, this disables the sending of sample counts through the event buffer (used by Mergers and Splitters). */
    bool sendSampleCount;

    /** An array of default parameters for this processor.*/
    OwnedArray<Parameter> availableParameters;
    
    /** An array of parameter collections from deleted dataStreams*/
    OwnedArray<ParameterCollection> savedDataStreamParameters;
    
    /** Used to quickly access parameters by name*/
    std::map<String, Parameter*> globalParameterMap;

    /** Used to quickly access parameters by name*/
    std::map<uint16, std::map<String, Parameter*>> streamParameterMap;
    
    /** Used to quickly access parameters by name*/
    std::map<EventChannel*, std::map<String, Parameter*>> eventChannelParameterMap;
    
    /** Used to quickly access parameters by name*/
    std::map<ContinuousChannel*, std::map<String, Parameter*>> continuousChannelParameterMap;
    
    /** Used to quickly access parameters by name*/
    std::map<SpikeChannel*, std::map<String, Parameter*>> spikeChannelParameterMap;

    /** Holds a pointer to the event channel for sending messages**/
    std::unique_ptr<EventChannel> messageChannel;
    
private:

    /** Clears the settings arrays.*/
    void clearSettings();

    /** Map between stream IDs and buffer sample counts. */
	std::map<uint16, uint32> numSamplesInBlock;

    /** Map between stream IDs and buffer timestamps. */
	std::map<uint16, double> startTimestampsForBlock;
    
    /** Map between stream IDs and buffer timestamps. */
    std::map<uint16, int64> startSamplesForBlock;

    /** Map between stream IDs and start time of process callbacks. */
    std::map<uint16, int64> processStartTimes;

    /** First software timestamp of process() callback. */
	juce::int64 m_initialProcessTime;

    /** Built-in method for creating continuous channels. */
	void createDataChannelsByType(ContinuousChannel::Type type);

	/** Each processor has a unique integer ID that can be used to identify it.*/
	int nodeId;

    /** Automatically extracts the number of samples in the buffer, then
    calls the process(), where custom actions take place.*/
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

    /** Extracts sample counts and timestamps from the MidiBuffer. */
    int processEventBuffer();

    /** The type of the processor. */
    Plugin::Processor::Type m_processorType;

    /** The name of the processor.*/
    String m_name;

    /** For getInputChannelName() and getOutputChannelName() */
    static const String m_unusedNameString;

    bool m_paramsWereLoaded;

	std::map<int,bool> m_needsToSendTimestampMessages;

	MidiBuffer* m_currentMidiBuffer;
    MidiBuffer messageCenterBuffer;

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
    Array<bool> ttlLineStates;

    bool wasConnected;

    std::unique_ptr<LatencyMeter> latencyMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);
};

/** 
    Measures the time elapsed between the start of each processing 
    cycle and the end of a GenericProcessor's work.
*/
class LatencyMeter
{
public:

    /** Constructor */
    LatencyMeter(GenericProcessor* processor);

    /** Sets the latest latency values for each data stream */
    void setLatestLatency(std::map<uint16, juce::int64>& processStartTimes);

    /** Updates the available data streams */
    void update(Array<const DataStream*>);

private:
    int counter;

    std::map<uint16, Array<int>> latencies;
    GenericProcessor* processor;
};


#endif  // __GENERICPROCESSOR_H_1F469DAF__
