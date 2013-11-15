/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

#include "RHD2000Thread.h"
#include "../SourceNode.h"

#if defined(_WIN32)
#define okLIB_NAME "okFrontPanel.dll"
#define okLIB_EXTENSION "*.dll"
#elif defined(__APPLE__)
#define okLIB_NAME "libokFrontPanel.dylib"
#define okLIB_EXTENSION "*.dylib"
#elif defined(__linux__)
#define okLIB_NAME "./libokFrontPanel.so"
#define okLIB_EXTENSION "*.so"
#endif

RHD2000Thread::RHD2000Thread(SourceNode* sn) : DataThread(sn),
    chipRegisters(30000.0f),
    numChannels(0),
    deviceFound(false),
    isTransmitting(false),
    dacOutputShouldChange(false),
    acquireAdcChannels(false),
    acquireAuxChannels(true),
    fastSettleEnabled(false),
    dspEnabled(true),
    desiredDspCutoffFreq(0.5f),
    desiredUpperBandwidth(7500.0f),
    desiredLowerBandwidth(1.0f),
    boardSampleRate(30000.0f),
    savedSampleRateIndex(16),
    cableLengthPortA(0.914f), cableLengthPortB(0.914f), cableLengthPortC(0.914f), cableLengthPortD(0.914f), // default is 3 feet (0.914 m),
    audioOutputL(-1), audioOutputR(-1) 
{
    evalBoard = new Rhd2000EvalBoard;
    dataBlock = new Rhd2000DataBlock(1);
    dataBuffer = new DataBuffer(2, 10000); // start with 2 channels and automatically resize

    // Open Opal Kelly XEM6010 board.
	// Returns 1 if successful, -1 if FrontPanel cannot be loaded, and -2 if XEM6010 can't be found.
    File executable = File::getSpecialLocation(File::currentExecutableFile);

    #if defined(__APPLE__)
        const String executableDirectory =
            executable.getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getFullPathName();
    #else
	   const String executableDirectory = executable.getParentDirectory().getFullPathName();
    

    #endif
    
    std::cout << executableDirectory << std::endl;
    

	String dirName = executableDirectory;
    libraryFilePath = dirName;
	libraryFilePath += File::separatorString;
	libraryFilePath += okLIB_NAME;
    
    if (openBoard(libraryFilePath))
    {

        // upload bitfile and restore default settings
        initializeBoard();

        // automatically find connected headstages
        scanPorts(); // things would appear to run more smoothly if this were done after the editor has been created
    
        if (0)
        {
            evalBoard->setContinuousRunMode(true);
            evalBoard->run();
        }

    }

}

