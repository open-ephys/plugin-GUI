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

#include "../../JuceLibraryCode/JuceHeader.h"
#include <stdio.h>
#include "TrialCircularBuffer.h"
#include "../SpikeSorter/SpikeSorter.h"
#include "../Channel/Channel.h"
#include <string>
#include "tictoc.h"
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

TicToc tictoc;

TrialCircularBufferParams::TrialCircularBufferParams()
{
}

TrialCircularBufferParams::~TrialCircularBufferParams()
{
}

void setDefaultColors(uint8 &R, uint8 &G, uint8 &B, int ID)
{
	int IDmodule = (ID-1) % 32; // ID can't be zero
	const int colors[32][3] = {

	{26, 188, 156},
	{46, 204, 113},
	{52, 152, 219},
	{155, 89, 182},
	{241, 196, 15},
	{52, 73, 94},
	{230, 126, 34},
	{231, 76, 60},
	{255, 0, 0},
	{39, 174, 96},
	{41, 128, 185},
	{142, 68, 173},
	{44, 62, 80},
	{243, 156, 18},
	{211, 84, 0},
	{192, 57, 43},
	{224,185,36},
	{214,210,182},
	{243,119,33},
	{186,157,168},
	{237,37,36},
	{179,122,79},
	{217,46,171},
	{217, 139,196},
	{101,31,255},
	{141,111,181},
	{48,117,255},
	{184,198,224},
	{116,227,156},
	{150,158,155},
	{82,173,0},
	{125,99,32}};

	/*
   {      0,         0,    255},
   {      0,    128,         0},
   { 255,         0,         0},
   {      0,    192,    192},
   { 192,         0,    192},
   { 192,    192,         0}};*/

	R = colors[IDmodule][0];
	G = colors[IDmodule][1];
	B = colors[IDmodule][2];

}


/******************************/
PSTH::PSTH(int ID, TrialCircularBufferParams params_, bool vis) : conditionID(ID),params(params_),numTrials(0),visible(vis)
{
	// if approximate is on, we won't sample exactly xmin and xmax
	if (params.approximate)
	{
		dx = params.binResolutionMS / 1000.0f;

		int numPostBins = ceil((params.postSec) / dx); 
		int numPosBins = ceil((params.postSec + params.maxTrialTimeSeconds) / dx); 
		int numNegBins = ceil(params.preSec / dx); 

		numBins = 1 + numPosBins + numNegBins; // include bin "0"
		timeSpanSecs=numBins * dx;

		mod_pre_sec = dx * numNegBins;
		mod_post_sec = dx *numPostBins;

		//
		avgResponse.resize(numBins);
		numDataPoints.resize(numBins);
		binTime.resize(numBins);

		for (int k = 0; k < numNegBins; k++)
		{
			numDataPoints[k] = 0;
			binTime[k] = -dx * (numNegBins-k);
			avgResponse[k] = 0;
		}
		for (int k = 0; k < 1+numPosBins; k++)
		{
			numDataPoints[numNegBins+k] = 0;
			binTime[numNegBins+k] = dx * k;
			avgResponse[numNegBins+k] = 0;
		}

	} else
	{
		// accurate, but slow sampling (bilinear interpolation is needed)
		mod_pre_sec = params.preSec;
		mod_post_sec = params.postSec;
		timeSpanSecs=params.preSec+ params.postSec + params.maxTrialTimeSeconds;
		numBins = ceil((timeSpanSecs) * 1000.0f / params.binResolutionMS); 
		avgResponse.resize(numBins);
		numDataPoints.resize(numBins);
		binTime.resize(numBins);
		for (int k = 0; k < numBins; k++)
		{
			numDataPoints[k] = 0;
			binTime[k] = (float)k / numBins * (timeSpanSecs) - params.preSec;
			avgResponse[k] = 0;

		}
		dx = binTime[1]-binTime[0];
	}
	xmin = -mod_pre_sec;
	xmax = mod_post_sec;
	ymax = -1e10;
	ymin = 1e10;
	setDefaultColors(colorRGB[0],colorRGB[1],colorRGB[2], ID);

}
PSTH::~PSTH()
{
	
}
double PSTH::getDx()
{
	return dx;
}

PSTH::PSTH(const PSTH& c)
{
	conditionID = c.conditionID;
	numTrials = c.numTrials;
	params = c.params;
	numBins = c.numBins;
	avgResponse=c.avgResponse;
	prevTrials = c.prevTrials;
	numDataPoints=c.numDataPoints;
	timeSpanSecs = c.timeSpanSecs;
	binTime = c.binTime;
	xmin = c.xmin;
	xmax = c.xmax;
	ymax = c.ymax;
	ymin = c.ymin;
	colorRGB[0] = c.colorRGB[0];
	colorRGB[1] = c.colorRGB[1];
	colorRGB[2] = c.colorRGB[2];
	visible = c.visible;
	dx = c.dx;
	mod_pre_sec = c.mod_pre_sec;
	mod_post_sec = c.mod_post_sec;
	
}


void PSTH::clear()
{
	numTrials= 0;

	xmin = -mod_pre_sec;
	xmax = mod_post_sec;

	ymax = -1e10;
	ymin = 1e10;
	for (int k = 0; k < numBins; k++)
	{
		numDataPoints[k] = 0;
		avgResponse[k] = 0;
	}
	
}

void PSTH::updatePSTH(SmartSpikeCircularBuffer *spikeBuffer, Trial *trial)
{

	tictoc.Tic(16);
	Time t;
	float ticksPerSec =t.getHighResolutionTicksPerSecond();

	tictoc.Tic(30);
	std::vector<int64> alignedSpikes = spikeBuffer->getAlignedSpikes(trial, mod_pre_sec, mod_post_sec);
	tictoc.Toc(31);

	tictoc.Tic(32);
	std::vector<float> instantaneousSpikesRate;
	instantaneousSpikesRate.resize(numBins);
	for (int k = 0; k < numBins; k++)
	{
		instantaneousSpikesRate[k] = 0;
	}

	std::cout << "Received " << alignedSpikes.size() << " spikes." << std::endl;

	for (int k=0;k<alignedSpikes.size();k++)
	{
		// spike times are aligned relative to trial alignment (i.e.) , onset is at "0"
		// convert ticks back to seconds, then to bins.
		float spikeTimeSec = float(alignedSpikes[k]) / ticksPerSec;

		int binIndex = (spikeTimeSec +mod_pre_sec) / timeSpanSecs * numBins;
		if (binIndex >= 0 && binIndex < numBins)
		{
			instantaneousSpikesRate[binIndex] += 1.0;
		}
	}

	float lastUpdateTS = float(trial->endTS-trial->alignTS) / ticksPerSec + mod_post_sec;
	int lastBinIndex = (int)( (lastUpdateTS + mod_pre_sec) / timeSpanSecs * numBins);


	xmax = MAX(xmax,lastUpdateTS);


	numTrials++;
	// Update average firing rate, up to when the trial ended. 
	ymax = -1e10;
	ymin = 1e10;

	 float scale = 1000.0;
	for (int k = 0; k < lastBinIndex; k++)
	{
		numDataPoints[k]++;
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + scale*instantaneousSpikesRate[k]) / numDataPoints[k];
		ymax = MAX(ymax,avgResponse[k]);
		ymin = MIN(ymin,avgResponse[k]);
	}
	tictoc.Toc(32);

	// keep existing trial
	if (prevTrials.size()+1 > params.maxTrialsInMemory)
	{
		prevTrials.pop_back();
	}
	prevTrials.push_front(instantaneousSpikesRate);
	tictoc.Toc(16);

}

void PSTH::getRange(float &xMin, float &xMax, float &yMin, float &yMax)
{
	xMin = xmin;
	yMax = ymax;
	xMax = xmax;
	yMin = ymin;
}

void PSTH::updatePSTH(std::vector<float> alignedLFP,std::vector<bool> valid)
{
	
	numTrials++;

	ymax = -1e10;
	ymin = 1e10;
	xmin = -mod_pre_sec;
	xmax = 0;

	// Update average firing rate, up to when the trial ended. 
	for (int k = 0; k < valid.size(); k++)
	{
		if (!valid[k]) {
			xmax = MAX(xmax, binTime[k]);
			break;
		}
		numDataPoints[k]++;
		avgResponse[k] = ((numDataPoints[k] - 1) * avgResponse[k] + alignedLFP[k]) / numDataPoints[k];
		ymax = MAX(ymax, avgResponse[k]);
		ymin = MIN(ymin, avgResponse[k]);
	}			
	
	// keep existing trial
	if (prevTrials.size()+1 > params.maxTrialsInMemory)
	{
		prevTrials.pop_back();
	}
	prevTrials.push_front(alignedLFP);
	
}

std::vector<float> PSTH::getAverageTrialResponse()
{
	std::vector<float> tmp;
	
	tmp = avgResponse;

	return tmp;
}

std::vector<float> PSTH::getLastTrial()
{
	std::vector<float> tmp;
	if (prevTrials.size() > 0)
	{
		tmp = prevTrials.front();
	}
	return tmp;
}

/***********************/
Condition::Condition()
{
	conditionID = 0;
}

Condition::Condition(const Condition &c)
{
       name = c.name;
	   colorRGB[0] = c.colorRGB[0];
	   colorRGB[1] = c.colorRGB[1];
	   colorRGB[2] = c.colorRGB[2];
	   trialTypes = c.trialTypes;
	   trialOutcomes = c.trialOutcomes;
	   postSec = c.postSec;
	   preSec = c.preSec;
	   visible = c.visible;
	   posX = c.posX;
	   posY = c.posY;
	   conditionGroup = c.conditionGroup;
	   conditionID = c.conditionID;
}

Condition::Condition(std::vector<String> items, int ID)
{
	 int k=1; // skip the addcontision in location 0
	 name = "Unknown";
	 setDefaultColors(colorRGB[0],colorRGB[1],colorRGB[2], ID);
	 postSec = preSec = 0.5;
	 conditionID = ID;

	 visible = true;
	 bool bTrialTypes = false;
	 bool bOutcomes = false;
	 while (k < items.size())
	 {
		 String lower_item = items[k].toLowerCase();
		 if (lower_item == "") {
			 k++;
			 continue;
		 }

		 if (lower_item == "name")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 name = items[k];
			 k++;
			 continue;
		 }
		  if (lower_item == "group")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 conditionGroup = items[k].getIntValue();
			 k++;
			 continue;
		 }
		if (lower_item == "spatialposition")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 posX = items[k].getFloatValue();
			 k++;
			 posY = items[k].getFloatValue();
			 k++;
			 continue;
		 }
		 
		 if (lower_item == "color")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 colorRGB[0] = items[k].getIntValue();
			 k++;
			 colorRGB[1] = items[k].getIntValue();
			 k++;
			 colorRGB[2] = items[k].getIntValue();
			 k++;
			 continue;
		 }
		 if (lower_item == "visible")
		 {
			 bTrialTypes = false;
			 bOutcomes = false;
			 k++;
			 visible = items[k].getIntValue() > 0;
			 k++;
			 continue;
		 }
		 if (lower_item == "trialtypes")
		 {
			 k++;
			 bTrialTypes = true;
			 bOutcomes = false;
			 continue;
		 }
		  if (lower_item == "outcomes")
		 {
			 k++;
			 bTrialTypes = false;
			 bOutcomes = true;
			 continue;
		 }
		 if (bOutcomes)
		 {
			 trialOutcomes.push_back(items[k].getIntValue());
			 k++;
		 } else  if (bTrialTypes) {
			 trialTypes.push_back(items[k].getIntValue());
			 k++;
		 } else {
			 // unknown parameter!
			 // skip ?
			 k++;
		 }

	 }
}

Condition::Condition(String Name, std::vector<int> types, std::vector<int> outcomes, double _postSec, double _preSec)
{
       name = Name;
       colorRGB[0] = colorRGB[1] = colorRGB[2] = 255;
       trialTypes = types;
       trialOutcomes = outcomes;
	   postSec = _postSec;
	   preSec = _preSec;
       visible = true;
}

