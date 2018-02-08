//----------------------------------------------------------------------------------
// rhd2000evalboardusb3.cpp
//
// Intan Technoloies RHD2000 USB3 Rhythm Interface API
// Rhd2000EvalBoardUsb3 Class
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

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <queue>
#include <cmath>
#include <mutex>

#include "rhd2000evalboardusb3.h"
#include "rhd2000datablockusb3.h"

#include "okFrontPanelDLL.h"

using namespace std;
using namespace OpalKellyLegacy;

// This class provides access to and control of the Opal Kelly XEM6310 USB/FPGA
// interface board running the Rhythm USB3 interface Verilog code.

// Constructor.  Set sampling rate variable to 30.0 kS/s/channel (FPGA default).
Rhd2000EvalBoardUsb3::Rhd2000EvalBoardUsb3()
{
    int i;
    usbBufferSize = MAX_NUM_BLOCKS * 2 * Rhd2000DataBlockUsb3::calculateDataBlockSizeInWords(MAX_NUM_DATA_STREAMS);
    cout << "Rhd2000EvalBoardUsb3: Allocating " << usbBufferSize / 1.0e6 << " MBytes for USB buffer." << endl;
    usbBuffer = new unsigned char [usbBufferSize];
    sampleRate = SampleRate30000Hz; // Rhythm FPGA boots up with 30.0 kS/s/channel sampling rate
    numDataStreams = 0;

    for (i = 0; i < MAX_NUM_DATA_STREAMS; ++i) {
        dataStreamEnabled[i] = 0;
    }

    cableDelay.resize(MAX_NUM_SPI_PORTS, -1);
    lastNumWordsInFifo = 0;
    numWordsHasBeenUpdated = false;
}

Rhd2000EvalBoardUsb3::~Rhd2000EvalBoardUsb3()
{
    delete [] usbBuffer;
}

// Find an Opal Kelly XEM6310-LX45 board attached to a USB port and open it.
// Returns 1 if successful, -1 if FrontPanel cannot be loaded, and -2 if XEM6310 can't be found.
int Rhd2000EvalBoardUsb3::open()
{
    lock_guard<mutex> lockOk(okMutex);
    char dll_date[32], dll_time[32];
    string serialNumber = "";
    int i, nDevices;

    cout << "---- Intan Technologies ---- Rhythm RHD2000 USB3 Controller v2.0 ----" << endl << endl;
//    if (okFrontPanelDLL_LoadLib(NULL) == false) {
//        cerr << "FrontPanel DLL could not be loaded.  " <<
//                "Make sure this DLL is in the application start directory." << endl;
//        return -1;
//    }
    okFrontPanelDLL_GetVersion(dll_date, dll_time);
    cout << "FrontPanel DLL loaded.  Built: " << dll_date << "  " << dll_time << endl;

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

    // Find first device in list of type XEM6310LX45.
    for (i = 0; i < nDevices; ++i) {
        if (dev->GetDeviceListModel(i) == OK_PRODUCT_XEM6310LX45) {
            serialNumber = dev->GetDeviceListSerial(i);
            break;
        }
    }

    if (serialNumber == "") {
        cerr << "No XEM6310-LX45 Opal Kelly board found." << endl;
        return -2;
    }

    cout << "Attempting to connect to device '" << serialNumber.c_str() << "'\n";

    okCFrontPanel::ErrorCode result = dev->OpenBySerial(serialNumber);
    // Attempt to open device.
    if (result != okCFrontPanel::NoError) {
        delete dev;
        cerr << "Device could not be opened.  Is one connected?" << endl;
        cerr << "Error = " << result << endl;
        return -2;
    }

    // Get some general information about the XEM.
    cout << "Opal Kelly device firmware version: " << dev->GetDeviceMajorVersion() << "." <<
            dev->GetDeviceMinorVersion() << endl;
    cout << "Opal Kelly device serial number: " << dev->GetSerialNumber().c_str() << endl;
    cout << "Opal Kelly device ID string: " << dev->GetDeviceID().c_str() << endl << endl;

    return 1;
}

// Uploads the configuration file (bitfile) to the FPGA.  Returns true if successful.
bool Rhd2000EvalBoardUsb3::uploadFpgaBitfile(string filename)
{
    lock_guard<mutex> lockOk(okMutex);
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
        return(false);
    }

    int boardId, boardVersion;
    dev->UpdateWireOuts();
    boardId = dev->GetWireOutValue(WireOutBoardId);
    boardVersion = dev->GetWireOutValue(WireOutBoardVersion);

    if (boardId != RHYTHM_BOARD_ID) {
        cerr << "FPGA configuration file does not support Rhythm USB3.  Incorrect board ID: " << boardId << endl;
        return(false);
    } else {
        cout << "Rhythm USB3 configuration file successfully loaded." << endl << endl;
    }

    return(true);
}

