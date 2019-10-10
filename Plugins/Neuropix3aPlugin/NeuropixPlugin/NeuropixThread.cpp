/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2017 Allen Institute for Brain Science and Open Ephys

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

#include "NeuropixThread.h"
#include "NeuropixEditor.h"

using namespace Neuropix;

DataThread* NeuropixThread::createDataThread(SourceNode *sn)
{
	return new NeuropixThread(sn);
}

GenericEditor* NeuropixThread::createEditor(SourceNode* sn)
{
    return new NeuropixEditor(sn, this, true);
}

NeuropixThread::NeuropixThread(SourceNode* sn) : DataThread(sn), baseStationAvailable(false)
{

    gains.add(50);
    gains.add(125);
    gains.add(250);
    gains.add(500);
    gains.add(1000);
    gains.add(1500);
    gains.add(2000);
    gains.add(2500);

    refs.add(0);
    refs.add(37);
    refs.add(76);
    refs.add(113);
    refs.add(152);
    refs.add(189);
    refs.add(228);
    refs.add(265);
    refs.add(304);
    refs.add(341);
    refs.add(380);

    counter = 0;
    timestampAp = 0;
	timestampLfp = 0;
    eventCode = 0;
    maxCounter = 0;

    openConnection(); //gets totalChans

	// channel selections:
	// Options 1 & 2 -- fixed 384 channels
	// Option 3 -- select 384 of 960 shank electrodes
	// Option 4 -- select 276 of 966 shank electrodes

	for (int i = 0; i < totalChans; i++)
	{
		lfpGains.add(0);
		apGains.add(0);
		channelMap.add(i);
		outputOn.add(true);
	}

	sourceBuffers.add(new DataBuffer(totalChans, 10000));  // AP band buffer
	sourceBuffers.add(new DataBuffer(totalChans, 10000));  // LFP band buffer

}

NeuropixThread::~NeuropixThread()
{
    closeConnection();
}

void NeuropixThread::openConnection()
{
    OpenErrorCode errorCode = neuropix.neuropix_open(); // establishes a data connection with the basestation

    if (errorCode == OPEN_SUCCESS)
    {
        std::cout << "Open success!" << std::endl;
    }
    else {
        CoreServices::sendStatusMessage("Failure with error code " + String(errorCode));
        std::cout << "Failure with error code " << String(errorCode) << std::endl;
        baseStationAvailable = false;
        return;
    }

    baseStationAvailable = true;
    internalTrigger = true;
    sendLfp = true;
    sendAp = true;
    recordToNpx = false;
    recordingNumber = 0;
	isRecording = false;

    // // GET SYSTEM INFO:
    ErrorCode error1 = neuropix.neuropix_getHardwareVersion(&hw_version);
    ConfigAccessErrorCode error2 = neuropix.neuropix_getBSVersion(bs_version);
    ConfigAccessErrorCode error3 = neuropix.neuropix_getBSRevision(bs_revision);
    vn = neuropix.neuropix_getAPIVersion();
    EepromErrorCode error4 = neuropix.neuropix_readId(asicId);

    std::cout << "  Hardware version number: " << hw_version.major << "." << hw_version.minor << std::endl;
    std::cout << "  Basestation version number: " << String(bs_version) << "." << String(bs_revision) << std::endl;
    std::cout << "  API version number: " << vn.major << "." << vn.minor << std::endl;
    std::cout << "  Asic info: " << String(asicId.probeType) << std::endl;

    // prepare probe for streaming data
    ErrorCode err1 = neuropix.neuropix_datamode(true);
    std::cout << "set datamode error code: " << err1 << std::endl;
    DigitalControlErrorCode err0 = neuropix.neuropix_mode(ASIC_RECORDING);
    std::cout << "set mode error code: " << err0 << std::endl;

    if (true)
    {
        DigitalControlErrorCode err3 = neuropix.neuropix_nrst(false);
        std::cout << "nrst 1 error code: " << err3 << std::endl;
        ErrorCode err4 = neuropix.neuropix_resetDatapath();
        std::cout << "reset datapath error code: " << err4 << std::endl;
        neuropix.neuropix_nrst(true);
    }
    
    // set default parameters
    getProbeOption();
    setAllApGains(3);
    setAllLfpGains(2);
    
    if (option >= 2)
    {
        for (int i = 0; i < totalChans; i++)
        {
            selectElectrode(i, 0, false);
        }
        selectElectrode(totalChans - 1, 0, true);
    }

    setAllReferences(0, 0);

	neuropix.neuropix_ledOff(true);
}

