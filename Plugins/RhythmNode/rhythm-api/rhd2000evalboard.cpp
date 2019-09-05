//----------------------------------------------------------------------------------
// rhd2000evalboard.cpp
//
// Intan Technoloies RHD2000 Rhythm Interface API
// Rhd2000EvalBoard Class
// Version 1.4 (26 February 2014)
//
// Copyright (c) 2013-2014 Intan Technologies LLC
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>

#include "rhd2000evalboard.h"
#include "rhd2000datablock.h"

#include "okFrontPanelDLL.h"

using namespace std;
using namespace OpalKellyLegacy;

// This class provides access to and control of the Opal Kelly XEM6010 USB/FPGA
// interface board running the Rhythm interface Verilog code.

// Constructor.  Set sampling rate variable to 30.0 kS/s/channel (FPGA default).
Rhd2000EvalBoard::Rhd2000EvalBoard()
{
    int i;
    sampleRate = SampleRate30000Hz; // Rhythm FPGA boots up with 30.0 kS/s/channel sampling rate
    numDataStreams = 0;
    dev = 0;
    usb3 = false;

    for (i = 0; i < MAX_NUM_DATA_STREAMS_USB3; ++i) {
        dataStreamEnabled[i] = 0;
    }

    cableDelay.resize(4, -1);
}

//Destructor: Deletes the device to avoid memory leak in Open ephys
Rhd2000EvalBoard::~Rhd2000EvalBoard()
{
    if (dev != 0) delete dev;
}

// Find an Opal Kelly XEM6010-LX45 board attached to a USB port and open it.
// Returns 1 if successful, -1 if FrontPanel cannot be loaded, and -2 if XEM6010 can't be found.
int Rhd2000EvalBoard::open(const char* libname)
{
    char dll_date[32], dll_time[32];
    string serialNumber = "";
    int i, nDevices;

    cout << "---- Intan Technologies ---- Rhythm RHD2000 Controller v1.41 ----" << endl << endl;
//    if (okFrontPanelDLL_LoadLib(libname) == false) {
//        cerr << "FrontPanel DLL could not be loaded.  " <<
//                "Make sure this DLL is in the application start directory." << endl;
//        return -1;
//    }
    okFrontPanelDLL_GetVersion(dll_date, dll_time);
    cout << endl << "FrontPanel DLL loaded.  Built: " << dll_date << "  " << dll_time << endl;

    dev = new okCFrontPanel;

    cout << endl << "Scanning USB for Opal Kelly devices..." << endl << endl;
    nDevices = dev->GetDeviceCount();
    cout << "Found " << nDevices << " Opal Kelly device" << ((nDevices == 1) ? "" : "s") <<
            " connected:" << endl;
    for (i = 0; i < nDevices; ++i) {
        cout << "  Device #" << i + 1 << ": Opal Kelly " <<
                opalKellyModelName(dev->GetDeviceListModel(i)).c_str() <<
                " with serial number " << dev->GetDeviceListSerial(i).c_str() << endl;
    }
    cout << endl;

	for (i = 0; i < nDevices; ++i)
	{
        okCFrontPanel::BoardModel model = dev->GetDeviceListModel(i);
		if (model == OK_PRODUCT_XEM6010LX45 || model == OK_PRODUCT_XEM6310LX45) //the two models we use
		{
			serialNumber = serialNumber = dev->GetDeviceListSerial(i);
			cout << "Trying to open device with serial " << serialNumber.c_str() << endl;
			if (dev->OpenBySerial(serialNumber) == okCFrontPanel::NoError) 
			{
				cout << "Device opened" << endl;
				if (model == OK_PRODUCT_XEM6310LX45)
					usb3 = true;
				break; //end loop if one device was opened
			}
		}
	}
	if (!dev->IsOpen())
	{
		delete dev;
        dev = 0;
        usb3 = false;
		cerr << "No device could be opened.  Is one connected?" << endl;
		return -2;
	}

    // Configure the on-board PLL appropriately.
    dev->LoadDefaultPLLConfiguration();

    // Get some general information about the XEM.
    cout << "FPGA system clock: " << getSystemClockFreq() << " MHz" << endl; // Should indicate 100 MHz
    cout << "Opal Kelly device firmware version: " << dev->GetDeviceMajorVersion() << "." <<
            dev->GetDeviceMinorVersion() << endl;
    cout << "Opal Kelly device serial number: " << dev->GetSerialNumber().c_str() << endl;
    cout << "Opal Kelly device ID string: " << dev->GetDeviceID().c_str() << endl << endl;

    return 1;
}

// Uploads the configuration file (bitfile) to the FPGA.  Returns true if successful.
bool Rhd2000EvalBoard::uploadFpgaBitfile(string filename)
{
    okCFrontPanel::ErrorCode errorCode = dev->ConfigureFPGA(filename);

    switch (errorCode) {
        case okCFrontPanel::NoError:
            break;
        case okCFrontPanel::DeviceNotOpen:
            cerr << "FPGA configuration failed: Device not open." << endl;
            return(false);
        case okCFrontPanel::FileError:
            cerr << "FPGA configuration failed: Cannot find configuration file." << endl;
            return(false);
        case okCFrontPanel::InvalidBitstream:
            cerr << "FPGA configuration failed: Bitstream is not properly formatted." << endl;
            return(false);
        case okCFrontPanel::DoneNotHigh:
            cerr << "FPGA configuration failed: FPGA DONE signal did not assert after configuration." << endl;
            return(false);
        case okCFrontPanel::TransferError:
            cerr << "FPGA configuration failed: USB error occurred during download." << endl;
            return(false);
        case okCFrontPanel::CommunicationError:
            cerr << "FPGA configuration failed: Communication error with firmware." << endl;
            return(false);
        case okCFrontPanel::UnsupportedFeature:
            cerr << "FPGA configuration failed: Unsupported feature." << endl;
            return(false);
        default:
            cerr << "FPGA configuration failed: Unknown error." << endl;
            return(false);
    }

    // Check for Opal Kelly FrontPanel support in the FPGA configuration.
    if (dev->IsFrontPanelEnabled() == false) {
        cerr << "Opal Kelly FrontPanel support is not enabled in this FPGA configuration." << endl;
        delete dev;
        dev = 0;
        return(false);
    }

    int boardId, boardVersion;
    dev->UpdateWireOuts();
    boardId = dev->GetWireOutValue(WireOutBoardId);
    boardVersion = dev->GetWireOutValue(WireOutBoardVersion);

    if (boardId != (usb3 ? RHYTHM_BOARD_ID_USB3 : RHYTHM_BOARD_ID_USB2)) {
        cerr << "FPGA configuration does not support Rhythm.  Incorrect board ID: " << boardId << endl;
        return(false);
    } else {
        cout << "Rhythm configuration file successfully loaded.  Rhythm version number: " <<
                boardVersion << endl << endl;
    }

    return(true);
}

