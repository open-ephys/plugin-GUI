//----------------------------------------------------------------------------------
// rhd2000evalboardusb3.h
//
// Intan Technoloies RHD2000 USB3 Rhythm Interface API
// Rhd2000EvalBoardUsb3 Class Header File
// Version 2.04 (28 March 2017)
//
// Copyright (c) 2013-2017 Intan Technologies LLC
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any applications that
// use Intan Technologies integrated circuits, and to alter it and redistribute it
// freely.
//
// See http://www.intantech.com for documentation and product information.
//----------------------------------------------------------------------------------

#ifndef RHD2000EVALBOARDUSB3_H
#define RHD2000EVALBOARDUSB3_H

#define RHYTHM_BOARD_ID 700
#define MAX_NUM_DATA_STREAMS 32
#define MAX_NUM_SPI_PORTS 8
#define RHD_BOARD_MODE 13

// The maximum number of Rhd2000DataBlockUsb3 objects we will need is set by the need
// to perform electrode impedance measurements at very low frequencies.
// (Maximum command length = 1024 for one period; seven periods required in worst case.)
#define MAX_NUM_BLOCKS 57

#define FIFO_CAPACITY_WORDS 67108864

#define USB3_BLOCK_SIZE	1024
#define RAM_BURST_SIZE 32

#include <queue>
#include <mutex>

using namespace std;

namespace OpalKellyLegacy
{
	class okCFrontPanel;
}
class Rhd2000DataBlockUsb3;

class Rhd2000EvalBoardUsb3
{

public:
    Rhd2000EvalBoardUsb3();
    ~Rhd2000EvalBoardUsb3();

    int open();
    bool uploadFpgaBitfile(string filename);
    void initialize();

    enum AmplifierSampleRate {
        SampleRate1000Hz,
        SampleRate1250Hz,
        SampleRate1500Hz,
        SampleRate2000Hz,
        SampleRate2500Hz,
        SampleRate3000Hz,
        SampleRate3333Hz,
        SampleRate4000Hz,
        SampleRate5000Hz,
        SampleRate6250Hz,
        SampleRate8000Hz,
        SampleRate10000Hz,
        SampleRate12500Hz,
        SampleRate15000Hz,
        SampleRate20000Hz,
        SampleRate25000Hz,
        SampleRate30000Hz
    };

    bool setSampleRate(AmplifierSampleRate newSampleRate);
    double getSampleRate() const;
    AmplifierSampleRate getSampleRateEnum() const;

    enum AuxCmdSlot {
        AuxCmd1,
        AuxCmd2,
        AuxCmd3
    };

    enum BoardPort {
        PortA,
        PortB,
        PortC,
        PortD,
        PortE,
        PortF,
        PortG,
        PortH
    };

    void uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank);
    void printCommandList(const vector<int> &commandList) const;
    void selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank);
    void selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex);

    void resetBoard();
    void resetFpga();
    void setContinuousRunMode(bool continuousMode);
    void setMaxTimeStep(unsigned int maxTimeStep);
    void run();
    bool isRunning();
    unsigned int getNumWordsInFifo();
    unsigned int getLastNumWordsInFifo();
    unsigned int getLastNumWordsInFifo(bool& hasBeenUpdated);
    static unsigned int fifoCapacityInWords();

    void setCableDelay(BoardPort port, int delay);
    void setCableLengthMeters(BoardPort port, double lengthInMeters);
    void setCableLengthFeet(BoardPort port, double lengthInFeet);
    double estimateCableLengthMeters(int delay) const;
    double estimateCableLengthFeet(int delay) const;

    void setDspSettle(bool enabled);
    void setAllDacsToZero();

    void enableDataStream(int stream, bool enabled);
    int getNumEnabledDataStreams() const;

    void clearTtlOut();
    void setTtlOut(int ttlOutArray[]);
    void getTtlIn(int ttlInArray[]);

    void setDacManual(int value);

    void setLedDisplay(int ledArray[]);
    void setSpiLedDisplay(int ledArray[]);

    void enableDac(int dacChannel, bool enabled);
    void setDacGain(int gain);
    void setAudioNoiseSuppress(int noiseSuppress);
    void selectDacDataStream(int dacChannel, int stream);
    void selectDacDataChannel(int dacChannel, int dataChannel);
    void enableExternalFastSettle(bool enable);
    void setExternalFastSettleChannel(int channel);
    void enableExternalDigOut(BoardPort port, bool enable);
    void setExternalDigOutChannel(BoardPort port, int channel);
    void enableDacHighpassFilter(bool enable);
    void setDacHighpassFilter(double cutoff);
    void setDacThreshold(int dacChannel, int threshold, bool trigPolarity);
    void setTtlMode(int mode);

    void flush();
    bool readDataBlock(Rhd2000DataBlockUsb3 *dataBlock, int nSamples = -1);
	long readDataBlocksRaw(int numBlocks, unsigned char* buffer, int nSamples = -1);
    bool readDataBlocks(int numBlocks, queue<Rhd2000DataBlockUsb3> &dataQueue);
    int queueToFile(queue<Rhd2000DataBlockUsb3> &dataQueue, std::ofstream &saveOut);
    int getBoardMode();
    int getCableDelay(BoardPort port) const;
    void getCableDelay(vector<int> &delays) const;

    int readDigitalInManual(bool& expanderBoardDetected);
    void readDigitalInExpManual();

    void setDacRerefSource(int stream, int channel);
    void enableDacReref(bool enabled);

	bool getStreamEnabled(int stream) const;

