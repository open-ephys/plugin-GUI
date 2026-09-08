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

#ifndef SYNCHRONIZER_H_INCLUDED
#define SYNCHRONIZER_H_INCLUDED

#include <algorithm>
#include <chrono>
#include <map>
#include <math.h>
#include <memory>
#include <array>
#include <vector>

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Utils/Utils.h"

class Synchronizer;
class HarpDecoder;

/** Total bits in Harp barcode */
static constexpr int TOTAL_BITS = 39;

/** 

	Represents a single sync pulse

*/
struct SyncPulse
{
    /** The time (in seconds) since the start of acquisition
        for the pulse's stream
        */
    double localTimestamp = 0.0f;

    /** The sample number at which this event occurred */
    int64 localSampleNumber = 0;

    /** The computer clock time at which this event was received 
        by the synchronizer */
    int64 computerTimeMillis;

    /** Whether the whole pulse has completed (on/off sequence) */
    bool complete = false;

    /** Duration of the event in seconds */
    double duration = -1.0;

    /** Time between the start of this event and the start of the last event */
    double interval = -1.0;

    /** Index of matching pulse in main stream */
    int matchingPulseIndex = -1;

    /** Global timestamp of pulse (if known) */
    double globalTimestamp = 0.0;
};

/**

    Represents a Harp barcode containing encoded timestamp

*/
struct HarpBarcode
{
    /** The decoded 32-bit timestamp in seconds */
    uint32_t encodedTime = 0;
    
    /** Stream time when start bit occurred */
    double localStartTimestamp = 0.0;
    
    /** Sample number when start bit occurred */
    int64 localStartSample = 0;
    
    /** System time when barcode was received */
    int64 computerTimeMillis = 0;
    
    /** Complete Harp timestamp bit sequence */
    std::array<bool, TOTAL_BITS> bitSequence {};
    
    /** Whether barcode is complete */
    bool isComplete = false;
    
    /** Whether barcode passed validation */
    bool isValid = false;
    
    /** Measured barcode duration in ms */
    double actualDuration = 0.0;
    
    /** Individual bit durations for validation */
    std::array<double, 43> bitDurations {};

    // Raw event sample numbers and states
    std::vector<std::pair<int64, bool>> barcodeEvents;
};

/**

    Harp detection state machine states

*/
enum class HarpDetectionState
{
    IDLE,                    // Waiting for potential start bit
    START_BIT_DETECTED,      // Found HIGH→LOW transition
    COLLECTING_BITS,         // Reading barcode bits
    VALIDATING_BARCODE,      // Checking complete barcode
    HARP_CONFIRMED,          // Valid Harp stream detected
    HARP_FAILED             // Invalid/timeout, fallback to pulse sync
};

/**

    Decodes Harp barcodes from ON/OFF event sequences

*/
class HarpDecoder
{
public:

    /** Expected duration of each bit in milliseconds */
    static constexpr double EXPECTED_BIT_DURATION_MS = 1.0;
    
    /** Timing tolerance for bit detection */
    static constexpr double BIT_TOLERANCE_MS = 0.2;
    
    /** Expected total barcode duration */
    static constexpr double EXPECTED_BARCODE_DURATION_MS = 43.0;
    
    /** Timeout for start bit detection */
    static constexpr double START_BIT_TIMEOUT_MS = 1100.0;
    
    /** Maximum reasonable time difference from system time */
    static constexpr int MAX_REASONABLE_TIME_DIFF_S = 300; // 5 minutes
    
    /** Decodes a barcode from event sequence */
    bool decodeBarcode(HarpBarcode& barcode, double expectedSampleRate);
                      
    /** Extracts 32-bit timestamp from bit sequence */
    uint32_t extractTimestamp(const std::array<bool, TOTAL_BITS>& bits);
    
    /** Validates bit timing */
    bool validateBitTiming(const HarpBarcode& barcode);
    
    /** Validates barcode structure */
    bool validateBarcodeStructure (const std::array<bool, TOTAL_BITS>& bits);
};

/**
 *
 * Represents an incoming data stream
 *
 * */
class SyncStream
{
public:
    /** Constructor */
    SyncStream (String streamKey, float expectedSampleRate, Synchronizer* synchronizer, bool generatesTimestamps = false);

    /** Resets stream parameters before acquisition */
    void reset (String mainStreamKey);

