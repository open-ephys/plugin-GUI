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

#include "FPGAThread.h"
#include "../SourceNode/SourceNode.h"

#include <string.h>
#include <math.h>
#if JUCE_WIN32
#define M_PI       3.14159265358979323846
#endif

FPGAThread::FPGAThread (SourceNode* sn)
    : DataThread        (sn)
    , isTransmitting    (false)
    , deviceFound       (false)
    , bytesToRead       (20000)
    , ttlState          (0)
    , ttlOutputVal      (0)
    , bufferWasAligned  (false)
    , numchannels       (32)
{
    //const char* bitfilename = "./pipetest.bit";
#if JUCE_LINUX
    const char* bitfilename = "./pipetest.bit";
    const char* libname = "./libokFrontPanel.so";
#endif
#if JUCE_WIN32
    const char* bitfilename = "pipetest.bit";
    const char* libname = NULL;
#endif
#if JUCE_MAC
    const char* bitfilename = "/Users/Josh/Programming/open-ephys/GUI/Resources/DLLs/pipetest.bit";
    const char* libname = "/Users/Josh/Programming/open-ephys/GUI/Resources/DLLs/libokFrontPanel.dylib";
#endif


    if (! okFrontPanelDLL_LoadLib (libname))
    {
        printf ("FrontPanel DLL could not be loaded.\n");
    }

    okFrontPanelDLL_GetVersion (dll_date, dll_time);
    //printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);

    dev = new okCFrontPanel;

    strncpy (bitfile, bitfilename, 128);

    // Initialize the FPGA with our configuration bitfile.
    deviceFound = initializeFPGA (true);

    if (! deviceFound)
    {
        printf ("FPGA could not be initialized.\n");
    }
    else
    {
        printf ("FPGA interface initialized.\n");
    }

    Ndatabytes = numchannels * 3;

    dataBuffer = new DataBuffer (numchannels, 10000);

    eventCode = 0;

    //High-Pass filter
    const double fL=0.5;

    filter_A = exp (-2 * M_PI * fL / getSampleRate());
    filter_B = (double)1.0 - filter_A;
}


FPGAThread::~FPGAThread()
{
    std::cout << "FPGA interface destroyed." << std::endl;

     deleteAndZero(dev);
}


