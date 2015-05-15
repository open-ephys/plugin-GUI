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

#ifndef __TRIALCIRCULARBUFFER_H__
#define __TRIALCIRCULARBUFFER_H__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Visualization/SpikeObject.h"
#include "../Visualization/MatlabLikePlot.h"
#include "../SpikeSorter/SpikeSorter.h"
#include "../NetworkEvents/NetworkEvents.h"
#include <algorithm>
#include <queue>
#include <vector>
#include <list>
class Electrode;

#define TTL_TRIAL_OFFSET 30000

#ifndef MAX
#define MAX(a,b)((a)<(b)?(b):(a))
#endif

#ifndef MIN
#define MIN(a,b)((a)<(b)?(a):(b))
#endif

class TrialCircularBufferParams
{
public:
    TrialCircularBufferParams();
    ~TrialCircularBufferParams();
    double desiredSamplingRateHz;
    double maxTrialTimeSeconds;
    double postSec, preSec;
    int maxTrialsInMemory;
    double binResolutionMS;
    int numChannels;
    double sampleRate;
    double ttlSupressionTimeSec;
    double ttlTrialLengthSec;
    int numTTLchannels;
    bool autoAddTTLconditions;
    bool buildTrialsPSTH;
    bool reconstructTTL;
    bool approximate;
};

class Condition
{
public:
    Condition();
    Condition(std::vector<String> items, int ID);
    Condition(const Condition& c);
    Condition(String Name, std::vector<int> types, std::vector<int> outcomes, double _postSec, double _preSec);
    String name;
    uint8 colorRGB[3];
    std::vector<int> trialTypes;
    std::vector<int> trialOutcomes;
    //void setDefaultColors(uint8 &R, uint8 &G, uint8 &B, int ID);

    double postSec, preSec;
    bool visible;
    int conditionID;
    float posX, posY; // some conditions might be displayed according to their spatial location
    int conditionGroup; // conditions may be groupped to allow better visualization.
};


class Trial
{
public:
    Trial();
    Trial(const Trial& t);

    int trialID;
    int outcome;
    int type;
    int64 startTS, alignTS, endTS, alignTS_hardware;
    bool trialInProgress;
    bool hardwareAlignment;
};

class SmartSpikeCircularBuffer
{
public:
    SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int maxTrialsInMemory, int _sampleRateHz);
    // contains spike times, but also pointers for trial onsets so we don't need to search
    // the entire array
    void addSpikeToBuffer(int64 spikeTimeSoftware,int64 spikeTimeHardware);
    std::vector<int64> getAlignedSpikes(Trial* trial, float preSecs, float postSecs);
    void addTrialStartToBuffer(Trial* t);

    int queryTrialStart(int trialID);
private:
    std::vector<int64> spikeTimesSoftware;
    std::vector<int64> spikeTimesHardware;
    std::vector<int> trialID;
    std::vector<int> pointers;
    int maxTrialsInMemory;
    int bufferSize;
    int bufferIndex;
    int trialIndex;
    int sampleRateHz;
    int numTrialsStored;
    int numSpikesStored;
};




class SmartContinuousCircularBuffer : public ContinuousCircularBuffer
{
public:
    SmartContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer);
    bool getAlignedData(std::vector<int> channels, Trial* trial, std::vector<float>* timeBins,
                        TrialCircularBufferParams params,
                        std::vector<std::vector<float> >& output,
                        std::vector<bool>& valid);

    bool getAlignedDataInterp(std::vector<int> channels, Trial* trial, std::vector<float>* timeBins,
                              float preSec, float postSec,
                              std::vector<std::vector<float> >& output,
                              std::vector<bool>& valid);

    void addTrialStartToSmartBuffer(int trialID);
    int trialptr;
    int numTrials;
    std::vector<int> smartPointerIndex;
    std::vector<int> smartPointerTrialID;

};


