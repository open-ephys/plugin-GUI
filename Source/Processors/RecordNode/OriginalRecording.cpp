/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Florian Franzen

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
#include "../../AccessClass.h"
#include "../../Audio/AudioComponent.h"

OriginalRecording::OriginalRecording() : separateFiles(false),
    recordingNumber(0), experimentNumber(0),  zeroBuffer(1, 50000),
    eventFile(nullptr), messageFile(nullptr), lastProcId(0)
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
    blockIndex.add(0);
    samplesSinceLastTimestamp.add(0);
}

void OriginalRecording::addSpikeElectrode(int index, SpikeRecordInfo* elec)
{
    spikeFileArray.add(nullptr);
}

void OriginalRecording::resetChannels()
{
    fileArray.clear();
    spikeFileArray.clear();
    blockIndex.clear();
    processorArray.clear();
    samplesSinceLastTimestamp.clear();
}

void OriginalRecording::openFiles(File rootFolder, int experimentNumber, int recordingNumber)
{
    this->recordingNumber = recordingNumber;
    this->experimentNumber = experimentNumber;

    processorArray.clear();
    lastProcId = 0;

    openFile(rootFolder,nullptr);
    openMessageFile(rootFolder);

    for (int i = 0; i < fileArray.size(); i++)
    {
        if (getChannel(i)->getRecordState())
        {
            openFile(rootFolder,getChannel(i));
            blockIndex.set(i,0);
            samplesSinceLastTimestamp.set(i,0);
        }

    }
    for (int i = 0; i < spikeFileArray.size(); i++)
    {
        openSpikeFile(rootFolder,getSpikeElectrode(i));
    }
}

void OriginalRecording::openFile(File rootFolder, Channel* ch)
{
    FILE* chFile;
    bool isEvent;
    String fullPath(rootFolder.getFullPathName() + rootFolder.separatorString);
    String fileName;

    recordPath = fullPath;

    isEvent = (ch == nullptr) ? true : false;
    if (isEvent)
    {
        if (experimentNumber > 1)
            fileName += "all_channels_" + String(experimentNumber) + ".events";
        else
            fileName += "all_channels.events";
    }
    else
    {
        fileName += getFileName(ch);
    }

    fullPath += fileName;
    std::cout << "OPENING FILE: " << fullPath << std::endl;

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

        // std::cout << "Block index: " << blockIndex << std::endl;

    }
    else
    {
        std::cout << "File already exists, just opening." << std::endl;
        fseek(chFile, 0, SEEK_END);
    }

    if (isEvent)
        eventFile = chFile;
    else
    {
        fileArray.set(ch->recordIndex,chFile);
        if (ch->nodeId != lastProcId)
        {
            lastProcId = ch->nodeId;
            ProcInfo* p = new ProcInfo();
            p->id = ch->nodeId;
            p->sampleRate = ch->sampleRate;
            processorArray.add(p);
        }
        ChannelInfo* c = new ChannelInfo();
        c->filename = fileName;
        c->name = ch->name;
        c->startPos = ftell(chFile);
        c->bitVolts = ch->bitVolts;
        processorArray.getLast()->channels.add(c);
    }
    diskWriteLock.exit();

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

	fullPath += "messages";

	if (experimentNumber > 1)
	{
		fullPath += "_" + String(experimentNumber);
	}

    fullPath += ".events";

    std::cout << "OPENING FILE: " << fullPath << std::endl;

    File f = File(fullPath);

    //bool fileExists = f.exists();

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
    if (renameFiles)
        filename += renamedPrefix + String(ch->mappedIndex + 1);
    else
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

    header += "header.version = "+ String(VERSION_STRING) +"; \n";
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
    header += AccessClass::getAudioComponent()->getBufferSize();
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
    header += "header.version = " + String(VERSION_STRING) + "; \n";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";

    header += "header.description = 'Each record contains 1 uint8 eventType, 1 int64 timestamp, 1 int64 software timestamp, "
              "1 uint16 sourceID, 1 uint16 numChannels (n), 1 uint16 numSamples (m), 1 uint16 sortedID, 1 uint16 electrodeID, "
              "1 uint16 channel, 3 uint8 color codes, 2 float32 component projections, n*m uint16 samples, n float32 channelGains, n uint16 thresholds, and 1 uint16 recordingNumber'; \n";

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
    if (isWritableEvent(eventType))
        writeTTLEvent(event,samplePosition);
    if (eventType == GenericProcessor::MESSAGE)
        writeMessage(event,samplePosition);
}

