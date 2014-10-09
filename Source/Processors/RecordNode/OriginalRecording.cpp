/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Florian Franzen

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

#include "OriginalRecording.h"
#include "../../Audio/AudioComponent.h"

OriginalRecording::OriginalRecording() : separateFiles(false),
    blockIndex(0), recordingNumber(0), experimentNumber(0),  zeroBuffer(1, 50000), eventFile(nullptr), messageFile(nullptr)
{
    continuousDataIntegerBuffer = new int16[10000];
    continuousDataFloatBuffer = new float[10000];

    recordMarker = new char[10];
    for (int i = 0; i < 9; i++)
    {
        recordMarker[i] = i;
    }
    recordMarker[9] = 255;

    zeroBuffer.clear();
}

OriginalRecording::~OriginalRecording()
{
    //Cleanup just in case
    for (int i=0; i < fileArray.size(); i++)
    {
        if (fileArray[i] != nullptr) fclose(fileArray[i]);
    }
    for (int i=0; i < spikeFileArray.size(); i++)
    {
        if (spikeFileArray[i] != nullptr) fclose(spikeFileArray[i]);
    }
    delete continuousDataFloatBuffer;
    delete continuousDataIntegerBuffer;
    delete recordMarker;
}

String OriginalRecording::getEngineID()
{
    return "OPENEPHYS";
}

void OriginalRecording::addChannel(int index, Channel* chan)
{
    //Just populate the file array with null so we can address it by index afterwards
    fileArray.add(nullptr);
}

void OriginalRecording::addSpikeElectrode(int index, SpikeRecordInfo* elec)
{
    spikeFileArray.add(nullptr);
}

void OriginalRecording::resetChannels()
{
    fileArray.clear();
    spikeFileArray.clear();
}

void OriginalRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
    this->recordingNumber = recordingNumber;
    this->experimentNumber = experimentNumber;
    openFile(rootFolder,nullptr);
    openMessageFile(rootFolder);
    for (int i = 0; i < fileArray.size(); i++)
    {
        if (getChannel(i)->getRecordState())
        {
            openFile(rootFolder,getChannel(i));
        }
    }
    for (int i = 0; i < spikeFileArray.size(); i++)
    {
        openSpikeFile(rootFolder,getSpikeElectrode(i));
    }
    blockIndex = 0;
}

void OriginalRecording::openFile(File rootFolder, Channel* ch)
{
    FILE* chFile;
    bool isEvent;
    String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);

    std::cout << "OPENING FILE: " << fullPath << std::endl;

    isEvent = (ch == nullptr) ? true : false;
    if (isEvent)
    {
        if (experimentNumber > 1)
            fullPath += "all_channels_" + String(experimentNumber) + ".events";
        else
            fullPath += "all_channels.events";
    }
    else
    {
        fullPath += getFileName(ch);
    }

    File f = File(fullPath);

    bool fileExists = f.exists();

    diskWriteLock.enter();

    chFile = fopen(fullPath.toUTF8(), "ab");

    if (!fileExists)
    {
        // create and write header
        std::cout << "Writing header." << std::endl;
        String header = generateHeader(ch);
        //std::cout << header << std::endl;
        std::cout << "File ID: " << chFile << ", number of bytes: " << header.getNumBytesAsUTF8() << std::endl;


        fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), chFile);

        std::cout << "Wrote header." << std::endl;

        std::cout << "Block index: " << blockIndex << std::endl;

    }
    else
    {
        std::cout << "File already exists, just opening." << std::endl;
    }

    diskWriteLock.exit();

    if (isEvent)
        eventFile = chFile;
    else
        fileArray.set(ch->recordIndex,chFile);

}

void OriginalRecording::openSpikeFile(File rootFolder, SpikeRecordInfo* elec)
{

    FILE* spFile;
    String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);
    fullPath += elec->name.removeCharacters(" ");

    if (experimentNumber > 1)
    {
        fullPath += "_" + String(experimentNumber);
    }

    fullPath += ".spikes";

    std::cout << "OPENING FILE: " << fullPath << std::endl;

    File f = File(fullPath);

    bool fileExists = f.exists();

    diskWriteLock.enter();

    spFile = fopen(fullPath.toUTF8(),"ab");

    if (!fileExists)
    {
        String header = generateSpikeHeader(elec);
        fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), spFile);
    }
    diskWriteLock.exit();
    spikeFileArray.set(elec->recordIndex,spFile);

}

