//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef RECORDNODE_H_DEFINED
#define RECORDNODE_H_DEFINED

#include <chrono>
#include <math.h>
#include <algorithm>
#include <map>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "RecordNodeEditor.h"
#include "RecordThread.h"
#include "DataQueue.h"
#include "Utils.h"

#include "BinaryFormat/BinaryRecording.h"

//#include "taskflow/taskflow.hpp"

#define WRITE_BLOCK_LENGTH		1024
#define DATA_BUFFER_NBLOCKS		300
#define EVENT_BUFFER_NEVENTS	512
#define SPIKE_BUFFER_NSPIKES	512

#define NIDAQ_BIT_VOLTS			0.001221f
#define NPX_BIT_VOLTS			0.195f
#define MAX_BUFFER_SIZE			40960
#define CHANNELS_PER_THREAD		384

#define DEBUG 1

class RecordNode : public GenericProcessor, public FilenameComponentListener
{

public:

	RecordNode();
	~RecordNode();

	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; }

	
	void updateSubprocessorMap();

	void updateSettings() override;
	int getNumSubProcessors() const override;

	void prepareToPlay(double sampleRate, int estimatedSamplesPerBlock);
	void startRecording() override;

	String generateDirectoryName();
	void createNewDirectory();
    void filenameComponentChanged(FilenameComponent *);
    String generateDateString() const;
	int getExperimentNumber() const;
	int getRecordingNumber() const;

	void updateChannelStates(int srcIndex, int subProcIdx, std::vector<bool> enabled);

	bool isFirstChannelInRecordedSubprocessor(int channel);

	void process(AudioSampleBuffer& buffer) override;

	void stopRecording() override;

	void setParameter(int parameterIndex, float newValue) override;

	void setRecordEvents(bool);

	ScopedPointer<RecordThread> recordThread;
	ScopedPointer<RecordEngine> recordEngine;

	int64 samplesWritten;
	String lastSettingsText;

	int numSubprocessors;

	/** Get the last settings.xml in string form. Since the string will be large, returns a const ref.*/
	const String &getLastSettingsXml() const;

	std::map<int, std::map<int, std::vector<bool>>> m;
	std::map<int, int> n;

	Array<int> channelMap; //Map from record channel index to source channel index
	std::vector<std::vector<int>> subProcessorMap;
	std::vector<int> startRecChannels;

    //TODO: Need to validate these new methods

    /** Must be called by a spike recording source on the "enable" method
    */
    void registerSpikeSource(const GenericProcessor *processor);

    /** Registers an electrode group for spike recording
    Must be called by a spike recording source on the "enable" method
    after the call to registerSpikeSource
    */
    int addSpikeElectrode(const SpikeChannel *elec);

    /** Called by a spike recording source to write a spike to file
    */
    void writeSpike(const SpikeEvent *spike, const SpikeChannel *spikeElectrode);

    bool getRecordThreadStatus() { return shouldRecord; };

    bool newDirectoryNeeded;

    /** Called by the ControlPanel to determine the amount of space
        left in the current dataDirectory.
    */
    float getFreeSpace() const;

    /** Adds a Record Engine to use
    */
    void registerRecordEngine(RecordEngine *engine);

    /** Clears the list of active Record Engines
    */
    void clearRecordEngines();

private:

	bool isRecording;
	bool hasRecorded;
	bool settingsNeeded;
    bool shouldRecord;

	bool recordEvents;
	bool recordSpikes;

	File dataDirectory;
	File rootFolder;

	int experimentNumber;
	int recordingNumber;

	int64 timestamp;
	int numSamples;
	int numChannels;

	ScopedPointer<DataQueue> dataQueue;
	ScopedPointer<EventMsgQueue> eventQueue;
    ScopedPointer<SpikeMsgQueue> spikeQueue;

    int spikeElectrodeIndex;

    Array<bool> validBlocks;
	std::atomic<bool> setFirstBlock;

	//Profiling data structures
	float scaleFactor;
	HeapBlock<float> scaledBuffer;  
	HeapBlock<int16> intBuffer;

	/** Cycle through the event buffer, looking for data to save */
	void handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int samplePosition) override;

	virtual void handleTimestampSyncTexts(const MidiMessage& event);

    /**RecordEngines loaded**/
    OwnedArray<RecordEngine> engineArray;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordNode);

};

#endif