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

#include "RecordNode.h"
#include "ProcessorGraph.h"
#include "../UI/EditorViewport.h"
#include "../UI/ControlPanel.h"

#include "Channel.h"

RecordNode::RecordNode()
    : GenericProcessor("Record Node"),
      newDirectoryNeeded(true),  zeroBuffer(1, 50000),  timestamp(0)
{

    isProcessing = false;
    isRecording = false;
    sampleCount = 0;
    signalFilesShouldClose = false;

    continuousDataIntegerBuffer = new int16[10000];
    continuousDataFloatBuffer = new float[10000];
    signalFilesShouldClose = false;

    settings.numInputs = 2048;
    settings.numOutputs = 0;

    eventChannel = new Channel(this, 0);
    eventChannel->isEventChannel = true;

    recordMarker = new char[10];
    for (int i = 0; i < 9; i++)
    {
        recordMarker[i] = i;
    }
    recordMarker[9] = 255;

    // 128 inputs, 0 outputs
    setPlayConfigDetails(getNumInputs(),getNumOutputs(),44100.0,128);

    zeroBuffer.clear();

}


RecordNode::~RecordNode()
{

}

void RecordNode::setChannel(Channel* ch)
{

    int channelNum = channelPointers.indexOf(ch);

    std::cout << "Record node setting channel to " << channelNum << std::endl;

    setCurrentChannel(channelNum);

    // for (int i = 0; i < con.size(); i++)
    // {

    // 	if (continuousChannels[i].nodeId == id &&
    // 		continuousChannels[i].chan == chan)
    // 	{
    // 		std::cout << "Found channel " << i << std::endl;
    // 		setCurrentChannel(i);
    // 		break;
    // 	}

    // }
}

void RecordNode::setChannelStatus(Channel* ch, bool status)
{

    //std::cout << "Setting channel status!" << std::endl;
    setChannel(ch);

    if (status)
        setParameter(2, 1.0f);
    else
        setParameter(2, 0.0f);

}

// void RecordNode::enableCurrentChannel(bool state)
// {
// 	continuousChannels[nextAvailableChannel].isRecording = state;
// }

void RecordNode::resetConnections()
{
    //std::cout << "Resetting connections" << std::endl;
    nextAvailableChannel = 0;
    wasConnected = false;

    channelPointers.clear();
    eventChannelPointers.clear();



}

void RecordNode::filenameComponentChanged(FilenameComponent* fnc)
{

    std::cout << "Got a new file" << std::endl;
    dataDirectory = fnc->getCurrentFile();
    std::cout << "File name: " << dataDirectory.getFullPathName();
    if (dataDirectory.isDirectory())
        std::cout << " is a directory." << std::endl;
    else
        std::cout << " is NOT a directory." << std::endl;

    //createNewDirectory();



}


void RecordNode::addInputChannel(GenericProcessor* sourceNode, int chan)
{

    if (chan != getProcessorGraph()->midiChannelIndex)
    {

        int channelIndex = getNextChannel(false);

        setPlayConfigDetails(channelIndex+1,0,44100.0,128);

        channelPointers.add(sourceNode->channels[chan]);

        //   std::cout << channelIndex << std::endl;

        updateFileName(channelPointers[channelIndex]);



        //if (channelPointers[channelIndex]->isRecording)
        //	std::cout << "  This channel will be recorded." << std::endl;
        //else
        //	std::cout << "  This channel will NOT be recorded." << std::endl;

        //std::cout << "adding channel " << getNextChannel(false) << std::endl;

        //std::pair<int, Channel> newPair (getNextChannel(false), newChannel);

        //std::cout << "adding channel " << getNextChannel(false) << std::endl;

        //continuouschannelPointers.insert(newPair);


    }
    else
    {

        for (int n = 0; n < sourceNode->eventChannels.size(); n++)
        {

            eventChannelPointers.add(sourceNode->eventChannels[n]);

        }

    }

}

void RecordNode::updateFileName(Channel* ch)
{
    String filename = rootFolder.getFullPathName();
    filename += rootFolder.separatorString;

    if (!ch->isEventChannel)
    {
        filename += ch->nodeId;
        filename += "_";
        filename += ch->name;
        filename += ".continuous";
    }
    else
    {
        filename += "all_channels.events";
    }

    ch->filename = filename;
    ch->file = 0;

    //std::cout << "Updating " << filename << std::endl;

}