RHD2000Thread::~RHD2000Thread()
{

    std::cout << "RHD2000 interface destroyed." << std::endl;

    if (deviceFound)
    {
        int ledArray[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        evalBoard->setLedDisplay(ledArray);
    }

	if (deviceFound)
		evalBoard->resetFpga();

    deleteAndZero(dataBlock);

}

bool RHD2000Thread::openBoard(String pathToLibrary)
{
    int return_code = evalBoard->open(pathToLibrary.getCharPointer());

    if (return_code == 1)
    {
        deviceFound = true;
    }
    else if (return_code == -1) // dynamic library not found
    {
        bool response = AlertWindow::showOkCancelBox (AlertWindow::NoIcon,
                                   "Opal Kelly library not found.",
                                    "The Opal Kelly library file was not found in the directory of the executable. Would you like to browse for it?",
                                     "Yes", "No", 0, 0);
        if (response)
        {
            // browse for file
            FileChooser fc("Select the library file...",
                               File::getCurrentWorkingDirectory(),
                               okLIB_EXTENSION,
                               true);

            if (fc.browseForFileToOpen())
            {
                File currentFile = fc.getResult();
                libraryFilePath = currentFile.getFullPathName();
                openBoard(libraryFilePath); // call recursively
            }
            else
            {
                //sendActionMessage("No configuration selected.");
                deviceFound = false;
            }

        } else {
            deviceFound = false;
        }
    } else if (return_code == -2) // board could not be opened
    {
        bool response = AlertWindow::showOkCancelBox (AlertWindow::NoIcon,
                                   "Acquisition board not found.",
                                    "An acquisition board could not be found. Please connect one now.",
                                     "OK", "Cancel", 0, 0);
    
        if (response)
        {
            openBoard(libraryFilePath.getCharPointer()); // call recursively
        } else {
            deviceFound = false;
        }

    }

    return deviceFound;

}

bool RHD2000Thread::uploadBitfile(String bitfilename)
{
    
    deviceFound = true;
    
    if (!evalBoard->uploadFpgaBitfile(bitfilename.toStdString()))
    {
        std::cout << "Couldn't upload bitfile from " << bitfilename << std::endl;
        
        bool response = AlertWindow::showOkCancelBox (AlertWindow::NoIcon,
                                   "FPGA bitfile not found.",
                                    "The rhd2000.bit file was not found in the directory of the executable. Would you like to browse for it?",
                                     "Yes", "No", 0, 0);
        if (response)
        {
            // browse for file
            FileChooser fc("Select the FPGA bitfile...",
                               File::getCurrentWorkingDirectory(),
                               "*.bit",
                               true);

            if (fc.browseForFileToOpen())
            {
                File currentFile = fc.getResult();
                uploadBitfile(currentFile.getFullPathName()); // call recursively
            }
            else
            {
                //sendActionMessage("No configuration selected.");
                deviceFound = false;
            }

        } else {
            deviceFound = false;
        }

    }
    
    return deviceFound;

}

void RHD2000Thread::initializeBoard()
{
    String bitfilename;

	File executable = File::getSpecialLocation(File::currentExecutableFile);

    #if defined(__APPLE__)
    const String executableDirectory = 
            executable.getParentDirectory().getParentDirectory().getParentDirectory().getParentDirectory().getFullPathName();
    #else
       const String executableDirectory = executable.getParentDirectory().getFullPathName();
    #endif

	bitfilename = executableDirectory;
	bitfilename += File::separatorString;
	bitfilename += "rhd2000.bit";

    if (!uploadBitfile(bitfilename))
    {
        return;
    }
    // Initialize the board
    std::cout << "Initializing acquisition board." << std::endl;
    evalBoard->initialize();
    // This applies the following settings:
    //  - sample rate to 30 kHz
    //  - aux command banks to zero
    //  - aux command lengths to zero
    //  - continuous run mode to 'true'
    //  - maxTimeStep to 2^32 - 1
    //  - all cable lengths to 3 feet
    //  - dspSettle to 'false'
    //  - data source mapping as 0->PortA1, 1->PortB1, 2->PortC1, 3->PortD1, etc.
    //  - enables all data streams
    //  - clears the ttlOut
    //  - disables all DACs and sets gain to 0

    // Select RAM Bank 0 for AuxCmd3 initially, so the ADC is calibrated.
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3, 0);

    // Since our longest command sequence is 60 commands, run the SPI interface for
    // 60 samples
    evalBoard->setMaxTimeStep(60);
    evalBoard->setContinuousRunMode(false);

    // Start SPI interface
    evalBoard->run();

    // Wait for the 60-sample run to complete
    while (evalBoard->isRunning())
    {
        ;
    }

    // Read the resulting single data block from the USB interface. We don't
    // need to do anything with this, since it was only used for ADC calibration
    Rhd2000DataBlock* dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());


    evalBoard->readDataBlock(dataBlock);

    // Now that ADC calibration has been performed, we switch to the command sequence
    // that does not execute ADC calibration.
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);


    updateRegisters();

    // Let's turn one LED on to indicate that the board is now connected
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

}

