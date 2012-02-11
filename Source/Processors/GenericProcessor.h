/*
  ==============================================================================

    GenericProcessor.h
    Created: 7 May 2011 2:26:54pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __GENERICPROCESSOR_H_1F469DAF__
#define __GENERICPROCESSOR_H_1F469DAF__


#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/GenericEditor.h"
#include "../UI/Configuration.h"
#include <time.h>
#include <stdio.h>

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

	virtual bool enable() {return true;}
	virtual bool disable() {return true;}

	bool enabledState() {return isEnabled;}
	void enabledState(bool t) {isEnabled = t;}

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

	void checkForMidiEvents(MidiBuffer& mb);
	void addMidiEvent(MidiBuffer& mb, int a);

private:

	void processBlock (AudioSampleBuffer &buffer, MidiBuffer &midiMessages);

	

	bool isEnabled;
	
	int getNumSamples(MidiBuffer&);
	void setNumSamples(MidiBuffer&, int);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericProcessor);

};




#endif  // __GENERICPROCESSOR_H_1F469DAF__