// Initialize Rhythm FPGA to default starting values.
void Rhd2000EvalBoardUsb3::initialize()
{
    int i;

    resetBoard();
    setSampleRate(SampleRate30000Hz);
    selectAuxCommandBank(PortA, AuxCmd1, 0);
    selectAuxCommandBank(PortB, AuxCmd1, 0);
    selectAuxCommandBank(PortC, AuxCmd1, 0);
    selectAuxCommandBank(PortD, AuxCmd1, 0);
    selectAuxCommandBank(PortE, AuxCmd1, 0);
    selectAuxCommandBank(PortF, AuxCmd1, 0);
    selectAuxCommandBank(PortG, AuxCmd1, 0);
    selectAuxCommandBank(PortH, AuxCmd1, 0);
    selectAuxCommandBank(PortA, AuxCmd2, 0);
    selectAuxCommandBank(PortB, AuxCmd2, 0);
    selectAuxCommandBank(PortC, AuxCmd2, 0);
    selectAuxCommandBank(PortD, AuxCmd2, 0);
    selectAuxCommandBank(PortE, AuxCmd2, 0);
    selectAuxCommandBank(PortF, AuxCmd2, 0);
    selectAuxCommandBank(PortG, AuxCmd2, 0);
    selectAuxCommandBank(PortH, AuxCmd2, 0);
    selectAuxCommandBank(PortA, AuxCmd3, 0);
    selectAuxCommandBank(PortB, AuxCmd3, 0);
    selectAuxCommandBank(PortC, AuxCmd3, 0);
    selectAuxCommandBank(PortD, AuxCmd3, 0);
    selectAuxCommandBank(PortE, AuxCmd3, 0);
    selectAuxCommandBank(PortF, AuxCmd3, 0);
    selectAuxCommandBank(PortG, AuxCmd3, 0);
    selectAuxCommandBank(PortH, AuxCmd3, 0);
    selectAuxCommandLength(AuxCmd1, 0, 0);
    selectAuxCommandLength(AuxCmd2, 0, 0);
    selectAuxCommandLength(AuxCmd3, 0, 0);

    setContinuousRunMode(true);
    setMaxTimeStep(4294967295);  // 4294967295 == (2^32 - 1)

    setCableLengthFeet(PortA, 3.0);  // assume 3 ft cables
    setCableLengthFeet(PortB, 3.0);
    setCableLengthFeet(PortC, 3.0);
    setCableLengthFeet(PortD, 3.0);
    setCableLengthFeet(PortE, 3.0);
    setCableLengthFeet(PortF, 3.0);
    setCableLengthFeet(PortG, 3.0);
    setCableLengthFeet(PortH, 3.0);

    setDspSettle(false);

    // Must first force all data streams off
    dev->SetWireInValue(WireInDataStreamEn, 0x00000000);
    dev->UpdateWireIns();

    enableDataStream(0, true);        // start with only one data stream enabled
    for (i = 1; i < MAX_NUM_DATA_STREAMS; i++) {
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
    enableExternalDigOut(PortE, false);
    enableExternalDigOut(PortF, false);
    enableExternalDigOut(PortG, false);
    enableExternalDigOut(PortH, false);
    setExternalDigOutChannel(PortA, 0);
    setExternalDigOutChannel(PortB, 0);
    setExternalDigOutChannel(PortC, 0);
    setExternalDigOutChannel(PortD, 0);
    setExternalDigOutChannel(PortE, 0);
    setExternalDigOutChannel(PortF, 0);
    setExternalDigOutChannel(PortG, 0);
    setExternalDigOutChannel(PortH, 0);

    enableDacReref(false);
}

// Set the per-channel sampling rate of the RHD2000 chips connected to the FPGA.
bool Rhd2000EvalBoardUsb3::setSampleRate(AmplifierSampleRate newSampleRate)
{
    lock_guard<mutex> lockOk(okMutex);

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
    dev->ActivateTriggerIn(TrigInConfig, 0);

    // Wait for DataClkLocked = 1 before allowing data acquisition to continue
    while (isDataClockLocked() == false) {}

    return(true);
}

// Returns the current per-channel sampling rate (in Hz) as a floating-point number.
double Rhd2000EvalBoardUsb3::getSampleRate() const
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

Rhd2000EvalBoardUsb3::AmplifierSampleRate Rhd2000EvalBoardUsb3::getSampleRateEnum() const
{
    return sampleRate;
}

// Print a command list to the console in readable form.
void Rhd2000EvalBoardUsb3::printCommandList(const vector<int> &commandList) const
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
void Rhd2000EvalBoardUsb3::uploadCommandList(const vector<int> &commandList, AuxCmdSlot auxCommandSlot, int bank)
{
    lock_guard<mutex> lockOk(okMutex);
    unsigned int i;

    if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
        cerr << "Error in Rhd2000EvalBoardUsb3::uploadCommandList: auxCommandSlot out of range." << endl;
        return;
    }

    if (bank < 0 || bank > 15) {
        cerr << "Error in Rhd2000EvalBoardUsb3::uploadCommandList: bank out of range." << endl;
        return;
    }

    for (i = 0; i < commandList.size(); ++i) {
        dev->SetWireInValue(WireInCmdRamData, commandList[i]);
        dev->SetWireInValue(WireInCmdRamAddr, i);
        dev->SetWireInValue(WireInCmdRamBank, bank);
        dev->UpdateWireIns();
        switch (auxCommandSlot) {
            case AuxCmd1:
                dev->ActivateTriggerIn(TrigInConfig, 1);
                break;
            case AuxCmd2:
                dev->ActivateTriggerIn(TrigInConfig, 2);
                break;
            case AuxCmd3:
                dev->ActivateTriggerIn(TrigInConfig, 3);
                break;
        }
    }
}

// Select an auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3) and bank (0-15) for a particular SPI port
// (PortA - PortH) on the FPGA.
void Rhd2000EvalBoardUsb3::selectAuxCommandBank(BoardPort port, AuxCmdSlot auxCommandSlot, int bank)
{
    lock_guard<mutex> lockOk(okMutex);
    int bitShift;

    if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectAuxCommandBank: auxCommandSlot out of range." << endl;
        return;
    }
    if (bank < 0 || bank > 15) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectAuxCommandBank: bank out of range." << endl;
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
    case PortE:
        bitShift = 16;
        break;
    case PortF:
        bitShift = 20;
        break;
    case PortG:
        bitShift = 24;
        break;
    case PortH:
        bitShift = 28;
        break;
    }

    switch (auxCommandSlot) {
    case AuxCmd1:
        dev->SetWireInValue(WireInAuxCmdBank1, bank << bitShift, 0x0000000f << bitShift);
        break;
    case AuxCmd2:
        dev->SetWireInValue(WireInAuxCmdBank2, bank << bitShift, 0x0000000f << bitShift);
        break;
    case AuxCmd3:
        dev->SetWireInValue(WireInAuxCmdBank3, bank << bitShift, 0x0000000f << bitShift);
        break;
    }
    dev->UpdateWireIns();
}