void RHD2000Thread::scanPorts()
{
	if (!deviceFound) //Safety to avoid crashes if board not present
	{
		return;
	}
    // Scan SPI ports

    int delay, stream, id;
    //int numChannelsOnPort[4] = {0, 0, 0, 0};
    Array<int> chipId;
    chipId.insertMultiple(0,-1,8);

    setSampleRate(16, true); // set to 30 kHz temporarily

    // Enable all data streams, and set sources to cover one or two chips
    // on Ports A-D.
    evalBoard->setDataSource(0, Rhd2000EvalBoard::PortA1);
    evalBoard->setDataSource(1, Rhd2000EvalBoard::PortA2);
    evalBoard->setDataSource(2, Rhd2000EvalBoard::PortB1);
    evalBoard->setDataSource(3, Rhd2000EvalBoard::PortB2);
    evalBoard->setDataSource(4, Rhd2000EvalBoard::PortC1);
    evalBoard->setDataSource(5, Rhd2000EvalBoard::PortC2);
    evalBoard->setDataSource(6, Rhd2000EvalBoard::PortD1);
    evalBoard->setDataSource(7, Rhd2000EvalBoard::PortD2);

    evalBoard->enableDataStream(0, true);
    evalBoard->enableDataStream(1, true);
    evalBoard->enableDataStream(2, true);
    evalBoard->enableDataStream(3, true);
    evalBoard->enableDataStream(4, true);
    evalBoard->enableDataStream(5, true);
    evalBoard->enableDataStream(6, true);
    evalBoard->enableDataStream(7, true);

    std::cout << "Number of enabled data streams: " << evalBoard->getNumEnabledDataStreams() << std::endl;


    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA,
                                    Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB,
                                    Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC,
                                    Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD,
                                    Rhd2000EvalBoard::AuxCmd3, 0);

    // Since our longest command sequence is 60 commands, we run the SPI
    // interface for 60 samples.
    evalBoard->setMaxTimeStep(60);
    evalBoard->setContinuousRunMode(false);

    Rhd2000DataBlock* dataBlock =
        new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());

    Array<int> sumGoodDelays;
    sumGoodDelays.insertMultiple(0,0,8);

    Array<int> indexFirstGoodDelay;
    indexFirstGoodDelay.insertMultiple(0,-1,8);

    Array<int> indexSecondGoodDelay;
    indexSecondGoodDelay.insertMultiple(0,-1,8);


    // Run SPI command sequence at all 16 possible FPGA MISO delay settings
    // to find optimum delay for each SPI interface cable.

    std::cout << "Checking for connected amplifier chips..." << std::endl;

    for (delay = 0; delay < 16; delay++)//(delay = 0; delay < 16; ++delay)
    {
        evalBoard->setCableDelay(Rhd2000EvalBoard::PortA, delay);
        evalBoard->setCableDelay(Rhd2000EvalBoard::PortB, delay);
        evalBoard->setCableDelay(Rhd2000EvalBoard::PortC, delay);
        evalBoard->setCableDelay(Rhd2000EvalBoard::PortD, delay);

        // Start SPI interface.
        evalBoard->run();

        // Wait for the 60-sample run to complete.
        while (evalBoard->isRunning())
        {
            ;
        }

        // Read the resulting single data block from the USB interface.
        evalBoard->readDataBlock(dataBlock);

        // Read the Intan chip ID number from each RHD2000 chip found.
        // Record delay settings that yield good communication with the chip.
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)//MAX_NUM_DATA_STREAMS; ++stream)
        {
            // std::cout << "Stream number " << stream << ", delay = " << delay << std::endl;

            id = deviceId(dataBlock, stream);

            if (id > 0) // 1 = RHD2132, 2 = RHD2216
            {
                //  std::cout << "Device ID found: " << id << std::endl;

                sumGoodDelays.set(stream,sumGoodDelays[stream] + 1);

                if (indexFirstGoodDelay[stream] == -1)
                {
                    indexFirstGoodDelay.set(stream, delay);
                    chipId.set(stream,id);
                }
                else if (indexSecondGoodDelay[stream] == -1)
                {
                    indexSecondGoodDelay.set(stream,delay);
                    chipId.set(stream,id);
                }
            }
        }
    }

    // std::cout << "Chip IDs found: ";
    // for (int i = 0; i < MAX_NUM_DATA_STREAMS; ++i)
    // {
    //     std::cout << chipId[i] << " ";
    // }
    //std::cout << std::endl;

    // Now, disable data streams where we did not find chips present.
    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)
    {
        if (chipId[stream] > 0)
        {
            //std::cout << "Enabling headstage on stream " << stream << std::endl;
            enableHeadstage(stream, true);
        }
        else
        {
            enableHeadstage(stream, false);
        }
    }

    std::cout << "Number of enabled data streams: " << evalBoard->getNumEnabledDataStreams() << std::endl;


    // Set cable delay settings that yield good communication with each
    // RHD2000 chip.
    Array<int> optimumDelay;
    optimumDelay.insertMultiple(0,0,8);

    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)
    {
        if (sumGoodDelays[stream] == 1 || sumGoodDelays[stream] == 2)
        {
            optimumDelay.set(stream,indexFirstGoodDelay[stream]);
        }
        else if (sumGoodDelays[stream] > 2)
        {
            optimumDelay.set(stream,indexSecondGoodDelay[stream]);
        }
    }

    evalBoard->setCableDelay(Rhd2000EvalBoard::PortA,
                             optimumDelay[0]);
    evalBoard->setCableDelay(Rhd2000EvalBoard::PortB,
                             optimumDelay[1]);
    evalBoard->setCableDelay(Rhd2000EvalBoard::PortC,
                             optimumDelay[2]);
    evalBoard->setCableDelay(Rhd2000EvalBoard::PortD,
                             optimumDelay[3]);

    cableLengthPortA =
        evalBoard->estimateCableLengthMeters(optimumDelay[0]);
    cableLengthPortB =
        evalBoard->estimateCableLengthMeters(optimumDelay[1]);
    cableLengthPortC =
        evalBoard->estimateCableLengthMeters(optimumDelay[2]);
    cableLengthPortD =
        evalBoard->estimateCableLengthMeters(optimumDelay[3]);

    setSampleRate(savedSampleRateIndex); // restore saved sample rate

    updateRegisters();
}