/**********************************************/


ChannelPSTHs::ChannelPSTHs(int ID, TrialCircularBufferParams params_) : channelID(ID), params(params_)
{
	redrawNeeded = true;
	numTrials = 0;
}

void ChannelPSTHs::updateConditionsWithLFP(std::vector<int> conditionsNeedUpdating, std::vector<float> alignedLFP, std::vector<bool> valid, Trial *trial)
{
	numTrials++;
	if (conditionsNeedUpdating.size() == 0)
		return ;

	redrawNeeded = true;
	for (int k=0;k<conditionPSTHs.size();k++) {
		for (int j=0;j<conditionsNeedUpdating.size();j++) 
		{
			if (conditionPSTHs[k].conditionID == conditionsNeedUpdating[j]) 
			{
				// this condition needs to be updated.
				conditionPSTHs[k].updatePSTH(alignedLFP,valid);
			}
		}
	}
	// update individual trial PSTH

	// first, make sure we have enough memory allocated to hold all these trials...
	if (params.buildTrialsPSTH)
	{
		int modifiedTrialType;
		if (trial->type >= TTL_TRIAL_OFFSET)
		{
			modifiedTrialType = trial->type-TTL_TRIAL_OFFSET;
		}
		else
		{
			modifiedTrialType = trial->type+params.numTTLchannels;
		}

		if (modifiedTrialType >= trialPSTHs.size())
		{
			for (int k=trialPSTHs.size();k<=modifiedTrialType;k++)
			{
				// increase the size of trialPSTH
				trialPSTHs.push_back(PSTH(k,params,true));
			}
		}
		// now update
		trialPSTHs[modifiedTrialType].updatePSTH(alignedLFP,valid);
	}
}

bool ChannelPSTHs::isNewDataAvailable()
{
	return redrawNeeded;
}

void ChannelPSTHs::informPainted()
{
	redrawNeeded = false;
}

void ChannelPSTHs::getRange(float &xmin, float &xmax, float &ymin, float &ymax)
{
	float xMin = 1e10;
	float xMax = -1e10;
	float yMin = 1e10;
	float yMax = -1e10;
	for (int k=0;k<conditionPSTHs.size();k++) 
	{
		if (conditionPSTHs[k].numTrials > 0 && conditionPSTHs[k].visible)
		{
			conditionPSTHs[k].getRange(xMin,xMax,yMin,yMax);
			xmin = MIN(xmin, xMin);
			ymin = MIN(ymin, yMin);
			xmax = MAX(xmax, xMax);
			ymax = MAX(ymax, yMax);
		}

	}
}


void ChannelPSTHs::clearStatistics()
{
	for (int k=0;k<conditionPSTHs.size();k++)
	{
		conditionPSTHs[k].clear();
	}
	trialPSTHs.clear();
	numTrials=0;
}

/***********************************************/
UnitPSTHs::UnitPSTHs(int ID,TrialCircularBufferParams params_, uint8 R, uint8 G, uint8 B): 
	unitID(ID), params(params_), spikeBuffer(params_.maxTrialTimeSeconds, params_.maxTrialsInMemory, params_.sampleRate)
{
	colorRGB[0] = R;
	colorRGB[1] = G;
	colorRGB[2] = B;
	redrawNeeded = false;
	numTrials = 0;
	uniqueIntervalID = -1;
}

void UnitPSTHs::startUniqueInterval(int uniqueCode)
{
	uniqueIntervalID = uniqueCode;
}

void UnitPSTHs::stopUniqueInterval()
{
	uniqueIntervalID = -1;
}

int UnitPSTHs::getUniqueInterval()
{
	return uniqueIntervalID;
}

	
void UnitPSTHs::informPainted()
{
	redrawNeeded = false;
}

void UnitPSTHs::addTrialStartToSmartBuffer(Trial *t)
{
	spikeBuffer.addTrialStartToBuffer(t);
}

void UnitPSTHs::clearStatistics()
{
	for (int k=0;k<conditionPSTHs.size();k++)
	{
		conditionPSTHs[k].clear();
	}
	trialPSTHs.clear();
	numTrials = 0;
}

void UnitPSTHs::addSpikeToBuffer(int64 spikeTimestampSoftware, int64 spikeTimestampHardware)
{
	spikeBuffer.addSpikeToBuffer(spikeTimestampSoftware,spikeTimestampHardware);
}

bool UnitPSTHs::isNewDataAvailable()
{
	return redrawNeeded;
}

void UnitPSTHs::getRange(float &xmin, float &xmax, float &ymin, float &ymax)
{
	xmin = 1e10;
	xmax = -1e10;
	ymax = -1e10;
	ymin = 1e10;
	float minX, maxX, maxY, minY;
	for (int k=0;k<conditionPSTHs.size();k++) {
		if (conditionPSTHs[k].visible)
		{
			conditionPSTHs[k].getRange(minX, maxX, minY, maxY);
			xmin = MIN(xmin, minX);
			xmax = MAX(xmax, maxX);
			ymax = MAX(ymax, maxY);
			ymin = MIN(ymin, minY);
		}
	}
}


void UnitPSTHs::updateConditionsWithSpikes(std::vector<int> conditionsNeedUpdating, Trial* trial)
{
	redrawNeeded = true;
	numTrials++;
	if (conditionsNeedUpdating.size() == 0)
		return ;

	tictoc.Tic(14);

	for (int k=0;k<conditionPSTHs.size();k++) {
		//std::cout << k << std::endl;
		for (int j=0;j<conditionsNeedUpdating.size();j++) 
		{
			if (conditionPSTHs[k].conditionID == conditionsNeedUpdating[j]) 
			{
				// this condition needs to be updated.
				conditionPSTHs[k].updatePSTH(&spikeBuffer, trial);
			}
		}
	}

	
	if (params.buildTrialsPSTH)
	{
		tictoc.Tic(15);
		// update individual trial PSTH
		int modifiedTrialType;
		if (trial->type >= TTL_TRIAL_OFFSET)
		{
			modifiedTrialType = trial->type-TTL_TRIAL_OFFSET;
		}
		else
		{
			modifiedTrialType = trial->type+params.numTTLchannels;
		}

		if (modifiedTrialType >= trialPSTHs.size())
		{
			for (int k=trialPSTHs.size();k<=modifiedTrialType;k++)
			{
				// increase the size of trialPSTH
				trialPSTHs.push_back(PSTH(k,params,true));
			}
		}
		// now update
		trialPSTHs[modifiedTrialType].updatePSTH(&spikeBuffer, trial);
		tictoc.Toc(15);
	}
	tictoc.Toc(14);
}

/********************/
Trial::Trial()
{
	outcome = type = -1;
	trialInProgress = false;
	hardwareAlignment = false;
	alignTS_hardware = trialID = startTS = alignTS = endTS = 0;
}

Trial::Trial(const Trial &t)
{
	outcome = t.outcome;
	type = t.type;
	trialID = t.trialID;
	startTS = t.startTS;
	alignTS = t.alignTS;
	endTS = t.endTS;
	trialInProgress = t.trialInProgress;
	hardwareAlignment = t.hardwareAlignment;
	alignTS_hardware = t.alignTS_hardware;
}
/***************************/
ElectrodePSTH::ElectrodePSTH()
{
	threadpool = nullptr;
}

ElectrodePSTH::ElectrodePSTH(int ID, String name) : electrodeID(ID), electrodeName(name)
{
	/*
	// create a thread pool
	int numCpus = SystemStats::getNumCpus();
	// create a thead pool to analyze incoming trials
	threadpool = new ThreadPool(numCpus);
	*/
}

ElectrodePSTH::~ElectrodePSTH()
{
	/*
	delete threadpool;
	threadpool = nullptr;
	*/
}

ElectrodePSTHlfpJob::ElectrodePSTHlfpJob(ElectrodePSTH *psth_, int ch_, std::vector<int> *conditionsNeedUpdate_, Trial *trial_, std::vector<float> *alignedLFP_, std::vector<bool> *valid_) : ThreadPoolJob("LFPjob"),
	psth(psth_),ch(ch_),conditionsNeedUpdate(conditionsNeedUpdate_),trial(trial_),alignedLFP(alignedLFP_),valid(valid_)
{
}

juce::ThreadPoolJob::JobStatus ElectrodePSTHlfpJob::runJob()
{
	psth->UpdateChannelConditionWithLFP(ch,conditionsNeedUpdate,trial,alignedLFP,valid);
	return jobHasFinished;
}

void ElectrodePSTH::UpdateChannelConditionWithLFP(int ch, std::vector<int> *conditionsNeedUpdate, Trial *trial, std::vector<float>* alignedLFP,std::vector<bool> *valid)
{
	channelsPSTHs[ch].updateConditionsWithLFP(*conditionsNeedUpdate, *alignedLFP, *valid, trial);
}


void ElectrodePSTH::updateChannelsConditionsWithLFP(std::vector<int> conditionsNeedUpdate, Trial *trial, SmartContinuousCircularBuffer *lfpBuffer)
{
	// compute trial aligned lfp for all channels 
	
	std::vector<bool> valid;
	std::vector<std::vector<float> > alignedLFP;

	tictoc.Tic(6);
	// resample all electrode channels 

	tictoc.Tic(18);
	bool success = lfpBuffer->getAlignedData(channels,trial,&channelsPSTHs[0].conditionPSTHs[0].binTime,
		channelsPSTHs[0].params, alignedLFP,valid);

	tictoc.Toc(18);
	// now we can average data
	tictoc.Tic(7);
	if (success)
	{
		for (int ch=0;ch<channelsPSTHs.size();ch++)
			{
				channelsPSTHs[ch].updateConditionsWithLFP(conditionsNeedUpdate, alignedLFP[ch], valid, trial);
			}
	//			ElectrodePSTHlfpJob *job = new ElectrodePSTHlfpJob(this,ch,&conditionsNeedUpdate,trial, &(alignedLFP[ch]), &valid);
	}
	tictoc.Toc(7);

	tictoc.Toc(6);

}

/****************************************/



void SmartContinuousCircularBuffer::addTrialStartToSmartBuffer(int trialID)
{
	smartPointerIndex[trialptr] = ptr;
	smartPointerTrialID[trialptr] = trialID;
	trialptr++;
	if (trialptr>=numTrials)
		trialptr = 0;
}

SmartContinuousCircularBuffer::SmartContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer) :
		ContinuousCircularBuffer(NumCh, SamplingRate, SubSampling, NumSecInBuffer)
{
	trialptr = 0;
	numTrials = 100;
	smartPointerIndex.resize(numTrials); // Number of trials to keep in memory
	smartPointerTrialID.resize(numTrials);
	for (int k=0;k<numTrials;k++)
	{
		smartPointerIndex[k] = 0;
		smartPointerTrialID[k] = 0;
	}
}