private:
    OpalKellyLegacy::okCFrontPanel *dev;
    AmplifierSampleRate sampleRate;
    unsigned int usbBufferSize;
    int numDataStreams; // total number of data streams currently enabled
    int dataStreamEnabled[MAX_NUM_DATA_STREAMS]; // 0 (disabled) or 1 (enabled)
    vector<int> cableDelay;

    // Methods in this class are designed to be thread-safe.  This variable is used to ensure that.
    std::mutex okMutex;

    // Buffer for reading bytes from USB interface
    unsigned char* usbBuffer;

    // Opal Kelly module USB interface endpoint addresses
    enum OkEndPoint {
        WireInResetRun = 0x00,
        WireInMaxTimeStep = 0x01,
        WireInSerialDigitalInCntl = 0x02,
        WireInDataFreqPll = 0x03,
        WireInMisoDelay = 0x04,
        WireInCmdRamAddr = 0x05,
        WireInCmdRamBank = 0x06,
        WireInCmdRamData = 0x07,
        WireInAuxCmdBank1 = 0x08,
        WireInAuxCmdBank2 = 0x09,
        WireInAuxCmdBank3 = 0x0a,
        WireInAuxCmdLength = 0x0b,
        WireInAuxCmdLoop = 0x0c,
        WireInLedDisplay = 0x0d,
        WireInDacReref = 0x0e,
        // Note: room for extra WireIns here
        WireInDataStreamEn = 0x14,
        WireInTtlOut = 0x15,
        WireInDacSource1 = 0x16,
        WireInDacSource2 = 0x17,
        WireInDacSource3 = 0x18,
        WireInDacSource4 = 0x19,
        WireInDacSource5 = 0x1a,
        WireInDacSource6 = 0x1b,
        WireInDacSource7 = 0x1c,
        WireInDacSource8 = 0x1d,
        WireInDacManual = 0x1e,
        WireInMultiUse = 0x1f,

        TrigInConfig = 0x40,
        TrigInSpiStart = 0x41,
        TrigInDacConfig = 0x42,

        WireOutNumWords = 0x20,
        WireOutSerialDigitalIn = 0x21,
        WireOutSpiRunning = 0x22,
        WireOutTtlIn = 0x23,
        WireOutDataClkLocked = 0x24,
        WireOutBoardMode = 0x25,
        // Note: room for extra WireOuts here
        WireOutBoardId = 0x3e,
        WireOutBoardVersion = 0x3f,

        PipeOutData = 0xa0
    };

    string opalKellyModelName(int model) const;

    bool isDcmProgDone() const;
    bool isDataClockLocked() const;

    unsigned int lastNumWordsInFifo;
    bool numWordsHasBeenUpdated;
    unsigned int numWordsInFifo();
};

#endif // RHD2000EVALBOARDUSB3_H
