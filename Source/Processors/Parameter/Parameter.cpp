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


#include "Parameter.h"
#include "../GenericProcessor/GenericProcessor.h"

String Parameter::getParameterTypeString() const
{
    if (m_parameterType == Parameter::BOOLEAN_PARAM)
        return "Boolean";
    else if (m_parameterType == Parameter::INT_PARAM)
        return "Integer";
    else if (m_parameterType == Parameter::CATEGORICAL_PARAM)
        return "Categorical";
    else if (m_parameterType == Parameter::FLOAT_PARAM)
        return "Float";
    else if (m_parameterType == Parameter::SELECTED_CHANNELS_PARAM)
        return "Selected Channels";
    else if (m_parameterType == Parameter::MASK_CHANNELS_PARAM)
        return "Mask Channels";

    // This should never happen
    jassertfalse;
    return String();
}

uint16 Parameter::getStreamId()
{
    if (dataStream != nullptr)
        return dataStream->getStreamId();
    
    if (spikeChannel != nullptr)
        return spikeChannel->getStreamId();
    
    if (continuousChannel != nullptr)
        return continuousChannel->getStreamId();
        
    if (eventChannel != nullptr)
        return eventChannel->getStreamId();
    
    return 0;
        
}

BooleanParameter::BooleanParameter(GenericProcessor* processor,
    ParameterScope scope,
    const String& name,
    const String& description,
    bool defaultValue,
    bool deactivateDuringAcquisition)
    : Parameter(processor,
                ParameterType::BOOLEAN_PARAM,
                scope,
                name,
                description,
                defaultValue,
                deactivateDuringAcquisition)
{

}

void BooleanParameter::setNextValue(var newValue_)
{
    if (newValue_.isBool())
    {
        newValue = newValue_;
    }

    processor->parameterChangeRequest(this);
}

bool BooleanParameter::getBoolValue()
{
    return (bool)currentValue;
}

String BooleanParameter::getValueAsString()
{
    if ((bool) currentValue)
    {
        return "true";
    } else {
        return "false";
    }
}

void BooleanParameter::toXml(XmlElement* xml) 
{
    xml->setAttribute(getName(), (bool) currentValue);
}

void BooleanParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getBoolAttribute(getName(), defaultValue);
}

CategoricalParameter::CategoricalParameter(GenericProcessor* processor,
    ParameterScope scope,
    const String& name,
    const String& description,
    StringArray categories_,
    int defaultIndex,
    bool deactivateDuringAcquisition)
    : Parameter(processor,
        ParameterType::CATEGORICAL_PARAM,
        scope,
        name,
        description,
        defaultIndex,
        deactivateDuringAcquisition),
    categories(categories_)
{

}

void CategoricalParameter::setNextValue(var newValue_)
{
    newValue = (int) newValue_;
    
    processor->parameterChangeRequest(this);
}

int CategoricalParameter::getSelectedIndex()
{
    return (int)currentValue;
}

String CategoricalParameter::getSelectedString()
{
    return categories[currentValue];
}

String CategoricalParameter::getValueAsString()
{
    return getSelectedString();
}

const StringArray& CategoricalParameter::getCategories()
{
    return categories;
}


void CategoricalParameter::setCategories(StringArray categories_)
{
    categories = categories_;
}

void CategoricalParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (int)currentValue);
}

void CategoricalParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getIntAttribute(getName(), defaultValue);
}

IntParameter::IntParameter(GenericProcessor* processor,
    ParameterScope scope,
    const String& name,
    const String& description,
    int defaultValue_,
    int minValue_,
    int maxValue_,
    bool deactivateDuringAcquisition)
    : Parameter(processor,
        ParameterType::INT_PARAM,
        scope,
        name,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxValue(maxValue_),
    minValue(minValue_)
{

}

void IntParameter::setNextValue(var newValue_)
{

    int value = (int) newValue_;

    if (value < minValue)
        newValue = minValue;
    else if (value > maxValue)
        newValue = maxValue;
    else
        newValue = value;

    processor->parameterChangeRequest(this);
}

int IntParameter::getIntValue()
{
    return int(currentValue);
}

String IntParameter::getValueAsString()
{
    return String(getIntValue());
}

void IntParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (int) currentValue);
}

void IntParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getIntAttribute(getName(), defaultValue);
}

StringParameter::StringParameter(GenericProcessor* processor,
    ParameterScope scope,
    const String& name,
    const String& description,
    String defaultValue_,
    bool deactivateDuringAcquisition)
    : Parameter(processor,
        ParameterType::INT_PARAM,
        scope,
        name,
        description,
        defaultValue_,
        deactivateDuringAcquisition)
{

}

void StringParameter::setNextValue(var newValue_)
{
    newValue = newValue_.toString();

    processor->parameterChangeRequest(this);
}

String StringParameter::getStringValue()
{
    return currentValue.toString();
}

String StringParameter::getValueAsString()
{
    return getStringValue();
}

void StringParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), currentValue.toString());
}

void StringParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getStringAttribute(getName(), defaultValue);
}