bool SmartContinuousCircularBuffer::getAlignedData(std::vector<int> channels, Trial *trial, std::vector<float> *timeBins,
												   TrialCircularBufferParams params,
									std::vector<std::vector<float> > &output,
									std::vector<bool> &valid)
{
	if (!params.approximate) 
	{
		// use this for buffers with gaps
		return getAlignedDataInterp(channels, trial, timeBins, params.preSec, params.postSec, output, valid);
	}

	// fast code.
	if (numSamplesInBuf <= 1 )
		return false;

	int numTimeBins = timeBins->size();

	output.resize(channels.size());
	valid.resize(numTimeBins);
	for (int ch=0;ch<channels.size();ch++)
	{
		output[ch].resize(numTimeBins);

		for (int j=0;j<numTimeBins;j++)
			output[ch][j] = 0;
	}
	for (int i=0;i<numTimeBins;i++)
	{
		valid[i] = false;
	}



	// 1. instead of searching the entire buffer, query when did the trial started....
	int k = 0;
	bool found = false;
	for (;k<numTrials;k++)
	{
		if (smartPointerTrialID[k] == trial->trialID)
		{
			found = true;
			break;
		}
	}
	if (!found) 
	{
		//jassertfalse;
		// couldn't find the trial !?!?!? buffer overrun?
		return false;
	}

	// now we have a handle where to search the data...

	int p=smartPointerIndex[k];
	
	// go backward and find the find time stamp we will need.
	int search_back_ptr = p;
	if (!trial->hardwareAlignment)
	{
		// software alignment is slightly harder. We actually need to search in the array (either forward / backward).
		// usually, this won't be more than a couple of samples....

		int next_index = search_back_ptr+1;
		if (next_index >= bufLen)
			next_index-=bufLen;

		if (softwareTS[search_back_ptr] < trial->alignTS && softwareTS[next_index] > trial->alignTS)
		{
			// perfect. nothing more to do!
		} else if (softwareTS[search_back_ptr] < trial->alignTS)
		{
			// search forward
			for (int q=0;q<numSamplesInBuf;q++) 
			{
				if (softwareTS[search_back_ptr] >= trial->alignTS) 
				{
					// we found the first sample prior to required trial alignment
					break;
				}
				search_back_ptr++;
				if (search_back_ptr >= bufLen)
					search_back_ptr=0;
			}
		} else 
		{
			// search backward
			for (int q=0;q<numSamplesInBuf;q++) 
			{
				if (softwareTS[search_back_ptr] <= trial->alignTS) 
				{
					// we found the first sample prior to required trial alignment
					break;
				}
				search_back_ptr--;
				if (search_back_ptr < 0)
					search_back_ptr=bufLen-1;
			}
		}
	} else {
		// hardware alignment is easy. hardwareTS always skips a fixed number.
		int64 hardwareTSatTrialStart = hardwareTS[p];
		// get an approximated hardware start...
		int move_indx = (trial->alignTS_hardware - hardwareTSatTrialStart) / subSampling;
		if (abs(move_indx) > bufLen )
		{
			// buffer overrun ?!?!?
			return false;
		}
		search_back_ptr += move_indx;
		if (search_back_ptr < 0)
			search_back_ptr += bufLen;
		if (search_back_ptr >= bufLen)
			search_back_ptr -= bufLen;
		jassert(search_back_ptr >= 0 && search_back_ptr < bufLen);
	}

	// consider search_back_ptr the buffer index pointing at time t=0
	float t0 = hardwareTS[search_back_ptr];
	int t0_indx = search_back_ptr;

	float trial_length_sec = float(trial->endTS-trial->alignTS)/numTicksPerSecond;
	// Assume we have no gaps
	// and that hardware timestamp difference is always fixed....

	// now assign values....
	float timeBindx = (*timeBins)[1]-(*timeBins)[0];
	if (fabs(timeBindx-buffer_dx) < 1e-5)
	{
		// easiest & fastest. No interpolation needed!
		float dx = params.binResolutionMS / 1000.0f;
		int numPosBins = ceil((params.postSec + trial_length_sec) / dx); 
		int numNegBins = ceil(params.preSec / dx); 
		int numBinsToUpdate = 1 + numPosBins + numNegBins; // include bin "0"
		int start_index = t0_indx-numNegBins;
		if (start_index < 0)
			start_index += bufLen;

		for (int index=0;index<numBinsToUpdate;index++)
		{
			valid[index] = true;
			int actual_index = start_index+index;
			if (actual_index>=bufLen)
				actual_index-=bufLen;

			for (int ch=0;ch<channels.size();ch++)
			{
				float value = Buf[channels[ch]][actual_index];
				output[ch][index] =  value;
			}
		}

	} else 
	{
		// the desired time bins do not have a time difference that match the buffer time difference.
		// need to use bilinear interpolation.
		for (int i = 0;i < numTimeBins; i++)
		{
			float tSamlple = (*timeBins)[i];
			if (tSamlple > trial_length_sec + params.postSec)
			{
				// do not update  after trial ended
				break;
			}
			valid[i] = true;

			float wanted_index = t0_indx + tSamlple / buffer_dx;
			int index1 = floor(wanted_index);
			float frac = wanted_index-index1;

			if (index1 < 0)
				index1 += bufLen;
			if (index1 >= bufLen)
				index1 -= bufLen;

			int index2 = index1+1;
			if (index2 < 0)
				index2 += bufLen;
			if (index2 >= bufLen)
				index2 -= bufLen;

			for (int ch=0;ch<channels.size();ch++)
			{
				output[ch][i] =  Buf[channels[ch]][index1] * (1-frac) +  Buf[channels[ch]][index2] * (frac);
			}

		}
	}
	return true;
}


bool SmartContinuousCircularBuffer::getAlignedDataInterp(std::vector<int> channels, Trial *trial, std::vector<float> *timeBins,
												   float preSec, float postSec,
									std::vector<std::vector<float> > &output,
									std::vector<bool> &valid)
{
	// to update a condition's continuous data psth, we will first find 
	// data samples in the vicinity of the trial, and then interpolate at the
	// needed time bins.
	if (numSamplesInBuf <= 1 )
		return false;



	// Debugging code
	/*
	int64 min_hard=MAXINT64, max_hard=0, min_soft=MAXINT64, max_soft=0;
	for (int k=0;k<softwareTS.size();k++)
	{
		if (softwareTS[k] <= min_soft)
			min_soft = softwareTS[k];

		if (hardwareTS[k] <= min_hard)
			min_hard = hardwareTS[k];

	if (softwareTS[k] >= max_soft)
			max_soft = softwareTS[k];

		if (hardwareTS[k] >= max_hard)
			max_hard = hardwareTS[k];
	
	}
	int64 time_span_min = min_soft-trial->alignTS;
	int64 time_span_max = max_soft-trial->alignTS;

	int64 time_span_min_h = min_hard-trial->alignTS_hardware;
	int64 time_span_max_h = max_hard-trial->alignTS_hardware;
	*/

	int numTimeBins = timeBins->size();

	output.resize(channels.size());
	valid.resize(numTimeBins);
	for (int ch=0;ch<channels.size();ch++)
	{
		output[ch].resize(numTimeBins);

		for (int j=0;j<numTimeBins;j++)
			output[ch][j] = 0;
	}
	for (int i=0;i<numTimeBins;i++)
	{
		valid[i] = false;
	}

	// 1. instead of searching the entire buffer, query when did the trial started....
	int k = 0;
	bool found = false;
	for (;k<numTrials;k++)
	{
		if (smartPointerTrialID[k] == trial->trialID)
		{
			found = true;
			break;
		}
	}
	if (!found) 
	{
		//jassertfalse;
		// couldn't find the trial !?!?!? buffer overrun?
		return false;
	}

	// now we have a handle where to search the data...

	int p=smartPointerIndex[k];
	
	// go backward and find the find time stamp we will need.
	//int nSamples = 1;
	int search_back_ptr = p;
	for (int q=0;q<numSamplesInBuf;q++) 
	{
		if ((!trial->hardwareAlignment && (softwareTS[search_back_ptr] < trial->alignTS - int64(preSec*numTicksPerSecond))) ||
			(trial->hardwareAlignment && (hardwareTS[search_back_ptr] < trial->alignTS_hardware - int64(preSec*samplingRate))))
		{
			// we found the first sample prior to required trial alignment
			break;
		}
		search_back_ptr--;
		//nSamples++;
		if (search_back_ptr < 0)
			search_back_ptr=bufLen-1;
	}

	// go forward and find the last time stamp we will need.
	/*
	int nSamples = 0;
	int search_forward_ptr = p;
	for (int q=0;q<numSamplesInBuf;q++) 
	{
		if (softwareTS[search_forward_ptr] > trial->endTS + int64(postSec*numTicksPerSecond))
		{
			// we found the first sample prior to required trial alignment
			break;
		}
		nSamples++;
		search_forward_ptr++;
		if (search_forward_ptr == bufLen)
			search_forward_ptr=0;
	}
	*/

	// we would like to return the lfp, sampled at specific time bins
	// (typically, 1 ms resolution, which is an overkill).
	//

	// we know that softwareTS[search_back_ptr]-trialAlign < preSec.
	// and that softwareTS[search_back_ptr+1]-trialAlign > preSec.

	int index=search_back_ptr;

	int index_next=index+1;
	if (index_next >= bufLen)
		index_next = 0;

	float tA,tB;
	float trial_length_sec = float(trial->endTS-trial->alignTS)/numTicksPerSecond;

	if (trial->hardwareAlignment)
	{
		tA = float(hardwareTS[index]-trial->alignTS_hardware)/(float)samplingRate;
		tB = float(hardwareTS[index_next]-trial->alignTS_hardware)/(float)samplingRate;
	} else
	{
		tA = float(softwareTS[index]-trial->alignTS)/numTicksPerSecond;
		tB = float(softwareTS[index_next]-trial->alignTS)/numTicksPerSecond;
	}
	

	for (int i = 0;i < numTimeBins; i++)
	{
		float tSamlple = (*timeBins)[i];
		if (tSamlple > trial_length_sec + postSec)
		{
			// do not update  after trial ended
			break;
		}
		float dA = tSamlple-tA;
		float dB = tB-tSamlple;
		float fracA = dA/(dA+dB);
		valid[i] = true;
		for (int ch=0;ch<channels.size();ch++)
		{
			output[ch][i] =  Buf[channels[ch]][index] * (1-fracA) +  Buf[channels[ch]][index_next] * (fracA);
		}
		// now advance pointers if needed
		if (i < numTimeBins-1) 
		{
			float tSamlple_next = (*timeBins)[i+1];
			int cnt = 0;
			while (cnt < bufLen)
			{
				if (tA <= tSamlple_next & tB > tSamlple_next)
				{
					break;
				} else
				{
					index++;
					if (index >= bufLen)
						index = 0;
					index_next=index+1;
					if (index_next >= bufLen)
						index_next = 0;

					if (trial->hardwareAlignment)
					{
						tA = float(hardwareTS[index]-trial->alignTS_hardware)/samplingRate;
						tB = float(hardwareTS[index_next]-trial->alignTS_hardware)/samplingRate;
					} else
					{
						tA = float(softwareTS[index]-trial->alignTS)/numTicksPerSecond;
						tB = float(softwareTS[index_next]-trial->alignTS)/numTicksPerSecond;
					}

				}
				cnt++;
			}

			if (cnt == bufLen) 
			{
				// missing data. This can happen when we just add a channel and a trial is in progress?!?!?
				return false; 
			}
		}
	}
	return true;
}

/*************************/
SmartSpikeCircularBuffer::SmartSpikeCircularBuffer(float maxTrialTimeSeconds, int _maxTrialsInMemory, int _sampleRateHz)
{
	jassert(maxTrialTimeSeconds > 0);
	sampleRateHz = _sampleRateHz;
	int MaxFiringRateHz = 300;
	maxTrialsInMemory = _maxTrialsInMemory;
	bufferSize = MaxFiringRateHz * maxTrialTimeSeconds * maxTrialsInMemory;
	jassert(bufferSize > 0);
	if (bufferSize == 0)
	{
		int dbg = 1;
	}

	bufferIndex = 0;
	trialIndex = 0;
	numSpikesStored = 0;
	numTrialsStored = 0;
	spikeTimesSoftware.resize(bufferSize);
	spikeTimesHardware.resize(bufferSize);
	for (int k=0;k<bufferSize;k++) 
	{
		spikeTimesSoftware[k] = 0;
		spikeTimesHardware[k] = 0;
	}

	trialID.resize(maxTrialsInMemory);
	pointers.resize(maxTrialsInMemory);
	for (int k=0;k<maxTrialsInMemory;k++)
	{
		trialID[k] = pointers[k] = 0;
	}
	jassert(trialID.size() > 0);
	
}

void SmartSpikeCircularBuffer::addSpikeToBuffer(int64 spikeTimeSoftware,int64 spikeTimeHardware)
{
	jassert(bufferSize > 0);
	spikeTimesHardware[bufferIndex] = spikeTimeHardware;
	spikeTimesSoftware[bufferIndex] = spikeTimeSoftware;
	bufferIndex = (bufferIndex+1) % bufferSize;
	numSpikesStored++;
	if (numSpikesStored>bufferSize)
		numSpikesStored=numSpikesStored;
}


