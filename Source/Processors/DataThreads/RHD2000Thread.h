/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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


#ifndef __RHD2000THREAD_H_2C4CBD67__
#define __RHD2000THREAD_H_2C4CBD67__


#include "../../../JuceLibraryCode/JuceHeader.h"

#include <stdio.h>
#include <string.h>

#include "rhythm-api/rhd2000evalboard.h"
#include "rhythm-api/rhd2000registers.h"
#include "rhythm-api/rhd2000datablock.h"
#include "rhythm-api/okFrontPanelDLL.h"

#include "DataThread.h"
#include "../GenericProcessor/GenericProcessor.h"

#define MAX_NUM_DATA_STREAMS 8
#define MAX_NUM_HEADSTAGES 8

class SourceNode;
class RHDHeadstage;
class RHDImpedanceMeasure;

struct ImpedanceData
{
	Array<int> streams;
	Array<int> channels;
	Array<float> magnitudes;
	Array<float> phases;
	bool valid;
};

/**

  Communicates with the RHD2000 Evaluation Board from Intan Technologies

  @see DataThread, SourceNode

*/

class RHD2000Thread : public DataThread, public Timer
{
	friend class RHDImpedanceMeasure;
public:
    RHD2000Thread(SourceNode* sn);
    ~RHD2000Thread();

    // for communication with SourceNode processors:
    bool foundInputSource();
    int getNumChannels();
    int getNumHeadstageOutputs();
    int getNumAuxOutputs();
    int getNumAdcOutputs();
    float getSampleRate();
    float getBitVolts(Channel* chan);
    float getAdcBitVolts(int channelNum);

    bool isHeadstageEnabled(int hsNum);
    int getChannelsInHeadstage(int hsNum);

    void setSampleRate(int index, bool temporary = false);

    double setUpperBandwidth(double upper); // set desired BW, returns actual BW
    double setLowerBandwidth(double lower);

    double setDspCutoffFreq(double freq);
    double getDspCutoffFreq();

    void setDSPOffset(bool state);

    int setNoiseSlicerLevel(int level);
    void setFastTTLSettle(bool state, int channel);
    void setTTLoutputMode(bool state);
    void setDAChpf(float cutoff, bool enabled);

    void scanPorts();
    int getNumEventChannels();

    void enableAdcs(bool);

    bool isAcquisitionActive();
	bool isReady();

    int modifyChannelGain(int channel, float gain);
    int modifyChannelName(int channel, String newName);
    void getEventChannelNames(StringArray& Names);
    Array<int> getDACchannels();
    void setDACchannel(int dacOutput, int channel);
    void setDACthreshold(int dacOutput, float threshold);
    void setDefaultNamingScheme(int scheme);

    String getChannelName(int ch);
    void setNumChannels(int hsNum, int nChannels);
    int getHeadstageChannels(int hsNum);
    int getActiveChannelsInHeadstage(int hsNum);
    bool usesCustomNames();

    /* Gets the absolute channel index from the headstage channel index*/
    int getChannelFromHeadstage(int hs, int ch);
    /*Gets the headstage relative channel index from the absolute channel index*/
    int getHeadstageChannel(int& hs, int ch);

	void runImpedanceTest(ImpedanceData* data);
	void enableBoardLeds(bool enable);

private:

    bool enableHeadstage(int hsNum, bool enabled, int nStr = 1, int strChans = 32);
    void updateBoardStreams();
    void setCableLength(int hsNum, float length);

    void setDefaultChannelNames();

    ScopedPointer<Rhd2000EvalBoard> evalBoard;
    Rhd2000Registers chipRegisters;
    Rhd2000DataBlock* dataBlock;

	int numChannels;
    bool deviceFound;

    float thisSample[256];
    float auxBuffer[256]; // aux inputs are only sampled every 4th sample, so use this to buffer the samples so they can be handles just like the regular neural channels later

    int blockSize;

    bool isTransmitting;

    bool dacOutputShouldChange;
    bool acquireAdcChannels;
    bool acquireAuxChannels;