void NeuropixThread::startRecording()
{
	if (recordToNpx)
	{
		recordingNumber++;
		File rootFolder = CoreServices::RecordNode::getRecordingPath();
		File fullPath = rootFolder.getChildFile("recording" + String(recordingNumber) + ".npx");
		const std::string fname = fullPath.getFullPathName().toStdString();
		ErrorCode caec = neuropix.neuropix_startRecording(fname);
		std::cout << "NeuropixThread recording to file: " << fname << std::endl;

		isRecording = true;
	}
}

void NeuropixThread::stopRecording()
{
	if (recordToNpx)
	{
		ErrorCode caec = neuropix.neuropix_stopRecording();
		std::cout << "NeuropixThread stopping recording." << std::endl;

		isRecording = false;
	}
}

void NeuropixThread::closeConnection()
{
    neuropix.neuropix_close(); // closes the data and configuration link 
}

/** Returns true if the data source is connected, false otherwise.*/
bool NeuropixThread::foundInputSource()
{
    return baseStationAvailable;
}

void NeuropixThread::getInfo(String& hwVersion, String& bsVersion, String& apiVersion, String& asicInfo, String& serialNumber)
{
	hwVersion = String(hw_version.major) + "." + String(hw_version.minor);
	bsVersion = String(bs_version) + "." + String(bs_revision);
	apiVersion = String(vn.major) + "." + String(vn.minor);
	asicInfo = String(asicId.probeType+1);
	serialNumber = String(asicId.serialNumber);
}

int NeuropixThread::getProbeOption()
{
    //option = asicId.probeType;
    //asicId.probeType = option - 1;
    //neuropix.neuropix_writeId(asicId);
    option = neuropix.neuropix_getOption();

    if (option < 3)
    {
        numRefs = 10;
        totalChans = 384;
    }
    else
    {
        numRefs = 7;
        totalChans = 276;
    }

    return option + 1;
}


/** Initializes data transfer.*/
bool NeuropixThread::startAcquisition()
{

    // clear the internal buffer
	sourceBuffers[0]->clear();
	sourceBuffers[1]->clear();
    //dataBuffer->clear();
	//extraDataBuffers[0]->clear();
   // dataBuffer2->clear();

    // stop data stream
    if (true)
    {
        DigitalControlErrorCode err3 = neuropix.neuropix_nrst(false);
        std::cout << "nrst 1 error code: " << err3 << std::endl;

        // clear the buffer
        ErrorCode err4 = neuropix.neuropix_resetDatapath();
        std::cout << "reset datapath error code: " << err4 << std::endl;

		err3 = neuropix.neuropix_nrst(true);
		std::cout << "nrst 1 error code: " << err3 << std::endl;
    }

    counter = 0;
    timestampAp = 0;
	timestampLfp = 0;
    eventCode = 0;
    maxCounter = 0;
    
	neuropix.neuropix_setNeuralStart();

	startThread();
   
    return true;
}

void NeuropixThread::timerCallback()
{

    stopTimer();

    // start data stream
    
	startRecording();

    //DigitalControlErrorCode err5 = neuropix.neuropix_nrst(true);
    //std::cout << "nrst 2 error code: " << err5 << std::endl;

}


