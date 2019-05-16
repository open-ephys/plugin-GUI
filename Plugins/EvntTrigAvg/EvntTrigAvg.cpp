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

#include <stdio.h>
#include "EvntTrigAvg.h"
#include "EvntTrigAvgCanvas.h"
//#include "HistogramLib/HistogramLib.h"
class EvntTrigAvg;

EvntTrigAvg::EvntTrigAvg()
    : GenericProcessor("Evnt Trig Avg")

{
    setProcessorType (PROCESSOR_TYPE_FILTER);
    windowSize = getDefaultSampleRate(); // 1 sec in samples
    binSize = getDefaultSampleRate()/100; // 10 milliseconds in samples
    updateSettings();
}

EvntTrigAvg::~EvntTrigAvg()
{
    clearHistogramArray();
    clearMinMaxMean();
}

void EvntTrigAvg::setParameter(int parameterIndex, float newValue)
{
    bool changed = false;
    if (parameterIndex == 0 && triggerEvent != static_cast<int>(newValue)){
        triggerEvent = static_cast<int>(newValue);
        changed = true;
    }
    else if (parameterIndex == 1 && triggerChannel != static_cast<int>(newValue)){
        triggerChannel = static_cast<int>(newValue);
        changed = true;
    }
    else if(parameterIndex == 2 && binSize != newValue*(getSampleRate()/1000)){
        binSize = newValue*(getSampleRate()/1000);

        changed = true;
    }
    else if(parameterIndex == 3 && windowSize != newValue*(getSampleRate()/1000)){
        windowSize = newValue*(getSampleRate()/1000);
        changed = true;
    }
    else if (parameterIndex == 4)
        changed = true;
    
    // If anything was changed, delete all data and start over
    if (changed){
        spikeData.clear();
        ttlTimestampBuffer.clear();
        lastTTLCalculated=0;
        updateSettings();
    }
}

void EvntTrigAvg::updateSettings()
{
    clearMinMaxMean();
    clearHistogramArray();
    initializeHistogramArray();
    initializeMinMaxMean();
  //  electrodeMap.clear();
 //   electrodeMap = createElectrodeMap();
    electrodeLabels.clear();
    electrodeLabels = createElectrodeLabels();
    if(spikeData.size()!=getTotalSpikeChannels())
        spikeData.resize(getTotalSpikeChannels());
    electrodeSortedId.clear();
    if(electrodeSortedId.size()!=getTotalSpikeChannels())
        electrodeSortedId.resize(getTotalSpikeChannels());
    for(int electrodeIt = 0 ; electrodeIt < spikeData.size() ; electrodeIt++){
        electrodeSortedId[electrodeIt].push_back(0);
        if(spikeData[electrodeIt].size()<1)
            spikeData[electrodeIt].resize(1);
    }
}
void EvntTrigAvg::initializeHistogramArray()
{
    const ScopedLock lock(mut);
    for (int i = 0 ; i < getTotalSpikeChannels() ; i++){
        histogramData.add(new uint64[1003]{0});
        histogramData[i][0]=i;//electrode
        histogramData[i][1]=0;//sortedID
        histogramData[i][2]=0;//num bins used
        for (int data = 3 ; data < 1003 ; data++){
            histogramData[i][data] = 0;
        }
    }
}

void EvntTrigAvg::initializeMinMaxMean()
{
    const ScopedLock lock(mut);
    for (int i = 0 ; i < getTotalSpikeChannels() ; i++){
        minMaxMean.add(new float[5]);
        minMaxMean[i][0]=i;//electrode
        minMaxMean[i][1]=0;//sortedId
        minMaxMean[i][2]=0;//minimum
        minMaxMean[i][3]=0;//maximum
        minMaxMean[i][4]=0;//Mean
    }
}

void EvntTrigAvg::clearHistogramArray()
{
    const ScopedLock lock(mut);
    for (int i = 0 ; i < histogramData.size() ; i++)
        delete[] histogramData[i];
    histogramData.clear();
}
void EvntTrigAvg::clearMinMaxMean()
{
    const ScopedLock lock(mut);
    for (int i = 0 ; i < minMaxMean.size() ; i++)
        delete[] minMaxMean[i];
    minMaxMean.clear();
}

bool EvntTrigAvg::enable()
{
    return true;
}

bool EvntTrigAvg::disable()
{
    return true;
}


