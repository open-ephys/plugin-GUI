#include "rhd2000PCIe.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <cmath>

#ifdef _WIN32
#include <io.h>
#define Open _open
#define Seek _lseek
#define Read _read
#define Write _write
#define Close _close
#define CONTROL_FILE "\\\\.\\xillybus_control_regs_16"
#define STATUS_FILE "\\\\.\\xillybus_status_regs_16"
#define FIFO_FILE "\\\\.\\xillybus_neural_data_32"
#define AUXCMD1_FILE "\\\\.\\xillybus_auxcmd1_membank_16"
#define AUXCMD2_FILE "\\\\.\\xillybus_auxcmd2_membank_16"
#define AUXCMD3_FILE "\\\\.\\xillybus_auxcmd3_membank_16"
#else
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#define Open open
#define Seek lseek
#define Read read
#define Write write
#define Close close
#define CONTROL_FILE "/dev/xillybus_control_regs_16"
#define STATUS_FILE "/dev/xillybus_status_regs_16"
#define FIFO_FILE "/dev/xillybus_neural_data_32"
#define AUXCMD1_FILE "/dev/xillybus_auxcmd1_membank_16"
#define AUXCMD2_FILE "/dev/xillybus_auxcmd2_membank_16"
#define AUXCMD3_FILE "/dev/xillybus_auxcmd3_membank_16"
#ifndef O_BINARY
#define O_BINARY 0
#endif
#endif
#include <fcntl.h>

#include "rhd2000datablock.h"
using namespace PCIeRhythm;

rhd2000PCIe::rhd2000PCIe()
{
	int i;
	sampleRate = SampleRate30000Hz; // Rhythm FPGA boots up with 30.0 kS/s/channel sampling rate
	numDataStreams = 0;
	fidControl = -1;
	fidStatus = -1;
	fidFIFO = -1;

	for (i = 0; i < MAX_NUM_DATA_STREAMS; ++i) {
		dataStreamEnabled[i] = 0;
	}

	cableDelay.resize(4, -1);
}


rhd2000PCIe::~rhd2000PCIe()
{
	if (fidControl >= 0)
		Close(fidControl);
	if (fidStatus >= 0)
		Close(fidStatus);
	if (fidFIFO >= 0)
		Close(fidFIFO);
}

void rhd2000PCIe::writeRegister(controlAddr reg, uint16_t value, uint16_t mask)
{
	int regAddr = static_cast<int>(reg);

	if (Seek(fidControl, regAddr, SEEK_SET) < 0)
	{
		std::cerr << "Error seeking control to addr " << regAddr << std::endl;
		return;
	}

	uint16_t writeVal;
	uint16_t curVal = 0x1010; //an easily recognizable value, to distinguish it from an actual read value of zero in debug strings
	if ((mask & 0xFFFF) != 0xFFFF)
	{

		int rd = Read(fidControl, &curVal, 2);
		if (rd < 2)
		{
			std::cerr << "Unsuccesful read to control addr " << regAddr << " code: " << rd << std::endl;
			return;
		}
		if (Seek(fidControl, regAddr, SEEK_SET) < 0)
		{
			std::cerr << "Error re-seeking control to addr " << regAddr << std::endl;
			return;
		}
		writeVal = (curVal & ~mask) | (value & mask);
	}
	else
		writeVal = value;

	int wd = Write(fidControl, &writeVal, 2);
	if (wd < 2)
	{
		std::cerr << "Unsuccesful write to control addr " << regAddr << " code: " << wd << std::endl;
		return;
	}
//	printf("Written registry %X val: %X mask: %X\n original: %X written: %X\n", reg, value,
//		mask, curVal, writeVal); //Debug line
}

uint16_t rhd2000PCIe::readRegister(statusAddr reg) const
{
	int16_t value = -1;
	int regAddr = static_cast<int>(reg);
	if (Seek(fidStatus, regAddr, SEEK_SET) < 0)
	{
		std::cerr << "Error seeking status to addr " << regAddr << std::endl;
		return value;
	}
	int rd = Read(fidStatus, &value, 2);
	if (rd < 2)
	{
		std::cerr << "Unsuccesful read to status addr " << regAddr << " code: " << rd << std::endl;
		return value;
	}
	return value;
}

