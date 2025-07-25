/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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
        return ((DataStream*) parameterOwner)->getStreamId();

    if (parameterOwner->getType() == ParameterOwner::SPIKE_CHANNEL)
        return ((SpikeChannel*) parameterOwner)->getStreamId();

    if (parameterOwner->getType() == ParameterOwner::CONTINUOUS_CHANNEL)
        return ((ContinuousChannel*) parameterOwner)->getStreamId();

    if (parameterOwner->getType() == ParameterOwner::EVENT_CHANNEL)
        return ((EventChannel*) parameterOwner)->getStreamId();

    return 0;
}

Colour Parameter::getColour()
{
    return parameterOwner->getColour (m_name);
}

void Parameter::setOwner (ParameterOwner* parameterOwner_)
{
    parameterOwner = parameterOwner_;

    if (parameterOwner == nullptr)
        return;

    String key;
    if (getScope() == ParameterScope::GLOBAL_SCOPE)
    {
        key = getName(); // name should already be unique
    }
    else if (getScope() == ParameterScope::STREAM_SCOPE)
    {
        auto stream = (DataStream*) parameterOwner;
        key = stream->getKey() + "|" + String (stream->getNodeId()) + "|" + getName();
    }
    else if (getScope() == ParameterScope::PROCESSOR_SCOPE)
    {
        auto processor = (GenericProcessor*) parameterOwner;
        key = String (processor->getNodeId()) + "|" + this->getName();
    }
    else if (getScope() == ParameterScope::VISUALIZER_SCOPE)
    {
        auto visualizer = (Visualizer*) parameterOwner;
        key = "v" + String (visualizer->getProcessor()->getNodeId()) + "|" + this->getName();
    }
    else if (getScope() == ParameterScope::SPIKE_CHANNEL_SCOPE)
    {
        auto channel = (SpikeChannel*) parameterOwner;
        key = String (channel->getNodeId()) + "_" + channel->getName() + "_" + this->getName();
    }

    this->setKey (key.toStdString());

    LOGD ("$$$ Registered Parameter : ", key);

    Parameter::registerParameter (this);
}

void Parameter::setEnabled (bool enabled)
{
    isEnabledFlag = enabled;

    for (auto listener : parameterListeners)
    {
        listener->parameterEnabled (isEnabledFlag);
    }
}

void Parameter::addListener (Parameter::Listener* listener)
{
    if (! parameterListeners.contains (listener))
    {
        parameterListeners.add (listener);
    }
}

void Parameter::removeListener (Parameter::Listener* listener)
{
    if (parameterListeners.contains (listener))
    {
        parameterListeners.removeAllInstancesOf (listener);
    }
}

void Parameter::valueChanged()
{
    for (auto listener : parameterListeners)
    {
        listener->parameterChanged (this);
    }
}

void Parameter::logValueChange()
{
    if (getScope() == ParameterScope::GLOBAL_SCOPE)
    {
        CoreServices::sendStatusMessage ("Set " + getDisplayName() + ": " + getChangeDescription());
    }
    else if (getScope() == ParameterScope::PROCESSOR_SCOPE)
    {
        auto processor = (GenericProcessor*) getOwner();
        CoreServices::sendStatusMessage ("Set " + processor->getName() + " " + getDisplayName() + ": " + getChangeDescription());
    }
    else if (getScope() == ParameterScope::STREAM_SCOPE)
    {
        //Get the processor that owns the stream
        auto stream = (DataStream*) getOwner();
        int nodeID = stream->getNodeId();
        GenericProcessor* proc = AccessClass::getProcessorGraph()->getProcessorWithNodeId (nodeID);
        String procKey = proc->getName();

        //Get stream source key
        int firstDelimPos = String (getKey()).indexOfChar ('|');
        int secondDelimPos = String (getKey()).indexOfChar (firstDelimPos + 1, '|');
        String srcKey = String (getKey()).substring (0, secondDelimPos + 1);
        CoreServices::sendStatusMessage ("Set " + procKey + " " + getDisplayName() + ": " + getChangeDescription());
    }
    else if (getScope() == ParameterScope::VISUALIZER_SCOPE)
    {
        auto visualizer = (Visualizer*) getOwner();
        auto processor = visualizer->getProcessor();
        CoreServices::sendStatusMessage ("Set " + visualizer->getName() + " " + getDisplayName() + ": " + getChangeDescription());
    }
    else if (getScope() == ParameterScope::SPIKE_CHANNEL_SCOPE)
    {
        //Get spike source node ID
        auto channel = (SpikeChannel*) getOwner();
        int firstDelimPos = String (channel->getIdentifier()).indexOfChar ('|');
        int secondDelimPos = String (channel->getIdentifier()).indexOfChar (firstDelimPos + 1, '|');
        int thirdDelimPos = String (channel->getIdentifier()).indexOfChar (secondDelimPos + 1, '|');

        String srcKey = String (channel->getIdentifier()).substring (secondDelimPos + 1, thirdDelimPos + 1);
        String streamKey = String (channel->getIdentifier()).substring (0, secondDelimPos + 1);

        CoreServices::sendStatusMessage ("Set " + channel->getName() + " " + getDisplayName() + ": " + getChangeDescription());
    }
}