void OriginalRecording::openMessageFile(File rootFolder)
{
    FILE* mFile;
    String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);
    fullPath += "messages.events";

    std::cout << "OPENING FILE: " << fullPath << std::endl;

    File f = File(fullPath);

    bool fileExists = f.exists();

    diskWriteLock.enter();

    mFile = fopen(fullPath.toUTF8(),"ab");

    //If this file needs a header, it goes here

    diskWriteLock.exit();
    messageFile = mFile;

}

String OriginalRecording::getFileName(Channel* ch)
{
    String filename;

    filename += ch->nodeId;
    filename += "_";
    filename += ch->name;

    if (experimentNumber > 1)
    {
        filename += "_" + String(experimentNumber);
    }

    if (separateFiles)
    {
        filename += "_";
        filename += recordingNumber;
    }
    filename += ".continuous";

    return filename;
}

String OriginalRecording::generateHeader(Channel* ch)
{

    String header = "header.format = 'Open Ephys Data Format'; \n";

    header += "header.version = 0.4;";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";

    if (ch == nullptr)
    {
        header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample position, one uint8 event type, one uint8 processor ID, one uint8 event ID, one uint8 event channel, and one uint16 recordingNumber'; \n";

    }
    else
    {
        header += "header.description = 'each record contains one 64-bit timestamp, one 16-bit sample count (N), 1 uint16 recordingNumber, N 16-bit samples, and one 10-byte record marker (0 1 2 3 4 5 6 7 8 255)'; \n";
    }


    header += "header.date_created = '";
    header += generateDateString();
    header += "';\n";

    header += "header.channel = '";
    header += (ch != nullptr) ? ch->name : "Events";
    header += "';\n";

    if (ch == nullptr)
    {

        header += "header.channelType = 'Event';\n";
    }
    else
    {
        header += "header.channelType = 'Continuous';\n";
    }

    header += "header.sampleRate = ";
    // all channels need to have the same sample rate under the current scheme
    header += String(getChannel(0)->sampleRate);
    header += ";\n";
    header += "header.blockLength = ";
    header += BLOCK_LENGTH;
    header += ";\n";
    header += "header.bufferSize = ";
    header += getAudioComponent()->getBufferSize();
    header += ";\n";
    header += "header.bitVolts = ";
    header += (ch != nullptr) ? String(ch->bitVolts) : "1";
    header += ";\n";

    header = header.paddedRight(' ', HEADER_SIZE);

    //std::cout << header << std::endl;

    return header;

}

String OriginalRecording::generateSpikeHeader(SpikeRecordInfo* elec)
{
    String header = "header.format = 'Open Ephys Data Format'; \n";
    header += "header.version = 0.4;";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";

    header += "header.description = 'Each record contains 1 uint8 eventType, 1 uint64 timestamp, 1 uint16 electrodeID, 1 uint16 numChannels (n), 1 uint16 numSamples (m), n*m uint16 samples, n uint16 channelGains, n uint16 thresholds, and 1 uint16 recordingNumber'; \n";

    header += "header.date_created = '";
    header += generateDateString();
    header += "';\n";

    header += "header.electrode = '";
    header += elec->name;
    header += "';\n";

    header += "header.num_channels = ";
    header += elec->numChannels;
    header += ";\n";

    header += "header.sampleRate = ";
    header += String(elec->sampleRate);
    header += ";\n";

    header = header.paddedRight(' ', HEADER_SIZE);

    //std::cout << header << std::endl;

    return header;
}

void OriginalRecording::writeEvent(int eventType, MidiMessage& event, int samplePosition)
{
    if (eventType == GenericProcessor::TTL)
        writeTTLEvent(event,samplePosition);
    else if (eventType == GenericProcessor::MESSAGE)
        writeMessage(event,samplePosition);
}

