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

#include "SourceNode.h"
#include "DataThreads/DataBuffer.h"
//#include "DataThreads/IntanThread.h"
#include "DataThreads/FPGAThread.h"
#include "DataThreads/FileReaderThread.h"
#include "DataThreads/RHD2000Thread.h"
#include "Editors/SourceNodeEditor.h"
#include "Editors/FileReaderEditor.h"
#include "Editors/RHD2000Editor.h"
#include "Channel.h"
#include <stdio.h>

SourceNode::SourceNode(const String& name_)
    : GenericProcessor(name_),
      sourceCheckInterval(2000), wasDisabled(true), dataThread(0),
      inputBuffer(0), ttlState(0)
{

    std::cout << "creating source node." << std::endl;

    if (getName().equalsIgnoreCase("RHA2000-EVAL"))
    {
        // dataThread = new IntanThread(this);
    }
    else if (getName().equalsIgnoreCase("Custom FPGA"))
    {
        dataThread = new FPGAThread(this);
    }
    else if (getName().equalsIgnoreCase("File Reader"))
    {
        dataThread = new FileReaderThread(this);
    }
    else if (getName().equalsIgnoreCase("Rhythm FPGA"))
    {
        dataThread = new RHD2000Thread(this);
    }

    if (dataThread != 0)
    {
        if (!dataThread->foundInputSource())
        {
            enabledState(false);
        }

        numEventChannels = dataThread->getNumEventChannels();
        eventChannelState = new int[numEventChannels];
        for (int i = 0; i < numEventChannels; i++)
        {
            eventChannelState[i] = 0;
        }

    }
    else
    {
        enabledState(false);
        eventChannelState = 0;
        numEventChannels = 0;
    }

    // check for input source every few seconds
    startTimer(sourceCheckInterval);

    timestamp = 0;
    eventCodeBuffer = new int16[10000]; //10000 samples per buffer max?


}

SourceNode::~SourceNode()
{

    if (dataThread->isThreadRunning())
    {
        std::cout << "Forcing thread to stop." << std::endl;
        dataThread->stopThread(500);
    }


    if (eventChannelState)
        delete[] eventChannelState;
}

DataThread* SourceNode::getThread()
{
    return dataThread;
}

void SourceNode::updateSettings()
{
    if (inputBuffer == 0 && dataThread != 0)
    {

        inputBuffer = dataThread->getBufferAddress();
        dataThread->updateChannelNames();
        std::cout << "Input buffer address is " << inputBuffer << std::endl;
    }

    for (int i = 0; i < dataThread->getNumEventChannels(); i++)
    {
        Channel* ch = new Channel(this, i);
        ch->eventType = TTL;
        ch->isEventChannel = true;
        eventChannels.add(ch);
    }


}

void SourceNode::actionListenerCallback(const String& msg)
{

    //std::cout << msg << std::endl;

    if (msg.equalsIgnoreCase("HI"))
    {
        // std::cout << "HI." << std::endl;
        // dataThread->setOutputHigh();
        ttlState = 1;
    }
    else if (msg.equalsIgnoreCase("LO"))
    {
        // std::cout << "LO." << std::endl;
        // dataThread->setOutputLow();
        ttlState = 0;
    }
}

int SourceNode::getTTLState()
{
    return ttlState;
}

float SourceNode::getSampleRate()
{

    if (dataThread != 0)
        return dataThread->getSampleRate();
    else
        return 44100.0;
}

float SourceNode::getDefaultSampleRate()
{
    if (dataThread != 0)
        return dataThread->getSampleRate();
    else
        return 44100.0;
}

int SourceNode::getDefaultNumOutputs()
{
    if (dataThread != 0)
        return dataThread->getNumChannels();
    else
        return 0;
}

float SourceNode::getDefaultBitVolts()
{
    if (dataThread != 0)
        return dataThread->getBitVolts();
    else
        return 1.0f;
}

void SourceNode::enabledState(bool t)
{
    if (t && !dataThread->foundInputSource())
    {
        isEnabled = false;
    }
    else
    {
        isEnabled = t;
    }

}

void SourceNode::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
    //std::cout << "Got parameter change notification";
}

AudioProcessorEditor* SourceNode::createEditor()
{

    if (getName().equalsIgnoreCase("Rhythm FPGA"))
    {
        editor = new RHD2000Editor(this, (RHD2000Thread*) dataThread.get(), true);

        //  RHD2000Editor* r2e = (RHD2000Editor*) editor.get();
        //  r2e->scanPorts();
    }
    //  else if (getName().equalsIgnoreCase("File Reader"))
    //  {
    //     editor = new FileReaderEditor(this, (FileReaderThread*) dataThread.get(), true);
    // }
    else
    {
        editor = new SourceNodeEditor(this, true);
    }
    return editor;
}