Parameter::ChangeValue::ChangeValue (std::string key_, var newValue_)
    : key (key_), newValue (newValue_)
{
    Parameter* p = Parameter::parameterMap[key_];
    if (p != nullptr)
    {
        originalValue = p->getValue();

        // Store linked parameter states before the change
        if (p->isLinked())
        {
            p->storeLinkedStates();
        }
    }
}

bool Parameter::ChangeValue::perform()
{
    Parameter* p = Parameter::parameterMap[key];
    if (p == nullptr || ! p->isEnabled() || p->getValue() == newValue)
        return false;

    p->setNextValue (newValue, false);
    p->valueChanged();

    // Compare current value with new value to check if the change was effective
    if (p->getValue() == newValue)
        return true;
    else
        return false;
}

bool Parameter::ChangeValue::undo()
{
    Parameter* p = Parameter::parameterMap[key];
    if (p == nullptr)
        return false;

    p->setNextValue (originalValue, false);
    p->valueChanged();

    // Restore linked parameter states
    if (p->isLinked())
    {
        p->restoreLinkedStates();
    }

    return true;
}

BooleanParameter::BooleanParameter (ParameterOwner* owner,
                                    ParameterScope scope,
                                    const String& name,
                                    const String& displayName,
                                    const String& description,
                                    bool defaultValue,
                                    bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::BOOLEAN_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue,
                 deactivateDuringAcquisition)
{
}

void BooleanParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    if (newValue_.isBool())
    {
        newValue = newValue_;
    }

    if (undoable)
    {
        ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

bool BooleanParameter::getBoolValue()
{
    return (bool) currentValue;
}

String BooleanParameter::getValueAsString()
{
    if ((bool) currentValue)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

String BooleanParameter::getChangeDescription()
{
    return getValueAsString();
}

void BooleanParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (bool) currentValue);
}

void BooleanParameter::fromXml (XmlElement* xml)
{
    currentValue = xml->getBoolAttribute (getName(), defaultValue);
}

CategoricalParameter::CategoricalParameter (ParameterOwner* owner,
                                            ParameterScope scope,
                                            const String& name,
                                            const String& displayName,
                                            const String& description,
                                            Array<String> categories_,
                                            int defaultIndex,
                                            bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::CATEGORICAL_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultIndex,
                 deactivateDuringAcquisition),
      categories (categories_)
{
}

void CategoricalParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue
        || ! newValue_.isInt()
        || (int) newValue_ < 0
        || (int) newValue_ >= categories.size())
        return;

    newValue = (int) newValue_;

    if (undoable)
    {
        ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

int CategoricalParameter::getSelectedIndex()
{
    return (int) currentValue;
}

String CategoricalParameter::getSelectedString()
{
    return categories[currentValue];
}

String CategoricalParameter::getValueAsString()
{
    return getSelectedString();
}

String CategoricalParameter::getChangeDescription()
{
    return getValueAsString();
}

const Array<String>& CategoricalParameter::getCategories()
{
    return categories;
}

void CategoricalParameter::setCategories (Array<String> categories_)
{
    categories = categories_;

    if (categories.size() > 0 && (int) currentValue >= categories.size())
    {
        currentValue = categories.size() - 1;
        valueChanged();
    }
}

void CategoricalParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (int) currentValue);
}

void CategoricalParameter::fromXml (XmlElement* xml)
{
    int xmlValue = xml->getIntAttribute (getName(), defaultValue);
    currentValue = xmlValue < categories.size() ? (var) xmlValue : defaultValue;
}