// Specify a command sequence length (endIndex = 0-1023) and command loop index (0-1023) for a particular
// auxiliary command slot (AuxCmd1, AuxCmd2, or AuxCmd3).
void Rhd2000EvalBoardUsb3::selectAuxCommandLength(AuxCmdSlot auxCommandSlot, int loopIndex, int endIndex)
{
    lock_guard<mutex> lockOk(okMutex);
    if (auxCommandSlot != AuxCmd1 && auxCommandSlot != AuxCmd2 && auxCommandSlot != AuxCmd3) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectAuxCommandLength: auxCommandSlot out of range." << endl;
        return;
    }

    if (loopIndex < 0 || loopIndex > 1023) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectAuxCommandLength: loopIndex out of range." << endl;
        return;
    }

    if (endIndex < 0 || endIndex > 1023) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectAuxCommandLength: endIndex out of range." << endl;
        return;
    }

    switch (auxCommandSlot) {
    case AuxCmd1:
        dev->SetWireInValue(WireInAuxCmdLoop, loopIndex, 0x000003ff);
        dev->SetWireInValue(WireInAuxCmdLength, endIndex, 0x000003ff);
        break;
    case AuxCmd2:
        dev->SetWireInValue(WireInAuxCmdLoop, loopIndex << 10, 0x000003ff << 10);
        dev->SetWireInValue(WireInAuxCmdLength, endIndex << 10, 0x000003ff << 10);
        break;
    case AuxCmd3:
        dev->SetWireInValue(WireInAuxCmdLoop, loopIndex << 20, 0x000003ff << 20);
        dev->SetWireInValue(WireInAuxCmdLength, endIndex << 20, 0x000003ff << 20);
        break;
    }
    dev->UpdateWireIns();
}

// Reset FPGA.  This clears all auxiliary command RAM banks, clears the USB FIFO, and resets the
// per-channel sampling rate to 30.0 kS/s/ch.
void Rhd2000EvalBoardUsb3::resetBoard()
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInResetRun, 0x01, 0x01);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInResetRun, 0x00, 0x01);
    dev->UpdateWireIns();

    // Set up USB3 block transfer parameters.
    dev->SetWireInValue(WireInMultiUse, USB3_BLOCK_SIZE / 4);  // Divide by 4 to convert from bytes to 32-bit words (used in FPGA FIFO)
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 9);
    dev->SetWireInValue(WireInMultiUse, RAM_BURST_SIZE);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 10);
}

// Low-level FPGA reset.  Call when closing application to make sure everything has stopped.
void Rhd2000EvalBoardUsb3::resetFpga()
{
    lock_guard<mutex> lockOk(okMutex);

    dev->ResetFPGA();
}

// Set the FPGA to run continuously once started (if continuousMode == true) or to run until
// maxTimeStep is reached (if continuousMode == false).
void Rhd2000EvalBoardUsb3::setContinuousRunMode(bool continuousMode)
{
    lock_guard<mutex> lockOk(okMutex);

    if (continuousMode) {
        dev->SetWireInValue(WireInResetRun, 0x02, 0x02);
    } else {
        dev->SetWireInValue(WireInResetRun, 0x00, 0x02);
    }
    dev->UpdateWireIns();
}

// Set maxTimeStep for cases where continuousMode == false.
void Rhd2000EvalBoardUsb3::setMaxTimeStep(unsigned int maxTimeStep)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInMaxTimeStep, maxTimeStep);
    dev->UpdateWireIns();
}

// Initiate SPI data acquisition.
void Rhd2000EvalBoardUsb3::run()
{
    lock_guard<mutex> lockOk(okMutex);

    dev->ActivateTriggerIn(TrigInSpiStart, 0);
}

