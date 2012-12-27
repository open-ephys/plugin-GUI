/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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


#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/GenericEditor.h"
#include "Parameter.h"
#include "../AccessClass.h"

#include <time.h>
#include <stdio.h>

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

class GenericProcessor : public AudioProcessor,
						 public AccessClass

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
	const String getName() const {return name;}
	
	/** Called by JUCE as soon as a processor is created, as well as before the start of audio callbacks. To avoid starting data acquisition prematurely, use the enable() function instead.
	*/
	virtual void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);

	/** Called by JUCE as soon as audio callbacks end. Use disable() instead. */
	void releaseResources();
	
	/** Allows parameters to change while acquisition is active. If the user wants
	to change ANY variables that are used within the process() method, this must
	be done through setParameter(). Otherwise the application will crash. */
	virtual void setParameter (int parameterIndex, float newValue);

	/** Creates a GenericEditor.*/
	virtual AudioProcessorEditor* createEditor();

	/** The default is to have no editor.*/
	bool hasEditor() const {return false;}
	
	/** JUCE method. Not used.*/
	void reset() {}

	/** JUCE method. Not used.*/
	void setCurrentProgramStateInformation(const void* data, int sizeInBytes) {}

	/** JUCE method. Not used.*/
	void setStateInformation(const void* data, int sizeInBytes) {}

	/** JUCE method. Not used.*/
	void getCurrentProgramStateInformation(MemoryBlock &destData) {}

	/** JUCE method. Not used.*/
	void getStateInformation (MemoryBlock &destData) {}

	/** JUCE method. Not used.*/
	void changeProgramName (int index, const String &newName) {}

	/** JUCE method. Not used.*/
	void setCurrentProgram (int index) {}

	/** Returns the name of the input channel with a given index.*/
	const String getInputChannelName (int channelIndex) const { }

	/** Returns the name of the output channel with a given index.*/
	const String getOutputChannelName (int channelIndex) const { }

	/** Returns the name of the parameter with a given index.*/
	const String getParameterName (int parameterIndex);

	/** Returns additional details about the parameter with a given index.*/
	const String getParameterText (int parameterIndex); 

	/** Returns the current value of a parameter with a given index.*/
	float getParameter (int parameterIndex) {return 1.0;}

	/** JUCE method. Not used.*/
	const String getProgramName (int index) {return "";}
	
	/** JUCE method. Not used.*/
	bool isInputChannelStereoPair (int index) const {return true;}

	/** JUCE method. Not used.*/
	bool isOutputChannelStereoPair (int index) const {return true;}

	/** All processors can accept MIDI (event) data by default.*/
	bool acceptsMidi () const {return true;}

	/** All processors can produce MIDI (event) data by default.*/
	bool producesMidi () const {return true;}

	/** JUCE method. Not used.*/
	bool isParameterAutomatable(int parameterIndex) {return false;}

	/** JUCE method. Not used.*/
	bool isMetaParameter(int parameterIndex) {return false;}
	
	/** Returns the number of user-editable parameters for this processor.*/
	int getNumParameters() {return parameters.size();}

	/** JUCE method. Not used.*/
	int getNumPrograms() {return 0;}

	/** JUCE method. Not used.*/
	int getCurrentProgram() {return 0;}

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
						 MidiBuffer& eventBuffer,
						 int& nSamples) = 0;

	/** Pointer to a processor's immediate source node.*/
	GenericProcessor* sourceNode;

	/** Pointer to a processor's immediate destination.*/
	GenericProcessor* destNode;

	/** Returns the sample rate for a processor (assumes the same rate for all channels).*/
	virtual float getSampleRate() {return settings.sampleRate;}

	/** Returns the default sample rate, in case a processor has no source (or is itself a source).*/
	virtual float getDefaultSampleRate() {return 44100.0;}

	/** Returns the number of inputs to a processor.*/
	virtual int getNumInputs() {return settings.numInputs;}

	/** Returns the number of outputs from a processor.*/
	virtual int getNumOutputs() {return settings.numOutputs;}

	/** Returns the default number of outputs, in case a processor has no source (or is itself a source).*/
	virtual int getDefaultNumOutputs() {return 2;}

	/** Returns the default number of volts per bit, in case a processor has no source (or is itself a source).*/
	virtual float getDefaultBitVolts() {return 1.0;}

	/** Returns the next available channel (and increments the channel if the input is set to 'true'. */
	virtual int getNextChannel(bool t);

	/** Resets all inter-processor connections prior to the start of data acquisition.*/
	virtual void resetConnections();
	
	/** Sets the current channel (for purposes of updating parameter).*/
	virtual void setCurrentChannel(int chan) {currentChannel = chan;}

	/** Returns the unique integer ID for a processor. */
	int getNodeId() {return nodeId;}

	/** Sets the unique integer ID for a processor. */
	void setNodeId(int id) {nodeId = id;}

	/** Returns a pointer to the processor immediately preceding a given processor in the signal chain. */
	GenericProcessor* getSourceNode() {return sourceNode;}

	/** Returns a pointer to the processor immediately following a given processor in the signal chain. */
	GenericProcessor* getDestNode() {return destNode;}

	/** Sets the input or output of a splitter or merger.*/
	virtual void switchIO(int) { }

	/** Switches the input or output of a splitter or merger.*/
	virtual void switchIO() { }

	/** Sets the input to a merger a given processor.*/
	virtual void setPathToProcessor(GenericProcessor* p) { }

	/** Sets a processor's source node.*/
	virtual void setSourceNode(GenericProcessor* sn);

	/** Sets a processor's destination node.*/
	virtual void setDestNode(GenericProcessor* dn);

	/** Sets one of two possible source nodes for a merger.*/
	virtual void setMergerSourceNode(GenericProcessor* sn) { }

	/** Sets one of two possible source nodes for a splitter.*/
	virtual void setSplitterDestNode(GenericProcessor* dn) { }

	/** Returns true if a processor is a source, false otherwise.*/
	virtual bool isSource() {return false;}

	/** Returns true if a processor is a sink, false otherwise.*/
	virtual bool isSink() {return false;}

	/** Returns true if a processor is a splitter, false otherwise.*/
	virtual bool isSplitter() {return false;}

	/** Returns true if a processor is a merger, false otherwise.*/
	virtual bool isMerger() {return false;}

	/** Returns true if a processor is able to send its output to a given processor.

	    Ideally, this should always return true, but there may be special cases
	    when this is not possible.*/
	virtual bool canSendSignalTo(GenericProcessor*) {return true;}

	/** Returns true if a processor is ready to process data (e.g., all of its parameters are initialized, and its data source is connected).*/
	virtual bool isReady() {return isEnabled;}

	/** Called immediately prior to the start of data acquisition, once all processors in the signal chain have indicated they are ready to process data.*/
	virtual bool enable() {return isEnabled;}

	/** Called immediately after the end of data acquisition.*/
	virtual bool disable() {return true;}

	/** Informs a processor's editor that data acquisition is about to begin. */
	virtual void enableEditor();

	/** Informs a processor's editor that data acquisition has ended. */
	virtual void disableEditor();

	/** Indicates whether or not a processor is currently enabled (i.e., able to process data). */
	virtual bool enabledState() {return isEnabled;}

	/** Sets whether or not a processor is enabled (i.e., able to process data). */
	virtual void enabledState(bool t) {isEnabled = t;}

	/** Turns a given channel on or off. */
	virtual void enableCurrentChannel(bool) {}

	/** Indicates whether a source node is connected to a processor (used for mergers).*/
	virtual bool stillHasSource() {return true;}

	bool isEnabled;
	bool wasConnected;

	/** Returns a pointer to the processor's internal continuous buffer, if it exists. */
	virtual AudioSampleBuffer* getContinuousBuffer() {return 0;}

	/** Returns a pointer to the processor's internal event buffer, if it exists. */
	virtual MidiBuffer* getEventBuffer() {return 0;}

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
	                      uint8* data = 0);

	/** Makes it easier for processors to respond to incoming events, such as TTLs and spikes.

	Called by checkForEvents(). */
	virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0) {}

	enum eventTypes 
 	{
 		TIMESTAMP = 0,
 		BUFFER_SIZE = 1,
 		PARAMETER_CHANGE = 2,
 		TTL = 3,
 		SPIKE = 4,
 		EEG = 5,
 		CONTINUOUS = 6
 	};

 	enum eventChannelTypes
 	{
 		GENERIC_EVENT = 999,
 		SINGLE_ELECTRODE = 1,
 		STEREOTRODE = 2,
 		TETRODE = 4
 	};

 	/** Variable used to orchestrate saving the ProcessorGraph. */
	int saveOrder;

	/** Variable used to orchestrate loading the ProcessorGraph. */
	int loadOrder;

	/** The channel that will be updated the next time a parameter is changed. */
	int currentChannel;

	/** Returns a pointer to the processor's editor. */
	virtual GenericEditor* getEditor() {return editor;}

	/** Pointer to the processor's editor. */
	ScopedPointer<GenericEditor> editor;

	/** Array of Channel objects for all continuous channels. */
	OwnedArray<Channel> channels;

	/** Array of Channel objects for all event channels. */
	OwnedArray<Channel> eventChannels;

	/** Settings used by most processors. */
	struct ProcessorSettings {

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
	virtual void updateSettings() {}

	/** Each processor has a unique integer ID that can be used to identify it.*/
	int nodeId;

	/** An array of parameters that the user can modify.*/
	Array<Parameter> parameters;

	/** Returns the parameter for a given name.*/
	Parameter& getParameterByName(String parameterName);

	/** Returns the parameter for a given index.*/
	Parameter& getParameterReference(int parameterIndex);

private:

	/** Automatically extracts the number of samples in the buffer, then 
	calls the process(), where custom actions take place.*/
	void processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages);

	/** The name of the processor.*/
	const String name;
	
	/** Returns the number of samples for the current continuous buffer (assumed to be
	the same for all channels).*/
	int getNumSamples(MidiBuffer&);

	/** Updates the number of samples for the current continuous buffer (assumed to be
	the same for all channels).*/
	void setNumSamples(MidiBuffer&, int);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);

};




#endif  // __GENERICPROCESSOR_H_1F469DAF__