IntParameter::IntParameter (ParameterOwner* owner,
                            ParameterScope scope,
                            const String& name,
                            const String& displayName,
                            const String& description,
                            int defaultValue_,
                            int minValue_,
                            int maxValue_,
                            bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::INT_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue_,
                 deactivateDuringAcquisition),
      maxValue (maxValue_),
      minValue (minValue_)
{
}

void IntParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    int value = (int) newValue_;

    if (value < minValue)
        newValue = minValue;
    else if (value > maxValue)
        newValue = maxValue;
    else
        newValue = value;

    if (undoable)
    {
        ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

int IntParameter::getIntValue()
{
    return int (currentValue);
}

String IntParameter::getValueAsString()
{
    return String (getIntValue());
}

String IntParameter::getChangeDescription()
{
    return getValueAsString();
}

void IntParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (int) currentValue);
}

void IntParameter::fromXml (XmlElement* xml)
{
    int xmlValue = xml->getIntAttribute (getName(), defaultValue);

    if (xmlValue < minValue || xmlValue > maxValue)
        currentValue = defaultValue;
    else
        currentValue = xmlValue;
}

StringParameter::StringParameter (ParameterOwner* owner,
                                  ParameterScope scope,
                                  const String& name,
                                  const String& displayName,
                                  const String& description,
                                  String defaultValue_,
                                  bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::STRING_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue_,
                 deactivateDuringAcquisition)
{
}

void StringParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    newValue = newValue_.toString();

    if (undoable)
    {
        ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

String StringParameter::getStringValue()
{
    return currentValue.toString();
}

String StringParameter::getValueAsString()
{
    return getStringValue();
}

String StringParameter::getChangeDescription()
{
    return getValueAsString();
}

void StringParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), currentValue.toString());
}

void StringParameter::fromXml (XmlElement* xml)
{
    currentValue = xml->getStringAttribute (getName(), defaultValue);
}

FloatParameter::FloatParameter (ParameterOwner* owner,
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
    : Parameter (owner,
                 ParameterType::FLOAT_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue_,
                 deactivateDuringAcquisition),
      maxValue (maxValue_),
      minValue (minValue_),
      stepSize (stepSize_),
      unit (unit_)
{
}

void FloatParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    float value = (float) newValue_;

    if (value < minValue)
        newValue = minValue;
    else if (value > maxValue)
        newValue = maxValue;
    else
        newValue = value;

    if (undoable)
    {
        ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

float FloatParameter::getFloatValue()
{
    return float (currentValue);
}

String FloatParameter::getValueAsString()
{
    return String (getFloatValue()) + " " + unit;
}

String FloatParameter::getChangeDescription()
{
    return getValueAsString();
}

void FloatParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (float) currentValue);
}

void FloatParameter::fromXml (XmlElement* xml)
{
    float xmlValue = (float) xml->getDoubleAttribute (getName(), defaultValue);

    if (xmlValue < minValue || xmlValue > maxValue)
        currentValue = defaultValue;
    else
        currentValue = xmlValue;
}

SelectedChannelsParameter::SelectedChannelsParameter (ParameterOwner* owner,
                                                      ParameterScope scope,
                                                      const String& name,
                                                      const String& displayName,
                                                      const String& description,
                                                      Array<var> defaultValue_,
                                                      int maxSelectableChannels_,
                                                      bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::SELECTED_CHANNELS_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue_,
                 deactivateDuringAcquisition),
      maxSelectableChannels (maxSelectableChannels_),
      channelCount (0)
{
}

void SelectedChannelsParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    int arraySize = newValue_.getArray()->size();
    if (arraySize <= maxSelectableChannels && arraySize <= channelCount)
    {
        for (int i = 0; i < arraySize; i++)
        {
            if ((int) newValue_[i] < 0 || (int) newValue_[i] >= channelCount)
                return;
        }

        newValue = newValue_;
    }
    else
    {
        return;
    }

    if (undoable)
    {
        Parameter::ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

std::vector<bool> SelectedChannelsParameter::getChannelStates()
{
    std::vector<bool> states;

    for (int i = 0; i < channelCount; i++)
    {
        if (currentValue.getArray()->contains (i))
            states.push_back (true);
        else
            states.push_back (false);
    }

    return states;
}

Array<int> SelectedChannelsParameter::getArrayValue()
{
    Array<int> out;

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        out.add (currentValue[i]);
    }

    return out;
}

