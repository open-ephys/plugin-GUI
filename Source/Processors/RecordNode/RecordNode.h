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

#define WRITE_BLOCK_LENGTH		1024
#define DATA_BUFFER_NBLOCKS		300
#define EVENT_BUFFER_NEVENTS	512
#define SPIKE_BUFFER_NSPIKES	512

#define NIDAQ_BIT_VOLTS			0.001221f
#define NPX_BIT_VOLTS			0.195f
#define MAX_BUFFER_SIZE			40960
#define CHANNELS_PER_THREAD		384


/**
	Class used internally by the RecordNode to count the number of incoming events
	Primarily useful for debugging purposes
*/
class EventMonitor
{
public:

	/* Constructor */
	EventMonitor();

	/* Destructor */
	~EventMonitor();

	/* Print information about incoming events.*/
	void displayStatus();

	/* Counts the total number of events received. */
	int receivedEvents;

};

/**
	A specialized processor that saves data from the signal chain

	Sends data to RecordEngines, which handle the file creation / disk writing

	@see: RecordThread, RecordEngine
*/
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
    
    /** Destructor */
	~RecordNode();

	/** Update DataQueue block size when Audio Settings buffer size changes */
	void updateBlockSize(int newBlockSize);

	/** Creates a custom editor */
	AudioProcessorEditor* createEditor() override;

	/* Updates the RecordNode settings*/
	void updateSettings() override;

	/* Called at start of acquisition; configures the associated RecordEngine*/
	bool startAcquisition() override;

	/* Called at end of acquisition */
	bool stopAcquisition() override;

	/* Called at start of recording; launches the RecordThread*/
	void startRecording() override;

	/* Called at end of recording; stops the RecordThread*/
	void stopRecording() override;

	/* Generates the name for the new recording directory*/
	String generateDirectoryName();

	/* Creates a new recording directory*/
	void createNewDirectory();

	/* Callback for responding to changes in data-directory-related settings*/
	void filenameComponentChanged(FilenameComponent*);

	/* Generates a date string to be used in the directory name*/
	String generateDateString() const;

	/* Returns the "experiment" count (number of times that acquisition was stopped and re-started)*/
	int getExperimentNumber() const;

	/* Returns the "recording" count (number of times that recording was stopped and re-started)*/
	int getRecordingNumber() const;

	void setPrimaryDataStream(uint16 streamId);
	bool isPrimaryDataStream(uint16 streamId);

	void setSyncBit(uint16 streamId, int bit);
	int getSyncBit(uint16 streamId);

	void updateChannelStates(uint64 satreamId, std::vector<bool> enabled);

	bool isFirstChannelInRecordedSubprocessor(int channel);

	void process(AudioBuffer<float>& buffer) override;

	void setParameter(int parameterIndex, float newValue) override;

	std::vector<RecordEngineManager*> getAvailableRecordEngines();

	void setEngine(int selectedEngineIndex);
	void setEngine(String engineId);
	void setRecordEvents(bool);
	void setRecordSpikes(bool);
	void setDataDirectory(File);
	File getDataDirectory();
	bool getRecordingStatus() const;

	/** Get the last settings.xml in string form. Since the string will be large, returns a const ref.*/
	const String &getLastSettingsXml() const;

    /** Must be called by a spike recording source on the "enable" method */
    void registerSpikeSource(const GenericProcessor *processor);

    /** Registers an electrode group for spike recording
    Must be called by a spike recording source on the "enable" method
    after the call to registerSpikeSource
    */
    int addSpikeElectrode(const SpikeChannel *elec);

    /** Called by a spike recording source to write a spike to file
    */
    void writeSpike(const Spike *spike, const SpikeChannel *spikeElectrode);

    /** Called by the ControlPanel to determine the amount of space
        left in the current dataDirectory.
    */
    float getFreeSpace() const;

	/** Called by CoreServices to determine the amount of space
		in kilobytes in the current dataDirectory.
	*/
	float getFreeSpaceKilobytes() const;

	void registerProcessor(const GenericProcessor* sourceNode);

    /** Adds a Record Engine to use */
    void registerRecordEngine(RecordEngine *engine);

    /** Clears the list of active Record Engines*/
    void clearRecordEngines();

	bool recordEvents;
	bool recordSpikes;

	bool newDirectoryNeeded;

	ScopedPointer<RecordThread> recordThread;
	ScopedPointer<RecordEngine> recordEngine;
	std::vector<RecordEngineManager*> availableEngines;

	ScopedPointer<Synchronizer> synchronizer;

	int64 samplesWritten;
	String lastSettingsText;

	int numDataStreams;

	std::vector<uint64> activeStreamIds;
	std::map<int, std::vector<bool>> dataChannelStates;
	std::map<int, int> dataChannelOrder;

	std::map<int, int> syncChannelMap;
	std::map<int, int> syncOrderMap;

	std::map<int, float> fifoUsage;

	ScopedPointer<EventMonitor> eventMonitor;

	Array<int> channelMap; //Map from record channel index to source channel index
	Array<int> ftsChannelMap; // Map from recorded channel index to recorded source processor idx
	std::vector<std::vector<int>> subProcessorMap;
	std::vector<int> startRecChannels;

	bool isSyncReady;

private:

	/** Forwards events to the EventQueue */
	void handleEvent(const EventChannel* eventInfo, const EventPacket& packet, int samplePosition) override;

	/** Writes incoming spikes to disk */
	void handleSpike(const SpikeChannel* spikeInfo, const EventPacket& packet, int samplePosition) override;

	/** Handles incoming timestamp sync messages */
	virtual void handleTimestampSyncTexts(const EventPacket& packet);

	/**RecordEngines loaded**/
	OwnedArray<RecordEngine> engineArray;

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

	std::unique_ptr<DataQueue> dataQueue;
	std::unique_ptr<EventMsgQueue> eventQueue;
    std::unique_ptr<SpikeMsgQueue> spikeQueue;

    int spikeElectrodeIndex;

    Array<bool> validBlocks;
	std::atomic<bool> setFirstBlock;

	//Profiling data structures
	float scaleFactor;
	HeapBlock<float> scaledBuffer;  
	HeapBlock<int16> intBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RecordNode);

};

#endif