void SmartSpikeCircularBuffer::addTrialStartToBuffer(Trial *t)
{
	jassert(trialIndex >= 0);
	trialID[trialIndex] = t->trialID;
	pointers[trialIndex] = bufferIndex;
	trialIndex = (trialIndex+1) % maxTrialsInMemory;
	numTrialsStored++;
	if (numTrialsStored > maxTrialsInMemory)
		numTrialsStored = maxTrialsInMemory;
}

int SmartSpikeCircularBuffer::queryTrialStart(int ID)
{
	for (int k=0;k<numTrialsStored;k++) 
	{
		int whereToLook = trialIndex-1-k;
		if (whereToLook < 0)
			whereToLook += maxTrialsInMemory;

		if (trialID[whereToLook] == ID)
			return pointers[whereToLook];
	}
	// trial not found?!!?!?
	return -1;
}



std::vector<int64> SmartSpikeCircularBuffer::getAlignedSpikes(Trial *trial, float preSecs, float postSecs)
{
	// we need to update the average firing rate with the spikes that were stored in the spike buffer.
	// first, query spike buffer where does the trial start....
	jassert(spikeTimesSoftware.size() > 0);
	std::vector<int64> alignedSpikes;
	Time t;
	int64 ticksPerSec = t.getHighResolutionTicksPerSecond();
	int64 numTicksPreTrial =preSecs * ticksPerSec;
	int64 numTicksPostTrial =postSecs * ticksPerSec;

	int64 samplesToTicks = 1.0/float(sampleRateHz) * ticksPerSec;

	int saved_ptr = queryTrialStart(trial->trialID);
	if (saved_ptr < 0)
		return alignedSpikes; // trial is not in memory??!?


	// return all spikes within a given interval aligned to AlignTS
	// Use ptr as a search reference in the buffer
	// The interval is defined as Start_TS-BeforeSec .. End_TS+AfterSec
	
		// Search Backward
		int CurrPtr = saved_ptr;
		int  N = 0;
		while (N < numSpikesStored)
		{
			if (spikeTimesSoftware[CurrPtr] < trial->startTS-numTicksPreTrial || spikeTimesSoftware[CurrPtr] > trial->endTS+numTicksPostTrial) 
				break;
			// Add spike..
			if (trial->hardwareAlignment)
				// convert from samples to ticks...
				alignedSpikes.push_back( (spikeTimesHardware[CurrPtr]-trial->alignTS_hardware)*samplesToTicks);
			else
				alignedSpikes.push_back(spikeTimesSoftware[CurrPtr]-trial->alignTS);

			CurrPtr--;
			N++;
			if (CurrPtr < 0)
				CurrPtr = bufferSize-1;
		}
		// Now Search Forward
		CurrPtr = saved_ptr + 1;

		while (N < numSpikesStored)
		{
			if (CurrPtr >= bufferSize)
				CurrPtr = 0;

			if (spikeTimesSoftware[CurrPtr] > trial->endTS + numTicksPostTrial || CurrPtr==bufferIndex)
				break;
			// Add spike..
			if (spikeTimesSoftware[CurrPtr] - trial->startTS >= -numTicksPreTrial)
			{

				if (trial->hardwareAlignment)
					alignedSpikes.push_back( (spikeTimesHardware[CurrPtr] - trial->alignTS_hardware) * samplesToTicks);
				else
					alignedSpikes.push_back(spikeTimesSoftware[CurrPtr] - trial->alignTS);

				N++;
			}
			CurrPtr++;

		}
	

	std::sort(alignedSpikes.begin(),alignedSpikes.begin()+alignedSpikes.size());
	return alignedSpikes;
}

/**********************/

TrialCircularBuffer::~TrialCircularBuffer()
{
	//delete lfpBuffer;
	//lfpBuffer = nullptr;
	//delete ttlBuffer;
	//ttlBuffer = nullptr;
	electrodesPSTH.clear();
	//delete threadpool;
	//threadpool = nullptr;
}

TrialCircularBuffer::TrialCircularBuffer()
{
	lfpBuffer = ttlBuffer = nullptr;
	threadpool = nullptr;
	hardwareTriggerAlignmentChannel = -1;
	lastSimulatedTrialTS = 0;
	lastTrialID = 0;
	uniqueIntervalID = 0;
	useThreads = true;
}

TrialCircularBuffer::TrialCircularBuffer(TrialCircularBufferParams params_) : params(params_)
{
	Time t;
	numTicksPerSecond = t.getHighResolutionTicksPerSecond();
	useThreads = false;
	conditionCounter = 0;
	firstTime = true;
	trialCounter = 0;
	lastSimulatedTrialTS = 0;
	lastTrialID = 0;
	hardwareTriggerAlignmentChannel = -1;
	uniqueIntervalID = 0;
	// sampling them should be at least 600 Hz (Nyquist!)
	// We typically sample everything at 30000, so a sub-sampling by a factor of 50 should be good
	int subSample = params.sampleRate/ params.desiredSamplingRateHz;
	float numSeconds = 2*(params.maxTrialTimeSeconds+params.preSec+params.postSec);
	lfpBuffer = new SmartContinuousCircularBuffer(params.numChannels, params.sampleRate, subSample, numSeconds);
	ttlBuffer = new SmartContinuousCircularBuffer(params.numTTLchannels, params.sampleRate, subSample, numSeconds);
	lastTTLts.resize(params.numTTLchannels);
	ttlChannelStatus.resize(params.numTTLchannels);
	for (int k=0;k<params.numTTLchannels;k++) {
		ttlChannelStatus[k] = false;
		lastTTLts[k] = 0;
	}
	int numCpus = SystemStats::getNumCpus();
	// create a thead pool to analyze incoming trials
	if (useThreads)
		threadpool = new ThreadPool(numCpus);

	clearDesign();
}

void TrialCircularBuffer::getLastTrial(int electrodeIndex, int channelIndex, int conditionIndex, float &x0, float &dx, std::vector<float> &y)
{
	const ScopedLock myScopedLock (psthMutex);
	//lockPSTH();
	x0 = electrodesPSTH[electrodeIndex].channelsPSTHs[channelIndex].conditionPSTHs[conditionIndex].binTime[0];
	dx = electrodesPSTH[electrodeIndex].channelsPSTHs[channelIndex].conditionPSTHs[conditionIndex].getDx();
	y = electrodesPSTH[electrodeIndex].channelsPSTHs[channelIndex].conditionPSTHs[conditionIndex].getLastTrial();
	//unlockPSTH();
}

Condition TrialCircularBuffer::getCondition(int conditionIndex)
{
	return conditions[conditionIndex];
}

int TrialCircularBuffer::getNumTrialsInCondition(int electrodeIndex, int channelIndex, int conditionIndex)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);
	int N = electrodesPSTH[electrodeIndex].channelsPSTHs[channelIndex].conditionPSTHs[conditionIndex].numTrials;
	//unlockPSTH();
	return N;
}

int TrialCircularBuffer::getNumConditions()
{
	return conditions.size();
}

int TrialCircularBuffer::getNumUnitsInElectrode(int electrodeIndex)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);
	int N = electrodesPSTH[electrodeIndex].unitsPSTHs.size();
	//unlockPSTH();
	return N;
}

int TrialCircularBuffer::getUnitID(int electrodeIndex, int unitIndex)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);
	int N= electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].unitID;
	//unlockPSTH();
	return N;
}

int TrialCircularBuffer::getNumElectrodes()
{
	return electrodesPSTH.size();
}

int TrialCircularBuffer::getElectrodeID(int index)
{
	return electrodesPSTH[index].electrodeID;
}

std::vector<int> TrialCircularBuffer::getElectrodeChannels(int e)
{
	return electrodesPSTH[e].channels;
}

String TrialCircularBuffer::getElectrodeName(int e)
{
	return electrodesPSTH[e].electrodeName;
}


TrialCircularBufferParams TrialCircularBuffer::getParams()
{
	return params;
}

void TrialCircularBuffer::setHardwareTriggerAlignmentChannel(int k)
{
	hardwareTriggerAlignmentChannel = k;
}



/*
void TrialCircularBuffer::lockPSTH()
{
	psthMutex.enter();
}

void TrialCircularBuffer::unlockPSTH()
{
	psthMutex.exit();
}


void TrialCircularBuffer::lockConditions()
{
	conditionMutex.enter();
}

void TrialCircularBuffer::unlockConditions()
{
	conditionMutex.exit();
}
*/

void TrialCircularBuffer::addDefaultTTLConditions(Array<bool> visibility)
{
	  int numExistingConditions = conditions.size();
	  for (int channel = 0; channel < params.numTTLchannels; channel++)
	  {
		  StringTS simulatedConditionString;

		  if (visibility[channel])
			simulatedConditionString = StringTS("addcondition name ttl"+String(channel+1)+" trialtypes "+String(TTL_TRIAL_OFFSET+channel)+" visible 1");
		  else
		    simulatedConditionString = StringTS("addcondition name ttl"+String(channel+1)+" trialtypes "+String(TTL_TRIAL_OFFSET+channel)+" visible 0");

		   std::vector<String> input = simulatedConditionString.splitString(' ');
 		  Condition newcondition(input,numExistingConditions+1+channel);
		  //lockConditions();
		  //const ScopedLock myScopedLock (conditionMutex);
		  const ScopedLock myScopedLock (psthMutex);
		  newcondition.conditionID = ++conditionCounter;
		  conditions.push_back(newcondition);
		  //unlockConditions();
		  // now add a new psth for this condition for all sorted units on all electrodes
		  //lockPSTH();
		  for (int i=0;i<electrodesPSTH.size();i++) 
		  {
			  for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
			  {
				  electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.push_back(PSTH(newcondition.conditionID, params,visibility[channel]));
			  }

			  for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			  {
				  electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.push_back(PSTH(newcondition.conditionID,params,visibility[channel]));
			  }
		  }
		  //unlockPSTH();
		  
	  }

}

void TrialCircularBuffer::clearAll()
{
	const ScopedLock myScopedLock (psthMutex);
	//lockPSTH();
	for (int i = 0; i < electrodesPSTH.size(); i++) 
	{
		for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].redrawNeeded = true;
			electrodesPSTH[i].channelsPSTHs[ch].clearStatistics();
		}

		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].redrawNeeded = true;
			electrodesPSTH[i].unitsPSTHs[u].clearStatistics();
		}
	}
	//unlockPSTH();

}

void TrialCircularBuffer::clearDesign()
{
	//lockConditions();
	//const ScopedLock myScopedLock (conditionMutex);
	const ScopedLock myScopedLock (psthMutex);
	// keep ttl visibility status
	Array<bool> ttlVisible;
	if (conditions.size() > 0)
	{
		for (int k=0;k<params.numTTLchannels;k++)
			ttlVisible.add(conditions[k].visible);
	} else
	{
		for (int k=0;k<params.numTTLchannels;k++)
			ttlVisible.add(false);
	}

	conditions.clear();
	conditionCounter = 0;
	//unlockConditions();
	// clear conditions from all units
	//lockPSTH();
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.clear();
		}

		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.clear();
		}
	}
	dropOutcomes.clear();
	//unlockPSTH();
	if (params.autoAddTTLconditions)
		addDefaultTTLConditions(ttlVisible);
}

void TrialCircularBuffer::modifyConditionVisibilityusingConditionID(int condID, bool newstate)
{
	//lockConditions();
	//const ScopedLock myScopedLock (conditionMutex);
	const ScopedLock myScopedLock (psthMutex);

	for (int k=0;k<conditions.size();k++) {
		if (conditions[k].conditionID == condID)
		{
			modifyConditionVisibility(k, newstate);
			break;
		}
	}
	//unlockConditions();
}