String SelectedChannelsParameter::getValueAsString()
{
    return selectedChannelsToString();
}

String SelectedChannelsParameter::getChangeDescription()
{
    // Check number of selected channels:
    int selectedChannelCount = currentValue.getArray()->size();

    if (selectedChannelCount == 0) // If no channels selected, return "None"
        return "none";
    else if (selectedChannelCount > 4) // If more than four channels, state the number selected
        return "selected " + String (selectedChannelCount) + " channels";
    else // Get string describing selected channels
    {
        String selectedChannelsString;

        for (int i = 0; i < selectedChannelCount; i++)
        {
            selectedChannelsString += String (int (currentValue[i]) + 1);

            if (i < selectedChannelCount - 1)
                selectedChannelsString += ", ";
        }

        return selectedChannelsString;
    }
}

void SelectedChannelsParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), selectedChannelsToString());
}

void SelectedChannelsParameter::fromXml (XmlElement* xml)
{
    if (xml->hasAttribute (getName()))
        currentValue = parseSelectedString (xml->getStringAttribute (getName(), ""));

    //std::cout << "Loading selected channels parameter at " << this << std::endl;
}

String SelectedChannelsParameter::selectedChannelsToString()
{
    String result = "";

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        result += String (int (currentValue[i]) + 1) + ",";
    }

    return result.substring (0, result.length() - 1);
}

Array<var> SelectedChannelsParameter::parseSelectedString (const String& input)
{
    StringArray channels = StringArray::fromTokens (input, ",", "");

    Array<var> selectedChannels;

    for (int i = 0; i < channels.size(); i++)
    {
        int ch = channels[i].getIntValue() - 1;

        selectedChannels.add (ch);
    }

    return selectedChannels;
}

void SelectedChannelsParameter::setChannelCount (int newCount)
{
    if (newCount > 0 && getScope() != ParameterScope::SPIKE_CHANNEL_SCOPE)
    {
        Array<var> values;

        // If the new count is less than the current count, remove any channels that are out of bounds
        if (channelCount > newCount)
        {
            for (int i = 0; i < currentValue.getArray()->size(); i++)
            {
                if ((int) currentValue[i] < newCount)
                {
                    values.add (currentValue[i]);
                }
            }

            currentValue = values;
        }
        else if (channelCount == 0 && currentValue.getArray()->size() == 0) // If the current count is 0, set the selected channels to the first maxSelectableChannels channels
        {
            for (int i = 0; i < maxSelectableChannels; i++)
            {
                if (i < newCount)
                {
                    values.add (i);
                }
            }

            currentValue = values;
        }
    }

    channelCount = newCount;

    LOGDD ("SelectedChannelsParameter: Setting selected channels count to ", newCount, " at ", getName());
}

MaskChannelsParameter::MaskChannelsParameter (ParameterOwner* owner,
                                              ParameterScope scope,
                                              const String& name,
                                              const String& displayName,
                                              const String& description,
                                              bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::MASK_CHANNELS_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 Array<var>(),
                 deactivateDuringAcquisition),
      channelCount (0)
{
}

void MaskChannelsParameter::setNextValue (var newValue_, bool undoable)
{
    Array<var> values;

    String result = "";
    for (int i = 0; i < channelCount; i++)
    {
        if (newValue_.getArray()->contains (i))
            values.add (i);
        else
            result += String (i + 1) + ",";
    }
    result = result.substring (0, result.length() - 1);

    if (result == maskChannelsToString())
        return;

    newValue = values;

    if (undoable)
    {
        Parameter::ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

        if (shouldDeactivateDuringAcquisition())
            AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
        else
            AccessClass::getUndoManager()->beginNewTransaction();

        AccessClass::getUndoManager()->perform (action);
    }
    else
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

std::vector<bool> MaskChannelsParameter::getChannelStates()
{
    std::vector<bool> states;

    for (int i = 0; i < channelCount; i++)
    {
        if (currentValue.getArray()->contains (i))
            states.push_back (true);
        else
            states.push_back (false);
    }

    return states;
}

Array<int> MaskChannelsParameter::getArrayValue()
{
    Array<int> out;

    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        out.add (currentValue[i]);
    }

    return out;
}

