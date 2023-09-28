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

#include "../Editors/GenericEditor.h"

DataThread::DataThread (SourceNode* s_)
    : Thread     ("Data Thread"),
      sn(s_)
{

}


DataThread::~DataThread()
{
}


void DataThread::addBooleanParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool defaultValue,
    bool deactivateDuringAcquisition)
{
    sn->addBooleanParameter(scope, name, displayName, description, defaultValue, deactivateDuringAcquisition);
}

void DataThread::addIntParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int defaultValue,
    int minValue,
    int maxValue,
    bool deactivateDuringAcquisition)
{
    sn->addIntParameter(scope, name, displayName, description, defaultValue, minValue, maxValue, deactivateDuringAcquisition);
}

void DataThread::addStringParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    String defaultValue,
    bool deactivateDuringAcquisition)
{
    sn->addStringParameter(scope, name, displayName, description, defaultValue, deactivateDuringAcquisition);
}

void DataThread::addFloatParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& unit,
    float defaultValue,
    float minValue,
    float maxValue,
    float stepSize,
    bool deactivateDuringAcquisition)
{
    sn->addFloatParameter(scope, name, displayName, description, unit, defaultValue, minValue, maxValue, stepSize, deactivateDuringAcquisition);
}

void DataThread::addSelectedChannelsParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int maxSelectedChannels,
    bool deactivateDuringAcquisition)
{
    sn->addSelectedChannelsParameter(scope, name, displayName, description, maxSelectedChannels, deactivateDuringAcquisition);
}

void DataThread::addMaskChannelsParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
{
    sn->addMaskChannelsParameter(scope, name, displayName, description, deactivateDuringAcquisition);
}

void DataThread::addCategoricalParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> categories,
    int defaultIndex,
    bool deactivateDuringAcquisition)
{
    sn->addCategoricalParameter(scope, name, displayName, description, categories, defaultIndex, deactivateDuringAcquisition);
}

void DataThread::addPathParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    const StringArray& validFileExtensions,
    bool isDirectory,
    bool deactivateDuringAcquisition)
{
    sn->addPathParameter(scope, name, displayName, description, defaultValue, validFileExtensions, isDirectory, deactivateDuringAcquisition);
}

void DataThread::addSelectedStreamParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> streamNames,
    const int defaultIndex,
    bool deactivateDuringAcquisition)
{
    sn->addSelectedStreamParameter(scope, name, displayName, description, streamNames, defaultIndex, deactivateDuringAcquisition);
}

void DataThread::addTimeParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    bool deactivateDuringAcquisition)
{
    sn->addTimeParameter(scope, name, displayName, description, defaultValue, deactivateDuringAcquisition);
}

void DataThread::addNotificationParameter(Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
{
    sn->addNotificationParameter(scope, name, displayName, description, deactivateDuringAcquisition);
}

Parameter* DataThread::getParameter(String name) const
{
    return sn->getParameter(name);
}

bool DataThread::hasParameter(String name) const
{
    return sn->hasParameter(name);
}

Array<Parameter*> DataThread::getParameters() 
{
    return sn->getParameters();
}


void DataThread::run()
{
    setPriority (Thread::Priority::highest);
    
    while (! threadShouldExit())
    {
        if (! updateBuffer())
        {
            const MessageManagerLock mmLock (Thread::getCurrentThread());

            LOGE("Aquisition error...stopping thread.");
            signalThreadShouldExit();
            LOGE("Notifying source node to stop acqusition.");
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