    bool fastSettleEnabled;
    bool fastTTLSettleEnabled;
    bool fastSettleTTLChannel;
    bool ttlMode;
    bool desiredDAChpfState;
    double desiredDAChpf;

    bool dspEnabled;
    double actualDspCutoffFreq, desiredDspCutoffFreq;
    double actualUpperBandwidth, desiredUpperBandwidth;
    double actualLowerBandwidth, desiredLowerBandwidth;
    int actualNoiseSlicerLevel, desiredNoiseSlicerLevel;
    double boardSampleRate;
    int savedSampleRateIndex;

    String libraryFilePath;

    void timerCallback();

    bool startAcquisition();
    bool stopAcquisition();

    bool openBoard(String pathToLibrary);
    bool uploadBitfile(String pathToBitfile);
    void initializeBoard();

    void updateRegisters();

    int deviceId(Rhd2000DataBlock* dataBlock, int stream, int& register59Value);

    bool updateBuffer();

    double cableLengthPortA, cableLengthPortB, cableLengthPortC, cableLengthPortD;

    int audioOutputL, audioOutputR;
    int* dacChannels, *dacStream;
    float* dacThresholds;
    bool* dacChannelsToUpdate;
    Array<int> chipId;
    OwnedArray<RHDHeadstage> headstagesArray;
    Array<Rhd2000EvalBoard::BoardDataSource> enabledStreams;
    Array<int> numChannelsPerDataStream;

    // used for data stream names...
    int numberingScheme ;
    Array<float> adcBitVolts;
    bool newScan;
	ScopedPointer<RHDImpedanceMeasure> impedanceThread;
	bool ledsEnabled;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Thread);
};

class RHDHeadstage
{
public:
    RHDHeadstage(Rhd2000EvalBoard::BoardDataSource stream);
    ~RHDHeadstage();
    void setNumStreams(int num);
    void setChannelsPerStream(int nchan, int index);
	int getStreamIndex(int index);
    int getNumChannels();
    int getNumStreams();
    void setHalfChannels(bool half); //mainly used for de 16ch rhd2132 board
    int getNumActiveChannels();
    Rhd2000EvalBoard::BoardDataSource getDataStream(int index);
    bool isPlugged();
private:
    Rhd2000EvalBoard::BoardDataSource dataStream;
	int streamIndex;
    int numStreams;
    int channelsPerStream;
    bool halfChannels;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHDHeadstage);
};

class RHDImpedanceMeasure : public Thread, public ActionBroadcaster
{
public:
	RHDImpedanceMeasure(RHD2000Thread* b);
	~RHDImpedanceMeasure();
	void prepareData(ImpedanceData* d);
	void stopThreadSafely();
	void waitSafely();
	void run();
private:
	void runImpedanceMeasurement();
	void restoreFPGA();
	void measureComplexAmplitude(std::vector<std::vector<std::vector<double>>>& measuredMagnitude,
		std::vector<std::vector<std::vector<double>>>& measuredPhase,
		int capIndex, int stream, int chipChannel, int numBlocks,
		double sampleRate, double frequency, int numPeriods);
	void amplitudeOfFreqComponent(double& realComponent, double& imagComponent,
		const std::vector<double>& data, int startIndex,
		int endIndex, double sampleRate, double frequency);
	float updateImpedanceFrequency(float desiredImpedanceFreq, bool& impedanceFreqValid);
	void factorOutParallelCapacitance(double& impedanceMagnitude, double& impedancePhase,
		double frequency, double parasiticCapacitance);
	void empiricalResistanceCorrection(double& impedanceMagnitude, double& impedancePhase,
		double boardSampleRate);
	int loadAmplifierData(queue<Rhd2000DataBlock>& dataQueue,
		int numBlocks, int numDataStreams);

	std::vector<std::vector<std::vector<double>>> amplifierPreFilter;

	ImpedanceData* data;
	RHD2000Thread* board;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHDImpedanceMeasure);
};

#endif  // __RHD2000THREAD_H_2C4CBD67__