class PSTH
{
public:
    PSTH(int ID, TrialCircularBufferParams params,bool vis);
    ~PSTH();
    PSTH(const PSTH& c);
    double getDx();
    void clear();
    void updatePSTH(SmartSpikeCircularBuffer* spikeBuffer, Trial* trial);
    void updatePSTH(std::vector<float> alignedLFP,std::vector<bool> valid);

    std::vector<float> getAverageTrialResponse();
    std::vector<float> getLastTrial();

    void getRange(float& xMin, float& xMax, float& yMin, float& yMax);

    int conditionID;
    float xmin, xmax, ymax,ymin;
    bool visible;
    int numBins;
    int numTrials;
    float timeSpanSecs;
    std::vector<int> numDataPoints;
    std::vector<float> binTime;
    uint8 colorRGB[3];
    TrialCircularBufferParams params;

private:
    double dx,mod_pre_sec, mod_post_sec;
    std::list<std::vector<float>> prevTrials;
    std::vector<float> avgResponse; // either firing rate or lfp


    std::vector<int64> getAlignSpikes(SmartSpikeCircularBuffer* spikeBuffer, Trial* t);

};

class UnitPSTHs
{
public:
    UnitPSTHs(int ID,TrialCircularBufferParams params,uint8 R, uint8 G, uint8 B);
    void updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial* trial);
    void addSpikeToBuffer(int64 spikeTimestampSoftware,int64 spikeTimestampHardware);
    void addTrialStartToSmartBuffer(Trial* t);
    void clearStatistics();
    void getRange(float& xmin, float& xmax, float& ymin, float& ymax);
    bool isNewDataAvailable();
    void informPainted();
    void startUniqueInterval(int uniqueCode);
    void stopUniqueInterval();
    int getUniqueInterval();

    std::vector<PSTH> conditionPSTHs;
    std::vector<PSTH> trialPSTHs;

    SmartSpikeCircularBuffer spikeBuffer;
    uint8 colorRGB[3];
    int unitID;
    int uniqueIntervalID;
    bool redrawNeeded;
    int numTrials;
    TrialCircularBufferParams params;

};

class ChannelPSTHs
{
public:
    ChannelPSTHs(int channelID, TrialCircularBufferParams params);
    void updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> lfpData, std::vector<bool> valid, Trial* trial);
    void clearStatistics();
    void getRange(float& xmin, float& xmax, float& ymin, float& ymax);
    bool isNewDataAvailable();
    void informPainted();
    int channelID;
    std::vector<PSTH> conditionPSTHs;
    std::vector<PSTH> trialPSTHs;
    TrialCircularBufferParams params;
    int numTrials;
    bool redrawNeeded;
};

class TTL_PSTHs
{
public:
    TTL_PSTHs(int ttlChannelID, TrialCircularBufferParams params);
    void updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> lfpData, std::vector<float> valid, Trial* trial);
    void clearStatistics();
    void getRange(float& xmin, float& xmax, float& ymin, float& ymax);
    int ttlChannelID;
    std::vector<PSTH> conditionPSTHs;
    TrialCircularBufferParams params;
    bool redrawNeeded;
};


class ElectrodePSTH
{
public:
    ElectrodePSTH();
    ElectrodePSTH(int ID, String name);
    ~ElectrodePSTH();
    void updateChannelsConditionsWithLFP(std::vector<int> conditionsNeedUpdate, Trial* trial, SmartContinuousCircularBuffer* lfpBuffer);
    void UpdateChannelConditionWithLFP(int ch, std::vector<int>* conditionsNeedUpdate, Trial* trial, std::vector<float>* alignedLFP,std::vector<bool>* valid);
    int electrodeID;
    String electrodeName;
    std::vector<int> channels;
    std::vector<UnitPSTHs> unitsPSTHs;
    std::vector<ChannelPSTHs> channelsPSTHs;
    std::vector<TTL_PSTHs> ttlPSTHs;

