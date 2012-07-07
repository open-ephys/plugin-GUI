/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

FPGAThread::FPGAThread(SourceNode* sn) : DataThread(sn),
			isTransmitting(false),
			numchannels(32),
			deviceFound(false)

{

	
	const char* bitfilename = "./pipetest.bit";

	printf("---- Opal Kelly ---- PipeTest Application v1.0 ----\n");
	
	if (!okFrontPanelDLL_LoadLib(NULL)) {
		printf("FrontPanel DLL could not be loaded.\n");
	}
	
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);

	dev = new okCFrontPanel;

	strncpy(bitfile, bitfilename, 128);

	// Initialize the FPGA with our configuration bitfile.
	deviceFound = initializeFPGA(true);

	if (!deviceFound) {
		printf("FPGA could not be initialized.\n");
	} else {
		printf("FPGA interface initialized.\n");
	}

	Ndatabytes = numchannels*3;
	
	dataBuffer = new DataBuffer(32, 10000);

	eventCode = 0;

}


FPGAThread::~FPGAThread() {
	
	std::cout << "FPGA interface destroyed." << std::endl;

	deleteAndZero(dev);

}

int FPGAThread::getNumChannels()
{
	return 32;
}

float FPGAThread::getSampleRate()
{
	return 25000.0;
}

float FPGAThread::getBitVolts()
{
	return 0.1907;
}

bool FPGAThread::foundInputSource()
{

	if (deviceFound)
	{
		if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile)) 
		{
			printf("FPGA configuration failed.\n");
			deviceFound = false;
			return false;
		}

	} else {

		// if (!initializeFPGA(false))
		// {
		// 	return false;
		// } else {
		// 	deviceFound = true;
		// }

	}

}

bool FPGAThread::startAcquisition()
{
   startThread();

   isTransmitting = true;

   return true;
}

bool FPGAThread::stopAcquisition()
{

	isTransmitting = false;

	if (isThreadRunning()) {

        signalThreadShouldExit();
    }


    return true;
}

bool FPGAThread::updateBuffer() {

	long return_code;
	
	return_code = dev->ReadFromPipeOut(0xA0, sizeof(pBuffer), pBuffer);

	if (return_code == 0)
		return false;

    int j = 0;

	while (j < sizeof(pBuffer))
	{
		// look for timecode block (6 bytes)
		if (  (pBuffer[j] & 1) && (pBuffer[j+1] & 1) && (pBuffer[j+2] & 1) && (pBuffer[j+3] & 1) && (pBuffer[j+4] & 1) && (pBuffer[j+5] & 1) &&  (j+5+Ndatabytes <= sizeof(pBuffer))   ) // indicated by last bit being 1
		{ //read 6 bytes, assemble to 6*7 = 42 bits,  arranged in 6 bytes
				char timecode[6]; // 1st byte throw out last bit of each byte and just concatenate the other bytes in ascending order 
				timecode[0] = (pBuffer[j] >> 1) | ((pBuffer[j+1] >> 1) << 7); // 2nd byte
				timecode[1] = (pBuffer[j+1] >> 2) | ((pBuffer[j+2] >> 1) << 6); // 3rd byte
				timecode[2] = (pBuffer[j+2] >> 3) | ((pBuffer[j+3] >> 1) << 5); // 4th byte
				timecode[3] = (pBuffer[j+3] >> 4) | ((pBuffer[j+4] >> 1) << 4); // 5th byte
				timecode[4] = (pBuffer[j+4] >> 5) | ((pBuffer[j+5] >> 1) << 3); // 6th byte
				timecode[5] = (pBuffer[j+5] >> 6);
			
				j += 6; //move cursor to 1st data byte

				// loop through sample data and condense from 3 bytes to 2 bytes
				char hi; char lo;
				for (int n = 0;  n < numchannels ; n++) 
				{
					// last bit of first 2 is zero, replace with bits 1 and 2 from 3rd byte
					hi = (pBuffer[j])    | (((  pBuffer[j+2]  >> 2) & ~(1<<6)) & ~(1<<7)) ;
					lo = (pBuffer[j+1])  | (((  pBuffer[j+2]  >> 1) & ~(1<<1)) & ~(1<<7)) ;
					j += 3;

					//thisSample[n] = float( hi  - 256)/256; 
					uint16 samp = ((hi << 8) + lo);
					thisSample[n] = (float(samp) - 32768.0f)/92768.0f; 

				}
				
				// should actually be converting timecode to timestamp: 
				timestamp = timer.getHighResolutionTicks();

				dataBuffer->addToBuffer(thisSample, &timestamp, &eventCode, 1);
			}
		j++; // keep scanning for timecodes
	}


	return true;
}




bool FPGAThread::initializeFPGA(bool verbose)
{

	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		if (verbose)
			printf("Device could not be opened.  Is one connected?\n");
		return false;
	}
	
	if (verbose)
		printf("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());

	dev->LoadDefaultPLLConfiguration();	

	// Get some general information about the XEM.
	if (verbose) {
		std::string str;
		printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
		str = dev->GetSerialNumber();
		printf("Device serial number: %s\n", str.c_str());
		str = dev->GetDeviceID();
		printf("Device device ID: %s\n", str.c_str());
	}

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile)) {
		if (verbose)
			printf("FPGA configuration failed.\n");
		return false;
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (verbose) {
		if (dev->IsFrontPanelEnabled())
			printf("FrontPanel support is enabled.\n");
		else
			printf("FrontPanel support is not enabled.\n");
	}

	return true;

	// this is not executed
	dev->SetWireInValue(0x00, 1<<2);  // set reset bit in cmd wire to 1 and back to 0
	dev->UpdateWireIns();
	dev->SetWireInValue(0x00, 0<<2);  
	dev->UpdateWireIns();

}