String MaskChannelsParameter::getValueAsString()
{
    return maskChannelsToString();
}

String MaskChannelsParameter::getChangeDescription()
{
    //compare previousValue to currentValue and return a string describing the change
    Array<int> prev;
    for (int i = 0; i < previousValue.getArray()->size(); i++)
    {
        prev.add (previousValue[i]);
    }

    Array<int> curr;
    for (int i = 0; i < currentValue.getArray()->size(); i++)
    {
        curr.add (currentValue[i]);
    }

    //find how many values in current were not in previous
    int added = 0;

    for (int i = 0; i < curr.size(); i++)
    {
        if (! prev.contains (curr[i]))
            added++;
    }

    // find how many values in previous are not in current
    int removed = 0;

    for (int i = 0; i < prev.size(); i++)
    {
        if (! curr.contains (prev[i]))
            removed++;
    }

    String selectionString;

    if (added > 0) //should never get here
        selectionString += "added " + String (added);

    if (removed > 0)
    {
        if (selectionString.length() > 0)
            selectionString += ", ";
        selectionString += "removed " + String (removed);
    }

    selectionString += " channel";

    if (added > 1 || removed > 1)
        selectionString += "s";

    if (added == 0 && removed == 0)
        selectionString = "no change";

    return selectionString;
}

void MaskChannelsParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), maskChannelsToString());
}

void MaskChannelsParameter::fromXml (XmlElement* xml)
{
    if (xml->hasAttribute (getName()))
        currentValue = parseMaskString (xml->getStringAttribute (getName(), ""));
}

String MaskChannelsParameter::maskChannelsToString()
{
    String result = "";

    for (int i = 0; i < channelCount; i++)
    {
        if (! currentValue.getArray()->contains (var (i)))
            result += String (i + 1) + ",";
    }

    return result.substring (0, result.length() - 1);
}

Array<var> MaskChannelsParameter::parseMaskString (const String& input)
{
    StringArray channels = StringArray::fromTokens (input, ",", "");

    Array<var> maskChannels;

    for (int i = 0; i < channels.size(); i++)
    {
        int ch = channels[i].getIntValue() - 1;

        maskChannels.add (ch);
    }

    Array<var> selectedChannels;

    for (int i = 0; i < channelCount; i++)
    {
        if (! maskChannels.contains (var (i)))
            selectedChannels.add (i);
    }

    return selectedChannels;
}

void MaskChannelsParameter::setChannelCount (int count)
{
    Array<var> values;

    if (channelCount < count)
    {
        for (int i = 0; i < channelCount; i++)
        {
            if (currentValue.getArray()->contains (i))
                values.add (i);
        }
        for (int i = channelCount; i < count; i++)
        {
            values.add (i);
        }
    }
    else if (channelCount > count)
    {
        for (int i = 0; i < count; i++)
        {
            if (currentValue.getArray()->contains (i))
                values.add (i);
        }
    }
    else
    {
        return;
    }

    currentValue = values;
    channelCount = count;
}

TtlLineParameter::TtlLineParameter (ParameterOwner* owner,
                                    ParameterScope scope,
                                    const String& name,
                                    const String& displayName,
                                    const String& description,
                                    int maxAvailableLines_,
                                    bool syncMode_,
                                    bool canSelectNone_,
                                    bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::TTL_LINE_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 0,
                 deactivateDuringAcquisition),
      lineCount (maxAvailableLines_),
      syncMode (syncMode_),
      selectNone (canSelectNone_)
{
    jassert ((lineCount >= 0 && lineCount < 256));
    jassert (scope == ParameterScope::STREAM_SCOPE);
}

void TtlLineParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    if (((int) newValue_ < lineCount && (int) newValue_ >= 0)
        || (selectNone && (int) newValue_ == -1)) // -1 is a valid value for non-sync mode
    {
        newValue = newValue_;

        if (undoable)
        {
            ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

            if (shouldDeactivateDuringAcquisition())
                AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
            else
                AccessClass::getUndoManager()->beginNewTransaction();

            AccessClass::getUndoManager()->perform (action);
        }
        else
        {
            getOwner()->parameterChangeRequest (this);
            valueChanged();
        }
    }
}

int TtlLineParameter::getSelectedLine()
{
    return (int) currentValue;
}

String TtlLineParameter::getValueAsString()
{
    return currentValue.toString();
}