    ThreadPool* threadpool; // used for multi-channel electrodes only
};

class ElectrodePSTHlfpJob : public ThreadPoolJob
{
public:
    ElectrodePSTHlfpJob(ElectrodePSTH* psth_, int ch_, std::vector<int>* conditionsNeedUpdate_, Trial* trial_, std::vector<float>* alignedLFP_, std::vector<bool>* valid_);
    JobStatus runJob();

    ElectrodePSTH* psth;
    int ch;
    std::vector<int>* conditionsNeedUpdate;
    Trial* trial;
    std::vector<float>* alignedLFP;
    std::vector<bool>* valid;
};


struct ttlStatus
{
    int channel;
    bool value;
    int64 ts;
};

class TrialCircularBuffer
{
public:
    TrialCircularBuffer();
    TrialCircularBuffer(TrialCircularBufferParams param_);
    ~TrialCircularBuffer();
    void updatePSTHwithTrial(Trial* trial);
    bool contains(std::vector<int> v, int x);
    void toggleConditionVisibility(int cond);
    void modifyConditionVisibility(int cond, bool newstate);
    void modifyConditionVisibilityusingConditionID(int condID, bool newstate);
    bool parseMessage(StringTS s);
    void addSpikeToSpikeBuffer(SpikeObject newSpike);
    void process(AudioSampleBuffer& buffer,int nSamples,int64 hardware_timestamp,int64 software_timestamp);
    void simulateHardwareTrial(int64 ttl_timestamp_software,int64 ttl_timestamp_hardware, int trialType, float lengthSec);
    //void simulateTrial(int64 ttl_timestamp_software, int trialType, float lengthSec);
    void addTTLevent(int channel,int64 ttl_timestamp_software,int64 ttl_timestamp_hardware, bool rise,bool simulateTrial);
    void addDefaultTTLConditions(Array<bool> visibility);
    void addCondition(std::vector<String> input);
    //void lockConditions();
    //void unlockConditions();
    //void lockPSTH();
    //void unlockPSTH();

    void reallocate(int numChannels);
    void simulateTTLtrial(int channel, int64 ttl_timestamp_software);
    void clearDesign();
    void clearAll();
    std::vector<std::vector<bool>>  reconstructTTLchannels(int64 hardware_timestamp,int nSamples);
    void channelChange(int electrodeID, int channelindex, int newchannel);
    void syncInternalDataStructuresWithSpikeSorter(Array<Electrode*> electrodes);
    void addNewElectrode(Electrode* electrode);
    void removeElectrode(int electrodeID);
    void addNewUnit(int electrodeID, int unitID, uint8 r,uint8 g,uint8 b);
    void removeUnit(int electrodeID, int unitID);

    void removeAllUnits(int electrodeID);
    int getNumTrialsInChannel(int electrodeID, int channelID);
    int getNumTrialsInUnit(int electrodeID, int unitID);
    void getElectrodeConditionRange(int electrodeID, int channelID, double& xmin, double& xmax)	;
    void getUnitConditionRange(int electrodeID, int unitID, double& xmin, double& xmax);
    int setUnitUniqueInterval(int electrodeID, int unitID, bool state);
    int getUnitUniqueInterval(int electrodeID, int unitID);

    String getElectrodeName(int electrodeID);
    void setHardwareTriggerAlignmentChannel(int k);
    TrialCircularBufferParams getParams();
    int getNumElectrodes();
    std::vector<int> getElectrodeChannels(int e);
    int getElectrodeID(int index);
    int getNumTrialsInCondition(int electrodeIndex, int channelIndex, int conditionIndex);
    int getNumUnitsInElectrode(int electrodeIndex);
    int getUnitID(int electrodeIndex, int unitIndex);
    int getNumConditions();
    void getLastTrial(int electrodeIndex, int channelIndex, int conditionIndex, float& x0, float& dx, std::vector<float>& y);
    Condition getCondition(int conditionIndex);
    std::vector<XYline> getElectrodeConditionCurves(int electrodeID, int channelID);
    std::vector<XYline> getUnitConditionCurves(int electrodeID, int unitID);