void TrialCircularBuffer::modifyConditionVisibility(int cond, bool newstate)
{
	// now add a new psth for this condition for all sorted units on all electrodes
	const ScopedLock myScopedLock (psthMutex);

//	lockPSTH();
	conditions[cond].visible = newstate;

	for (int i = 0; i < electrodesPSTH.size(); i++) 
	{
		for (int ch = 0; ch < electrodesPSTH[i].channelsPSTHs.size(); ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[cond].visible = newstate;
			electrodesPSTH[i].channelsPSTHs[ch].redrawNeeded = true;
		}

		for (int u = 0; u < electrodesPSTH[i].unitsPSTHs.size(); u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[cond].visible = newstate;
			electrodesPSTH[i].unitsPSTHs[u].redrawNeeded = true;
		}
	}
//	unlockPSTH();
}

void TrialCircularBuffer::toggleConditionVisibility(int cond)
{
	// now add a new psth for this condition for all sorted units on all electrodes
	const ScopedLock myScopedLock (psthMutex);

	//lockPSTH();
	conditions[cond].visible = !conditions[cond].visible;

	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		for (int ch=0;ch<electrodesPSTH[i].channelsPSTHs.size();ch++)
		{
			electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[cond].visible = !electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs[cond].visible;
			electrodesPSTH[i].channelsPSTHs[ch].redrawNeeded = true;
		}

		for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
		{
			electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[cond].visible = !electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs[cond].visible;
			electrodesPSTH[i].unitsPSTHs[u].redrawNeeded = true;
		}
	}
	//unlockPSTH();
	// inform editor repaint is needed

}

void TrialCircularBuffer::channelChange(int electrodeID, int channelindex, int newchannel)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	for (int i=0;i<electrodesPSTH.size();i++) 
	{
		if (electrodesPSTH[i].electrodeID == electrodeID)
		{
			electrodesPSTH[i].channels[channelindex] = newchannel;
			electrodesPSTH[i].channelsPSTHs[channelindex].clearStatistics();
			for (int k=0;k<electrodesPSTH[i].unitsPSTHs.size();k++)
			{
				electrodesPSTH[i].unitsPSTHs[k].clearStatistics();
			}
		}
	}
	
	//unlockConditions();
	//unlockPSTH();
}

void TrialCircularBuffer::syncInternalDataStructuresWithSpikeSorter(Array<Electrode *> electrodes)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	// note. This will erase all existing internal structures. Only call this in the constructor.
	electrodesPSTH.clear();

	for (int electrodeIter = 0; electrodeIter < electrodes.size(); electrodeIter++)
	{

		ElectrodePSTH electrodePSTH(electrodes[electrodeIter]->electrodeID,electrodes[electrodeIter]->name);
		int numChannels = electrodes[electrodeIter]->numChannels;
		  
		for (int k = 0; k < numChannels; k++) {
			int channelID = electrodes[electrodeIter]->channels[k];
			electrodePSTH.channels.push_back(channelID);
			ChannelPSTHs channelPSTH(channelID,params);
			// add all known conditions
			for (int c=0;c<conditions.size();c++)
			{
				channelPSTH.conditionPSTHs.push_back(PSTH(conditions[c].conditionID,params,conditions[c].visible));
			}
			electrodePSTH.channelsPSTHs.push_back(channelPSTH);
		  }

		  // add all known units
		  std::vector<BoxUnit> boxUnits = electrodes[electrodeIter]->spikeSort->getBoxUnits();
		  std::vector<PCAUnit> pcaUnits = electrodes[electrodeIter]->spikeSort->getPCAUnits();
		  for (int boxIter = 0; boxIter < boxUnits.size(); boxIter++)
		  {

			  int unitID = boxUnits[boxIter].UnitID;
			  UnitPSTHs unitPSTHs(unitID, params,boxUnits[boxIter].ColorRGB[0],
				  boxUnits[boxIter].ColorRGB[1],boxUnits[boxIter].ColorRGB[2]);
			  for (int k=0;k<conditions.size();k++)
			  {
				  unitPSTHs.conditionPSTHs.push_back(PSTH(conditions[k].conditionID,params,conditions[k].visible));
			  }
			  electrodePSTH.unitsPSTHs.push_back(unitPSTHs);
		  }

		  for (int pcaIter = 0; pcaIter < pcaUnits.size(); pcaIter++)
		  {

			  int unitID = pcaUnits[pcaIter].UnitID;
			  UnitPSTHs unitPSTHs(unitID, params,pcaUnits[pcaIter].ColorRGB[0],
				  pcaUnits[pcaIter].ColorRGB[1],pcaUnits[pcaIter].ColorRGB[2]);
			  for (int k = 0; k < conditions.size(); k++)
			  {
				  unitPSTHs.conditionPSTHs.push_back(PSTH(conditions[k].conditionID, params,conditions[k].visible));
			  }
			  electrodePSTH.unitsPSTHs.push_back(unitPSTHs);
		  }
		  electrodesPSTH.push_back(electrodePSTH);
	}
	//unlockConditions();
	//unlockPSTH();
}

void TrialCircularBuffer::addNewElectrode(Electrode *electrode)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);

	ElectrodePSTH e(electrode->electrodeID,electrode->name);
	int numChannels = electrode->numChannels;

	for (int k = 0; k < numChannels; k++) 
	{
		int channelID = electrode->channels[k];
		e.channels.push_back(channelID);
		ChannelPSTHs channelPSTH(channelID, params);
		// add all known conditions
		for (int c=0;c<conditions.size();c++)
		{
			channelPSTH.conditionPSTHs.push_back(PSTH(conditions[c].conditionID, params, conditions[c].visible));
		}
		e.channelsPSTHs.push_back(channelPSTH);	
	}
	electrodesPSTH.push_back(e);

	// Usually when we add a new electrode it doesn't have any units, unless it was added when loading an xml...
		if (electrode->spikeSort != nullptr)
		{
			std::vector<BoxUnit> boxUnits = electrode->spikeSort->getBoxUnits();
			for (int boxIter = 0; boxIter < boxUnits.size(); boxIter++)
			{
				addNewUnit(electrode->electrodeID, boxUnits[boxIter].UnitID, boxUnits[boxIter].ColorRGB[0],boxUnits[boxIter].ColorRGB[1],boxUnits[boxIter].ColorRGB[2]);
			}
			std::vector<PCAUnit> PcaUnits = electrode->spikeSort->getPCAUnits();
			for (int pcaIter = 0; pcaIter < PcaUnits.size(); pcaIter++)
			{
				addNewUnit(electrode->electrodeID, PcaUnits[pcaIter].UnitID, PcaUnits[pcaIter].ColorRGB[0],PcaUnits[pcaIter].ColorRGB[1],PcaUnits[pcaIter].ColorRGB[2]);
			}
		}
	//unlockPSTH();
}

void  TrialCircularBuffer::addNewUnit(int electrodeID, int unitID, uint8 r,uint8 g,uint8 b)
{
	// build a new PSTH for all defined conditions
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);

	UnitPSTHs unitPSTHs(unitID, params,r,g,b);
	for (int k = 0; k < conditions.size(); k++)
	{
		unitPSTHs.conditionPSTHs.push_back(PSTH(conditions[k].conditionID, params,conditions[k].visible));
	}
	for (int k = 0; k < electrodesPSTH.size(); k++) {
		if (electrodesPSTH[k].electrodeID == electrodeID) {
			electrodesPSTH[k].unitsPSTHs.push_back(unitPSTHs);
			break;
		}
	}
	//unlockPSTH();

}

void  TrialCircularBuffer::removeUnit(int electrodeID, int unitID)
{
  //lockPSTH();
  const ScopedLock myScopedLock (psthMutex);

  for (int e =0;e<electrodesPSTH.size();e++)
  {
	  if (electrodesPSTH[e].electrodeID == electrodeID)
	  {
		  for (int u=0;u<electrodesPSTH[e].unitsPSTHs.size();u++)
		  {
			  if (electrodesPSTH[e].unitsPSTHs[u].unitID == unitID)
			  {
				  electrodesPSTH[e].unitsPSTHs.erase(electrodesPSTH[e].unitsPSTHs.begin()+u);
			  }
		  }
	  }
  }
 // unlockPSTH();

}

void  TrialCircularBuffer::removeAllUnits(int electrodeID)
{
 const ScopedLock myScopedLock (psthMutex);

  for (int e =0;e<electrodesPSTH.size();e++)
  {
	  if (electrodesPSTH[e].electrodeID == electrodeID)
	  {
		  electrodesPSTH[e].unitsPSTHs.clear();
	  }
  }
}


void TrialCircularBuffer::removeElectrode(int electrodeID)
{
	const ScopedLock myScopedLock (psthMutex);

//	lockPSTH();
	for (int e =0;e<electrodesPSTH.size();e++)
	{
		if (electrodesPSTH[e].electrodeID == electrodeID)
		{
			electrodesPSTH.erase(electrodesPSTH.begin() + e);
			break;
		}
	}
//	unlockPSTH();
}

bool TrialCircularBuffer::parseMessage(StringTS msg)
  {
	  bool redrawNeeded = false;
	  std::vector<String> input = msg.splitString(' ');
	  String command = input[0].toLowerCase();

	  std::cout << "PSTH node received: " << command << std::endl;

	if (command == "tictoc_print")
	{
		if (useThreads)
			printf("******** Using threads to analyze trials\n");
		else 
			printf("******** NOT Using threads to analyze trials\n");

		tictoc.print();
	}
	if (command == "tictoc_clear")
	{
		tictoc.clear();
	} 
	if (command == "flip_thread_usage")
	{
		useThreads = !useThreads;
	}

 if (command == "trialstart")
	  {

	  	std::cout << "Got start of trial!" << std::endl;

		  currentTrial.trialID = ++trialCounter;
		  currentTrial.startTS = msg.timestamp;
		  currentTrial.alignTS = 0;
		  currentTrial.alignTS_hardware = 0;
		  currentTrial.endTS = 0;
		  currentTrial.hardwareAlignment = false;
  		  currentTrial.trialInProgress = true;
		  currentTrial.type = -1;
		  lfpBuffer->addTrialStartToSmartBuffer(currentTrial.trialID);
		  ttlBuffer->addTrialStartToSmartBuffer(currentTrial.trialID);
		  const ScopedLock myScopedLock (psthMutex);

		  //lockPSTH();
		  for (int i = 0; i < electrodesPSTH.size(); i++) 
			{
				for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
				{
					electrodesPSTH[i].unitsPSTHs[u].addTrialStartToSmartBuffer(&currentTrial);
				}
		  }
		  //unlockPSTH();
		  if (input.size() > 1) {
			  currentTrial.type = input[1].getIntValue();
		  }
	  } else if (command == "dropoutcomes")
	  {
		  dropOutcomes.clear();
		  for (int k = 1; k < input.size(); k++)
		  {
			  dropOutcomes.push_back(input[k].getIntValue());
		  }

	  } else if (command == "trialend") 
	  {
		  currentTrial.endTS = msg.timestamp;
		  currentTrial.trialInProgress = false;
		  if (input.size() > 1) {
			  currentTrial.outcome = input[1].getIntValue();
		  }


		  // look at trial outcome. If it is in the dropOutcomes list, drop the trial.
		  bool shouldDrop = false;
		  for (int k=0;k<dropOutcomes.size();k++)
		  {
			  if (dropOutcomes[k] == currentTrial.outcome)
			  {
				  shouldDrop = true;
				  break;
			  }
		  }

		  int64 maxTrialTicks = numTicksPerSecond * params.maxTrialTimeSeconds;
		  if (!shouldDrop && currentTrial.type >= 0 && currentTrial.startTS > 0 &&  ((currentTrial.endTS - currentTrial.startTS) <= maxTrialTicks))
		  {
			  if (currentTrial.alignTS == 0) 
			  {
				  currentTrial.alignTS = currentTrial.startTS;
			  }

			  double trialLength = (currentTrial.endTS-currentTrial.alignTS)/numTicksPerSecond;
			  if (trialLength <= params.maxTrialTimeSeconds)
				aliveTrials.push(Trial(currentTrial));
		  }
	  } else if (command == "trialtype")
	  {
		  if (input.size() > 1) {
			  currentTrial.type = input[1].getIntValue();
		  }
	  } else if (command == "trialoutcome")
	  {
		  if (input.size() > 1) {
			  currentTrial.outcome = input[1].getIntValue();
		  }
	  } else if (command == "trialalign")
	  {
		  currentTrial.alignTS = msg.timestamp;
	  }  else if (command == "newdesign")
	  {
		  //clearDesign();
		  designName = input[1];
		  redrawNeeded = true;
	  }
	  else if (command == "cleardesign")
	  {
	  		std::cout << "Clearing design" << std::endl;
			clearDesign();
			// inform editor repaint is needed
			redrawNeeded = true;
	  } else if (command == "addcondition")
	  {
		  int numExistingConditions = conditions.size();
		  Condition newcondition(input,numExistingConditions+1);

		  const ScopedLock myScopedLock (psthMutex);
		  //const ScopedLock myScopedLock (conditionMutex);

		  //lockConditions();
		  newcondition.conditionID = ++conditionCounter;
		  conditions.push_back(newcondition);
		  //unlockConditions();
		  // now add a new psth for this condition for all sorted units on all electrodes
		  //lockPSTH();
		  for (int i = 0; i < electrodesPSTH.size(); i++) 
		  {
			  for (int ch = 0; ch < electrodesPSTH[i].channelsPSTHs.size(); ch++)
			  {
				  electrodesPSTH[i].channelsPSTHs[ch].conditionPSTHs.push_back(PSTH(newcondition.conditionID, params, newcondition.visible));
			  }

			  for (int u = 0; u < electrodesPSTH[i].unitsPSTHs.size(); u++)
			  {
				  electrodesPSTH[i].unitsPSTHs[u].conditionPSTHs.push_back(PSTH(newcondition.conditionID, params, newcondition.visible));
			  }
		  }
		  //unlockPSTH();
		  // inform editor repaint is needed
		  redrawNeeded = true;

	  }
	  return   redrawNeeded ;
}

