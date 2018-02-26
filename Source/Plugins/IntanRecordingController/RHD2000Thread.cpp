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

#include "RHD2000Thread.h"
#include "RHD2000Editor.h"
#include "USBThread.h"

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

#define CHIP_ID_RHD2132  1
#define CHIP_ID_RHD2216  2
#define CHIP_ID_RHD2164  4
#define CHIP_ID_RHD2164_B  1000
#define REGISTER_59_MISO_A  53
#define REGISTER_59_MISO_B  58
#define RHD2132_16CH_OFFSET 8

#define INIT_STEP 256

//#define SCAN_DEBUG
#ifdef SCAN_DEBUG
#define PRINT_ARRAYS  for (int i = 0; i < MAX_NUM_HEADSTAGES; i++) {\
	std::cout << "c" << chipId[i] << " t" << tmpChipId[i] << " s" << sumGoodDelays[i] << " if" << indexFirstGoodDelay[i] << " is" << indexSecondGoodDelay[i] << std::endl;}
#define S_DEBUG(x) do { x } while(false);
#else
#define PRINT_ARRAYS {}
#define S_DEBUG(x) {}
#endif

// Allocates memory for a 3-D array of doubles.
void allocateDoubleArray3D(std::vector<std::vector<std::vector<double> > >& array3D,
                           int xSize, int ySize, int zSize)
{
    int i, j;

    if (xSize == 0) return;
    array3D.resize(xSize);
    for (i = 0; i < xSize; ++i)
    {
        array3D[i].resize(ySize);
        for (j = 0; j < ySize; ++j)
        {
            array3D[i][j].resize(zSize);
        }
    }
}

DataThread* RHD2000Thread::createDataThread(SourceNode *sn)
{
	return new RHD2000Thread(sn);
}

RHD2000Thread::RHD2000Thread(SourceNode* sn) : DataThread(sn),
    chipRegisters(30000.0f),
    numChannels(0),
    deviceFound(false),
    isTransmitting(false),
    dacOutputShouldChange(false),
    acquireAdcChannels(false),
    acquireAuxChannels(true),
    fastSettleEnabled(false),
    fastTTLSettleEnabled(false),
    fastSettleTTLChannel(-1),
    ttlMode(false),
    dspEnabled(true),
    desiredDspCutoffFreq(0.5f),
    desiredUpperBandwidth(7500.0f),
    desiredLowerBandwidth(1.0f),
    boardSampleRate(30000.0f),
    savedSampleRateIndex(16),
    cableLengthPortA(0.914f), cableLengthPortB(0.914f), cableLengthPortC(0.914f), cableLengthPortD(0.914f), // default is 3 feet (0.914 m),
    audioOutputL(-1), audioOutputR(-1) ,numberingScheme(1),
	newScan(true)
{
	impedanceThread = new RHDImpedanceMeasure(this);
	memset(auxBuffer, 0, sizeof(auxBuffer));
	memset(auxSamples, 0, sizeof(auxSamples));

    for (int i=0; i < MAX_NUM_HEADSTAGES; i++)
        headstagesArray.add(new RHDHeadstage(i));

    evalBoard = new Rhd2000EvalBoardUsb3;
    sourceBuffers.add(new DataBuffer(2, 10000)); // start with 2 channels and automatically resize

    // Open Opal Kelly XEM6010 board.
    // Returns 1 if successful, -1 if FrontPanel cannot be loaded, and -2 if XEM6010 can't be found.

#if defined(__APPLE__)
    File appBundle = File::getSpecialLocation(File::currentApplicationFile);
    const String executableDirectory = appBundle.getChildFile("Contents/Resources").getFullPathName();
#else
    File executable = File::getSpecialLocation(File::currentExecutableFile);
    const String executableDirectory = executable.getParentDirectory().getFullPathName();
#endif

    std::cout << executableDirectory << std::endl;


    String dirName = executableDirectory;
    libraryFilePath = dirName;
    libraryFilePath += File::separatorString;
    libraryFilePath += okLIB_NAME;

    dacStream = nullptr;
    dacChannels = nullptr;
    dacThresholds = nullptr;
    dacChannelsToUpdate = nullptr;
    if (openBoard(libraryFilePath))
    {
		dataBlock = new Rhd2000DataBlockUsb3(1);
        // upload bitfile and restore default settings
        initializeBoard();
		std::cout << "USB3 board mode enabled" << std::endl;

        // automatically find connected headstages
        scanPorts(); // things would appear to run more smoothly if this were done after the editor has been created

        // probably better to do this with a thread, but a timer works for now:
        // startTimer(10); // initialize the board in the background
        dacStream = new int[8];
        dacChannels = new int[8];
        dacThresholds = new float[8];
        dacChannelsToUpdate = new bool[8];
        for (int k = 0; k < 8; k++)
        {
            dacChannelsToUpdate[k] = true;
            dacStream[k] = 0;
            setDACthreshold(k, 65534);
            dacChannels[k] = 0;
            dacThresholds[k] = 0;
        }

        // evalBoard->getDacInformation(dacChannels,dacThresholds);

        //	setDefaultNamingScheme(numberingScheme);
        //setDefaultChannelNamesAndType();
    }
}

GenericEditor* RHD2000Thread::createEditor(SourceNode* sn)
{
	return new RHD2000Editor(sn, this, true);
}

void RHD2000Thread::timerCallback()
{
    stopTimer();
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

    //deleteAndZero(dataBlock);

    delete[] dacStream;
    delete[] dacChannels;
    delete[] dacThresholds;
    delete[] dacChannelsToUpdate;
}

bool RHD2000Thread::usesCustomNames() const
{
    return true;
}

unsigned int RHD2000Thread::getNumSubProcessors() const
{
	return 1;
}

void RHD2000Thread::setDACthreshold(int dacOutput, float threshold)
{
    dacThresholds[dacOutput]= threshold;
    dacChannelsToUpdate[dacOutput] = true;
    dacOutputShouldChange = true;

    //  evalBoard->setDacThresholdVoltage(dacOutput,threshold);
}

void RHD2000Thread::setDACchannel(int dacOutput, int channel)
{
    if (channel < getNumDataOutputs(DataChannel::HEADSTAGE_CHANNEL, 0))
    {
        int channelCount = 0;
        for (int i = 0; i < enabledStreams.size(); i++)
        {
            if (channel < channelCount + numChannelsPerDataStream[i])
            {
                dacChannels[dacOutput] = channel - channelCount;
                dacStream[dacOutput] = i;
                break;
            }
            else
            {
                channelCount += numChannelsPerDataStream[i];
            }
        }
        dacChannelsToUpdate[dacOutput] = true;
        dacOutputShouldChange = true;
    }
}

Array<int> RHD2000Thread::getDACchannels() const
{
    Array<int> dacChannelsArray;
    //dacChannelsArray.addArray(dacChannels,8);
    for (int k = 0; k < 8; ++k)
    {
        dacChannelsArray.add (dacChannels[k]);
    }

    return dacChannelsArray;
}

bool RHD2000Thread::openBoard(String pathToLibrary)
{
    int return_code = evalBoard->open();

    if (return_code == 1)
    {
        deviceFound = true;
    }
    else if (return_code == -1) // dynamic library not found
    {
        bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
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

        }
        else
        {
            deviceFound = false;
        }
    }
    else if (return_code == -2)   // board could not be opened
    {
        bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
                                                     "Acquisition board not found.",
                                                     "An acquisition board could not be found. Please connect one now.",
                                                     "OK", "Cancel", 0, 0);

        if (response)
        {
            openBoard(libraryFilePath.getCharPointer()); // call recursively
        }
        else
        {
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

        bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
                                                     "FPGA bitfile not found.",
                                                     "The intan_rec_controller.bit file was not found in the directory of the executable. Would you like to browse for it?",
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

        }
        else
        {
            deviceFound = false;
        }

    }

    return deviceFound;

}