void EvntTrigAvg::process(AudioSampleBuffer& buffer)
{
    
    checkForEvents(true);// see if got any spikes
    
    if(buffer.getNumChannels() != numChannels)
        numChannels = buffer.getNumChannels();
    if(ttlTimestampBuffer.size() > lastTTLCalculated && buffer.getNumSamples() + getTimestamp(0) >= ttlTimestampBuffer[lastTTLCalculated+1] + windowSize/2){ // if need to recalc
        recalc = true;
    }
    if(recalc){ // triggered after window time has expirered
        //process the data
        processSpikeData(spikeData, ttlTimestampBuffer);
        
        //clear the data
        for(int channelIterator = 0 ; channelIterator < spikeData.size() ; channelIterator++){
            for(int sortedIdIterator = 0 ; sortedIdIterator < spikeData[channelIterator].size() ; sortedIdIterator++){
                spikeData[channelIterator][sortedIdIterator].clear();
            }
        }
        //advance the TTL that needs to be calculated
        lastTTLCalculated+=1;
        //just recalculated, don't need to again until next ttl window has expired
        recalc=false;
    }
}

void EvntTrigAvg::handleEvent(const EventChannel* eventInfo, const MidiMessage& event, int sampleNum)
{
    if (triggerEvent < 0) return;
    else if (eventInfo->getChannelType() == EventChannel::TTL && eventInfo == eventChannelArray[triggerEvent])
    {// if TTL from right channel
        TTLEventPtr ttl = TTLEvent::deserializeFromMessage(event, eventInfo);
        if (ttl->getChannel() == triggerChannel && ttl->getState())
            ttlTimestampBuffer.push_back(Event::getTimestamp(event)); // add timestamp of TTL to buffer
    }
}

void EvntTrigAvg::handleSpike(const SpikeChannel* spikeInfo, const MidiMessage& event, int samplePosition)
{
    SpikeEventPtr newSpike = SpikeEvent::deserializeFromMessage(event, spikeInfo);
    if (!newSpike)
        return;
    else {
        // extract information from spike
        
        const SpikeChannel* chan = newSpike->getChannelInfo();
        Array<SourceChannelInfo> chanInfo = chan->getSourceChannelInfo();
        //int chanIDX = chanInfo[0].channelIDX;
        int electrode = getSpikeChannelIndex(newSpike);
        //std::cout<<"chanIDX: " << chanIDX << "\n";
        int sortedID = newSpike->getSortedID();
        //int electrode = electrodeMap[chanInfo];
        if(sortedID!=0 && sortedID>idIndex.size()){ // respond to new sortedID
            idIndex.push_back(spikeData[electrode].size());// update map of what sorted ID is on what electrode
        }
            
        bool newID = true;
        //for(int i = 0 ; i < electrodeSortedId[chanIDX].size() ; i++){
        for(int i = 0 ; i < electrodeSortedId[electrode].size() ; i++){
            //if(sortedID == electrodeSortedId[chanIDX][i])
           if(sortedID == electrodeSortedId[electrode][i])
               newID=false;
        }
        if(newID){
            //electrodeSortedId[chanIDX].push_back(sortedID);
            electrodeSortedId[electrode].push_back(sortedID);
            addNewSortedIdMinMaxMean(electrode,sortedID);
            addNewSortedIdHistoData(electrode,sortedID); //insert new sortedId into histogramArray
            spikeData[electrode].resize(spikeData[electrode].size()+1);
            }
        
        int relativeSortedID = 0;
        if (sortedID>0)
            relativeSortedID = idIndex[sortedID-1];
        spikeData[electrode][0].push_back(newSpike->getTimestamp());
        if (sortedID>0)
            spikeData[electrode][relativeSortedID].push_back(newSpike->getTimestamp());
    }
}

void EvntTrigAvg::addNewSortedIdHistoData(int electrode,int sortedId)
{
    const ScopedLock myScopedLock(mut);
    if(electrode == getTotalSpikeChannels()-1){
        histogramData.add(new uint64[1003]{0});
        histogramData.getLast()[0]=electrode;//electrode
        histogramData.getLast()[1]=sortedId;//sortedID
        histogramData.getLast()[2]=windowSize/binSize;//num bins used

        return;
    }
    else{
        for(int i = 1 ; i < histogramData.size() ; i++){
            if(histogramData[i][0]>electrode){
                histogramData.insert(i,new uint64[1003]{0});
                histogramData[i][0]=electrode;//electrode
                histogramData[i][1]=sortedId;//sortedID
                histogramData[i][2]=windowSize/binSize;//num bins used
                return;
            }
        }
    }
}

