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

#include "Synchronizer.h"
#include <cmath>
#include <algorithm>

// =======================================================
// HarpDecoder Implementation
// =======================================================

bool HarpDecoder::decodeBarcode(HarpBarcode& barcode, double expectedSampleRate)
{
    barcode.isValid = false;
    barcode.isComplete = false;
    barcode.encodedTime = 0;

    if (barcode.barcodeEvents.size() < 2
        || !std::isfinite (expectedSampleRate) || expectedSampleRate <= 0.0)
        return false;
    
    // Convert events to bit sequence
    std::array<bool, TOTAL_BITS> bits;
    bits.fill(false);
    
    int bitIndex = 0;
    for (size_t i = 0; i < barcode.barcodeEvents.size() - 1 && bitIndex < TOTAL_BITS; i++)
    {
        int64 duration = barcode.barcodeEvents[i + 1].first - barcode.barcodeEvents[i].first;
        double durationMs = (double(duration) / expectedSampleRate) * 1000.0;

        if (duration <= 0 || !std::isfinite (durationMs)
            || durationMs > EXPECTED_BARCODE_DURATION_MS + 5.0)
            return false;
        
        // Determine number of bits based on duration
        int numBits = std::round(durationMs / EXPECTED_BIT_DURATION_MS);
        numBits = std::max(1, std::min(numBits, TOTAL_BITS - bitIndex));
        
        // Fill bits with current state
        bool currentState = barcode.barcodeEvents[i].second;
        for (int j = 0; j < numBits && bitIndex < TOTAL_BITS; j++)
        {
            bits[bitIndex] = currentState;
            barcode.bitDurations[bitIndex] = durationMs / numBits;
            bitIndex++;
        }
    }

    //for (auto bit : bits)
    //    std::cout << bit ? 1 : 0;
    //std::cout << std::endl;

    //LOGD ("Bits decoded: ", bitIndex);
    
    if (bitIndex < TOTAL_BITS) return false;
    
    barcode.bitSequence = bits;
    barcode.actualDuration = double (barcode.barcodeEvents.back().first - barcode.barcodeEvents[0].first) / expectedSampleRate * 1000.0;
    
    // Validate structure
    if (!validateBarcodeStructure(bits)) return false;
    
    // Validate timing
    if (!validateBitTiming(barcode)) return false;
    
    // Extract timestamp
    barcode.encodedTime = extractTimestamp(bits);
    barcode.isComplete = true;
    barcode.isValid = true;

    //LOGD ("Found Harp barcode: ", barcode.encodedTime, " at sample ", barcode.localStartSample);
    
    return true;
}

uint32_t HarpDecoder::extractTimestamp(const std::array<bool, TOTAL_BITS>& bits)
{
    uint32_t timestamp = 0;
    
    // Extract 4 bytes, each with LSB first
    std::array<int, 4> byteStartPositions = {1, 11, 21, 31};
    
    for (int byteIdx = 0; byteIdx < 4; byteIdx++)
    {
        uint8_t byteValue = 0;
        int startPos = byteStartPositions[byteIdx];
        
        // Read 8 bits, LSB first
        for (int bitIdx = 0; bitIdx < 8; bitIdx++)
        {
            if (bits[startPos + bitIdx])
            {
                byteValue |= (1 << bitIdx);
            }
        }
        
        timestamp |= (uint32_t(byteValue) << (byteIdx * 8));
    }
    
    return timestamp;
}

bool HarpDecoder::validateBitTiming(const HarpBarcode& barcode)
{
    //LOGD ("Validating bit timing.");
    // Check overall duration
    if (std::abs(barcode.actualDuration - EXPECTED_BARCODE_DURATION_MS) > 5.0)
    {
        //LOGD ("Overall duration more than 5 ms off.");
        return false;
    }
        
        
    // Check individual bit durations
    for (int i = 0; i < TOTAL_BITS; i++)
    {
        if (std::abs(barcode.bitDurations[i] - EXPECTED_BIT_DURATION_MS) > BIT_TOLERANCE_MS)
        {
           // LOGD ("Failed tolerance for bit ", i);
            return false;
        }
    }

   // LOGD ("OK.");
    
    return true;
}