bool rhd2000PCIe::openBoard()
{
	fidControl = Open(CONTROL_FILE, O_RDWR | O_BINARY);
	if (fidControl < 0)
	{
		std::cerr << "Error opening control file " << std::endl;
		return false;
	}

	fidStatus = Open(STATUS_FILE, O_RDONLY | O_BINARY);
	if (fidStatus < 0)
	{
		std::cerr << "Error opening status file" << std::endl;
		Close(fidControl);
		fidControl = -1;
		return false;
	}

	std::cout << "Device files opened" << std::endl;
	return true;
}

bool rhd2000PCIe::openPipe()
{
	fidFIFO = Open(FIFO_FILE, O_RDONLY | O_BINARY);
	if (fidFIFO < 0)
	{
	std:cerr << "Error opening data FIFO" << std::endl;
		return false;
	}
	std::cout << "Pipe opened" << std::endl;
	return true;
}

void rhd2000PCIe::closePipe()
{
	if (fidFIFO >= 0)
	{
		Close(fidFIFO);
		fidFIFO = -1;
		std::cout << "Pipe closed" << std::endl;
	}
	else
		std::cerr << "ERROR: pipe already closed" << std::endl;
}

// Initialize Rhythm FPGA to default starting values.
void rhd2000PCIe::initialize()
{
	int i;

	//resetBoard();
	setSampleRate(SampleRate30000Hz);
	selectAuxCommandBank(PortA, AuxCmd1, 0);
	selectAuxCommandBank(PortB, AuxCmd1, 0);
	selectAuxCommandBank(PortC, AuxCmd1, 0);
	selectAuxCommandBank(PortD, AuxCmd1, 0);
	selectAuxCommandBank(PortA, AuxCmd2, 0);
	selectAuxCommandBank(PortB, AuxCmd2, 0);
	selectAuxCommandBank(PortC, AuxCmd2, 0);
	selectAuxCommandBank(PortD, AuxCmd2, 0);
	selectAuxCommandBank(PortA, AuxCmd3, 0);
	selectAuxCommandBank(PortB, AuxCmd3, 0);
	selectAuxCommandBank(PortC, AuxCmd3, 0);
	selectAuxCommandBank(PortD, AuxCmd3, 0);
	selectAuxCommandLength(AuxCmd1, 0, 0);
	selectAuxCommandLength(AuxCmd2, 0, 0);
	selectAuxCommandLength(AuxCmd3, 0, 0);
	setContinuousRunMode(true);
	setMaxTimeStep(4294967295);  // 4294967295 == (2^32 - 1)

	setCableLengthFeet(PortA, 3.0);  // assume 3 ft cables
	setCableLengthFeet(PortB, 3.0);
	setCableLengthFeet(PortC, 3.0);
	setCableLengthFeet(PortD, 3.0);

	setDspSettle(false);

	setDataSource(0, PortA1);
	setDataSource(1, PortB1);
	setDataSource(2, PortC1);
	setDataSource(3, PortD1);
	setDataSource(4, PortA2);
	setDataSource(5, PortB2);
	setDataSource(6, PortC2);
	setDataSource(7, PortD2);
	setDataSource(8, PortA1);
	setDataSource(9, PortB1);
	setDataSource(10, PortC1);
	setDataSource(11, PortD1);
	setDataSource(12, PortA2);
	setDataSource(13, PortB2);
	setDataSource(14, PortC2);
	setDataSource(15, PortD2);

	enableDataStream(0, true);        // start with only one data stream enabled
	for (i = 1; i < MAX_NUM_DATA_STREAMS; i++) {
		enableDataStream(i, false);
	}
}

void rhd2000PCIe::resetBoard()
{
	writeRegister(ResetRun, 0x1, 0x1);
	writeRegister(ResetRun, 0x0, 0x1);
}