// Is the FPGA currently running?
bool Rhd2000EvalBoardUsb3::isRunning()
{
    lock_guard<mutex> lockOk(okMutex);
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
// (Private method.)
unsigned int Rhd2000EvalBoardUsb3::numWordsInFifo()
{
    dev->UpdateWireOuts();
    lastNumWordsInFifo = dev->GetWireOutValue(WireOutNumWords);
    numWordsHasBeenUpdated = true;
    return lastNumWordsInFifo;
}

// Returns the number of 16-bit words in the USB FIFO.  The user should never attempt to read
// more data than the FIFO currently contains, as it is not protected against underflow.
// (Public, threadsafe method.)
unsigned int Rhd2000EvalBoardUsb3::getNumWordsInFifo()
{
    lock_guard<mutex> lockOk(okMutex);

    return numWordsInFifo();
}

// Returns the most recently measured number of 16-bit words in the USB FIFO.  Does not directly
// read this value from the USB port, and so may be out of date, but does not have to wait on
// other USB access to finish in order to execute.
unsigned int Rhd2000EvalBoardUsb3::getLastNumWordsInFifo()
{
    numWordsHasBeenUpdated = false;
    return lastNumWordsInFifo;
}

// Returns the most recently measured number of 16-bit words in the USB FIFO.  Does not directly
// read this value from the USB port, and so may be out of date, but does not have to wait on
// other USB access to finish in order to execute.  The boolean variable hasBeenUpdated indicates
// if this value has been updated since the last time this function was called.
unsigned int Rhd2000EvalBoardUsb3::getLastNumWordsInFifo(bool& hasBeenUpdated)
{
    hasBeenUpdated = numWordsHasBeenUpdated;
    numWordsHasBeenUpdated = false;
    return lastNumWordsInFifo;
}

// Returns the number of 16-bit words the USB SDRAM FIFO can hold.  The FIFO can actually hold a few
// thousand words more than the number returned by this method due to FPGA "mini-FIFOs" interfacing
// with the SDRAM, but this provides a conservative estimate of FIFO capacity.
unsigned int Rhd2000EvalBoardUsb3::fifoCapacityInWords()
{
    return FIFO_CAPACITY_WORDS;
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortH), in integer clock
// steps, where each clock step is 1/2800 of a per-channel sampling period.
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void Rhd2000EvalBoardUsb3::setCableDelay(BoardPort port, int delay)
{
    lock_guard<mutex> lockOk(okMutex);
    int bitShift;

    if (delay < 0 || delay > 15) {
        cerr << "Warning in Rhd2000EvalBoardUsb3::setCableDelay: delay out of range: " << delay << endl;
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
    case PortE:
        bitShift = 16;
        cableDelay[4] = delay;
        break;
    case PortF:
        bitShift = 20;
        cableDelay[5] = delay;
        break;
    case PortG:
        bitShift = 24;
        cableDelay[6] = delay;
        break;
    case PortH:
        bitShift = 28;
        cableDelay[7] = delay;
        break;
    default:
        cerr << "Error in RHD2000EvalBoardUsb3::setCableDelay: unknown port." << endl;
    }

    dev->SetWireInValue(WireInMisoDelay, delay << bitShift, 0x0000000f << bitShift);
    dev->UpdateWireIns();
}

// Set the delay for sampling the MISO line on a particular SPI port (PortA - PortH) based on the length
// of the cable between the FPGA and the RHD2000 chip (in meters).
// Note: Cable delay must be updated after sampleRate is changed, since cable delay calculations are
// based on the clock frequency!
void Rhd2000EvalBoardUsb3::setCableLengthMeters(BoardPort port, double lengthInMeters)
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
void Rhd2000EvalBoardUsb3::setCableLengthFeet(BoardPort port, double lengthInFeet)
{
    setCableLengthMeters(port, 0.3048 * lengthInFeet);   // convert feet to meters
}

// Estimate cable length based on a particular delay used in setCableDelay.
// (Note: Depends on sample rate.)
double Rhd2000EvalBoardUsb3::estimateCableLengthMeters(int delay) const
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
double Rhd2000EvalBoardUsb3::estimateCableLengthFeet(int delay) const
{
    return 3.2808 * estimateCableLengthMeters(delay);
}

// Turn on or off DSP settle function in the FPGA.  (Executes only when CONVERT commands are sent.)
void Rhd2000EvalBoardUsb3::setDspSettle(bool enabled)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInResetRun, (enabled ? 0x04 : 0x00), 0x04);
    dev->UpdateWireIns();
}

// Enable or disable one of the 32 available USB data streams (0-31).
void Rhd2000EvalBoardUsb3::enableDataStream(int stream, bool enabled)
{
    lock_guard<mutex> lockOk(okMutex);

    if (stream < 0 || stream > (MAX_NUM_DATA_STREAMS - 1)) {
        cerr << "Error in Rhd2000EvalBoardUsb3::enableDataStream: stream out of range." << endl;
        return;
    }

    if (enabled) {
        if (dataStreamEnabled[stream] == 0) {
            dev->SetWireInValue(WireInDataStreamEn, 0x00000001 << stream, 0x00000001 << stream);
            dev->UpdateWireIns();
            dataStreamEnabled[stream] = 1;
            numDataStreams++;
        }
    } else {
        if (dataStreamEnabled[stream] == 1) {
            dev->SetWireInValue(WireInDataStreamEn, 0x00000000 << stream, 0x00000001 << stream);
            dev->UpdateWireIns();
            dataStreamEnabled[stream] = 0;
            numDataStreams--;
        }
    }
}

// Returns the number of enabled data streams.
int Rhd2000EvalBoardUsb3::getNumEnabledDataStreams() const
{
    return numDataStreams;
}

// Set all 16 bits of the digital TTL output lines on the FPGA to zero.
void Rhd2000EvalBoardUsb3::clearTtlOut()
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInTtlOut, 0x0000);
    dev->UpdateWireIns();
}

// Set the 16 bits of the digital TTL output lines on the FPGA high or low according to integer array.
void Rhd2000EvalBoardUsb3::setTtlOut(int ttlOutArray[])
{
    lock_guard<mutex> lockOk(okMutex);
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
void Rhd2000EvalBoardUsb3::getTtlIn(int ttlInArray[])
{
    lock_guard<mutex> lockOk(okMutex);
    int i, ttlIn;

    dev->UpdateWireOuts();
    ttlIn = dev->GetWireOutValue(WireOutTtlIn);

    for (i = 0; i < 16; ++i) {
        ttlInArray[i] = 0;
        if ((ttlIn & (1 << i)) > 0)
            ttlInArray[i] = 1;
    }
}

// Set manual value for DACs.  Must run SPI commands for this value to take effect.
void Rhd2000EvalBoardUsb3::setDacManual(int value)
{
    lock_guard<mutex> lockOk(okMutex);
    if (value < 0 || value > 65535) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacManual: value out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInDacManual, value);
    dev->UpdateWireIns();
}