bool HarpDecoder::validateBarcodeStructure(const std::array<bool, TOTAL_BITS>& bits)
{
    //LOGD ("Validating barcode structure.");
    // Check start bit (should be LOW)
    if (bits[0] != false) return false;
    
    // Check transition bits at positions 9,10 19,20 29,30
    std::array<int, 6> transitionPositions = {9, 10, 19, 20, 29, 30};
    
    for (int i = 0; i < transitionPositions.size(); i += 2)
    {
        // Transition bits should be different (HIGH/LOW pattern)
        if (bits[transitionPositions[i]] == bits[transitionPositions[i + 1]])
            return false;
    }

    //LOGD ("OK.");
    
    return true;
}

// =======================================================
// SyncStream Implementation
// =======================================================

SyncStream::SyncStream (String streamKey_, float expectedSampleRate_, Synchronizer* synchronizer_, bool generatesTimestamps_)
    : streamKey (streamKey_), synchronizer(synchronizer_),
      expectedSampleRate (expectedSampleRate_),
      actualSampleRate (-1.0f),
      isActive (true),
      generatesTimestamps (generatesTimestamps_),
      syncLine (0),
      isSynchronized (false)
{
    LOGD ("Created sync stream ", streamKey, " with sample rate ", expectedSampleRate);
}

void SyncStream::reset (String mainStreamKey)
{
    isMainStream = (streamKey == mainStreamKey);

    pulses.clear();
    baselineMatchingPulse = SyncPulse();
    nextMatchingPulse = SyncPulse();

    latestSyncSampleNumber = 0;
    latestGlobalSyncTime = 0.0;
    latestSyncMillis = -1;

    // Reset Harp detection state
    harpState = HarpDetectionState::IDLE;
    completedBarcodes.clear();
    validatedBarcodes.clear();
    currentBarcode = HarpBarcode();
    barcodeStartTime = -1;
    lastEventSample = -1;
    lastEventState = true;
    consecutiveValidBarcodes = 0;
    expectedNextStartSample = -1;
    isHarpStream = false;
    harpDetectionActive = true;
    baselineMatchingBarcode = HarpBarcode();

    if (isMainStream)
    {
        actualSampleRate = expectedSampleRate;
        globalStartTime = 0.0;
        isSynchronized = true;
        overrideHardwareTimestamps = true; // main stream overrides hardware timestamps
    }
    else
    {
        overrideHardwareTimestamps = syncLine > -1; // override hardware timestamps for other streams if sync line is set

        if (generatesTimestamps && ! overrideHardwareTimestamps)
        {
            // if the stream generates its own timestamps, it is synchronized unless it overrides hardware timestamps
            actualSampleRate = expectedSampleRate;
            isSynchronized = true;
        }
        else
        {
            actualSampleRate = -1.0;
            isSynchronized = false;
        }

        globalStartTime = 0.0;
    }
}

void SyncStream::addEvent (int64 sampleNumber, bool state)
{
    //LOGD ("[+] Adding event for stream ", streamKey, " (", sampleNumber, ")");
    
    // Harp barcode detection (runs in parallel)
    if (harpDetectionActive)
    {
        processHarpEvent(sampleNumber, state);
    }

    if (state) // on event received, pulse initiated
    {
        SyncPulse latestPulse;
        latestPulse.localSampleNumber = sampleNumber;
        latestPulse.localTimestamp = double(sampleNumber) / expectedSampleRate;
        latestPulse.computerTimeMillis = Time::currentTimeMillis();

        pulses.insert (pulses.begin(), latestPulse);
    }
    else // off event received, pulse terminated
    {
        if (pulses.size() > 0)
        {
            SyncPulse& latestPulse = pulses.front();

            latestPulse.complete = true;
            latestPulse.duration =
               double(sampleNumber) / expectedSampleRate - latestPulse.localTimestamp;

            if (pulses.size() > 1)
            {
                latestPulse.interval = latestPulse.localTimestamp - pulses[1].localTimestamp;
            }
        }

        if (pulses.size() > MAX_PULSES_IN_BUFFER)
        {
            pulses.pop_back();
        }
    }

}