bool SourceNode::tryEnablingEditor()
{
    if (!isReady())
    {
        //std::cout << "No input source found." << std::endl;
        return false;
    }
    else if (isEnabled)
    {
        // If we're already enabled (e.g. if we're being called again
        // due to timerCallback()), then there's no need to go through
        // the editor again.
        return true;
    }

    std::cout << "Input source found." << std::endl;
    enabledState(true);
    GenericEditor* ed = getEditor();
    getEditorViewport()->makeEditorVisible(ed);
    return true;
}

void SourceNode::timerCallback()
{
    if (!tryEnablingEditor() && isEnabled)
    {
        std::cout << "Input source lost." << std::endl;
        enabledState(false);
        GenericEditor* ed = getEditor();
        getEditorViewport()->makeEditorVisible(ed);
    }
}

bool SourceNode::isReady()
{
    return dataThread && dataThread->foundInputSource();
}

bool SourceNode::enable()
{

    std::cout << "Source node received enable signal" << std::endl;

    wasDisabled = false;

    if (dataThread != 0)
    {
        dataThread->startAcquisition();
        return true;
    }
    else
    {
        return false;
    }

    stopTimer();

}

bool SourceNode::disable()
{

    std::cout << "Source node received disable signal" << std::endl;

    if (dataThread != 0)
        dataThread->stopAcquisition();

    startTimer(2000);

    wasDisabled = true;

    std::cout << "SourceNode returning true." << std::endl;

    return true;
}

void SourceNode::acquisitionStopped()
{
    //if (!dataThread->foundInputSource()) {

    if (!wasDisabled)
    {
        std::cout << "Source node sending signal to UI." << std::endl;
        getUIComponent()->disableCallbacks();
        enabledState(false);
        GenericEditor* ed = (GenericEditor*) getEditor();
        getEditorViewport()->makeEditorVisible(ed);
    }
    //}
}


void SourceNode::process(AudioSampleBuffer& buffer,
                         MidiBuffer& events,
                         int& nSamples)
{

    //std::cout << "SOURCE NODE" << std::endl;

    // clear the input buffers
    events.clear();
    buffer.clear();

    nSamples = inputBuffer->readAllFromBuffer(buffer, &timestamp, eventCodeBuffer, buffer.getNumSamples());

    //std::cout << *buffer.getReadPointer(0) << std::endl;

    //std::cout << "Source node timestamp: " << timestamp << std::endl;

    //std::cout << "Samples per buffer: " << nSamples << std::endl;

    uint8 data[8];
    memcpy(data, &timestamp, 8);

    // generate timestamp
    addEvent(events,    // MidiBuffer
             TIMESTAMP, // eventType
             0,         // sampleNum
             nodeId,    // eventID
             0,		 // eventChannel
             8,         // numBytes
             data   // data
            );

    // std::cout << (int) *(data + 7) << " " <<
    //                 (int) *(data + 6) << " " <<
    //                 (int) *(data + 5) << " " <<
    //                 (int) *(data + 4) << " " <<
    //                 (int) *(data + 3) << " " <<
    //                 (int) *(data + 2) << " " <<
    //                 (int) *(data + 1) << " " <<
    //                 (int) *(data + 0) << std::endl;


    // fill event buffer
    for (int i = 0; i < nSamples; i++)
    {
        for (int c = 0; c < numEventChannels; c++)
        {
            int state = eventCodeBuffer[i] & (1 << c);

            if (eventChannelState[c] != state)
            {
                if (state == 0)
                {

                    //std::cout << "OFF" << std::endl;
                    //std::cout << c << std::endl;
                    // signal channel state is OFF
                    addEvent(events, // MidiBuffer
                             TTL,    // eventType
                             i,      // sampleNum
                             0,	     // eventID
                             c		 // eventChannel
                            );
                }
                else
                {

                    // std::cout << "ON" << std::endl;
                    // std::cout << c << std::endl;

                    // signal channel state is ON
                    addEvent(events, // MidiBuffer
                             TTL,    // eventType
                             i,      // sampleNum
                             1,		 // eventID
                             c		 // eventChannel
                            );


                }

                eventChannelState[c] = state;
            }
        }
    }

}



void SourceNode::saveCustomParametersToXml(XmlElement* parentElement)
{

    if (getName().equalsIgnoreCase("File Reader"))
    {
        XmlElement* childNode = parentElement->createNewChildElement("FILENAME");
        FileReaderThread* thread = (FileReaderThread*) dataThread.get();
        childNode->setAttribute("path", thread->getFile());

    }

}

void SourceNode::loadCustomParametersFromXml()
{

    if (parametersAsXml != nullptr)
    {
        // use parametersAsXml to restore state

        forEachXmlChildElement(*parametersAsXml, xmlNode)
        {
            if (xmlNode->hasTagName("FILENAME"))
            {
                String filepath = xmlNode->getStringAttribute("path");
                FileReaderEditor* fre = (FileReaderEditor*) getEditor();
                fre->setFile(filepath);

            }
        }
    }

}