// Set the eight red LEDs on the Opal Kelly XEM6310 board according to integer array.
void Rhd2000EvalBoardUsb3::setLedDisplay(int ledArray[])
{
    lock_guard<mutex> lockOk(okMutex);
    int i, ledOut;

    ledOut = 0;
    for (i = 0; i < 8; ++i) {
        if (ledArray[i] > 0)
            ledOut += 1 << i;
    }
    dev->SetWireInValue(WireInLedDisplay, ledOut);
    dev->UpdateWireIns();
}

// Set the eight red LEDs on the front panel SPI ports according to integer array.
void Rhd2000EvalBoardUsb3::setSpiLedDisplay(int ledArray[])
{
    lock_guard<mutex> lockOk(okMutex);
    int i, ledOut;

    ledOut = 0;
    for (i = 0; i < 8; ++i) {
        if (ledArray[i] > 0)
            ledOut += 1 << i;
    }
    dev->SetWireInValue(WireInMultiUse, ledOut);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 8);
}

// Enable or disable DAC channel (0-7)
void Rhd2000EvalBoardUsb3::enableDac(int dacChannel, bool enabled)
{
    lock_guard<mutex> lockOk(okMutex);
    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoardUsb3::enableDac: dacChannel out of range." << endl;
        return;
    }

    switch (dacChannel) {
    case 0:
        dev->SetWireInValue(WireInDacSource1, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 1:
        dev->SetWireInValue(WireInDacSource2, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 2:
        dev->SetWireInValue(WireInDacSource3, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 3:
        dev->SetWireInValue(WireInDacSource4, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 4:
        dev->SetWireInValue(WireInDacSource5, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 5:
        dev->SetWireInValue(WireInDacSource6, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 6:
        dev->SetWireInValue(WireInDacSource7, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    case 7:
        dev->SetWireInValue(WireInDacSource8, (enabled ? 0x0800 : 0x0000), 0x0800);
        break;
    }
    dev->UpdateWireIns();
}

// Set the gain level of all eight DAC channels to 2^gain (gain = 0-7).
void Rhd2000EvalBoardUsb3::setDacGain(int gain)
{
    lock_guard<mutex> lockOk(okMutex);
    if (gain < 0 || gain > 7) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacGain: gain setting out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, gain << 13, 0xe000);
    dev->UpdateWireIns();
}

// Suppress the noise on DAC channels 0 and 1 (the audio channels) between
// +16*noiseSuppress and -16*noiseSuppress LSBs.  (noiseSuppress = 0-127).
void Rhd2000EvalBoardUsb3::setAudioNoiseSuppress(int noiseSuppress)
{
    lock_guard<mutex> lockOk(okMutex);

    if (noiseSuppress < 0 || noiseSuppress > 127) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setAudioNoiseSuppress: noiseSuppress out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, noiseSuppress << 6, 0x1fc0);
    dev->UpdateWireIns();
}

// Assign a particular data stream (0-31) to a DAC channel (0-7).  Setting stream
// to 32 selects DacManual value.
void Rhd2000EvalBoardUsb3::selectDacDataStream(int dacChannel, int stream)
{
    lock_guard<mutex> lockOk(okMutex);

    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectDacDataStream: dacChannel out of range." << endl;
        return;
    }

    if (stream < 0 || stream > MAX_NUM_DATA_STREAMS - 1) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectDacDataStream: stream out of range." << endl;
        return;
    }

    switch (dacChannel) {
    case 0:
        dev->SetWireInValue(WireInDacSource1, stream << 5, 0x07e0);
        break;
    case 1:
        dev->SetWireInValue(WireInDacSource2, stream << 5, 0x07e0);
        break;
    case 2:
        dev->SetWireInValue(WireInDacSource3, stream << 5, 0x07e0);
        break;
    case 3:
        dev->SetWireInValue(WireInDacSource4, stream << 5, 0x07e0);
        break;
    case 4:
        dev->SetWireInValue(WireInDacSource5, stream << 5, 0x07e0);
        break;
    case 5:
        dev->SetWireInValue(WireInDacSource6, stream << 5, 0x07e0);
        break;
    case 6:
        dev->SetWireInValue(WireInDacSource7, stream << 5, 0x07e0);
        break;
    case 7:
        dev->SetWireInValue(WireInDacSource8, stream << 5, 0x07e0);
        break;
    }
    dev->UpdateWireIns();
}

// Assign a particular amplifier channel (0-31) to a DAC channel (0-7).
void Rhd2000EvalBoardUsb3::selectDacDataChannel(int dacChannel, int dataChannel)
{
    lock_guard<mutex> lockOk(okMutex);

    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectDacDataChannel: dacChannel out of range." << endl;
        return;
    }

    if (dataChannel < 0 || dataChannel > 31) {
        cerr << "Error in Rhd2000EvalBoardUsb3::selectDacDataChannel: dataChannel out of range." << endl;
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
void Rhd2000EvalBoardUsb3::enableExternalFastSettle(bool enable)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 6);
}

// Select which of the TTL inputs 0-15 is used to perform a hardware 'fast settle' (blanking)
// of the amplifiers if external triggering of fast settling is enabled.
void Rhd2000EvalBoardUsb3::setExternalFastSettleChannel(int channel)
{
    lock_guard<mutex> lockOk(okMutex);

    if (channel < 0 || channel > 15) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setExternalFastSettleChannel: channel out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInMultiUse, channel);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 7);
}

