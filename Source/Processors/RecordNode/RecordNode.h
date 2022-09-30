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
#define EVENT_BUFFER_NEVENTS	200000
#define SPIKE_BUFFER_NSPIKES	200000

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

	/** Reset counts */
	void reset();

	/* Counts the total number of events received. */
	int receivedEvents;

	/* Counts the total number of spikes received */
	int receivedSpikes;

	/* Counts the total of number of events sent to the recording buffer */
	int bufferedEvents;

	/* Counts the total of number of events sent to the recording buffer */
	int bufferedSpikes;

};

/**
	A specialized processor that saves data from the signal chain

	Sends data to RecordEngines, which handle the file creation / disk writing

	@see: RecordThread, RecordEngine
*/
class RecordNode :
    public GenericProcessor,
    public SynchronizingProcessor,
    public FilenameComponentListener
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

	/** Allow configuration via OpenEphysHttpServer */
	String handleConfigMessage(String msg) override;

	/** Writes TEXT messages sent from the MessageCenter to disk */
	void handleBroadcastMessage(String msg) override;

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

	/** Updates the channels to record for a given stream */
	void updateChannelStates(uint16 streamId, std::vector<bool> enabled);

	/** Copies incoming data to the record buffer, if recording is active*/
	void process(AudioBuffer<float>& buffer) override;

	/** Returns a vector of available record engines*/
	std::vector<RecordEngineManager*> getAvailableRecordEngines();

	/** Gets the engine ID for this record node*/
	String getEngineId();

	/** Sets the engine ID for this record node */
	void setEngine(String engineId);

	/** Turns event recording on or off*/
	void setRecordEvents(bool);

	/** Turns spike recording on or off*/
	void setRecordSpikes(bool);

	/** Sets the parent directory for this Record Node (can be different from default directory) */
	void setDataDirectory(File);

	/** Returns the parent directory for this Record Node (can be different from default directory) */
	File getDataDirectory();

	/** Checks if the current recording directory has sufficient space to record */
	void checkDiskSpace();

	/** Returns true if this Record Node is writing data*/
	bool getRecordingStatus() const;

	/** Get the last settings.xml in string form. Since the string will be large, returns a const ref.*/
	const String &getLastSettingsXml() const;

  /** Called by handleEvent() */
  void writeSpike(const Spike *spike, const SpikeChannel *spikeElectrode);

  /** Called by the ControlPanel to determine the amount of space
      left in the current dataDirectory.
  */
  float getFreeSpace() const;

   /** Called by CoreServices to determine the amount of space
		in kilobytes in the current dataDirectory.
	*/
  float getFreeSpaceKilobytes() const;

  /** Adds a Record Engine to use */
  void registerRecordEngine(RecordEngine *engine);

  /** Clears the list of active Record Engines*/
  void clearRecordEngines();
    
    /** Returns true if all streams within this Record Node are synchronized*/
    bool isSynchronized();
    
    /** Returns the number of data streams with recorded continuous channels*/
    int getTotalRecordedStreams();

  /** Variables to track whether or not particular channels are recorded*/
	bool recordEvents;
	bool recordSpikes;
	std::map<uint16, std::vector<bool>> recordContinuousChannels;

	bool newDirectoryNeeded;

    std::unique_ptr<RecordThread> recordThread;
	std::unique_ptr<RecordEngine> recordEngine;
	std::vector<RecordEngineManager*> availableEngines;

	int64 samplesWritten;
	String lastSettingsText;

	int numDataStreams;

	Array<uint16> activeStreamIds;

	std::map<uint16, float> fifoUsage;

	ScopedPointer<EventMonitor> eventMonitor;

	Array<int> channelMap; //Map from record channel index to source channel index
    Array<int> localChannelMap; // Map from record channel index to recorded index within stream
	Array<int> timestampChannelMap; // Map from recorded channel index to recorded source processor idx

	bool isSyncReady;
    
    OwnedArray<RecordEngine> previousEngines;

	const int getEventChannelIndex(EventChannel*);
	const int getSpikeChannelIndex(SpikeChannel*);
    
    /** Save parameters*/
    void saveCustomParametersToXml(XmlElement* xml);
    
    /** Load parameters*/
    void loadCustomParametersFromXml(XmlElement* xml);


private:
    
	/** Handles other types of events (text, sync texts, etc.) */
	void handleEvent(const EventChannel* channel, const EventPacket& eventPacket);

	/** Forwards TTL events to the EventQueue */
	void handleTTLEvent(TTLEventPtr event) override;

	/** Writes incoming spikes to disk */
	void handleSpike(SpikePtr spike) override;

	/** Handles incoming timestamp sync messages */
	virtual void handleTimestampSyncTexts(const EventPacket& packet);

	/**RecordEngines loaded**/
	OwnedArray<RecordEngine> engineArray;

    bool isProcessing;
	bool isRecording;
	bool hasRecorded;
	bool settingsNeeded;
    bool shouldRecord;

	File dataDirectory;
	File rootFolder;

	int experimentNumber;
	int recordingNumber;

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