/** Stops data transfer.*/
bool NeuropixThread::stopAcquisition()
{

    if (isThreadRunning())
    {
        signalThreadShouldExit();
    }

	stopRecording();

    if (true)
    {
        // stop data stream
        DigitalControlErrorCode err3 = neuropix.neuropix_nrst(false);
        std::cout << "nrst 1 error code: " << err3 << std::endl;

        // clear the buffer
        ErrorCode err4 = neuropix.neuropix_resetDatapath();
        std::cout << "reset datapath error code: " << err4 << std::endl;

		err3 = neuropix.neuropix_nrst(true);
		std::cout << "nrst 1 error code: " << err3 << std::endl;
    }

    return true;
}

void NeuropixThread::setDefaultChannelNames()
{

	//std::cout << "Setting channel bitVolts to 0.195" << std::endl;

	for (int i = 0; i < 384; i++)
	{
		ChannelCustomInfo info;
		info.name = "AP" + String(i + 1);
		info.gain = 0.1950000f;
		channelInfo.set(i, info);
	}

	for (int i = 0; i < 384; i++)
	{
		ChannelCustomInfo info;
		info.name = "LFP" + String(i + 1);
		info.gain = 0.1950000f;
		channelInfo.set(384 + i, info);
	}
}

bool NeuropixThread::usesCustomNames() const
{
	return true;
}

void NeuropixThread::toggleApData(bool state)
{
     sendAp = state;
}

void NeuropixThread::toggleLfpData(bool state)
{
     sendLfp = state;
}

/** Returns the number of virtual subprocessors this source can generate */
unsigned int NeuropixThread::getNumSubProcessors() const
{
	return 2;
}

/** Returns the number of continuous headstage channels the data source can provide.*/
int NeuropixThread::getNumDataOutputs(DataChannel::DataChannelTypes type, int subProcessorIdx) const
{

	int numChans;

	if (type == DataChannel::DataChannelTypes::HEADSTAGE_CHANNEL && subProcessorIdx == 0)
		numChans = 384;
	else if (type == DataChannel::DataChannelTypes::HEADSTAGE_CHANNEL && subProcessorIdx == 1)
		numChans = 384;
	else
		numChans = 0;

	//std::cout << "Num chans for subprocessor " << subProcessorIdx << " = " << numChans << std::endl;
	
	return numChans;
}

/** Returns the number of TTL channels that each subprocessor generates*/
int NeuropixThread::getNumTTLOutputs(int subProcessorIdx) const 
{
	if (subProcessorIdx == 0)
	{
		return 16;
	}
	else {
		return 0;
	}
}

/** Returns the sample rate of the data source.*/
float NeuropixThread::getSampleRate(int subProcessorIdx) const
{

	float rate;

	if (subProcessorIdx == 0)
		rate = 30000.0f;
	else
		rate = 2500.0f;


//	std::cout << "Sample rate for subprocessor " << subProcessorIdx << " = " << rate << std::endl;

	return rate;
}

/** Returns the volts per bit of the data source.*/
float NeuropixThread::getBitVolts(const DataChannel* chan) const
{
	//std::cout << "BIT VOLTS == 0.195" << std::endl;
	return 0.1950000f;
}

void NeuropixThread::selectElectrode(int chNum, int connection, bool transmit)
{

	ShankConfigErrorCode ec;

	if (!refs.contains(chNum + 1))
	{
		ec = neuropix.neuropix_selectElectrode(chNum, connection, transmit);
		if (ec != SHANK_SUCCESS)
			std::cout << "Failed to write ";
		else
			std::cout << "Succesfully wrote ";
		std::cout << "channel: " << String(chNum) << " to bank : " << String(connection) << std::endl;

	}
	else
	{
		ec = neuropix.neuropix_selectElectrode(chNum, 0xFF, transmit);
		if (ec != SHANK_SUCCESS)
			std::cout << "Failed to disconnect ";
		else
			std::cout << "Succesfully disconnected ";
		std::cout << "ref channel: " << String(chNum) << std::endl;
	}

    //std::cout << "Connecting input " << chNum << " to channel " << connection << "; error code = " << scec << std::endl;

}

