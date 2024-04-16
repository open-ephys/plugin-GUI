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
#include "../Visualization/Visualizer.h"

int64 Parameter::parameterCounter = 0;

std::map<int64, String> Parameter::p_name_map;
std::map<std::string, Parameter*> Parameter::parameterMap;

String Parameter::getParameterTypeString() const
{

    if (m_parameterType == BOOLEAN_PARAM)
        return "Boolean";
    else if (m_parameterType == CATEGORICAL_PARAM)
        return "Categorical";
    else if (m_parameterType == STRING_PARAM)
        return "String";
    else if (m_parameterType == FLOAT_PARAM)
        return "Float";
    else if (m_parameterType == INT_PARAM)
        return "Integer";
    else if (m_parameterType == SELECTED_CHANNELS_PARAM)
        return "Selected Channels";
    else if (m_parameterType == SELECTED_SPIKE_CHANNEL_PARAM)
        return "Selected Spike Channel";
    else if (m_parameterType == SELECTED_EVENT_CHANNEL_PARAM)
        return "Selected Event Channel";
    else if (m_parameterType == SELECTED_PROCESSOR_PARAM)
        return "Selected Processor";
    else if (m_parameterType == SELECTED_STREAM_PARAM)
        return "Selected Stream";
    else if (m_parameterType == MASK_CHANNELS_PARAM)
        return "Mask Channels";
    else if (m_parameterType == TTL_LINE_PARAM)
        return "TTL Line";
    else if (m_parameterType == PATH_PARAM)
        return "Path";
    else if (m_parameterType == TIME_PARAM)
        return "Time";
    else if (m_parameterType == NAME_PARAM)
        return "Name";
    else if (m_parameterType == COLOUR_PARAM)
        return "Colour";
    else if (m_parameterType == NOTIFICATION_PARAM)
        return "Notification";
    else
        return "Unknown";

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

Colour Parameter::getColor() 
{
    return parameterOwner->getColor(m_name);
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
        key = stream->getKey() + "|" + String(stream->getNodeId()) + "|" + getName();
    }
    else if (getScope() == ParameterScope::PROCESSOR_SCOPE)
    {
        auto processor = (GenericProcessor*)parameterOwner;
        key = String(processor->getNodeId()) + "|" + this->getName();
    }
    else if (getScope() == ParameterScope::VISUALIZER_SCOPE)
    {
        auto visualizer = (Visualizer*)parameterOwner;
        key = "v" + String(visualizer->getProcessor()->getNodeId()) + "|" + this->getName();
    }
    else if (getScope() == ParameterScope::SPIKE_CHANNEL_SCOPE)
    {
        auto channel = (SpikeChannel*)parameterOwner;
        key = String(channel->getNodeId()) + "_" + channel->getName() + "_" + this->getName();
    }

    this->setKey(key.toStdString());

    LOGD("$$$ Registered Parameter : ", key);

    Parameter::registerParameter(this);
}

void Parameter::setEnabled(bool enabled)
{
    isEnabledFlag = enabled;

    for (auto listener : parameterListeners)
    {
        listener->parameterEnabled(isEnabledFlag);
    }
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

    if (!p->isEnabled())
        return false;
    
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

void BooleanParameter::setNextValue(var newValue_, bool undoable)
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

void CategoricalParameter::setNextValue(var newValue_, bool undoable)
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

    if (categories.size() > 0 && (int)currentValue >= categories.size())
    {
        currentValue = categories.size() - 1;
        valueChanged();
    }
}

void CategoricalParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (int)currentValue);
}

void CategoricalParameter::fromXml(XmlElement* xml)
{
    int xmlValue = xml->getIntAttribute(getName(), defaultValue);
    currentValue = xmlValue < categories.size() ? (var)xmlValue : defaultValue;
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

void IntParameter::setNextValue(var newValue_, bool undoable)
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
    int xmlValue = xml->getIntAttribute(getName(), defaultValue);

    if (xmlValue < minValue || xmlValue > maxValue)
        currentValue = defaultValue;
    else
        currentValue = xmlValue;
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

void StringParameter::setNextValue(var newValue_, bool undoable)
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

void FloatParameter::setNextValue(var newValue_, bool undoable)
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
    float xmlValue = (float)xml->getDoubleAttribute(getName(), defaultValue);

    if (xmlValue < minValue || xmlValue > maxValue)
        currentValue = defaultValue;
    else
        currentValue = xmlValue;
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
    if (defaultValue_.size() == 0)
    {
        //Set default selected to the first maxSelectableChannels channels
        Array<var> values;
        for (int i = 0; i < maxSelectableChannels; i++)
            values.add(i);
        currentValue = values;
    }
    else
    {
        currentValue = defaultValue_;
    }
}

void SelectedChannelsParameter::setNextValue(var newValue_, bool undoable)
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
    
    LOGDD("SelectedChannelsParameter: Setting selected channels count to ", count, " at ", getName());
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

void MaskChannelsParameter::setNextValue(var newValue_, bool undoable)
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


TtlLineParameter::TtlLineParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int maxAvailableLines_,
    bool syncMode_,
    bool canSelectNone_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::TTL_LINE_PARAM,
        scope,
        name,
        displayName,
        description,
        0,
        deactivateDuringAcquisition),
    lineCount(maxAvailableLines_),
    syncMode(syncMode_),
    selectNone(canSelectNone_)
{
    jassert((lineCount >= 0 && lineCount < 256));
    jassert(scope == ParameterScope::STREAM_SCOPE);
    
    // Can't have both sync mode and select none
    if (syncMode && selectNone)
        jassertfalse;
}