void EvntTrigAvg::addNewSortedIdMinMaxMean(int electrode,int sortedId)
{
    const ScopedLock myScopedLock(mut);
    if(electrode == getTotalSpikeChannels()-1){
        minMaxMean.add(new float[5]);
        minMaxMean.getLast()[0]=electrode;//electrode
        minMaxMean.getLast()[1]=sortedId;//sortedID
        minMaxMean.getLast()[2]=0;//minimum
        minMaxMean.getLast()[3]=0;//maximum
        minMaxMean.getLast()[4]=0;//mean
        return;
    }
    else{
        for(int i = 1 ; i < histogramData.size() ; i++){
            if(minMaxMean[i][0]>electrode){
                minMaxMean.insert(i,new float[5]);
                minMaxMean[i][0]=electrode;//electrode
                minMaxMean[i][1]=sortedId;//sortedID
                minMaxMean[i][2]=0;//minimum
                minMaxMean[i][3]=0;//maximum
                minMaxMean[i][4]=0;//mean
                return;
            }
        }
    }
}

//AudioProcessorEditor* EvntTrigAvg::createEditor()
AudioProcessorEditor* EvntTrigAvg::createEditor()
{
    editor = new EvntTrigAvgEditor (this, true);
    return editor;
}

float EvntTrigAvg::getSampleRate()
{
	return CoreServices::getGlobalSampleRate();
}

int EvntTrigAvg::getLastTTLCalculated()
{
    return lastTTLCalculated;
}

/** creates map to convert channelIDX to electrode number */
/*
std::map<SourceChannelInfo, int> EvntTrigAvg::createElectrodeMap()
{
    std::map<SourceChannelInfo,int> map;
    int numSpikeChannels = getTotalSpikeChannels();
    int electrodeCounter=0;
    for (int chanIt = 0 ; chanIt < numSpikeChannels ; chanIt++){
        const SpikeChannel* chan = getSpikeChannel(chanIt);
        // add to running count of each electrode

        Array<SourceChannelInfo> chanInfo = chan->getSourceChannelInfo();
        for (int subChanIt = 0 ; subChanIt < chan->getNumChannels() ; subChanIt++){
			
            map[chanInfo[subChanIt]]=electrodeCounter;
        }
        electrodeCounter+=1;
    }
    return map;
}*/

std::vector<String> EvntTrigAvg::createElectrodeLabels()
{
    std::vector<String> map;
    int numSpikeChannels = getTotalSpikeChannels();
    map.resize(numSpikeChannels);
    String electrodeNames[3]{"Si ","St ","TT "};
    int electrodeCounter[3]{0};
    for (int chanIt = 0 ; chanIt < numSpikeChannels ; chanIt++){
        const SpikeChannel* chan = getSpikeChannel(chanIt);
        // add to running count of each electrode
        int chanType = chan->getChannelType();
        electrodeCounter[chanType]+=1;
        map[chanIt]=electrodeNames[chanType]+String(electrodeCounter[chanType]);
    }
    return map;
}

/** passes data into createHistogramData() by electrode and sorted ID */
void EvntTrigAvg::processSpikeData(std::vector<std::vector<std::vector<uint64>>> spikeData,std::vector<uint64> ttlData)
{
    
    for (int channelIterator = 0 ; channelIterator < getTotalSpikeChannels() ; channelIterator++){
        const ScopedLock myScopedLock(mut);
        for (int sortedIdIterator = 0 ; sortedIdIterator < spikeData[channelIterator].size() ; sortedIdIterator++){
            uint64* data = createHistogramData(spikeData[channelIterator][sortedIdIterator],ttlData);
            histogramData[channelIterator+sortedIdIterator][2]=windowSize/binSize;
            for(int dataIterator = 3 ; dataIterator<windowSize/binSize+3 ; dataIterator++){
                histogramData[channelIterator+sortedIdIterator][dataIterator] += (data[dataIterator-3]);
            }
        minMaxMean[channelIterator+sortedIdIterator][2]= findMin(&histogramData[channelIterator+sortedIdIterator][3]);
            
        minMaxMean[channelIterator+sortedIdIterator][3]= findMax(&histogramData[channelIterator+sortedIdIterator][3]);
            
        minMaxMean[channelIterator+sortedIdIterator][4] = findMean(&histogramData[channelIterator+sortedIdIterator][3]);
        }
    }
}

/** returns bin counts */
uint64* EvntTrigAvg::createHistogramData(std::vector<uint64> spikeData, std::vector<uint64> ttlData)
{
    uint64 numberOfBins = windowSize/binSize;
    std::vector<uint64> histoData;
    for(int ttlIterator = 0 ; ttlIterator < ttlData.size() ; ttlIterator++){
        for(int spikeIterator = 0 ; spikeIterator < spikeData.size() ; spikeIterator++){
            int relativeSpikeValue = int(spikeData[spikeIterator])-int(ttlData[ttlIterator]);
            if (relativeSpikeValue >= -int(windowSize)/2 && relativeSpikeValue <= int(windowSize)/2){
                uint64 bin = binDataPoint(0, numberOfBins, binSize, relativeSpikeValue+windowSize/2);
                histoData.push_back(bin);
            }
        }
    }
    return binCount(histoData,numberOfBins);
}


