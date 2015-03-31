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

#ifndef __SPIKESORTER_H_3F920F95__
#define __SPIKESORTER_H_3F920F95__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../GenericProcessor/GenericProcessor.h"
#include "SpikeSorterEditor.h"
#include "SpikeSortBoxes.h"
#include "../Visualization/SpikeObject.h"
#include "../SourceNode/SourceNode.h"
#include "../DataThreads/RHD2000Thread.h"
#include <algorithm>    // std::sort
#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

class SpikeSorterEditor;
class SpikeHistogramPlot;
class Trial;
/**

  Detects spikes in a continuous signal and outputs events containing the spike data.

  Allows the user to draw boundaries around clusters.

  @see GenericProcessor, SpikeSorterEditor

*/

/*
class Histogram {
public:
	Histogram(float _minValue, float _maxValue, float _resolution, bool _throwOutsideSamples);
	//Histogram(float _minValue, float _maxValue, int _numBins, bool _throwOutsideSamples);
	void addSamples(float *Samples, int numSamples);
	~Histogram();
	void clear();

	float minValue, maxValue, resolution;
	int numBins;
	bool throwOutsideSamples;
	unsigned long *binCounts;
	float *binCenters;
};
*/

class PCAjob;
class PCAcomputingThread;
class UniqueIDgenerator
{
public:
	UniqueIDgenerator() {globalUniqueID=0;}
	int generateUniqueID() {return ++globalUniqueID;};
	void setUniqueID(int ID) {globalUniqueID= ID;}
	int getLastUniqueID() {return globalUniqueID;}
private:
	int globalUniqueID;
};

/* snatched from http://www.johndcook.com/blog/standard_deviation/ */
class RunningStat
{
public:
	RunningStat() : m_n(0) {}

	void Clear()
	{
		m_n = 0;
	}

	void Push(double x)
	{
		m_n++;

		// See Knuth TAOCP vol 2, 3rd edition, page 232
		if (m_n == 1)
		{
			m_oldM = m_newM = x;
			m_oldS = 0.0;
		}
		else
		{
			m_newM = m_oldM + (x - m_oldM) / m_n;
			m_newS = m_oldS + (x - m_oldM)*(x - m_newM);

			// set up for next iteration
			m_oldM = m_newM;
			m_oldS = m_newS;
		}
	}

	int NumDataValues() const
	{
		return m_n;
	}

	double Mean() const
	{
		return (m_n > 0) ? m_newM : 0.0;
	}

	double Variance() const
	{
		return ((m_n > 1) ? m_newS / (m_n - 1) : 0.0);
	}

	double StandardDeviation() const
	{
		return sqrt(Variance());
	}

private:
	int m_n;
	double m_oldM, m_newM, m_oldS, m_newS;
};

class Electrode
{
	public:
		Electrode(int electrodeID, UniqueIDgenerator *uniqueIDgenerator_, PCAcomputingThread *pth,String _name, int _numChannels, int *_channels, float default_threshold, int pre, int post, float samplingRate , int sourceNodeId);
        ~Electrode();

		void resizeWaveform(int numPre, int numPost);

		String name;

        int numChannels;
        int prePeakSamples, postPeakSamples;
        int lastBufferIndex;

		int advancerID;
		float depthOffsetMM;

		int electrodeID;
		int sourceNodeId;
        int* channels;
	    double* thresholds;
        bool* isActive;
		double *voltageScale;
		//float PCArange[4];

		RunningStat *runningStats;
		SpikeHistogramPlot* spikePlot;
		SpikeSortBoxes* spikeSort;
		PCAcomputingThread *computingThread;
		UniqueIDgenerator *uniqueIDgenerator;
        bool isMonitored;
};


class ContinuousCircularBuffer
{
public:
	ContinuousCircularBuffer(int NumCh, float SamplingRate, int SubSampling, float NumSecInBuffer);
	void reallocate(int N);
	void update(std::vector<std::vector<bool>> contdata, int64 hardware_ts, int64 software_ts, int numpts);
	void update(AudioSampleBuffer& buffer, int64 hardware_ts, int64 software_ts, int numpts);
	void update(int channel, int64 hardware_ts, int64 software_ts, bool rise);
	int GetPtr();
	void addTrialStartToSmartBuffer(int trialID);
	int numCh;
	int subSampling;
	float samplingRate;
	CriticalSection mut;
	int numSamplesInBuf;
	double numTicksPerSecond;
	int ptr;
	int bufLen;
	int leftover_k;
	double buffer_dx;
	