void RHD2000Thread::initializeBoard()
{
    String bitfilename;

#if defined(__APPLE__)
    File appBundle = File::getSpecialLocation(File::currentApplicationFile);
    const String executableDirectory = appBundle.getChildFile("Contents/Resources").getFullPathName();
#else
    File executable = File::getSpecialLocation(File::currentExecutableFile);
    const String executableDirectory = executable.getParentDirectory().getFullPathName();
#endif

    bitfilename = executableDirectory;
    bitfilename += File::separatorString;
	bitfilename += "intan_rec_controller.bit";

    if (!uploadBitfile(bitfilename))
    {
        return;
    }

	//Instantiate usb thread
	usbThread = new USBThread(evalBoard);

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

    setSampleRate(Rhd2000EvalBoardUsb3::SampleRate30000Hz);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortA, cableLengthPortA);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortB, cableLengthPortB);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortC, cableLengthPortC);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortD, cableLengthPortD);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortE, cableLengthPortA);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortF, cableLengthPortB);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortG, cableLengthPortC);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortH, cableLengthPortD);

    // Select RAM Bank 0 for AuxCmd3 initially, so the ADC is calibrated.
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd3, 0);

    // Since our longest command sequence is 60 commands, run the SPI interface for
    // 60 samples (64 for usb3 power-of two needs)
	evalBoard->setMaxTimeStep(INIT_STEP);
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
    ScopedPointer<Rhd2000DataBlockUsb3> dataBlock = new Rhd2000DataBlockUsb3(evalBoard->getNumEnabledDataStreams());

	evalBoard->readDataBlock(dataBlock, INIT_STEP);
    // Now that ADC calibration has been performed, we switch to the command sequence
    // that does not execute ADC calibration.
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);


    //updateRegisters();

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
	std::cout << "Scanning" << std::endl;
	impedanceThread->stopThreadSafely();
	//Clear previous known streams
	enabledStreams.clear();
	numChannelsPerDataStream.clear();

	// Scan SPI ports

	int ledArray[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	evalBoard->setSpiLedDisplay(ledArray);

	int delay, hs, id;
	int register59Value;
	//int numChannelsOnPort[4] = {0, 0, 0, 0};

	/*
	Rhd2000EvalBoard::BoardDataSource initStreamDdrPorts[8] =
	{
	Rhd2000EvalBoard::PortA1Ddr,
	Rhd2000EvalBoard::PortA2Ddr,
	Rhd2000EvalBoard::PortB1Ddr,
	Rhd2000EvalBoard::PortB2Ddr,
	Rhd2000EvalBoard::PortC1Ddr,
	Rhd2000EvalBoard::PortC2Ddr,
	Rhd2000EvalBoard::PortD1Ddr,
	Rhd2000EvalBoard::PortD2Ddr
	};
	*/

	chipId.clearQuick();
	chipId.insertMultiple(0, -1, MAX_NUM_HEADSTAGES);
	Array<int> tmpChipId(chipId);

	setSampleRate(Rhd2000EvalBoardUsb3::SampleRate30000Hz, true); // set to 30 kHz temporarily

	// Enable all data streams, and set sources to cover one or two chips
	// on Ports A-D.
	for (int i = 0; i < MAX_NUM_DATA_STREAMS; i += 2)
	{
		evalBoard->enableDataStream(i, true);
		evalBoard->enableDataStream(i + 1, false);
	}

	std::cout << "Number of enabled data streams: " << evalBoard->getNumEnabledDataStreams() << std::endl;


	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH,
		Rhd2000EvalBoardUsb3::AuxCmd3, 0);

	// Since our longest command sequence is 60 commands, we run the SPI
	// interface for 60 samples. (64 for usb3 power-of two needs)
	evalBoard->setMaxTimeStep(INIT_STEP);
	evalBoard->setContinuousRunMode(false);

	ScopedPointer<Rhd2000DataBlockUsb3> dataBlock =
		new Rhd2000DataBlockUsb3(evalBoard->getNumEnabledDataStreams());

	Array<int> sumGoodDelays;
	sumGoodDelays.insertMultiple(0, 0, MAX_NUM_HEADSTAGES);

	Array<int> indexFirstGoodDelay;
	indexFirstGoodDelay.insertMultiple(0, -1, MAX_NUM_HEADSTAGES);

	Array<int> indexSecondGoodDelay;
	indexSecondGoodDelay.insertMultiple(0, -1, MAX_NUM_HEADSTAGES);

	PRINT_ARRAYS;


	// Run SPI command sequence at all 16 possible FPGA MISO delay settings
	// to find optimum delay for each SPI interface cable.

	std::cout << "Checking for connected amplifier chips..." << std::endl;

	for (delay = 0; delay < 16; delay++)//(delay = 0; delay < 16; ++delay)
	{
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortA, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortB, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortC, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortD, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortE, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortF, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortG, delay);
		evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortH, delay);

		// Start SPI interface.
		evalBoard->run();

		// Wait for the 60-sample run to complete.
		while (evalBoard->isRunning())
		{
			;
		}
		// Read the resulting single data block from the USB interface.
		evalBoard->readDataBlock(dataBlock.get(), INIT_STEP);
		
		// Read the Intan chip ID number from each RHD2000 chip found.
		// Record delay settings that yield good communication with the chip.
		for (hs = 0; hs < MAX_NUM_HEADSTAGES; ++hs)//MAX_NUM_DATA_STREAMS; ++stream)
		{
			S_DEBUG(std::cout << "Stream number " << hs << ", delay = " << delay << std::endl;
			dataBlock->print(hs);)

			id = deviceId(dataBlock, hs, register59Value);
			S_DEBUG(std::cout << "h " << hs << " id " << id << std::endl;)

			if (id == CHIP_ID_RHD2132 || id == CHIP_ID_RHD2216 ||
				(id == CHIP_ID_RHD2164 && register59Value == REGISTER_59_MISO_A))
			{
				//  std::cout << "Device ID found: " << id << std::endl;

				sumGoodDelays.set(hs, sumGoodDelays[hs] + 1);

				if (indexFirstGoodDelay[hs] == -1)
				{
					indexFirstGoodDelay.set(hs, delay);
					tmpChipId.set(hs, id);
				}
				else if (indexSecondGoodDelay[hs] == -1)
				{
					indexSecondGoodDelay.set(hs, delay);
					tmpChipId.set(hs, id);
				}
			}
		}
	}
	PRINT_ARRAYS;
	S_DEBUG(
	std::cout << "s: " << enabledStreams.size() << std::endl;
	for (int i = 0; i < enabledStreams.size(); i++)
		std::cout << "s " << enabledStreams[i];

	std::cout << "n: " << numChannelsPerDataStream.size() << std::endl;
	for (int i = 0; i < numChannelsPerDataStream.size(); i++)
		std::cout << "n " << numChannelsPerDataStream[i];
	)
    // Now, disable data streams where we did not find chips present.
    int chipIdx = 0;
    for (hs = 0; hs < MAX_NUM_HEADSTAGES; ++hs)
    {
        if ((tmpChipId[hs] > 0) && (enabledStreams.size() < MAX_NUM_DATA_STREAMS))
        {
            chipId.set(chipIdx++,tmpChipId[hs]);
			S_DEBUG(std::cout << "Enabling headstage on stream " << hs << std::endl;)
            if (tmpChipId[hs] == CHIP_ID_RHD2164) //RHD2164
            {
                if (enabledStreams.size() < MAX_NUM_DATA_STREAMS - 1)
                {
                    enableHeadstage(hs,true,2,32);
                    chipId.set(chipIdx++,CHIP_ID_RHD2164_B);
                }
                else //just one stream left
                {
                    enableHeadstage(hs,true,1,32);
                }
            }
            else
            {
                enableHeadstage(hs, true,1,tmpChipId[hs] == 1 ? 32:16);
            }
        }
        else
        {
			S_DEBUG(std::cout << "Disabling headstage on stream " << hs << std::endl;)
            enableHeadstage(hs, false);
        }
    }
	S_DEBUG(
	std::cout << "s: " << enabledStreams.size() << std::endl;
	for (int i = 0; i < enabledStreams.size(); i++)
		std::cout << "s " << enabledStreams[i];

	std::cout << "n: " << numChannelsPerDataStream.size() << std::endl;
	for (int i = 0; i < numChannelsPerDataStream.size(); i++)
		std::cout << "n " << numChannelsPerDataStream[i];
	)