// Enable external control of RHD2000 auxiliary digital output pin (auxout).
// If external control is enabled, the digital output of all chips connected to a
// selected SPI port will be controlled in real time via one of the 16 TTL inputs.
void Rhd2000EvalBoardUsb3::enableExternalDigOut(BoardPort port, bool enable)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();

    switch (port) {
    case PortA:
        dev->ActivateTriggerIn(TrigInDacConfig, 16);
        break;
    case PortB:
        dev->ActivateTriggerIn(TrigInDacConfig, 17);
        break;
    case PortC:
        dev->ActivateTriggerIn(TrigInDacConfig, 18);
        break;
    case PortD:
        dev->ActivateTriggerIn(TrigInDacConfig, 19);
        break;
    case PortE:
        dev->ActivateTriggerIn(TrigInDacConfig, 20);
        break;
    case PortF:
        dev->ActivateTriggerIn(TrigInDacConfig, 21);
        break;
    case PortG:
        dev->ActivateTriggerIn(TrigInDacConfig, 22);
        break;
    case PortH:
        dev->ActivateTriggerIn(TrigInDacConfig, 23);
        break;
    default:
        cerr << "Error in Rhd2000EvalBoardUsb3::enableExternalDigOut: port out of range." << endl;
    }
}

// Select which of the TTL inputs 0-15 is used to control the auxiliary digital output
// pin of the chips connected to a particular SPI port, if external control of auxout is enabled.
void Rhd2000EvalBoardUsb3::setExternalDigOutChannel(BoardPort port, int channel)
{
    lock_guard<mutex> lockOk(okMutex);

    if (channel < 0 || channel > 15) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setExternalDigOutChannel: channel out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInMultiUse, channel);
    dev->UpdateWireIns();

    switch (port) {
    case PortA:
        dev->ActivateTriggerIn(TrigInDacConfig, 24);
        break;
    case PortB:
        dev->ActivateTriggerIn(TrigInDacConfig, 25);
        break;
    case PortC:
        dev->ActivateTriggerIn(TrigInDacConfig, 26);
        break;
    case PortD:
        dev->ActivateTriggerIn(TrigInDacConfig, 27);
        break;
    case PortE:
        dev->ActivateTriggerIn(TrigInDacConfig, 28);
        break;
    case PortF:
        dev->ActivateTriggerIn(TrigInDacConfig, 29);
        break;
    case PortG:
        dev->ActivateTriggerIn(TrigInDacConfig, 30);
        break;
    case PortH:
        dev->ActivateTriggerIn(TrigInDacConfig, 31);
        break;
    default:
        cerr << "Error in Rhd2000EvalBoardUsb3::setExternalDigOutChannel: port out of range." << endl;
    }
}

// Enable optional FPGA-implemented digital high-pass filters associated with DAC outputs
// on USB interface board.. These one-pole filters can be used to record wideband neural data
// while viewing only spikes without LFPs on the DAC outputs, for example.  This is useful when
// using the low-latency FPGA thresholds to detect spikes and produce digital pulses on the TTL
// outputs, for example.
void Rhd2000EvalBoardUsb3::enableDacHighpassFilter(bool enable)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInMultiUse, enable ? 1 : 0);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInConfig, 4);
}

// Set cutoff frequency (in Hz) for optional FPGA-implemented digital high-pass filters
// associated with DAC outputs on USB interface board.  These one-pole filters can be used
// to record wideband neural data while viewing only spikes without LFPs on the DAC outputs,
// for example.  This is useful when using the low-latency FPGA thresholds to detect spikes
// and produce digital pulses on the TTL outputs, for example.
void Rhd2000EvalBoardUsb3::setDacHighpassFilter(double cutoff)
{
    lock_guard<mutex> lockOk(okMutex);

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
    dev->ActivateTriggerIn(TrigInConfig, 5);
}

// Set thresholds for DAC channels; threshold output signals appear on TTL outputs 0-7.
// The parameter 'threshold' corresponds to the RHD2000 chip ADC output value, and must fall
// in the range of 0 to 65535, where the 'zero' level is 32768.
// If trigPolarity is true, voltages equaling or rising above the threshold produce a high TTL output.
// If trigPolarity is false, voltages equaling or falling below the threshold produce a high TTL output.
void Rhd2000EvalBoardUsb3::setDacThreshold(int dacChannel, int threshold, bool trigPolarity)
{
    lock_guard<mutex> lockOk(okMutex);

    if (dacChannel < 0 || dacChannel > 7) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacThreshold: dacChannel out of range." << endl;
        return;
    }

    if (threshold < 0 || threshold > 65535) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacThreshold: threshold out of range." << endl;
        return;
    }

    // Set threshold level.
    dev->SetWireInValue(WireInMultiUse, threshold);
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacConfig, dacChannel);

    // Set threshold polarity.
    dev->SetWireInValue(WireInMultiUse, (trigPolarity ? 1 : 0));
    dev->UpdateWireIns();
    dev->ActivateTriggerIn(TrigInDacConfig, dacChannel + 8);
}

// Set the TTL output mode of the board.
// mode = 0: All 16 TTL outputs are under manual control
// mode = 1: Top 8 TTL outputs are under manual control;
//           Bottom 8 TTL outputs are outputs of DAC comparators
void Rhd2000EvalBoardUsb3::setTtlMode(int mode)
{
    lock_guard<mutex> lockOk(okMutex);

    if (mode < 0 || mode > 1) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setTtlMode: mode out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInResetRun, mode << 3, 0x0008);
    dev->UpdateWireIns();
}

// Is variable-frequency clock DCM programming done?
bool Rhd2000EvalBoardUsb3::isDcmProgDone() const
{
    int value;

    dev->UpdateWireOuts();
    value = dev->GetWireOutValue(WireOutDataClkLocked);

    return ((value & 0x0002) > 1);
}