void SyncStream::setHardwareTimestamp (int64 sampleNumber, double timestamp)
{

    if (generatesTimestamps && ! overrideHardwareTimestamps)
    {
        latestSyncSampleNumber = sampleNumber;
        latestGlobalSyncTime = timestamp;

        if (! baselineMatchingPulse.complete)
        {
            baselineMatchingPulse.localSampleNumber = sampleNumber;
            baselineMatchingPulse.globalTimestamp = timestamp;
            baselineMatchingPulse.complete = true;

            actualSampleRate = expectedSampleRate; // safe initial
            globalStartTime = timestamp - (double (sampleNumber) / expectedSampleRate);
            latestSyncMillis = Time::currentTimeMillis();
            return;
        }

        double dt = timestamp - baselineMatchingPulse.globalTimestamp;
        int64 ds = sampleNumber - baselineMatchingPulse.localSampleNumber;

        if (dt <= 1e-6 || ds <= 0)
            return;

        double estimated = double (ds) / dt;

        if (std::abs (estimated - expectedSampleRate) / expectedSampleRate < 0.01)
        {
            actualSampleRate = estimated;
            latestSyncMillis = Time::currentTimeMillis();
        }
    }

}

double SyncStream::getLatestSyncTime()
{
    //LOGC ("Getting latest sync time for stream ", streamKey, "...");
    //LOGC ("Time::currentTimeMillis(): ", Time::currentTimeMillis());
    //LOGC ("latestSyncMillis: ", latestSyncMillis);
   

    if (latestSyncMillis != -1)
    {
       // LOGD ("Returning: ", double (Time::currentTimeMillis() - latestSyncMillis) / 1000.0f);
        return double (Time::currentTimeMillis() - latestSyncMillis) / 1000.0f;
	}
    else
    {
       // LOGD ("Returning: ", -1);
		return -1.0;
	}
}

double SyncStream::getSyncAccuracy()
{

    if (generatesTimestamps && ! overrideHardwareTimestamps)
        return 0.0; // or compute error vs. expectedSampleRate drift

    if (pulses.size() > 0)
    {

        //if (!isMainStream)
        //{
            //LOGC ("Sync accuracy for stream ", streamKey);

            //LOGC ("latestSyncSampleNumber: ", latestSyncSampleNumber);
            //LOGC ("latestGlobalSyncTime: ", latestGlobalSyncTime);
            //LOGC ("globalStartTime: ", globalStartTime);
            //LOGC ("actualSampleRate: ", actualSampleRate);
            //LOGC ("baselineMatchingPulse.globalTimestamp: ", baselineMatchingPulse.globalTimestamp);
            //LOGC ("baselineMatchingPulse.localSampleNumber: ", baselineMatchingPulse.localSampleNumber);
            //LOGC (" ");
        //}


        // NEW CALCULATION:
        double estimatedGlobalTime = double(latestSyncSampleNumber - baselineMatchingPulse.localSampleNumber)
                                         / actualSampleRate
                                     + (baselineMatchingPulse.globalTimestamp);
        //LOGD ("estimatedGlobalTime: ", estimatedGlobalTime);
        //LOGD ("difference: ", latestGlobalSyncTime - estimatedGlobalTime);

        return (estimatedGlobalTime - latestGlobalSyncTime) * 1000;
    }
    else
    {
        return 0.0;
    }
}

