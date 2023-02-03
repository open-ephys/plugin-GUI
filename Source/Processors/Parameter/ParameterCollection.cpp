/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#include "ParameterCollection.h"
#include "../Settings/InfoObject.h"
#include "../Settings/DataStream.h"
#include "../Settings/ContinuousChannel.h"
#include "../Settings/SpikeChannel.h"
#include "../Settings/EventChannel.h"
#include "../Settings/DeviceInfo.h"

#include "Parameter.h"

ParameterCollection::ParameterCollection(InfoObject* object)
{
    for (auto parameter : object->getParameters())
        addParameter(parameter);

    owner.name = object->getName();
    
    if (object->getType() == InfoObject::DATASTREAM_INFO)
    {
        DataStream* s = (DataStream*)object;
        owner.channel_count = s->getChannelCount();
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();

        if (s->hasDevice())
            owner.deviceName = s->device->getName();
    } 
    else if (object->getType() == InfoObject::CONTINUOUS_CHANNEL)
    {
        ContinuousChannel* s = (ContinuousChannel*)object;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    } 
    else if (object->getType() == InfoObject::SPIKE_CHANNEL)
    {
        SpikeChannel* s = (SpikeChannel*)object;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (object->getType() == InfoObject::EVENT_CHANNEL)
    {
        EventChannel* s = (EventChannel*)object;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }

}

ParameterCollection::~ParameterCollection()
{
}


void ParameterCollection::copyParameterValuesTo(InfoObject* obj)
{
    for (auto parameter : parameters)
    {
        if (obj->hasParameter(parameter->getName()))
            obj->getParameter(parameter->getName())->setNextValue(parameter->getValue());
    }
}

void ParameterCollection::copyParametersTo(InfoObject* obj)
{
    int channelCount = -1;
    
    if (obj->getType() == InfoObject::DATASTREAM_INFO)
    {
        DataStream* s = (DataStream*)obj;
		channelCount = s->getChannelCount();
    }
    
    obj->parameters.clear();
    
    for (auto parameter : parameters)
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            obj->addParameter(new IntParameter(*p));
            setParameterOwner(p, obj);
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            obj->addParameter(new BooleanParameter(*p));
            setParameterOwner(p, obj);
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            obj->addParameter(new StringParameter(*p));
            setParameterOwner(p, obj);
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            
            if (channelCount != -1)
                p->setChannelCount(channelCount);

            obj->addParameter(new SelectedChannelsParameter(*p));
            setParameterOwner(p, obj);
            
        }
        else if (parameter->getType() == Parameter::MASK_CHANNELS_PARAM)
        {
            MaskChannelsParameter* p = (MaskChannelsParameter*) parameter;

            if (channelCount != -1)
                p->setChannelCount(channelCount);
            
            obj->addParameter(new MaskChannelsParameter(*p));
            setParameterOwner(p, obj);
            
        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            obj->addParameter(new CategoricalParameter(*p));
            setParameterOwner(p, obj);
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            obj->addParameter(new FloatParameter(*p));
            setParameterOwner(p, obj);
        }
        
 
    }
}

void ParameterCollection::setParameterOwner(Parameter* p, InfoObject* obj)
{
    
    if (obj->getType() == InfoObject::DATASTREAM_INFO)
        p->setDataStream((DataStream*) obj);
    else if (obj->getType() == InfoObject::SPIKE_CHANNEL)
        p->setSpikeChannel((SpikeChannel*) obj);
    else if (obj->getType() == InfoObject::CONTINUOUS_CHANNEL)
        p->setContinuousChannel((ContinuousChannel*) obj);
    else if (obj->getType() == InfoObject::EVENT_CHANNEL)
        p->setEventChannel((EventChannel*) obj);
       
}

void ParameterCollection::clearParameterOwner(Parameter* p)
{
    p->setDataStream(nullptr);
    p->setContinuousChannel(nullptr);
    p->setEventChannel(nullptr);
    p->setSpikeChannel(nullptr);
}

void ParameterCollection::copyParametersFrom(InfoObject* obj)
{
    
    clear();
    
    for (auto parameter : obj->getParameters())
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            addParameter(new IntParameter(*p));
            clearParameterOwner(p);
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            addParameter(new BooleanParameter(*p));
            clearParameterOwner(p);
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            addParameter(new StringParameter(*p));
            clearParameterOwner(p);
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            addParameter(new SelectedChannelsParameter(*p));
            clearParameterOwner(p);
            
        }
        else if (parameter->getType() == Parameter::MASK_CHANNELS_PARAM)
        {
            MaskChannelsParameter* p = (MaskChannelsParameter*) parameter;
            addParameter(new MaskChannelsParameter(*p));
            clearParameterOwner(p);
            
        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            addParameter(new CategoricalParameter(*p));
            clearParameterOwner(p);
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            addParameter(new FloatParameter(*p));
            clearParameterOwner(p);
        }
            
    }

    if (obj->getType() == InfoObject::DATASTREAM_INFO)
    {
        DataStream* s = (DataStream*)obj;
        owner.channel_count = s->getChannelCount();
        owner.sample_rate = s->getSampleRate();
        owner.name = s->getName();
        owner.streamId = s->getStreamId();
        owner.sourceNodeId = s->getSourceNodeId();

        //if (s->hasDevice())
        //    owner.deviceName = s->device->getName();
    }
    else if (obj->getType() == InfoObject::CONTINUOUS_CHANNEL)
    {
        ContinuousChannel* s = (ContinuousChannel*)obj;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (obj->getType() == InfoObject::SPIKE_CHANNEL)
    {
        SpikeChannel* s = (SpikeChannel*)obj;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (obj->getType() == InfoObject::EVENT_CHANNEL)
    {
        EventChannel* s = (EventChannel*)obj;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
}

void ParameterCollection::addParameter(Parameter* p)
{

    if (parameterMap.find(p->getName()) != parameterMap.end()) {
        for (int i = 0; i < parameters.size(); i++) {
			if (parameters[i]->getName() == p->getName()) {
				parameters.remove(i);
				break;
			}
        }   
    }
    
    parameters.add(p);
    parameterMap[p->getName()] = p;
}

Array<Parameter*> ParameterCollection::getParameters()
{
    Array<Parameter*> params;
    
    for (auto param : parameters)
        params.add(param);
        
    return params;
}


void ParameterCollection::clear()
{
    parameters.clear();
    parameterMap.clear();
}