int RHD2000Thread::deviceId(Rhd2000DataBlock* dataBlock, int stream)
{
    bool intanChipPresent;

    // First, check ROM registers 32-36 to verify that they hold 'INTAN'.
    // This is just used to verify that we are getting good data over the SPI
    // communication channel.
    // std::cout << dataBlock->auxiliaryData[stream][2][32] << " ";
    // std::cout << dataBlock->auxiliaryData[stream][2][33] << " ";
    // std::cout << dataBlock->auxiliaryData[stream][2][34] << " ";
    // std::cout << dataBlock->auxiliaryData[stream][2][35] << " ";
    // std::cout << dataBlock->auxiliaryData[stream][2][36] << std::endl;

    intanChipPresent = (dataBlock->auxiliaryData[stream][2][32] == 73 && // I = 73
                        dataBlock->auxiliaryData[stream][2][33] == 78 && // N = 78
                        dataBlock->auxiliaryData[stream][2][34] == 84 && // T = 84
                        dataBlock->auxiliaryData[stream][2][35] == 65 && // A = 65
                        dataBlock->auxiliaryData[stream][2][36] == 78);  // N = 78

    // If the SPI communication is bad, return -1.  Otherwise, return the Intan
    // chip ID number stored in ROM regstier 63.
    if (!intanChipPresent)
    {
        return -1;
    }
    else
    {
        return dataBlock->auxiliaryData[stream][2][19]; // chip ID (Register 63)
    }
}



bool RHD2000Thread::isAcquisitionActive()
{
    return isTransmitting;
}

void RHD2000Thread::setNumChannels(int hsNum, int numChannels)
{
    numChannelsPerDataStream.set(hsNum, numChannels);
}

int RHD2000Thread::getNumChannels()
{

    numChannels = 0;

    for (int i = 0; i < MAX_NUM_DATA_STREAMS; i++)
    {

        if (numChannelsPerDataStream[i] > 0)
        {
            numChannels += numChannelsPerDataStream[i];
            numChannels += 3; // to account for aux inputs
        }


        /*
        if (chipRegisters->adcAux1En){ // no public function to read these? fix this in some way
        	numChannels += 1;
        }
        if (chipRegisters->adcAux2En){
        	numChannels += 1;
        }
        if (chipRegisters->adcAux3En){
        	numChannels += 1;
        }
        */
    }


    if (acquireAdcChannels)
    {
        numChannels += 8; // add 8 channels for the ADCs
    }

    if (numChannels > 0)
        return numChannels;
    else
        return 1; // to prevent crashing with 0 channels
}