// Set the per-channel sampling rate of the RHD2000 chips connected to the FPGA.
bool rhd2000PCIe::setSampleRate(AmplifierSampleRate newSampleRate)
{
	// Assuming a 100 MHz reference clock is provided to the FPGA, the programmable FPGA clock frequency
	// is given by:
	//
	//       FPGA internal clock frequency = 100 MHz * (M/D) / 2
	//
	// M and D are "multiply" and "divide" integers used in the FPGA's digital clock manager (DCM) phase-
	// locked loop (PLL) frequency synthesizer, and are subject to the following restrictions:
	//
	//                M must have a value in the range of 2 - 256
	//                D must have a value in the range of 1 - 256
	//                M/D must fall in the range of 0.05 - 3.33
	//
	// (See pages 85-86 of Xilinx document UG382 "Spartan-6 FPGA Clocking Resources" for more details.)
	//
	// This variable-frequency clock drives the state machine that controls all SPI communication
	// with the RHD2000 chips.  A complete SPI cycle (consisting of one CS pulse and 16 SCLK pulses)
	// takes 80 clock cycles.  The SCLK period is 4 clock cycles; the CS pulse is high for 14 clock
	// cycles between commands.
	//
	// Rhythm samples all 32 channels and then executes 3 "auxiliary" commands that can be used to read
	// and write from other registers on the chip, or to sample from the temperature sensor or auxiliary ADC
	// inputs, for example.  Therefore, a complete cycle that samples from each amplifier channel takes
	// 80 * (32 + 3) = 80 * 35 = 2800 clock cycles.
	//
	// So the per-channel sampling rate of each amplifier is 2800 times slower than the clock frequency.
	//
	// Based on these design choices, we can use the following values of M and D to generate the following
	// useful amplifier sampling rates for electrophsyiological applications:
	//
	//   M    D     clkout frequency    per-channel sample rate     per-channel sample period
	//  ---  ---    ----------------    -----------------------     -------------------------
	//    7  125          2.80 MHz               1.00 kS/s                 1000.0 usec = 1.0 msec
	//    7  100          3.50 MHz               1.25 kS/s                  800.0 usec
	//   21  250          4.20 MHz               1.50 kS/s                  666.7 usec
	//   14  125          5.60 MHz               2.00 kS/s                  500.0 usec
	//   35  250          7.00 MHz               2.50 kS/s                  400.0 usec
	//   21  125          8.40 MHz               3.00 kS/s                  333.3 usec
	//   14   75          9.33 MHz               3.33 kS/s                  300.0 usec
	//   28  125         11.20 MHz               4.00 kS/s                  250.0 usec
	//    7   25         14.00 MHz               5.00 kS/s                  200.0 usec
	//    7   20         17.50 MHz               6.25 kS/s                  160.0 usec
	//  112  250         22.40 MHz               8.00 kS/s                  125.0 usec
	//   14   25         28.00 MHz              10.00 kS/s                  100.0 usec
	//    7   10         35.00 MHz              12.50 kS/s                   80.0 usec
	//   21   25         42.00 MHz              15.00 kS/s                   66.7 usec
	//   28   25         56.00 MHz              20.00 kS/s                   50.0 usec
	//   35   25         70.00 MHz              25.00 kS/s                   40.0 usec
	//   42   25         84.00 MHz              30.00 kS/s                   33.3 usec
	//
	// To set a new clock frequency, assert new values for M and D (e.g., using okWireIn modules) and
	// pulse DCM_prog_trigger high (e.g., using an okTriggerIn module).  If this module is reset, it
	// reverts to a per-channel sampling rate of 30.0 kS/s.

	unsigned long M, O, D;
	D = 0;
	switch (newSampleRate) {
	case SampleRate1000Hz:
		M = 7;
		O = 125;
		break;
	case SampleRate1250Hz:
		M = 7;
		O = 100;
		break;
	case SampleRate1500Hz:
		M = 21;
		O = 125;
		D = 0x8000;
		break;
	case SampleRate2000Hz:
		M = 14;
		O = 125;
		break;
	case SampleRate2500Hz:
		M = 35;
		O = 125;
		D = 0x8000;
		break;
	case SampleRate3000Hz:
		M = 21;
		O = 125;
		break;
	case SampleRate3333Hz:
		M = 14;
		O = 75;
		break;
	case SampleRate4000Hz:
		M = 28;
		O = 125;
		break;
	case SampleRate5000Hz:
		M = 7;
		O = 25;
		break;
	case SampleRate6250Hz:
		M = 7;
		O = 20;
		break;
	case SampleRate8000Hz:
		M = 56;
		O = 125;
		break;
	case SampleRate10000Hz:
		M = 14;
		O = 25;
		break;
	case SampleRate12500Hz:
		M = 7;
		O = 10;
		break;
	case SampleRate15000Hz:
		M = 21;
		O = 25;
		break;
	case SampleRate20000Hz:
		M = 28;
		O = 25;
		break;
	case SampleRate25000Hz:
		M = 35;
		O = 25;
		break;
	case SampleRate30000Hz:
		M = 42;
		O = 25;
		break;
	default:
		return(false);
	}

	sampleRate = newSampleRate;

	// Wait for DcmProgDone = 1 before reprogramming clock synthesizer
	while (isDcmProgDone() == false) {}

	// Reprogram clock synthesizer
	writeRegister(DataFreqPll, D + ((M << 8) & 0x7F00) + (O & 0x00FF));

	// Wait for DataClkLocked = 1 before allowing data acquisition to continue
	while (isDataClockLocked() == false) {}

	return(true);
}