void RecordNode::createNewDirectory()
{
    std::cout << "Creating new directory." << std::endl;

    rootFolder = File(dataDirectory.getFullPathName() + File::separator + generateDirectoryName());

    updateFileName(eventChannel);

    for (int i = 0; i < channelPointers.size(); i++)
    {
        updateFileName(channelPointers[i]);
    }

    newDirectoryNeeded = false;

}

String RecordNode::generateDirectoryName()
{
    Time calendar = Time::getCurrentTime();

    Array<int> t;
    t.add(calendar.getYear()-2000);
    t.add(calendar.getMonth()+1); // January = 0
    t.add(calendar.getDayOfMonth());
    t.add(calendar.getHours());
    t.add(calendar.getMinutes());
    t.add(calendar.getSeconds());

    String filename = getControlPanel()->getTextToPrepend();

    String datestring = "";

    for (int n = 0; n < t.size(); n++)
    {
        if (t[n] < 10)
            datestring += "0";

        datestring += t[n];

        if (n == 2)
            datestring += "_";
        else if (n < 5)
            datestring += "-";
    }

    getControlPanel()->setDateText(datestring);

    filename += datestring;
    filename += getControlPanel()->getTextToAppend();

    return filename;

}

String RecordNode::generateDateString()
{
    Time calendar = Time::getCurrentTime();

    String datestring;

    datestring += String(calendar.getDayOfMonth());
    datestring += "-";
    datestring += calendar.getMonthName(true);
    datestring += "-";
    datestring += String(calendar.getYear());
    datestring += " ";

    int hrs, mins, secs;
    hrs = calendar.getHours();
    mins = calendar.getMinutes();
    secs = calendar.getSeconds();

    datestring += hrs;

    if (mins < 10)
        datestring += 0;

    datestring += mins;

    if (secs < 0)
        datestring += 0;

    datestring += secs;

    return datestring;

}


void RecordNode::setParameter(int parameterIndex, float newValue)
{
    //editor->updateParameterButtons(parameterIndex);

    // 0 = stop recording
    // 1 = start recording
    // 2 = toggle individual channel (0.0f = OFF, anything else = ON)

    if (parameterIndex == 1)
    {

        isRecording = true;
        std::cout << "START RECORDING." << std::endl;

        if (newDirectoryNeeded)
            createNewDirectory();

        if (!rootFolder.exists())
        {
            rootFolder.createDirectory();
            String settingsFileName = rootFolder.getFullPathName() + File::separator + "settings.xml";
            getEditorViewport()->saveState(File(settingsFileName));
        }

        openFile(eventChannel);

        sampleCount = 0; // reset sample count

        // create / open necessary files
        for (int i = 0; i < channelPointers.size(); i++)
        {
            std::cout << "Checking channel " << i << std::endl;

            if (channelPointers[i]->getRecordState())
            {
                openFile(channelPointers[i]);
            }
        }

    }
    else if (parameterIndex == 0)
    {


        std::cout << "STOP RECORDING." << std::endl;

        if (isRecording)
        {

            // close necessary files
            signalFilesShouldClose = true;

        }

        isRecording = false;


    }
    else if (parameterIndex == 2)
    {

        if (isProcessing)
        {

            std::cout << "Toggling channel " << currentChannel << std::endl;

            if (newValue == 0.0f)
            {
                channelPointers[currentChannel]->setRecordState(false);

                if (isRecording)
                {

                    if (sampleCount < BLOCK_LENGTH)
                    {
                        // fill out the rest of the current buffer
                        writeContinuousBuffer(zeroBuffer.getSampleData(0), BLOCK_LENGTH - sampleCount, currentChannel);
                    }

                    closeFile(channelPointers[currentChannel]);
                }

            }
            else
            {
                channelPointers[currentChannel]->setRecordState(true);

                if (isRecording)
                {

                    openFile(channelPointers[currentChannel]);

                    if (sampleCount > 0)
                    {
                        writeTimestampAndSampleCount(channelPointers[currentChannel]->file);
                        // fill up the first data block up to sample count
                        writeContinuousBuffer(zeroBuffer.getSampleData(0), sampleCount, currentChannel);
                    }

                }
            }
        }
    }
}