int FPGAThread::getNumChannels()        const { return numchannels; }
int FPGAThread::getNumEventChannels()   const { return 16; // 8 inputs, 8 outputs }

float FPGAThread::getSampleRate() const { return 28344.67;//12520.0; }

float FPGAThread::getBitVolts (int chan) const { return 0.1907; }


bool FPGAThread::foundInputSource()
{
    return true;

    // if (deviceFound)
    // {
    // 	if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile))
    // 	{
    // 		printf("FPGA configuration failed.\n");
    // 		deviceFound = false;
    // 		return false;
    // 	}

    // } else {

    // 	// if (!initializeFPGA(false))
    // 	// {
    // 	// 	return false;
    // 	// } else {
    // 	// 	deviceFound = true;
    // 	// }

    // }
}


bool FPGAThread::startAcquisition()
{
    //alignBuffer(200);
    //alignBuffer(200);
    //alignBuffer(200);

    // alignBuffer();

    // alignBuffer();

    bufferWasAligned = false;

    memset (filter_states, 0, 256 * sizeof (double));

    startThread();

    isTransmitting = true;
    accumulator = 0;

    return true;
}


bool FPGAThread::stopAcquisition()
{
    isTransmitting = false;

    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

    return true;
}


int FPGAThread::alignBuffer (int nBytes)
{
    long return_code;

    return_code = dev->ReadFromPipeOut (0xA0, nBytes, pBuffer);

    //std::cout << "Bytes read: " << return_code << std::endl;

    int j = 0;

    while (j < nBytes)
    {
        // look for timecode block (6 bytes)
        if ((pBuffer[j] & 1)
            && (pBuffer[j + 1] & 1)
            && (pBuffer[j + 2] & 1)
            && (pBuffer[j + 3] & 1)
            && (pBuffer[j + 4] & 1)
            && (pBuffer[j + 5] & 1))
            //&& (j+5+Ndatabytes <= bytesToRead)   ) // indicated by last bit being 1
        {
            int numNeeded = j;

            std::cout << j << " ";

            return_code = dev->ReadFromPipeOut (0xA0, numNeeded, pBuffer);
            //std::cout << "First sample is " << j << std::endl;
            //std::cout << "Samples needed:  " << numNeeded << std::endl;
            break;
        }

        j++;
    }

    return j;
}


bool FPGAThread::updateBuffer()
{
    long return_code;
    double currentSample;

    if (! bufferWasAligned)
    {
        alignBuffer (100000);
        alignBuffer (2000);
        //return_code = dev->ReadFromPipeOut(0xA0, 206, pBuffer);
        //alignBuffer(2000);
        //alignBuffer(200);
        bufferWasAligned = true;
    }

    return_code = dev->ReadFromPipeOut (0xA0, bytesToRead, pBuffer);

    //std::cout << return_code << std::endl; should return number of bytes read [sizeof(pBuffer)]

    if (return_code == 0)
        return false;

    int j = 0;

    // coding scheme:
    // the code works on a per-byte level where each byte ends in a 0 for data bytes
    // or in 1 for timecode bytes. This is some overhead but makes data integrity checks
    // pretty trivial.
    //
    // headstages are A,B,C,D and another one for the breakout box T for the 0-5v TTL input
    // A1 is stage A channel 1 etc
    // ...............
    // tc     ttttttt1
    // tc     ttttttt1    (6*7bit timecode gives 42 bit gives 4.3980e+12 samples max
    // tc     ttttttt1     which should be about 7 years at 30KHz)
    // tc     ttttttt1
    // tc     ttttttt1
    // tc     ttttttt1
    // ttl_in xxxxxxxx
    // ttl_o  xxxxxxxx
    // A1  Hi xxxxxxx0
    // A1  Lo xxxxxxx0    (last bits 0)
    // A1  ch cccccYY0
    // B1  Hi xxxxxxx0    (YY are the two missing bits from A1, cccc is a 5bit ch code 1-32, or maybe checksum?)
    // B1  Lo xxxxxxx0
    // B1  ch cccccYY0
    // A2  Hi ........
    // ... remaining channel data ...
    // B32 ch cccccYY0
    //
    // ... next sample ...
    //


    int i = 0;
    // int samplesUsed = 0;
    // int startSample = 0;

    // new strategy: read in 201 bytes & find the first sample

    while (j < bytesToRead)
    {
        // look for timecode block (6 bytes)
        if ((pBuffer[j] & 1)
            && (pBuffer[j + 1] & 1)
            && (pBuffer[j + 2] & 1)
            && (pBuffer[j + 3] & 1)
            && (pBuffer[j + 4] & 1)
            && (pBuffer[j + 5] & 1)
            && (j + 5 + Ndatabytes <= bytesToRead))    // indicated by last bit being 1
        {
            //read 6 bytes, assemble to 6*7 = 42 bits,  arranged in 6 bytes
            //std::cout << j << std::endl;

            i++;

            if (j % 200 != 0)
            {
                std::cout << "Buffer not aligned " << j << " " << accumulator << std::endl;
                //return false;
            }

            if (i == 1)
            {
                // firstSample = j;
            }

            unsigned char timecode[6];
            timecode[0] = (pBuffer[j] >> 1) | ((pBuffer[j+1] >> 1) << 7); // 1st byte throw out last bit of each byte and just concatenate the other bytes in ascending order
            timecode[1] = (pBuffer[j + 1] >> 2) | ((pBuffer[j + 2] >> 1) << 6); // 2nd byte
            timecode[2] = (pBuffer[j + 2] >> 3) | ((pBuffer[j + 3] >> 1) << 5); // 3rd byte
            timecode[3] = (pBuffer[j + 3] >> 4) | ((pBuffer[j + 4] >> 1) << 4); // 4th byte
            timecode[4] = (pBuffer[j + 4] >> 5) | ((pBuffer[j + 5] >> 1) << 3); // 5th byte
            timecode[5] = (pBuffer[j + 5] >> 6);                              // 6th byte

            timestamp = (uint64 (timecode[5]) << 40)
                            + (uint64 (timecode[4]) << 32)
                            + (uint64 (timecode[3]) << 24)
                            + (uint64 (timecode[2]) << 16)
                            + (uint64 (timecode[1]) << 8)
                            + (uint64 (timecode[0]));


            eventCode = pBuffer[j + 6]; // TTL input
            ttl_out = pBuffer[j + 7];

            if (ttl_out > 0)
            {
                eventCode |= 0x100;   // TTL output
                //std::cout << "TLL out!" << std::endl;
            }


            j += 8; //move cursor to 1st data byte

            // loop through sample data and condense from 3 bytes to 2 bytes
            uint16 hi;
            uint16 lo;

            // only take data from the first headstage (i.e., skip every other channel)
            for (int n = 0;  n < numchannels * 2 ; ++n)
            {
                if (n % 2 == 0)
                {
                    // last bit of first 2 is zero, replace with bits 1 and 2 from 3rd byte
                    hi = (pBuffer[j])    | (((pBuffer[j + 2]  >> 2) & ~(1<<6)) & ~(1<<7)) ;
                    lo = (pBuffer[j + 1])  | (((pBuffer[j + 2]  >> 1) & ~(1<<1)) & ~(1<<7)) ;

                    uint16 samp = ((hi << 8) + lo);

                    //high-pass filter
                    currentSample = double (samp) * 0.1907f - 3000.0f; //- 6175.0f;
                    thisSample[n / 2]    = float (currentSample - filter_states[n / 2]);
                    filter_states[n / 2] = filter_B * currentSample + filter_A*filter_states[n / 2];
                }

                j += 3;
            }

            j -= 1; // step back in time

            dataBuffer->addToBuffer (thisSample, &timestamp, &eventCode, 1);

            // samplesUsed += 200;
        }

        j++; // keep scanning for timecodes
    }

    // if (startSample != 0 && bytesToRead > 10000)
    //    bytesToRead -= 2;
    //else
    //   bytesToRead = 20000;


    // - startSample - 199;// + (200-startSample) - 1;// + startSample +1;

    //overflowSize = sizeof(pBuffer) - samplesUsed;

    //    if (overflowSize != 0)
    //    {
    //        memcpy(&overflowBuffer, &pBuffer[j-overflowSize], overflowSize);
    //
    //    }

    //  std::cout << "Overflow size: " << overflowSize << std::endl;

    // std::cout << "End time: " << timestamp << std::endl;



    // std::cout << "TTL out:" << ttl_out << std::endl;

    //accumulator++;

    checkTTLState();

    //    if (accumulator == 50)
    //    {
    //        //dev->SetWireInValue(0x01, 0x00); //, 0x06);
    //        ttlOutputVal = 0;
    //        //accumulator = 0;
    //        //dev->UpdateWireIns();
    //     //   std::cout << return_code << " " << i << std::endl; // number of samples found
    //       // std::cout << "Start sample: " << firstSample << std::endl;
    //    } else if (accumulator > 100) {
    //        //dev->SetWireInValue(0x01, 0xFF);//, 0x06);
    //        //ttlOutputVal = 1;
    //        accumulator = 0;
    //        //dev->UpdateWireIns();
    //    }


    return true;
}


void FPGAThread::checkTTLState()
{
    if (sn->getTTLState() != ttlState)
    {
        ttlState = sn->getTTLState();

        if (ttlState == 1)
        {
            dev->SetWireInValue (0x01, 0xFF);
        }
        else
        {
            dev->SetWireInValue (0x01, 0x00);
        }

        dev->UpdateWireIns();
    }
}


void FPGAThread::setOutputHigh()
{
    dev->SetWireInValue (0x01, 0x01); //, 0x06);

    dev->UpdateWireIns();
}


void FPGAThread::setOutputLow()
{
    dev->SetWireInValue (0x01, 0x00); //, 0x06);

    dev->UpdateWireIns();
}


bool FPGAThread::initializeFPGA (bool verbose)
{
    std::cout << "okCFrontPanel found " << dev->GetDeviceCount() << " devices." << std::endl;

    if (okCFrontPanel::NoError != dev->OpenBySerial())
    {
        if (verbose)
            printf ("Device could not be opened.  Is one connected?\n");

        return false;
    }

    if (verbose)
        printf ("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());

    dev->LoadDefaultPLLConfiguration();

    // Get some general information about the XEM.
    if (verbose)
    {
        std::string str;
        printf ("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
        str = dev->GetSerialNumber();
        printf ("Device serial number: %s\n", str.c_str());
        str = dev->GetDeviceID();
        printf ("Device device ID: %s\n", str.c_str());
    }
    // Download the configuration file.
    if (okCFrontPanel::NoError != dev->ConfigureFPGA (bitfile))
    {
        if (verbose)
            printf ("FPGA configuration failed.\n");
        j
        return false;
    }
    else
    {
        printf ("Bitfile uploaded.\n");
    }

    // Check for FrontPanel support in the FPGA configuration.
    if (verbose)
    {
        if (dev->IsFrontPanelEnabled())
            printf ("FrontPanel support is enabled.\n");
        else
            printf ("FrontPanel support is not enabled.\n");
    }

    dev->SetWireInValue (0x01, 0);
    dev->UpdateWireIns();

    return true;

    // this is not executed (after returning true)
    dev->SetWireInValue (0x00, 1<<2);  // set reset bit in cmd wire to 1 and back to 0
    dev->UpdateWireIns();
    dev->SetWireInValue (0x00, 0<<2);
    dev->UpdateWireIns();
}

