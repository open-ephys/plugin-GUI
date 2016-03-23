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

#ifndef __GENERICPROCESSOR_H_1F469DAF__
#define __GENERICPROCESSOR_H_1F469DAF__

enum ChannelType {HEADSTAGE_CHANNEL = 0, AUX_CHANNEL = 1, ADC_CHANNEL = 2, EVENT_CHANNEL = 3,
                  ELECTRODE_CHANNEL = 4,  MESSAGE_CHANNEL = 5
                 };

//defines which events are writable to files
#define isWritableEvent(ev) (((int)(ev) == GenericProcessor::TTL) || ((int)(ev) == GenericProcessor::MESSAGE) || ((int)(ev) == GenericProcessor::BINARY_MSG))

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../Parameter/Parameter.h"
#include "../Channel/Channel.h"
#include "../../CoreServices.h"
#include "../PluginManager/PluginClass.h"
#include "../../Processors/Dsp/LinearSmoothedValueAtomic.h"

#include <time.h>
#include <stdio.h>
#include <map>

class EditorViewport;
class DataViewport;
class UIComponent;
class GenericEditor;
class Parameter;
class Channel;

/**

  Abstract base class for creating processors.

  All processors must be derived from this class, and must provide an
  implementation of the process() method.

  Any processors that are not filters must override the isSource(),
  isSink(), isSplitter(), and isMerger() methods.

  See https://github.com/open-ephys/GUI/wiki/Custom-processors for information
  on how to design a processor that inherits from GenericProcessor.

  @see ProcessorGraph, GenericEditor, SourceNode, FilterNode, LfpDisplayNode

*/

class PLUGIN_API GenericProcessor : public AudioProcessor, public PluginClass
{
public:

    /*
    ------------------------------------------------------------------------
    ----------------------------- JUCE METHODS -----------------------------
    ------------------------------------------------------------------------
    */

    /** Constructor (sets the processor's name). */
    GenericProcessor(const String& name_);

    /** Destructor. */
    virtual ~GenericProcessor();

    /** Returns the name of the processor. */
    const String getName() const;