String TtlLineParameter::getChangeDescription()
{
    if ((int) currentValue == -1)
        return "none";
    else
        return String (int (currentValue) + 1);
}

void TtlLineParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (int) currentValue);
}

void TtlLineParameter::fromXml (XmlElement* xml)
{
    if (xml->hasAttribute (getName()))
        currentValue = xml->getIntAttribute (getName(), defaultValue);

    //std::cout << "Loading selected channels parameter at " << this << std::endl;
}

PathParameter::PathParameter (ParameterOwner* owner,
                              ParameterScope scope,
                              const String& name,
                              const String& displayName,
                              const String& description,
                              const File& defaultValue_,
                              const StringArray& fileExtensions_,
                              bool isDirectory_,
                              bool isRequired_,
                              bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::PATH_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue_.getFullPathName(),
                 deactivateDuringAcquisition),
      filePatternsAllowed (fileExtensions_),
      isDirectory (isDirectory_),
      isRequired (isRequired_)
{
    currentValue = "None";
    if (isRequired)
        currentValue = defaultValue;
}

void PathParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    if (newValue_.isString())
    {
        if (! isRequired && newValue_.toString() == "None")
        {
            newValue = newValue_;
        }
        else if (! isDirectory && File (newValue_.toString()).existsAsFile())
        {
            newValue = newValue_;
        }
        else if (isDirectory && File (newValue_.toString()).exists())
        {
            newValue = newValue_;
        }
        else
        {
            LOGE (getKey(), ": Invalid path");
            return;
        }

        if (! undoable)
        {
            getOwner()->parameterChangeRequest (this);
            valueChanged();
        }
        else if (isLinked())
        {
            getOwner()->handleLinkedParameterChange (this, newValue);
        }
        else
        {
            ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

            if (shouldDeactivateDuringAcquisition())
                AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
            else
                AccessClass::getUndoManager()->beginNewTransaction();

            AccessClass::getUndoManager()->perform (action);
        }
    }
    else
    {
        LOGE ("Invalid file path");
    }
}

bool PathParameter::isValid()
{
    if (currentValue.toString() == "None")
    {
        if (isRequired)
            currentValue = defaultValue;

        return true;
    }
    else if (! isDirectory && File (currentValue.toString()).existsAsFile())
    {
        return true;
    }
    else if (isDirectory && File (currentValue.toString()).exists())
    {
        return true;
    }

    return false;
}

void PathParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), currentValue.toString());
}

void PathParameter::fromXml (XmlElement* xml)
{
    String savedValue = xml->getStringAttribute (getName(), defaultValue);
    if (savedValue.equalsIgnoreCase ("default"))
        savedValue = "None";

    currentValue = savedValue;
}

String PathParameter::getChangeDescription()
{
    if (! isRequired && currentValue.toString() == "None" && defaultValue.toString().isNotEmpty())
        return "default";

    return currentValue.toString();
}

SelectedStreamParameter::SelectedStreamParameter (ParameterOwner* owner,
                                                  ParameterScope scope,
                                                  const String& name,
                                                  const String& displayName,
                                                  const String& description,
                                                  Array<String> streamNames_,
                                                  int defaultIndex,
                                                  bool syncWithStreamSelector_,
                                                  bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::SELECTED_STREAM_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultIndex,
                 deactivateDuringAcquisition),
      streamNames (streamNames_),
      syncWithStreamSelector (syncWithStreamSelector_)
{
}

void SelectedStreamParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_ == currentValue)
        return;

    if (newValue_.isInt()
        && (int) newValue_ >= -1
        && (int) newValue_ < streamNames.size())
    {
        newValue = newValue_;

        if (undoable)
        {
            ChangeValue* action = new Parameter::ChangeValue (getKey(), newValue);

            if (shouldDeactivateDuringAcquisition())
                AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
            else
                AccessClass::getUndoManager()->beginNewTransaction();

            AccessClass::getUndoManager()->perform (action);
        }
        else
        {
            getOwner()->parameterChangeRequest (this);
            valueChanged();
        }
    }
    else
    {
        LOGE ("Invalid stream name: ", newValue_.toString());
    }
}