//	for (int i = 0; i < 16; i++)
//		enableHeadstage(i, true, 2, 32);
	updateBoardStreams();


    std::cout << "Number of enabled data streams: " << evalBoard->getNumEnabledDataStreams() << std::endl;

	for (int i = 0; i < MAX_NUM_HEADSTAGES; i+= 2)
	{
		if (headstagesArray[i]->isPlugged() || headstagesArray[i + 1]->isPlugged())
			ledArray[i / 2] = 1;
	}
	evalBoard->setSpiLedDisplay(ledArray);

    // Set cable delay settings that yield good communication with each
    // RHD2000 chip.
    Array<int> optimumDelay;
    optimumDelay.insertMultiple(0,0,MAX_NUM_HEADSTAGES);

    for (hs = 0; hs < MAX_NUM_HEADSTAGES; ++hs)
    {
        if (sumGoodDelays[hs] == 1 || sumGoodDelays[hs] == 2)
        {
            optimumDelay.set(hs,indexFirstGoodDelay[hs]);
        }
        else if (sumGoodDelays[hs] > 2)
        {
            optimumDelay.set(hs,indexSecondGoodDelay[hs]);
        }
    }

    evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortA,
                             max(optimumDelay[0],optimumDelay[1]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortB,
                             max(optimumDelay[2],optimumDelay[3]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortC,
                             max(optimumDelay[4],optimumDelay[5]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortD,
                             max(optimumDelay[6],optimumDelay[7]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortE,
							max(optimumDelay[8], optimumDelay[9]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortF,
							max(optimumDelay[10], optimumDelay[11]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortG,
							max(optimumDelay[12], optimumDelay[13]));
	evalBoard->setCableDelay(Rhd2000EvalBoardUsb3::PortH,
							max(optimumDelay[14], optimumDelay[15]));

    cableLengthPortA =
        evalBoard->estimateCableLengthMeters(max(optimumDelay[0],optimumDelay[1]));
    cableLengthPortB =
        evalBoard->estimateCableLengthMeters(max(optimumDelay[2],optimumDelay[3]));
    cableLengthPortC =
        evalBoard->estimateCableLengthMeters(max(optimumDelay[4],optimumDelay[5]));
    cableLengthPortD =
        evalBoard->estimateCableLengthMeters(max(optimumDelay[6],optimumDelay[7]));
	cableLengthPortE =
		evalBoard->estimateCableLengthMeters(max(optimumDelay[8], optimumDelay[9]));
	cableLengthPortF =
		evalBoard->estimateCableLengthMeters(max(optimumDelay[10], optimumDelay[11]));
	cableLengthPortG =
		evalBoard->estimateCableLengthMeters(max(optimumDelay[12], optimumDelay[13]));
	cableLengthPortH =
		evalBoard->estimateCableLengthMeters(max(optimumDelay[14], optimumDelay[15]));

    setSampleRate(savedSampleRateIndex); // restore saved sample rate
    //updateRegisters();
    newScan = true;
}

int RHD2000Thread::deviceId(Rhd2000DataBlockUsb3* dataBlock, int stream, int& register59Value)
{
    bool intanChipPresent;

    // First, check ROM registers 32-36 to verify that they hold 'INTAN', and
    // the initial chip name ROM registers 24-26 that hold 'RHD'.
    // This is just used to verify that we are getting good data over the SPI
    // communication channel.
    intanChipPresent = ((char) dataBlock->auxiliaryData[stream][2][32] == 'I' &&
                        (char) dataBlock->auxiliaryData[stream][2][33] == 'N' &&
                        (char) dataBlock->auxiliaryData[stream][2][34] == 'T' &&
                        (char) dataBlock->auxiliaryData[stream][2][35] == 'A' &&
                        (char) dataBlock->auxiliaryData[stream][2][36] == 'N' &&
                        (char) dataBlock->auxiliaryData[stream][2][24] == 'R' &&
                        (char) dataBlock->auxiliaryData[stream][2][25] == 'H' &&
                        (char) dataBlock->auxiliaryData[stream][2][26] == 'D');

    // If the SPI communication is bad, return -1.  Otherwise, return the Intan
    // chip ID number stored in ROM regstier 63.
    if (!intanChipPresent)
    {
        register59Value = -1;
        return -1;
    }
    else
    {
        register59Value = dataBlock->auxiliaryData[stream][2][23]; // Register 59
        return dataBlock->auxiliaryData[stream][2][19]; // chip ID (Register 63)
    }
}



bool RHD2000Thread::isAcquisitionActive() const
{
    return isTransmitting;
}

void RHD2000Thread::setNumChannels(int hsNum, int numChannels)
{
    if (headstagesArray[hsNum]->getNumChannels() == 32)
    {
        if (numChannels < headstagesArray[hsNum]->getNumChannels())
            headstagesArray[hsNum]->setHalfChannels(true);
        else
            headstagesArray[hsNum]->setHalfChannels(false);
        numChannelsPerDataStream.set(headstagesArray[hsNum]->getStreamIndex(0), numChannels);
    }
}

int RHD2000Thread::getHeadstageChannels (int hsNum) const
{
    return headstagesArray[hsNum]->getNumChannels();
}


void RHD2000Thread::getEventChannelNames (StringArray& Names) const
{
    Names.clear();
    for (int k = 0; k < 8; ++k)
    {
        Names.add ("TTL" + String (k + 1));
    }
}


/* go over the old names and tests whether this particular channel name was changed.
if so, return the old name */
int RHD2000Thread::modifyChannelName(int channel, String newName)
{
    ChannelCustomInfo i = channelInfo[channel];
    i.name = newName;
    i.modified = true;
    channelInfo.set(channel, i);
    return 0;
}

String RHD2000Thread::getChannelName (int ch) const
{
    return channelInfo[ch].name;
}

int RHD2000Thread::modifyChannelGain(int channel, float gain)
{
    ChannelCustomInfo i = channelInfo[channel];
    i.gain = gain;
    i.modified = true;
    channelInfo.set(channel, i);
    return 0;
}

void RHD2000Thread::setDefaultNamingScheme(int scheme)
{
    numberingScheme = scheme;
    newScan = true; //if the scheme is changed, reset all names
    setDefaultChannelNames();
}

/* This will give default names & gains to channels, unless they were manually modified by the user
 In that case, the query channelModified, will return the values that need to be put */
void RHD2000Thread::setDefaultChannelNames()
{
    int aux_counter = 1;
    int channelNumber = 1;
    String oldName;
    //int dummy;
    //float oldGain;
    StringArray stream_prefix;
    stream_prefix.add("A1");
    stream_prefix.add("A2");
    stream_prefix.add("B1");
    stream_prefix.add("B2");
    stream_prefix.add("C1");
    stream_prefix.add("C2");
    stream_prefix.add("D1");
    stream_prefix.add("D2");
	stream_prefix.add("E1");
	stream_prefix.add("E2");
	stream_prefix.add("F1");
	stream_prefix.add("F2");
	stream_prefix.add("G1");
	stream_prefix.add("G2");
	stream_prefix.add("H1");
	stream_prefix.add("H2");

    for (int i = 0; i < MAX_NUM_HEADSTAGES; i++)
    {
        if (headstagesArray[i]->isPlugged())
        {
            for (int k = 0; k < headstagesArray[i]->getNumActiveChannels(); k++)
            {
                if (newScan || !channelInfo[k].modified)
                {
                    ChannelCustomInfo in;
                    if (numberingScheme == 1)
                        in.name = "CH" + String(channelNumber);
                    else
                        in.name = "CH_" + stream_prefix[i] + "_" + String(1 + k);
                    in.gain = getBitVolts(sn->getDataChannel(k));
                    channelInfo.set(channelNumber-1, in);

                }
                channelNumber++;
            }
        }
    }
    //Aux channels
    for (int i = 0; i < MAX_NUM_HEADSTAGES; i++)
    {
        if (headstagesArray[i]->isPlugged())
        {
            for (int k = 0; k < 3; k++)
            {
                int chn = channelNumber - 1;

                if (newScan || !channelInfo[chn].modified)
                {
                    ChannelCustomInfo in;
                    if (numberingScheme == 1)
                        in.name = "AUX" + String(aux_counter);
                    else
                        in.name = "AUX_" + stream_prefix[i] + "_" + String(1 + k);
                    in.gain = getBitVolts(sn->getDataChannel(chn));
                    channelInfo.set(chn, in);

                }
                channelNumber++;
                aux_counter++;
            }
        }
    }
    //ADC channels
    if (acquireAdcChannels)
    {
        for (int k = 0; k < 8; k++)
        {
            int chn = channelNumber - 1;
            if (newScan || !channelInfo[chn].modified)
            {
                ChannelCustomInfo in;
                in.name = "ADC" + String(k + 1);
                in.gain = getAdcBitVolts(k);
                channelInfo.set(chn, in);
            }
            channelNumber++;
        }
    }
    newScan = false;
}

int RHD2000Thread::getNumChannels() const
{
	return getNumDataOutputs(DataChannel::HEADSTAGE_CHANNEL, 0) + getNumDataOutputs(DataChannel::AUX_CHANNEL, 0) + getNumDataOutputs(DataChannel::ADC_CHANNEL, 0);
}

int RHD2000Thread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subproc) const
{
	if (subproc > 0) return 0;
	if (type == DataChannel::HEADSTAGE_CHANNEL)
	{
		int newNumChannels = 0;
		for (int i = 0; i < MAX_NUM_HEADSTAGES; ++i)
		{
			if (headstagesArray[i]->isPlugged())
			{
				newNumChannels += headstagesArray[i]->getNumActiveChannels();
			}
		}

		return newNumChannels;
	}
	if (type == DataChannel::AUX_CHANNEL)
	{
		int numAuxOutputs = 0;

		for (int i = 0; i < MAX_NUM_HEADSTAGES; ++i)
		{
			if (headstagesArray[i]->isPlugged() > 0)
			{
				numAuxOutputs += 3;
			}
		}

		return numAuxOutputs;
	}
	if (type == DataChannel::ADC_CHANNEL)
	{
		if (acquireAdcChannels)
		{
			return 8;
		}
		else
		{
			return 0;
		}
	}
    return 0;
}

String RHD2000Thread::getChannelUnits(int chanIndex) const
{
	switch (sn->getDataChannel(chanIndex)->getChannelType())
	{
	case DataChannel::AUX_CHANNEL:
		return "mV";
	case DataChannel::ADC_CHANNEL:
		return "V";
	default:
		return "uV";
	}
}


int RHD2000Thread::getNumTTLOutputs(int subproc) const
{
	if (subproc > 0) return 0;
	return 8;
}

float RHD2000Thread::getSampleRate(int subproc) const
{
    return evalBoard->getSampleRate();
}

float RHD2000Thread::getBitVolts (const DataChannel* ch) const
{
    if (ch->getChannelType() == DataChannel::ADC_CHANNEL)
        return getAdcBitVolts (ch->getSourceTypeIndex());
    else if (ch->getChannelType() == DataChannel::AUX_CHANNEL)
        return 0.0000374;
    else
        return 0.195f;
}

float RHD2000Thread::getAdcBitVolts (int chan) const
{
    if (chan < adcBitVolts.size())
        return adcBitVolts[chan];
    else
        return 0.00015258789;
}

double RHD2000Thread::setUpperBandwidth(double upper)
{
	impedanceThread->stopThreadSafely();
    desiredUpperBandwidth = upper;

    updateRegisters();

    return actualUpperBandwidth;
}


double RHD2000Thread::setLowerBandwidth(double lower)
{
	impedanceThread->stopThreadSafely();
    desiredLowerBandwidth = lower;

    updateRegisters();

    return actualLowerBandwidth;
}

double RHD2000Thread::setDspCutoffFreq(double freq)
{
	impedanceThread->stopThreadSafely();
    desiredDspCutoffFreq = freq;

    updateRegisters();

    return actualDspCutoffFreq;
}

double RHD2000Thread::getDspCutoffFreq() const
{
    return actualDspCutoffFreq;
}

void RHD2000Thread::setDSPOffset(bool state)
{
	impedanceThread->stopThreadSafely();
    dspEnabled = state;
    updateRegisters();
}

void RHD2000Thread::setTTLoutputMode(bool state)
{
    ttlMode = state;
    dacOutputShouldChange = true;
}

void RHD2000Thread::setDAChpf(float cutoff, bool enabled)
{
    dacOutputShouldChange = true;
    desiredDAChpf = cutoff;
    desiredDAChpfState = enabled;
}

void RHD2000Thread::setFastTTLSettle(bool state, int channel)
{
    fastTTLSettleEnabled = state;
    fastSettleTTLChannel = channel;
    dacOutputShouldChange = true;
}

int RHD2000Thread::setNoiseSlicerLevel(int level)
{
    desiredNoiseSlicerLevel = level;
    if (deviceFound)
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

bool RHD2000Thread::enableHeadstage(int hsNum, bool enabled, int nStr, int strChans)
{
    /*   evalBoard->enableDataStream(hsNum, enabled);*/
    if (enabled)
    {
        headstagesArray[hsNum]->setNumStreams(nStr);
        headstagesArray[hsNum]->setChannelsPerStream(strChans,enabledStreams.size());
        enabledStreams.add(headstagesArray[hsNum]->getDataStream(0));
        numChannelsPerDataStream.add(strChans);
        if (nStr > 1)
        {
            enabledStreams.add(headstagesArray[hsNum]->getDataStream(1));
            numChannelsPerDataStream.add(strChans);
        }
    }
    else
    {
        int idx = enabledStreams.indexOf(headstagesArray[hsNum]->getDataStream(0));
        if (idx >= 0)
        {
            enabledStreams.remove(idx);
            numChannelsPerDataStream.remove(idx);
        }
        if (headstagesArray[hsNum]->getNumStreams() > 1)
        {
            idx = enabledStreams.indexOf(headstagesArray[hsNum]->getDataStream(1));
            if (idx >= 0)
            {
                enabledStreams.remove(idx);
                numChannelsPerDataStream.remove(idx);
            }
        }
        headstagesArray[hsNum]->setNumStreams(0);
    }

    /*
    std::cout << "Enabled channels: ";

    for (int i = 0; i < MAX_NUM_DATA_STREAMS; i++)
    {
        std::cout << numChannelsPerDataStream[i] << " ";
    }*/

    sourceBuffers[0]->resize(getNumChannels(), 10000);

    return true;
}

void RHD2000Thread::updateBoardStreams()
{
    for (int i=0; i <  MAX_NUM_DATA_STREAMS; i++)
    {
        if (enabledStreams.contains(i))
        {
            evalBoard->enableDataStream(i,true);
        }
        else
        {
            evalBoard->enableDataStream(i,false);
        }
    }
}

bool RHD2000Thread::isHeadstageEnabled(int hsNum) const
{
    return headstagesArray[hsNum]->isPlugged();
}

bool RHD2000Thread::isReady()
{
	return deviceFound && (getNumChannels() > 0);
}

int RHD2000Thread::getActiveChannelsInHeadstage (int hsNum) const
{
    return headstagesArray[hsNum]->getNumActiveChannels();
}

int RHD2000Thread::getChannelsInHeadstage (int hsNum) const
{
    return headstagesArray[hsNum]->getNumChannels();
}

/*void RHD2000Thread::assignAudioOut(int dacChannel, int dataChannel)
{
    if (deviceFound)
    {
        if (dacChannel == 0)
        {
            audioOutputR = dataChannel;
            dacChannels[0] = dataChannel;
        }
        else if (dacChannel == 1)
        {
            audioOutputL = dataChannel;
            dacChannels[1] = dataChannel;
        }

        dacOutputShouldChange = true; // set a flag and take care of setting wires
        // during the updateBuffer() method
        // to avoid problems
    }

}*/

void RHD2000Thread::enableAdcs(bool t)
{
    acquireAdcChannels = t;

    sourceBuffers[0]->resize (getNumChannels(), 10000);
}


void RHD2000Thread::setSampleRate(int sampleRateIndex, bool isTemporary)
{
	impedanceThread->stopThreadSafely();
    if (!isTemporary)
    {
        savedSampleRateIndex = sampleRateIndex;
    }

    int numUsbBlocksToRead = 0; // placeholder - make this change the number of blocks that are read in RHD2000Thread::updateBuffer()

	Rhd2000EvalBoardUsb3::AmplifierSampleRate sampleRate; // just for local use

    switch (sampleRateIndex)
    {
        case 0:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate1000Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1000.0f;
            break;
        case 1:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate1250Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1250.0f;
            break;
        case 2:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate1500Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 1500.0f;
            break;
        case 3:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate2000Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 2000.0f;
            break;
        case 4:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate2500Hz;
            numUsbBlocksToRead = 1;
            boardSampleRate = 2500.0f;
            break;
        case 5:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate3000Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 3000.0f;
            break;
        case 6:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate3333Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 3333.0f;
            break;
        case 7:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate4000Hz;
            numUsbBlocksToRead = 2;
            boardSampleRate = 4000.0f;
            break;
        case 8:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate5000Hz;
            numUsbBlocksToRead = 3;
            boardSampleRate = 5000.0f;
            break;
        case 9:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate6250Hz;
            numUsbBlocksToRead = 3;
            boardSampleRate = 6250.0f;
            break;
        case 10:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate8000Hz;
            numUsbBlocksToRead = 4;
            boardSampleRate = 8000.0f;
            break;
        case 11:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate10000Hz;
            numUsbBlocksToRead = 6;
            boardSampleRate = 10000.0f;
            break;
        case 12:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate12500Hz;
            numUsbBlocksToRead = 7;
            boardSampleRate = 12500.0f;
            break;
        case 13:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate15000Hz;
            numUsbBlocksToRead = 8;
            boardSampleRate = 15000.0f;
            break;
        case 14:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate20000Hz;
            numUsbBlocksToRead = 12;
            boardSampleRate = 20000.0f;
            break;
        case 15:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate25000Hz;
            numUsbBlocksToRead = 14;
            boardSampleRate = 25000.0f;
            break;
        case 16:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate30000Hz;
            numUsbBlocksToRead = 16;
            boardSampleRate = 30000.0f;
            break;
        default:
			sampleRate = Rhd2000EvalBoardUsb3::SampleRate10000Hz;
            numUsbBlocksToRead = 6;
            boardSampleRate = 10000.0f;
    }


    // Select per-channel amplifier sampling rate.
    evalBoard->setSampleRate(sampleRate);

    std::cout << "Sample rate set to " << evalBoard->getSampleRate() << std::endl;

    // Now that we have set our sampling rate, we can set the MISO sampling delay
    // which is dependent on the sample rate.
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortA, cableLengthPortA);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortB, cableLengthPortB);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortC, cableLengthPortC);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortD, cableLengthPortD);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortE, cableLengthPortE);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortF, cableLengthPortF);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortG, cableLengthPortG);
	evalBoard->setCableLengthMeters(Rhd2000EvalBoardUsb3::PortH, cableLengthPortH);

    updateRegisters();

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

    // Create a command list for the AuxCmd1 slot.  This command sequence will continuously
    // update Register 3, which controls the auxiliary digital output pin on each RHD2000 chip.
    // In concert with the v1.4 Rhythm FPGA code, this permits real-time control of the digital
    // output pin on chips on each SPI port.
    chipRegisters.setDigOutLow();   // Take auxiliary output out of HiZ mode.
    commandSequenceLength = chipRegisters.createCommandListUpdateDigOut(commandList);
	evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd1, 0, commandSequenceLength - 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd1, 0);

    // // Next, we'll create a command list for the AuxCmd2 slot.  This command sequence
    // // will sample the temperature sensor and other auxiliary ADC inputs.
    commandSequenceLength = chipRegisters.createCommandListTempSensor(commandList);
	evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd2, 0, commandSequenceLength - 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd2, 0);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd2, 0);

    // Before generating register configuration command sequences, set amplifier
    // bandwidth paramters.
    actualDspCutoffFreq = chipRegisters.setDspCutoffFreq(desiredDspCutoffFreq);
    //std::cout << "DSP Cutoff Frequency " << actualDspCutoffFreq << std::endl;
    actualLowerBandwidth = chipRegisters.setLowerBandwidth(desiredLowerBandwidth);
    actualUpperBandwidth = chipRegisters.setUpperBandwidth(desiredUpperBandwidth);
    chipRegisters.enableDsp(dspEnabled);
    //std::cout << "DSP Offset Status " << dspEnabled << std::endl;

    // turn on aux inputs
    chipRegisters.enableAux1(true);
    chipRegisters.enableAux2(true);
    chipRegisters.enableAux3(true);

    chipRegisters.createCommandListRegisterConfig(commandList, true);
    // Upload version with ADC calibration to AuxCmd3 RAM Bank 0.
	evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 0);
	evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    commandSequenceLength = chipRegisters.createCommandListRegisterConfig(commandList, false);
    // Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
	evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 1);
	evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd3, 0,
                                      commandSequenceLength - 1);


    chipRegisters.setFastSettle(true);

    commandSequenceLength = chipRegisters.createCommandListRegisterConfig(commandList, false);
    // Upload version with fast settle enabled to AuxCmd3 RAM Bank 2.
	evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 2);
	evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd3, 0,
                                      commandSequenceLength - 1);

    chipRegisters.setFastSettle(false);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd3,
                                    fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
	evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd3,
									fastSettleEnabled ? 2 : 1);
}

