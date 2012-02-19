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

#include "IntanThread.h"

IntanThread::IntanThread(SourceNode* sn) : DataThread(sn),
			vendorID(0x0403),
			productID(0x6010),
			baudrate(115200),
			startCode(83),
			stopCode(115),
			ch(-1),
			isTransmitting(false)

{

	 dataBuffer = new DataBuffer(16,4096);

     deviceFound = initializeUSB(true);

}

IntanThread::~IntanThread() 
{
	//closeUSB();
	deleteAndZero(dataBuffer);
}

int IntanThread::getNumChannels()
{
    return 16;
}

float IntanThread::getSampleRate()
{
    return 25000.0;
}

bool IntanThread::foundInputSource()
{
    if (deviceFound)
    {
        int return_value;
        // try to update the baud rate
        if ((return_value = ftdi_set_baudrate(&ftdic, baudrate)) < 0)
        {
            deviceFound = false;
            return false;
        }
    } else {
        // try to initialize USB
        if (!initializeUSB(false))
        {
            return false;
        } else {
            deviceFound = true;
        }
    }

    return true;

}

bool IntanThread::startAcquisition()
{
    closeUSB();
    initializeUSB(false);
    ftdi_write_data(&ftdic, &startCode, 1);
    startThread();

    return true;
}

bool IntanThread::stopAcquisition()
{
    std::cout << "Received signal to terminate thread." << std::endl;
    
    if (isThreadRunning()) {
        signalThreadShouldExit();
    }

    std::cout << "Thread stopped successfully, stopping Intan Board." << std::endl;

    int return_value;

    if ((return_value = ftdi_write_data(&ftdic, &stopCode, 1)) > 0) {
        unsigned char buf[4097]; // has to be bigger than the on-chip buffer
        ftdi_read_data(&ftdic, buf, sizeof(buf));
        closeUSB();
    } else {
        deviceFound = false;
    }

    return true;
}


bool IntanThread::initializeUSB(bool verbose)
{
	int return_value;

	 // Step 1: initialise the ftdi_context:
	if (ftdi_init(&ftdic) < 0) {// -1 = couldn't allocate read buffer
                               // -2 = couldn't allocate struct buffer
        if (verbose)
            fprintf(stderr, "ftdi_init failed\n");
        return false;
    } else {
        if (verbose)
            std::cout << "FTDI context initialized." << std::endl;
    }

	// Step 2: open USB device
    // -3 = device not found
    // -8 = wrong permissions
   	if ((return_value = ftdi_usb_open(&ftdic, vendorID, productID)) < 0)
    {
        if (verbose)
            fprintf(stderr, "unable to open FTDI device: %d (%s)\n",
                        return_value, 
                        ftdi_get_error_string(&ftdic));
        return false;
    } else {
        std::cout << "USB connection opened." << std::endl;
    }

	// Step 3: set the baud rate
	if ((return_value = ftdi_set_baudrate(&ftdic, baudrate)) < 0)
    {
        if (verbose)
            fprintf(stderr, "unable to set baud rate: %d (%s)\n",
                        return_value, 
                        ftdi_get_error_string(&ftdic));
        return false;
    } else {
        std::cout << "Baud rate set to 115200" << std::endl;
    }

	return true;

}

bool IntanThread::closeUSB()
{
    ftdi_usb_close(&ftdic);
    ftdi_deinit(&ftdic);
    std::cout << "FTDI interface destroyed." << std::endl;
}


bool IntanThread::updateBuffer()
{

    int bytes_read;

    // Step 1: update buffer
    // error codes:
    //  -666: USB device unavailable
    //  <0  : error code from libusb_bulk_transfer()
    //  0   : no data available
    //  >0  : number of bytes read
    if ((bytes_read = ftdi_read_data(&ftdic, buffer, sizeof(buffer))) < 0)
    {
        std::cout << "NO DATA FOUND!" << std::endl;
        return false;
    }

	// Step 2: sort data
	int TTLval, channelVal;

    for (int index = 0; index < sizeof(buffer); index += 3) { 
           
          ++ch;
           
         for (int n = 0; n < 1; n++) { // 

         thisSample[ch%16+n*16] = float((buffer[index] & 127) + 
                     ((buffer[index+1] & 127) << 7) + 
                     ((buffer[index+2] & 3) << 14) - 32768)/32768;

         }
  
         TTLval = (buffer[index+2] & 4) >> 2; // extract TTL value (bit 3)
         channelVal = buffer[index+2] & 60;   // extract channel value

         if (channelVal == 60) {
         	dataBuffer->addToBuffer(thisSample,1);
         	ch = -1;
         }

    }

    return true;

}

