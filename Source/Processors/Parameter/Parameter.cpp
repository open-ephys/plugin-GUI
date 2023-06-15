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
    if (infoObject->getType() == InfoObject::DATASTREAM_INFO)
        return ((DataStream*)infoObject)->getStreamId();
    
    if (infoObject->getType() == InfoObject::SPIKE_CHANNEL)
        return ((SpikeChannel*)infoObject)->getStreamId();
    
    if (infoObject->getType() == InfoObject::CONTINUOUS_CHANNEL)
        return ((ContinuousChannel*)infoObject)->getStreamId();
        
    if (infoObject->getType() == InfoObject::DATASTREAM_INFO)
        return ((EventChannel*)infoObject)->getStreamId();
    
    return 0;
        
}

void Parameter::setOwner(InfoObject* infoObject_)
{
    infoObject = infoObject_;

    if (infoObject == nullptr) return;

    String key;
    if (getScope() == ParameterScope::GLOBAL_SCOPE)
        key = getName(); // name should already be unique
    else if (getScope() == ParameterScope::STREAM_SCOPE)
        key = String(infoObject->getNodeId()) + "_" + infoObject->getName() + "_" + this->getName();
    else if (getScope() == ParameterScope::PROCESSOR_SCOPE)
        key = String(infoObject->getNodeId()) + "_" + this->getName();
    else if (getScope() == ParameterScope::SPIKE_CHANNEL_SCOPE)
        key = "TODO"; //Currently handled by spike processors

    this->setKey(key.toStdString());

    LOGDD("$ Registered Parameter $: ", key);

    Parameter::registerParameter(this);
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
    
    return true;
}

bool Parameter::ChangeValue::undo()
{
    Parameter* p = Parameter::parameterMap[key];

    p->newValue = originalValue;
    p->getOwner()->parameterChangeRequest(p);
    
    return true;
}

BooleanParameter::BooleanParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& description,
    bool defaultValue,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
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

void BooleanParameter::toXml(XmlElement* xml, bool useKey) 
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), (bool) currentValue);
}

void BooleanParameter::fromXml(XmlElement* xml, bool useKey)
{
    currentValue = xml->getBoolAttribute(useKey ? String(getKey()) : getName(), defaultValue);
}

CategoricalParameter::CategoricalParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& description,
    Array<String> categories_,
    int defaultIndex,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
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

void CategoricalParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), (int)currentValue);
}

void CategoricalParameter::fromXml(XmlElement* xml, bool useKey)
{
    currentValue = xml->getIntAttribute(useKey ? String(getKey()) : getName(), defaultValue);
}

IntParameter::IntParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& description,
    int defaultValue_,
    int minValue_,
    int maxValue_,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
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

void IntParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), (int) currentValue);
}

void IntParameter::fromXml(XmlElement* xml, bool useKey)
{
    currentValue = xml->getIntAttribute(useKey ? String(getKey()) : getName(), defaultValue);
}

StringParameter::StringParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& description,
    String defaultValue_,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
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

void StringParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), currentValue.toString());
}

void StringParameter::fromXml(XmlElement* xml, bool useKey)
{
    currentValue = xml->getStringAttribute(useKey ? String(getKey()) : getName(), defaultValue);
}


FloatParameter::FloatParameter(InfoObject* infoObject,
    ParameterScope scope,
    const String& name,
    const String& description,
    float defaultValue_,
    float minValue_,
    float maxValue_,
    float stepSize_,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject,
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

void FloatParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), (float)currentValue);
}

void FloatParameter::fromXml(XmlElement* xml, bool useKey)
{
    currentValue = xml->getDoubleAttribute(useKey ? String(getKey()) : getName(), defaultValue);
}

SelectedChannelsParameter::SelectedChannelsParameter(InfoObject* infoObject_,
    ParameterScope scope,
    const String& name,
    const String& description,
    Array<var> defaultValue_,
    int maxSelectableChannels_,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject_,
        ParameterType::SELECTED_CHANNELS_PARAM,
        scope,
        name,
        description,
        defaultValue_,
        deactivateDuringAcquisition),
    maxSelectableChannels(maxSelectableChannels_),
    channelCount(0)
{
    //std::cout << "Creating new selected channels parameter at " << this << std::endl;
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

void SelectedChannelsParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), selectedChannelsToString());
}

void SelectedChannelsParameter::fromXml(XmlElement* xml, bool useKey)
{
    if (xml->hasAttribute(useKey ? String(getKey()) : getName()))
        currentValue = parseSelectedString(xml->getStringAttribute(useKey ? String(getKey()) : getName(), ""));
    
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
    
   // std::cout << "Setting selected channels channels count to " << count << " at " << this << std::endl;
}

MaskChannelsParameter::MaskChannelsParameter(InfoObject* infoObject_,
    ParameterScope scope,
    const String& name,
    const String& description,
    bool deactivateDuringAcquisition)
    : Parameter(infoObject_,
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
    
    bool isDifferentValue = false;

    for (int i = 0; i < channelCount; i++)
    {
        if (newValue_.getArray()->contains(i))
            values.add(i);

        if (!currentValue.getArray()->contains(i))
            isDifferentValue = true;
    }
    
    if (!isDifferentValue) return;

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

void MaskChannelsParameter::toXml(XmlElement* xml, bool useKey)
{
    xml->setAttribute(useKey ? String(getKey()) : getName(), maskChannelsToString());
}

void MaskChannelsParameter::fromXml(XmlElement* xml, bool useKey)
{
    if (xml->hasAttribute(useKey ? String(getKey()) : getName()))
        currentValue = parseMaskString(xml->getStringAttribute(useKey ? String(getKey()) : getName(), ""));
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
         for (int i = 0; i < count; i++)
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