bool RHD2000Thread::startAcquisition()
{
	impedanceThread->waitSafely();
    dataBlock = new Rhd2000DataBlockUsb3(evalBoard->getNumEnabledDataStreams());

    std::cout << "Expecting " << getNumChannels() << " channels." << std::endl;

    //memset(filter_states,0,256*sizeof(double));

    int ledArray[8] = {1, 1, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);

    cout << "Number of 16-bit words in FIFO: " << evalBoard->getNumWordsInFifo() << endl;
    cout << "Is eval board running: " << evalBoard->isRunning() << endl;


    //std::cout << "Setting max timestep." << std::endl;
    //evalBoard->setMaxTimeStep(100);

	blockSize = dataBlock->calculateDataBlockSizeInWords(evalBoard->getNumEnabledDataStreams());
	std::cout << "Expecting blocksize of " << blockSize << " for " << evalBoard->getNumEnabledDataStreams() << " streams" << std::endl;

	//evalBoard->printFIFOmetrics();

	// evalBoard->setContinuousRunMode(false);
	//  evalBoard->setMaxTimeStep(0);
	std::cout << "Flushing FIFO." << std::endl;
	evalBoard->flush();
	std::cout << "FIFO count " << evalBoard->getNumWordsInFifo() << std::endl;

	std::cout << "Starting usb thread with buffer of " << blockSize * 2 << " bytes" << std::endl;
	usbThread->startAcquisition(blockSize * 2);

	std::cout << "Starting acquisition." << std::endl;
	evalBoard->setContinuousRunMode(true);
	//evalBoard->printFIFOmetrics();
	evalBoard->run();
	//evalBoard->printFIFOmetrics();
	startThread();
	


    
    


    isTransmitting = true;

    return true;
}