// Reads system clock frequency from Opal Kelly board (in MHz).  Should be 100 MHz for normal
// Rhythm operation.
double Rhd2000EvalBoard::getSystemClockFreq() const
{
    // Read back the CY22393 PLL configuation
    okCPLL22393 pll;
    dev->GetEepromPLL22393Configuration(pll);

    return pll.GetOutputFrequency(0);
}

// Initialize Rhythm FPGA to default starting values.
void Rhd2000EvalBoard::initialize()
{
    int i;

    resetBoard();
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

    if (usb3)
    {
        setDataSource(8, PortA1);
        setDataSource(9, PortB1);
        setDataSource(10, PortC1);
        setDataSource(11, PortD1);
        setDataSource(12, PortA2);
        setDataSource(13, PortB2);
        setDataSource(14, PortC2);
        setDataSource(15, PortD2);
    }

    enableDataStream(0, true);        // start with only one data stream enabled
    for (i = 1; i < MAX_NUM_DATA_STREAMS(usb3); i++) {
        enableDataStream(i, false);
    }

    clearTtlOut();

    enableDac(0, false);
    enableDac(1, false);
    enableDac(2, false);
    enableDac(3, false);
    enableDac(4, false);
    enableDac(5, false);
    enableDac(6, false);
    enableDac(7, false);
    selectDacDataStream(0, 0);
    selectDacDataStream(1, 0);
    selectDacDataStream(2, 0);
    selectDacDataStream(3, 0);
    selectDacDataStream(4, 0);
    selectDacDataStream(5, 0);
    selectDacDataStream(6, 0);
    selectDacDataStream(7, 0);
    selectDacDataChannel(0, 0);
    selectDacDataChannel(1, 0);
    selectDacDataChannel(2, 0);
    selectDacDataChannel(3, 0);
    selectDacDataChannel(4, 0);
    selectDacDataChannel(5, 0);
    selectDacDataChannel(6, 0);
    selectDacDataChannel(7, 0);

    setDacManual(32768);    // midrange value = 0 V

    setDacGain(0);
    setAudioNoiseSuppress(0);

    setTtlMode(1);          // Digital outputs 0-7 are DAC comparators; 8-15 under manual control

    setDacThreshold(0, 32768, true);
    setDacThreshold(1, 32768, true);
    setDacThreshold(2, 32768, true);
    setDacThreshold(3, 32768, true);
    setDacThreshold(4, 32768, true);
    setDacThreshold(5, 32768, true);
    setDacThreshold(6, 32768, true);
    setDacThreshold(7, 32768, true);

    enableExternalFastSettle(false);
    setExternalFastSettleChannel(0);

    enableExternalDigOut(PortA, false);
    enableExternalDigOut(PortB, false);
    enableExternalDigOut(PortC, false);
    enableExternalDigOut(PortD, false);
    setExternalDigOutChannel(PortA, 0);
    setExternalDigOutChannel(PortB, 0);
    setExternalDigOutChannel(PortC, 0);
    setExternalDigOutChannel(PortD, 0);

    enableBoardLeds(true);
}

// Set the per-channel sampling rate of the RHD2000 chips connected to the FPGA.
bool Rhd2000EvalBoard::setSampleRate(AmplifierSampleRate newSampleRate)
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

    unsigned long M, D;

    switch (newSampleRate) {
    case SampleRate1000Hz:
        M = 7;
        D = 125;
        break;
    case SampleRate1250Hz:
        M = 7;
        D = 100;
        break;
    case SampleRate1500Hz:
        M = 21;
        D = 250;
        break;
    case SampleRate2000Hz:
        M = 14;
        D = 125;
        break;
    case SampleRate2500Hz:
        M = 35;
        D = 250;
        break;
    case SampleRate3000Hz:
        M = 21;
        D = 125;
        break;
    case SampleRate3333Hz:
        M = 14;
        D = 75;
        break;
    case SampleRate4000Hz:
        M = 28;
        D = 125;
        break;
    case SampleRate5000Hz:
        M = 7;
        D = 25;
        break;
    case SampleRate6250Hz:
        M = 7;
        D = 20;
        break;
    case SampleRate8000Hz:
        M = 112;
        D = 250;
        break;
    case SampleRate10000Hz:
        M = 14;
        D = 25;
        break;
    case SampleRate12500Hz:
        M = 7;
        D = 10;
        break;
    case SampleRate15000Hz:
        M = 21;
        D = 25;
        break;
    case SampleRate20000Hz:
        M = 28;
        D = 25;
        break;
    case SampleRate25000Hz:
        M = 35;
        D = 25;
        break;
    case SampleRate30000Hz:
        M = 42;
        D = 25;
        break;
    default:
        return(false);
    }

    sampleRate = newSampleRate;

    // Wait for DcmProgDone = 1 before reprogramming clock synthesizer
    while (isDcmProgDone() == false) {}

    // Reprogram clock synthesizer
    dev->SetWireInValue(WireInDataFreqPll, (256 * M + D));
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDcmProg, 0);

    // Wait for DataClkLocked = 1 before allowing data acquisition to continue
    while (isDataClockLocked() == false) {}

    return(true);
}

// Returns the current per-channel sampling rate (in Hz) as a floating-point number.
double Rhd2000EvalBoard::getSampleRate() const
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

Rhd2000EvalBoard::AmplifierSampleRate Rhd2000EvalBoard::getSampleRateEnum() const
{
    return sampleRate;
}