void SyncStream::syncWith (const SyncStream* mainStream)
{
    //LOGC ("Synchronizing ", streamKey, " with ", mainStream->streamKey, "...");
    //LOGC ("Expected sample rate: ", expectedSampleRate);

    if (mainStream->pulses.size() < 2 || pulses.size() < 2)
    {
        //LOGC ("Not enough pulses to synchronize.");
        return;
    }

    int localIndex = 0;
    bool foundMatchingPulse = false;

    for (auto& pulse : pulses) // loop through pulses in this stream
    {
        if (pulse.complete)
        {
            int index = 0;

            for (auto& mainPulse : mainStream->pulses) // loop through pulses in main stream
            {
                if (mainPulse.complete)
                {
                    if (comparePulses (pulse, mainPulse)) // putative match
                    {
                        if (pulses.size() > localIndex + 2 && mainStream->pulses.size() > index + 2)
                        {
                            // previous two pulses also match
                            if (comparePulses (pulses[localIndex + 1], mainStream->pulses[index + 1])
                                && comparePulses (pulses[localIndex + 2], mainStream->pulses[index + 2]))
                            {
                                pulse.matchingPulseIndex = index;
                                pulse.globalTimestamp = mainPulse.localTimestamp;
                                latestSyncSampleNumber = pulse.localSampleNumber;
                                latestGlobalSyncTime = pulse.globalTimestamp;
                                latestSyncMillis = pulse.computerTimeMillis;
                                //LOGC ("Pulse at ", pulse.localTimestamp, " matches with 3 main pulses at ", index);
                                //LOGC ("latestSyncSampleNumber: ", latestSyncSampleNumber, ", latestGlobalSyncTime: ", latestGlobalSyncTime);


                                if (baselineMatchingPulse.complete == false)
                                {
                                    baselineMatchingPulse.localTimestamp = pulse.localTimestamp;
                                    baselineMatchingPulse.globalTimestamp = mainPulse.localTimestamp;
                                    baselineMatchingPulse.localSampleNumber = pulse.localSampleNumber;
                                    baselineMatchingPulse.complete = true;
                                    //LOGC ("Time of first matching pulse: ", baselineMatchingPulse.localTimestamp, " (local), ", baselineMatchingPulse.globalTimestamp, " (global)");
                                }
                                else
                                {
                                    if (pulse.localTimestamp - baselineMatchingPulse.localTimestamp > INITIAL_SAMPLE_RATE_UPDATE_INTERVAL_S)
                                    {

                                        if (nextMatchingPulse.complete == false)
                                        {
											nextMatchingPulse.localTimestamp = pulse.localTimestamp;
											nextMatchingPulse.globalTimestamp = mainPulse.localTimestamp;
											nextMatchingPulse.localSampleNumber = pulse.localSampleNumber;
											nextMatchingPulse.complete = true;
										}
                                        else
                                        {
                                            if (pulse.localTimestamp - nextMatchingPulse.localTimestamp > SAMPLE_RATE_UPDATE_INTERVAL_S)
                                            {

                                                double estimatedActualSampleRate = double (nextMatchingPulse.localSampleNumber - baselineMatchingPulse.localSampleNumber)
                                                                                   / double (nextMatchingPulse.globalTimestamp - baselineMatchingPulse.globalTimestamp);

                                                if (std::abs (estimatedActualSampleRate - expectedSampleRate) / expectedSampleRate < 0.01)
                                                {
                                                    actualSampleRate = estimatedActualSampleRate;
                                                }

                                                //baselineMatchingPulse = nextMatchingPulse;

                                                //LOGD ("Time of new matching pulse: ",
                                                 //     nextMatchingPulse.localTimestamp,
                                                  //    " (local), ",
                                                  //    nextMatchingPulse.globalTimestamp,
                                                  //    " (global)");

                                                double estimatedPulseTime = synchronizer->convertSampleNumberToTimestamp (streamKey, nextMatchingPulse.localSampleNumber);
                                                //LOGD ("pulses[localIndex].estimatedPulseTime: ", estimatedPulseTime);

                                                nextMatchingPulse.localTimestamp = pulse.localTimestamp;
                                                nextMatchingPulse.globalTimestamp = mainPulse.localTimestamp;
                                                nextMatchingPulse.localSampleNumber = pulse.localSampleNumber;
                                            }
											
										}
                                        

                                    }
                                }
                                
                            }
                        }

                        break;
                    }
                }

                index++;
            }
        }

        if (pulse.matchingPulseIndex != -1)
        {
            foundMatchingPulse = true;
            break;
        }

        localIndex++;
    }

    if (foundMatchingPulse)
    {
        if (baselineMatchingPulse.complete && (pulses[localIndex].localTimestamp - baselineMatchingPulse.localTimestamp) > 1.0)
        {
            //LOGD ("pulses[localIndex].localSampleNumber: ", pulses[localIndex].localSampleNumber, ", baselineMatchingPulse.localSampleNumber: ", baselineMatchingPulse.localSampleNumber);
            //LOGD ("pulses[localIndex].localTimestamp: ", pulses[localIndex].localTimestamp, ", baselineMatchingPulse.localTimestamp: ", baselineMatchingPulse.localTimestamp);
            //LOGD ("pulses[localIndex].globalTimestamp: ", pulses[localIndex].globalTimestamp, ", baselineMatchingPulse.globalTimestamp: ", baselineMatchingPulse.globalTimestamp);

            double estimatedActualSampleRate = double(pulses[localIndex].localSampleNumber - baselineMatchingPulse.localSampleNumber)
                / double(pulses[localIndex].globalTimestamp - baselineMatchingPulse.globalTimestamp);

            //LOGC (streamKey, " actualSampleRate: ", actualSampleRate, ", estimatedActualSampleRate: ", estimatedActualSampleRate);

            double estimatedGlobalStartTime = pulses[localIndex].globalTimestamp
                - double(pulses[localIndex].localSampleNumber) / estimatedActualSampleRate;

            double estimatedPulseTime = synchronizer->convertSampleNumberToTimestamp(streamKey, pulses[localIndex].localSampleNumber);
            //LOGC (streamKey, " estimatedPulseTime: ", estimatedPulseTime);
            //LOGC (streamKey, " diff from global time: ", estimatedPulseTime - pulses[localIndex].globalTimestamp);

            if (std::abs(estimatedActualSampleRate - expectedSampleRate) / expectedSampleRate < 0.05)
            {
                if (actualSampleRate == -1.0)
                {
                    actualSampleRate = estimatedActualSampleRate;

                    if (std::abs(estimatedGlobalStartTime) < 1.0)
                    {
                        if (!isSynchronized)

                        {
                            globalStartTime = estimatedGlobalStartTime;

                            isSynchronized = true;

                        }
                    }
                    else
                    {
                        //LOGD ("Estimated global start time of ", estimatedGlobalStartTime, " is out of bounds. Ignoring.");
                    }
                }

            }
            else
            {
                //LOGD ("Estimated sample rate of ", estimatedActualSampleRate, " is out of bounds. Expected sample rate = ", expectedSampleRate);
                return;
            }

            //LOGD ("Stream ", streamKey, " synchronized with main stream. Sample rate: ", actualSampleRate, ", start time: ", globalStartTime);
        }
        else
        {
            //LOGD ("At least 1 second must elapse before synchronization can be attempted.");
        }
    }


}