	std::vector<std::vector<float> > Buf;
	std::vector<bool> valid;
	std::vector<int64> hardwareTS,softwareTS;
};


//class StringTS;



class SpikeSorter : public GenericProcessor
{
public:

    // CONSTRUCTOR AND DESTRUCTOR //

    /** constructor */
    SpikeSorter();

    /** destructor */
    ~SpikeSorter();


    // PROCESSOR METHODS //

    /** Processes an incoming continuous buffer and places new
        spikes into the event buffer. */
    void process(AudioSampleBuffer& buffer, MidiBuffer& events);

    /** Used to alter parameters of data acquisition. */
    void setParameter(int parameterIndex, float newValue);

    /** Called whenever the signal chain is altered. */
    void updateSettings();

    /** Called prior to start of acquisition. */
    bool enable();

    /** Called after acquisition is finished. */
    bool disable();

	
	bool isReady();
    /** Creates the SpikeSorterEditor. */
    AudioProcessorEditor* createEditor();

	float getSelectedElectrodeNoise();
	void clearRunningStatForSelectedElectrode();

	//void addNetworkEventToQueue(StringTS S);

	void postEventsInQueue(MidiBuffer& events);

    // INTERNAL BUFFERS //

    /** Extra samples are placed in this buffer to allow seamless
        transitions between callbacks. */
    AudioSampleBuffer overflowBuffer;


    // CREATE AND DELETE ELECTRODES //

    /** Adds an electrode with n channels to be processed. */
    bool addElectrode(int nChans, String name, double depth);

	void addProbes(String probeType,int numProbes, int nElectrodesPerProbe, int nChansPerElectrode,  double firstContactOffset, double interelectrodeDistance);

    /** Removes an electrode with a given index. */
    bool removeElectrode(int index);


    // EDIT AND QUERY ELECTRODE SETTINGS //

    /** Returns the number of channels for a given electrode. */
    int getNumChannels(int index);

    /** Edits the mapping between input channels and electrode channels. */
    void setChannel(int electrodeIndex, int channelNum, int newChannel);

    /** Returns the continuous channel that maps to a given
    	electrode channel. */
    int getChannel(int index, int chan);

    /** Sets the name of a given electrode. */
    void setElectrodeName(int index, String newName);

    /** */
    void setChannelActive(int electrodeIndex, int channelNum, bool active);

    /** */
    bool isChannelActive(int electrodeIndex, int channelNum);

	/** returns the current active electrode, i.e., the one displayed in the editor */
	Electrode* getActiveElectrode();
    
    /** Returns a StringArray containing the names of all electrodes */
    StringArray getElectrodeNames();

	/** modify a channel spike detection threshold */
    void setChannelThreshold(int electrodeNum, int channelNum, float threshold);

	/** returns a channel's detection threshold */
    double getChannelThreshold(int electrodeNum, int channelNum);

	/** used to generate messages over the network and to inform PSTH sink */
	void addNewUnit(int electrodeID, int newUnitID, uint8 r, uint8 g, uint8 b);
	void removeUnit(int electrodeID, int newUnitID);

	/** saves all electrodes, thresholds, units, etc to xml */
    void saveCustomParametersToXml(XmlElement* parentElement);
    void loadCustomParametersFromXml();

	/** returns the depth of an electrode. The depth is calculated as the
	known depth of the advancer that is used to control that electrode, plus
	the defined depth offset. Depth offset is mainly useful for depth probes,
	in which the contact position is not always the at the tip */
	//double getElectrodeDepth(int electrodeID);

	/** returns the number of electrodes */
	int getNumElectrodes();

	/** clears up the spike plots. Called during updates */
	void removeSpikePlots();