void NeuropixThread::setReference(int chNum, int refSetting)
{

    BaseConfigErrorCode bcec = neuropix.neuropix_setReference(chNum, refSetting);

    std::cout << "Set channel " << chNum << " reference to " << refSetting << "; error code = " << bcec << std::endl;
}

void NeuropixThread::setAllReferences(int refChan, int bankForReference)
{
    
    // Option 1-3, numRefs = 10
    // Option 4, numRefs = 7
    int refSetting = refs.indexOf(refChan);

    if (true)
    {
        if (option >= 2) // ensure unused references are disconnected:
        {

            int i; 
            ShankConfigErrorCode ec;

            for (i = 0; i < numRefs - 1; i++)
            {
                if (i == refSetting)
                {
                    if (i == 0)
                        ec = neuropix.neuropix_setExtRef(true, false);
                    else
                        ec = neuropix.neuropix_selectElectrode(refChan - 1, bankForReference, false);

                    //std::cout << "Selecting reference channel error code: " << ec << std::endl;
                }

                else
                {
                    if (i == 0)
                        ec = neuropix.neuropix_setExtRef(false, false);
                    else
                        ec = neuropix.neuropix_selectElectrode(refs[i] - 1, 0xFF, false);

                    //std::cout << "De-selecting reference channel error code: " << ec << std::endl;
                }

            }

            i = numRefs - 1;

            // write everything to shank in the last step:
            if (i == refSetting)
                ec = neuropix.neuropix_selectElectrode(refs[i] - 1, bankForReference, true);
            else
                ec = neuropix.neuropix_selectElectrode(refs[i] - 1, 0xFF, true);

            //std::cout << "Final selection error code: " << ec << std::endl;
        }
    }

    //ShankConfigErrorCode ec;
    //if (refSetting == 0)
    //  ec = neuropix.neuropix_setExtRef(true, true);
    //else
    //  ec = neuropix.neuropix_setExtRef(false, true);

    //std::cout << "setExtRef error code: " << ec << std::endl;
    
    // update reference settings for probe:
    //for (int i = 0; i < 5; i++)
    //{
    //  neuropix.neuropix_setReference(i, (unsigned char)refSetting, false);
    //}
    //BaseConfigErrorCode bcec = neuropix.neuropix_setReference(totalChans - 1, (unsigned char)refSetting, true);

    BaseConfigErrorCode bcec = neuropix.neuropix_writeAllReferences((unsigned char)refSetting);

    std::cout << "Set all references to " << refSetting << "; error code = " << bcec << std::endl;
}

void NeuropixThread::setGain(int chNum, int apGain, int lfpGain)
{
    BaseConfigErrorCode bcec = neuropix.neuropix_setGain(chNum, apGain, lfpGain);

    std::cout << "Set channel " << chNum << " gain to " << apGain << " and " << lfpGain << "; error code = " << bcec << std::endl;
    apGains.set(chNum, apGain);
    lfpGains.set(chNum, lfpGain);
}

void NeuropixThread::setAllApGains(int apGain)
{
    BaseConfigErrorCode bcec = neuropix.neuropix_writeAllAPGains(apGain);

    std::cout << "Set all AP gains to " << apGain << "; error code = " << bcec << std::endl;

    for (int i = 0; i < 384; i++)
        apGains.set(i, apGain);
}

void NeuropixThread::setAllLfpGains(int lfpGain)
{
    BaseConfigErrorCode bcec = neuropix.neuropix_writeAllLFPGains(lfpGain);

    std::cout << "Set all LFP gains to " << lfpGain << "; error code = " << bcec << std::endl;

    for (int i = 0; i < 384; i++)
        lfpGains.set(i, lfpGain);
}


