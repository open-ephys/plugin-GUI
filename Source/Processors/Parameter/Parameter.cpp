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
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"
#include "../GenericProcessor/GenericProcessor.h"

int64 Parameter::parameterCounter = 0;

std::map<int64, String> Parameter::p_name_map;
std::map<std::string, Parameter*> Parameter::parameterMap;

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
    if (parameterOwner->getType() == ParameterOwner::DATASTREAM)
        return ((DataStream*)parameterOwner)->getStreamId();
    
    if (parameterOwner->getType() == ParameterOwner::SPIKE_CHANNEL)
        return ((SpikeChannel*)parameterOwner)->getStreamId();
    
    if (parameterOwner->getType() == ParameterOwner::CONTINUOUS_CHANNEL)
        return ((ContinuousChannel*)parameterOwner)->getStreamId();
        
    if (parameterOwner->getType() == ParameterOwner::EVENT_CHANNEL)
        return ((EventChannel*)parameterOwner)->getStreamId();
    
    return 0;
        
}

void Parameter::setOwner(ParameterOwner* parameterOwner_)
{
    parameterOwner = parameterOwner_;

    if (parameterOwner == nullptr) return;

    String key;
    if (getScope() == ParameterScope::GLOBAL_SCOPE)
    {
        key = getName(); // name should already be unique
    }
    else if (getScope() == ParameterScope::STREAM_SCOPE)
    {
        auto stream = (DataStream*)parameterOwner;
        key = String(stream->getNodeId()) + "_" + stream->getName() + "_" + this->getName();
    }
    else if (getScope() == ParameterScope::PROCESSOR_SCOPE)
    {
        auto processor = (GenericProcessor*)parameterOwner;
        key = String(processor->getNodeId()) + "_" + this->getName();
    }
    else if (getScope() == ParameterScope::SPIKE_CHANNEL_SCOPE)
    {
        key = "TODO"; //Currently handled by spike processors
    }

    this->setKey(key.toStdString());

    LOGDD("$ Registered Parameter $: ", key);

    Parameter::registerParameter(this);
}

void Parameter::addListener(Parameter::Listener* listener)
{
    if (!parameterListeners.contains(listener))
    {
        parameterListeners.add(listener);
    }
}

void Parameter::removeListener(Parameter::Listener* listener)
{
    if (parameterListeners.contains(listener))
    {
        parameterListeners.removeAllInstancesOf(listener);
    }
}

void Parameter::valueChanged()
{
    for (auto listener : parameterListeners)
    {
		listener->parameterChanged(this);
    }
}

Parameter::ChangeValue::ChangeValue(std::string key_, var newValue_)
    : key(key_), newValue(newValue_)
{
    Parameter* p = Parameter::parameterMap[key_];

    originalValue = p->currentValue;
}

bool Parameter::ChangeValue::perform()
{
    Parameter* p = Parameter::parameterMap[key];
    
    p->newValue = newValue;
    p->getOwner()->parameterChangeRequest(p);
    
    p->valueChanged();
    
    return true;
}

bool Parameter::ChangeValue::undo()
{
    Parameter* p = Parameter::parameterMap[key];

    p->newValue = originalValue;
    p->getOwner()->parameterChangeRequest(p);

    p->valueChanged();
    
    return true;
}

BooleanParameter::BooleanParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool defaultValue,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
                ParameterType::BOOLEAN_PARAM,
                scope,
                name,
                displayName,
                description,
                defaultValue,
                deactivateDuringAcquisition)
{

}

void BooleanParameter::setNextValue(var newValue_)
{

    if (newValue_ == currentValue) return;

    if (newValue_.isBool())
    {
        newValue = newValue_;
    }

    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);

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

CategoricalParameter::CategoricalParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> categories_,
    int defaultIndex,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::CATEGORICAL_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultIndex,
        deactivateDuringAcquisition),
    categories(categories_)
{

}

void CategoricalParameter::setNextValue(var newValue_)
{
    if (newValue_ == currentValue) return;

    newValue = (int) newValue_;
    
    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);
    
    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);   
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

const Array<String>& CategoricalParameter::getCategories()
{
    return categories;
}


void CategoricalParameter::setCategories(Array<String> categories_)
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

IntParameter::IntParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int defaultValue_,
    int minValue_,
    int maxValue_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::INT_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxValue(maxValue_),
    minValue(minValue_)
{

}

void IntParameter::setNextValue(var newValue_)
{

    if (newValue_ == currentValue) return;

    int value = (int) newValue_;

    if (value < minValue)
        newValue = minValue;
    else if (value > maxValue)
        newValue = maxValue;
    else
        newValue = value;

    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);
    
    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);
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

StringParameter::StringParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    String defaultValue_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::INT_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue_,
        deactivateDuringAcquisition)
{

}

