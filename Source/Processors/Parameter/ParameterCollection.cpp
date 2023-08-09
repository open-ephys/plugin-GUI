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
#include "ParameterOwner.h"
#include "../Settings/DataStream.h"
#include "../Settings/ContinuousChannel.h"
#include "../Settings/SpikeChannel.h"
#include "../Settings/EventChannel.h"
#include "../Settings/DeviceInfo.h"

#include "Parameter.h"

ParameterCollection::ParameterCollection(ParameterOwner* pOwner)
{
    for (auto parameter : pOwner->getParameters())
        addParameter(parameter);
    
    if (pOwner->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* s = (DataStream*)pOwner;
        owner.name = s->getName();
        owner.channel_count = s->getChannelCount();
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();

        if (s->hasDevice())
            owner.deviceName = s->device->getName();
    } 
    else if (pOwner->getType() == ParameterOwner::CONTINUOUS_CHANNEL)
    {
        ContinuousChannel* s = (ContinuousChannel*)pOwner;
        owner.name = s->getName();
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    } 
    else if (pOwner->getType() == ParameterOwner::SPIKE_CHANNEL)
    {
        SpikeChannel* s = (SpikeChannel*)pOwner;
        owner.name = s->getName();
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (pOwner->getType() == ParameterOwner::EVENT_CHANNEL)
    {
        EventChannel* s = (EventChannel*)pOwner;
        owner.name = s->getName();
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }

}

ParameterCollection::~ParameterCollection()
{
}


void ParameterCollection::copyParameterValuesTo(ParameterOwner* pOwner)
{
    for (auto parameter : parameters)
    {
        if (pOwner->hasParameter(parameter->getName()))
            pOwner->getParameter(parameter->getName())->currentValue = parameter->getValue();
    }
}

void ParameterCollection::copyParametersTo(ParameterOwner* pOwner)
{
    int channelCount = -1;
    
    if (pOwner->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* s = (DataStream*)pOwner;
		channelCount = s->getChannelCount();
    }
    
    pOwner->parameters.clear();
    
    for (auto parameter : parameters)
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            pOwner->addParameter(new IntParameter(*p));
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            pOwner->addParameter(new BooleanParameter(*p));
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            pOwner->addParameter(new StringParameter(*p));
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            SelectedChannelsParameter* p2 = new SelectedChannelsParameter(*p);
            
            pOwner->addParameter(p2);

            if (channelCount != -1)
                p2->setChannelCount(channelCount);
            
        }
        else if (parameter->getType() == Parameter::MASK_CHANNELS_PARAM)
        {
            MaskChannelsParameter* p = (MaskChannelsParameter*) parameter;
            MaskChannelsParameter* p2 = new MaskChannelsParameter(*p);

            pOwner->addParameter(p2);

            if (channelCount != -1)
                p2->setChannelCount(channelCount); 

        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            pOwner->addParameter(new CategoricalParameter(*p));
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            pOwner->addParameter(new FloatParameter(*p));
        }
        else if (parameter->getType() == Parameter::PATH_PARAM)
        {
            PathParameter* p = (PathParameter*) parameter;
            pOwner->addParameter(new PathParameter(*p));
        }
        else if (parameter->getType() == Parameter::SELECTED_STREAM_PARAM)
        {
            SelectedStreamParameter* p = (SelectedStreamParameter*) parameter;
            pOwner->addParameter(new SelectedStreamParameter(*p));
        }
        else if (parameter->getType() == Parameter::TIME_PARAM)
        {
            TimeParameter* p = (TimeParameter*) parameter;
            pOwner->addParameter(new TimeParameter(*p));
        }
 
    }
}

void ParameterCollection::copyParametersFrom(ParameterOwner* pOwner)
{
    
    clear();
    
    for (auto parameter : pOwner->getParameters())
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            IntParameter* p2 = new IntParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            BooleanParameter* p2 = new BooleanParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            StringParameter* p2 = new StringParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            SelectedChannelsParameter* p2 = new SelectedChannelsParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
            
        }
        else if (parameter->getType() == Parameter::MASK_CHANNELS_PARAM)
        {
            MaskChannelsParameter* p = (MaskChannelsParameter*) parameter;
            MaskChannelsParameter* p2 = new MaskChannelsParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
            
        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            CategoricalParameter* p2 = new CategoricalParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            FloatParameter* p2 = new FloatParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::PATH_PARAM)
        {
            PathParameter* p = (PathParameter*) parameter;
            PathParameter* p2 = new PathParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::SELECTED_STREAM_PARAM)
        {
            SelectedStreamParameter* p = (SelectedStreamParameter*) parameter;
            SelectedStreamParameter* p2 = new SelectedStreamParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
        else if (parameter->getType() == Parameter::TIME_PARAM)
        {
            TimeParameter* p = (TimeParameter*) parameter;
            TimeParameter* p2 = new TimeParameter(*p);
            p2->setOwner(nullptr);
            addParameter(p2);
        }
            
    }

    if (pOwner->getType() == ParameterOwner::DATASTREAM)
    {
        DataStream* s = (DataStream*)pOwner;
        owner.channel_count = s->getChannelCount();
        owner.sample_rate = s->getSampleRate();
        owner.name = s->getName();
        owner.streamId = s->getStreamId();
        owner.sourceNodeId = s->getSourceNodeId();

        //if (s->hasDevice())
        //    owner.deviceName = s->device->getName();
    }
    else if (pOwner->getType() == ParameterOwner::CONTINUOUS_CHANNEL)
    {
        ContinuousChannel* s = (ContinuousChannel*)pOwner;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (pOwner->getType() == ParameterOwner::SPIKE_CHANNEL)
    {
        SpikeChannel* s = (SpikeChannel*)pOwner;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
    else if (pOwner->getType() == ParameterOwner::EVENT_CHANNEL)
    {
        EventChannel* s = (EventChannel*)pOwner;
        owner.sample_rate = s->getSampleRate();
        owner.streamId = s->getStreamId();
        owner.name = s->getName();
        owner.sourceNodeId = s->getSourceNodeId();
    }
}

void ParameterCollection::addParameter(Parameter* p)
{

    // if (parameterMap.find(p->getName()) != parameterMap.end()) {
    //     for (int i = 0; i < parameters.size(); i++) {
	// 		if (parameters[i]->getName() == p->getName()) {
	// 			parameters.remove(i);
	// 			break;
	// 		}
    //     }   
    // }
    
    if (!parameters.contains(p))
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


Array<String> ParameterCollection::getParameterNames() const
{
    Array<String> names;

    for (auto param : parameters)
        names.add(param->getName());

    return names;
}


void ParameterCollection::clear()
{
    parameters.clear();
    parameterMap.clear();
}