    /** True if this is the main stream */
    bool isMainStream;

    /** Adds a sync event with a particular sample number and state*/
    void addEvent (int64 sampleNumber, bool state);

    /** Global start time of this stream */
    double globalStartTime;

    /** Returns time of latest sync pulse */
    double getLatestSyncTime();

    /** Returns difference between actual and expected sync times */
    double getSyncAccuracy();

    /** Synchronize this stream with another one */
    void syncWith (const SyncStream* mainStream);
    
    /** Synchronize this stream with Harp timestamps */
    void syncWithHarp ();

    /** Sets hardware timestamps (hardware-synced streams only) */
    void setHardwareTimestamp (int64 sampleNumber, double timestamp);

    /** Compares pulses; returns true if a match is found */
    bool comparePulses (const SyncPulse& pulse1, const SyncPulse& pulse2);

    /** Stated sample rate for this stream */
    double expectedSampleRate;

    /** Computed sample rate for this stream */
    double actualSampleRate;

    /** TTL line to use for synchronization */
    int syncLine;

    /** true if this stream is successfully synchronized */
    bool isSynchronized;

    /** Holds the unique key for this stream */
    String streamKey;

    /** true if the stream is in active use */
    bool isActive;

    /** true if the stream generates its own timestamps */
    bool generatesTimestamps = false;

    /** true if the synchronizer overrides hardware timestamps */
    bool overrideHardwareTimestamps = false;
    
    /** true if this stream is using Harp synchronization */
    bool isHarpStream = false;
    
    /** true if Harp detection is currently active */
    bool harpDetectionActive = true;

    /** Baseline matching barcode for Harp synchronization */
    HarpBarcode baselineMatchingBarcode;

    /** The sync pulses for this stream
    
    The latest pulse is added to the beginning of the vector
    Expired pulses are removed from the end
    */
	std::vector<SyncPulse> pulses;

    /** Baseline matching the global stream */
    SyncPulse baselineMatchingPulse;

    /** Next pulse to become the baseline matching pulse */
    SyncPulse nextMatchingPulse;

    /** Time interval (in seconds) for updating initial sample rate estimate */
    const float INITIAL_SAMPLE_RATE_UPDATE_INTERVAL_S = 5;

    /** Time interval (in seconds) for subsequent updating sample rate estimate */
    const float SAMPLE_RATE_UPDATE_INTERVAL_S = 10;

    /** Determines the maximum size of the sync pulse buffer */
    const int MAX_PULSES_IN_BUFFER = 10;

    /** Threshold for calling pulses synchronous */
    const int MAX_TIME_DIFFERENCE_MS = 85;

    /** Threshold of calling durations equal */
    const double MAX_DURATION_DIFFERENCE_MS = 2;

    /** Threshold of calling intervals equal */
    const double MAX_INTERVAL_DIFFERENCE_MS = 2;
    
    /** Minimum valid barcodes needed for Harp synchronization */
    static constexpr int MIN_VALID_BARCODES_FOR_HARP = 3;

    /** Decodes all pending completed barcodes in acquisition order */
    void attemptBarcodeDecoding();

private:

    int64 latestSyncSampleNumber = 0;
    double latestGlobalSyncTime = 0.0;
    int64 latestSyncMillis = -1;

    Synchronizer* synchronizer;
    
    // Harp detection members
    HarpDetectionState harpState = HarpDetectionState::IDLE;
    std::vector<HarpBarcode> completedBarcodes; // Pending decoding
    std::vector<HarpBarcode> validatedBarcodes; // Baseline and two most recent valid barcodes
    HarpBarcode currentBarcode;
    HarpDecoder harpDecoder;
    
    int64 barcodeStartTime = -1; // System time when current barcode collection started
    int64 lastEventSample = -1;
    bool lastEventState = true; // Default HIGH
   
    
    // Detection state
    int consecutiveValidBarcodes = 0;
    int64 expectedNextStartSample = -1;
    
    // Harp-specific methods
    void processHarpEvent(int64 sampleNumber, bool state);
    void collectBarcodeEvent(int64 sampleNumber, bool state);
    bool validateBarcodeStructure(const HarpBarcode& barcode);
    bool validateBarcodeTimestamp(const HarpBarcode& barcode);
    void predictNextBarcodeStart(const HarpBarcode& barcode);
    

};