// Is variable-frequency clock PLL locked?
bool Rhd2000EvalBoardUsb3::isDataClockLocked() const
{
    int value;

    dev->UpdateWireOuts();
    value = dev->GetWireOutValue(WireOutDataClkLocked);

    return ((value & 0x0001) > 0);
}

// Flush all remaining data out of the FIFO.  (This function should only be called when SPI
// data acquisition has been stopped.)
void Rhd2000EvalBoardUsb3::flush()
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInResetRun, 1 << 16, 1 << 16); // override pipeout block throttle
    dev->UpdateWireIns();

    while (numWordsInFifo() >= usbBufferSize / 2) {
        dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, usbBufferSize, usbBuffer);
    }
    while (numWordsInFifo() > 0) {
        dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, USB3_BLOCK_SIZE * max(2 * numWordsInFifo() / USB3_BLOCK_SIZE, (unsigned int)1), usbBuffer);
    }

    dev->SetWireInValue(WireInResetRun, 0 << 16, 1 << 16);
    dev->UpdateWireIns();
}

// Read data block from the USB interface, if one is available.  Returns true if data block
// was available.
bool Rhd2000EvalBoardUsb3::readDataBlock(Rhd2000DataBlockUsb3 *dataBlock, int nSamples)
{
    lock_guard<mutex> lockOk(okMutex);

    unsigned int numBytesToRead;
    long result;

    numBytesToRead = 2 * dataBlock->calculateDataBlockSizeInWords(numDataStreams, nSamples);

    if (numBytesToRead > usbBufferSize) {
        cerr << "Error in Rhd2000EvalBoardUsb3::readDataBlock: USB buffer size exceeded.  " <<
                "Increase value of MAX_NUM_BLOCKS." << endl;
        return false;
    }
	//std::cout << " Reading " << nSamples << " samples " << numBytesToRead << " bytes with block size " << USB3_BLOCK_SIZE << std::endl;
    result = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, USB3_BLOCK_SIZE * max(numBytesToRead / USB3_BLOCK_SIZE, (unsigned int)1), usbBuffer);
	//std::cout << "Read " << result << std::endl;
    if (result == ok_Failed) {
        cerr << "CRITICAL (readDataBlock): Failure on pipe read.  Check block and buffer sizes." << endl;
    } else if (result == ok_Timeout) {
        cerr << "CRITICAL (readDataBlock): Timeout on pipe read.  Check block and buffer sizes." << endl;
    }

    dataBlock->fillFromUsbBuffer(usbBuffer, 0, numDataStreams, nSamples);

    return true;
}

// Reads a certain number of USB data blocks, if the specified number is available, and writes the raw bytes
// to a buffer.  Returns total number of bytes read.
long Rhd2000EvalBoardUsb3::readDataBlocksRaw(int numBlocks, unsigned char* buffer, int nSamples)
{
    lock_guard<mutex> lockOk(okMutex);

    unsigned int numWordsToRead = numBlocks * Rhd2000DataBlockUsb3::calculateDataBlockSizeInWords(numDataStreams, nSamples);

    if (numWordsInFifo() < numWordsToRead)
	   return 0;
    long result = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, 2 * numWordsToRead, buffer);

    if (result == ok_Failed) {
        cerr << "CRITICAL (readDataBlocksRaw): Failure on BT pipe read.  Check block and buffer sizes." << endl;
    } else if (result == ok_Timeout) {
        cerr << "CRITICAL (readDataBlocksRaw): Timeout on BT pipe read.  Check block and buffer sizes." << endl;
    }

    return result;
}

// Reads a certain number of USB data blocks, if the specified number is available, and appends them
// to queue.  Returns true if data blocks were available.
bool Rhd2000EvalBoardUsb3::readDataBlocks(int numBlocks, queue<Rhd2000DataBlockUsb3> &dataQueue)
{
    lock_guard<mutex> lockOk(okMutex);

    unsigned int numWordsToRead, numBytesToRead;
    int j;
    Rhd2000DataBlockUsb3 *dataBlock;
    long result;

    numWordsToRead = numBlocks * dataBlock->calculateDataBlockSizeInWords(numDataStreams);

    if (numWordsInFifo() < numWordsToRead)
        return false;

    numBytesToRead = 2 * numWordsToRead;

    if (numBytesToRead > usbBufferSize) {
        cerr << "Error in Rhd2000EvalBoardUsb3::readDataBlocks: USB buffer size exceeded.  " <<
                "Increase value of MAX_NUM_BLOCKS." << endl;
        return false;
    }

    result = dev->ReadFromBlockPipeOut(PipeOutData, USB3_BLOCK_SIZE, numBytesToRead, usbBuffer);

    if (result == ok_Failed) {
        cerr << "CRITICAL (readDataBlocks): Failure on pipe read.  Check block and buffer sizes." << endl;
    } else if (result == ok_Timeout) {
        cerr << "CRITICAL (readDataBlocks): Timeout on pipe read.  Check block and buffer sizes." << endl;
    }

    dataBlock = new Rhd2000DataBlockUsb3(numDataStreams);

    for (j = 0; j < numBlocks; ++j) {
        dataBlock->fillFromUsbBuffer(usbBuffer, j, numDataStreams);
        dataQueue.push(*dataBlock);
    }
    delete dataBlock;

    return true;
}

