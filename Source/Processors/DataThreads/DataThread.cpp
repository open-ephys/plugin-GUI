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


DataThread::DataThread (SourceNode* s_)
    : Thread     ("Data Thread"),
      sn(s_)
{
    setPriority (10);
}


DataThread::~DataThread()
{
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
            sn->connectionLost();
        }
    }
}


DataBuffer* DataThread::getBufferAddress(int streamIdx) const
{
	return sourceBuffers[streamIdx];
}


std::unique_ptr<GenericEditor> DataThread::createEditor (SourceNode*)
{
    return nullptr;
}


void DataThread::broadcastMessage(String msg)
{
    sn->broadcastDataThreadMessage(msg);
}