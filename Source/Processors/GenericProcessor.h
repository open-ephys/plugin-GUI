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
#include "../UI/Configuration.h"
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


class FilterViewport;
class DataViewport;
class UIComponent;

class GenericProcessor : public AudioProcessor,
						 public ActionBroadcaster

{
public:

	GenericProcessor(const String& name_);
	virtual ~GenericProcessor();
	
	const String getName() const {return name;}
	//virtual void setName(const String& name_) {}
	
	void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
	void releaseResources();
	
	void setParameter (int parameterIndex, float newValue);

	virtual AudioProcessorEditor* createEditor();
	bool hasEditor() const {return true;}
	
	void reset() {}
	void setCurrentProgramStateInformation(const void* data, int sizeInBytes) {}
	void setStateInformation(const void* data, int sizeInBytes) {}
	void getCurrentProgramStateInformation(MemoryBlock &destData) {}
	void getStateInformation (MemoryBlock &destData) {}
	void changeProgramName (int index, const String &newName) {}
	void setCurrentProgram (int index) {}

	const String getInputChannelName (int channelIndex) const {return T(" ");}
	const String getOutputChannelName (int channelIndex) const {return T(" ");}
	const String getParameterName (int parameterIndex) {return T(" ");}
	const String getParameterText (int parameterIndex) {return T(" ");}
	const String getProgramName (int index) {return T(" ");}
	
	bool isInputChannelStereoPair (int index) const {return true;}
	bool isOutputChannelStereoPair (int index) const {return true;}
	bool acceptsMidi () const {return true;}
	bool producesMidi () const {return true;}

	bool isParameterAutomatable(int parameterIndex) {return false;}
	bool isMetaParameter(int parameterIndex) {return false;}
	
	int getNumParameters() {return 0;}
	int getNumPrograms() {return 0;}
	int getCurrentProgram() {return 0;}
	
	float getParameter (int parameterIndex) {return 1.0;}

	// custom methods:

	// pure virtual function
	virtual void process(AudioSampleBuffer& /*buffer*/,
						 MidiBuffer& /*buffer*/,
						 int& /*nSamples*/) = 0;

	const String name;
	//int* numSamplesInThisBuffer;
	//const CriticalSection& lock;
	int nodeId;

	GenericProcessor* sourceNode;
	GenericProcessor* destNode;

	FilterViewport* viewport;
	DataViewport* dataViewport;

	Configuration* config;

	AudioProcessorEditor* editor;

	int numInputs;
	int numOutputs;

	float sampleRate;

	UIComponent* UI;

	//void sendMessage(const String& msg);

	virtual float getSampleRate();
	virtual void setSampleRate(float sr);

	virtual int getNumInputs();
	virtual void setNumInputs(int);
	virtual void setNumInputs();

	virtual int getNumOutputs();
	virtual void setNumOutputs(int);
	virtual void setNumOutputs();

	int getNodeId() {return nodeId;}
	void setNodeId(int id) {nodeId = id;}

	// get/set source node functions
	GenericProcessor* getSourceNode() {return sourceNode;}
	GenericProcessor* getDestNode() {return destNode;}
	GenericProcessor* getOriginalSourceNode();

	virtual void setSourceNode(GenericProcessor* sn);
	virtual void setDestNode(GenericProcessor* dn);

	virtual bool isSource() {return false;}
	virtual bool isSink() {return false;}
	virtual bool isSplitter() {return false;}
	virtual bool isMerger() {return false;}

	virtual bool canSendSignalTo(GenericProcessor*) {return true;}

	virtual bool enable() {return isEnabled;}
	virtual bool disable() {return true;}

	virtual bool enabledState() {return isEnabled;}
	virtual void enabledState(bool t) {isEnabled = t;}

	virtual AudioSampleBuffer* getContinuousBuffer() {return 0;}
	virtual MidiBuffer* getEventBuffer() {return 0;}

	AudioProcessorEditor* getEditor() {return editor;}
	void setEditor(AudioProcessorEditor* e) {editor = e;}

	void setUIComponent(UIComponent* ui) {UI = ui;}
	UIComponent* getUIComponent() {return UI;}

	virtual void setConfiguration(Configuration* cf) {config = cf;}
	Configuration* getConfiguration() {return config;}

	void setFilterViewport(FilterViewport* vp) {viewport = vp;}
	void setDataViewport(DataViewport* dv);
	DataViewport* getDataViewport() {return dataViewport;}

	int checkForMidiEvents(MidiBuffer& mb);
	void addMidiEvent(MidiBuffer& mb, int a, int b);

	bool isEnabled;

private:

	void processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages);

	

	
	
	int getNumSamples(MidiBuffer&);
	void setNumSamples(MidiBuffer&, int);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);

};




#endif  // __GENERICPROCESSOR_H_1F469DAF__