void TrialCircularBuffer::addSpikeToSpikeBuffer(SpikeObject newSpike)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);

	for (int e = 0; e < electrodesPSTH.size(); e++)
	{
		if (electrodesPSTH[e].electrodeID == newSpike.electrodeID)
		{
			for (int u = 0; u < electrodesPSTH[e].unitsPSTHs.size(); u++)
			{
				if (electrodesPSTH[e].unitsPSTHs[u].unitID == newSpike.sortedId)
				{
					electrodesPSTH[e].unitsPSTHs[u].addSpikeToBuffer(newSpike.timestamp_software,newSpike.timestamp);
					//unlockPSTH();
					return;
				}
			}
			
		}
	}
	// get got a sorted spike event before we got the information about the new unit?!?!?!
//	unlockPSTH();
}


bool TrialCircularBuffer::contains(std::vector<int> v, int x)
{
	for (int k = 0; k < v.size(); k++)
		if (v[k] == x)
			return true;
	return false;
}

void TrialCircularBuffer::updateLFPwithTrial(int electrodeIndex, std::vector<int> *conditionsNeedUpdate, Trial *trial)
{
	electrodesPSTH[electrodeIndex].updateChannelsConditionsWithLFP(*conditionsNeedUpdate,trial, lfpBuffer); 
}

void TrialCircularBuffer::updateSpikeswithTrial(int electrodeIndex, int unitIndex, std::vector<int> *conditionsNeedUpdate, Trial *trial)
{
	electrodesPSTH[electrodeIndex].unitsPSTHs[unitIndex].updateConditionsWithSpikes(*conditionsNeedUpdate,trial); 
}

void TrialCircularBuffer::updatePSTHwithTrial(Trial *trial)
{
	//printf("Calling updatePSTHwithTrial::lock conditions started\n");
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);
	//
	//lockConditions();
	//printf("Calling updatePSTHwithTrial::lock PSTH started\n");
	//lockPSTH();
	//printf("Finished updatePSTHwithTrial::lock PSTH started\n");
//	printf("Calling updatePSTHwithTrial::lock conditions finished \n");
	
	// find out which conditions need to be updated
	std::vector<int> conditionsNeedUpdating;
	for (int c=0;c<conditions.size();c++)
	{
		if (contains(conditions[c].trialTypes, trial->type) &&
			( (conditions[c].trialOutcomes.size() == 0) || 
			(conditions[c].trialOutcomes.size() > 0 && contains(conditions[c].trialOutcomes, trial->outcome))))
			conditionsNeedUpdating.push_back(conditions[c].conditionID);
	}

	if (conditionsNeedUpdating.size() == 0)
	{
		// none of the conditions match. nothing to update.
		//unlockPSTH();
		//unlockConditions();
		std::cout << "No updates needed." << std::endl;

		return;
	}



	if (!useThreads)
	{
		std::cout << "Updating without threads..." << std::endl;
		// these two parts can be fully distributed along several threads because they are completely independent.
		//printf("Calling updatePSTHwithTrial::update without threads\n");

		tictoc.Tic(23);
		for (int i=0;i<electrodesPSTH.size();i++) 
		{
			electrodesPSTH[i].updateChannelsConditionsWithLFP(conditionsNeedUpdating,trial, lfpBuffer); // timer 6 -> 7,18
		}
		// update both spikes and LFP PSTHs 
		for (int i=0;i<electrodesPSTH.size();i++) 
		{
			for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			{
				electrodesPSTH[i].unitsPSTHs[u].updateConditionsWithSpikes(conditionsNeedUpdating,trial); // timer 14,15
			}
		}
		tictoc.Toc(23);
		//printf("Finished updatePSTHwithTrial::update without threads\n");

	} else {

		std::cout << "Updating with threads..." << std::endl;
		tictoc.Tic(24);
		int cnt = 0;
		int numElectrodes = electrodesPSTH.size();
		//printf("Calling updatePSTHwithTrial::update with threads\n");

		for (int i = 0; i < numElectrodes; i++) 
		{
			TrialCircularBufferThread *job = new TrialCircularBufferThread(this,&conditionsNeedUpdating,trial,cnt++,0,i,-1);
			threadpool->addJob(job, true);
			for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			{
				TrialCircularBufferThread *job = new TrialCircularBufferThread(this,&conditionsNeedUpdating,trial,cnt++,1,i,u);
				threadpool->addJob(job, true);
			}
		}

		//printf("Calling updatePSTHwithTrial::Waiting for jobs\n");

		while (threadpool->getNumJobs() > 0)
		{
			#if JUCE_WINDOWS
				Sleep(2); // sleep 2 ms
			#else
				usleep(2000);
			#endif
		}
		tictoc.Toc(24);
		//printf("Finished updatePSTHwithTrial::Waiting for jobs\n");

	}

	//unlockPSTH();
	//unlockConditions();
}

void TrialCircularBuffer::reallocate(int numChannels)
{
	lfpBuffer->reallocate(numChannels);
	ttlBuffer->reallocate(numChannels);

}

void TrialCircularBuffer::simulateHardwareTrial(int64 ttl_timestamp_software,int64 ttl_timestamp_hardware, int trialType, float lengthSec)
{
	int64 tickdiff = ttl_timestamp_software- lastSimulatedTrialTS;
	float secElapsed = float(tickdiff) / numTicksPerSecond;
	if (secElapsed > params.ttlSupressionTimeSec)
	{
		std::cout << "Adding a new trial." << std::endl;
		const ScopedLock myScopedLock (psthMutex);
		//lockPSTH();
		Trial ttlTrial;
		ttlTrial.trialID = ++trialCounter;
		ttlTrial.startTS = ttl_timestamp_software;
		ttlTrial.alignTS = ttl_timestamp_software;
		ttlTrial.alignTS_hardware = ttl_timestamp_hardware;
		ttlTrial.endTS = ttl_timestamp_software + int64(lengthSec*numTicksPerSecond);
		ttlTrial.outcome = 0;
		ttlTrial.trialInProgress = false;
		ttlTrial.type = trialType;
		ttlTrial.hardwareAlignment = true;
		lfpBuffer->addTrialStartToSmartBuffer(ttlTrial.trialID);
		ttlBuffer->addTrialStartToSmartBuffer(ttlTrial.trialID);
		for (int i=0;i<electrodesPSTH.size();i++) 
		{
			for (int u=0;u<electrodesPSTH[i].unitsPSTHs.size();u++)
			{
				electrodesPSTH[i].unitsPSTHs[u].addTrialStartToSmartBuffer(&ttlTrial);
			}
		}
		aliveTrials.push(ttlTrial);
		lastSimulatedTrialTS = ttl_timestamp_software;
		//unlockPSTH();
	}
}

void TrialCircularBuffer::addTTLevent(int channel,int64 ttl_timestamp_software, int64 ttl_timestamp_hardware, bool rise, bool simulateTrial)
{
	// measure how much time passed since last ttl on this channel.
	// and only if its above some threshold, simulate a ttl trial.
	// this is useful when sending train of pulses and you are interested in aligning things just
	// to the first pulse.

	if (channel >= 0 && channel < lastTTLts.size())
	{

		std::cout << "Got that TTL event" << std::endl;

		ttlBuffer->update(channel, ttl_timestamp_software, ttl_timestamp_hardware, rise);

		if (params.reconstructTTL)
		{
			ttlStatus tmp;
			tmp.channel = channel;
			tmp.value = rise;
			tmp.ts = ttl_timestamp_hardware;
			ttlQueue.push(tmp);
		}

		if (simulateTrial)
		{
			int64 tickdiff = ttl_timestamp_software-lastTTLts[channel];
			float secElapsed = float(tickdiff) / numTicksPerSecond;
			if (secElapsed > params.ttlSupressionTimeSec)
			{
				// simulate a ttl trial 
				simulateHardwareTrial(ttl_timestamp_software,ttl_timestamp_hardware, TTL_TRIAL_OFFSET + channel, params.ttlTrialLengthSec);
			}
			lastTTLts[channel] = tickdiff;

			if (channel == hardwareTriggerAlignmentChannel)
			{

				//std::cout << "TTL channel matches alignment channel!" << std::endl;
				currentTrial.alignTS = ttl_timestamp_software; 
				currentTrial.alignTS_hardware = ttl_timestamp_hardware;
				currentTrial.hardwareAlignment = true;
			}
		}
	}
	  
}
	
std::vector<std::vector<bool>> TrialCircularBuffer::reconstructTTLchannels(int64 hardware_timestamp,int nSamples)
{
	std::vector<std::vector<bool>> contdata;
	contdata.resize(params.numTTLchannels);
	
	for (int k=0;k<params.numTTLchannels;k++)
	{
		contdata[k].resize(nSamples);
	}

	int64 currTS = hardware_timestamp;
	for (int i=0;i<nSamples;i++, currTS++)
	{
		bool repeat = true;
		while (repeat && ttlQueue.size() > 0)
		{
			ttlStatus tmp = ttlQueue.front();
			if (tmp.ts <= currTS)
			{
				jassert(tmp.channel >= 0 && tmp.channel < ttlChannelStatus.size());
				ttlChannelStatus[tmp.channel] = tmp.value;
				ttlQueue.pop();
			} else {
				repeat = false;
			}
		}

		for (int ch=0;ch<params.numTTLchannels;ch++)
		{
			contdata[ch][i] = ttlChannelStatus[ch];
		}
		
	}
	return contdata;
}


int TrialCircularBuffer::getNumberAliveTrials()
{
	return aliveTrials.size();
}

int TrialCircularBuffer::getLastTrialID()
{
	return lastTrialID;
}