bool SyncStream::comparePulses(const SyncPulse& pulse1, const SyncPulse& pulse2)
{
    if (std::abs(pulse1.computerTimeMillis - pulse2.computerTimeMillis) < MAX_TIME_DIFFERENCE_MS)
    {
        if (std::abs(pulse1.duration - pulse2.duration) < MAX_DURATION_DIFFERENCE_MS)
        {
            if (std::abs(pulse1.interval - pulse2.interval) < MAX_INTERVAL_DIFFERENCE_MS)
            {
                return true;
            }
        }
    }

    return false;
}

void SyncStream::processHarpEvent(int64 sampleNumber, bool state)
{

    //LOGD ("currentBarcode.barcodeEvents.size: ", currentBarcode.barcodeEvents.size());

    // if more than 0.5 s has elapsed since last event, log previous barcode
    if (currentBarcode.barcodeEvents.size() > 0)
    {
        double timeSinceLastEvent = double (sampleNumber - currentBarcode.barcodeEvents.back().first) / expectedSampleRate;

        if (timeSinceLastEvent > 0.5)
        {
                //LOGD ("COMPLETED BARCODE.");
                completedBarcodes.push_back (currentBarcode);
                currentBarcode = {};
        }
    }

    //LOGD ("currentBarcode.localStartSample: ", (currentBarcode.localStartSample));

    if (currentBarcode.barcodeEvents.empty())
    {
        //LOGD ("STARTING NEW BARCODE COLLECTION.");
        currentBarcode.localStartSample = sampleNumber;
        currentBarcode.localStartTimestamp = double (sampleNumber) / expectedSampleRate;
        currentBarcode.computerTimeMillis = Time::currentTimeMillis();
    }

    currentBarcode.barcodeEvents.push_back (std::make_pair (sampleNumber, state));

}


void SyncStream::collectBarcodeEvent(int64 sampleNumber, bool state)
{

    
    
    // Timeout check
    /* if (! currentBarcodeEvents.empty())
    {
        LOGD (" Checking for timeout. currentBarcodeEvents[0].first=", currentBarcodeEvents[0].first, ", sampleNumber=", sampleNumber);
        double elapsedTime = (double(sampleNumber - currentBarcodeEvents[0].first) / expectedSampleRate) * 1000.0;
        if (elapsedTime > HarpDecoder::START_BIT_TIMEOUT_MS)
        {
            // Timeout - reset to IDLE
            harpState = HarpDetectionState::IDLE;
            currentBarcodeEvents.clear();
            barcodeStartTime = -1;
        }
    }*/
}


bool SyncStream::validateBarcodeStructure(const HarpBarcode& barcode)
{
    return harpDecoder.validateBarcodeStructure(barcode.bitSequence);
}