void RHD2000Thread::updateChannelNames()
{

    int chNum = -1;

    for (int i = 0; i < MAX_NUM_DATA_STREAMS; i++)
    {

        for (int j = 0; j < numChannelsPerDataStream[i]; j++)
        {
            chNum++;

            sn->channels[chNum]->setName(String(chNum));
        }
    }

    if (acquireAuxChannels)
    {
        for (int i = 0; i < MAX_NUM_DATA_STREAMS; i++)
        {

            for (int j = 0; j < 3; j++)
            {

                chNum++;

                String chName = "AUX";
                chName += (j+1);

                // this is causing a seg fault for some reason:
                //  sn->channels[chNum]->setName(chName);
            }
        }
    }


    if (acquireAdcChannels)
    {
        for (int j = 0; j < 8; j++)
        {
            chNum++;

            String chName = "ADC";
            chName += (j+1);

            //  sn->channels[chNum]->setName(chName);
        }
    }

}


int RHD2000Thread::getNumEventChannels()
{
    return 16; // 8 inputs, 8 outputs
}

float RHD2000Thread::getSampleRate()
{
    return evalBoard->getSampleRate();
}

float RHD2000Thread::getBitVolts()
{
    return 0.195f;
}

double RHD2000Thread::setUpperBandwidth(double upper)
{

    desiredUpperBandwidth = upper;

    updateRegisters();

    return actualUpperBandwidth;
}

double RHD2000Thread::setLowerBandwidth(double lower)
{
    desiredLowerBandwidth = lower;

    updateRegisters();

    return actualLowerBandwidth;
}

int RHD2000Thread::setNoiseSlicerLevel(int level)
{
    desiredNoiseSlicerLevel = level;
    evalBoard->setAudioNoiseSuppress(desiredNoiseSlicerLevel);

    // Level has been checked once before this and then is checked again in setAudioNoiseSuppress.
    // This may be overkill - maybe API should change so that the final function returns the value?
    actualNoiseSlicerLevel = level;
    
    return actualNoiseSlicerLevel;
}


bool RHD2000Thread::foundInputSource()
{

    return deviceFound;

}

bool RHD2000Thread::enableHeadstage(int hsNum, bool enabled)
{

    evalBoard->enableDataStream(hsNum, enabled);

    if (enabled)
    {
        numChannelsPerDataStream.set(hsNum, 32);
    }
    else
    {
        numChannelsPerDataStream.set(hsNum, 0);
    }

    std::cout << "Enabled channels: ";

    for (int i = 0; i < MAX_NUM_DATA_STREAMS; i++)
    {
        std::cout << numChannelsPerDataStream[i] << " ";
    }

    std:: cout << std::endl;


    dataBuffer->resize(getNumChannels(), 10000);

    return true;
}

bool RHD2000Thread::isHeadstageEnabled(int hsNum)
{

    if (numChannelsPerDataStream[hsNum] > 0)
    {
        return true;
    }

    return false;

}

void RHD2000Thread::assignAudioOut(int dacChannel, int dataChannel)
{

    if (dacChannel == 0)
    {
        audioOutputR = dataChannel;


    }
    else if (dacChannel == 1)
    {
        audioOutputL = dataChannel;

    }

    dacOutputShouldChange = true; // set a flag and take care of setting wires
    // during the updateBuffer() method
    // to avoid problems

}

void RHD2000Thread::enableAdcs(bool t)
{

    acquireAdcChannels = t;

    dataBuffer->resize(getNumChannels(), 10000);

}

