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


#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/GenericEditor.h"
#include "Parameter.h"
#include "../AccessClass.h"

#include <time.h>
#include <stdio.h>

/**
  
  Abstract base class for creating processors.

  All processors must be derived from this class, and must provide an
  implementation of the process() method.

  Any processors that are not filters must override the isSource(),
  isSink(), isSplitter(), and isMerger() methods.

  @see ProcessorGraph, GenericEditor, SourceNode, FilterNode, LfpDisplayNode

*/

class EditorViewport;
class DataViewport;
class UIComponent;
class GenericEditor;
class Parameter;

class GenericProcessor : public AudioProcessor,
						 public AccessClass

{
public:

	//-----------------------------------------------------------------------
	// Juce methods:

	GenericProcessor(const String& name_);
	virtual ~GenericProcessor();
	
	const String getName() const {return name;}
	
	virtual void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	
	virtual void setParameter (int parameterIndex, float newValue);

	virtual AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}
	
	void reset() {}
	void setCurrentProgramStateInformation(const void* data, int sizeInBytes) {}
	void setStateInformation(const void* data, int sizeInBytes) {}
	void getCurrentProgramStateInformation(MemoryBlock &destData) {}
	void getStateInformation (MemoryBlock &destData) {}
	void changeProgramName (int index, const String &newName) {}
	void setCurrentProgram (int index) {}

	const String getInputChannelName (int channelIndex) const {return settings.inputChannelNames[channelIndex];}
	const String getOutputChannelName (int channelIndex) const {return settings.outputChannelNames[channelIndex];}
	const String getParameterName (int parameterIndex); //{return parameters[parameterIndex]->getName();}
	const String getParameterText (int parameterIndex); //{return parameters[parameterIndex]->getDescription();}
	const String getProgramName (int index) {return "";}
	
	bool isInputChannelStereoPair (int index) const {return true;}
	bool isOutputChannelStereoPair (int index) const {return true;}
	bool acceptsMidi () const {return true;}
	bool producesMidi () const {return true;}

	bool isParameterAutomatable(int parameterIndex) {return false;}
	bool isMetaParameter(int parameterIndex) {return false;}
	
	int getNumParameters() {return parameters.size();}
	int getNumPrograms() {return 0;}
	int getCurrentProgram() {return 0;}
	
	float getParameter (int parameterIndex) {return 1.0;}
	Parameter& getParameterByName(String parameterName);
	Parameter& getParameterReference(int parameterIndex);

	//----------------------------------------------------------------------
	// Custom methods:

	// pure virtual function (must be implemented by sub-classes)
	virtual void process(AudioSampleBuffer& /*buffer*/,
						 MidiBuffer& /*buffer*/,
						 int& /*nSamples*/) = 0;

	GenericProcessor* sourceNode;
	GenericProcessor* destNode;

	virtual float getSampleRate() {return settings.sampleRate;}
	virtual float getDefaultSampleRate() {return 44100.0;}

	virtual int getNumInputs() {return settings.numInputs;}
	virtual int getNumOutputs() {return settings.numOutputs;}
	virtual int getDefaultNumOutputs() {return 2;}

	//virtual float getBitVolts() {return settings.bitVolts;}
	virtual float getDefaultBitVolts() {return 1.0;}

	virtual int getNextChannel(bool);
	virtual void resetConnections();
	
	virtual void setCurrentChannel(int chan) {currentChannel = chan;}

	int getNodeId() {return nodeId;}
	void setNodeId(int id) {nodeId = id;}

	// get/set source node functions
	GenericProcessor* getSourceNode() {return sourceNode;}
	GenericProcessor* getDestNode() {return destNode;}

	virtual void switchIO(int) { };
	virtual void switchIO() { };

	virtual void setSourceNode(GenericProcessor* sn);
	virtual void setDestNode(GenericProcessor* dn);
	virtual void setMergerSourceNode(GenericProcessor* sn) { }
	virtual void setSplitterDestNode(GenericProcessor* dn) { }

	virtual bool isSource() {return false;}
	virtual bool isSink() {return false;}
	virtual bool isSplitter() {return false;}
	virtual bool isMerger() {return false;}

	virtual bool canSendSignalTo(GenericProcessor*) {return true;}

	virtual bool isReady() {return isEnabled;}
	virtual bool enable() {return isEnabled;}
	virtual bool disable() {return true;}

	virtual bool enabledState() {return isEnabled;}
	virtual void enabledState(bool t) {isEnabled = t;}

	virtual bool stillHasSource() {return true;}

	bool isEnabled;
	bool wasConnected;

	virtual AudioSampleBuffer* getContinuousBuffer() {return 0;}
	virtual MidiBuffer* getEventBuffer() {return 0;}

	int nextAvailableChannel;

	// event buffers
	virtual int checkForEvents(MidiBuffer& mb);
	virtual void addEvent(MidiBuffer& mb,
	                      uint8 type,
	                      int sampleNum,
	                      uint8 eventID = 0,
	                      uint8 eventChannel = 0,
	                      uint8 numBytes = 0,
	                      uint8* data = 0);

	virtual void handleEvent(int eventType, MidiMessage& event) {}

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

	int saveOrder;
	int loadOrder;

	int currentChannel;

	virtual GenericEditor* getEditor() {return editor;}
	ScopedPointer<GenericEditor> editor;

	struct ProcessorSettings {

		GenericProcessor* originalSource;

		int numInputs;
		int numOutputs;
		StringArray inputChannelNames;
		StringArray outputChannelNames;

		float sampleRate;
		Array<float> bitVolts;

		Array<int> eventChannelIds;
		StringArray eventChannelNames;
		Array<int> eventChannelTypes;

	};

	ProcessorSettings settings;

	virtual bool isAudioOrRecordNode() {return false;}

	virtual bool recordStatus(int chan);

	virtual void clearSettings();

	virtual void generateDefaultChannelNames(StringArray&);

	virtual void update(); // default node updating
	virtual void updateSettings() {} // custom node updating

	int nodeId;

	// parameters:
	Array<Parameter> parameters;
	StringArray parameterNames;

	Parameter nullParam;

	void setStartChannel(int i) {audioAndRecordNodeStartChannel = i;}
	int getStartChannel() {return audioAndRecordNodeStartChannel;}

private:

	int audioAndRecordNodeStartChannel;

	void processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages);

	const String name;
	
	int getNumSamples(MidiBuffer&);
	void setNumSamples(MidiBuffer&, int);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);

};




#endif  // __GENERICPROCESSOR_H_1F469DAF__
