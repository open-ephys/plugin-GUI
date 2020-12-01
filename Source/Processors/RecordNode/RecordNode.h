//This prevents include loops. We recommend changing the macro to a name suitable for your plugin
#ifndef RECORDNODE_H_DEFINED
#define RECORDNODE_H_DEFINED

#include <chrono>
#include <math.h>
#include <algorithm>
#include <memory>
#include <map>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "RecordNodeEditor.h"
#include "RecordThread.h"
#include "DataQueue.h"
#include "Synchronizer.h"
#include "../../Utils/Utils.h"

//#include "taskflow/taskflow.hpp"

#define WRITE_BLOCK_LENGTH		1024
#define DATA_BUFFER_NBLOCKS		300
#define EVENT_BUFFER_NEVENTS	512
#define SPIKE_BUFFER_NSPIKES	512

#define NIDAQ_BIT_VOLTS			0.001221f
#define NPX_BIT_VOLTS			0.195f
#define MAX_BUFFER_SIZE			40960
#define CHANNELS_PER_THREAD		384

class EventMonitor
{
public:

	EventMonitor();
	~EventMonitor();

	int receivedEvents;

	void displayStatus();

};

class RecordNode : public GenericProcessor, public FilenameComponentListener
{

public:

    /** Constructor
      - Creates: DataQueue, EventQueue, SpikeQueue, Synchronizer,
        RecordThread, EventMonitor
      - Sets the Record Engine
      - Gets the Recording Directory from the control panel
      - Sets a bunch of internal variables
     */
	RecordNode();
    
    /** Destructor
            - Doesn't need to delete anything manually
     */
	~RecordNode();

    /** If messageCenter event channel is not present in EventChannelArray, add it*/
	void connectToMessageCenter();

    /** Need to remove message center event channel after recording*/
    void disconnectMessageCenter();

	void updateRecordChannelIndexes();

	AudioProcessorEditor* createEditor() override;
	bool hasEditor() const override { return true; }

	void addSpecialProcessorChannels(Array<EventChannel*>& channels);

	void updateSubprocessorMap();
	void setMasterSubprocessor(int srcIdx, int subProcIdx);
	bool isMasterSubprocessor(int srcIdx, int subProcIdx);
	void setSyncChannel(int srcIdx, int subProcIdx, int channel);
	int getSyncChannel(int srcIdx, int subProcIdx);

	void updateSettings() override;
    bool enable() override;
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

	std::vector<RecordEngineManager*> getAvailableRecordEngines();

	void setEngine(int selectedEngineIndex);
	void setRecordEvents(bool);
	void setRecordSpikes(bool);
	void setDataDirectory(File);
	File getDataDirectory();

	ScopedPointer<RecordThread> recordThread;
	ScopedPointer<RecordEngine> recordEngine;
	std::vector<RecordEngineManager*> availableEngines;	

    ScopedPointer<Synchronizer> synchronizer;

	int64 samplesWritten;
	String lastSettingsText;

	int numSubprocessors;

	/** Get the last settings.xml in string form. Since the string will be large, returns a const ref.*/
	const String &getLastSettingsXml() const;

	std::map<int, std::map<int, std::vector<bool>>> dataChannelStates;
	std::map<int, int> dataChannelOrder;

	std::map<int, std::map<int, int>> eventMap;
	std::map<int, std::map<int, int>> syncChannelMap;
	std::map<int, std::map<int, int>> syncOrderMap;

	std::map<int, std::map<int, float>> fifoUsage;

	Array<int> channelMap; //Map from record channel index to source channel index
	Array<int> ftsChannelMap; // Map from recorded channel index to recorded source processor idx
	std::vector<std::vector<int>> subProcessorMap;
	std::vector<int> startRecChannels;

    bool isSyncReady;

    //TODO: Need to validate these new methods
    /** Deprecated*/
	void addInputChannel(const GenericProcessor* sourceNode, int chan);

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

	void registerProcessor(const GenericProcessor* sourceNode);

    /** Adds a Record Engine to use
    */
    void registerRecordEngine(RecordEngine *engine);

    /** Clears the list of active Record Engines
    */
    void clearRecordEngines();

	bool recordEvents;
	bool recordSpikes;

	ScopedPointer<EventMonitor> eventMonitor;

private:

	bool isConnectedToMessageCenter;
	Array<int64> msgCenterMessages;

	bool useSynchronizer; 

	bool receivedSoftwareTime;

	int lastDataChannelArraySize;

    bool isProcessing;
	bool isRecording;
	bool hasRecorded;
	bool settingsNeeded;
    bool shouldRecord;

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
	void handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition) override;

	virtual void handleTimestampSyncTexts(const MidiMessage& event);

    /**RecordEngines loaded**/
    OwnedArray<RecordEngine> engineArray;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordNode);

};

#endif