void RHD2000Thread::setSampleRate(int sampleRateIndex, bool isTemporary)
{

    if (!isTemporary)
    {
        savedSampleRateIndex = sampleRateIndex;
    }

    int numUsbBlocksToRead = 0; // placeholder - make this change the number of blocks that are read in RHD2000Thread::updateBuffer()

    Rhd2000EvalBoard::AmplifierSampleRate sampleRate; // just for local use

    switch (sampleRateIndex)
    {
        case 0:
            sampleRate = Rhd2000EvalBoard::SampleRate1000Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1000.0f;
            break;
        case 1:
            sampleRate = Rhd2000EvalBoard::SampleRate1250Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1250.0f;
            break;
        case 2:
            sampleRate = Rhd2000EvalBoard::SampleRate1500Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1500.0f;
            break;
        case 3:
            sampleRate = Rhd2000EvalBoard::SampleRate2000Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 2000.0f;
            break;
        case 4:
            sampleRate = Rhd2000EvalBoard::SampleRate2500Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 2500.0f;
            break;
        case 5:
            sampleRate = Rhd2000EvalBoard::SampleRate3000Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 3000.0f;
            break;
        case 6:
            sampleRate = Rhd2000EvalBoard::SampleRate3333Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 3333.0f;
            break;
        case 7:
            sampleRate = Rhd2000EvalBoard::SampleRate4000Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 4000.0f;
            break;
        case 8:
            sampleRate = Rhd2000EvalBoard::SampleRate5000Hz;
            numUsbBlocksToRead = 3;
            boardSampleRate = 5000.0f;
            break;
        case 9:
            sampleRate = Rhd2000EvalBoard::SampleRate6250Hz;
            numUsbBlocksToRead = 3;
            boardSampleRate = 6250.0f;
            break;
        case 10:
            sampleRate = Rhd2000EvalBoard::SampleRate8000Hz;
            numUsbBlocksToRead = 4;
            boardSampleRate = 8000.0f;
            break;
        case 11:
            sampleRate = Rhd2000EvalBoard::SampleRate10000Hz;
            numUsbBlocksToRead = 6;
            boardSampleRate = 10000.0f;
            break;
        case 12:
            sampleRate = Rhd2000EvalBoard::SampleRate12500Hz;
            numUsbBlocksToRead = 7;
            boardSampleRate = 12500.0f;
            break;
        case 13:
            sampleRate = Rhd2000EvalBoard::SampleRate15000Hz;
            numUsbBlocksToRead = 8;
            boardSampleRate = 15000.0f;
            break;
        case 14:
            sampleRate = Rhd2000EvalBoard::SampleRate20000Hz;
            numUsbBlocksToRead = 12;
            boardSampleRate = 20000.0f;
            break;
        case 15:
            sampleRate = Rhd2000EvalBoard::SampleRate25000Hz;
            numUsbBlocksToRead = 14;
            boardSampleRate = 25000.0f;
            break;
        case 16:
            sampleRate = Rhd2000EvalBoard::SampleRate30000Hz;
            numUsbBlocksToRead = 16;
            boardSampleRate = 30000.0f;
            break;
        default:
            sampleRate = Rhd2000EvalBoard::SampleRate10000Hz;
            numUsbBlocksToRead = 6;
            boardSampleRate = 10000.0f;
    }


    // Select per-channel amplifier sampling rate.
    evalBoard->setSampleRate(sampleRate);

    std::cout << "Sample rate set to " << evalBoard->getSampleRate() << std::endl;

    // Now that we have set our sampling rate, we can set the MISO sampling delay
    // which is dependent on the sample rate.
    evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortA, cableLengthPortA);
    evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortB, cableLengthPortB);
    evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortC, cableLengthPortC);
    evalBoard->setCableLengthMeters(Rhd2000EvalBoard::PortD, cableLengthPortD);

}