// Writes the contents of a data block queue (dataQueue) to a binary output stream (saveOut).
// Returns the number of data blocks written.
int Rhd2000EvalBoardUsb3::queueToFile(queue<Rhd2000DataBlockUsb3> &dataQueue, ofstream &saveOut)
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
string Rhd2000EvalBoardUsb3::opalKellyModelName(int model) const
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
int Rhd2000EvalBoardUsb3::getBoardMode()
{
    lock_guard<mutex> lockOk(okMutex);
    int mode;

    dev->UpdateWireOuts();
    mode = dev->GetWireOutValue(WireOutBoardMode);

    return mode;
}

// Return FPGA cable delay for selected SPI port.
int Rhd2000EvalBoardUsb3::getCableDelay(BoardPort port) const
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
    case PortE:
        return cableDelay[4];
    case PortF:
        return cableDelay[5];
    case PortG:
        return cableDelay[6];
    case PortH:
        return cableDelay[7];
    default:
        cerr << "Error in Rhd2000EvalBoardUsb3::getCableDelay: unknown port." << endl;
        return -1;
    }
}

// Return FPGA cable delays for all SPI ports.
void Rhd2000EvalBoardUsb3::getCableDelay(vector<int> &delays) const
{
    if (delays.size() != MAX_NUM_SPI_PORTS) {
        delays.resize(MAX_NUM_SPI_PORTS);
    }
    for (int i = 0; i < MAX_NUM_SPI_PORTS; ++i) {
        delays[i] = cableDelay[i];
    }
}

void Rhd2000EvalBoardUsb3::setAllDacsToZero()
{
    int i;

    setDacManual(32768);    // midrange value = 0 V
    for (i = 0; i < 8; i++) {
        selectDacDataStream(i, 32);
    }
}

// Returns number of SPI ports (4 or 8) and if I/O expander board is present
int Rhd2000EvalBoardUsb3::readDigitalInManual(bool& expanderBoardDetected)
{
    lock_guard<mutex> lockOk(okMutex);
    int expanderBoardIdNumber;
    bool spiPortPresent[8];
    bool userId[3];
    bool serialId[4];
    bool digOutVoltageLevel;

    dev->UpdateWireOuts();
    expanderBoardDetected = (dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x04) != 0;
    expanderBoardIdNumber = ((dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x08) ? 1 : 0);

    dev->SetWireInValue(WireInSerialDigitalInCntl, 2);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);  // Load digital in shift registers on falling edge of serial_LOAD
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[7] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[6] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[5] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[4] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[3] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[2] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[1] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    spiPortPresent[0] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    digOutVoltageLevel = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    userId[2] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    userId[1] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    userId[0] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    serialId[3] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    serialId[2] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    serialId[1] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    serialId[0] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x01;

    int numPorts = 4;
    for (int i = 4; i < 8; i++) {
        if (spiPortPresent[i]) {
            numPorts = 8;
        }
    }

    // cout << "expanderBoardDetected: " << expanderBoardDetected << endl;
    // cout << "expanderBoardId: " << expanderBoardIdNumber << endl;
    // cout << "spiPortPresent: " << spiPortPresent[7] << spiPortPresent[6] << spiPortPresent[5] << spiPortPresent[4] << spiPortPresent[3] << spiPortPresent[2] << spiPortPresent[1] << spiPortPresent[0] << endl;
    // cout << "serialId: " << serialId[3] << serialId[2] << serialId[1] << serialId[0] << endl;
    // cout << "userId: " << userId[2] << userId[1] << userId[0] << endl;
    // cout << "digOutVoltageLevel: " << digOutVoltageLevel << endl;

    return numPorts;
}

void Rhd2000EvalBoardUsb3::readDigitalInExpManual()
{
    lock_guard<mutex> lockOk(okMutex);
    int ttlIn[16];

    dev->SetWireInValue(WireInSerialDigitalInCntl, 2);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);  // Load digital in shift registers on falling edge of serial_LOAD
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[15] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[14] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[13] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[12] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[11] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[10] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[9] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[8] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[7] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[6] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[5] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[4] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[3] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[2] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[1] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    dev->SetWireInValue(WireInSerialDigitalInCntl, 1);
    dev->UpdateWireIns();
    dev->SetWireInValue(WireInSerialDigitalInCntl, 0);
    dev->UpdateWireIns();

    dev->UpdateWireOuts();
    ttlIn[0] = dev->GetWireOutValue(WireOutSerialDigitalIn) & 0x02;

    // for (int i = 0; i < 16; i++) {
    //     cout << "TTL IN " << i + 1 << " = " << ttlIn[i]/2 << endl;
    // }
}

// Selects an amplifier channel from a particular data stream to be subtracted from all DAC signals.
void Rhd2000EvalBoardUsb3::setDacRerefSource(int stream, int channel)
{
    lock_guard<mutex> lockOk(okMutex);

    if (stream < 0 || stream > (MAX_NUM_DATA_STREAMS - 1)) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacRerefSource: stream out of range." << endl;
        return;
    }

    if (channel < 0 || channel > 31) {
        cerr << "Error in Rhd2000EvalBoardUsb3::setDacRerefSource: channel out of range." << endl;
        return;
    }

    dev->SetWireInValue(WireInDacReref, (stream << 5) + channel, 0x0000003ff);
    dev->UpdateWireIns();
}

// Enables DAC rereferencing, where a selected amplifier channel is subtracted from all DACs in real time.
void Rhd2000EvalBoardUsb3::enableDacReref(bool enabled)
{
    lock_guard<mutex> lockOk(okMutex);

    dev->SetWireInValue(WireInDacReref, (enabled ? 0x00000400 : 0x00000000), 0x00000400);
    dev->UpdateWireIns();
}

bool Rhd2000EvalBoardUsb3::getStreamEnabled(int stream) const
{
	return (dataStreamEnabled[stream] == 0 ? false : true);
}