// Print a command list to the console in readable form.
void Rhd2000EvalBoard::printCommandList(const vector<int> &commandList) const
{
    unsigned int i;
    int cmd, channel, reg, data;

    cout << endl;
    for (i = 0; i < commandList.size(); ++i) {
        cmd = commandList[i];
        if (cmd < 0 || cmd > 0xffff) {
            cout << "  command[" << i << "] = INVALID COMMAND: " << cmd << endl;
        } else if ((cmd & 0xc000) == 0x0000) {
            channel = (cmd & 0x3f00) >> 8;
            cout << "  command[" << i << "] = CONVERT(" << channel << ")" << endl;
        } else if ((cmd & 0xc000) == 0xc000) {
            reg = (cmd & 0x3f00) >> 8;
            cout << "  command[" << i << "] = READ(" << reg << ")" << endl;
        } else if ((cmd & 0xc000) == 0x8000) {
            reg = (cmd & 0x3f00) >> 8;
            data = (cmd & 0x00ff);
            cout << "  command[" << i << "] = WRITE(" << reg << ",";
            cout << hex << uppercase << internal << setfill('0') << setw(2) << data << nouppercase << dec;
            cout << ")" << endl;
        } else if (cmd == 0x5500) {
            cout << "  command[" << i << "] = CALIBRATE" << endl;
        } else if (cmd == 0x6a00) {
            cout << "  command[" << i << "] = CLEAR" << endl;
        } else {
            cout << "  command[" << i << "] = INVALID COMMAND: ";
            cout << hex << uppercase << internal << setfill('0') << setw(4) << cmd << nouppercase << dec;
            cout << endl;
        }
    }
    cout << endl;
}

// Upload an auxiliary command list to a particular command slot (AuxCmd1, AuxCmd2, or AuxCmd3) and RAM bank (0-15)
// on the FPGA.
void Rhd2000EvalBoard::uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank)
{
    unsigned int i;

    if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
        cerr << "Error in Rhd2000EvalBoard::uploadCommandList: auxCommandSlot out of range." << endl;
        return;
    }

    if (bank < 0 || bank > 15) {
        cerr << "Error in Rhd2000EvalBoard::uploadCommandList: bank out of range." << endl;
        return;
    }

    for (i = 0; i < commandList.size(); ++i) {
        dev->SetWireInValue(WireInCmdRamData, commandList[i]);
        dev->SetWireInValue(WireInCmdRamAddr, i);
        dev->SetWireInValue(WireInCmdRamBank, bank);
        dev->UpdateWireIns();
        switch (auxCommandSlot) {
            case AuxCmd1:
                dev->ActivateTriggerIn(TrigInRamWrite, 0);
                break;
            case AuxCmd2:
                dev->ActivateTriggerIn(TrigInRamWrite, 1);
                break;
            case AuxCmd3:
                dev->ActivateTriggerIn(TrigInRamWrite, 2);
                break;
        }
    }
}

// Select an auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3) and bank (0-15) for a particular SPI port
// (PortA, PortB, PortC, or PortD) on the FPGA.
void Rhd2000EvalBoard::selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank)
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
        dev->SetWireInValue(WireInAuxCmdBank1, bank << bitShift, 0x000f << bitShift);
        break;
    case AuxCmd2:
        dev->SetWireInValue(WireInAuxCmdBank2, bank << bitShift, 0x000f << bitShift);
        break;
    case AuxCmd3:
        dev->SetWireInValue(WireInAuxCmdBank3, bank << bitShift, 0x000f << bitShift);
        break;
    }
    dev->UpdateWireIns();
}

// Specify a command sequence length (endIndex = 0-1023) and command loop index (0-1023) for a particular
// auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3).
void Rhd2000EvalBoard::selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex)
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
        dev->SetWireInValue(WireInAuxCmdLoop1, loopIndex);
        dev->SetWireInValue(WireInAuxCmdLength1, endIndex);
        break;
    case AuxCmd2:
        dev->SetWireInValue(WireInAuxCmdLoop2, loopIndex);
        dev->SetWireInValue(WireInAuxCmdLength2, endIndex);
        break;
    case AuxCmd3:
        dev->SetWireInValue(WireInAuxCmdLoop3, loopIndex);
        dev->SetWireInValue(WireInAuxCmdLength3, endIndex);
        break;
    }
    dev->UpdateWireIns();
}

// Reset FPGA.  This clears all auxiliary command RAM banks, clears the USB FIFO, and resets the
// per-channel sampling rate to 30.0 kS/s/ch.
void Rhd2000EvalBoard::resetBoard()
{
    dev->SetWireInValue(WireInResetRun, 0x01, 0x01);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInResetRun, 0x00, 0x01);
    dev->UpdateWireIns();
    if (usb3)
    {
        dev->SetWireInValue(WireInMultiUse, USB3_BLOCK_SIZE / 4);
        dev->UpdateWireIns();
        dev->ActivateTriggerIn(TrigInOpenEphys, 16);
        cout << "Blocksize set to " << USB3_BLOCK_SIZE << endl;
        dev->SetWireInValue(WireInMultiUse, DDR_BLOCK_SIZE);
        dev->UpdateWireIns();
        dev->ActivateTriggerIn(TrigInOpenEphys, 17);
        cout << "DDR burst set to " << DDR_BLOCK_SIZE << endl;
    }
}

// Set the FPGA to run continuously once started (if continuousMode == true) or to run until
// maxTimeStep is reached (if continuousMode == false).
void Rhd2000EvalBoard::setContinuousRunMode(bool continuousMode)
{
    if (continuousMode) {
        dev->SetWireInValue(WireInResetRun, 0x02, 0x02);
    } else {
        dev->SetWireInValue(WireInResetRun, 0x00, 0x02);
    }
    dev->UpdateWireIns();
}

// Set maxTimeStep for cases where continuousMode == false.
void Rhd2000EvalBoard::setMaxTimeStep(unsigned int maxTimeStep)
{
    unsigned int maxTimeStepLsb, maxTimeStepMsb;

    maxTimeStepLsb = maxTimeStep & 0x0000ffff;
    maxTimeStepMsb = maxTimeStep & 0xffff0000;

    dev->SetWireInValue(WireInMaxTimeStepLsb, maxTimeStepLsb);
    dev->SetWireInValue(WireInMaxTimeStepMsb, maxTimeStepMsb >> 16);
    dev->UpdateWireIns();


}