void RHD2000Thread::updateRegisters()
{

	if (!deviceFound) //Safety to avoid crashes loading a chain with Rythm node withouth a board
	{
		return;
	}
    // Set up an RHD2000 register object using this sample rate to
    // optimize MUX-related register settings.
    chipRegisters.defineSampleRate(boardSampleRate);


    int commandSequenceLength;
    vector<int> commandList;

    // Create a command list for the AuxCmd1 slot.  This command sequence will create a 250 Hz,
    // zero-amplitude sine wave (i.e., a flatline).  We will change this when we want to perform
    // impedance testing.
    commandSequenceLength = chipRegisters.createCommandListZcheckDac(commandList, 250.0, 0.0);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd1, 0, commandSequenceLength - 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd1, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd1, 0);

    // // Next, we'll create a command list for the AuxCmd2 slot.  This command sequence
    // // will sample the temperature sensor and other auxiliary ADC inputs.
    commandSequenceLength = chipRegisters.createCommandListTempSensor(commandList);
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd2, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd2, 0, commandSequenceLength - 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd2, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd2, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd2, 0);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd2, 0);

    // Before generating register configuration command sequences, set amplifier
    // bandwidth paramters.
    actualDspCutoffFreq = chipRegisters.setDspCutoffFreq(desiredDspCutoffFreq);
    actualLowerBandwidth = chipRegisters.setLowerBandwidth(desiredLowerBandwidth);
    actualUpperBandwidth = chipRegisters.setUpperBandwidth(desiredUpperBandwidth);
    chipRegisters.enableDsp(dspEnabled);

    // turn on aux inputs
    chipRegisters.enableAux1(true);
    chipRegisters.enableAux2(true);
    chipRegisters.enableAux3(true);

    chipRegisters.createCommandListRegisterConfig(commandList, true);
    // Upload version with ADC calibration to AuxCmd3 RAM Bank 0.
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 0);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    commandSequenceLength = chipRegisters.createCommandListRegisterConfig(commandList, false);
    // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 1);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    chipRegisters.setFastSettle(true);
    commandSequenceLength = chipRegisters.createCommandListRegisterConfig(commandList, false);
    // Upload version with fast settle enabled to AuxCmd3 RAM Bank 2.
    evalBoard->uploadCommandList(commandList, Rhd2000EvalBoard::AuxCmd3, 2);
    evalBoard->selectAuxCommandLength(Rhd2000EvalBoard::AuxCmd3, 0,
                                      commandSequenceLength - 1);
    chipRegisters.setFastSettle(false);



    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
    evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
}

void RHD2000Thread::setCableLength(int hsNum, float length)
{
    // Set the MISO sampling delay, which is dependent on the sample rate.

    switch (hsNum)
    {
        case 0:
            evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortA, length);
            break;
        case 1:
            evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortB, length);
            break;
        case 2:
            evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortC, length);
            break;
        case 3:
            evalBoard->setCableLengthFeet(Rhd2000EvalBoard::PortD, length);
            break;
        default:
            break;
    }

}

bool RHD2000Thread::startAcquisition()
{

    dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());

    std::cout << "Expecting " << getNumChannels() << " channels." << std::endl;

    //memset(filter_states,0,256*sizeof(double));

    int ledArray[8] = {1, 1, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

    cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
    cout << "Is eval board running: " << evalBoard->isRunning() << endl;


    //std::cout << "Setting max timestep." << std::endl;
    //evalBoard->setMaxTimeStep(100);
    

    std::cout << "Starting acquisition." << std::endl;
    if (1)
    {
       // evalBoard->setContinuousRunMode(false);
      //  evalBoard->setMaxTimeStep(0);
        std::cout << "Flushing FIFO." << std::endl;
        evalBoard->flush();
        evalBoard->setContinuousRunMode(true);
        evalBoard->run();
    }

    blockSize = dataBlock->calculateDataBlockSizeInWords(evalBoard->getNumEnabledDataStreams());

    startThread();


    isTransmitting = true;

    return true;
}

bool RHD2000Thread::stopAcquisition()
{

    //	isTransmitting = false;
    std::cout << "RHD2000 data thread stopping acquisition." << std::endl;

    if (isThreadRunning())
    {
        signalThreadShouldExit();

    }

    if (waitForThreadToExit(500))
    {
        std::cout << "Thread exited." << std::endl;
    }
    else
    {
        std::cout << "Thread failed to exit, continuing anyway..." << std::endl;
    }

    if (1)
    {
        evalBoard->setContinuousRunMode(false);
        evalBoard->setMaxTimeStep(0);
        std::cout << "Flushing FIFO." << std::endl;
        evalBoard->flush();
     //   evalBoard->setContinuousRunMode(true);
     //   evalBoard->run();

    }

    dataBuffer->clear();

    cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;

   // std::cout << "Stopped eval board." << std::endl;


    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

    isTransmitting = false;

    return true;
}