void StringParameter::setNextValue(var newValue_)
{

    if (newValue_ == currentValue) return;

    newValue = newValue_.toString();

    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);
    
    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);
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


FloatParameter::FloatParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& unit_,
    float defaultValue_,
    float minValue_,
    float maxValue_,
    float stepSize_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::FLOAT_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxValue(maxValue_),
    minValue(minValue_),
    stepSize(stepSize_),
    unit(unit_)
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

    if (currentValue != newValue)
    {
    
        ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

        AccessClass::getUndoManager()->beginNewTransaction();
        AccessClass::getUndoManager()->perform(action);

    }
    
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

SelectedChannelsParameter::SelectedChannelsParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<var> defaultValue_,
    int maxSelectableChannels_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::SELECTED_CHANNELS_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxSelectableChannels(maxSelectableChannels_),
    channelCount(0)
{

}

void SelectedChannelsParameter::setNextValue(var newValue_)
{

    if (newValue_ == currentValue) return;

    if (newValue_.getArray()->size() <= maxSelectableChannels)
    {
        newValue = newValue_;
    }
    
    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);

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
    
    //std::cout << "Loading selected channels parameter at " << this << std::endl;
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
    
   LOGD("************************ Setting selected channels count to ", count, " at ", getName());
}

MaskChannelsParameter::MaskChannelsParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::MASK_CHANNELS_PARAM,
        scope,
        name,
        displayName,
        description,
        Array<var>(),
        deactivateDuringAcquisition),
    channelCount(0)
{

}

void MaskChannelsParameter::setNextValue(var newValue_)
{

    Array<var> values;

    String result = "";
    for (int i = 0; i < channelCount; i++)
    {
        if (newValue_.getArray()->contains(i))
            values.add(i);
        else
            result += String(i + 1) + ",";
    }
    result = result.substring(0, result.length() - 1);

    if (result == maskChannelsToString()) return;

    newValue = values;
    
    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);

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
    
    Array<var> values;
    
    if (channelCount < count)
    {
        for (int i = 0; i < channelCount; i++)
        {
            if (currentValue.getArray()->contains(i))
                values.add(i);
        }
        for (int i = channelCount; i < count; i++)
        {
            values.add(i);
        }
    }
    else if (channelCount > count)
    {
        for (int i = 0; i < count; i++)
        {
            if (currentValue.getArray()->contains(i))
                values.add(i);
        }
            
    }
    else
    {
        return;
    }
    
    currentValue = values;
    channelCount = count;
}

PathParameter::PathParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const StringArray& fileExtensions_,
    bool isDirectory_,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
        ParameterType::PATH_PARAM,
        scope,
        name,
        displayName,
        description,
        deactivateDuringAcquisition),
    validFileExtensions(fileExtensions_),
    isDirectory(isDirectory_)
{

}

void PathParameter::setNextValue(var newValue_)
{
    if (newValue_ == currentValue) return;

    if (newValue_.isString())
    {
        if (!isDirectory && File(newValue_.toString()).existsAsFile())
        {
            newValue = newValue_;
        }
        else if (isDirectory && File(newValue_.toString()).exists())
        {
            newValue = newValue_;
        }

        ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

        AccessClass::getUndoManager()->beginNewTransaction();
        AccessClass::getUndoManager()->perform(action);
    }
    else
    {
        LOGE("Invalid file path");
    }

}

void PathParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), currentValue.toString());
}

void PathParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getStringAttribute(getName(), defaultValue);
}

SelectedStreamParameter::SelectedStreamParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
        ParameterType::SELECTED_STREAM_PARAM,
        scope,
        name,
        displayName,
        description,
        "UNKNOWN_STREAM",
        deactivateDuringAcquisition)
{

}


void SelectedStreamParameter::setNextValue(var newValue_)
{
    if (newValue_ == currentValue) return;

    //TODO: Get a list of valid stream names from the processor

    if (newValue_.isString())
    {
        ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

        AccessClass::getUndoManager()->beginNewTransaction();
        AccessClass::getUndoManager()->perform(action);
    }
    else
    {
        LOGE("Invalid stream name");
    }

}

void SelectedStreamParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), currentValue.toString());
}

void SelectedStreamParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getStringAttribute(getName(), defaultValue);
}

TimeParameter::TimeParameter(
    InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    String defaultValue,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
        ParameterType::TIME_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue,
        deactivateDuringAcquisition),
    timeValue(defaultValue)
{

}

void TimeParameter::setNextValue(var newValue_)
{
    if (newValue_ == currentValue) return;

    if (newValue_.isDouble())
    {
        newValue = newValue_;
    }

    ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

    AccessClass::getUndoManager()->beginNewTransaction();
    AccessClass::getUndoManager()->perform(action);
}

void TimeParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), currentValue.toString());
}

void TimeParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getStringAttribute(getName(), defaultValue);
}