/** Returns the bin a data point belongs to given the very first bin, the very last bin, bin size and the data point to bin, currently only works for positive numbers (can get around by adding minimum value to all values*/
uint64 EvntTrigAvg::binDataPoint(uint64 startBin, uint64 endBin, uint64 binSize, uint64 dataPoint)
{
    uint64 binsInRange = (endBin-startBin);
    uint64 binsToSearch = binsInRange/2;
    if (binsToSearch <= 1){
        
        if (dataPoint < (startBin+binsToSearch)*binSize){
            return startBin;
        }
        else if (dataPoint < (startBin+1+binsToSearch) * binSize){
            return startBin+1;
        }
        else{
            return startBin+2;
        }
    }

    else if (dataPoint < (startBin+binsToSearch)*binSize){ // if in first half of search range
        return binDataPoint(startBin,startBin+(binsToSearch),binSize,dataPoint);
    }
    else if (dataPoint >= (startBin+binsToSearch) * binSize){ // if in second half of search range
        return binDataPoint(startBin+(binsToSearch),endBin,binSize,dataPoint);
    }
    else{
        return (uint64) NULL;
    }
}

/** count the number of bin instances */
uint64* EvntTrigAvg::binCount(std::vector<uint64> binData,uint64 numberOfBins)
{
    for (int i = 0 ; i < 1000 ; i++){
        bins[i]=0;
    }
    for (int dataIterator = 0 ; dataIterator < binData.size() ; dataIterator++){
        bins[binData[dataIterator]] = bins[binData[dataIterator]]+1;
    }
    return bins;
}

uint64 EvntTrigAvg::getBinSize()
{
    return binSize;
}

uint64 EvntTrigAvg::getWindowSize()
{
    return windowSize;
}

Array<uint64 *> EvntTrigAvg::getHistoData()
{
    const ScopedLock myScopedLock(mut);
    return histogramData;
}

Array<float *> EvntTrigAvg::getMinMaxMean()
{
    const ScopedLock myScopedLock(mut);
    return minMaxMean;
}

float EvntTrigAvg::findMin(uint64* data_)
{
    const ScopedLock myScopedLock(mut);
    //uint64 min = UINT64_MAX;
    uint64 min = 18446744073709551614U;
    for (int i = 0 ; i < windowSize/binSize ; i++){
        if(data_[i]<min){
            min=data_[i];
        }
    }
    return float(min);
}

float EvntTrigAvg::findMax(uint64* data_)
{
    const ScopedLock myScopedLock(mut);
    uint64 max = 0;
    for (int i = 0 ; i < windowSize/binSize ; i++){
        if(data_[i]>max){
            max=data_[i];
        }
    }
    return float(max);
}

float EvntTrigAvg::findMean(uint64* data_)
{
    const ScopedLock myScopedLock(mut);
    uint64 runningSum=0;
    for(int i=0 ; i < windowSize/binSize ; i++){
        runningSum += data_[i];
    }
    float mean = float(runningSum)/(float(windowSize)/float(binSize));
    return mean;
}



std::vector<String> EvntTrigAvg::getElectrodeLabels()
{
    return electrodeLabels;
}

void EvntTrigAvg::clearHistogramData(uint64 * dataptr)
{
    const ScopedLock myScopedLock(mut);
    for(int i = 0 ; i < 1000 ; i++)
        dataptr[i] = 0;
}


void EvntTrigAvg::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("EVNTTRIGAVG");
    mainNode->setAttribute ("trigger", triggerChannel);
    mainNode->setAttribute ("bin", int(binSize/(getSampleRate()/1000)));
    mainNode->setAttribute ("window", int(windowSize/(getSampleRate()/1000)));
}

void EvntTrigAvg::loadCustomParametersFromXml()
{
    if (parametersAsXml)
    {
        EvntTrigAvgEditor* ed = (EvntTrigAvgEditor*) getEditor();
        
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("EVNTTRIGAVG"))
            {
                triggerChannel = mainNode->getIntAttribute("trigger");
                std::cout<<"set trigger channel to: " << triggerChannel << "\n";
                ed->setTrigger(mainNode->getIntAttribute("trigger"));
                
                binSize = uint64(mainNode->getIntAttribute("bin"));
                std::cout<<"set bin size to: " << binSize << "\n";
                ed->setBin(mainNode->getIntAttribute("bin"));
                
                windowSize = uint64(mainNode->getIntAttribute("window"));
                std::cout<<"set window size to: " << windowSize << "\n";
                ed->setWindow(mainNode->getIntAttribute("window"));
            }
        }
    }
}