enum PLUGIN_API SyncStatus
{
    OFF, //Synchronizer is not running
    SYNCING, //Synchronizer is attempting to sync
    SYNCED, //Stream has been synchronized
    HARDWARE_SYNCED, //Stream has been synchronized by hardware
    HARP_DETECTING, //Analyzing for Harp barcodes
    HARP_CLOCK // Stream is using HARP clock
};

/**

    Uses events across multiple streams
    to synchronize data to a common clock.

    One incoming stream is set as the "main" stream,
    which is used to determine the clock start time.
    The main stream's sample rate is treated as ground
    truth, and all other streams clocks are scaled
    to align with this one.

    The synchronizer works best when the sync line
    has a TTL pulse with a relatively slow inter-event
    interval (e.g. 1 Hz). This interval does not have
    to be regular, however.

*/
class PLUGIN_API Synchronizer : public HighResolutionTimer
{
public:
    /** Constructor*/
    Synchronizer();

    /** Destructor */
    ~Synchronizer() {}

    /** Converts an int64 sample number to a double timestamp */
    double convertSampleNumberToTimestamp (String streamKey, int64 sampleNumber);

    /** Converts a double timestamp to an int64 sample number */
    int64 convertTimestampToSampleNumber (String streamKey, double timestamp);

    /** Returns offset (relative start time) for stream in ms */
    double getStartTime (String streamKey);

    /** Get latest sync time */
    double getLastSyncEvent (String streamKey);

    /** Get the accuracy of synchronization (difference between expected and actual event time) */
    double getAccuracy (String streamKey);

    /** Resets all values when acquisition is re-started */
    void reset();

    /** Sets main stream ID to 0 and stream count to 0*/
    void prepareForUpdate();

    /** Adds a new data stream with an expected sample rate, a synchronization line, and a flag indicating whether it generates its own timestamps */
    void addDataStream (String streamKey, float expectedSampleRate, int synLine = 0, bool generatesTimestamps = false);

    /** Checks if there is only one stream */
    void finishedUpdate();

    /** Sets the ID of the main data stream */
    void setMainDataStream (String streamKey);

    /** Sets the TTL line to use for synchronization (0-based indexing)*/
    void setSyncLine (String streamKey, int line);

    /** Returns the TTL line to use for synchronization (0-based indexing)*/
    int getSyncLine (String streamKey);

    /** Returns true if a stream is synchronized */
    bool isStreamSynced (String streamKey);

    /** Returns true if the stream genrates its own timestamps and overriding hardware timestamps is disabled */
    bool streamGeneratesTimestamps (String streamKey);
    
    /** Returns true if the stream is using Harp synchronization */
    bool isHarpStream (String streamKey);

    /** Returns the status (OFF / SYNCING / SYNCED / HARP_DETECTING / HARP_CLOCK) of a given stream*/
    SyncStatus getStatus (String streamKey);

    /** Adds an event for a stream ID / line combination */
    void addEvent (String streamKey, int ttlLine, int64 sampleNumber, bool state);

    /** Updates the synchronizer for a hardware-synced stream */
    void setHardwareTimestamp (int64 sampleNumber, double timestamp, String streamKey);

    /** Signals start of acquisition */
    void startAcquisition();

    /** Signals start of acquisition */
    void stopAcquisition();

    String mainStreamKey = String();
    String previousMainStreamKey = String();

    /** Total number of streams*/
    int streamCount = 0;

private:
    int eventCount = 0;
    bool acquisitionIsActive = false;

    void hiResTimerCallback();

    CriticalSection synchronizerLock;

    std::map<String, SyncStream*> streams;
    OwnedArray<SyncStream> dataStreamObjects;

};

/**
 
    Abstract base class for Record Node and Event Translator
 
 */
class PLUGIN_API SynchronizingProcessor
{
public:
    /** Sets the main data stream to use for synchronization */
    void setMainDataStream (String streamKey);

    /** Returns true if a stream ID matches the one to use for synchronization*/
    bool isMainDataStream (String streamKey);

    /** Sets the TTL line to use for synchronization*/
    void setSyncLine (String streamKey, int line);

    /** Returns the TTL line to use for synchronization*/
    int getSyncLine (String streamKey);

    /** The synchronizer for this processor */
    Synchronizer synchronizer;
};

#endif //SYNCHRONIZER_H_INCLUDED
