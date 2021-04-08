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
#include "../../Utils/Utils.h"


DataThread::DataThread (SourceNode* s)
    : Thread     ("Data Thread")
{
    sn = s;
    setPriority (10);

	int nSub = getNumSubProcessors();
	for (int i = 0; i < nSub; i++)
	{
		ttlEventWords.add(0);
		timestamps.add(0);
	}
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

            LOGD("Aquisition error...stopping thread.");
            signalThreadShouldExit();
            LOGD("Notifying source node to stop acqusition.");
            sn->acquisitionStopped();
        }
    }
}


DataBuffer* DataThread::getBufferAddress(int subProcessor) const
{

	return sourceBuffers[subProcessor];
}


void DataThread::getChannelInfo (Array<ChannelCustomInfo>& infoArray) const
{
    infoArray.clear();
    infoArray.addArray (channelInfo);
}


void DataThread::updateChannels()
{
	ttlEventWords.clear();
	timestamps.clear();
	int nSub = getNumSubProcessors();

	for (int i = 0; i < nSub; i++)
	{
		ttlEventWords.add(0);
		timestamps.add(0);
	}

    if (foundInputSource() && usesCustomNames())
    {
        channelInfo.resize (sn->getTotalDataChannels());
        setDefaultChannelNames();

        for (int i = 0; i < channelInfo.size(); ++i)
        {
			sn->setChannelInfo(i, channelInfo[i].name, channelInfo[i].gain);
        }
    }
}


void DataThread::setOutputHigh() {}
void DataThread::setOutputLow() {}

unsigned int DataThread::getNumSubProcessors() const { return 1; }

int DataThread::getNumTTLOutputs(int subproc) const { return 0; }

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

void DataThread::setDefaultChannelNames()
{
}

GenericEditor* DataThread::createEditor (SourceNode*)
{
    return nullptr;
}

void DataThread::createExtraEvents(Array<EventChannel*>&)
{}

void DataThread::resizeBuffers()
{}

String DataThread::getChannelUnits(int chanIndex) const
{
	return String::empty;
}