// Initiate SPI data acquisition.
void Rhd2000EvalBoard::run()
{
    dev->UpdateWireOuts();
//  std::cout << "Block size: " << dev->GetWireOutValue(0x26) << std::endl;
//  std::cout << "Burst len: " << dev->GetWireOutValue(0x27) << std::endl;
    dev->ActivateTriggerIn(TrigInSpiStart, 0);
}

// Is the FPGA currently running?
bool Rhd2000EvalBoard::isRunning() const
{
    int value;

    dev->UpdateWireOuts();
    value = dev->GetWireOutValue(WireOutSpiRunning);

    if ((value & 0x01) == 0) {
        return false;
    } else {
        return true;
    }
}

// Returns the number of 16-bit words in the USB FIFO.  The user should never attempt to read
// more data than the FIFO currently contains, as it is not protected against underflow.
unsigned int Rhd2000EvalBoard::numWordsInFifo() const
{
    dev->UpdateWireOuts();
    return (dev->GetWireOutValue(WireOutNumWordsMsb) << 16) + dev->GetWireOutValue(WireOutNumWordsLsb);
}

// Returns the number of 16-bit words the USB SDRAM FIFO can hold.  The FIFO can actually hold a few
// thousand words more than the number returned by this method due to FPGA "mini-FIFOs" interfacing
// with the SDRAM, but this provides a conservative estimate of FIFO capacity.
unsigned int Rhd2000EvalBoard::fifoCapacityInWords()
{
    return FIFO_CAPACITY_WORDS;
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortD), in integer clock
// steps, where each clock step is 1/2800 of a per-channel sampling period.
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void Rhd2000EvalBoard::setCableDelay(BoardPort port, int delay)
{
    int bitShift;

    if (delay < 0 || delay > 15) {
        cerr << "Warning in Rhd2000EvalBoard::setCableDelay: delay out of range: " << delay  << endl;
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

    dev->SetWireInValue(WireInMisoDelay, delay << bitShift, 0x000f << bitShift);
    dev->UpdateWireIns();
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortD) based on the length
// of the cable between the FPGA and the RHD2000 chip (in meters).
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void Rhd2000EvalBoard::setCableLengthMeters(BoardPort port, double lengthInMeters)
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

    delay = (int) floor(((timeDelay / tStep) + 1.0) + 0.5);

    if (delay < 1) delay = 1;   // delay of zero is too short (due to I/O delays), even for zero-length cables

    setCableDelay(port, delay);
}

// Same function as above, but accepts lengths in feet instead of meters
void Rhd2000EvalBoard::setCableLengthFeet(BoardPort port, double lengthInFeet)
{
    setCableLengthMeters(port, 0.3048 * lengthInFeet);   // convert feet to meters
}

// Estimate cable length based on a particular delay used in setCableDelay.
// (Note: Depends on sample rate.)
double Rhd2000EvalBoard::estimateCableLengthMeters(int delay) const
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
    distance = cableVelocity * ((((double) delay) - 1.0) * tStep - (xilinxLvdsOutputDelay + rhd2000Delay + xilinxLvdsInputDelay + misoSettleTime));  // version 1.4 improvement
    if (distance < 0.0) distance = 0.0;

    return (distance / 2.0);
}

// Same function as above, but returns length in feet instead of meters
double Rhd2000EvalBoard::estimateCableLengthFeet(int delay) const
{
    return 3.2808 * estimateCableLengthMeters(delay);
}

// Turn on or off DSP settle function in the FPGA.  (Only executes when CONVERT commands are sent.)
void Rhd2000EvalBoard::setDspSettle(bool enabled)
{
    dev->SetWireInValue(WireInResetRun, (enabled ? 0x04 : 0x00), 0x04);
    dev->UpdateWireIns();
}

// Assign a particular data source (e.g., PortA1, PortA2, PortB1,...) to one of the eight
// available USB data streams (0-7).
void Rhd2000EvalBoard::setDataSource(int stream, BoardDataSource dataSource)
{
    int bitShift;
    OkEndPoint endPoint;

    if (stream < 0 || stream > (MAX_NUM_DATA_STREAMS(usb3) - 1)) {
        cerr << "Error in Rhd2000EvalBoard::setDataSource: stream out of range." << endl;
        return;
    }

    switch (stream) {
    case 0:
        endPoint = WireInDataStreamSel1234;
        bitShift = 0;
        break;
    case 1:
        endPoint = WireInDataStreamSel1234;
        bitShift = 4;
        break;
    case 2:
        endPoint = WireInDataStreamSel1234;
        bitShift = 8;
        break;
    case 3:
        endPoint = WireInDataStreamSel1234;
        bitShift = 12;
        break;
    case 4:
        endPoint = WireInDataStreamSel5678;
        bitShift = 0;
        break;
    case 5:
        endPoint = WireInDataStreamSel5678;
        bitShift = 4;
        break;
    case 6:
        endPoint = WireInDataStreamSel5678;
        bitShift = 8;
        break;
    case 7:
        endPoint = WireInDataStreamSel5678;
        bitShift = 12;
        break;
    case 8:
        endPoint = WireInDataStreamSel1234;
        bitShift = 16;
        break;
    case 9:
        endPoint = WireInDataStreamSel1234;
        bitShift = 20;
        break;
    case 10:
        endPoint = WireInDataStreamSel1234;
        bitShift = 24;
        break;
    case 11:
        endPoint = WireInDataStreamSel1234;
        bitShift = 28;
        break;
    case 12:
        endPoint = WireInDataStreamSel5678;
        bitShift = 16;
        break;
    case 13:
        endPoint = WireInDataStreamSel5678;
        bitShift = 20;
        break;
    case 14:
        endPoint = WireInDataStreamSel5678;
        bitShift = 24;
        break;
    case 15:
        endPoint = WireInDataStreamSel5678;
        bitShift = 28;
        break;
    }

    dev->SetWireInValue(endPoint, dataSource << bitShift, 0x000f << bitShift);
    dev->UpdateWireIns();
}