bool SyncStream::validateBarcodeTimestamp(const HarpBarcode& barcode)
{
    if (validatedBarcodes.empty())
        return true;

    const auto& previousBarcode = validatedBarcodes.back();
    // Widen before subtracting so backwards timestamps cannot wrap unsigned.
    const int64 timeDifference = int64 (barcode.encodedTime) - int64 (previousBarcode.encodedTime);
    const int64 sampleDifference = barcode.localStartSample - previousBarcode.localStartSample;

    if (timeDifference <= 0 || sampleDifference <= 0
        || !std::isfinite (expectedSampleRate) || expectedSampleRate <= 0.0)
        return false;

    // Missing or rejected barcodes are OK if elapsed Harp time agrees with
    // elapsed sample time. Do not require timestamps to differ by exactly one.
    const double estimatedSampleRate = double (sampleDifference) / double (timeDifference);
    return std::isfinite (estimatedSampleRate)
           && std::abs (estimatedSampleRate - expectedSampleRate) / expectedSampleRate < 0.05;
}

void SyncStream::predictNextBarcodeStart(const HarpBarcode& barcode)
{
    // Predict next barcode will start 1 second later
    expectedNextStartSample = barcode.localStartSample + int64(expectedSampleRate);
}

void SyncStream::attemptBarcodeDecoding()
{
    // Timer callbacks need not coincide with barcode arrivals. Process every
    // pending barcode once, rather than skipping directly to the newest one.
    for (auto& barcode : completedBarcodes)
    {
        if (!harpDecoder.decodeBarcode (barcode, expectedSampleRate))
            continue;

        if (!isHarpStream)
        {
            LOGD ("Successful decoding...setting isHarpStream to true.");
            isHarpStream = true;
            isSynchronized = false;

        }

        if (!validateBarcodeTimestamp (barcode))
        {
            LOGD (streamKey, " Harp timestamp/sample discontinuity; rebuilding synchronization history.");
            validatedBarcodes.clear();
            isSynchronized = false;
        }

        // Keep the first barcode as the long-term rate baseline, plus the two
        // newest barcodes. Raw event history must not grow for the whole run.
        validatedBarcodes.push_back (std::move (barcode));
        if (validatedBarcodes.size() > MIN_VALID_BARCODES_FOR_HARP)
        {
            validatedBarcodes.erase (validatedBarcodes.begin() + 1);
        }
    }
    completedBarcodes.clear();
}


void SyncStream::syncWithHarp()
{
    if (validatedBarcodes.size() < MIN_VALID_BARCODES_FOR_HARP)
    {
        return;
    }

    // Preserve the existing one-barcode lag, using only decoded entries.
    const HarpBarcode& lastBarcode = validatedBarcodes[validatedBarcodes.size() - 2];
    const HarpBarcode& firstBarcode = validatedBarcodes.front();

    const int64 timeDifference = int64 (lastBarcode.encodedTime) - int64 (firstBarcode.encodedTime);
    const int64 sampleDifference = lastBarcode.localStartSample - firstBarcode.localStartSample;

    if (!firstBarcode.isValid || !lastBarcode.isValid
        || timeDifference <= 0 || sampleDifference <= 0
        || !std::isfinite (expectedSampleRate) || expectedSampleRate <= 0.0)
    {
        LOGD (streamKey, " invalid Harp synchronization baseline; rebuilding synchronization history.");
        isSynchronized = false;
        validatedBarcodes.clear();
        return;
    }

    const double estimatedSampleRate = double (sampleDifference) / double (timeDifference);
    
    if (std::isfinite (estimatedSampleRate)
        && std::abs (estimatedSampleRate - expectedSampleRate) / expectedSampleRate < 0.05)
    {
        actualSampleRate = estimatedSampleRate;

        // Calculate global start time
        //LOGD ("Estimated global start time: ", double (firstBarcode.encodedTime) - firstBarcode.localStartTimestamp);
        globalStartTime = (double (firstBarcode.encodedTime) - firstBarcode.localStartTimestamp) / 1000; // divide by 1000 so that time appears in seconds in the stream info view
        baselineMatchingPulse.globalTimestamp = double (firstBarcode.encodedTime);
        baselineMatchingPulse.localSampleNumber = firstBarcode.localStartSample;
        latestSyncSampleNumber = lastBarcode.localStartSample;
        latestGlobalSyncTime = double(lastBarcode.encodedTime);

        isSynchronized = true;
        latestSyncMillis = Time::currentTimeMillis();

        //LOGD ("Harp stream ", streamKey, " synchronized. Sample rate: ", actualSampleRate, ", start time: ", globalStartTime);
    }
    else
    {
        LOGD (streamKey, " estimated sample rate out of range: ", estimatedSampleRate,
              "; expected: ", expectedSampleRate, "; rebuilding Harp synchronization history.");
        isSynchronized = false;
        validatedBarcodes.clear();
    }
    
}