void OriginalRecording::writeMessage(MidiMessage& event, int samplePosition)
{
    if (messageFile == nullptr)
        return;
    uint64 samplePos = (uint64) samplePosition;

    int64 eventTimestamp = timestamp + samplePos;

    int msgLength = event.getRawDataSize() - 5;
    const char* dataptr = (const char*)event.getRawData() + 4;

    String timestampText(eventTimestamp);

    diskWriteLock.enter();
    fwrite(timestampText.toUTF8(),1,timestampText.length(),messageFile);
    fwrite(" ",1,1,messageFile);
    fwrite(dataptr,1,msgLength,messageFile);
    fwrite("\n",1,1,messageFile);
    diskWriteLock.exit();

}

void OriginalRecording::writeTTLEvent(MidiMessage& event, int samplePosition)
{
    // find file and write samples to disk
    // std::cout << "Received event!" << std::endl;

    if (eventFile == nullptr)
        return;

    const uint8* dataptr = event.getRawData();

    uint64 samplePos = (uint64) samplePosition;

    int64 eventTimestamp = timestamp + samplePos; // add the sample position to the buffer timestamp

    diskWriteLock.enter();

    fwrite(&eventTimestamp,					// ptr
           8,   							// size of each element
           1, 		  						// count
           eventFile);   			// ptr to FILE object

    fwrite(&samplePos,							// ptr
           2,   							// size of each element
           1, 		  						// count
           eventFile);   			// ptr to FILE object

    // write 1st four bytes of event (type, nodeId, eventId, eventChannel)
    fwrite(dataptr, 1, 4, eventFile);

    // write recording number
    fwrite(&recordingNumber,                     // ptr
           2,                               // size of each element
           1,                               // count
           eventFile);             // ptr to FILE object

    diskWriteLock.exit();
}

void OriginalRecording::writeData(AudioSampleBuffer& buffer, int nSamples)
{
    int samplesWritten = 0;

    while (samplesWritten < nSamples) // there are still unwritten samples in the buffer
    {

        int numSamplesToWrite = nSamples - samplesWritten; // samples remaining in the buffer

        if (blockIndex + numSamplesToWrite < BLOCK_LENGTH) // we still have space in this block
        {
            for (int i = 0; i < buffer.getNumChannels(); i++)
            {

                if (getChannel(i)->getRecordState())
                {
                    // write buffer to disk!
                    writeContinuousBuffer(buffer.getReadPointer(i,samplesWritten),
                                          numSamplesToWrite,
                                          i);


                }
            }

            // update our variables
            samplesWritten += numSamplesToWrite;
            timestamp += numSamplesToWrite;
            blockIndex += numSamplesToWrite;

        }
        else // there's not enough space left in this block for all remaining samples
        {

            numSamplesToWrite = BLOCK_LENGTH - blockIndex;

            for (int i = 0; i < buffer.getNumChannels(); i++)
            {

                if (getChannel(i)->getRecordState())
                {
                    // write buffer to disk!
                    writeContinuousBuffer(buffer.getReadPointer(i,samplesWritten),
                                          numSamplesToWrite,
                                          i);

                    //std::cout << "Record channel " << i << std::endl;
                }
            }

            // update our variables
            samplesWritten += numSamplesToWrite;
            timestamp += numSamplesToWrite;
            blockIndex = 0; // back to the beginning of the block

        }
    }
}

void OriginalRecording::writeContinuousBuffer(const float* data, int nSamples, int channel)
{
    // check to see if the file exists
    if (fileArray[channel] == nullptr)
        return;

    // scale the data back into the range of int16
    float scaleFactor =  float(0x7fff) * getChannel(channel)->bitVolts;
    for (int n = 0; n < nSamples; n++)
    {
        *(continuousDataFloatBuffer+n) = *(data+n) / scaleFactor;
    }
    AudioDataConverters::convertFloatToInt16BE(continuousDataFloatBuffer, continuousDataIntegerBuffer, nSamples);

    if (blockIndex == 0)
    {
        writeTimestampAndSampleCount(fileArray[channel]);
    }

    diskWriteLock.enter();

    size_t count = fwrite(continuousDataIntegerBuffer, // ptr
                          2,                               // size of each element
                          nSamples,                        // count
                          fileArray[channel]); // ptr to FILE object

    jassert(count == nSamples); // make sure all the data was written

    diskWriteLock.exit();

    if (blockIndex + nSamples == BLOCK_LENGTH)
    {
        writeRecordMarker(fileArray[channel]);
    }
}

