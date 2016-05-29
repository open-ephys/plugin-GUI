#ifndef RHD2000PCIE_H
#define RHD2000PCIE_H

#include <vector>
#include <cstdint>
#include <queue>

#define MAX_NUM_DATA_STREAMS 16
#define DATA_BUFFER_SIZE 2560000

using namespace std;

namespace PCIeRhythm {
	class Rhd2000DataBlock;

	class rhd2000PCIe
	{
	public:
		rhd2000PCIe();
		~rhd2000PCIe();

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

		enum BoardDataSource {
			PortA1 = 0,
			PortA2 = 1,
			PortB1 = 2,
			PortB2 = 3,
			PortC1 = 4,
			PortC2 = 5,
			PortD1 = 6,
			PortD2 = 7,
			PortA1Ddr = 8,
			PortA2Ddr = 9,
			PortB1Ddr = 10,
			PortB2Ddr = 11,
			PortC1Ddr = 12,
			PortC2Ddr = 13,
			PortD1Ddr = 14,
			PortD2Ddr = 15
		};

		enum AuxCmdSlot {
			AuxCmd1,
			AuxCmd2,
			AuxCmd3
		};

		enum BoardPort {
			PortA,
			PortB,
			PortC,
			PortD
		};

		bool openBoard();
		void initialize();
		void resetBoard();

		bool setSampleRate(AmplifierSampleRate newSampleRate);
		double getSampleRate() const;
		AmplifierSampleRate getSampleRateEnum() const;

		void uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank);
		void selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank);
		void selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex);

		void setContinuousRunMode(bool continuousMode);
		void setMaxTimeStep(unsigned int maxTimeStep);

		void run();
		bool isRunning() const;

		void setCableDelay(BoardPort port, int delay);
		void setCableLengthMeters(BoardPort port, double lengthInMeters);
		void setCableLengthFeet(BoardPort port, double lengthInFeet);
		double estimateCableLengthMeters(int delay) const;
		double estimateCableLengthFeet(int delay) const;

		void setDspSettle(bool enabled);

		void setDataSource(int stream, BoardDataSource dataSource);
		void enableDataStream(int stream, bool enabled);
		int getNumEnabledDataStreams() const;

		int getCableDelay(BoardPort port) const;
		void getCableDelay(vector<int> &delays) const;
		bool isStreamEnabled(int streamIndex);

		bool readRawDataBlock(unsigned char** bufferPtr, int nSamples = -1);
		bool readDataBlock(Rhd2000DataBlock *dataBlock, int nSamples = -1);
		bool readDataBlocks(int numBlocks, queue<Rhd2000DataBlock> &dataQueue, int nSamples = -1);

		bool openPipe();
		void closePipe();

		void setOuputSigs(int sigs);

		//void flush();


	private:
		int fidControl, fidStatus, fidFIFO;

		enum controlAddr {
			ResetRun = 0x00,
			MaxTimeStepLsb = 0x02,
			MaxTimeStepMsb = 0x04,
			DataFreqPll = 0x06,
			MisoDelay = 0x08,
			AuxCmdBank1 = 0x10,
			AuxCmdBank2 = 0x12,
			AuxCmdBank3 = 0x14,
			AuxCmdLength1 = 0x16,
			AuxCmdLength2 = 0x18,
			AuxCmdLength3 = 0x1A,
			AuxCmdLoop1 = 0x1C,
			AuxCmdLoop2 = 0x1E,
			AuxCmdLoop3 = 0x20,
			DataStreamSel1234 = 0x24,
			DataStreamSel5678 = 0x26,
			DataStreamSel9ABC = 0x28,
			DataStreamSelDEF10 = 0x2A,
			DataStreamEn = 0x2C,
			AuxOutputs = 0x2E,
			StartTrigger = 0x3E
		};
		enum statusAddr {
			NumWordsLsb = 0x00,
			NumWordsMsb = 0x02,
			SpiRunning = 0x04,
			DataClkLocked = 0x08,
			BoardId = 0x0A,
			BoardVersion = 0x0C
		};

		AmplifierSampleRate sampleRate;
		int numDataStreams; // total number of data streams currently enabled
		int dataStreamEnabled[MAX_NUM_DATA_STREAMS]; // 0 (disabled) or 1 (enabled), set for maximum stream number
		vector<int> cableDelay;

		void writeRegister(controlAddr reg, uint16_t value, uint16_t mask = 0xFFFF);
		uint16_t readRegister(statusAddr reg) const;

		bool isDcmProgDone() const;
		bool isDataClockLocked() const;

		unsigned char dataBuffer[DATA_BUFFER_SIZE];

	};
};
#endif // !RHD2000PCIE_H