    /** Called by JUCE as soon as a processor is created, as well as before the start of audio callbacks. To avoid starting data acquisition prematurely, use the enable() function instead.
    */
    virtual void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);

    /** Called by JUCE as soon as audio callbacks end. Use disable() instead. */
    void releaseResources();

    /** Allows parameters to change while acquisition is active. If the user wants
    to change ANY variables that are used within the process() method, this must
    be done through setParameter(). Otherwise the application will crash. */
    virtual void setParameter(int parameterIndex, float newValue);

    /** Creates a GenericEditor.*/
    virtual AudioProcessorEditor* createEditor();

    /** The default is to have no editor.*/
    virtual bool hasEditor() const;

    /** JUCE method. Not used.*/
    void reset();

    /** JUCE method. Not used.*/
    void setCurrentProgramStateInformation(const void* data, int sizeInBytes);

    /** JUCE method. Not used.*/
    void setStateInformation(const void* data, int sizeInBytes);

    /** JUCE method. Not used.*/
    void getCurrentProgramStateInformation(MemoryBlock& destData);

    /** JUCE method. Not used.*/
    void getStateInformation(MemoryBlock& destData);

    /** JUCE method. Not used.*/
    void changeProgramName(int index, const String& newName);

    /** JUCE method. Not used.*/
    void setCurrentProgram(int index);

    /** Returns the name of the input channel with a given index.*/
    virtual const String getInputChannelName(int channelIndex) const;

    /** Returns the name of the output channel with a given index.*/
    virtual const String getOutputChannelName(int channelIndex) const;

    virtual void getEventChannelNames(StringArray& Names);

    /** Returns the name of the parameter with a given index.*/
    const String getParameterName(int parameterIndex);

    /** Returns additional details about the parameter with a given index.*/
    const String getParameterText(int parameterIndex);

    /** Returns the current value of a parameter with a given index.
     Currently set to always return 1. See getParameterVar below*/
    float getParameter(int parameterIndex);

    /**Returns the current value of a parameter with a give index */
    var getParameterVar(int parameterIndex, int parameterChannel);

    /** JUCE method. Not used.*/
    const String getProgramName(int index);

    /** JUCE method. Not used.*/
    bool isInputChannelStereoPair(int index) const;

    /** JUCE method. Not used.*/
    bool isOutputChannelStereoPair(int index) const;

    /** All processors can accept MIDI (event) data by default.*/
    bool acceptsMidi() const;

    /** All processors can produce MIDI (event) data by default.*/
    bool producesMidi() const;

    /** JUCE method. Not used.*/
    bool isParameterAutomatable(int parameterIndex);

    /** JUCE method. Not used.*/
    bool isMetaParameter(int parameterIndex);

    /** Returns the number of user-editable parameters for this processor.*/
    int getNumParameters();

    /** JUCE method. Not used.*/
    int getNumPrograms();

    /** JUCE method. Not used.*/
    int getCurrentProgram();

    /** JUCE method. Not used.*/
    bool silenceInProducesSilenceOut() const;

    /** JUCE method. Not used.*/
    double getTailLengthSeconds() const;

    /*
    ------------------------------------------------------------------------
    ---------------------------- CUSTOM METHODS ----------------------------
    ------------------------------------------------------------------------
    */

    /** Defines a processor's functionality.

    This is the most important function for each
    processor, as it determines how it creates, modifies, or responds to incoming data
    streams. Rather than use the default JUCE processBlock() method, processBlock()
    automatically calls process() in order to add the 'nSamples' variable to indicate
    the number of samples in the current buffer.
    */
    virtual void process(AudioSampleBuffer& continuousBuffer,
                         MidiBuffer& eventBuffer) = 0;

    /** Pointer to a processor's immediate source node.*/
    GenericProcessor* sourceNode;

    /** Pointer to a processor's immediate destination.*/
    GenericProcessor* destNode;

    /** Returns the sample rate for a processor (assumes the same rate for all channels).*/
    virtual float getSampleRate();

    /** Returns the default sample rate, in case a processor has no source (or is itself a source).*/
    virtual float getDefaultSampleRate();

    /** Returns the number of inputs to a processor.*/
    virtual int getNumInputs();

    /** Returns the number of outputs from a processor.*/
    virtual int getNumOutputs();

    /** Returns the default number of headstage (neural data) outputs, in case a processor has no source (or is itself a source).*/
    virtual int getNumHeadstageOutputs();

    /** Returns the default number of ADC (typically 0-5V, or -5 to +5V) outputs. */
    virtual int getNumAdcOutputs();

    /** Returns the default number of auxiliary (e.g. accelerometer) outputs. */
    virtual int getNumAuxOutputs();

    /** Returns the default number of event channels. */
    virtual int getNumEventChannels();

    /** Returns the default number of volts per bit, in case a processor is a source, of the processor gain otherwise. (assumes data comes from a 16bit source)*/
    virtual float getDefaultBitVolts();

    /** Returns the bit volts for a given channel **/
    virtual float getBitVolts(Channel* chan);


    /** Returns the next available channel (and increments the channel if the input is set to 'true'. */
    virtual int getNextChannel(bool t);

    /** Resets all inter-processor connections prior to the start of data acquisition.*/
    virtual void resetConnections();

    /** Sets the current channel (for purposes of updating parameter).*/
    virtual void setCurrentChannel(int chan);

    /** Returns the unique integer ID for a processor. */
    int getNodeId();

    /** Sets the unique integer ID for a processor. */
    void setNodeId(int id);

    /** Returns a pointer to the processor immediately preceding a given processor in the signal chain. */
    GenericProcessor* getSourceNode();

    /** Returns a pointer to the processor immediately following a given processor in the signal chain. */
    GenericProcessor* getDestNode();

    /** Sets the input or output of a splitter or merger.*/
    virtual void switchIO(int);

    /** Switches the input or output of a splitter or merger.*/
    virtual void switchIO();

    /** Sets the input to a merger a given processor.*/
    virtual void setPathToProcessor(GenericProcessor* p);

    /** Sets a processor's source node.*/
    virtual void setSourceNode(GenericProcessor* sn);

    /** Sets a processor's destination node.*/
    virtual void setDestNode(GenericProcessor* dn);

    /** Sets one of two possible source nodes for a merger.*/
    virtual void setMergerSourceNode(GenericProcessor* sn);

    /** Sets one of two possible source nodes for a splitter.*/
    virtual void setSplitterDestNode(GenericProcessor* dn);

    /** Returns trus if a processor generates its own timestamps, false otherwise.*/
    virtual bool generatesTimestamps();

    /** Returns true if a processor is a source, false otherwise.*/
    virtual bool isSource();

    /** Returns true if a processor is a sink, false otherwise.*/
    virtual bool isSink();

    /** Returns true if a processor is a splitter, false otherwise.*/
    virtual bool isSplitter();

    /** Returns true if a processor is a merger, false otherwise.*/
    virtual bool isMerger();

    /** Returns true if a processor is a utility (non-merger or splitter), false otherwise.*/
    virtual bool isUtility();

    /** Returns true if a processor is able to send its output to a given processor.

        Ideally, this should always return true, but there may be special cases
        when this is not possible.*/
    virtual bool canSendSignalTo(GenericProcessor*);

    /** Returns true if a processor is ready to process data (e.g., all of its parameters are initialized, and its data source is connected).*/
    virtual bool isReady();

    /** Called immediately prior to the start of data acquisition, once all processors in the signal chain have indicated they are ready to process data.*/
    virtual bool enable();

    /** Called immediately after the end of data acquisition.*/
    virtual bool disable();

    /** Called when recording starts/stops **/
    void setRecording(bool state);

    /** Called from setRecording whenever recording has started. */
    virtual void startRecording();

    /** Called from setRecording whenever recording has stopped. */
    virtual void stopRecording();

    /** Informs a processor's editor that data acquisition is about to begin. */
    virtual void enableEditor();

    /** Informs a processor's editor that data acquisition has ended. */
    virtual void disableEditor();

    /** Indicates whether or not a processor is currently enabled (i.e., able to process data). */
    virtual bool enabledState();

    /** Sets whether or not a processor is enabled (i.e., able to process data). */
    virtual void enabledState(bool t);

    /** Turns a given channel on or off. */
    virtual void enableCurrentChannel(bool);

    /** Indicates whether a source node is connected to a processor (used for mergers).*/
    virtual bool stillHasSource();

    bool isEnabled;
    bool wasConnected;

    /** Returns a pointer to the processor's internal continuous buffer, if it exists. */
    virtual AudioSampleBuffer* getContinuousBuffer();

    /** Returns a pointer to the processor's internal event buffer, if it exists. */
    virtual MidiBuffer* getEventBuffer();

    int nextAvailableChannel;

    /** Can be called by processors that need to respond to incoming events. */
    virtual int checkForEvents(MidiBuffer& mb);

    /** Makes it easier for processors to add events to the MidiBuffer. */
    virtual void addEvent(MidiBuffer& mb,
                          uint8 type,
                          int sampleNum,
                          uint8 eventID = 0,
                          uint8 eventChannel = 0,
                          uint8 numBytes = 0,
                          uint8* data = 0,
                          bool isTimestamp = false);

    /** Makes it easier for processors to respond to incoming events, such as TTLs and spikes.

    Called by checkForEvents(). */
    virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0);

    enum eventTypes
    {
        TIMESTAMP = 0,
        BUFFER_SIZE = 1,
        PARAMETER_CHANGE = 2,
        TTL = 3,
        SPIKE = 4,
        MESSAGE = 5,
        BINARY_MSG = 6
    };

    /** Variable used to orchestrate saving the ProcessorGraph. */
    int saveOrder;

    /** Variable used to orchestrate loading the ProcessorGraph. */
    int loadOrder;

    /** The channel that will be updated the next time a parameter is changed. */
    int currentChannel;

    /** Returns a pointer to the processor's editor. */
    virtual GenericEditor* getEditor();

    /** Pointer to the processor's editor. */
    ScopedPointer<GenericEditor> editor;

    /** Array of Channel objects for all continuous channels. */
    OwnedArray<Channel> channels;

    /** Array of Channel objects for all event channels. */
    OwnedArray<Channel> eventChannels;

    /** Returns total number of channels */
    int totalNumberOfChannels();

    /** Settings used by most processors. */
    struct ProcessorSettings
    {

        GenericProcessor* originalSource;

        int numInputs;
        int numOutputs;

        float sampleRate;

    };

    ProcessorSettings settings;

    /** Resets the 'settings' struct to its default state.*/
    virtual void clearSettings();

    /** Default method for updating settings, called by every processor.*/
    virtual void update();

    /** Custom method for updating settings, called automatically by update().*/
    virtual void updateSettings();

    /** Toggles record ON for all channels */
    void setAllChannelsToRecord();
    /** Each processor has a unique integer ID that can be used to identify it.*/
    int nodeId;

    /** An array of parameters that the user can modify.*/
    Array<Parameter> parameters;

    /** Initialize Parameters */
    //virtual void initializeParameters();

    /** Returns the parameter for a given name.*/
    Parameter& getParameterByName(String parameterName);

    /** Returns the parameter for a given index.*/
    Parameter& getParameterReference(int parameterIndex);

    /** Save generic settings to XML (called by all processors).*/
    void saveToXml(XmlElement* parentElement);

    /** Saving custom settings to XML. */
    virtual void saveCustomParametersToXml(XmlElement* parentElement);

    /** Saving generic settings for each channel (called by all processors). */
    void saveChannelParametersToXml(XmlElement* parentElement, int channelNumber, bool isEventChannel=false);

    /** Saving custom settings for each channel. */
    virtual void saveCustomChannelParametersToXml(XmlElement* channelElement, int channelNumber, bool isEventChannel=false);

    /** Load generic settings from XML (called by all processors). */
    void loadFromXml();

    /** Load custom settings from XML*/
    virtual void loadCustomParametersFromXml();

    /** Load generic parameters for each channel (called by all processors). */
    void loadChannelParametersFromXml(XmlElement* channelElement, bool isEventChannel=false);

    /** Load custom parameters for each channel. */
    virtual void loadCustomChannelParametersFromXml(XmlElement* channelElement, bool isEventChannel=false);

    /** handle messages from other processors */
    virtual String interProcessorCommunication(String command);

    /** Holds loaded parameters */
    XmlElement* parametersAsXml;

    /** When set to false, this disables the sending of sample counts through the event buffer. */
    bool sendSampleCount;

    /** Used to get the number of samples in a given buffer, for a given channel. */
    int getNumSamples(int channelNumber);

    /** Used to get the number of samples in a given buffer, for a given source node. */
    void setNumSamples(MidiBuffer&, int numSamples);

    /** Used to get the timestamp for a given buffer, for a given channel. */
    int64 getTimestamp(int channelNumber);

    /** Used to set the timestamp for a given buffer, for a given source node. */
    void setTimestamp(MidiBuffer&, int64 timestamp);

    std::map<uint8, int> numSamples;
    std::map<uint8, int64> timestamps;

private:

    /** Automatically extracts the number of samples in the buffer, then
    calls the process(), where custom actions take place.*/
    virtual void processBlock(AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    /** The name of the processor.*/
    const String name;

    /** Saves the record status of individual channels, even when other parameters are updated. */
    Array<bool> recordStatus;
    Array<bool> monitorStatus;

    /** Extracts sample counts and timestamps from the MidiBuffer. */
    int processEventBuffer(MidiBuffer&);

    /** For getInputChannelName() and getOutputChannelName() */
    static const String unusedNameString;

    bool paramsWereLoaded;
    bool needsToSendTimestampMessage;

    bool timestampSet;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GenericProcessor);

};





#endif  // __GENERICPROCESSOR_H_1F469DAF__