// =======================================================

Synchronizer::Synchronizer()
{
}

void Synchronizer::reset()
{
    for (auto [id, stream] : streams)
        stream->reset (mainStreamKey);
}

void Synchronizer::prepareForUpdate()
{
    previousMainStreamKey = mainStreamKey;
    mainStreamKey = String();
    dataStreamObjects.clear();
    streams.clear();
    streamCount = 0;
}

void Synchronizer::finishedUpdate()
{
    if (mainStreamKey.isEmpty() && streamCount > 0)
    {
        // if no main stream is set, set the first non-hardware-synced stream as the main stream
        for (auto stream : dataStreamObjects)
        {
            if (! streamGeneratesTimestamps (stream->streamKey))
            {
                mainStreamKey = stream->streamKey;
                LOGD ("No main stream set, setting ", mainStreamKey, " as the main stream");
                break;
            }
        }
    }
    
    reset();
}

void Synchronizer::addDataStream (String streamKey, float expectedSampleRate, int syncLine, bool generatesTimestamps)
{
    LOGD ("Synchronizer adding ", streamKey, " with sample rate ", expectedSampleRate);

    //std::cout << "Main stream ID: " << mainStreamId << std::endl;

    // if there's a stored value, and it appears again,
    // re-instantiate this as the main stream
    if (streamKey == previousMainStreamKey)
        mainStreamKey = previousMainStreamKey;

    //std::cout << "Creating new Stream object" << std::endl;
    dataStreamObjects.add (new SyncStream (streamKey, expectedSampleRate, this, generatesTimestamps));
    streams[streamKey] = dataStreamObjects.getLast();
    streams[streamKey]->syncLine = syncLine;

    streamCount++;
}

void Synchronizer::setMainDataStream (String streamKey)
{
    if (streamKey.isNotEmpty() && streams.count (streamKey) == 0)
    {
        LOGD ("Cannot set ", streamKey, " as main data stream. Stream not found.");
        return;
    }
    LOGD ("Synchronizer setting mainDataStream to ", streamKey);
    mainStreamKey = streamKey;
    reset();
}

void Synchronizer::setSyncLine (String streamKey, int ttlLine)
{
    LOGD ("Synchronizer setting sync line for ", streamKey, " to ", ttlLine);

    streams[streamKey]->syncLine = ttlLine;

    if (streamKey == mainStreamKey)
        reset();
    else
        streams[streamKey]->reset (mainStreamKey);
}

int Synchronizer::getSyncLine (String streamKey)
{
    return streams[streamKey]->syncLine;
}

void Synchronizer::startAcquisition()
{
    reset();

    acquisitionIsActive = true;
    startTimer (1000);
}

void Synchronizer::stopAcquisition()
{
    acquisitionIsActive = false;

    stopTimer();
}

void Synchronizer::addEvent (String streamKey,
                             int ttlLine,
                             int64 sampleNumber,
                             bool state)
{

    //LOGD ("Synchronizer adding event for stream ", streamKey, " at sample ", sampleNumber, " state=", state, " ttlLine=", ttlLine);

    const ScopedLock sl (synchronizerLock);

    //if (streamCount == 1 || sampleNumber < 1000)
    //    return;

    if (streams[streamKey]->syncLine == ttlLine)
    {
        streams[streamKey]->addEvent (sampleNumber, state);
    }
}


void Synchronizer::setHardwareTimestamp (int64 sampleNumber, double timestamp, String streamKey)
{
    const ScopedLock sl (synchronizerLock);
    streams[streamKey]->setHardwareTimestamp (sampleNumber, timestamp);
}


double Synchronizer::convertSampleNumberToTimestamp (String streamKey, int64 sampleNumber)
{
    if (streams[streamKey]->isSynchronized)
    {

        // Use standard pulse baseline for conversion
        return double (sampleNumber - streams[streamKey]->baselineMatchingPulse.localSampleNumber)
                   / streams[streamKey]->actualSampleRate
               + streams[streamKey]->baselineMatchingPulse.globalTimestamp;
        ;
    }
    else
    {
        return (double) -1.0f;
    }
}