double rhd2000PCIe::getSampleRate() const
{
	switch (sampleRate) {
	case SampleRate1000Hz:
		return 1000.0;
		break;
	case SampleRate1250Hz:
		return 1250.0;
		break;
	case SampleRate1500Hz:
		return 1500.0;
		break;
	case SampleRate2000Hz:
		return 2000.0;
		break;
	case SampleRate2500Hz:
		return 2500.0;
		break;
	case SampleRate3000Hz:
		return 3000.0;
		break;
	case SampleRate3333Hz:
		return (10000.0 / 3.0);
		break;
	case SampleRate4000Hz:
		return 4000.0;
		break;
	case SampleRate5000Hz:
		return 5000.0;
		break;
	case SampleRate6250Hz:
		return 6250.0;
		break;
	case SampleRate8000Hz:
		return 8000.0;
		break;
	case SampleRate10000Hz:
		return 10000.0;
		break;
	case SampleRate12500Hz:
		return 12500.0;
		break;
	case SampleRate15000Hz:
		return 15000.0;
		break;
	case SampleRate20000Hz:
		return 20000.0;
		break;
	case SampleRate25000Hz:
		return 25000.0;
		break;
	case SampleRate30000Hz:
		return 30000.0;
		break;
	default:
		return -1.0;
	}
}

rhd2000PCIe::AmplifierSampleRate rhd2000PCIe::getSampleRateEnum() const
{
	return sampleRate;
}

// Upload an auxiliary command list to a particular command slot (AuxCmd1, AuxCmd2, or AuxCmd3) and RAM bank (0-15)
// on the FPGA.
void rhd2000PCIe::uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank)
{
	unsigned int i;
	const char* devFile;
	int bankPos;

	if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
		cerr << "Error in Rhd2000EvalBoard::uploadCommandList: auxCommandSlot out of range." << endl;
		return;
	}

	if (bank < 0 || bank > 15) {
		cerr << "Error in Rhd2000EvalBoard::uploadCommandList: bank out of range." << endl;
		return;
	}

	switch (auxCommandSlot)
	{
	case AuxCmd1:
		devFile = AUXCMD1_FILE;
		break;
	case AuxCmd2:
		devFile = AUXCMD2_FILE;
		break;
	case AuxCmd3:
		devFile = AUXCMD3_FILE;
		break;
	default:
		devFile = "";
	}
	bankPos = 2048 * bank;
	int fidMem = Open(devFile, O_WRONLY | O_BINARY);
	if (fidMem < 0)
	{
		std::cerr << "Error opening auxcmd device " << auxCommandSlot << std::endl;
		return;
	}
	if (Seek(fidMem, bankPos, SEEK_SET) < 0)
	{
		std::cerr << "Error seeking auxcmd " << auxCommandSlot << " to addr " << bankPos << std::endl;
		Close(fidMem);
		return;
	}

	for (i = 0; i < commandList.size(); ++i) {
		int16_t value = commandList[i];
		int wd = Write(fidMem, &value, 2);
		if (wd < 2)
		{
			std::cerr << "Error writing auxcmd " << auxCommandSlot << " index " << i << std::endl;
		}
	}
	Close(fidMem);
}