bool RHD2000Thread::stopAcquisition()
{

    //  isTransmitting = false;
    std::cout << "RHD2000 data thread stopping acquisition." << std::endl;
	usbThread->stopAcquisition();

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

    if (deviceFound)
    {
        evalBoard->setContinuousRunMode(false);
        evalBoard->setMaxTimeStep(0);
        std::cout << "Flushing FIFO." << std::endl;
        evalBoard->flush();
        //   evalBoard->setContinuousRunMode(true);
        //   evalBoard->run();

    }

    sourceBuffers[0]->clear();

    if (deviceFound)
    {
        cout << "Number of 16-bit words in FIFO: " << evalBoard->getNumWordsInFifo() << endl;

        // std::cout << "Stopped eval board." << std::endl;


        int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
        evalBoard->setLedDisplay(ledArray);
    }

    isTransmitting = false;
	dacOutputShouldChange = false;

    return true;
}

bool RHD2000Thread::updateBuffer()
{
	//int chOffset;
    //cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;
	//cout << "Block size: " << blockSize << endl;
	unsigned char* bufferPtr;
	//std::cout << "Current number of words: " <<  evalBoard->numWordsInFifo() << " for " << blockSize << std::endl;
	long return_code;

	return_code = usbThread->usbRead(bufferPtr);
	if (return_code == 0)
		return true;

	int index = 0;
	int auxIndex, chanIndex;
	int numStreams = enabledStreams.size();
	int nSamps = Rhd2000DataBlockUsb3::getSamplesPerDataBlock();

	//evalBoard->printFIFOmetrics();
	for (int samp = 0; samp < nSamps; samp++)
	{
		int channel = -1;

		if (!Rhd2000DataBlockUsb3::checkUsbHeader(bufferPtr, index))
		{
			cerr << "Error in Rhd2000EvalBoard::readDataBlock: Incorrect header." << endl;
			cerr << "Read code: " << return_code << endl;
			break;
		}

		index += 8;
		timestamps.set(0, Rhd2000DataBlockUsb3::convertUsbTimeStamp(bufferPtr, index));
		index += 4;
		auxIndex = index;
		//skip the aux channels
		index += numStreams * 6;
		// do the neural data channels first
		for (int dataStream = 0; dataStream < numStreams; dataStream++)
		{
			int nChans = numChannelsPerDataStream[dataStream];
			chanIndex = index + 2 * dataStream;
			if ((chipId[dataStream] == CHIP_ID_RHD2132) && (nChans == 16)) //RHD2132 16ch. headstage
			{
				chanIndex += 2 * RHD2132_16CH_OFFSET*numStreams;
			}
			for (int chan = 0; chan < nChans; chan++)
			{
				channel++;
				thisSample[channel] = float(*(uint16*)(bufferPtr + chanIndex) - 32768)*0.195f;
				chanIndex += 2 * numStreams;
			}
		}
		index += 2 * CHANNELS_PER_STREAM * numStreams;
		//now we can do the aux channels
		auxIndex += 2 * numStreams;
		for (int dataStream = 0; dataStream < numStreams; dataStream++)
		{
			if (chipId[dataStream] != CHIP_ID_RHD2164_B)
			{
				int auxNum = (samp + 3) % 4;
				if (auxNum < 3)
				{
					auxSamples[dataStream][auxNum] = float(*(uint16*)(bufferPtr + auxIndex) - 32768)*0.0000374;
				}
				for (int chan = 0; chan < 3; chan++)
				{
					channel++;
					if (auxNum == 3)
					{
						auxBuffer[channel] = auxSamples[dataStream][chan];
					}
					thisSample[channel] = auxBuffer[channel];
				}
			}
			auxIndex += 2;

		}

		//skip filler words
		index += 2 * (numStreams % 4);
		if (acquireAdcChannels)
		{
			for (int adcChan = 0; adcChan < 8; ++adcChan)
			{

				channel++;
				// ADC waveform units = volts
				thisSample[channel] =
					//0.000050354 * float(dataBlock->boardAdcData[adcChan][samp]);
					0.00015258789 * float(*(uint16*)(bufferPtr + index)) - 5 - 0.4096; // account for +/-5V input range and DC offset
				index += 2;
			}
		}
		else
		{
			index += 16;
		}
		ttlEventWords.set(0, *(uint16*)(bufferPtr + index));
		index += 4;
		sourceBuffers[0]->addToBuffer(thisSample, &timestamps.getReference(0), &ttlEventWords.getReference(0), 1);
	}




	if (dacOutputShouldChange)
	{
        for (int k=0; k<8; k++)
        {
            if (dacChannelsToUpdate[k])
            {
                dacChannelsToUpdate[k] = false;
                if (dacChannels[k] >= 0)
                {
                    evalBoard->enableDac(k, true);
                    evalBoard->selectDacDataStream(k, dacStream[k]);
                    evalBoard->selectDacDataChannel(k, dacChannels[k]);
                    evalBoard->setDacThreshold(k, (int)abs((dacThresholds[k]/0.195) + 32768),dacThresholds[k] >= 0);
                   // evalBoard->setDacThresholdVoltage(k, (int) dacThresholds[k]);
                }
                else
                {
                    evalBoard->enableDac(k, false);
                }
            }
        }

        evalBoard->setTtlMode(ttlMode ? 1 : 0);
        evalBoard->enableExternalFastSettle(fastTTLSettleEnabled);
        evalBoard->setExternalFastSettleChannel(fastSettleTTLChannel);
        evalBoard->setDacHighpassFilter(desiredDAChpf);
        evalBoard->enableDacHighpassFilter(desiredDAChpfState);

        dacOutputShouldChange = false;
    }
	
    return true;

}