void TtlLineParameter::setNextValue(var newValue_, bool undoable)
{

    if (newValue_ == currentValue) return;

    if (((int)newValue_ < lineCount && (int)newValue_ >= 0) 
        || (!syncMode && (int)newValue_ == -1)) // -1 is a valid value for non-sync mode
    {
        newValue = newValue_;
    
        ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

        AccessClass::getUndoManager()->beginNewTransaction();
        AccessClass::getUndoManager()->perform(action);
    }

}

int TtlLineParameter::getSelectedLine()
{
    return (int)currentValue;
}

String TtlLineParameter::getValueAsString()
{
    return currentValue.toString();
}

void TtlLineParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (int)currentValue);
}

void TtlLineParameter::fromXml(XmlElement* xml)
{
    if (xml->hasAttribute(getName()))
        currentValue = xml->getIntAttribute(getName(), defaultValue);
    
    //std::cout << "Loading selected channels parameter at " << this << std::endl;
}


PathParameter::PathParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    const StringArray& fileExtensions_,
    bool isDirectory_,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::PATH_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue,
        deactivateDuringAcquisition),
    filePatternsAllowed(fileExtensions_),
    isDirectory(isDirectory_)
{
    currentValue = defaultValue;
}

void PathParameter::setNextValue(var newValue_, bool undoable)
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

SelectedStreamParameter::SelectedStreamParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> streamNames_,
    int defaultIndex,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::SELECTED_STREAM_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultIndex,
        deactivateDuringAcquisition),
    streamNames(streamNames_)
{
}


void SelectedStreamParameter::setNextValue(var newValue_, bool undoable)
{
    if (newValue_ == currentValue) return;

    if (newValue_.isInt())
    {

        newValue = newValue_;

        ChangeValue* action = new Parameter::ChangeValue(getKey(), newValue);

        AccessClass::getUndoManager()->beginNewTransaction();
        AccessClass::getUndoManager()->perform(action);        
    }
    else
    {
        LOGE("Invalid stream name: ", newValue_.toString());
    }

}

void SelectedStreamParameter::setStreamNames(Array<String> streamNames_)
{
    streamNames = streamNames_;

    if (streamNames.size() > 0 && (int)currentValue >= streamNames.size())
        currentValue = streamNames.size() - 1;
    else if (streamNames.size() == 0)
        currentValue = -1;
    else if ((int)currentValue == -1)
        currentValue = 0;
}

int SelectedStreamParameter::getSelectedIndex()
{
    return (int)currentValue;
}

void SelectedStreamParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), (int)currentValue);
}

void SelectedStreamParameter::fromXml(XmlElement* xml)
{
    int loadValue = xml->getIntAttribute(getName(), defaultValue);
    currentValue = loadValue < streamNames.size() ? (var)loadValue : defaultValue;
}

TimeParameter::TimeParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
        ParameterType::TIME_PARAM,
        scope,
        name,
        displayName,
        description,
        defaultValue,
        deactivateDuringAcquisition),
        timeValue(new TimeParameter::TimeValue(defaultValue))
{
    timeValue->setMinTimeInMilliseconds(0);
}

void TimeParameter::setNextValue(var newValue_, bool undoable)
{

    if (newValue_.toString() == currentValue.toString()) return;

    if (newValue_.isString())
    {

        newValue = newValue_;

        if (undoable)
        {
            ChangeValue* action = new TimeParameter::ChangeValue(getKey(), newValue);

            AccessClass::getUndoManager()->beginNewTransaction();
            AccessClass::getUndoManager()->perform(action);

        }
        else
        {
            currentValue = newValue;
            timeValue->setTimeFromString(currentValue.toString());
            valueChanged();
        }

    }
}

void TimeParameter::toXml(XmlElement* xml)
{
    xml->setAttribute(getName(), currentValue.toString());
}

void TimeParameter::fromXml(XmlElement* xml)
{
    currentValue = xml->getStringAttribute(getName(), defaultValue);
    timeValue->setTimeFromString(currentValue.toString());
}

TimeParameter::ChangeValue::ChangeValue(std::string key_, var newValue_)
    : key(key_), newValue(newValue_)
{
    Parameter* p = Parameter::parameterMap[key_];
    originalValue = p->currentValue;
}

bool TimeParameter::ChangeValue::perform()
{
    TimeParameter* p = (TimeParameter*)Parameter::parameterMap[key];
    p->getTimeValue()->setTimeFromString(newValue.toString());

    p->newValue = newValue;
    p->getOwner()->parameterChangeRequest(p);

    p->valueChanged();

    return true;
}

bool TimeParameter::ChangeValue::undo()
{
    TimeParameter* p = (TimeParameter*)Parameter::parameterMap[key];
    p->getTimeValue()->setTimeFromString(originalValue.toString());

    p->newValue = originalValue;
    p->getOwner()->parameterChangeRequest(p);

    p->valueChanged();

    return true;
}

NotificationParameter::NotificationParameter(ParameterOwner* owner,
    ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
    : Parameter(owner,
                ParameterType::NOTIFICATION_PARAM,
                scope,
                name,
                displayName,
                description,
                false,
                deactivateDuringAcquisition)
{

}

void NotificationParameter::triggerNotification()
{
    setNextValue(true, true);
}

void NotificationParameter::setNextValue(var newValue_, bool undoable)
{
    getOwner()->parameterValueChanged(this);
}
