/*
  ==============================================================================

    FPGAThread.cpp
    Created: 9 Jun 2011 2:08:11pm
    Author:  jsiegle

  ==============================================================================
*/

#include "FPGAThread.h"

FPGAThread::FPGAThread() : DataThread(),
			isRunning(false),
			numchannels(32),
			m_u32SegmentSize(1048576)

{

	// Initialize the FPGA with our configuration bitfile.
	printf("New device created.\n");
	const char* bitfilename = "./pipetest.bit";

	printf("---- Opal Kelly ---- PipeTest Application v1.0 ----\n");
	
	if (FALSE == okFrontPanelDLL_LoadLib(NULL)) {
		printf("FrontPanel DLL could not be loaded.\n");
	}
	
	okFrontPanelDLL_GetVersion(dll_date, dll_time);
	printf("FrontPanel DLL loaded.  Built: %s  %s\n", dll_date, dll_time);

	dev = new okCFrontPanel;

	strncpy(bitfile, bitfilename, 128);

	if (!initializeFPGA(dev, bitfile)) {
		printf("FPGA could not be initialized.\n");
	}

	Ndatabytes = numchannels*3;
	
	std::cout << "FPGA interface initialized." << std::endl;

	dataBuffer = new DataBuffer(32, 10000);

	startThread();

}


FPGAThread::~FPGAThread() {
	
	stopThread(500);

	std::cout << "FPGA interface destroyed." << std::endl;

	// probably not the best way to do this:
	delete dataBuffer;
	delete dev;
	dev = 0;
	dataBuffer = 0;

}

void FPGAThread::updateBuffer() {
	
	dev->ReadFromPipeOut(0xA0, sizeof(pBuffer), pBuffer);

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
					thisSample[n] = float(float(hi)*256 + lo - 32768)/32768; 

				}
				dataBuffer->addToBuffer(thisSample,1);
			}
		j++; // keep scanning for timecodes
	}
}




bool FPGAThread::initializeFPGA(okCFrontPanel *dev, char *bitfile)
{
	if (okCFrontPanel::NoError != dev->OpenBySerial()) {
		delete dev;
		printf("Device could not be opened.  Is one connected?\n");
		return(NULL);
	}
	
	printf("Found a device: %s\n", dev->GetBoardModelString(dev->GetBoardModel()).c_str());

	dev->LoadDefaultPLLConfiguration();	

	// Get some general information about the XEM.
	std::string str;
	printf("Device firmware version: %d.%d\n", dev->GetDeviceMajorVersion(), dev->GetDeviceMinorVersion());
	str = dev->GetSerialNumber();
	printf("Device serial number: %s\n", str.c_str());
	str = dev->GetDeviceID();
	printf("Device device ID: %s\n", str.c_str());

	// Download the configuration file.
	if (okCFrontPanel::NoError != dev->ConfigureFPGA(bitfile)) {
		printf("FPGA configuration failed.\n");
		return(false);
	}

	// Check for FrontPanel support in the FPGA configuration.
	if (dev->IsFrontPanelEnabled())
		printf("FrontPanel support is enabled.\n");
	else
		printf("FrontPanel support is not enabled.\n");

	return(true);

	dev->SetWireInValue(0x00, 1<<2);  // set reset bit in cmd wire to 1 and back to 0
	dev->UpdateWireIns();
	dev->SetWireInValue(0x00, 0<<2);  
	dev->UpdateWireIns();

}