// Enable or disable one of the eight available USB data streams (0-7).
void Rhd2000EvalBoard::enableDataStream(int stream, bool enabled)
{
    if (stream < 0 || stream > (MAX_NUM_DATA_STREAMS(usb3) - 1)) {
        cerr << "Error in Rhd2000EvalBoard::setDataSource: stream out of range." << endl;
        return;
    }

    if (enabled) {
        if (dataStreamEnabled[stream] == 0) {
            dev->SetWireInValue(WireInDataStreamEn, 0x0001 << stream, 0x0001 << stream);
            dev->UpdateWireIns();
            dataStreamEnabled[stream] = 1;
            ++numDataStreams;
        }
    } else {
        if (dataStreamEnabled[stream] == 1) {
            dev->SetWireInValue(WireInDataStreamEn, 0x0000 << stream, 0x0001 << stream);
            dev->UpdateWireIns();
            dataStreamEnabled[stream] = 0;
            numDataStreams--;
        }
    }
}

// Returns the number of enabled data streams.
int Rhd2000EvalBoard::getNumEnabledDataStreams() const
{
    return numDataStreams;
}

// Set all 16 bits of the digital TTL output lines on the FPGA to zero.
void Rhd2000EvalBoard::clearTtlOut()
{
    dev->SetWireInValue(WireInTtlOut, 0x0000);
    dev->UpdateWireIns();
}

// Set the 16 bits of the digital TTL output lines on the FPGA high or low according to integer array.
void Rhd2000EvalBoard::setTtlOut(int ttlOutArray[])
{
    int i, ttlOut;

    ttlOut = 0;
    for (i = 0; i < 16; ++i) {
        if (ttlOutArray[i] > 0)
            ttlOut += 1 << i;
    }
    dev->SetWireInValue(WireInTtlOut, ttlOut);
    dev->UpdateWireIns();
}

// Read the 16 bits of the digital TTL input lines on the FPGA into an integer array.
void Rhd2000EvalBoard::getTtlIn(int ttlInArray[])
{
    int i, ttlIn;

    dev->UpdateWireOuts();
    ttlIn = dev->GetWireOutValue(WireOutTtlIn);

    for (i = 0; i < 16; ++i) {
        ttlInArray[i] = 0;
        if ((ttlIn & (1 << i)) > 0)
            ttlInArray[i] = 1;
    }
}