	int getNumberOfChannelsForElectrode(int i);
	String getNameForElectrode(int i);
	void addSpikePlotForElectrode(SpikeHistogramPlot* sp, int i);
	int getCurrentElectrodeIndex();
	Electrode* setCurrentElectrodeIndex(int i);
	Electrode* getElectrode(int i);
	//StringTS createStringTS(String S);
	//int64 getExtrapolatedHardwareTimestamp(int64 softwareTS);
	//void postTimestamppedStringToMidiBuffer(StringTS s, MidiBuffer& events);
	//void setElectrodeAdvancer(int i,int ID);
	//void setElectrodeAdvancerOffset(int i, double v);
	//double getAdvancerPosition(int advancerID);
	//double getSelectedElectrodeDepth();
	bool getAutoDacAssignmentStatus();
	void seteAutoDacAssignment(bool status);
	int getNumPreSamples();
	int getNumPostSamples();
	void setNumPreSamples(int numSamples);
	void setNumPostSamples(int numSamples);
	int getDACassignment(int channel);
	void assignDACtoChannel(int dacOutput, int channel);
	Array<int> getDACassignments();
	void updateDACthreshold(int dacChannel, float threshold);
	bool getThresholdSyncStatus();
	void setThresholdSyncStatus(bool status);
	bool getFlipSignalState();
	void setFlipSignalState(bool state);
	void startRecording();
	std::vector<float> getElectrodeVoltageScales(int electrodeID);
	//void getElectrodePCArange(int electrodeID, float &minX,float &maxX,float &minY,float &maxY);
	//void setElectrodePCArange(int electrodeID, float minX,float maxX,float minY,float maxY);
	
	void removeAllUnits(int electrodeID);

	void setElectrodeVoltageScale(int electrodeID, int index, float newvalue);
	bool isSelectedElectrodeRecorded(int channel);
	std::vector<int> getElectrodeChannels(int ID);

	Array<Electrode*> getElectrodes();

    std::vector<String> electrodeTypes;

	/** sync PSTH : inform of a new electrode added  */
	void updateSinks(Electrode* newElectrode);
	/** sync PSTH : inform of an electrode removal */
	void updateSinks(int electrodeID);
	/** sync PSTH : inform of a channel swap */
	void updateSinks(int electrodeID, int channelindex, int newchannel);
	/** sync PSTH: inform of a new unit added / removed */
	void updateSinks(int electrodeID, int unitID, uint8 r, uint8 g, uint8 b, bool addRemove);
	/** sync PSTH: inform of a name change*/
	void updateSinks(int electrodeID, String NewName);
	/** sync PSTH: remove all units*/
	void updateSinks(int electrodeID, bool b);



private:
	UniqueIDgenerator uniqueIDgenerator;
	long uniqueSpikeID;
	SpikeObject prevSpike;

	void addElectrode(Electrode* newElectrode);
	void increaseUniqueProbeID(String type);
	int getUniqueProbeID(String type);

	float ticksPerSec;
	int uniqueID;
	//std::queue<StringTS> eventQueue;
    /** pointer to a continuous buffer. */
    AudioSampleBuffer* dataBuffer;

    float getDefaultThreshold();

    int overflowBufferSize;

    int sampleIndex;

    std::vector<int> electrodeCounter;
    float getNextSample(int& chan);
    float getCurrentSample(int& chan);
    bool samplesAvailable(int nSamples);

    Array<bool> useOverflowBuffer;

    int currentElectrode;
    int currentChannelIndex;
    int currentIndex;


	int numPreSamples,numPostSamples;
    uint8_t* spikeBuffer;///[256];
    //int64 timestamp;
		  int64 hardware_timestamp;
		  int64 software_timestamp;

	bool PCAbeforeBoxes;
 	ContinuousCircularBuffer* channelBuffers; // used to compute auto threshold

     void handleEvent(int eventType, MidiMessage& event, int sampleNum);

    void addSpikeEvent(SpikeObject* s, MidiBuffer& eventBuffer, int peakIndex);
 
    void resetElectrode(Electrode*);
	CriticalSection mut;
	bool autoDACassignment;
	bool syncThresholds;
	RHD2000Thread* getRhythmAccess();
	bool flipSignal;

	Time timer;

   void addWaveformToSpikeObject(SpikeObject* s,
                                  int& peakIndex,
                                  int& electrodeNumber,
                                  int& currentChannel);


		   Array<Electrode*> electrodes;
		   PCAcomputingThread computingThread;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeSorter);

};




/*

class circularBuffer {
public:
	circularBuffer(int NumCh, int NumSamplesToHoldPerChannel, double SamplingRate);
	~circularBuffer();
	
	std::vector<double> getDataArray(int channel, int N);
	double findThresholdForChannel(int channel);
	void update(AudioSampleBuffer& buffer);

private:
     CriticalSection mut;
 
	int numCh;
	int numSamplesInBuf;
	int ptr;
	double samplingRate;
	int bufLen;
	std::vector<std::vector<double>> Buf;
	std::vector<double> BufTS_H;
	std::vector<double> BufTS_S;
};


*/











#endif  // __SPIKESORTER_H_3F920F95__
