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

#include "SpikeDisplayNode.h"
#include "RecordNode.h"
#include "Visualization/SpikeDisplayCanvas.h"
#include "Channel.h"

#include <stdio.h>


SpikeDisplayNode::SpikeDisplayNode()
    : GenericProcessor("Spike Viewer"), displayBufferSize(5),  redrawRequested(false), isRecording(false),
      signalFilesShouldClose(false)
{


    spikeBuffer = new uint8_t[MAX_SPIKE_BUFFER_LEN]; // MAX_SPIKE_BUFFER_LEN defined in SpikeObject.h

    recordingNumber = -1;

}

SpikeDisplayNode::~SpikeDisplayNode()
{

}

AudioProcessorEditor* SpikeDisplayNode::createEditor()
{
    //std::cout<<"Creating SpikeDisplayCanvas."<<std::endl;

    editor = new SpikeDisplayEditor(this);
    return editor;

}

void SpikeDisplayNode::updateSettings()
{
    //std::cout << "Setting num inputs on SpikeDisplayNode to " << getNumInputs() << std::endl;

    electrodes.clear();

    for (int i = 0; i < eventChannels.size(); i++)
    {
        if ((eventChannels[i]->eventType < 999) && (eventChannels[i]->eventType > SPIKE_BASE_CODE))
        {

            Electrode elec;
            elec.numChannels = eventChannels[i]->eventType - 100;
            elec.name = eventChannels[i]->name;
            elec.currentSpikeIndex = 0;
            elec.mostRecentSpikes.ensureStorageAllocated(displayBufferSize);

            for (int j = 0; j < elec.numChannels; j++)
            {
                elec.displayThresholds.add(0);
                elec.detectorThresholds.add(0);
            }

            electrodes.add(elec);
        }
    }

    recordNode = getProcessorGraph()->getRecordNode();
    diskWriteLock = recordNode->getLock();

}

// void SpikeDisplayNode::updateVisualizer()
// {

// }

bool SpikeDisplayNode::enable()
{
    std::cout << "SpikeDisplayNode::enable()" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->enable();
    return true;

}

bool SpikeDisplayNode::disable()
{
    std::cout << "SpikeDisplayNode disabled!" << std::endl;
    SpikeDisplayEditor* editor = (SpikeDisplayEditor*) getEditor();
    editor->disable();
    return true;
}

int SpikeDisplayNode::getNumberOfChannelsForElectrode(int i)
{
    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i].numChannels;
    }
    else
    {
        return 0;
    }
}

String SpikeDisplayNode::getNameForElectrode(int i)
{

    if (i > -1 && i < electrodes.size())
    {
        return electrodes[i].name;
    }
    else
    {
        return " ";
    }
}

void SpikeDisplayNode::addSpikePlotForElectrode(SpikePlot* sp, int i)
{
    Electrode& e = electrodes.getReference(i);
    e.spikePlot = sp;

}

void SpikeDisplayNode::removeSpikePlots()
{
    for (int i = 0; i < getNumElectrodes(); i++)
    {
        Electrode& e = electrodes.getReference(i);
        e.spikePlot = nullptr;
    }
}

int SpikeDisplayNode::getNumElectrodes()
{
    return electrodes.size();

}