// Set manual value for DACs.
void Rhd2000EvalBoard::setDacManual(int value)
{
    if (value < 0 || value > 65535) {
        cerr << "Error in Rhd2000EvalBoard::setDacManual: value out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInDacManual, value);
    dev->UpdateWireIns();
}

// Set the eight red LEDs on the XEM6010 board according to integer array.
void Rhd2000EvalBoard::setLedDisplay(int ledArray[])
{
    int i, ledOut;

    ledOut = 0;
    for (i = 0; i < 8; ++i) {
        if (ledArray[i] > 0)
            ledOut += 1 << i;
    }
    dev->SetWireInValue(WireInLedDisplay, ledOut);
    dev->UpdateWireIns();
}

// Enable or disable AD5662 DAC channel (0-7)
void Rhd2000EvalBoard::enableDac(int dacChannel, bool enabled)
{
    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoard::enableDac: dacChannel out of range." << endl;
        return;
    }

    UINT32 dacEnMask = usb3 ? 0x0400 : 0x0200;

    switch (dacChannel) {
    case 0:
        dev->SetWireInValue(WireInDacSource1, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 1:
        dev->SetWireInValue(WireInDacSource2, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 2:
        dev->SetWireInValue(WireInDacSource3, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 3:
        dev->SetWireInValue(WireInDacSource4, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 4:
        dev->SetWireInValue(WireInDacSource5, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 5:
        dev->SetWireInValue(WireInDacSource6, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 6:
        dev->SetWireInValue(WireInDacSource7, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    case 7:
        dev->SetWireInValue(WireInDacSource8, (enabled ? dacEnMask : 0x0000), dacEnMask);
        break;
    }
    dev->UpdateWireIns();
}

// Set the gain level of all eight DAC channels to 2^gain (gain = 0-7).
void Rhd2000EvalBoard::setDacGain(int gain)
{
    if (gain < 0 || gain > 7) {
        cerr << "Error in Rhd2000EvalBoard::setDacGain: gain out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, gain << 13, 0xe000);
    dev->UpdateWireIns();
}

// Suppress the noise on DAC channels 0 and 1 (the audio channels) between
// +16*noiseSuppress and -16*noiseSuppress LSBs.  (noiseSuppress = 0-127).
void Rhd2000EvalBoard::setAudioNoiseSuppress(int noiseSuppress)
{
    if (noiseSuppress < 0 || noiseSuppress > 127) {
        cerr << "Error in Rhd2000EvalBoard::setAudioNoiseSuppress: noiseSuppress out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, noiseSuppress << 6, 0x1fc0);
    dev->UpdateWireIns();
}

// Assign a particular data stream (0-7) to a DAC channel (0-7).  Setting stream
// to 8 selects DacManual1 value; setting stream to 9 selects DacManual2 value.
void Rhd2000EvalBoard::selectDacDataStream(int dacChannel, int stream)
{
    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoard::selectDacDataStream: dacChannel out of range." << endl;
        return;
    }

    if (stream < 0 || stream > MAX_NUM_DATA_STREAMS(usb3)+1) {
        cerr << "Error in Rhd2000EvalBoard::selectDacDataStream: stream out of range." << endl;
        return;
    }

    UINT32 dacStreamMask = (usb3 ? 0x03e0 : 0x01e0);

    switch (dacChannel) {
    case 0:
        dev->SetWireInValue(WireInDacSource1, stream << 5, dacStreamMask);
        break;
    case 1:
        dev->SetWireInValue(WireInDacSource2, stream << 5, dacStreamMask);
        break;
    case 2:
        dev->SetWireInValue(WireInDacSource3, stream << 5, dacStreamMask);
        break;
    case 3:
        dev->SetWireInValue(WireInDacSource4, stream << 5, dacStreamMask);
        break;
    case 4:
        dev->SetWireInValue(WireInDacSource5, stream << 5, dacStreamMask);
        break;
    case 5:
        dev->SetWireInValue(WireInDacSource6, stream << 5, dacStreamMask);
        break;
    case 6:
        dev->SetWireInValue(WireInDacSource7, stream << 5, dacStreamMask);
        break;
    case 7:
        dev->SetWireInValue(WireInDacSource8, stream << 5, dacStreamMask);
        break;
    }
    dev->UpdateWireIns();
}

// Assign a particular amplifier channel (0-31) to a DAC channel (0-7).
void Rhd2000EvalBoard::selectDacDataChannel(int dacChannel, int dataChannel)
{
    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoard::selectDacDataChannel: dacChannel out of range." << endl;
        return;
    }

    if (dataChannel < 0 || dataChannel > 31) {
        cerr << "Error in Rhd2000EvalBoard::selectDacDataChannel: dataChannel out of range." << endl;
        return;
    }

    switch (dacChannel) {
    case 0:
        dev->SetWireInValue(WireInDacSource1, dataChannel << 0, 0x001f);
        break;
    case 1:
        dev->SetWireInValue(WireInDacSource2, dataChannel << 0, 0x001f);
        break;
    case 2:
        dev->SetWireInValue(WireInDacSource3, dataChannel << 0, 0x001f);
        break;
    case 3:
        dev->SetWireInValue(WireInDacSource4, dataChannel << 0, 0x001f);
        break;
    case 4:
        dev->SetWireInValue(WireInDacSource5, dataChannel << 0, 0x001f);
        break;
    case 5:
        dev->SetWireInValue(WireInDacSource6, dataChannel << 0, 0x001f);
        break;
    case 6:
        dev->SetWireInValue(WireInDacSource7, dataChannel << 0, 0x001f);
        break;
    case 7:
        dev->SetWireInValue(WireInDacSource8, dataChannel << 0, 0x001f);
        break;
    }
    dev->UpdateWireIns();
}

// Enable external triggering of amplifier hardware 'fast settle' function (blanking).
// If external triggering is enabled, the fast settling of amplifiers on all connected
// chips will be controlled in real time via one of the 16 TTL inputs.
void Rhd2000EvalBoard::enableExternalFastSettle(bool enable)
{
    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInExtFastSettle, 0);
}

// Select which of the TTL inputs 0-15 is used to perform a hardware 'fast settle' (blanking)
// of the amplifiers if external triggering of fast settling is enabled.
void Rhd2000EvalBoard::setExternalFastSettleChannel(int channel)
{
    if (channel < 0 || channel > 15) {
        cerr << "Error in Rhd2000EvalBoard::setExternalFastSettleChannel: channel "<< channel << " out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInMultiUse, channel);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInExtFastSettle, 1);
}

// Enable external control of RHD2000 auxiliary digital output pin (auxout).
// If external control is enabled, the digital output of all chips connected to a
// selected SPI port will be controlled in real time via one of the 16 TTL inputs.
void Rhd2000EvalBoard::enableExternalDigOut(BoardPort port, bool enable)
{
    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();

    switch (port) {
    case PortA:
        dev->ActivateTriggerIn(TrigInExtDigOut, 0);
        break;
    case PortB:
        dev->ActivateTriggerIn(TrigInExtDigOut, 1);
        break;
    case PortC:
        dev->ActivateTriggerIn(TrigInExtDigOut, 2);
        break;
    case PortD:
        dev->ActivateTriggerIn(TrigInExtDigOut, 3);
        break;
    default:
        cerr << "Error in Rhd2000EvalBoard::enableExternalDigOut: port out of range." << endl;
    }
}

// Select which of the TTL inputs 0-15 is used to control the auxiliary digital output
// pin of the chips connected to a particular SPI port, if external control of auxout is enabled.
void Rhd2000EvalBoard::setExternalDigOutChannel(BoardPort port, int channel)
{
    if (channel < 0 || channel > 15) {
        cerr << "Error in Rhd2000EvalBoard::setExternalDigOutChannel: channel out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInMultiUse, channel);
    dev->UpdateWireIns();

    switch (port) {
    case PortA:
        dev->ActivateTriggerIn(TrigInExtDigOut, 4);
        break;
    case PortB:
        dev->ActivateTriggerIn(TrigInExtDigOut, 5);
        break;
    case PortC:
        dev->ActivateTriggerIn(TrigInExtDigOut, 6);
        break;
    case PortD:
        dev->ActivateTriggerIn(TrigInExtDigOut, 7);
        break;
    default:
        cerr << "Error in Rhd2000EvalBoard::setExternalDigOutChannel: port out of range." << endl;
    }
}

// Enable optional FPGA-implemented digital high-pass filters associated with DAC outputs
// on USB interface board.. These one-pole filters can be used to record wideband neural data
// while viewing only spikes without LFPs on the DAC outputs, for example.  This is useful when
// using the low-latency FPGA thresholds to detect spikes and produce digital pulses on the TTL
// outputs, for example.
void Rhd2000EvalBoard::enableDacHighpassFilter(bool enable)
{
    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacHpf, 0);
}

// Set cutoff frequency (in Hz) for optional FPGA-implemented digital high-pass filters
// associated with DAC outputs on USB interface board.  These one-pole filters can be used
// to record wideband neural data while viewing only spikes without LFPs on the DAC outputs,
// for example.  This is useful when using the low-latency FPGA thresholds to detect spikes
// and produce digital pulses on the TTL outputs, for example.
void Rhd2000EvalBoard::setDacHighpassFilter(double cutoff)
{
    double b;
    int filterCoefficient;
    const double pi = 3.1415926535897;

    // Note that the filter coefficient is a function of the amplifier sample rate, so this
    // function should be called after the sample rate is changed.
    b = 1.0 - exp(-2.0 * pi * cutoff / getSampleRate());

    // In hardware, the filter coefficient is represented as a 16-bit number.
    filterCoefficient = (int) floor(65536.0 * b + 0.5);

    if (filterCoefficient < 1) {
        filterCoefficient = 1;
    } else if (filterCoefficient > 65535) {
        filterCoefficient = 65535;
    }

    dev->SetWireInValue(WireInMultiUse, filterCoefficient);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacHpf, 1);
}

// Set thresholds for DAC channels; threshold output signals appear on TTL outputs 0-7.
// The parameter 'threshold' corresponds to the RHD2000 chip ADC output value, and must fall
// in the range of 0 to 65535, where the 'zero' level is 32768.
// If trigPolarity is true, voltages equaling or rising above the threshold produce a high TTL output.
// If trigPolarity is false, voltages equaling or falling below the threshold produce a high TTL output.
void Rhd2000EvalBoard::setDacThreshold(int dacChannel, int threshold, bool trigPolarity)
{
    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoard::setDacThreshold: dacChannel out of range." << endl;
        return;
    }

    if (threshold < 0 || threshold > 65535) {
        cerr << "Error in Rhd2000EvalBoard::setDacThreshold: threshold out of range." << endl;
        return;
    }

    // Set threshold level.
    dev->SetWireInValue(WireInMultiUse, threshold);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacThresh, dacChannel);

    // Set threshold polarity.
    dev->SetWireInValue(WireInMultiUse, (trigPolarity ? 1 : 0));
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacThresh, dacChannel + 8);
}

// Set the TTL output mode of the board.
// mode = 0: All 16 TTL outputs are under manual control
// mode = 1: Top 8 TTL outputs are under manual control;
//           Bottom 8 TTL outputs are outputs of DAC comparators
void Rhd2000EvalBoard::setTtlMode(int mode)
{
    if (mode < 0 || mode > 1) {
        cerr << "Error in Rhd2000EvalBoard::setTtlMode: mode out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, mode << 3, 0x0008);
    dev->UpdateWireIns();
}

// Is variable-frequency clock DCM programming done?
bool Rhd2000EvalBoard::isDcmProgDone() const
{
    int value;

    dev->UpdateWireOuts();
    value = dev->GetWireOutValue(WireOutDataClkLocked);

    return ((value & 0x0002) > 1);
}

// Is variable-frequency clock PLL locked?
bool Rhd2000EvalBoard::isDataClockLocked() const
{
    int value;

    dev->UpdateWireOuts();
    value = dev->GetWireOutValue(WireOutDataClkLocked);

    return ((value & 0x0001) > 0);
}

// Flush all remaining data out of the FIFO.  (This function should only be called when SPI
// data acquisition has been stopped.)
void Rhd2000EvalBoard::flush()
{

    if (usb3)
    {
        dev->SetWireInValue(WireInResetRun, 1 << 16, 1 << 16); //Override pipeout block throttle
        dev->UpdateWireIns();
        //cout << "Pre-Flush: " << numWordsInFifo() << endl;
        while (numWordsInFifo() >= USB_BUFFER_SIZE / 2) {
            dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, USB_BUFFER_SIZE, usbBuffer);
        //  cout << "Flush phase A: " << numWordsInFifo() << endl;
        }
        while (numWordsInFifo() > 0) {
            dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, USB3_BLOCK_SIZE *max(2 * numWordsInFifo() / USB3_BLOCK_SIZE, (unsigned int)1), usbBuffer);
        //  cout << "Flush phase B: " << numWordsInFifo() << endl;
        //  printFIFOmetrics();
        }
        dev->SetWireInValue(WireInResetRun, 0, 1 << 16);
        dev->UpdateWireIns();
    }
    else
    {
        while (numWordsInFifo() >= USB_BUFFER_SIZE / 2) {
            dev->ReadFromPipeOut(PipeOutData, USB_BUFFER_SIZE, usbBuffer);
        }
        while (numWordsInFifo() > 0) {
            dev->ReadFromPipeOut(PipeOutData, 2 * numWordsInFifo(), usbBuffer);
        }
    }
}

// Read data block from the USB interface, if one is available.  Returns true if data block
// was available.
bool Rhd2000EvalBoard::readDataBlock(Rhd2000DataBlock *dataBlock, int nSamples)
{
    unsigned int numBytesToRead;
    long res;

    numBytesToRead = 2 * dataBlock->calculateDataBlockSizeInWords(numDataStreams, usb3, nSamples);

    if (numBytesToRead > USB_BUFFER_SIZE) {
        cerr << "Error in Rhd2000EvalBoard::readDataBlock: USB buffer size exceeded.  " <<
                "Increase value of USB_BUFFER_SIZE." << endl;
        return false;
    }

    if (usb3)
    {
        //std::cout << "usb3 read : " << numBytesToRead << " in " << USB3_BLOCK_SIZE << " blocks" << std::endl;
        res = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, numBytesToRead, usbBuffer);

    }
    else
    {
        //std::cout << "usb2 read: " << numBytesToRead << std::endl;
        res = dev->ReadFromPipeOut(PipeOutData, numBytesToRead, usbBuffer);
    }
    if (res == ok_Timeout)
    {
        cerr << "CRITICAL: Timeout on pipe read. Check block and buffer sizes." << endl;
    }
    dataBlock->fillFromUsbBuffer(usbBuffer, 0, numDataStreams, nSamples);

    return true;
}

bool Rhd2000EvalBoard::readRawDataBlock(unsigned char** bufferPtr, int nSamples)
{
    unsigned int numBytesToRead;
    long res;

    numBytesToRead = 2 * Rhd2000DataBlock::calculateDataBlockSizeInWords(numDataStreams, usb3, nSamples);

    if (numBytesToRead > USB_BUFFER_SIZE) {
        cerr << "Error in Rhd2000EvalBoard::readDataBlock: USB buffer size exceeded.  " <<
            "Increase value of USB_BUFFER_SIZE." << endl;
        *bufferPtr = nullptr;
        return false;
    }

    if (usb3)
    {
        //std::cout << "usb3 read : " << numBytesToRead << " in " << USB3_BLOCK_SIZE << " blocks" << std::endl;
        res = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, numBytesToRead, usbBuffer);

    }
    else
    {
        //std::cout << "usb2 read: " << numBytesToRead << std::endl;
        res = dev->ReadFromPipeOut(PipeOutData, numBytesToRead, usbBuffer);
    }
    if (res == ok_Timeout)
    {
        cerr << "CRITICAL: Timeout on pipe read. Check block and buffer sizes." << endl;
    }
    *bufferPtr = usbBuffer;
    return true;
}