int RHD2000Thread::getChannelFromHeadstage (int hs, int ch) const
{
    int channelCount = 0;
    int hsCount = 0;
    if (hs < 0 || hs >= MAX_NUM_HEADSTAGES+1)
        return -1;
    if (hs == MAX_NUM_HEADSTAGES) //let's consider this the ADC channels
    {
		if (getNumDataOutputs(DataChannel::ADC_CHANNEL, 0) > 0)
        {
			return getNumDataOutputs(DataChannel::HEADSTAGE_CHANNEL, 0) + getNumDataOutputs(DataChannel::AUX_CHANNEL, 0) + ch;
        }
        else
            return -1;
    }
    if (headstagesArray[hs]->isPlugged())
    {
        if (ch < 0)
            return -1;
        if (ch < headstagesArray[hs]->getNumActiveChannels())
        {
            for (int i = 0; i < hs; i++)
            {
                channelCount += headstagesArray[i]->getNumActiveChannels();
            }
            return channelCount + ch;
        }
        else if (ch < headstagesArray[hs]->getNumActiveChannels() + 3)
        {
            for (int i = 0; i < MAX_NUM_HEADSTAGES; i++)
            {
                if (headstagesArray[i]->isPlugged())
                {
                    channelCount += headstagesArray[i]->getNumActiveChannels();
                    if (i < hs)
                        hsCount++;
                }
            }
			return channelCount + hsCount * 3 + ch-headstagesArray[hs]->getNumActiveChannels();
        }
        else
        {
            return -1;
        }

    }
    else
    {
        return -1;
    }
}

int RHD2000Thread::getHeadstageChannel (int& hs, int ch) const
{
    int channelCount = 0;
    int hsCount = 0;

    if (ch < 0)
        return -1;

    for (int i = 0; i < MAX_NUM_HEADSTAGES; i++)
    {
        if (headstagesArray[i]->isPlugged())
        {
            int chans = headstagesArray[hs]->getNumActiveChannels();
            if (ch >= channelCount && ch < channelCount + chans)
            {
                hs = i;
                return ch - channelCount;
            }
            channelCount += chans;
            hsCount++;
        }
    }
    if (ch < (channelCount + hsCount * 3)) //AUX
    {
        hsCount = (ch - channelCount) / 3;
        for (int i = 0; i < MAX_NUM_HEADSTAGES; i++)
        {
            if (headstagesArray[i]->isPlugged())
            {
                if (hsCount == 0)
                {
                    hs = i;
                    return ch - channelCount;
                }
                hsCount--;
                channelCount++;
            }
        }
    }
    return -1;
}

void RHD2000Thread::runImpedanceTest(ImpedanceData* data)
{
	impedanceThread->stopThreadSafely();
	impedanceThread->prepareData(data);
	impedanceThread->startThread();
}


RHDHeadstage::RHDHeadstage(int stream) :
    dataStream(stream), numStreams(0), channelsPerStream(32), halfChannels(false)
{
	streamIndex = -1;
}

RHDHeadstage::~RHDHeadstage()
{
}

void RHDHeadstage::setNumStreams(int num)
{
    numStreams = num;
}

void RHDHeadstage::setChannelsPerStream(int nchan, int index)
{
    channelsPerStream = nchan;
	streamIndex = index;
}

int RHDHeadstage::getStreamIndex (int index) const
{
    return streamIndex + index;
}

int RHDHeadstage::getNumChannels() const
{
    return channelsPerStream*numStreams;
}

int RHDHeadstage::getNumStreams() const
{
    return numStreams;
}

void RHDHeadstage::setHalfChannels(bool half)
{
    halfChannels = half;
}

int RHDHeadstage::getNumActiveChannels() const
{
    return (int)(getNumChannels() / (halfChannels ? 2 : 1));
}

int RHDHeadstage::getDataStream (int index) const
{
    if (index < 0 || index > 1) index = 0;
    return 2*dataStream+index;
}

bool RHDHeadstage::isPlugged() const
{
    return (numStreams > 0);
}

/***********************************/
/* Below is code for impedance measurements */

RHDImpedanceMeasure::RHDImpedanceMeasure(RHD2000Thread* b) : Thread(""), data(nullptr), board(b)
{
	// to perform electrode impedance measurements at very low frequencies.
	const int maxNumBlocks = 120;
	int numStreams = MAX_NUM_DATA_STREAMS;
	allocateDoubleArray3D(amplifierPreFilter, numStreams, 32, SAMPLES_PER_DATA_BLOCK * maxNumBlocks);
}

RHDImpedanceMeasure::~RHDImpedanceMeasure()
{
	stopThreadSafely();
}

void RHDImpedanceMeasure::stopThreadSafely()
{
	if (isThreadRunning())
	{
		CoreServices::sendStatusMessage("Impedance measure in progress. Stopping it.");
		if (!stopThread(3000)) //wait three seconds max for it to exit gracefully
		{
			std::cerr << "ERROR: Impedance measurement thread did not exit. Force killed it. This might led to crashes." << std::endl;
		}
	}
}

void RHDImpedanceMeasure::waitSafely()
{
	if (!waitForThreadToExit(120000)) //two minutes should be enough for completing a scan
	{
		CoreServices::sendStatusMessage("Impedance measurement took too much. Aborting.");
		if (!stopThread(3000)) //wait three seconds max for it to exit gracefully
		{
			std::cerr << "ERROR: Impedance measurement thread did not exit. Force killed it. This might led to crashes." << std::endl;
		}
	}
}

void RHDImpedanceMeasure::prepareData(ImpedanceData* d)
{
	data = d;
}


// Update electrode impedance measurement frequency, after checking that
// requested test frequency lies within acceptable ranges based on the
// amplifier bandwidth and the sampling rate.  See impedancefreqdialog.cpp
// for more information.
float RHDImpedanceMeasure::updateImpedanceFrequency(float desiredImpedanceFreq, bool& impedanceFreqValid)
{
	int impedancePeriod;
	double lowerBandwidthLimit, upperBandwidthLimit;
	float actualImpedanceFreq;

	upperBandwidthLimit = board->actualUpperBandwidth / 1.5;
	lowerBandwidthLimit = board->actualLowerBandwidth * 1.5;
	if (board->dspEnabled)
	{
		if (board->actualDspCutoffFreq > board->actualLowerBandwidth)
		{
			lowerBandwidthLimit = board->actualDspCutoffFreq * 1.5;
		}
	}

	if (desiredImpedanceFreq > 0.0)
	{
		impedancePeriod = (board->boardSampleRate / desiredImpedanceFreq);
		if (impedancePeriod >= 4 && impedancePeriod <= 1024 &&
			desiredImpedanceFreq >= lowerBandwidthLimit &&
			desiredImpedanceFreq <= upperBandwidthLimit)
		{
			actualImpedanceFreq = board->boardSampleRate / impedancePeriod;
			impedanceFreqValid = true;
		}
		else
		{
			actualImpedanceFreq = 0.0;
			impedanceFreqValid = false;
		}
	}
	else
	{
		actualImpedanceFreq = 0.0;
		impedanceFreqValid = false;
	}

	return actualImpedanceFreq;
}


// Reads numBlocks blocks of raw USB data stored in a queue of Rhd2000DataBlock
// objects, loads this data into this SignalProcessor object, scaling the raw
// data to generate waveforms with units of volts or microvolts.
int RHDImpedanceMeasure::loadAmplifierData(queue<Rhd2000DataBlockUsb3>& dataQueue,
	int numBlocks, int numDataStreams)
{

	int block, t, channel, stream;
	int indexAmp = 0;
    /*
	int indexAux = 0;
	int indexSupply = 0;
	int indexAdc = 0;
	int indexDig = 0;
	int numWordsWritten = 0;

	int bufferIndex;
	int16 tempQint16;
	uint16 tempQuint16;
	int32 tempQint32;

	bool triggerFound = false;
	const double AnalogTriggerThreshold = 1.65;
     */


	for (block = 0; block < numBlocks; ++block)
	{

		// Load and scale RHD2000 amplifier waveforms
		// (sampled at amplifier sampling rate)
		for (t = 0; t < SAMPLES_PER_DATA_BLOCK; ++t)
		{
			for (channel = 0; channel < 32; ++channel)
			{
				for (stream = 0; stream < numDataStreams; ++stream)
				{
					const Rhd2000DataBlockUsb3& block = dataQueue.front();
					// Amplifier waveform units = microvolts
					amplifierPreFilter[stream][channel][indexAmp] = 0.195 *
						(block.amplifierDataFast[block.fastIndex(stream,channel,t)] - 32768);
				}
			}
			++indexAmp;
		}
		// We are done with this Rhd2000DataBlock object; remove it from dataQueue
		dataQueue.pop();
	}

	return 0;
}