bool RHD2000Thread::updateBuffer()
{

    //cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
    //cout << "Block size: " << blockSize << endl;

    bool return_code;

    if (evalBoard->numWordsInFifo() >= blockSize)
    {
        return_code = evalBoard->readDataBlock(dataBlock);

        for (int samp = 0; samp < dataBlock->getSamplesPerDataBlock(); samp++)
        {
            int streamNumber = -1;
            int channel = -1;

            // do the neural data channels first
            for (int dataStream = 0; dataStream < MAX_NUM_DATA_STREAMS; dataStream++)
            {
                if (numChannelsPerDataStream[dataStream] > 0)
                {
                    streamNumber++;

                    for (int chan = 0; chan < numChannelsPerDataStream[dataStream]; chan++)
                    {
                        
                      //  std::cout << "reading sample stream " << streamNumber << " chan " << chan << " sample "<< samp << std::endl;
                        
                        channel++;

                        int value = dataBlock->amplifierData[streamNumber][chan][samp];

                        thisSample[channel] = float(value-32768)*0.195f;
                    }

                }

            }

            streamNumber = -1;

            // then do the Intan ADC channels
            for (int dataStream = 0; dataStream < MAX_NUM_DATA_STREAMS; dataStream++)
            {
                if (numChannelsPerDataStream[dataStream] > 0)
                {
                    streamNumber++;

                    if (samp % 4 == 1)   // every 4th sample should have auxiliary input data
                    {

                       // std::cout << "reading sample stream " << streamNumber << " aux ADCs " << std::endl;

                        channel++;
                        thisSample[channel] = 0.0374 *
                                              float(dataBlock->auxiliaryData[streamNumber][1][samp+0] - 45000.0f) ;
                                              // constant offset keeps the values visible in the LFP Viewer

                        auxBuffer[channel] = thisSample[channel];

                        channel++;
                        thisSample[channel] = 0.0374 *
                                              float(dataBlock->auxiliaryData[streamNumber][1][samp+1] - 45000.0f) ;
                                              // constant offset keeps the values visible in the LFP Viewer

                        auxBuffer[channel] = thisSample[channel];


                        channel++;
                        thisSample[channel] = 0.0374 *
                                              float(dataBlock->auxiliaryData[streamNumber][1][samp+2] - 45000.0f) ;
                                              // constant offset keeps the values visible in the LFP Viewer

                        auxBuffer[channel] = thisSample[channel];

                    }
                    else    // repeat last values from buffer
                    {

                        //std::cout << "reading sample stream " << streamNumber << " aux ADCs " << std::endl;

                        channel++;
                        thisSample[channel] = auxBuffer[channel];
                        channel++;
                        thisSample[channel] = auxBuffer[channel];
                        channel++;
                        thisSample[channel] = auxBuffer[channel];
                    }
                }

            }

            // finally, loop through acquisition board ADC channels if necessary
            if (acquireAdcChannels)
            {
                for (int adcChan = 0; adcChan < 8; ++adcChan)
                {

                    channel++;
                    // ADC waveform units = volts
                    thisSample[channel] =
                        //0.000050354 * float(dataBlock->boardAdcData[adcChan][samp]);
                        0.050354 * float(dataBlock->boardAdcData[adcChan][samp]);
                }
            }
            // std::cout << channel << std::endl;

            timestamp = dataBlock->timeStamp[samp];
            //timestamp = timestamp;
            eventCode = dataBlock->ttlIn[samp];

            dataBuffer->addToBuffer(thisSample, &timestamp, &eventCode, 1);

        }

    }


    if (dacOutputShouldChange)
    {
        if (audioOutputR >= 0)
        {
            evalBoard->enableDac(0, true);
            evalBoard->selectDacDataChannel(0, audioOutputR);
        }
        else
        {
            evalBoard->enableDac(0, false);
        }

        if (audioOutputL >= 0)
        {
            evalBoard->enableDac(1, true);
            evalBoard->selectDacDataChannel(1, audioOutputL);
        }
        else
        {
            evalBoard->enableDac(1, false);
        }

        dacOutputShouldChange = false;
    }


    return true;

}