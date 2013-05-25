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

RHD2000Thread::RHD2000Thread(SourceNode* sn) : DataThread(sn), isTransmitting(false),
    fastSettleEnabled(false)
{
    evalBoard = new Rhd2000EvalBoard;
    dataBlock = new Rhd2000DataBlock(1);

    // Open Opal Kelly XEM6010 board.
    int return_code = evalBoard->open();

    if (return_code == 1)
    {
        deviceFound = true;
    }
    else
    {
        deviceFound = false;
    }

    if (deviceFound)
    {

        numChannelsPerDataStream.insertMultiple(0,0,4);

        // initialize data buffer for 32 channels + 3 aux.
        dataBuffer = new DataBuffer(35*1, 10000);

        initializeBoard();

		// manually set cable delay for now
		//2 for one cable 
		//3 for 2 cables daisy-chained
		evalBoard->setCableDelay(Rhd2000EvalBoard::PortA, 3);
        evalBoard->setCableDelay(Rhd2000EvalBoard::PortB, 3);

        enableHeadstage(0,true); // start off with one headstage
		enableHeadstage(1,true); // start off with one headstage



        // automatically find connected headstages -- needs debugging
       // scanPorts();

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

    deleteAndZero(dataBlock);

}

void RHD2000Thread::initializeBoard()
{
    string bitfilename;
    bitfilename = "rhd2000.bit";
    evalBoard->uploadFpgaBitfile(bitfilename);

    // Initialize board.
    evalBoard->initialize();

    // Select per-channel amplifier sampling rate.
    evalBoard->setSampleRate(Rhd2000EvalBoard::SampleRate30000Hz);

    // // Select RAM Bank 0 for AuxCmd3 initially, so the ADC is calibrated.
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3, 0);
    // evalBoard->flush(); // flush in case it crashed with data remaining

    // // Since our longest command sequence is 60 commands, run the SPI interface for
    // // 60 samples
    // evalBoard->setMaxTimeStep(60);
    // evalBoard->setContinuousRunMode(false);

    // // Start SPI interface
    // evalBoard->run();

    // // Wait for the 60-sample run to complete
    // while (evalBoard->isRunning())
    // {
    //     ;
    // }

    // // Read the resulting single data block from the USB interface. We don't
    // // need to do anything with this, since it was only used for ADC calibration
    // Rhd2000DataBlock* dataBlock = new Rhd2000DataBlock(evalBoard->getNumEnabledDataStreams());
    // evalBoard->readDataBlock(dataBlock);

    // // Now that ADC calibration has been performed, we switch to the command sequence
    // // that does not execute ADC calibration.
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortA, Rhd2000EvalBoard::AuxCmd3,
    //                                 fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortB, Rhd2000EvalBoard::AuxCmd3,
    //                                 fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortC, Rhd2000EvalBoard::AuxCmd3,
    //                                 fastSettleEnabled ? 2 : 1);
    // evalBoard->selectAuxCommandBank(Rhd2000EvalBoard::PortD, Rhd2000EvalBoard::AuxCmd3,
    //                                 fastSettleEnabled ? 2 : 1);


    // // Set default configuration for all eight DACs on interface board.
    // evalBoard->enableDac(0, false);
    // evalBoard->enableDac(1, false);
    // evalBoard->enableDac(2, false);
    // evalBoard->enableDac(3, false);
    // evalBoard->enableDac(4, false);
    // evalBoard->enableDac(5, false);
    // evalBoard->enableDac(6, false);
    // evalBoard->enableDac(7, false);
    // evalBoard->selectDacDataStream(0, 0);
    // evalBoard->selectDacDataStream(1, 0);
    // evalBoard->selectDacDataStream(2, 0);
    // evalBoard->selectDacDataStream(3, 0);
    // evalBoard->selectDacDataStream(4, 0);
    // evalBoard->selectDacDataStream(5, 0);
    // evalBoard->selectDacDataStream(6, 0);
    // evalBoard->selectDacDataStream(7, 0);
    // evalBoard->selectDacDataChannel(0, 0);
    // evalBoard->selectDacDataChannel(1, 1);
    // evalBoard->selectDacDataChannel(2, 0);
    // evalBoard->selectDacDataChannel(3, 0);
    // evalBoard->selectDacDataChannel(4, 0);
    // evalBoard->selectDacDataChannel(5, 0);
    // evalBoard->selectDacDataChannel(6, 0);
    // evalBoard->selectDacDataChannel(7, 0);
    // evalBoard->setDacManual(Rhd2000EvalBoard::DacManual1, 32768);
    // evalBoard->setDacManual(Rhd2000EvalBoard::DacManual2, 32768);
    // evalBoard->setDacGain(0);
    // evalBoard->setAudioNoiseSuppress(0);


    // Set up an RHD2000 register object using this sample rate to optimize MUX-related
    // register settings.
    
    std::cout << "Rhd sample rate : " << evalBoard->getSampleRate() << std::endl;
	chipRegisters = new Rhd2000Registers(evalBoard->getSampleRate());



    // Before generating register configuration command sequences, set amplifier
    // bandwidth paramters.
    double dspCutoffFreq;
    dspCutoffFreq = chipRegisters->setDspCutoffFreq(10.0);
    cout << "Actual DSP cutoff frequency: " << dspCutoffFreq << " Hz" << endl;

    chipRegisters->setLowerBandwidth(1.0);
    chipRegisters->setUpperBandwidth(7500.0);



			// turn on aux inputs
		chipRegisters->enableAux1(true);
		chipRegisters->enableAux2(true);
	   chipRegisters->enableAux3(true);

    // Let's turn one LED on to indicate that the program is running.
    int ledArray[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    evalBoard->setLedDisplay(ledArray);


}

void RHD2000Thread::scanPorts()
{
    // Scan SPI ports

    int delay, stream, id;

    // assume we only have 32-channel headstages, for the sake of
    // simplicity; this will have to be changed once 64-channel
    // headstages are an option
    evalBoard->setDataSource(0, Rhd2000EvalBoard::PortA1);
    evalBoard->setDataSource(1, Rhd2000EvalBoard::PortB1);
    evalBoard->setDataSource(2, Rhd2000EvalBoard::PortC1);
    evalBoard->setDataSource(3, Rhd2000EvalBoard::PortD1);

    evalBoard->enableDataStream(0, true);
    evalBoard->enableDataStream(1, true);
    evalBoard->enableDataStream(2, true);
    evalBoard->enableDataStream(3, true);

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
    sumGoodDelays.insertMultiple(0,0,4);

    Array<int> indexFirstGoodDelay;
    indexFirstGoodDelay.insertMultiple(0,-1,4);

    Array<int> indexSecondGoodDelay;
    indexSecondGoodDelay.insertMultiple(0,-1,4);

    Array<int> chipId;
    chipId.insertMultiple(0,0,4);

    Array<int> optimumDelay;
    optimumDelay.insertMultiple(0,0,4);

    // Run SPI command sequence at all 16 possible FPGA MISO delay settings
    // to find optimum delay for each SPI interface cable.
    for (delay = 0; delay < 16; ++delay)
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
        for (stream = 0; stream < 4; ++stream)
        {
            id = deviceId(dataBlock, stream);
            std::cout << "Device ID found: " << id << std::endl;
            if (id > 0)
            {
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

    // Now, disable data streams where we did not find chips present.
    for (stream = 0; stream < 4; ++stream)
    {
        if (chipId[stream] > 0)
        {
            enableHeadstage(stream, true);
        }
        else
        {
            enableHeadstage(stream, false);
        }
    }

    // Set cable delay settings that yield good communication with each
    // RHD2000 chip.
    for (stream = 0; stream < 4; ++stream)
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


}

int RHD2000Thread::deviceId(Rhd2000DataBlock* dataBlock, int stream)
{
    bool intanChipPresent;

    // First, check ROM registers 32-36 to verify that they hold 'INTAN'.
    // This is just used to verify that we are getting good data over the SPI
    // communication channel.
    intanChipPresent = ((char) dataBlock->auxiliaryData[stream][2][32] == 'I' &&
                        (char) dataBlock->auxiliaryData[stream][2][33] == 'N' &&
                        (char) dataBlock->auxiliaryData[stream][2][34] == 'T' &&
                        (char) dataBlock->auxiliaryData[stream][2][35] == 'A' &&
                        (char) dataBlock->auxiliaryData[stream][2][36] == 'N');

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

int RHD2000Thread::getNumChannels()
{

    numChannels = 0;

    for (int i = 0; i < numChannelsPerDataStream.size(); i++)
    {
    
		numChannels += numChannelsPerDataStream[i];
		
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
	numChannels += 6;

    if (numChannels > 0)
        return numChannels;
    else
        return 1; // to prevent crashing with 0 channels
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

double RHD2000Thread::setUpperBandwidth(double desiredUpperBandwidth)
{
    return chipRegisters->setUpperBandwidth(desiredUpperBandwidth);
}

double RHD2000Thread::setLowerBandwidth(double desiredLowerBandwidth)
{
    return chipRegisters->setLowerBandwidth(desiredLowerBandwidth);
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

    std::cout << "Enabled channels: " << numChannelsPerDataStream[0] <<
              " " << numChannelsPerDataStream[1] <<
              " " << numChannelsPerDataStream[2] <<
              " " << numChannelsPerDataStream[3] << std::endl;

    std::cout << "Enabled data streams: " << evalBoard->getNumEnabledDataStreams() << std::endl;

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

void RHD2000Thread::setSampleRate(int sampleRateIndex)
{
 

    int numUsbBlocksToRead=0; // placeholder - make this change the number of blocks that are read in  RHD2000Thread::updateBuffer()
    Rhd2000EvalBoard::AmplifierSampleRate sampleRate; // just for local use


    switch (sampleRateIndex) {
    case 0:
        sampleRate = Rhd2000EvalBoard::SampleRate1000Hz;
        numUsbBlocksToRead = 1;
        break;
    case 1:
        sampleRate = Rhd2000EvalBoard::SampleRate1250Hz;
        numUsbBlocksToRead = 1;
        break;
    case 2:
        sampleRate = Rhd2000EvalBoard::SampleRate1500Hz;
        numUsbBlocksToRead = 1;
        break;
    case 3:
        sampleRate = Rhd2000EvalBoard::SampleRate2000Hz;
        numUsbBlocksToRead = 1;
        break;
    case 4:
        sampleRate = Rhd2000EvalBoard::SampleRate2500Hz;
        numUsbBlocksToRead = 1;
        break;
    case 5:
        sampleRate = Rhd2000EvalBoard::SampleRate3000Hz;
        numUsbBlocksToRead = 2;
        break;
    case 6:
        sampleRate = Rhd2000EvalBoard::SampleRate3333Hz;
        numUsbBlocksToRead = 2;
        break;
    case 7:
        sampleRate = Rhd2000EvalBoard::SampleRate4000Hz;
        numUsbBlocksToRead = 2;
        break;
    case 8:
        sampleRate = Rhd2000EvalBoard::SampleRate5000Hz;
        numUsbBlocksToRead = 3;
        break;
    case 9:
        sampleRate = Rhd2000EvalBoard::SampleRate6250Hz;
        numUsbBlocksToRead = 3;
        break;
    case 10:
        sampleRate = Rhd2000EvalBoard::SampleRate8000Hz;
        numUsbBlocksToRead = 4;
        break;
    case 11:
        sampleRate = Rhd2000EvalBoard::SampleRate10000Hz;
        numUsbBlocksToRead = 6;
        break;
    case 12:
        sampleRate = Rhd2000EvalBoard::SampleRate12500Hz;
        numUsbBlocksToRead = 7;
        break;
    case 13:
        sampleRate = Rhd2000EvalBoard::SampleRate15000Hz;
        numUsbBlocksToRead = 8;
        break;
    case 14:
        sampleRate = Rhd2000EvalBoard::SampleRate20000Hz;
        numUsbBlocksToRead = 12;
        break;
    case 15:
        sampleRate = Rhd2000EvalBoard::SampleRate25000Hz;
        numUsbBlocksToRead = 14;
        break;
    case 16:
        sampleRate = Rhd2000EvalBoard::SampleRate30000Hz;
        numUsbBlocksToRead = 16;
        break;
    }


    // Select per-channel amplifier sampling rate.
    evalBoard->setSampleRate(sampleRate);

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
    evalBoard->setContinuousRunMode(true);

    std::cout << "Starting acquisition." << std::endl;
    evalBoard->run();

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

    evalBoard->setContinuousRunMode(false);
    evalBoard->setMaxTimeStep(0);
    std::cout << "Flushing FIFO." << std::endl;
    evalBoard->flush();

    cout << "Number of 16-bit words in FIFO: " << evalBoard->numWordsInFifo() << endl;

    std::cout << "Stopped eval board." << std::endl;


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
	

    for (int n = 0; n < 10; n++)
    {
        if (evalBoard->numWordsInFifo() >= blockSize)
        {
            return_code = evalBoard->readDataBlock(dataBlock);

            for (int samp = 0; samp < dataBlock->getSamplesPerDataBlock(); samp++)
            {
                int streamNumber = -1;
                int channel = -1;

                for (int dataStream = 0; dataStream < numChannelsPerDataStream.size(); dataStream++)
                {
                    if (numChannelsPerDataStream[dataStream] > 0)
                    {
                        streamNumber++;

                        for (int chan = 0; chan < numChannelsPerDataStream[dataStream]; chan++)
                        {
                         //   std::cout << "reading sample stream" << streamNumber << " chan " << chan << " sample "<< samp << std::endl;
							channel++;

                            int value = dataBlock->amplifierData[streamNumber][chan][samp];

                            thisSample[channel] = float(value-32768)*0.195f;
                        }
					
						
						if (samp % 4 == 1) { // every 4th sample should have auxiliary input data
						
							channel++;
							thisSample[channel] = 0.0374 *
							float(dataBlock->auxiliaryData[dataStream][1][samp+0]);
							auxBuffer[channel]=thisSample[channel];

							channel++;
							thisSample[channel] = 0.0374 *
							float(dataBlock->auxiliaryData[dataStream][1][samp+1]);
							auxBuffer[channel]=thisSample[channel];

						
							channel++;
							thisSample[channel] = 0.0374 *
							float(dataBlock->auxiliaryData[dataStream][1][samp+2]);
							auxBuffer[channel]=thisSample[channel];

						} else{ // repeat last values from buffer
							channel++;
							thisSample[channel] = auxBuffer[channel];
							channel++;
							thisSample[channel] = auxBuffer[channel];
							channel++;
							thisSample[channel] = auxBuffer[channel];
						}
						
					}

              

					// std::cout << channel << std::endl;

					timestamp = dataBlock->timeStamp[samp];
					timestamp = timestamp;
					eventCode = dataBlock->ttlIn[samp];

					dataBuffer->addToBuffer(thisSample, &timestamp, &eventCode, 1);
				  }
            }

        }
    }




    return true;

}