void OriginalRecording::writeTimestampAndSampleCount(FILE* file)
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

    fwrite(&recordingNumber,                         // ptr
           2,                               // size of each element
           1,                               // count
           file); // ptr to FILE object

    diskWriteLock.exit();
}

void OriginalRecording::writeRecordMarker(FILE* file)
{
    // write a 10-byte marker indicating the end of a record

    diskWriteLock.enter();
    fwrite(recordMarker,        // ptr
           1,                   // size of each element
           10,                  // count
           file);               // ptr to FILE object

    diskWriteLock.exit();
}

void OriginalRecording::closeFiles()
{
    for (int i = 0; i < fileArray.size(); i++)
    {
        if (fileArray[i] != nullptr)
        {
            if (blockIndex < BLOCK_LENGTH)
            {
                // fill out the rest of the current buffer
                writeContinuousBuffer(zeroBuffer.getReadPointer(0), BLOCK_LENGTH - blockIndex, i);
                diskWriteLock.enter();
                fclose(fileArray[i]);
                fileArray.set(i,nullptr);
                diskWriteLock.exit();
            }
        }
    }
    for (int i = 0; i < spikeFileArray.size(); i++)
    {
        if (spikeFileArray[i] != nullptr)
        {
            diskWriteLock.enter();
            fclose(spikeFileArray[i]);
            spikeFileArray.set(i,nullptr);
            diskWriteLock.exit();
        }
    }
    if (eventFile != nullptr)
    {
        diskWriteLock.enter();
        fclose(eventFile);
        eventFile = nullptr;
        diskWriteLock.exit();
    }
    if (messageFile != nullptr)
    {
        diskWriteLock.enter();
        fclose(messageFile);
        messageFile = nullptr;
        diskWriteLock.exit();
    }
    blockIndex = 0;
}

void OriginalRecording::updateTimeStamp(int64 timestamp)
{
    this->timestamp = timestamp;
}

void OriginalRecording::writeSpike(const SpikeObject& spike, int electrodeIndex)
{
    uint8_t spikeBuffer[MAX_SPIKE_BUFFER_LEN];

    if (spikeFileArray[electrodeIndex] == nullptr)
        return;

    packSpike(&spike, spikeBuffer, MAX_SPIKE_BUFFER_LEN);

    int totalBytes = spike.nSamples * spike.nChannels * 2 + // account for samples
                     spike.nChannels * 4 +            // acount for gain
                     spike.nChannels * 2 +            // account for thresholds
                     SPIKE_METADATA_SIZE;             // 42, from SpikeObject.h


    // format:
    // 1 byte of event type (always = 4 for spikes)
    // 8 bytes for 64-bit timestamp
    // 2 bytes for 16-bit electrode ID
    // 2 bytes for 16-bit number of channels (n)
    // 2 bytes for 16-bit number of samples (m)
    // 2*n*m bytes for 16-bit samples
    // 2*n bytes for 16-bit gains
    // 2*n bytes for 16-bit thresholds

    // const MessageManagerLock mmLock;

    diskWriteLock.enter();

    fwrite(spikeBuffer, 1, totalBytes, spikeFileArray[electrodeIndex]);

    fwrite(&recordingNumber,                         // ptr
           2,                               // size of each element
           1,                               // count
           spikeFileArray[electrodeIndex]); // ptr to FILE object

    diskWriteLock.exit();
}

void OriginalRecording::setParameter(EngineParameter& parameter)
{
    boolParameter(0, separateFiles);
}

RecordEngineManager* OriginalRecording::getEngineManager()
{
    RecordEngineManager* man = new RecordEngineManager("OPENEPHYS","Open Ephys",nullptr);
    EngineParameter* param;
    param = new EngineParameter(EngineParameter::BOOL,0,"Separate Files",false);
    man->addParameter(param);
    return man;
}