void RecordNode::openFile(Channel* ch)
{
    std::cout << "OPENING FILE: " << ch->filename << std::endl;

    File f = File(ch->filename);
    FILE* chFile;

    bool fileExists = f.exists();
    
    diskWriteLock.enter();

    chFile = fopen(ch->filename.toUTF8(), "ab");

    if (!fileExists)
    {
        // create and write header
        std::cout << "Writing header." << std::endl;
        String header = generateHeader(ch);
        //std::cout << header << std::endl;
        std::cout << "File ID: " << chFile << ", number of bytes: " << header.getNumBytesAsUTF8() << std::endl;

        fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), chFile);

        std::cout << "Wrote header." << std::endl;

    }
    else
    {
        std::cout << "File already exists, just opening." << std::endl;
    }

    diskWriteLock.exit();

    //To avoid a race condition resulting on data written before the header,
    //do not assign the channel pointer until the header has been written
    ch->file = chFile;


}

void RecordNode::closeFile(Channel* ch)
{
    
    diskWriteLock.enter();
    
    std::cout << "CLOSING FILE: " << ch->filename << std::endl;
    if (ch->file != NULL)
        fclose(ch->file);
    
    diskWriteLock.exit();
}

String RecordNode::generateHeader(Channel* ch)
{

    String header = "header.format = 'Open Ephys Data Format'; \n";

    header += "header.version = 0.1;";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";

    if (ch->isEventChannel)
    {
        header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample position, one uint8 event type, one uint8 processor ID, one uint8 event ID, and one uint8 event channel'; \n";

    }
    else
    {
        header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample count (N), N 16-bit samples, and one 10-byte record marker (0 1 2 3 4 5 6 7 8 255)'; \n";
    }


    header += "header.date_created = '";
    header += generateDateString();
    header += "';\n";

    header += "header.channel = '";
    header += ch->name;
    header += "';\n";

    if (ch->isEventChannel)
    {

        header += "header.channelType = 'Event';\n";

    }
    else
    {

        header += "header.channelType = 'Continuous';\n";
    }

    header += "header.sampleRate = ";
    header += String(channelPointers[0]->sampleRate); // all channels need to have the
    // same sample rate under the current
    // scheme
    header += ";\n";
    header += "header.blockLength = ";
    header += BLOCK_LENGTH;
    header += ";\n";

    header += "header.bitVolts = ";
    header += String(ch->bitVolts);
    header += ";\n";

    header = header.paddedRight(' ', HEADER_SIZE);

    //std::cout << header << std::endl;

    return header;

}

void RecordNode::closeAllFiles()
{

    for (int i = 0; i < channelPointers.size(); i++)
    {
        if (channelPointers[i]->getRecordState())
        {

            if (sampleCount < BLOCK_LENGTH)
            {
                // fill out the rest of the current buffer
                writeContinuousBuffer(zeroBuffer.getSampleData(0), BLOCK_LENGTH - sampleCount, i);
            }

            closeFile(channelPointers[i]);
        }
    }

    closeFile(eventChannel);
}

bool RecordNode::enable()
{

    //updateFileName(eventChannel);

    isProcessing = true;
    return true;
}


bool RecordNode::disable()
{
    // close files if necessary
    setParameter(0, 10.0f);

    isProcessing = false;

    return true;
}

float RecordNode::getFreeSpace()
{
    return 1.0f - float(dataDirectory.getBytesFreeOnVolume())/float(dataDirectory.getVolumeTotalSize());
}

void RecordNode::writeContinuousBuffer(float* data, int nSamples, int channel)
{

    // check to see if the file exists
    if (channelPointers[channel]->file == NULL)
        return;

    // scale the data back into the range of int16
    float scaleFactor =  float(0x7fff) * channelPointers[channel]->bitVolts;
    for (int n = 0; n < nSamples; n++)
    {
        *(continuousDataFloatBuffer+n) = *(data+n) / scaleFactor;
    }
    AudioDataConverters::convertFloatToInt16BE(continuousDataFloatBuffer, continuousDataIntegerBuffer, nSamples);

    //
    //int16 samps = (int16) nSamples;

    if (sampleCount == 0)
    {
        writeTimestampAndSampleCount(channelPointers[channel]->file);
    }

    diskWriteLock.enter();
    // FIXME: ensure fwrite returns equal "count"; otherwise,
    // there was an error.
    fwrite(continuousDataIntegerBuffer,     // ptr
           2,                               // size of each element
           nSamples,                        // count
           channelPointers[channel]->file); // ptr to FILE object

    diskWriteLock.exit();
    
    if (sampleCount + nSamples == BLOCK_LENGTH)
    {
        writeRecordMarker(channelPointers[channel]->file);
    }

}