// Reads a certain number of USB data blocks, if the specified number is available, and appends them
// to queue.  Returns true if data blocks were available.
bool Rhd2000EvalBoard::readDataBlocks(int numBlocks, queue<Rhd2000DataBlock> &dataQueue)
{
    unsigned int numWordsToRead, numBytesToRead;
    int i;
    Rhd2000DataBlock *dataBlock;
    long res;

    numWordsToRead = numBlocks * dataBlock->calculateDataBlockSizeInWords(numDataStreams, usb3);

    if (numWordsInFifo() < numWordsToRead)
        return false;

    numBytesToRead = 2 * numWordsToRead;

    if (numBytesToRead > USB_BUFFER_SIZE) {
        cerr << "Error in Rhd2000EvalBoard::readDataBlocks: USB buffer size exceeded.  " <<
                "Increase value of USB_BUFFER_SIZE." << endl;
        return false;
    }

    if (usb3)
    {
        res = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, numBytesToRead, usbBuffer);
    }
    else
    {
        res = dev->ReadFromPipeOut(PipeOutData, numBytesToRead, usbBuffer);
    }
    if (res == ok_Timeout)
    {
        cerr << "CRITICAL: Timeout on pipe read. Check block and buffer sizes." << endl;
    }

    dataBlock = new Rhd2000DataBlock(numDataStreams, usb3);
    for (i = 0; i < numBlocks; ++i) {
        dataBlock->fillFromUsbBuffer(usbBuffer, i, numDataStreams);
        dataQueue.push(*dataBlock);
    }
    delete dataBlock;

    return true;
}

