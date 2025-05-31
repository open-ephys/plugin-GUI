/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef RECORDNODE_H_DEFINED
#define RECORDNODE_H_DEFINED

#include <algorithm>
#include <chrono>
#include <map>
#include <math.h>
#include <memory>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../TestableExport.h"
#include "../../Utils/Utils.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Synchronizer/Synchronizer.h"
#include "DataQueue.h"
#include "RecordNodeEditor.h"
#include "RecordThread.h"

#include "DiskMonitor/DiskSpaceChecker.h"

#define WRITE_BLOCK_LENGTH 1024
#define DATA_BUFFER_NBLOCKS 300
#define EVENT_BUFFER_NEVENTS 200000
#define SPIKE_BUFFER_NSPIKES 200000

#define NIDAQ_BIT_VOLTS 0.001221f
#define NPX_BIT_VOLTS 0.195f
#define MAX_BUFFER_SIZE 40960
#define CHANNELS_PER_THREAD 384

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
class TESTABLE RecordNode : public GenericProcessor,
                            public SynchronizingProcessor,
                            public Timer
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

    /** Register parameters */
    void registerParameters() override;

    /** Initialize */
    void initialize (bool signalChainIsLoading) override;

    /** Respond to parameter value changes */
    void parameterValueChanged (Parameter* p) override;

    /** Allow configuration via OpenEphysHttpServer */
    String handleConfigMessage (const String& msg) override;

    /** Writes TEXT messages sent from the MessageCenter to disk */
    void handleBroadcastMessage (const String& msg, const int64 messageSystemTime) override;

    /** Update DataQueue block size when Audio Settings buffer size changes */
    void updateBlockSize (int newBlockSize);

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
    void createNewDirectory (bool resetCounters = false);

    /* Generates a date string to be used in the directory name*/
    String generateDateString() const;

    /* Returns the "experiment" count (number of times that acquisition was stopped and re-started)*/
    int getExperimentNumber() const;

    /* Returns the "recording" count (number of times that recording was stopped and re-started)*/
    int getRecordingNumber() const;

    /** Copies incoming data to the record buffer, if recording is active*/
    void process (AudioBuffer<float>& buffer) override;

    /** Returns a vector of available record engines*/
    std::vector<RecordEngineManager*> getAvailableRecordEngines();

    /** Gets the engine ID for this record node*/
    String getEngineId();

    /** Sets the engine ID for this record node */
    void setEngine (String engineId);

    /** Turns event recording on or off*/
    void setRecordEvents (bool);

    /** Turns spike recording on or off*/
    void setRecordSpikes (bool);

    /** Sets the parent directory for this Record Node (can be different from default directory) */
    void setDataDirectory (File);

    /** Sets the root folder for this Record Node (can be different from default directory) */
    void setDefaultRecordingDirectory (File);

    /** Returns the parent directory for this Record Node (can be different from default directory) */
    File getDataDirectory();

    /** Checks if the current recording directory has sufficient space to record */
    void checkDiskSpace();

    /** Returns true if this Record Node is writing data*/
    bool getRecordingStatus() const;

    /** Called by handleEvent() */
    void writeSpike (const Spike* spike, const SpikeChannel* spikeElectrode);

    /** Called by the ControlPanel to determine the amount of space
      left in the current dataDirectory.
  */
    float getFreeSpace() const;

    /** Called by CoreServices to determine the amount of space
		in kilobytes in the current dataDirectory.
	*/
    float getFreeSpaceKilobytes() const;

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

    Array<uint16> activeStreamIds;

    std::map<uint16, float> fifoUsage;

    ScopedPointer<EventMonitor> eventMonitor;

    std::unique_ptr<DiskSpaceChecker> diskSpaceChecker;

    Array<int> channelMap; //Map from record channel index to source channel index
    Array<int> localChannelMap; // Map from record channel index to recorded index within stream
    Array<int> timestampChannelMap; // Map from recorded channel index to recorded source processor idx

    bool isSyncReady;

    OwnedArray<RecordEngine> previousEngines;

    const int getEventChannelIndex (EventChannel*);
    const int getSpikeChannelIndex (SpikeChannel*);

    /** Save parameters*/
    void saveCustomParametersToXml (XmlElement* xml);

    /** Load parameters*/
    void loadCustomParametersFromXml (XmlElement* xml);

    DiskSpaceChecker* getDiskSpaceChecker() { return diskSpaceChecker.get(); }

    /** Used to update sync monitors */
    void timerCallback() override;

    /** Actual sync monitor update -- can be called independently of timer*/
    void updateSyncMonitors();

    /** Static flag to ensure override timestamps warning 
     * for hardware-synced streams is shown only once per run */
    static bool overrideTimestampWarningShown;

private:
    /** Handles other types of events (text, sync texts, etc.) */
    void handleEvent (const EventChannel* channel, const EventPacket& eventPacket);

    /** Forwards TTL events to the EventQueue */
    void handleTTLEvent (TTLEventPtr event) override;

    /** Writes incoming spikes to disk */
    void handleSpike (SpikePtr spike) override;

    /** Handles incoming timestamp sync messages */
    virtual void handleTimestampSyncTexts (const EventPacket& packet);

    /**RecordEngines loaded**/
    OwnedArray<RecordEngine> engineArray;

    bool isProcessing;
    bool isRecording;
    bool hasRecorded;
    bool settingsNeeded;
    bool shouldRecord;

    File defaultRecordDirectory; // Default directory for saving data (set by the ControlPanel)
    File dataDirectory;
    File rootFolder;

    int engineIndex;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordNode);
};

#endif