void NeuropixThread::setFilter(int filter)
{
    BaseConfigErrorCode bcec = neuropix.neuropix_setFilter(filter);

    std::cout << "Set filter to " << filter << "; error code = " << bcec << std::endl;
}

void NeuropixThread::setTriggerMode(bool trigger)
{
    ConfigAccessErrorCode caec = neuropix.neuropix_triggerMode(trigger);
    
    internalTrigger = trigger;
}

void NeuropixThread::setRecordMode(bool record)
{
    recordToNpx = record;
}

void NeuropixThread::setAutoRestart(bool restart)
{
	autoRestart = restart;
}


void NeuropixThread::calibrateProbe()
{

	ErrorCode error;

    std::cout << "Applying ADC calibration..." << std::endl;
    error = neuropix.neuropix_applyAdcCalibrationFromEeprom();
	if (error == 0)
		std::cout << "Successfully applied ADC calibration" << std::endl;
	else
		CoreServices::sendStatusMessage("ADC calibration failed.");

	std::cout << "Applying gain correction settings..." << std::endl;
    error = neuropix.neuropix_applyGainCalibrationFromEeprom();

	if (error == 0)
		std::cout << "Successfully applied gain calibration" << std::endl;
	else
		CoreServices::sendStatusMessage("Gain calibration failed.");

    std::cout << "Done." << std::endl;

}

void NeuropixThread::calibrateADCs()
{

	std::cout << "Applying ADC calibration..." << std::endl;
	ErrorCode e = neuropix.neuropix_applyAdcCalibrationFromEeprom();
	std::cout << "Finished with error code " << e << std::endl;

}

void NeuropixThread::calibrateGains()
{

	std::cout << "Applying gain correction settings..." << std::endl;
	ErrorCode e = neuropix.neuropix_applyGainCalibrationFromEeprom();
	std::cout << "Finished with error code " << e << std::endl;

}

void NeuropixThread::calibrateFromCsv(File directory)
{

    // NOT WORKING //

    std::vector<adcComp> adcCompC;
    std::vector<adcPairCommon> adcPairCommonC;

    //Read from csv and apply to API and read from API
    std::cout << "Reading files from " << directory.getFullPathName() << std::endl;

    File comparatorCsv = directory.getChildFile("Comparator_calibration.csv");
    File offsetCsv = directory.getChildFile("Offset_calibration.csv");
    File slopeCsv = directory.getChildFile("Slope_calibration.csv");
    File gainCsv = directory.getChildFile("Gain_calibration.csv");

    //std::cout << File::getCurrentWorkingDirectory().getFullPathName() << std::endl;

	ReadCsvErrorCode err = neuropix.neuropix_readComparatorCalibrationFromCsv(comparatorCsv.getFileName().toStdString());
    std::cout << "Read comparator calibration error code: " << err << std::endl;
    
	err = neuropix.neuropix_readADCOffsetCalibrationFromCsv(offsetCsv.getFileName().toStdString());
    std::cout << "Read ADC offset calibration error code: " << err << std::endl;
    
	err = neuropix.neuropix_readADCSlopeCalibrationFromCsv(slopeCsv.getFileName().toStdString());
    std::cout << "Read ADC slope calibration error code: " << err << std::endl;
    
    //Read parameters from API
    neuropix.neuropix_getADCCompCalibration(adcCompC);
    
    neuropix.neuropix_getADCPairCommonCalibration(adcPairCommonC); 
    
    std::cout << "Writing ADC calibration data to probe..." << std::endl;
    //Write parameters to probe
    for(int i = 0; i < 15; i = i+2)
    { 
        neuropix.neuropix_ADCCalibration(i, adcCompC[2 * i].compP, adcCompC[2 * i].compN, adcCompC[2 * i + 2].compP, adcCompC[2 * i + 2].compN, adcPairCommonC[i].slope, adcPairCommonC[i].fine, adcPairCommonC[i].coarse, adcPairCommonC[i].cfix); 
        neuropix.neuropix_ADCCalibration(i + 1, adcCompC[2 * i + 1].compP, adcCompC[2 * i + 1].compN, adcCompC[2 * i + 3].compP, adcCompC[2 * i + 3].compN, adcPairCommonC[i + 1].slope, adcPairCommonC[i + 1].fine, adcPairCommonC[i + 1].coarse, adcPairCommonC[i + 1].cfix);
    }

    std::vector<unsigned short> gainCorrectionData_;

    std::cout << "Reading gain correction data..." << std::endl;
    //Read gain correction from csv and apply to API member
	err = neuropix.neuropix_readGainCalibrationFromCsv(gainCsv.getFileName().toStdString());
	std::cout << "Read gain calibration error code: " << err << std::endl;

    //Read gain correction from API member
    neuropix.neuropix_getGainCorrectionCalibration(gainCorrectionData_); 

    //resize according to probe type
    if (option < 2)
        gainCorrectionData_.resize(384);
    else if (option == 2)
        gainCorrectionData_.resize(960);
    else if (option == 3)
        gainCorrectionData_.resize(966);
    
    //Write to basestation FPGA
    std::cout << "Applying gain correction to probe..." << std::endl;
    neuropix.neuropix_gainCorrection(gainCorrectionData_);

}