// Writes the contents of a data block queue (dataQueue) to a binary output stream (saveOut).
// Returns the number of data blocks written.
int Rhd2000EvalBoard::queueToFile(queue<Rhd2000DataBlock> &dataQueue, ofstream &saveOut)
{
    int count = 0;

    while (!dataQueue.empty()) {
        dataQueue.front().write(saveOut, getNumEnabledDataStreams());
        dataQueue.pop();
        ++count;
    }

    return count;
}

// Return name of Opal Kelly board based on model code.
string Rhd2000EvalBoard::opalKellyModelName(int model) const
{
    switch (model) {
    case OK_PRODUCT_XEM3001V1:
        return("XEM3001V1");
    case OK_PRODUCT_XEM3001V2:
        return("XEM3001V2");
    case OK_PRODUCT_XEM3010:
        return("XEM3010");
    case OK_PRODUCT_XEM3005:
        return("XEM3005");
    case OK_PRODUCT_XEM3001CL:
        return("XEM3001CL");
    case OK_PRODUCT_XEM3020:
        return("XEM3020");
    case OK_PRODUCT_XEM3050:
        return("XEM3050");
    case OK_PRODUCT_XEM9002:
        return("XEM9002");
    case OK_PRODUCT_XEM3001RB:
        return("XEM3001RB");
    case OK_PRODUCT_XEM5010:
        return("XEM5010");
    case OK_PRODUCT_XEM6110LX45:
        return("XEM6110LX45");
    case OK_PRODUCT_XEM6001:
        return("XEM6001");
    case OK_PRODUCT_XEM6010LX45:
        return("XEM6010LX45");
    case OK_PRODUCT_XEM6010LX150:
        return("XEM6010LX150");
    case OK_PRODUCT_XEM6110LX150:
        return("XEM6110LX150");
    case OK_PRODUCT_XEM6006LX9:
        return("XEM6006LX9");
    case OK_PRODUCT_XEM6006LX16:
        return("XEM6006LX16");
    case OK_PRODUCT_XEM6006LX25:
        return("XEM6006LX25");
    case OK_PRODUCT_XEM5010LX110:
        return("XEM5010LX110");
    case OK_PRODUCT_ZEM4310:
        return("ZEM4310");
    case OK_PRODUCT_XEM6310LX45:
        return("XEM6310LX45");
    case OK_PRODUCT_XEM6310LX150:
        return("XEM6310LX150");
    case OK_PRODUCT_XEM6110V2LX45:
        return("XEM6110V2LX45");
    case OK_PRODUCT_XEM6110V2LX150:
        return("XEM6110V2LX150");
    case OK_PRODUCT_XEM6002LX9:
        return("XEM6002LX9");
    case OK_PRODUCT_XEM6320LX130T:
        return("XEM6320LX130T");
    default:
        return("UNKNOWN");
    }
}

// Return 4-bit "board mode" input.
int Rhd2000EvalBoard::getBoardMode() const
{
    int mode;

    dev->UpdateWireOuts();
    mode = dev->GetWireOutValue(WireOutBoardMode);

    cout << "Board mode: " << mode << endl << endl;

    return mode;
}

// Return FPGA cable delay for selected SPI port.
int Rhd2000EvalBoard::getCableDelay(BoardPort port) const
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
void Rhd2000EvalBoard::getCableDelay(vector<int> &delays) const
{
    if (delays.size() != 4) {
        delays.resize(4);
    }
    for (int i = 0; i < 4; ++i) {
        delays[i] = cableDelay[i];
    }
}

// Uses the Opal Kelly library to reset the FPGA
void Rhd2000EvalBoard::resetFpga()
{
    dev->ResetFPGA();
}

bool Rhd2000EvalBoard::isStreamEnabled(int streamIndex)
{
  if (streamIndex < 0 || streamIndex > (MAX_NUM_DATA_STREAMS_USB3 - 1))
    return false;

  return dataStreamEnabled[streamIndex];
}

void Rhd2000EvalBoard::enableBoardLeds(bool enable)
{
    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInOpenEphys, 0);
}

// Ratio    divide_factor
// 1        0
// >=2      Ratio/2
void Rhd2000EvalBoard::setClockDivider(int divide_factor)
{

    dev->SetWireInValue(WireInMultiUse, divide_factor);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInOpenEphys, 1);
}

bool Rhd2000EvalBoard::isUSB3()
{
    return usb3;
}

void Rhd2000EvalBoard::printFIFOmetrics()
{
    dev->UpdateWireOuts();
    std::cout << "In FIFO: " << dev->GetWireOutValue(0x28) << " DDR: " << dev->GetWireOutValue(0x2a) << " Out FIFO: " << dev->GetWireOutValue(0x29) << std::endl;
}
