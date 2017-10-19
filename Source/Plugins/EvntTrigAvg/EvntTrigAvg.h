/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#ifndef __EvntTrigAvg_H_3F920F95__
#define __EvntTrigAvg_H_3F920F95__

#include <ProcessorHeaders.h>
#include "EvntTrigAvgEditor.h"
#include <vector>

class EvntTrigAvgEditor;

/**
Aligns spike times with TTL input.
 
@see EvntTrigAvgCanvas, EvntTrigAvgEditor

*/

class EvntTrigAvg : public GenericProcessor
{
public:

    /** constructor */
    EvntTrigAvg();

    /** destructor */
    ~EvntTrigAvg();


    // PROCESSOR METHODS //

    void handleEvent (const EventChannel* eventInfo, const MidiMessage& event, int sampleNum) override;
    void handleSpike(const SpikeChannel* channelInfo, const MidiMessage& event, int samplePosition) override;
    void process(AudioSampleBuffer& buffer) override;

    /** Used to alter parameters of data acquisition. */
    void setParameter(int parameterIndex, float newValue) override;

    /** Called whenever the signal chain is altered. */
    void updateSettings() override;

    /** Called prior to start of acquisition. */
    bool enable() override;

    /** Called after acquisition is finished. */
    bool disable() override;

    /** Creates the EvntTrigAvgEditor. */
    //AudioProcessorEditor* createEditor() override;
    AudioProcessorEditor* createEditor() override;
   
    float getSampleRate();
    int getLastTTLCalculated();
    uint64 getWindowSize();
    uint64 getBinSize();
    std::vector<String> getElectrodeLabels();
    CriticalSection* getMutex() { return &mut; }
    //get pointers to shared data
    Array<uint64 *> getHistoData();
    Array<float *> getMinMaxMean();

    //create histogram data
    uint64* createHistogramData(std::vector<uint64> spikeData, std::vector<uint64> ttlData); // shared data
    uint64 binDataPoint(uint64 startBin, uint64 endBin, uint64 binSize, uint64 dataPoint); // shared data
    void processSpikeData(std::vector<std::vector<std::vector<uint64>>> spikeData,std::vector<uint64> ttlData);
    uint64* binCount(std::vector<uint64> binData, uint64 numberOfBins); // shared data
    bool shouldReadHistoData();
    float findMin(uint64* data_);
    float findMax(uint64* data_);
    float findMean(uint64* data_); // TODO make running
    
    
    std::vector<int> createElectrodeMap();
    std::vector<String> createElectrodeLabels();
    
    void saveCustomParametersToXml (XmlElement* parentElement) override;
    void loadCustomParametersFromXml() override;
private:
    CriticalSection mut;
    void initializeHistogramArray();
    void initializeMinMaxMean();
    void clearHistogramArray();
    void clearMinMaxMean();
    void addNewSortedIdHistoData(int electrode, int sortedId);
    void addNewSortedIdMinMaxMean(int electrode,int sortedID);
    std::atomic<int> triggerEvent;
    std::atomic<int> triggerChannel;

    uint64 bins[1000]; // workspace for bin counting

    int numChannels = 0;
    bool recalc = false;
    int lastTTLCalculated = 0;
    uint64 windowSize;
    uint64 binSize;
    
    std::vector<uint64> ttlTimestampBuffer;
    std::vector<std::vector<std::vector<uint64>>> spikeData;// channel.sortedID.spikeInstance.timestamp
    void clearHistogramData(uint64 * const);
    Array<uint64*> histogramData; // shared data
    Array<float*> minMaxMean; // shared data
    std::vector<int> electrodeMap; // Used to identify what electrode a spike came from
    std::vector<String> electrodeLabels;
    std::vector<int> idIndex; //sorted ID, electrode. used to match a sortedID with its electrode
    std::vector<std::vector<int>> electrodeSortedId; 
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvntTrigAvg);

};



#endif  // __EvntTrigAvg_H_3F920F95__