void RecordNode::writeTimestampAndSampleCount(FILE* file)
{

    diskWriteLock.enter();
    
    uint16 samps = BLOCK_LENGTH;

    fwrite(&timestamp,                       // ptr
           8,                               // size of each element
           1,                               // count
           file); // ptr to FILE object

    fwrite(&samps,                           // ptr
           2,                               // size of each element
           1,                               // count
           file); // ptr to FILE object

    diskWriteLock.exit();
}

void RecordNode::writeRecordMarker(FILE* file)
{
    // write a 10-byte marker indicating the end of a record

    diskWriteLock.enter();
    fwrite(recordMarker,        // ptr
           1,                   // size of each element
           10,                  // count
           file);               // ptr to FILE object

    diskWriteLock.exit();
}

void RecordNode::writeEventBuffer(MidiMessage& event, int samplePosition) //, int node, int channel)
{
    // find file and write samples to disk
    //std::cout << "Received event!" << std::endl;

    const uint8* dataptr = event.getRawData();
    uint64 samplePos = (uint64) samplePosition;

    int64 eventTimestamp = timestamp + samplePos;

    diskWriteLock.enter();
    // write timestamp (for buffer only, not the actual event timestamp!!!!!)
    fwrite(&eventTimestamp,							// ptr
           8,   							// size of each element
           1, 		  						// count
           eventChannel->file);   			// ptr to FILE object

    fwrite(&samplePos,							// ptr
           2,   							// size of each element
           1, 		  						// count
           eventChannel->file);   			// ptr to FILE object

    // write 1st four bytes of event (type, nodeId, eventId, eventChannel)
    fwrite(dataptr, 1, 4, eventChannel->file);
    diskWriteLock.exit();

}

void RecordNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{
    if (eventType == TTL)
    {
        writeEventBuffer(event, samplePosition);
    }
    else if (eventType == TIMESTAMP)
    {
        const uint8* dataptr = event.getRawData();

        // std::cout << (int) *(dataptr + 11) << " " <<
        //             (int) *(dataptr + 10) << " " <<
        //             (int) *(dataptr + 9) << " " <<
        //             (int) *(dataptr + 8) << " " <<
        //             (int) *(dataptr + 7) << " " <<
        //             (int) *(dataptr + 6) << " " <<
        //             (int) *(dataptr + 5) << " " <<
        //             (int) *(dataptr + 4) << std::endl;


        memcpy(&timestamp, dataptr + 4, 8); // remember to skip first four bytes
    }

}

void RecordNode::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events,
                         int& nSamples)
{

    //std::cout << "Record node processing block." << std::endl;
    //std::cout << "Num channels: " << buffer.getNumChannels() << std::endl;


    if (isRecording)
    {

        //timestamp = timer.getHighResolutionTicks();

        // WHY IS THIS AFFECTING THE LFP DISPLAY?
        //buffer.applyGain(0, nSamples, 5.2438f);

        // cycle through events -- extract the TTLs and the timestamps
        checkForEvents(events);

        // cycle through buffer channels

        int samplesWritten = 0;

        if (channelPointers.size() > 0)
        {

            while (samplesWritten < nSamples)
            {

                int numSamplesToWrite = nSamples - samplesWritten;

                if (sampleCount + numSamplesToWrite < BLOCK_LENGTH)
                {

                    for (int i = 0; i < buffer.getNumChannels(); i++)
                    {

                        if (channelPointers[i]->getRecordState())
                        {
                            // write buffer to disk!
                            writeContinuousBuffer(buffer.getSampleData(i,samplesWritten),
                                                  numSamplesToWrite,
                                                  i);

                            //std::cout << "Record channel " << i << std::endl;
                        }
                    }

                    samplesWritten += numSamplesToWrite;
                    sampleCount += numSamplesToWrite;

                }
                else
                {

                    numSamplesToWrite = BLOCK_LENGTH - sampleCount;

                    for (int i = 0; i < buffer.getNumChannels(); i++)
                    {

                        if (channelPointers[i]->getRecordState())
                        {
                            // write buffer to disk!
                            writeContinuousBuffer(buffer.getSampleData(i,samplesWritten),
                                                  numSamplesToWrite,
                                                  i);

                            //std::cout << "Record channel " << i << std::endl;
                        }
                    }

                    timestamp += numSamplesToWrite;
                    samplesWritten += numSamplesToWrite;
                    sampleCount = 0;

                }
            }
        }

        //  std::cout << nSamples << " " << samplesWritten << " " << sampleCount << std::endl;

        return;

    }

    // this is intended to prevent parameter changes from closing files
    // before recording stops
    if (signalFilesShouldClose)
    {
        closeAllFiles();
        signalFilesShouldClose = false;
    }

}