// Select an auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3) and bank (0-15) for a particular SPI port
// (PortA, PortB, PortC, or PortD) on the FPGA.
void rhd2000PCIe::selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank)
{
	int bitShift;

	if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
		cerr << "Error in Rhd2000EvalBoard::selectAuxCommandBank: auxCommandSlot out of range." << endl;
		return;
	}
	if (bank < 0 || bank > 15) {
		cerr << "Error in Rhd2000EvalBoard::selectAuxCommandBank: bank out of range." << endl;
		return;
	}

	switch (port) {
	case PortA:
		bitShift = 0;
		break;
	case PortB:
		bitShift = 4;
		break;
	case PortC:
		bitShift = 8;
		break;
	case PortD:
		bitShift = 12;
		break;
	}

	switch (auxCommandSlot) {
	case AuxCmd1:
		writeRegister(AuxCmdBank1, bank << bitShift, 0x000f << bitShift);
		break;
	case AuxCmd2:
		writeRegister(AuxCmdBank2, bank << bitShift, 0x000f << bitShift);
		break;
	case AuxCmd3:
		writeRegister(AuxCmdBank3, bank << bitShift, 0x000f << bitShift);
		break;
	}
}

// Specify a command sequence length (endIndex = 0-1023) and command loop index (0-1023) for a particular
// auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3).
void rhd2000PCIe::selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex)
{
	if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
		cerr << "Error in Rhd2000EvalBoard::selectAuxCommandLength: auxCommandSlot out of range." << endl;
		return;
	}

	if (loopIndex < 0 || loopIndex > 1023) {
		cerr << "Error in Rhd2000EvalBoard::selectAuxCommandLength: loopIndex out of range." << endl;
		return;
	}

	if (endIndex < 0 || endIndex > 1023) {
		cerr << "Error in Rhd2000EvalBoard::selectAuxCommandLength: endIndex out of range." << endl;
		return;
	}

	switch (auxCommandSlot) {
	case AuxCmd1:
		writeRegister(AuxCmdLoop1, loopIndex);
		writeRegister(AuxCmdLength1, endIndex);
		break;
	case AuxCmd2:
		writeRegister(AuxCmdLoop2, loopIndex);
		writeRegister(AuxCmdLength2, endIndex);
		break;
	case AuxCmd3:
		writeRegister(AuxCmdLoop3, loopIndex);
		writeRegister(AuxCmdLength3, endIndex);
		break;
	}
}

// Set the FPGA to run continuously once started (if continuousMode == true) or to run until
// maxTimeStep is reached (if continuousMode == false).
void rhd2000PCIe::setContinuousRunMode(bool continuousMode)
{
	if (continuousMode) {
		writeRegister(ResetRun, 0x02, 0x2);
	}
	else {
		writeRegister(ResetRun, 0x00, 0x2);
	}
}

void rhd2000PCIe::setMaxTimeStep(unsigned int maxTimeStep)
{
	unsigned int maxTimeStepLsb, maxTimeStepMsb;

	maxTimeStepLsb = maxTimeStep & 0x0000ffff;
	maxTimeStepMsb = maxTimeStep & 0xffff0000;

	writeRegister(MaxTimeStepLsb, maxTimeStepLsb);
	writeRegister(MaxTimeStepMsb, maxTimeStepMsb >> 16);
}

// Initiate SPI data acquisition.
void rhd2000PCIe::run()
{
	writeRegister(StartTrigger, 0x1);
}