std::vector<XYline> TrialCircularBuffer::getElectrodeConditionCurves(int electrodeID, int channelID)
{
	std::vector<XYline> lines;

	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);
	//lockPSTH();
	//lockConditions();
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					// great. we found our electrode.
					// now iterate over conditions and build lines.
					for (int cond=0;cond<electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs.size();cond++)
					{
						if (!electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].visible)
							continue;
						
						
						juce::Colour lineColor = juce::Colour(electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[0],
									 electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[1],
									 electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[2]);
						double x0 = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].binTime[0];
						double dx = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].getDx();
						std::vector<float> y = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].getAverageTrialResponse();
						XYline l(x0,dx,y, 1.0, lineColor);
						lines.push_back(l);
					}

					//unlockConditions();
					//unlockPSTH();
					return lines;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return lines;
}

int TrialCircularBuffer::getUnitUniqueInterval(int electrodeID, int unitID)
{
	const ScopedLock myScopedLock (psthMutex);

	//lockPSTH();
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					int id = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].uniqueIntervalID;
					//unlockPSTH();
					return id;
				}
			}
		}
	}
	//unlockPSTH();
	return -1;
}

int TrialCircularBuffer::setUnitUniqueInterval(int electrodeID, int unitID, bool state)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					int id = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].uniqueIntervalID;
					if (state) {
						uniqueIntervalID++;
						electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].uniqueIntervalID = uniqueIntervalID;
						id = uniqueIntervalID;
					}
					
					//unlockPSTH();
					return id;
				}
			}
		}
	}
	//unlockPSTH();
	return -1;
}

juce::Colour TrialCircularBuffer::getUnitColor(int electrodeID, int unitID)
{
	//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					juce::Colour col = juce::Colour(electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[0],
										electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[1],
										electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].colorRGB[2]);
					//unlockPSTH();
					return col ;
				}
			}
		}
	}
	//unlockPSTH();
	return juce::Colours::black;
}

std::vector<XYline> TrialCircularBuffer::getUnitConditionCurves(int electrodeID, int unitID)
{
	std::vector<XYline> lines;
	//lockPSTH();
	//lockConditions();
	const ScopedLock myScopedLock (psthMutex);

	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					// great. we found our unit.
					// now iterate over conditions and build lines.
					for (int cond=0;cond<electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs.size();cond++)
					{
						if (!electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].visible)
							continue;
						
						juce::Colour lineColor = juce::Colour(electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[0],
									 electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[1],
									 electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].colorRGB[2]);
						double x0 = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].binTime[0];
						double dx = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].getDx();
						std::vector<float> y = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].getAverageTrialResponse();
						XYline l(x0,dx,y, 1.0, lineColor);
						lines.push_back(l);
					}

					//unlockConditions();
					//unlockPSTH();
					return lines;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return lines;
}


void TrialCircularBuffer::getElectrodeConditionRange(int electrodeID, int channelID, double &xmin, double &xmax)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					// great. we found our electrode.
					// now iterate over conditions and build lines.
					xmin = 1e10;
					xmax = -1e10;
			
					for (int cond=0;cond<electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs.size();cond++)
					{
						if (!electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].visible)
							continue;

						float xmin_,xmax_,ymin_,ymax_;
						electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].conditionPSTHs[cond].getRange(xmin_,xmax_,ymin_,ymax_);
						xmin = MIN(xmin,xmin_);
						xmax = MAX(xmax,xmax_);
	
						}

					//unlockConditions();
					//unlockPSTH();
					return;
				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return;
}


void TrialCircularBuffer::getUnitConditionRange(int electrodeID, int unitID, double &xmin, double &xmax)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					// great. we found our unit.
					// now iterate over conditions and build lines.
					xmin = 1e10;
					xmax = -1e10;
					for (int cond=0;cond<electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs.size();cond++)
					{
						if (!electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].visible)
							continue;
						float xmin_,xmax_,ymin_,ymax_;
						electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].conditionPSTHs[cond].getRange(xmin_,xmax_,ymin_,ymax_);
						xmin = MIN(xmin,xmin_);
						xmax = MAX(xmax,xmax_);
					}

					//unlockConditions();
					//unlockPSTH();
					return;
				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
}

void TrialCircularBuffer::clearUnitStatistics(int electrodeID, int unitID)
{
	const ScopedLock myScopedLock (psthMutex);

	//lockPSTH();

	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].clearStatistics();
					//unlockPSTH();
					return;

				}
			}
		}
	}
	//unlockPSTH();
}

void TrialCircularBuffer::clearChanneltatistics(int electrodeID, int channelID)
{
	const ScopedLock myScopedLock (psthMutex);
//	lockPSTH();

	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].clearStatistics();
					//unlockPSTH();
					return;
				}
			}
		}
	}
	//unlockPSTH();
}


void TrialCircularBuffer::updateElectrodeName(int electrodeID, String newName)
{
//lockPSTH();
	const ScopedLock myScopedLock (psthMutex);

	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			electrodesPSTH[electrodeIndex].electrodeName = newName;
			//unlockPSTH();
			return;
		}
	}
	//unlockPSTH();
}

 std::vector<float> TrialCircularBuffer::buildSmoothKernel(float guassianStandardDeviationMS, float binSizeInMS)
{
	 std::vector<float> smoothKernel;
	// assume each bin correponds to one millisecond.
	// build the gaussian kernel
	int numKernelBins = 2*(int)(guassianStandardDeviationMS*3.5/binSizeInMS)+1; // +- 3.5 standard deviations.
	int zeroIndex = (numKernelBins-1)/2;
	smoothKernel.resize(numKernelBins); 
	float sumZ = 0;
	for (int k=0;k<numKernelBins;k++) 
	{
		float z = float(k*binSizeInMS-zeroIndex);
		smoothKernel[k] = exp(- (z*z)/(2*guassianStandardDeviationMS*guassianStandardDeviationMS));
		sumZ+=smoothKernel[k];
	}
	// normalize kernel
	for (int k=0;k<numKernelBins;k++) 
	{
		smoothKernel[k] /= sumZ;
	}
	return smoothKernel;
}

std::vector<float> TrialCircularBuffer::smooth(std::vector<float> y, std::vector<float> smoothKernel, int xmin, int xmax)
{
	std::vector<float> smoothy;
	smoothy.resize(xmax-xmin+1);

	int numKernelBins = smoothKernel.size();
	int zeroIndex = (numKernelBins-1)/2;
	int numXbins = y.size();
	for (int k=xmin;k<=xmax;k++)
	{
		float response = 0;
		for (int j=-zeroIndex;j<zeroIndex;j++)
		{
			if (k+j >=0 && k+j < numXbins)
				response+=y[k+j] * smoothKernel[j+zeroIndex];
		}
		smoothy[k-xmin] = response;
	}
	return  smoothy;
}
// Builds average raster matrix.
// each line corresponds to a trial type, and contains the average response for that trial type.
// each column corresponds to a specific time point, returned by x_time
// the number of trial types is returned, as well as the number of repetitions of each individual trial type.
// constrain output to be between xmin & xmax (approximately).
std::vector<std::vector<float>> TrialCircularBuffer::getTrialsAverageUnitResponse(int electrodeID, int unitID, 
	std::vector<float> &x_time, int &numTrialTypes, std::vector<int> &numTrialRepeats, double smoothMS, float xmin, float xmax)
{
	// allocate a matrix with average trial responses (not conditions!)
	std::vector<std::vector<float>> avgResponseMatrix;
	avgResponseMatrix.clear();
	x_time.clear();
	numTrialRepeats.clear();
	numTrialTypes = 0;

	std::vector<float> smoothKernel;
	if (smoothMS > 0)
		smoothKernel = buildSmoothKernel(smoothMS, 1.0);

	//lockPSTH();
	//lockConditions();
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					numTrialTypes = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs.size();
					if (numTrialTypes == 0)
						return avgResponseMatrix;

					int xminIndex = -1,xmaxIndex=-1;
					for (int t=0;t<electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[0].binTime.size();t++)
					{
						if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[0].binTime[t] >= xmin && xminIndex == -1)
						{
							xminIndex = t;
						}
						if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[0].binTime[t] >= xmax && xmaxIndex == -1)
						{
							xmaxIndex = t;
						}

					}
					// constrain output to be between [xminIndex,xmaxIndex]
					if (xmaxIndex-xminIndex < 10)
						return avgResponseMatrix; // no point displaying an image with less than 10 pixels ?

					numTrialRepeats.resize(numTrialTypes);

					x_time.resize(xmaxIndex-xminIndex+1);
					for (int t=0;t<xmaxIndex-xminIndex+1;t++)
					{
						x_time[t] = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[0].binTime[xminIndex+t];
					}

					avgResponseMatrix.resize(numTrialTypes);
					int numTimePoints = x_time.size();
					for (int trialIter = 0;trialIter < numTrialTypes;trialIter++)
					{
						numTrialRepeats[trialIter] = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[trialIter].numTrials;
						if (numTrialRepeats[trialIter] == 0)
						{
							avgResponseMatrix[trialIter].resize(numTimePoints);
							for (int q=0;q<numTimePoints;q++)
								avgResponseMatrix[trialIter][q] = 0;
						} else
						{

						if (smoothMS > 0)
							avgResponseMatrix[trialIter] = smooth(electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[trialIter].getAverageTrialResponse(), smoothKernel,xminIndex,xmaxIndex);
						else
							avgResponseMatrix[trialIter] = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs[trialIter].getAverageTrialResponse();
						}
						
					}

					//unlockConditions();
					//unlockPSTH();
					return avgResponseMatrix;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return avgResponseMatrix;
}




// Builds average raster matrix.
// each line corresponds to a trial type, and contains the average response for that trial type.
// each column corresponds to a specific time point, returned by x_time
// the number of trial types is returned, as well as the number of repetitions of each individual trial type.
// constrain output to be between xmin & xmax (approximately).
std::vector<std::vector<float>> TrialCircularBuffer::getTrialsAverageChannelResponse(int electrodeID, int channelID, 
	std::vector<float> &x_time, int &numTrialTypes, std::vector<int> &numTrialRepeats, double smoothMS, float xmin, float xmax)
{
	// allocate a matrix with average trial responses (not conditions!)
	std::vector<std::vector<float>> avgResponseMatrix;
	avgResponseMatrix.clear();
	x_time.clear();
	numTrialRepeats.clear();
	numTrialTypes = 0;

	std::vector<float> smoothKernel;
	if (smoothMS > 0)
		smoothKernel = buildSmoothKernel(smoothMS, 1.0);

	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					numTrialTypes = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs.size();
					if (numTrialTypes == 0)
						return avgResponseMatrix;

					int xminIndex = -1,xmaxIndex=-1;
					for (int t=0;t<electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[0].binTime.size();t++)
					{
						if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[0].binTime[t] >= xmin && xminIndex == -1)
						{
							xminIndex = t;
						}
						if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[0].binTime[t] >= xmax && xmaxIndex == -1)
						{
							xmaxIndex = t;
						}

					}
					// constrain output to be between [xminIndex,xmaxIndex]
					if (xmaxIndex-xminIndex < 10)
						return avgResponseMatrix; // no point displaying an image with less than 10 pixels ?

					numTrialRepeats.resize(numTrialTypes);

					x_time.resize(xmaxIndex-xminIndex+1);
					for (int t=0;t<xmaxIndex-xminIndex+1;t++)
					{
						x_time[t] = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[0].binTime[xminIndex+t];
					}

					avgResponseMatrix.resize(numTrialTypes);
					int numTimePoints = x_time.size();
					for (int trialIter = 0;trialIter < numTrialTypes;trialIter++)
					{
						numTrialRepeats[trialIter] = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[trialIter].numTrials;
						if (numTrialRepeats[trialIter] == 0)
						{
							avgResponseMatrix[trialIter].resize(numTimePoints);
							for (int q=0;q<numTimePoints;q++)
								avgResponseMatrix[trialIter][q] = 0;
						} else
						{

						if (smoothMS > 0)
							avgResponseMatrix[trialIter] = smooth(electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[trialIter].getAverageTrialResponse(), smoothKernel,xminIndex,xmaxIndex);
						else
							avgResponseMatrix[trialIter] = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs[trialIter].getAverageTrialResponse();
						}
						
					}

					//unlockConditions();
					//unlockPSTH();
					return avgResponseMatrix;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return avgResponseMatrix;
}
const uint8 jet_colors_64[64][3] = {
{0,0,143},
{0,0,159},
{0,0,175},
{0,0,191},
{0,0,207},
{0,0,223},
{0,0,239},
{0,0,255},
{0,16,255},
{0,32,255},
{0,48,255},
{0,64,255},
{0,80,255},
{0,96,255},
{0,112,255},
{0,128,255},
{0,143,255},
{0,159,255},
{0,175,255},
{0,191,255},
{0,207,255},
{0,223,255},
{0,239,255},
{0,255,255},
{16,255,239},
{32,255,223},
{48,255,207},
{64,255,191},
{80,255,175},
{96,255,159},
{112,255,143},
{128,255,128},
{143,255,112},
{159,255,96},
{175,255,80},
{191,255,64},
{207,255,48},
{223,255,32},
{239,255,16},
{255,255,0},
{255,239,0},
{255,223,0},
{255,207,0},
{255,191,0},
{255,175,0},
{255,159,0},
{255,143,0},
{255,128,0},
{255,112,0},
{255,96,0},
{255,80,0},
{255,64,0},
{255,48,0},
{255,32,0},
{255,16,0},
{255,0,0},
{239,0,0},
{223,0,0},
{207,0,0},
{191,0,0},
{175,0,0},
{159,0,0},
{143,0,0},
{128,0,0}};