    std::vector<std::vector<float>> getTrialsAverageUnitResponse(int electrodeID, int unitID,
                                                                 std::vector<float>& x_time, int& numTrialTypes,
                                                                 std::vector<int>& numTrialRepeats, double smoothMS, float xmin, float xmax);


    std::vector<std::vector<float>> getTrialsAverageChannelResponse(int electrodeID, int channelID,
                                                                    std::vector<float>& x_time, int& numTrialTypes,
                                                                    std::vector<int>& numTrialRepeats, double smoothMS, float xmin, float xmax);

    juce::Image getTrialsAverageUnitResponseAsJuceImage(int electrodeID, int unitID, float guassianStandardDeviationMS, float xmin, float xmax, int ymin, int ymax, float& maxValue);
    juce::Image getTrialsAverageChannelResponseAsJuceImage(int electrodeID, int channelID, float guassianStandardDeviationMS, float xmin, float xmax, int ymin, int ymax, float& maxValue);
    int getNumTrialTypesInUnit(int electrodeID, int unitID);
    int getNumTrialTypesInChannel(int electrodeID, int channelID);
    void clearUnitStatistics(int electrodeID, int unitID);
    void clearChanneltatistics(int electrodeID, int channelID);
    std::vector<float> buildSmoothKernel(float guassianStandardDeviationMS, float binSizeInMS);
    void updateElectrodeName(int electrodeID, String newName);
    juce::Colour getUnitColor(int electrodeID, int unitID);
    int getLastTrialID();
    int getNumberAliveTrials();

    // thread job functions
    void updateLFPwithTrial(int electrodeIndex, std::vector<int>* conditionsNeedUpdate, Trial* trial);
    void updateSpikeswithTrial(int electrodeIndex, int unitIndex, std::vector<int>* conditionsNeedUpdate, Trial* trial);

    CriticalSection psthMutex;//conditionMutex
private:
    bool useThreads;
    std::vector<int> dropOutcomes;

    juce::Image getTrialsAverageResponseAsJuceImage(int  ymin, int ymax,	std::vector<float> x_time,	int numTrialTypes,
                                                    std::vector<int> numTrialRepeats,	std::vector<std::vector<float>> trialResponseMatrix, float& maxValue);

    std::vector<float> smooth(std::vector<float> y, std::vector<float> smoothKernel, int xmin, int xmax);

    bool firstTime;
    int lastTrialID;
    float numTicksPerSecond;
    int trialCounter;
    int conditionCounter;

    Trial currentTrial;
    String designName;
    int hardwareTriggerAlignmentChannel;
    int64 lastSimulatedTrialTS;
    int uniqueIntervalID;
    std::vector<int64> lastTTLts;
    std::vector<bool> ttlChannelStatus;
    std::queue<Trial> aliveTrials;
    std::vector<Condition> conditions;
    std::vector<ElectrodePSTH> electrodesPSTH;
    ScopedPointer<SmartContinuousCircularBuffer> lfpBuffer;
    ScopedPointer<SmartContinuousCircularBuffer> ttlBuffer;
    std::queue<ttlStatus> ttlQueue;
    TrialCircularBufferParams params;
    ScopedPointer<ThreadPool> threadpool;
};

class TrialCircularBufferThread : public ThreadPoolJob
{
public:
    TrialCircularBufferThread(TrialCircularBuffer* tcb_, std::vector<int>* conditions, Trial* trial_, int jobID_, int jobType_, int electrodeID_, int subID_);
    JobStatus runJob();
    TrialCircularBuffer* tcb;
    Trial* trial;
    std::vector<int>* conditionsNeedUpdate;
    int jobID;
    int jobType;
    int electrodeID;
    int subID;
};


#endif  // __TRIALCIRCULARBUFFER_H__