// Is the FPGA currently running?
bool rhd2000PCIe::isRunning() const
{
	int value;

	value = readRegister(SpiRunning);

	if ((value & 0x01) == 0) {
		return false;
	}
	else {
		return true;
	}
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortD), in integer clock
// steps, where each clock step is 1/2800 of a per-channel sampling period.
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void rhd2000PCIe::setCableDelay(BoardPort port, int delay)
{
	int bitShift;

	if (delay < 0 || delay > 15) {
		cerr << "Warning in Rhd2000EvalBoard::setCableDelay: delay out of range: " << delay << endl;
	}

	if (delay < 0) delay = 0;
	if (delay > 15) delay = 15;

	switch (port) {
	case PortA:
		bitShift = 0;
		cableDelay[0] = delay;
		break;
	case PortB:
		bitShift = 4;
		cableDelay[1] = delay;
		break;
	case PortC:
		bitShift = 8;
		cableDelay[2] = delay;
		break;
	case PortD:
		bitShift = 12;
		cableDelay[3] = delay;
		break;
	default:
		cerr << "Error in RHD2000EvalBoard::setCableDelay: unknown port." << endl;
	}

	writeRegister(MisoDelay, delay << bitShift, 0x000f << bitShift);
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortD) based on the length
// of the cable between the FPGA and the RHD2000 chip (in meters).
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void rhd2000PCIe::setCableLengthMeters(BoardPort port, double lengthInMeters)
{
	int delay;
	double tStep, cableVelocity, distance, timeDelay;
	const double speedOfLight = 299792458.0;  // units = meters per second
	const double xilinxLvdsOutputDelay = 1.9e-9;    // 1.9 ns Xilinx LVDS output pin delay
	const double xilinxLvdsInputDelay = 1.4e-9;     // 1.4 ns Xilinx LVDS input pin delay
	const double rhd2000Delay = 9.0e-9;             // 9.0 ns RHD2000 SCLK-to-MISO delay
	const double misoSettleTime = 6.7e-9;           // 6.7 ns delay after MISO changes, before we sample it

	tStep = 1.0 / (2800.0 * getSampleRate());  // data clock that samples MISO has a rate 35 x 80 = 2800x higher than the sampling rate
	// cableVelocity = 0.67 * speedOfLight;  // propogation velocity on cable: version 1.3 and earlier
	cableVelocity = 0.555 * speedOfLight;  // propogation velocity on cable: version 1.4 improvement based on cable measurements
	distance = 2.0 * lengthInMeters;      // round trip distance data must travel on cable
	timeDelay = (distance / cableVelocity) + xilinxLvdsOutputDelay + rhd2000Delay + xilinxLvdsInputDelay + misoSettleTime;

	delay = (int)floor(((timeDelay / tStep) + 1.0) + 0.5);

	if (delay < 1) delay = 1;   // delay of zero is too short (due to I/O delays), even for zero-length cables

	setCableDelay(port, delay);
}

// Same function as above, but accepts lengths in feet instead of meters
void rhd2000PCIe::setCableLengthFeet(BoardPort port, double lengthInFeet)
{
	setCableLengthMeters(port, 0.3048 * lengthInFeet);   // convert feet to meters
}

// Estimate cable length based on a particular delay used in setCableDelay.
// (Note: Depends on sample rate.)
double rhd2000PCIe::estimateCableLengthMeters(int delay) const
{
	double tStep, cableVelocity, distance;
	const double speedOfLight = 299792458.0;  // units = meters per second
	const double xilinxLvdsOutputDelay = 1.9e-9;    // 1.9 ns Xilinx LVDS output pin delay
	const double xilinxLvdsInputDelay = 1.4e-9;     // 1.4 ns Xilinx LVDS input pin delay
	const double rhd2000Delay = 9.0e-9;             // 9.0 ns RHD2000 SCLK-to-MISO delay
	const double misoSettleTime = 6.7e-9;           // 6.7 ns delay after MISO changes, before we sample it

	tStep = 1.0 / (2800.0 * getSampleRate());  // data clock that samples MISO has a rate 35 x 80 = 2800x higher than the sampling rate
	// cableVelocity = 0.67 * speedOfLight;  // propogation velocity on cable: version 1.3 and earlier
	cableVelocity = 0.555 * speedOfLight;  // propogation velocity on cable: version 1.4 improvement based on cable measurements

	// distance = cableVelocity * (delay * tStep - (xilinxLvdsOutputDelay + rhd2000Delay + xilinxLvdsInputDelay));  // version 1.3 and earlier
	distance = cableVelocity * ((((double)delay) - 1.0) * tStep - (xilinxLvdsOutputDelay + rhd2000Delay + xilinxLvdsInputDelay + misoSettleTime));  // version 1.4 improvement
	if (distance < 0.0) distance = 0.0;

	return (distance / 2.0);
}

// Same function as above, but returns length in feet instead of meters
double rhd2000PCIe::estimateCableLengthFeet(int delay) const
{
	return 3.2808 * estimateCableLengthMeters(delay);
}

// Turn on or off DSP settle function in the FPGA.  (Only executes when CONVERT commands are sent.)
void rhd2000PCIe::setDspSettle(bool enabled)
{
	writeRegister(ResetRun, (enabled ? 0x04 : 0x00), 0x04);
}

