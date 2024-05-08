/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#include "ParameterOwner.h"


//ParameterOwnerCommon
ParameterOwner::ParameterOwner(ParameterOwner::Type type)
	:	m_type(type)
{
}

ParameterOwner::~ParameterOwner()
{}

ParameterOwner::ParameterOwner(const ParameterOwner& other) :
    m_type(other.m_type)
{
}

const ParameterOwner::Type ParameterOwner::getType() const
{
	return m_type;
}

void ParameterOwner::copyParameters(ParameterOwner* object)
{

    if (object->getType() != getType())
    {
        std::cout << "Cannot copy parameters between different types of ParameterOwners" << std::endl;
        return;
    }

    for (auto parameter : object->getParameters())
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

void ParameterOwner::addParameter(Parameter* p)
{

    p->setOwner(this);
    parameters.addParameter(p);
}

Parameter* ParameterOwner::getParameter(String name) const
{
    if(parameters.contains(name))
        return parameters[name];
    else
        return nullptr;
}

Array<String> ParameterOwner::getParameterNames() const
{
    return parameters.getParameterNames();
}

var ParameterOwner::operator [](String name) const
{
    if (parameters.contains(name))
        return parameters[name]->getValue();
    else
        return var::undefined();
}


Array<ParameterEditor*> ParameterOwner::createDefaultEditor()
{
    
    Array<ParameterEditor*> editors;

    for (auto parameterName : getParameterNames())
    {
		Parameter* parameter = getParameter(parameterName);
        
        switch (parameter->getType())
        {
        case Parameter::INT_PARAM:
        {
            if (parameter->getParameterEditorType() == Parameter::ParameterEditorType::COMBOBOX_EDITOR)
            {
                ComboBoxParameterEditor* intParameterEditor = new ComboBoxParameterEditor(parameter);
                editors.add(intParameterEditor);
            }
            else if (parameter->getParameterEditorType() == Parameter::ParameterEditorType::BOUNDED_VALUE_EDITOR)
            {
                BoundedValueParameterEditor* intParameterEditor = new BoundedValueParameterEditor(parameter);
                editors.add(intParameterEditor);
            }
            else if (parameter->getParameterEditorType() == Parameter::ParameterEditorType::SELECTED_STREAM_EDITOR)
            {
                SelectedStreamParameterEditor* intParameterEditor = new SelectedStreamParameterEditor(parameter);
                editors.add(intParameterEditor);
            }
            else
            {
                TextBoxParameterEditor* intParameterEditor = new TextBoxParameterEditor(parameter);
                editors.add(intParameterEditor);
            }
            break;
        }
        case Parameter::FLOAT_PARAM:
        {
            if (parameter->getParameterEditorType() == Parameter::ParameterEditorType::BOUNDED_VALUE_EDITOR)
            {
                BoundedValueParameterEditor* floatParameterEditor = new BoundedValueParameterEditor(parameter);
                editors.add(floatParameterEditor);
            }
            else
            {
                TextBoxParameterEditor* floatParameterEditor = new TextBoxParameterEditor(parameter);
                editors.add(floatParameterEditor);
            }
            break;
        }
        case Parameter::BOOLEAN_PARAM:
        {
            if (parameter->getName() != "enable_stream")
            {
                ToggleParameterEditor* booleanParameterEditor = new ToggleParameterEditor(parameter);
                editors.add(booleanParameterEditor);
            }
            break;
        }
        case Parameter::CATEGORICAL_PARAM:
        {
            ComboBoxParameterEditor* categoricalParameterEditor = new ComboBoxParameterEditor(parameter);
            editors.add(categoricalParameterEditor);
            break;
        }
        case Parameter::SELECTED_CHANNELS_PARAM:
        {
            SelectedChannelsParameterEditor* selectedChannelsParameterEditor = new SelectedChannelsParameterEditor(parameter);
            editors.add(selectedChannelsParameterEditor);
            break;
        }
        case Parameter::MASK_CHANNELS_PARAM:
        {
            MaskChannelsParameterEditor* maskChannelsParameterEditor = new MaskChannelsParameterEditor(parameter);
            editors.add(maskChannelsParameterEditor);
            break;
        }
        case Parameter::SELECTED_STREAM_PARAM:
        {
            SelectedStreamParameterEditor* selectedStreamParameterEditor = new SelectedStreamParameterEditor(parameter);
            editors.add(selectedStreamParameterEditor);
            break;
        }
        case Parameter::PATH_PARAM:
        {
            PathParameterEditor* pathParameterEditor = new PathParameterEditor(parameter);
            editors.add(pathParameterEditor);
            break;
        }
        case Parameter::TIME_PARAM:
        {
            TimeParameterEditor* timeParameterEditor = new TimeParameterEditor(parameter);
            editors.add(timeParameterEditor);
            break;
        }
        case Parameter::STRING_PARAM:
        {
            TextBoxParameterEditor* stringParameterEditor = new TextBoxParameterEditor(parameter);
            editors.add(stringParameterEditor);
            break;
        }
        case Parameter::TTL_LINE_PARAM:
        {
            TtlLineParameterEditor* ttlLineParameterEditor = new TtlLineParameterEditor(parameter);
            editors.add(ttlLineParameterEditor);
            break;
        }
        default:
        {
            break;
        }
            
        }
        
    }

     return editors;
    
}