void SelectedStreamParameter::setStreamNames (Array<String> streamNames_)
{
    streamNames = streamNames_;

    newValue = currentValue;

    if (streamNames.size() > 0 && (int) currentValue >= streamNames.size())
        newValue = streamNames.size() - 1;
    else if (streamNames.size() == 0)
        newValue = -1;
    else if ((int) currentValue == -1)
        newValue = 0;

    if (newValue != currentValue)
    {
        getOwner()->parameterChangeRequest (this);
        valueChanged();
    }
}

int SelectedStreamParameter::getSelectedIndex()
{
    return (int) currentValue;
}

String SelectedStreamParameter::getValueAsString()
{
    if ((int) currentValue == -1 || streamNames.size() == 0)
        return String();
    else
        return streamNames[(int) currentValue];
}

void SelectedStreamParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), (int) currentValue);
}

void SelectedStreamParameter::fromXml (XmlElement* xml)
{
    int loadValue = xml->getIntAttribute (getName(), defaultValue);
    currentValue = loadValue < streamNames.size() ? (var) loadValue : defaultValue;
}

String SelectedStreamParameter::getChangeDescription()
{
    if ((int) currentValue == -1)
        return "None";
    else
        return getValueAsString();
}

TimeParameter::TimeParameter (ParameterOwner* owner,
                              ParameterScope scope,
                              const String& name,
                              const String& displayName,
                              const String& description,
                              const String& defaultValue,
                              bool deactivateDuringAcquisition)
    : Parameter (owner,
                 ParameterType::TIME_PARAM,
                 scope,
                 name,
                 displayName,
                 description,
                 defaultValue,
                 deactivateDuringAcquisition)
{
    timeValue = std::make_shared<TimeValue> (defaultValue);
    timeValue->setMinTimeInMilliseconds (0);
}

void TimeParameter::setNextValue (var newValue_, bool undoable)
{
    if (newValue_.toString() == currentValue.toString())
        return;

    if (newValue_.isString())
    {
        newValue = newValue_;

        if (undoable)
        {
            ChangeValue* action = new TimeParameter::ChangeValue (getKey(), newValue);

            if (shouldDeactivateDuringAcquisition())
                AccessClass::getUndoManager()->beginNewTransaction ("Disabled during acquisition");
            else
                AccessClass::getUndoManager()->beginNewTransaction();

            AccessClass::getUndoManager()->perform (action);
        }
        else
        {
            currentValue = newValue;
            timeValue->setTimeFromString (currentValue.toString());
            getOwner()->parameterChangeRequest (this);
            valueChanged();
        }
    }
}

void TimeParameter::toXml (XmlElement* xml)
{
    xml->setAttribute (getName(), currentValue.toString());
}

void TimeParameter::fromXml (XmlElement* xml)
{
    currentValue = xml->getStringAttribute (getName(), defaultValue);
    timeValue->setTimeFromString (currentValue.toString());
}

TimeParameter::ChangeValue::ChangeValue (std::string key_, var newValue_)
    : key (key_), newValue (newValue_)
{
    Parameter* p = Parameter::parameterMap[key_];
    if (p != nullptr)
    {
        originalValue = p->getValue();

        // Store linked parameter states before the change
        if (p->isLinked())
        {
            p->storeLinkedStates();
        }
    }
}

bool TimeParameter::ChangeValue::perform()
{
    Parameter* p = Parameter::parameterMap[key];
    if (p == nullptr || ! p->isEnabled() || p->getValue() == newValue)
        return false;

    p->setNextValue (newValue, false);
    p->valueChanged();

    // Compare current value with new value to check if the change was effective
    if (p->getValue() == newValue)
        return true;
    else
        return false;
}

bool TimeParameter::ChangeValue::undo()
{
    Parameter* p = Parameter::parameterMap[key];
    if (p == nullptr)
        return false;

    p->setNextValue (originalValue, false);
    p->valueChanged();

    // Restore linked parameter states
    if (p->isLinked())
    {
        p->restoreLinkedStates();
    }

    return true;
}

String TimeParameter::getChangeDescription()
{
    return getValueAsString();
}

NotificationParameter::NotificationParameter (ParameterOwner* owner,
                                              ParameterScope scope,
                                              const String& name,
                                              const String& displayName,
                                              const String& description,
                                              bool deactivateDuringAcquisition)
    : Parameter (owner,
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
    setNextValue (true, true);
}

void NotificationParameter::setNextValue (var newValue_, bool undoable)
{
    getOwner()->parameterValueChanged (this);
}