void SpikeDisplayNode::startRecording()
{

    setParameter(1, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}

void SpikeDisplayNode::stopRecording()
{
    setParameter(0, 0.0f); // need to use the 'setParameter' method to interact with 'process'
}


void SpikeDisplayNode::setParameter(int param, float val)
{
    //std::cout<<"SpikeDisplayNode got Param:"<< param<< " with value:"<<val<<std::endl;

    if (param == 0) // stop recording
    {
        isRecording = false;
        signalFilesShouldClose = true;

    }
    else if (param == 1)   // start recording
    {
        isRecording = true;

        dataDirectory = recordNode->getDataDirectory();

        if (dataDirectory.getFullPathName().length() == 0)
        {
            // temporary fix in case nothing is returned by the record node.
            dataDirectory = File::getSpecialLocation(File::userHomeDirectory);
        }

        baseDirectory = dataDirectory.getFullPathName();

        for (int i = 0; i < getNumElectrodes(); i++)
        {
            openFile(i);
        }

    }
    else if (param == 2)   // redraw
    {
        redrawRequested = true;

    }

}



void SpikeDisplayNode::process(AudioSampleBuffer& buffer, MidiBuffer& events, int& nSamples)
{

    checkForEvents(events); // automatically calls 'handleEvent

    if (signalFilesShouldClose)
    {
        for (int i = 0; i < getNumElectrodes(); i++)
        {
            closeFile(i);
        }

        signalFilesShouldClose = false;
    }

    if (redrawRequested)
    {
        // update incoming thresholds
        for (int i = 0; i < getNumElectrodes(); i++)
        {

            Electrode& e = electrodes.getReference(i);

            // update thresholds
            for (int j = 0; j < e.numChannels; j++)
            {
                e.displayThresholds.set(j,
                                        e.spikePlot->getDisplayThresholdForChannel(j));

                e.spikePlot->setDetectorThresholdForChannel(j, e.detectorThresholds[j]);
            }

            // transfer buffered spikes to spike plot
            for (int j = 0; j < e.currentSpikeIndex; j++)
            {
                //std::cout << "Transferring spikes." << std::endl;
                e.spikePlot->processSpikeObject(e.mostRecentSpikes[j]);
                e.currentSpikeIndex = 0;
            }

        }

        redrawRequested = false;
    }

}

void SpikeDisplayNode::handleEvent(int eventType, MidiMessage& event, int samplePosition)
{

    //std::cout << "Received event of type " << eventType << std::endl;

    if (eventType == SPIKE)
    {

        const uint8_t* dataptr = event.getRawData();
        int bufferSize = event.getRawDataSize();

        if (bufferSize > 0)
        {

            SpikeObject newSpike;

            bool isValid = unpackSpike(&newSpike, dataptr, bufferSize);

            if (isValid)
            {
                int electrodeNum = newSpike.source;

                Electrode& e = electrodes.getReference(electrodeNum);
                // std::cout << electrodeNum << std::endl;

                bool aboveThreshold = false;

                // update threshold / check threshold
                for (int i = 0; i < e.numChannels; i++)
                {
                    e.detectorThresholds.set(i, float(newSpike.threshold[i])); // / float(newSpike.gain[i]));

                    aboveThreshold = aboveThreshold | checkThreshold(i, e.displayThresholds[i], newSpike);
                }

                if (aboveThreshold)
                {

                    // add to buffer
                    if (e.currentSpikeIndex < displayBufferSize)
                    {
                        //  std::cout << "Adding spike " << e.currentSpikeIndex + 1 << std::endl;
                        e.mostRecentSpikes.set(e.currentSpikeIndex, newSpike);
                        e.currentSpikeIndex++;
                    }

                    // save spike
                    if (isRecording)
                    {
                        writeSpike(newSpike, electrodeNum);
                    }
                }

            }

        }

    }

}

bool SpikeDisplayNode::checkThreshold(int chan, float thresh, SpikeObject& s)
{
    int sampIdx = s.nSamples*chan;

    for (int i = 0; i < s.nSamples-1; i++)
    {

        if (float(s.data[sampIdx]-32768)/float(*s.gain)*1000.0f > thresh)
        {
            return true;
        }

        sampIdx++;
    }

    return false;
}

void SpikeDisplayNode::openFile(int i)
{

    String filename = baseDirectory;
    filename += File::separator;
    filename += getNameForElectrode(i).removeCharacters(" ");
    filename += ".spikes";

    std::cout << "OPENING FILE: " << filename << std::endl;

    File fileToUse = File(filename);

    diskWriteLock->enter();
    //const MessageManagerLock mmLock;

    Electrode& e = electrodes.getReference(i);

    FILE* file;

    if (!fileToUse.exists())
    {
        // open it and write header

        if (i == 0)
        {
            recordingNumber = 0;
        }

        file = fopen(filename.toUTF8(), "ab");
        String header = generateHeader(i);
        fwrite(header.toUTF8(), 1, header.getNumBytesAsUTF8(), file);

    }
    else
    {
        // append it
        if (i == 0)
        {
            recordingNumber++;
        }

        file = fopen(filename.toUTF8(), "ab");
    }

    diskWriteLock->exit();

    e.file = file;
}

void SpikeDisplayNode::closeFile(int i)
{

    Electrode& e = electrodes.getReference(i);

    std::cout << "CLOSING FILE for " << e.name << std::endl;

    diskWriteLock->enter();

    if (e.file != NULL)
    {
        fclose(e.file);
    }

    diskWriteLock->exit();

}

void SpikeDisplayNode::writeSpike(const SpikeObject& s, int i)
{

    packSpike(&s, spikeBuffer, MAX_SPIKE_BUFFER_LEN);

    int totalBytes = s.nSamples * s.nChannels * 2 + // account for samples
                     s.nChannels * 4 +            // acount for threshold and gain
                     15;                        // 15 bytes in every SpikeObject


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

    diskWriteLock->enter();

    fwrite(spikeBuffer, 1, totalBytes, electrodes[i].file);

    fwrite(&recordingNumber,                         // ptr
           2,                               // size of each element
           1,                               // count
           electrodes[i].file); // ptr to FILE object

    diskWriteLock->exit();


}

String SpikeDisplayNode::generateHeader(int electrodeNum)
{
    String header = "header.format = 'Open Ephys Data Format'; \n";
    header += "header.version = 0.2;";
    header += "header.header_bytes = ";
    header += String(HEADER_SIZE);
    header += ";\n";

    header += "header.description = 'Each record contains 1 uint8 eventType, 1 uint64 timestamp, 1 uint16 electrodeID, 1 uint16 numChannels (n), 1 uint16 numSamples (m), n*m uint16 samples, n uint16 channelGains, n uint16 thresholds, and 1 uint16 recordingNumber'; \n";

    header += "header.date_created = '";
    header += recordNode->generateDateString();
    header += "';\n";

    header += "header.electrode = '";
    header += electrodes[electrodeNum].name;
    header += "';\n";

    header += "header.num_channels = ";
    header += electrodes[electrodeNum].numChannels;
    header += ";\n";

    header += "header.sampleRate = ";
    header += String(settings.sampleRate);
    header += ";\n";

    header = header.paddedRight(' ', HEADER_SIZE);

    //std::cout << header << std::endl;

    return header;
}