FloatParameter::FloatParameter(GenericProcessor* processor,
    ParameterScope scope,
    const String& name,
    const String& description,
    float defaultValue_,
    float minValue_,
    float maxValue_,
    float stepSize_,
    bool deactivateDuringAcquisition)
    : Parameter(processor,
        ParameterType::FLOAT_PARAM,
        scope,
        name,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxValue(maxValue_),
    minValue(minValue_),
    stepSize(stepSize_)
{

}

void FloatParameter::setNextValue(var newValue_)
{
    if (newValue_.isDouble())
    {
        float value = (float) newValue_;

        if (value < minValue)
            newValue = minValue;
        else if (value > maxValue)
            newValue = maxValue;
        else
            newValue = value;

    }

    processor->parameterChangeRequest(this);
}

float FloatParameter::getFloatValue()
{
    return float(currentValue);
}

String FloatParameter::getValueAsString()
{
    return String(getFloatValue());
}

void FloatParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (float)currentValue);
}

void FloatParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getDoubleAttribute(getName(), defaultValue);
}

SelectedChannelsParameter::SelectedChannelsParameter(GenericProcessor* processor_,
    ParameterScope scope,
    const String& name,
    const String& description,
    Array<var> defaultValue_,
    int maxSelectableChannels_,
    bool deactivateDuringAcquisition)
    : Parameter(processor_,
        ParameterType::SELECTED_CHANNELS_PARAM,
        scope,
        name,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxSelectableChannels(maxSelectableChannels_),
    channelCount(0)
{
}

void SelectedChannelsParameter::setNextValue(var newValue_)
{

    if (newValue_.getArray()->size() <= maxSelectableChannels)
    {
        newValue = newValue_;
    }
    
    processor->parameterChangeRequest(this);
}

std::vector<bool> SelectedChannelsParameter::getChannelStates()
{
    std::vector<bool> states;

    for (int i = 0; i < channelCount; i++)
    {
        if (currentValue.getArray()->contains(i))
            states.push_back(true);
        else
            states.push_back(false);
    }

    return states;
}


Array<int> SelectedChannelsParameter::getArrayValue()
{
    Array<int> out;

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        out.add(currentValue[i]);
    }

    return out;
}

String SelectedChannelsParameter::getValueAsString()
{
    return selectedChannelsToString();
}

void SelectedChannelsParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), selectedChannelsToString());
}

void SelectedChannelsParameter::fromXml(XmlElement* xml)
{
    if (xml->hasAttribute(getName()))
        currentValue = parseSelectedString(xml->getStringAttribute(getName(), ""));
}


String SelectedChannelsParameter::selectedChannelsToString()
{

    String result = "";

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        result += String(int(currentValue[i]) + 1) + ",";
    }

    return result.substring(0, result.length() - 1);
}

Array<var> SelectedChannelsParameter::parseSelectedString(const String& input)
{

    StringArray channels = StringArray::fromTokens(input, ",", "");

    Array<var> selectedChannels;

    for (int i = 0; i < channels.size(); i++)
    {
        int ch = channels[i].getIntValue() - 1;

        selectedChannels.add(ch);
    }

    return selectedChannels;
}

void SelectedChannelsParameter::setChannelCount(int count)
{
    channelCount = count;
}

MaskChannelsParameter::MaskChannelsParameter(GenericProcessor* processor_,
    ParameterScope scope,
    const String& name,
    const String& description,
    bool deactivateDuringAcquisition)
    : Parameter(processor_,
        ParameterType::MASK_CHANNELS_PARAM,
        scope,
        name,
        description,
        Array<var>(),
        deactivateDuringAcquisition),
    channelCount(0)
{
}

void MaskChannelsParameter::setNextValue(var newValue_)
{
    Array<var> values;
    
    for (int i = 0; i < channelCount; i++)
    {
        if (newValue_.getArray()->contains(i))
            values.add(i);
    }
    
    newValue = values;

    processor->parameterChangeRequest(this);
}

std::vector<bool> MaskChannelsParameter::getChannelStates()
{
    std::vector<bool> states;

    for (int i = 0; i < channelCount; i++)
    {
        if (currentValue.getArray()->contains(i))
            states.push_back(true);
        else
            states.push_back(false);
    }

    return states;
}


Array<int> MaskChannelsParameter::getArrayValue()
{
    Array<int> out;

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        out.add(currentValue[i]);
    }

    return out;
}

String MaskChannelsParameter::getValueAsString()
{
    return maskChannelsToString();
}

void MaskChannelsParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), maskChannelsToString());
}

void MaskChannelsParameter::fromXml(XmlElement* xml)
{
    if (xml->hasAttribute(getName()))
        currentValue = parseMaskString(xml->getStringAttribute(getName(), ""));
}

String MaskChannelsParameter::maskChannelsToString()
{

    String result = "";

    for (int i = 0; i < channelCount; i++)
    {
        if (!currentValue.getArray()->contains(var(i)))
            result += String(i + 1) + ",";
    }

    return result.substring(0, result.length() - 1);
}

Array<var> MaskChannelsParameter::parseMaskString(const String& input)
{

    StringArray channels = StringArray::fromTokens(input, ",", "");

    Array<var> maskChannels;

    for (int i = 0; i < channels.size(); i++)
    {
        int ch = channels[i].getIntValue() - 1;

        maskChannels.add(ch);
    }

    Array<var> selectedChannels;

    for (int i = 0; i < channelCount; i++)
    {
        if (!maskChannels.contains(var(i)))
            selectedChannels.add(i);
    }

    return selectedChannels;
}


void MaskChannelsParameter::setChannelCount(int count)
{
    
    Array<var>* value = currentValue.getArray();
    
    if (channelCount < count)
    {
        for (int i = channelCount; i < count; i++)
        {
            value->add(i);
        }
    }
    else if (channelCount > count)
    {
        for (int i = count; i < channelCount; i++)
        {
            value->remove(value->indexOf(var(i)));
        }
            
    }
    
    channelCount = count;
    
}