// use a nice jet transform for pretty colors :)



int TrialCircularBuffer::getNumTrialTypesInChannel(int electrodeID, int channelID)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					int N= electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].trialPSTHs.size();
					//unlockConditions();
					//unlockPSTH();
					return N;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return 0;
}


int TrialCircularBuffer::getNumTrialTypesInUnit(int electrodeID, int unitID)
{

	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					int N= electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].trialPSTHs.size();
					//unlockConditions();
					//unlockPSTH();
					return N;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return 0;
}



int TrialCircularBuffer::getNumTrialsInChannel(int electrodeID, int channelID)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].channelsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].channelID == channelID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					int N = electrodesPSTH[electrodeIndex].channelsPSTHs[entryindex].numTrials;
					//unlockConditions();
					//unlockPSTH();
					return N;
				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return 0;
}



int TrialCircularBuffer::getNumTrialsInUnit(int electrodeID, int unitID)
{
	const ScopedLock myScopedLock (psthMutex);
	//const ScopedLock myScopedLock (conditionMutex);

	//lockPSTH();
	//lockConditions();
	bool fisrtTime = true;
	for (int electrodeIndex=0;electrodeIndex<electrodesPSTH.size();electrodeIndex++)
	{
		if (electrodesPSTH[electrodeIndex].electrodeID == electrodeID)
		{
			for (int entryindex = 0; entryindex < electrodesPSTH[electrodeIndex].unitsPSTHs.size();entryindex++)
			{
				if (electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].unitID == unitID)
				{
					// great. we found our unit.
					// now iterate over trial and build the matrix.
					int N = electrodesPSTH[electrodeIndex].unitsPSTHs[entryindex].numTrials;
					//unlockConditions();
					//unlockPSTH();
					return N;

				}
			}
		}
	}
	//unlockConditions();
	//unlockPSTH();
	return 0;
}

juce::Image TrialCircularBuffer::getTrialsAverageResponseAsJuceImage(int  ymin, int ymax,	std::vector<float> x_time,	int numTrialTypes,	std::vector<int> numTrialRepeats,	std::vector<std::vector<float>> trialResponseMatrix, float &maxValue)
{
	if (trialResponseMatrix.size() == 0)
	{
		int imageWidth = 200, imageHeight = 200;
		juce::Image I(juce::Image::PixelFormat::RGB, imageWidth, imageHeight, true);
		juce::Image::BitmapData bitmap(I,0,0,imageWidth,imageHeight,Image::BitmapData::ReadWriteMode::readWrite);
		return I;
	}

	// normalize response matrix to maximum across all trial types.
	maxValue = -1e10;
	double minValue = 1e10;
	for (int i=0;i<trialResponseMatrix.size();i++)
	{
		for (int j=0;j<trialResponseMatrix[i].size();j++)
		{
			maxValue=MAX(maxValue, trialResponseMatrix[i][j]);
			minValue=MIN(minValue, trialResponseMatrix[i][j]);
		}
	}
	int imageWidth = x_time.size();
	ymin = MAX(0,ymin);
	ymax = MIN(trialResponseMatrix.size()-1,ymax);

	int imageHeight = ymax-ymin+1;
	juce::Image I(juce::Image::PixelFormat::RGB, imageWidth, imageHeight, true);
	juce::Image::BitmapData bitmap(I,0,0,imageWidth,imageHeight,Image::BitmapData::ReadWriteMode::readWrite);
	if (maxValue < 1e-5)
		return I;

	uint8 *ptr = bitmap.data;
    
	
	for (int i=0;i<imageHeight;i++)
	{
		int yoffset = 3 * imageWidth;
		if (numTrialRepeats[ymin+i] == 0)
		{
			// special case, make all pixels black...
			// in other words - do nothing :)
		} else 
		{
			for (int j=0;j<imageWidth;j++)
			{
				float normalizedValue = (trialResponseMatrix[ymin+i][j]-minValue) / (maxValue-minValue);
				if (normalizedValue > 1) 
					normalizedValue = 1.0;
				int jetIndex = (int)(normalizedValue*63) % 64;
				// convert to a nice Jet RGB value.
				// just take the closest value...
				ptr[i * bitmap.lineStride + j * bitmap.pixelStride + 0] = jet_colors_64[jetIndex][2];
				ptr[i * bitmap.lineStride + j * bitmap.pixelStride + 1] = jet_colors_64[jetIndex][1];
				ptr[i * bitmap.lineStride + j * bitmap.pixelStride + 2] = jet_colors_64[jetIndex][0];
			}
		}
	}
	// finally, we can start storing values.

	return I;
}

juce::Image TrialCircularBuffer::getTrialsAverageChannelResponseAsJuceImage(int electrodeID, int channelID, float guassianStandardDeviationMS, float xmin, float xmax, int ymin, int ymax, float &maxValue)
{
	std::vector<float> x_time;
	int numTrialTypes;
	std::vector<int> numTrialRepeats;
	std::vector<std::vector<float>> trialResponseMatrix = getTrialsAverageChannelResponse(electrodeID, channelID, x_time, numTrialTypes, numTrialRepeats,guassianStandardDeviationMS,xmin, xmax);
	return getTrialsAverageResponseAsJuceImage(ymin,ymax,x_time,	numTrialTypes,	numTrialRepeats,	trialResponseMatrix, maxValue);
}

juce::Image TrialCircularBuffer::getTrialsAverageUnitResponseAsJuceImage(int electrodeID, int unitID, float guassianStandardDeviationMS, float xmin, float xmax, int ymin, int ymax, float &maxValue)
{
	std::vector<float> x_time;
	int numTrialTypes;
	std::vector<int> numTrialRepeats;
	std::vector<std::vector<float>> trialResponseMatrix = getTrialsAverageUnitResponse(electrodeID, unitID, x_time, numTrialTypes, numTrialRepeats,guassianStandardDeviationMS,xmin, xmax);
	return getTrialsAverageResponseAsJuceImage(ymin,ymax,x_time,	numTrialTypes,	numTrialRepeats,	trialResponseMatrix, maxValue);
}





void TrialCircularBuffer::process(AudioSampleBuffer& buffer,int nSamples,int64 hardware_timestamp,int64 software_timestamp)
{
	//printf("Entering TrialCircularBuffer::process\n");
	
	tictoc.Tic(0);

	// first, update LFP circular buffers
	//printf("Entering lfpBuffer->update\n");
	tictoc.Tic(1);
	lfpBuffer->update(buffer, hardware_timestamp,software_timestamp, nSamples);
	tictoc.Toc(1);
	//printf("Exiting lfpBuffer->update\n");

	//printf("Entering reconstructedTTLs\n");
	tictoc.Tic(2);

	// for oscilloscope purposes, it is easier to reconstruct TTL changes to "continuous" form.
	if (params.reconstructTTL) {
		std::vector<std::vector<bool>> reconstructedTTLs = reconstructTTLchannels(hardware_timestamp,nSamples);
		ttlBuffer->update(reconstructedTTLs,hardware_timestamp,software_timestamp,nSamples);
	}
	tictoc.Toc(2);

	//printf("Exiting reconstructedTTLs\n");

	// now, check if a trial finished, and enough time has elapsed so we also
	// have post trial information
	tictoc.Tic(3);
	Time t;
	long startTime = t.getHighResolutionTicks();
	double MaxPSTHprocessingTime = 50/1000; // 50 ms
	if (electrodesPSTH.size() > 0 && aliveTrials.size() > 0)
	{
		printf("Entering alive loop\n");

		std::cout << "Hardware timestamp: " << hardware_timestamp << std::endl;
		std::cout << "Software timestamp: " << software_timestamp << std::endl;
		for (int k=0;k<aliveTrials.size();k++)
		{
			Trial topTrial = aliveTrials.front();
			int64 ticksElapsed = software_timestamp - topTrial.endTS;
			std::cout << "Ticks elapsed: " << ticksElapsed << std::endl;
			float timeElapsedSec = float(ticksElapsed)/ numTicksPerSecond;
			std::cout << "Time elapsed: " << timeElapsedSec << std::endl;
			bool trialEndedAndEnoughDataInBuffer;

			if (!topTrial.hardwareAlignment)
			{
				//trialEndedAndEnoughDataInBuffer = timeElapsedSec > params.postSec + 0.1;
				trialEndedAndEnoughDataInBuffer = software_timestamp > topTrial.alignTS + (params.postSec + 0.1)*numTicksPerSecond;
			} else
			{
				 // add 100 ms (safety measure. jitter shouldn't be more than 3-4 ms)
				trialEndedAndEnoughDataInBuffer = hardware_timestamp+nSamples > topTrial.alignTS_hardware+ (params.postSec + 0.1)*params.sampleRate;
			}

			if (trialEndedAndEnoughDataInBuffer)
			{
				aliveTrials.pop();
				lastTrialID = topTrial.trialID;
				//printf("Entering updatePSTHwithTrial\n");
				tictoc.Tic(4);
				updatePSTHwithTrial(&topTrial);
				//std::cout << "Updating PSTH" << std::endl;
				tictoc.Toc(4);
				//printf("Exitting updatePSTHwithTrial\n");
			}

			long endTime = t.getHighResolutionTicks();

			std::cout << "End time: " << endTime << std::endl;
			
		//	if  ((endTime-startTime) > MaxPSTHprocessingTime*t.getHighResolutionTicksPerSecond())
				//break;
			

		}
		printf("Exiting alive loop\n");
	}
	tictoc.Toc(3);

	tictoc.Toc(0);
	//printf("Exitting TrialCircularBuffer::process\n");
}


TrialCircularBufferThread::TrialCircularBufferThread(TrialCircularBuffer *tcb_,  std::vector<int> *conditions, Trial* trial_, int jobID_, int jobType_, int electrodeID_, int subID_) : ThreadPoolJob("Job "+String(jobID)),
	tcb(tcb_),jobID(jobID_), jobType(jobType_), electrodeID(electrodeID_), subID(subID_), trial(trial_), conditionsNeedUpdate(conditions)
{

}

juce::ThreadPoolJob::JobStatus TrialCircularBufferThread::runJob()
{
	if (jobType == 0)
	{
		tcb->updateLFPwithTrial(electrodeID,conditionsNeedUpdate,trial);
	} else if (jobType == 1)
	{
		tcb->updateSpikeswithTrial(electrodeID,subID,conditionsNeedUpdate,trial);
	}
	return jobHasFinished;
}



