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

#include "Parameter.h"

ParameterCollection::ParameterCollection(InfoObject* object)
{
    for (auto parameter : object->getParameters())
        addParameter(parameter);
}

ParameterCollection::~ParameterCollection()
{
}

void ParameterCollection::copyParametersTo(InfoObject* obj)
{
    for (auto parameter : parameters)
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            obj->addParameter(new IntParameter(*p));
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            obj->addParameter(new BooleanParameter(*p));
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            obj->addParameter(new StringParameter(*p));
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            obj->addParameter(new SelectedChannelsParameter(*p));
            
        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            obj->addParameter(new CategoricalParameter(*p));
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            obj->addParameter(new FloatParameter(*p));
        }
            
    }
}

void ParameterCollection::copyParametersFrom(InfoObject* obj)
{
    for (auto parameter : obj->getParameters())
    {
        if (parameter->getType() == Parameter::INT_PARAM)
        {
            IntParameter* p = (IntParameter*) parameter;
            addParameter(new IntParameter(*p));
        }
        else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
        {
            BooleanParameter* p = (BooleanParameter*) parameter;
            addParameter(new BooleanParameter(*p));
        }
        else if (parameter->getType() == Parameter::STRING_PARAM)
        {
            StringParameter* p = (StringParameter*) parameter;
            addParameter(new StringParameter(*p));
        }
        else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
        {
            SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;
            addParameter(new SelectedChannelsParameter(*p));
            
        }
        else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
        {
            CategoricalParameter* p = (CategoricalParameter*) parameter;
            addParameter(new CategoricalParameter(*p));
        }
        else if (parameter->getType() == Parameter::FLOAT_PARAM)
        {
            FloatParameter* p = (FloatParameter*) parameter;
            addParameter(new FloatParameter(*p));
        }
            
    }
}

void ParameterCollection::addParameter(Parameter* p)
{
    parameters.add(p);
    
    std::cout << "Adding parameter with name " << p->getName() << std::endl;
    
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