#define PI  3.14159265359
#define TWO_PI  6.28318530718
#define DEGREES_TO_RADIANS  0.0174532925199
#define RADIANS_TO_DEGREES  57.2957795132

// Return the magnitude and phase (in degrees) of a selected frequency component (in Hz)
// for a selected amplifier channel on the selected USB data stream.
void RHDImpedanceMeasure::measureComplexAmplitude(std::vector<std::vector<std::vector<double>>>& measuredMagnitude,
	std::vector<std::vector<std::vector<double>>>& measuredPhase,
	int capIndex, int stream, int chipChannel, int numBlocks,
	double sampleRate, double frequency, int numPeriods)
{
	int period = (sampleRate / frequency);
	int startIndex = 0;
	int endIndex = startIndex + numPeriods * period - 1;

	// Move the measurement window to the end of the waveform to ignore start-up transient.
	while (endIndex < SAMPLES_PER_DATA_BLOCK * numBlocks - period)
	{
		startIndex += period;
		endIndex += period;
	}

	double iComponent, qComponent;

	// Measure real (iComponent) and imaginary (qComponent) amplitude of frequency component.
	amplitudeOfFreqComponent(iComponent, qComponent, amplifierPreFilter[stream][chipChannel],
		startIndex, endIndex, sampleRate, frequency);
	// Calculate magnitude and phase from real (I) and imaginary (Q) components.
	measuredMagnitude[stream][chipChannel][capIndex] =
		sqrt(iComponent * iComponent + qComponent * qComponent);
	measuredPhase[stream][chipChannel][capIndex] =
		RADIANS_TO_DEGREES *atan2(qComponent, iComponent);
}

// Returns the real and imaginary amplitudes of a selected frequency component in the vector
// data, between a start index and end index.
void RHDImpedanceMeasure::amplitudeOfFreqComponent(double& realComponent, double& imagComponent,
	const std::vector<double>& data, int startIndex,
	int endIndex, double sampleRate, double frequency)
{
	int length = endIndex - startIndex + 1;
	const double k = TWO_PI * frequency / sampleRate;  // precalculate for speed

	// Perform correlation with sine and cosine waveforms.
	double meanI = 0.0;
	double meanQ = 0.0;
	for (int t = startIndex; t <= endIndex; ++t)
	{
		meanI += data.at(t) * cos(k * t);
		meanQ += data.at(t) * -1.0 * sin(k * t);
	}
	meanI /= (double)length;
	meanQ /= (double)length;

	realComponent = 2.0 * meanI;
	imagComponent = 2.0 * meanQ;
}



// Given a measured complex impedance that is the result of an electrode impedance in parallel
// with a parasitic capacitance (i.e., due to the amplifier input capacitance and other
// capacitances associated with the chip bondpads), this function factors out the effect of the
// parasitic capacitance to return the acutal electrode impedance.
void RHDImpedanceMeasure::factorOutParallelCapacitance(double& impedanceMagnitude, double& impedancePhase,
	double frequency, double parasiticCapacitance)
{
	// First, convert from polar coordinates to rectangular coordinates.
	double measuredR = impedanceMagnitude * cos(DEGREES_TO_RADIANS * impedancePhase);
	double measuredX = impedanceMagnitude * sin(DEGREES_TO_RADIANS * impedancePhase);

	double capTerm = TWO_PI * frequency * parasiticCapacitance;
	double xTerm = capTerm * (measuredR * measuredR + measuredX * measuredX);
	double denominator = capTerm * xTerm + 2 * capTerm * measuredX + 1;
	double trueR = measuredR / denominator;
	double trueX = (measuredX + xTerm) / denominator;

	// Now, convert from rectangular coordinates back to polar coordinates.
	impedanceMagnitude = sqrt(trueR * trueR + trueX * trueX);
	impedancePhase = RADIANS_TO_DEGREES * atan2(trueX, trueR);
}

// This is a purely empirical function to correct observed errors in the real component
// of measured electrode impedances at sampling rates below 15 kS/s.  At low sampling rates,
// it is difficult to approximate a smooth sine wave with the on-chip voltage DAC and 10 kHz
// 2-pole lowpass filter.  This function attempts to somewhat correct for this, but a better
// solution is to always run impedance measurements at 20 kS/s, where they seem to be most
// accurate.
void RHDImpedanceMeasure::empiricalResistanceCorrection(double& impedanceMagnitude, double& impedancePhase,
	double boardSampleRate)
{
	// First, convert from polar coordinates to rectangular coordinates.
	double impedanceR = impedanceMagnitude * cos(DEGREES_TO_RADIANS * impedancePhase);
	double impedanceX = impedanceMagnitude * sin(DEGREES_TO_RADIANS * impedancePhase);

	// Emprically derived correction factor (i.e., no physical basis for this equation).
	impedanceR /= 10.0 * exp(-boardSampleRate / 2500.0) * cos(TWO_PI * boardSampleRate / 15000.0) + 1.0;

	// Now, convert from rectangular coordinates back to polar coordinates.
	impedanceMagnitude = sqrt(impedanceR * impedanceR + impedanceX * impedanceX);
	impedancePhase = RADIANS_TO_DEGREES * atan2(impedanceX, impedanceR);
}

void RHDImpedanceMeasure::run()
{
	RHD2000Editor* ed;
	ed = (RHD2000Editor*)board->sn->editor.get();
	if (data == nullptr)
		return;
	runImpedanceMeasurement();
	restoreFPGA();
	ed->triggerAsyncUpdate();
	data = nullptr;
}

#define CHECK_EXIT if (threadShouldExit()) return