// Assign a particular data source (e.g., PortA1, PortA2, PortB1,...) to one of the eight
// available USB data streams (0-7).
void rhd2000PCIe::setDataSource(int stream, BoardDataSource dataSource)
{
	int bitShift;
	controlAddr endPoint;

	if (stream < 0 || stream >(MAX_NUM_DATA_STREAMS - 1)) {
		cerr << "Error in Rhd2000EvalBoard::setDataSource: stream out of range." << endl;
		return;
	}

	switch (stream) {
	case 0:
		endPoint = DataStreamSel1234;
		bitShift = 0;
		break;
	case 1:
		endPoint = DataStreamSel1234;
		bitShift = 4;
		break;
	case 2:
		endPoint = DataStreamSel1234;
		bitShift = 8;
		break;
	case 3:
		endPoint = DataStreamSel1234;
		bitShift = 12;
		break;
	case 4:
		endPoint = DataStreamSel5678;
		bitShift = 0;
		break;
	case 5:
		endPoint = DataStreamSel5678;
		bitShift = 4;
		break;
	case 6:
		endPoint = DataStreamSel5678;
		bitShift = 8;
		break;
	case 7:
		endPoint = DataStreamSel5678;
		bitShift = 12;
		break;
	case 8:
		endPoint = DataStreamSel9ABC;
		bitShift = 0;
		break;
	case 9:
		endPoint = DataStreamSel9ABC;
		bitShift = 4;
		break;
	case 10:
		endPoint = DataStreamSel9ABC;
		bitShift = 8;
		break;
	case 11:
		endPoint = DataStreamSel9ABC;
		bitShift = 12;
		break;
	case 12:
		endPoint = DataStreamSelDEF10;
		bitShift = 0;
		break;
	case 13:
		endPoint = DataStreamSelDEF10;
		bitShift = 4;
		break;
	case 14:
		endPoint = DataStreamSelDEF10;
		bitShift = 8;
		break;
	case 15:
		endPoint = DataStreamSelDEF10;
		bitShift = 12;
		break;
	}

	writeRegister(endPoint, dataSource << bitShift, 0x000f << bitShift);
}

// Enable or disable one of the eight available USB data streams (0-7).
void rhd2000PCIe::enableDataStream(int stream, bool enabled)
{
	if (stream < 0 || stream >(MAX_NUM_DATA_STREAMS - 1)) {
		cerr << "Error in Rhd2000EvalBoard::setDataSource: stream out of range." << endl;
		return;
	}

	if (enabled) {
		if (dataStreamEnabled[stream] == 0) {
			writeRegister(DataStreamEn, 0x0001 << stream, 0x0001 << stream);
			dataStreamEnabled[stream] = 1;
			++numDataStreams;
		}
	}
	else {
		if (dataStreamEnabled[stream] == 1) {
			writeRegister(DataStreamEn, 0x0000 << stream, 0x0001 << stream);
			dataStreamEnabled[stream] = 0;
			numDataStreams--;
		}
	}
}

// Returns the number of enabled data streams.
int rhd2000PCIe::getNumEnabledDataStreams() const
{
	return numDataStreams;
}

// Is variable-frequency clock DCM programming done?
bool rhd2000PCIe::isDcmProgDone() const
{
	int value;

	value = readRegister(DataClkLocked);

	return ((value & 0x0002) > 1);
}

// Is variable-frequency clock PLL locked?
bool rhd2000PCIe::isDataClockLocked() const
{
	int value;

	value = readRegister(DataClkLocked);

	return ((value & 0x0001) > 0);
}

// Return FPGA cable delay for selected SPI port.
int rhd2000PCIe::getCableDelay(BoardPort port) const
{
	switch (port) {
	case PortA:
		return cableDelay[0];
	case PortB:
		return cableDelay[1];
	case PortC:
		return cableDelay[2];
	case PortD:
		return cableDelay[3];
	default:
		cerr << "Error in RHD2000EvalBoard::getCableDelay: unknown port." << endl;
		return -1;
	}
}

// Return FPGA cable delays for all SPI ports.
void rhd2000PCIe::getCableDelay(vector<int> &delays) const
{
	if (delays.size() != 4) {
		delays.resize(4);
	}
	for (int i = 0; i < 4; ++i) {
		delays[i] = cableDelay[i];
	}
}

bool rhd2000PCIe::isStreamEnabled(int streamIndex)
{
	if (streamIndex < 0 || streamIndex >(MAX_NUM_DATA_STREAMS - 1))
		return false;

	return dataStreamEnabled[streamIndex];
}

