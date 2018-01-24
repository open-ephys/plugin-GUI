/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include <DataThreadHeaders.h>

#include <stdio.h>
#include <string.h>

#include "rhythm-api/rhd2000evalboardusb3.h"
#include "rhythm-api/rhd2000registersusb3.h"
#include "rhythm-api/rhd2000datablockusb3.h"
#include "rhythm-api/okFrontPanelDLL.h"

#define MAX_NUM_HEADSTAGES ( MAX_NUM_DATA_STREAMS / 2 )

#define MAX_NUM_CHANNELS MAX_NUM_DATA_STREAMS*35

class SourceNode;
class RHDHeadstage;
class RHDImpedanceMeasure;
class USBThread;

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
class RHD2000Thread : public DataThread
                    , public Timer
{
    friend class RHDImpedanceMeasure;

public:
    RHD2000Thread (SourceNode* sn);
    ~RHD2000Thread();

    int getNumChannels() const;

    // for communication with SourceNode processors:
    bool foundInputSource() override;

	int getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessor) const override;

	unsigned int getNumSubProcessors() const override;

	int getNumTTLOutputs(int subprocessor) const override;
    
    bool usesCustomNames() const override;

    float getSampleRate(int subprocessor) const override;
    float getBitVolts (const DataChannel* chan) const override;

    float getAdcBitVolts (int channelNum) const;

    bool isHeadstageEnabled (int hsNum) const;
    int getChannelsInHeadstage (int hsNum) const;

    void setSampleRate (int index, bool temporary = false);

    double setUpperBandwidth (double upper); // set desired BW, returns actual BW
    double setLowerBandwidth (double lower);

    double setDspCutoffFreq (double freq);
    double getDspCutoffFreq() const;

    void setDSPOffset (bool state);

    int setNoiseSlicerLevel (int level);
    void setFastTTLSettle (bool state, int channel);
    void setTTLoutputMode (bool state);
    void setDAChpf (float cutoff, bool enabled);

    void scanPorts();
    void enableAdcs (bool);

    bool isReady() override;

    bool isAcquisitionActive() const;

    int modifyChannelGain (int channel, float gain)      override;
    int modifyChannelName (int channel, String newName)  override;

    void getEventChannelNames (StringArray& Names) const override;
    Array<int> getDACchannels() const;

    void setDACchannel   (int dacOutput, int channel);
    void setDACthreshold (int dacOutput, float threshold);
    void setDefaultNamingScheme (int scheme);

    String getChannelName (int ch) const;
    void setNumChannels (int hsNum, int nChannels);

	String getChannelUnits(int chanIndex) const override;

    int getHeadstageChannels         (int hsNum) const;
    int getActiveChannelsInHeadstage (int hsNum) const;

    /* Gets the absolute channel index from the headstage channel index*/
    int getChannelFromHeadstage (int hs, int ch) const;
    /*Gets the headstage relative channel index from the absolute channel index*/
    int getHeadstageChannel (int& hs, int ch) const;

    void runImpedanceTest (ImpedanceData* data);
    GenericEditor* createEditor (SourceNode* sn);

    static DataThread* createDataThread (SourceNode* sn);


private:
    bool enableHeadstage (int hsNum, bool enabled, int nStr = 1, int strChans = 32);
    void updateBoardStreams();

    void setDefaultChannelNames() override;

    bool updateBuffer() override;

    void timerCallback() override;

    bool startAcquisition() override;
    bool stopAcquisition()  override;

    ScopedPointer<Rhd2000EvalBoardUsb3> evalBoard;
	Rhd2000RegistersUsb3 chipRegisters;
	ScopedPointer<Rhd2000DataBlockUsb3> dataBlock;

    int numChannels;
    bool deviceFound;

    float thisSample[MAX_NUM_CHANNELS];
    // aux inputs are only sampled every 4th sample, so use this to buffer the samples so they can be handles just like the regular neural channels later
    float auxBuffer[MAX_NUM_CHANNELS];
    float auxSamples[MAX_NUM_DATA_STREAMS][3];

    unsigned int blockSize;

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

    bool openBoard      (String pathToLibrary);
    bool uploadBitfile  (String pathToBitfile);
    void initializeBoard();

    void updateRegisters();

	int deviceId(Rhd2000DataBlockUsb3* dataBlock, int stream, int& register59Value);

    double cableLengthPortA, cableLengthPortB, cableLengthPortC, cableLengthPortD;
	double cableLengthPortE, cableLengthPortF, cableLengthPortG, cableLengthPortH;

    int audioOutputL, audioOutputR;
    int* dacChannels, *dacStream;
    float* dacThresholds;
    bool* dacChannelsToUpdate;
    Array<int> chipId;
    OwnedArray<RHDHeadstage> headstagesArray;
	Array<int> enabledStreams;
    Array<int> numChannelsPerDataStream;

    // used for data stream names...
    int numberingScheme ;
    Array<float> adcBitVolts;
    bool newScan;
    ScopedPointer<RHDImpedanceMeasure> impedanceThread;
	ScopedPointer<USBThread> usbThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RHD2000Thread);
};


class RHDHeadstage
{
public:
	RHDHeadstage(int stream);
    ~RHDHeadstage();

	int getStreamIndex (int index)  const;
    int getNumChannels()            const;
    int getNumStreams()             const;
    int getNumActiveChannels()      const;

    bool isPlugged() const;

	int getDataStream(int index) const;

    void setNumStreams (int num);
    void setChannelsPerStream (int nchan, int index);

    void setHalfChannels (bool half); //mainly used for de 16ch rhd2132 board


private:
	int dataStream;

    int streamIndex;
    int numStreams;
    int channelsPerStream;
    bool halfChannels;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RHDHeadstage);
};


class RHDImpedanceMeasure : public Thread
{
public:
    RHDImpedanceMeasure (RHD2000Thread* b);
    ~RHDImpedanceMeasure();

    void run() override;

    void prepareData (ImpedanceData* d);
    void stopThreadSafely();
    void waitSafely();


private:
    void runImpedanceMeasurement();
    void restoreFPGA();

    void measureComplexAmplitude (std::vector<std::vector<std::vector<double>>>& measuredMagnitude,
                                  std::vector<std::vector<std::vector<double>>>& measuredPhase,
                                  int capIndex, int stream, int chipChannel, int numBlocks,
                                  double sampleRate, double frequency, int numPeriods);

    void amplitudeOfFreqComponent (double& realComponent, double& imagComponent,
                                   const std::vector<double>& data, int startIndex,
                                   int endIndex, double sampleRate, double frequency);

    void factorOutParallelCapacitance(double& impedanceMagnitude, double& impedancePhase,
                                      double frequency, double parasiticCapacitance);

    void empiricalResistanceCorrection(double& impedanceMagnitude, double& impedancePhase,
                                       double boardSampleRate);

    float updateImpedanceFrequency (float desiredImpedanceFreq, bool& impedanceFreqValid);
    int loadAmplifierData (queue<Rhd2000DataBlockUsb3>& dataQueue,
                           int numBlocks, int numDataStreams);

    std::vector<std::vector<std::vector<double>>> amplifierPreFilter;

    ImpedanceData* data;
    RHD2000Thread* board;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RHDImpedanceMeasure);
};

#endif  // __RHD2000THREAD_H_2C4CBD67__