void RHDImpedanceMeasure::runImpedanceMeasurement()
{
	int commandSequenceLength, stream, channel, capRange;
	double cSeries;
	vector<int> commandList;
	//int triggerIndex;                       // dummy reference variable; not used
	queue<Rhd2000DataBlockUsb3> bufferQueue;    // dummy reference variable; not used
	int numdataStreams = board->evalBoard->getNumEnabledDataStreams();

	bool rhd2164ChipPresent = false;
	int chOffset;

	Array<int> enabledStreams;
	for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream)
	{
		CHECK_EXIT;
		if (board->evalBoard->getStreamEnabled(stream))
		{
			enabledStreams.add(stream);
		}

		if (board->chipId[stream] == CHIP_ID_RHD2164_B)
		{
			rhd2164ChipPresent = true;
		}
	}

	bool validImpedanceFreq;
	float actualImpedanceFreq = updateImpedanceFrequency(1000.0, validImpedanceFreq);
	if (!validImpedanceFreq)
	{
		return;
	}
	// Create a command list for the AuxCmd1 slot.
	commandSequenceLength = board->chipRegisters.createCommandListZcheckDac(commandList, actualImpedanceFreq, 128.0);
	CHECK_EXIT;
	board->evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd1,
		0, commandSequenceLength - 1);
	if (board->fastTTLSettleEnabled)
	{
		board->evalBoard->enableExternalFastSettle(false);
	}
	CHECK_EXIT;
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH,
		Rhd2000EvalBoardUsb3::AuxCmd1, 1);

	// Select number of periods to measure impedance over
	int numPeriods = (0.020 * actualImpedanceFreq); // Test each channel for at least 20 msec...
	if (numPeriods < 5) numPeriods = 5; // ...but always measure across no fewer than 5 complete periods
	double period = board->boardSampleRate / actualImpedanceFreq;
	int numBlocks = ceil((numPeriods + 2.0) * period / 60.0);  // + 2 periods to give time to settle initially
	if (numBlocks < 2) numBlocks = 2;   // need first block for command to switch channels to take effect.

	CHECK_EXIT;
	board->actualDspCutoffFreq = board->chipRegisters.setDspCutoffFreq(board->desiredDspCutoffFreq);
	board->actualLowerBandwidth = board->chipRegisters.setLowerBandwidth(board->desiredLowerBandwidth);
	board->actualUpperBandwidth = board->chipRegisters.setUpperBandwidth(board->desiredUpperBandwidth);
	board->chipRegisters.enableDsp(board->dspEnabled);
	board->chipRegisters.enableZcheck(true);
	commandSequenceLength = board->chipRegisters.createCommandListRegisterConfig(commandList, false);
	CHECK_EXIT;
	// Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
	board->evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd3, 0, commandSequenceLength - 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd3, 3);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd3, 3);

	CHECK_EXIT;
	board->evalBoard->setContinuousRunMode(false);
	board->evalBoard->setMaxTimeStep(SAMPLES_PER_DATA_BLOCK * numBlocks);

	// Create matrices of doubles of size (numStreams x 32 x 3) to store complex amplitudes
	// of all amplifier channels (32 on each data stream) at three different Cseries values.
	std::vector<std::vector<std::vector<double>>>  measuredMagnitude;
	std::vector<std::vector<std::vector<double>>>  measuredPhase;

	measuredMagnitude.resize(board->evalBoard->getNumEnabledDataStreams());
	measuredPhase.resize(board->evalBoard->getNumEnabledDataStreams());
	for (int i = 0; i < board->evalBoard->getNumEnabledDataStreams(); ++i)
	{
		measuredMagnitude[i].resize(32);
		measuredPhase[i].resize(32);
		for (int j = 0; j < 32; ++j)
		{
			measuredMagnitude[i][j].resize(3);
			measuredPhase[i][j].resize(3);
		}
	}



	double distance, minDistance, current, Cseries;
	double impedanceMagnitude, impedancePhase;

	const double bestAmplitude = 250.0;  // we favor voltage readings that are closest to 250 uV: not too large,
	// and not too small.
	const double dacVoltageAmplitude = 128 * (1.225 / 256);  // this assumes the DAC amplitude was set to 128
	const double parasiticCapacitance = 14.0e-12;  // 14 pF: an estimate of on-chip parasitic capacitance,
	// including 10 pF of amplifier input capacitance.
	double relativeFreq = actualImpedanceFreq / board->boardSampleRate;

	int bestAmplitudeIndex;

	// We execute three complete electrode impedance measurements: one each with
	// Cseries set to 0.1 pF, 1 pF, and 10 pF.  Then we select the best measurement
	// for each channel so that we achieve a wide impedance measurement range.
	for (capRange = 0; capRange < 3; ++capRange)
	{

		switch (capRange)
		{
		case 0:
			board->chipRegisters.setZcheckScale(Rhd2000RegistersUsb3::ZcheckCs100fF);
			cSeries = 0.1e-12;
			cout << "setting capacitance to 0.1pF" << endl;
			break;
		case 1:
			board->chipRegisters.setZcheckScale(Rhd2000RegistersUsb3::ZcheckCs1pF);
			cSeries = 1.0e-12;
			cout << "setting capacitance to 1pF" << endl;
			break;
		case 2:
			board->chipRegisters.setZcheckScale(Rhd2000RegistersUsb3::ZcheckCs10pF);
			cSeries = 10.0e-12;
			cout << "setting capacitance to 10pF" << endl;
			break;
		}

		// Check all 32 channels across all active data streams.
		for (channel = 0; channel < 32; ++channel)
		{
			CHECK_EXIT;
			cout << "running impedance on channel " << channel << endl;

			board->chipRegisters.setZcheckChannel(channel);
			commandSequenceLength =
				board->chipRegisters.createCommandListRegisterConfig(commandList, false);
			// Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
			board->evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 3);

			board->evalBoard->run();
			while (board->evalBoard->isRunning())
			{

			}
			queue<Rhd2000DataBlockUsb3> dataQueue;
			board->evalBoard->readDataBlocks(numBlocks, dataQueue);
			loadAmplifierData(dataQueue, numBlocks, numdataStreams);
			for (stream = 0; stream < numdataStreams; ++stream)
			{
				if (board->chipId[stream] != CHIP_ID_RHD2164_B)
				{
					measureComplexAmplitude(measuredMagnitude, measuredPhase,
						capRange, stream, channel, numBlocks, board->boardSampleRate,
						actualImpedanceFreq, numPeriods);
				}
			}

			// If an RHD2164 chip is plugged in, we have to set the Zcheck select register to channels 32-63
			// and repeat the previous steps.
			if (rhd2164ChipPresent)
			{
				CHECK_EXIT;
				board->chipRegisters.setZcheckChannel(channel + 32); // address channels 32-63
				commandSequenceLength =
					board->chipRegisters.createCommandListRegisterConfig(commandList, false);
				// Upload version with no ADC calibration to AuxCmd3 RAM Bank 1.
				board->evalBoard->uploadCommandList(commandList, Rhd2000EvalBoardUsb3::AuxCmd3, 3);

				board->evalBoard->run();
				while (board->evalBoard->isRunning())
				{

				}
				board->evalBoard->readDataBlocks(numBlocks, dataQueue);
				loadAmplifierData(dataQueue, numBlocks, numdataStreams);

				for (stream = 0; stream < board->evalBoard->getNumEnabledDataStreams(); ++stream)
				{
					if (board->chipId[stream] == CHIP_ID_RHD2164_B)
					{
						measureComplexAmplitude(measuredMagnitude, measuredPhase,
							capRange, stream, channel, numBlocks, board->boardSampleRate,
							actualImpedanceFreq, numPeriods);
					}
				}
			}
		}
	}

	data->streams.clear();
	data->channels.clear();
	data->magnitudes.clear();
	data->phases.clear();

	for (stream = 0; stream < board->evalBoard->getNumEnabledDataStreams(); ++stream)
	{
		if ((board->chipId[stream] == CHIP_ID_RHD2132) && (board->numChannelsPerDataStream[stream] == 16))
			chOffset = RHD2132_16CH_OFFSET;
		else
			chOffset = 0;

		for (channel = 0; channel < board->numChannelsPerDataStream[stream]; ++channel)
		{
			if (1)
			{
				minDistance = 9.9e99;  // ridiculously large number
				for (capRange = 0; capRange < 3; ++capRange)
				{
					// Find the measured amplitude that is closest to bestAmplitude on a logarithmic scale
					distance = abs(log(measuredMagnitude[stream][channel+chOffset][capRange] / bestAmplitude));
					if (distance < minDistance)
					{
						bestAmplitudeIndex = capRange;
						minDistance = distance;
					}
				}
				switch (bestAmplitudeIndex)
				{
				case 0:
					Cseries = 0.1e-12;
					break;
				case 1:
					Cseries = 1.0e-12;
					break;
				case 2:
					Cseries = 10.0e-12;
					break;
				}

				// Calculate current amplitude produced by on-chip voltage DAC
				current = TWO_PI * actualImpedanceFreq * dacVoltageAmplitude * Cseries;

				// Calculate impedance magnitude from calculated current and measured voltage.
				impedanceMagnitude = 1.0e-6 * (measuredMagnitude[stream][channel + chOffset][bestAmplitudeIndex] / current) *
					(18.0 * relativeFreq * relativeFreq + 1.0);

				// Calculate impedance phase, with small correction factor accounting for the
				// 3-command SPI pipeline delay.
				impedancePhase = measuredPhase[stream][channel + chOffset][bestAmplitudeIndex] + (360.0 * (3.0 / period));

				// Factor out on-chip parasitic capacitance from impedance measurement.
				factorOutParallelCapacitance(impedanceMagnitude, impedancePhase, actualImpedanceFreq,
					parasiticCapacitance);

				// Perform empirical resistance correction to improve accuarcy at sample rates below
				// 15 kS/s.
				empiricalResistanceCorrection(impedanceMagnitude, impedancePhase,
					board->boardSampleRate);

				data->streams.add(enabledStreams[stream]);
				data->channels.add(channel + chOffset);
				data->magnitudes.add(impedanceMagnitude);
				data->phases.add(impedancePhase);

				if (impedanceMagnitude > 1000000)
					cout << "stream " << stream << " channel " << 1 + channel << " magnitude: " << String(impedanceMagnitude / 1e6, 2) << " MOhm , phase : " << impedancePhase << endl;
				else
					cout << "stream " << stream << " channel " << 1 + channel << " magnitude: " << String(impedanceMagnitude / 1e3, 2) << " kOhm , phase : " << impedancePhase << endl;

			}
		}
	}
	data->valid = true;
	
}

void RHDImpedanceMeasure::restoreFPGA()
{
	board->evalBoard->setContinuousRunMode(false);
	board->evalBoard->setMaxTimeStep(0);
	board->evalBoard->flush();

	// Switch back to flatline
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd1, 0);
	board->evalBoard->selectAuxCommandLength(Rhd2000EvalBoardUsb3::AuxCmd1, 0, 1);

	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortA, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortB, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortC, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortD, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortE, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortF, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortG, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);
	board->evalBoard->selectAuxCommandBank(Rhd2000EvalBoardUsb3::PortH, Rhd2000EvalBoardUsb3::AuxCmd3,
		board->fastSettleEnabled ? 2 : 1);

	if (board->fastTTLSettleEnabled)
	{
		board->evalBoard->enableExternalFastSettle(true);
	}
}