bool rhd2000PCIe::readRawDataBlock(unsigned char** bufferPtr, int nSamples)
{
	unsigned int numBytesToRead;
	unsigned int numread = 0;

	numBytesToRead = 2 * Rhd2000DataBlock::calculateDataBlockSizeInWords(numDataStreams, nSamples);

	if (numBytesToRead > DATA_BUFFER_SIZE) {
		cerr << "Error in rhd2000PCIe::readRawDataBlock: Data buffer size exceeded.  " <<
			"Increase value of DATA_BUFFER_SIZE." << endl;
		*bufferPtr = nullptr;
		return false;
	}

	do {
		int nr = Read(fidFIFO, dataBuffer + numread, numBytesToRead - numread);


		if (nr == 0)
		{
			std::cout << "DMA buffer overflow. Stop acquisition" << std::endl;
			return false;
		}
		else if (nr < 0)
		{
			std::cerr << "Error reading from pipe" << std::endl;
			return false;
		}
		numread += nr;
	} while (numread < numBytesToRead);

	*bufferPtr = dataBuffer;
	return true;
}
/*
void rhd2000PCIe::flush()
{
int nr = 0;
do
{
nr = Read(fidFIFO, &dataBuffer, DATA_BUFFER_SIZE);

} while (nr > 0);

}*/

// Read data block from the USB interface, if one is available.  Returns true if data block
// was available.
bool rhd2000PCIe::readDataBlock(Rhd2000DataBlock *dataBlock, int nSamples)
{
	unsigned int numBytesToRead;
	long res;
	unsigned int numread = 0;

	numBytesToRead = 2 * dataBlock->calculateDataBlockSizeInWords(numDataStreams, nSamples);
	//std::cout << "To read: " << numBytesToRead << std::endl;

	if (numBytesToRead > DATA_BUFFER_SIZE) {
		cerr << "Error in rhd2000PCIe::readDataBlock: Data buffer size exceeded.  " <<
			"Increase value of DATA_BUFFER_SIZE." << endl;
		return false;
	}

	do {
		int nr = Read(fidFIFO, dataBuffer + numread, numBytesToRead - numread);

		if (nr == 0)
		{
			std::cout << "DMA buffer overflow. Stop acquisition" << std::endl;
			return false;
		}
		else if (nr < 0)
		{
			std::cerr << "Error reading from pipe" << std::endl;
			return false;
		}
		numread += nr;
		//std::cout << "Read: " << nr << " total: " << numread << " left: " << numBytesToRead - numread << std::endl;
	} while (numread < numBytesToRead);


	dataBlock->fillFromUsbBuffer(dataBuffer, 0, numDataStreams, nSamples);

	return true;
}

// Reads a certain number of USB data blocks, if the specified number is available, and appends them
// to queue.  Returns true if data blocks were available.
bool rhd2000PCIe::readDataBlocks(int numBlocks, queue<Rhd2000DataBlock> &dataQueue, int nSamples)
{
	unsigned int numWordsToRead, numBytesToRead;
	int i;
	unsigned int numread = 0;
	Rhd2000DataBlock *dataBlock;

	if (nSamples < 1) nSamples = SAMPLES_PER_DATA_BLOCK_PCIE;

	numWordsToRead = numBlocks * dataBlock->calculateDataBlockSizeInWords(numDataStreams, nSamples);


	numBytesToRead = 2 * numWordsToRead;

	if (numBytesToRead > DATA_BUFFER_SIZE) {
		cerr << "Error in rhd2000PCIe::readDataBlocks: Data buffer size exceeded.  " <<
			"Increase value of DATA_BUFFER_SIZE." << endl;
		return false;
	}

	do {
		int nr = Read(fidFIFO, dataBuffer + numread, numBytesToRead - numread);

		if (nr == 0)
		{
			std::cout << "DMA buffer overflow. Stop acquisition" << std::endl;
			return false;

		}
		else if (nr < 0)
		{
			std::cerr << "Error reading from pipe" << std::endl;
			return false;
		}
		numread += nr;
	} while (numread < numBytesToRead);

	dataBlock = new Rhd2000DataBlock(numDataStreams, nSamples);
	for (i = 0; i < numBlocks; ++i) {
		dataBlock->fillFromUsbBuffer(dataBuffer, i, numDataStreams);
		dataQueue.push(*dataBlock);
	}
	delete dataBlock;

	return true;
}

void rhd2000PCIe::setOuputSigs(int sigs)
{
	writeRegister(AuxOutputs, sigs);
}