int64 Synchronizer::convertTimestampToSampleNumber (String streamKey, double timestamp)
{
    if (streams[streamKey]->isSynchronized)
    {

        // Use standard pulse baseline for conversion
        int64 t = int64 ((timestamp - streams[streamKey]->baselineMatchingPulse.globalTimestamp) 
            * streams[streamKey]->actualSampleRate)
                    + streams[streamKey]->baselineMatchingPulse.localSampleNumber;
        return t;
    }
    else
    {
        return -1;
    }
}

double Synchronizer::getStartTime (String streamKey)
{
	return streams[streamKey]->globalStartTime * 1000;
}

double Synchronizer::getLastSyncEvent (String streamKey)
{
    return streams[streamKey]->getLatestSyncTime();
}

double Synchronizer::getAccuracy (String streamKey)
{

    if(! streams[streamKey]->isSynchronized)
		return 0.0;
    else
    {
        if (streamKey == mainStreamKey && !streams[streamKey]->isHarpStream)
			return 0.0;
        else
        {
            return streams[streamKey]->getSyncAccuracy();
        }
        
    }

}

bool Synchronizer::isStreamSynced (String streamKey)
{
    return streams[streamKey]->isSynchronized;
}

bool Synchronizer::streamGeneratesTimestamps (String streamKey)
{
    if (streams.count (streamKey) == 0)
        return false;

    return streams[streamKey]->generatesTimestamps && ! streams[streamKey]->overrideHardwareTimestamps;
}

bool Synchronizer::isHarpStream (String streamKey)
{
    if (streams.count (streamKey) == 0)
        return false;
        
    return streams[streamKey]->isHarpStream;
}

SyncStatus Synchronizer::getStatus (String streamKey)
{

    if (streamGeneratesTimestamps (streamKey))
        return SyncStatus::HARDWARE_SYNCED;
   
    if (streams.count (streamKey) == 0 || ! streamKey.length() || ! acquisitionIsActive)
        return SyncStatus::OFF;

    if (streams[streamKey]->isHarpStream)
    {
        if (isStreamSynced (streamKey))
            return SyncStatus::HARP_CLOCK;
        else
            return SyncStatus::HARP_DETECTING;
    }

    if (isStreamSynced (streamKey))
        return SyncStatus::SYNCED;
    else
        return SyncStatus::SYNCING;
}

void Synchronizer::hiResTimerCallback()
{
    if (mainStreamKey.isEmpty())
        return;

    const ScopedLock sl (synchronizerLock);

    // First, attempt to decode any pending Harp barcodes
    for (auto [key, stream] : streams)
    {
        if (stream->harpDetectionActive)
        {
            stream->attemptBarcodeDecoding();
        }
    }
    
    bool mainStreamIsHarp = streams[mainStreamKey]->isHarpStream;

    // Then perform synchronization
    for (auto [key, stream] : streams)
    {
        if (key != mainStreamKey && ! streamGeneratesTimestamps (key))
        {
            if (stream->isHarpStream)
            {
                // Both streams use Harp - use Harp synchronization
                stream->syncWithHarp();
            }
            else
            {
                // Both streams use standard pulses - use standard synchronization
                stream->syncWith (streams[mainStreamKey]);
            }
            // Mixed mode: Harp + standard pulse streams cannot sync with each other
        }
        else
        {
            // Main stream gets synced with Harp if applicable
            if (key == mainStreamKey && stream->isHarpStream)
            {
                stream->syncWithHarp();
            }
        }
    }
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setMainDataStream (String streamKey)
{
    //LOGD("Setting ", streamId, " as the main stream");
    synchronizer.setMainDataStream (streamKey);
}

// called by RecordNodeEditor (when loading), SyncControlButton
void SynchronizingProcessor::setSyncLine (String streamKey, int line)
{
    synchronizer.setSyncLine (streamKey, line);
}

// called by SyncControlButton
int SynchronizingProcessor::getSyncLine (String streamKey)
{
    return synchronizer.getSyncLine (streamKey);
}

// called by SyncControlButton
bool SynchronizingProcessor::isMainDataStream (String streamKey)
{
    return (streamKey == synchronizer.mainStreamKey);
}
