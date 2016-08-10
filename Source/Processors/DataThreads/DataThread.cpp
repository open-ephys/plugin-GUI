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

#include "DataThread.h"
#include "../SourceNode/SourceNode.h"


DataThread::DataThread (SourceNode* s)
    : Thread     ("Data Thread")
    , dataBuffer (0)
{
    sn = s;
    setPriority (10);

    // set default to zero, so that sources that
    // do not generate their own timestamps can simply increment
    // this value
    timestamp = 0;
}


DataThread::~DataThread()
{
    //deleteAndZero(dataBuffer);
}


void DataThread::run()
{
    while (! threadShouldExit())
    {
        if (! updateBuffer())
        {
            const MessageManagerLock mmLock (Thread::getCurrentThread());

            std::cout << "Aquisition error...stopping thread." << std::endl;
            signalThreadShouldExit();
            std::cout << "Notifying source node to stop acqusition." << std::endl;
            sn->acquisitionStopped();
        }
    }
}


DataBuffer* DataThread::getBufferAddress() const
{
    std::cout << "Setting buffer address to " << dataBuffer << std::endl;

    return dataBuffer;
}


void DataThread::getChannelInfo (Array<ChannelCustomInfo>& infoArray) const
{
    infoArray.clear();
    infoArray.addArray (channelInfo);
}


void DataThread::updateChannels()
{
    if (usesCustomNames())
    {
        channelInfo.resize (sn->channels.size());
        setDefaultChannelNames();

        for (int i = 0; i < channelInfo.size(); ++i)
        {
            sn->channels[i]->setName (channelInfo[i].name);
            sn->channels[i]->bitVolts = channelInfo[i].gain;
        }
    }
}


void DataThread::setOutputHigh() {}
void DataThread::setOutputLow() {}

int DataThread::getNumAuxOutputs() const    { return 0; }
int DataThread::getNumAdcOutputs() const    { return 0; }
int DataThread::getNumEventChannels() const { return 0; }

void DataThread::getEventChannelNames (StringArray& names) const { }

bool DataThread::isReady() { return true; }


int DataThread::modifyChannelName (int channel, String newName)
{
    return -1;
}

int DataThread::modifyChannelGain (int channel, float gain)
{
    return -1;
}

bool DataThread::usesCustomNames() const
{
    return false;
}

void* DataThread::getDevice()
{
    return 0;
}

void DataThread::setDefaultChannelNames()
{
}

GenericEditor* DataThread::createEditor (SourceNode*)
{
    return nullptr;
}

bool DataThread::isDualSampleRate()
{
    return false;
}