bool NeuropixThread::updateBuffer()
{

	bool shouldRecord = CoreServices::getRecordingStatus();

	if (!isRecording && shouldRecord)
	{
		isRecording = true;
		startTimer(500); // delay to allow RecordNode to update recording directory
	}
	else if (isRecording && !shouldRecord)
	{
		stopRecording();
	}
		

    ElectrodePacket packet;

    ReadErrorCode rec = neuropix.neuropix_readElectrodeData(packet);

    if (rec == READ_SUCCESS)
    {

        if (counter <= 0)
        {
          std::cout << "Fifo fill percentage: ";
          std::cout << neuropix.neuropix_fifoFilling() << std::endl;
          counter = 5000;
		}
		else {
			counter--;
		}

        //if (packet.ctrs[0][0] > maxCounter)
        //  maxCounter = packet.ctrs[0][0];
        
        //counter--;

		std::vector<float> lfpBuffer;

        for (int i = 0; i < 12; i++)
        {
            eventCode = (uint64) packet.synchronization[i]; // currently returning 65535

			std::vector<float> apBuffer;

            for (int j = 0; j < totalChans; j++)
            {
                apBuffer.push_back((packet.apData[i][j] - 0.6) / gains[apGains[j]] * -1000000.0f); // convert to microvolts

                if (i == 0 && sendLfp)
                    lfpBuffer.push_back((packet.lfpData[j] - 0.6) / gains[lfpGains[j]] * -1000000.0f); // convert to microvolts
            }

			sourceBuffers[0]->addToBuffer(apBuffer.data(), &timestampAp, &eventCode, 1);
            timestampAp += 1;
        }

        eventCode = 0;

		if (sendLfp)
		{
			sourceBuffers[1]->addToBuffer(lfpBuffer.data(), &timestampLfp, &eventCode, 1);
			timestampLfp += 1;
		}
            
		//if (counter == 0)
		//{
		//	std::cout << timestampAp << ":" << timestampLfp << std::endl;
		//}

        //std::cout << "READ SUCCESS!" << std::endl;    
        
    }
    else {
        if (rec == NO_DATA_LINK)
        {
            std::cout << "NO DATA LINK" << std::endl;
        }
        else if (rec == WRONG_DATA_MODE)
        {
            std::cout << "WRONG DATA MODE" << std::endl;
        }
        else if (rec == DATA_BUFFER_EMPTY)
        {
            std::cout << "DATA BUFFER EMPTY" << std::endl;
        }
        else if (rec == DATA_ERROR)
        {
            std::cout << "DATA ERROR" << std::endl; // buffer overflow
        }
        else {
            std::cout << "ERROR CODE: " << rec << std::endl;
        }

    }
     
    return true;
}