void OriginalRecording::writeMessage(MidiMessage& event, int samplePosition)
{
    if (messageFile == nullptr)
        return;
    uint64 samplePos = (uint64) samplePosition;
    uint8 sourceNodeId = event.getNoteNumber();

    int64 eventTimestamp = (*timestamps)[sourceNodeId] + samplePos;

    int msgLength = event.getRawDataSize() - 6;
    const char* dataptr = (const char*)event.getRawData() + 6;

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

    uint8 sourceNodeId = event.getNoteNumber();

    int64 eventTimestamp = (*timestamps)[sourceNodeId] + samplePos; // add the sample position to the buffer timestamp

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

void OriginalRecording::writeData(AudioSampleBuffer& buffer)
{

    for (int i = 0; i < buffer.getNumChannels(); i++)
    {
        if (getChannel(i)->getRecordState())
        {
            int samplesWritten = 0;

            int sourceNodeId = getChannel(i)->sourceNodeId;

            samplesSinceLastTimestamp.set(i,0);

            int nSamples = (*numSamples)[sourceNodeId];

            while (samplesWritten < nSamples) // there are still unwritten samples in this buffer
            {
                int numSamplesToWrite = nSamples - samplesWritten;

                if (blockIndex[i] + numSamplesToWrite < BLOCK_LENGTH) // we still have space in this block
                {

                    // write buffer to disk!
                    writeContinuousBuffer(buffer.getReadPointer(i,samplesWritten),
                                          numSamplesToWrite,
                                          i);

                    //timestamp += numSamplesToWrite;
                    samplesSinceLastTimestamp.set(i, samplesSinceLastTimestamp[i] + numSamplesToWrite);
                    blockIndex.set(i, blockIndex[i] + numSamplesToWrite);
                    samplesWritten += numSamplesToWrite;

                }
                else   // there's not enough space left in this block for all remaining samples
                {

                    numSamplesToWrite = BLOCK_LENGTH - blockIndex[i];

                    // write buffer to disk!
                    writeContinuousBuffer(buffer.getReadPointer(i,samplesWritten),
                                          numSamplesToWrite,
                                          i);

                    // update our variables
                    samplesWritten += numSamplesToWrite;
                    //timestamp += numSamplesToWrite;
                    samplesSinceLastTimestamp.set(i, samplesSinceLastTimestamp[i] + numSamplesToWrite);
                    blockIndex.set(i,0); // back to the beginning of the block
                }
            }

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

    if (blockIndex[channel] == 0)
    {
        writeTimestampAndSampleCount(fileArray[channel], channel);
    }

    diskWriteLock.enter();

    size_t count = fwrite(continuousDataIntegerBuffer, // ptr
                          2,                               // size of each element
                          nSamples,                        // count
                          fileArray[channel]); // ptr to FILE object

    //std::cout << channel << " : " << nSamples << " : " << count << std::endl;

    jassert(count == nSamples); // make sure all the data was written

    diskWriteLock.exit();

    if (blockIndex[channel] + nSamples == BLOCK_LENGTH)
    {
        writeRecordMarker(fileArray[channel]);
    }
}

void OriginalRecording::writeTimestampAndSampleCount(FILE* file, int channel)
{
    diskWriteLock.enter();

    uint16 samps = BLOCK_LENGTH;

    int sourceNodeId = getChannel(channel)->sourceNodeId;

    int64 ts = (*timestamps)[sourceNodeId] + samplesSinceLastTimestamp[channel];

    fwrite(&ts,                       // ptr
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
            if (blockIndex[i] < BLOCK_LENGTH)
            {
                // fill out the rest of the current buffer
                writeContinuousBuffer(zeroBuffer.getReadPointer(0), BLOCK_LENGTH - blockIndex[i], i);
                diskWriteLock.enter();
                fclose(fileArray[i]);
                fileArray.set(i,nullptr);
                diskWriteLock.exit();
            }
        }

        blockIndex.set(i,0);
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

    writeXml();

}

// void OriginalRecording::updateTimeStamp(int64 timestamp)
// {
//     this->timestamp = timestamp;
// }

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


    diskWriteLock.enter();

    fwrite(spikeBuffer, 1, totalBytes, spikeFileArray[electrodeIndex]);

    fwrite(&recordingNumber,                         // ptr
           2,                               // size of each element
           1,                               // count
           spikeFileArray[electrodeIndex]); // ptr to FILE object

    diskWriteLock.exit();
}

void OriginalRecording::writeXml()
{
    String name = recordPath + "Continuous_Data";
    if (experimentNumber > 1)
    {
        name += "_";
        name += String(experimentNumber);
    }
    name += ".openephys";

    File file(name);
    XmlDocument doc(file);
    ScopedPointer<XmlElement> xml = doc.getDocumentElement();
    if (!xml || ! xml->hasTagName("EXPERIMENT"))
    {
        xml = new XmlElement("EXPERIMENT");
        xml->setAttribute("version",VERSION);
        xml->setAttribute("number",experimentNumber);
        xml->setAttribute("separatefiles",separateFiles);
    }
    XmlElement* rec = new XmlElement("RECORDING");
    rec->setAttribute("number",recordingNumber);
    //rec->setAttribute("length",(double)(timestamp-startTimestamp));
    for (int i = 0; i < processorArray.size(); i++)
    {
        XmlElement* proc = new XmlElement("PROCESSOR");
        proc->setAttribute("id",processorArray[i]->id);
        rec->setAttribute("samplerate",processorArray[i]->sampleRate);
        for (int j = 0; j < processorArray[i]->channels.size(); j++)
        {
            ChannelInfo* c = processorArray[i]->channels[j];
            XmlElement* chan = new XmlElement("CHANNEL");
            chan->setAttribute("name",c->name);
            chan->setAttribute("bitVolts",c->bitVolts);
            chan->setAttribute("filename",c->filename);
            chan->setAttribute("position",(double)(c->startPos)); //As long as the file doesnt exceed 2^53 bytes, this will have integer precission. Better than limiting to 32bits.
            proc->addChildElement(chan);
        }
        rec->addChildElement(proc);
    }
    xml->addChildElement(rec);
    xml->writeToFile(file,String::empty);
}

void OriginalRecording::setParameter(EngineParameter& parameter)
{
    boolParameter(0, separateFiles);
    boolParameter(1, renameFiles);
    strParameter(2, renamedPrefix);
}

RecordEngineManager* OriginalRecording::getEngineManager()
{
    RecordEngineManager* man = new RecordEngineManager("OPENEPHYS","Open Ephys",nullptr);
    EngineParameter* param;
    param = new EngineParameter(EngineParameter::BOOL,0,"Separate Files",false);
    man->addParameter(param);
    param = new EngineParameter(EngineParameter::BOOL, 1, "Rename files based on channel order", false);
    man->addParameter(param);
    param = new EngineParameter(EngineParameter::STR, 2, "Renamed files prefix", "CH");
    man->addParameter(param);
    return man;
}