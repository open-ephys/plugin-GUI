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
    
    //std::cout << "Next categorical param value: " << int(newValue) << std::endl;

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
    //if (newValue_.isInt())
    //{
    int value = (int) newValue_;

    //std::cout << "val: " << value << std::endl;
   // std::cout << "minvalue: " << minValue << std::endl;
    //std::cout << "maxvalue: " << maxValue << std::endl;
    //std::cout << "streamId: " << getStreamId() << std::endl;

    if (value < minValue)
        newValue = minValue;
    else if (value > maxValue)
        newValue = maxValue;
    else
        newValue = value;

    //std::cout << "newvalue: " << value << std::endl;
    //}

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
        int value = (float) newValue_;

       // std::cout << "val: " << value << std::endl;
       // std::cout << "minvalue: " << minValue << std::endl;
       // std::cout << "maxvalue: " << maxValue << std::endl;

        if (value < minValue)
            newValue = minValue;
        else if (value > maxValue)
            newValue = maxValue;
        else
            newValue = value;

        //std::cout << "newvalue: " << (float)newValue << std::endl;
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
    std::cout << "Parameter received " << newValue_.getArray()->size() << " more channels" << std::endl;
    
    if (newValue_.getArray()->size() <= maxSelectableChannels)
    {
        std::cout << "Setting next value" << std::endl;
        newValue = newValue_;
    } else {
        std::cout << "Not setting next value" << std::endl;
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
    
    //std::cout << getName() << " setting channel count to " << count << std::endl;
    
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
    std::cout << "Parameter received " << newValue_.getArray()->size() << " more channels" << std::endl;
    
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
            value->add(i);
        
    } else if (channelCount > count)
    {
        for (int i = count; i < channelCount; i++)
            value->remove(value->indexOf(var(i)));
    }
    
    channelCount = count;
    
    //std::cout << getName() << " setting channel count to " << count << std::endl;
    
}


/*Parameter::Parameter (const String& name, bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_parameterType                   (PARAMETER_TYPE_BOOLEAN)
    , m_nameValueObject                 (name)
    , m_descriptionValueObject          (String())
    , m_parameterIdValueObject          (ID)
    , m_defaultValueObject              (defaultValue)
{
    m_possibleValues.add (true);
    m_possibleValues.add (false);

    m_minValueObject = 0;
    m_maxValueObject = 0;

    registerValueListeners();
}


Parameter::Parameter (const String& name,
                      float minPossibleValue, float maxPossibleValue, float defaultValue,
                      int ID,
                      bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_parameterType                   (PARAMETER_TYPE_CONTINUOUS)
    , m_nameValueObject                 (name)
    , m_descriptionValueObject          (String())
    , m_parameterIdValueObject          (ID)
    , m_defaultValueObject              (defaultValue)
{
    m_possibleValues.add (minPossibleValue);
    m_possibleValues.add (maxPossibleValue);

    // Initialize default value
    m_values.set (0, defaultValue);

    m_minValueObject = minPossibleValue;
    m_maxValueObject = maxPossibleValue;

    registerValueListeners();
}


Parameter::Parameter (const String& name,
                      Array<var> a,
                      int defaultValue, int ID,
                      bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_parameterType                   (PARAMETER_TYPE_DISCRETE)
    , m_nameValueObject                 (name)
    , m_descriptionValueObject          (String())
    , m_parameterIdValueObject          (ID)
    , m_defaultValueObject              (defaultValue)
{
    m_possibleValues = a;

    m_minValueObject = 0;
    m_maxValueObject = 0;

    registerValueListeners();
}


Parameter::Parameter (const String& name, const String& labelName,
                      double minPossibleValue, double maxPossibleValue, double defaultValue,
                      int ID,
                      bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_parameterType                   (PARAMETER_TYPE_NUMERICAL)
    , m_nameValueObject                 (name)
    , m_descriptionValueObject          (String())
    , m_parameterIdValueObject          (ID)
    , m_defaultValueObject              (defaultValue)
{
    m_possibleValues.add (minPossibleValue);
    m_possibleValues.add (maxPossibleValue);

    // Initialize default value
    m_values.set (0, defaultValue);

    m_minValueObject = minPossibleValue;
    m_maxValueObject = maxPossibleValue;

    registerValueListeners();
}


void Parameter::registerValueListeners()
{
    m_desiredXValueObject.addListener (this);
    m_desiredYValueObject.addListener (this);
    m_desiredWidthValueObject.addListener   (this);
    m_desiredHeightValueObject.addListener  (this);

    m_minValueObject.addListener (this);
    m_maxValueObject.addListener (this);

    m_possibleValuesObject.addListener (this);

    m_nameValueObject.addListener        (this);
    m_parameterIdValueObject.addListener (this);
    m_descriptionValueObject.addListener (this);
    m_defaultValueObject.addListener     (this);
}


String Parameter::getName()          const noexcept { return m_nameValueObject.toString(); }
String Parameter::getDescription()   const noexcept { return m_descriptionValueObject.toString(); }

int Parameter::getID() const noexcept { return m_parameterIdValueObject.getValue(); }

var Parameter::getDefaultValue() const noexcept             { return m_defaultValueObject.getValue(); }
const Array<var>& Parameter::getPossibleValues() const      { return m_possibleValues; }

var Parameter::getValue   (int channel)   const { return m_values[channel]; }
var Parameter::operator[] (int channel)   const { return m_values[channel]; }

Parameter::ParameterType Parameter::getParameterType() const noexcept { return m_parameterType; }

bool Parameter::isBoolean()     const noexcept { return m_parameterType == PARAMETER_TYPE_BOOLEAN; }
bool Parameter::isContinuous()  const noexcept { return m_parameterType == PARAMETER_TYPE_CONTINUOUS; }
bool Parameter::isDiscrete()    const noexcept { return m_parameterType == PARAMETER_TYPE_DISCRETE; }
bool Parameter::isNumerical()   const noexcept { return m_parameterType == PARAMETER_TYPE_NUMERICAL; }

bool Parameter::hasCustomEditorBounds() const noexcept { return m_hasCustomEditorBounds; }

const Rectangle<int>& Parameter::getEditorDesiredBounds() const noexcept { return m_editorBounds; }

Value& Parameter::getValueObjectForID()                noexcept { return m_parameterIdValueObject;     };
Value& Parameter::getValueObjectForName()              noexcept { return m_nameValueObject;            };
Value& Parameter::getValueObjectForDescription()       noexcept { return m_descriptionValueObject;     };
Value& Parameter::getValueObjectForDefaultValue()      noexcept { return m_defaultValueObject;         };
Value& Parameter::getValueObjectForMinValue()          noexcept { return m_minValueObject;             };
Value& Parameter::getValueObjectForMaxValue()          noexcept { return m_maxValueObject;             };
Value& Parameter::getValueObjectForPossibleValues()    noexcept { return m_possibleValuesObject;       };
Value& Parameter::getValueObjectForDesiredX()          noexcept { return m_desiredXValueObject;        };
Value& Parameter::getValueObjectForDesiredY()          noexcept { return m_desiredYValueObject;        };
Value& Parameter::getValueObjectForDesiredWidth()      noexcept { return m_desiredWidthValueObject;    };
Value& Parameter::getValueObjectForDesiredHeight()     noexcept { return m_desiredHeightValueObject;   };

void Parameter::addListener    (Listener* listener)    { m_listeners.add (listener); }
void Parameter::removeListener (Listener* listener)    { m_listeners.remove (listener); }


String Parameter::getParameterTypeString() const noexcept
{
    if (isBoolean())
        return "Boolean";
    else if (isContinuous())
        return "Continuous";
    else if (isDiscrete())
        return "Discrete";
    else if (isNumerical())
        return "Numerical";

    // This should never happen
    jassertfalse;
    return String();
}


Parameter::ParameterType Parameter::getParameterTypeFromString (const String& parameterTypeString)
{
    if (parameterTypeString == "Boolean")
        return Parameter::PARAMETER_TYPE_BOOLEAN;
    else if (parameterTypeString == "Continuous")
        return Parameter::PARAMETER_TYPE_CONTINUOUS;
    else if (parameterTypeString == "Discrete")
        return Parameter::PARAMETER_TYPE_DISCRETE;
    else
        return Parameter::PARAMETER_TYPE_NUMERICAL;
}


int Parameter::getEditorRecommendedWidth() const noexcept
{
    if (isBoolean())
        return 120;
    else if (isContinuous())
        return 80;
    else if (isDiscrete())
        return 35 * getPossibleValues().size();
    else if (isNumerical())
        return 60;
    else
        return 0;
}


int Parameter::getEditorRecommendedHeight() const noexcept
{
    if (isBoolean())
        return 25;
    else if (isContinuous())
        return 80;
    else if (isDiscrete())
        return 30;
    else if (isNumerical())
        return 40;
    else
        return 0;
}


void Parameter::setName (const String& newName)
{
    m_nameValueObject = newName;
}


void Parameter::setDescription (const String& description)
{
    m_descriptionValueObject = description;
}


void Parameter::setValue (float value, int channel)
{
    if (isBoolean())
    {
        const bool newValue = (value > 0.0f) ? true : false;
        m_values.set (channel, newValue);
    }
    else if (isContinuous())
    {
        const float newValue = jlimit (float (m_possibleValues[0]), float (m_possibleValues[1]), value);
        m_values.set (channel, newValue);
    }
    else if (isNumerical())
    {
        const double newValue = jlimit (double (m_possibleValues[0]), double (m_possibleValues[1]), (double)value);
        m_values.set (channel, newValue);
    }
    else
    {
        m_values.set (channel, value);
    }
}


bool Parameter::setValue(const var& val, int chan) {
    if (isBoolean()) {
        if (!val.isBool()) {
            return false;
        }
    }
    else if (isContinuous()) {
        if (!val.isDouble()) {
            return false;
        }
    }
    else if (isDiscrete()) {
        if (!val.isInt()) {
            return false;
        }
    }
    else if (isNumerical()) {
        if (!val.isDouble()) {
            return false;
        }
    }
    /*else if (isContinuousArray()) {
        // Must be an array of doubles
        if (!val.isArray()) {
            return false;
        }
        if (val.size() > 0) {
            if (!val[0].isDouble()) {
                return false;
            }
        }
    }
    else if (isString()) {
        if (!val.isString()) {
            return false;
        }
    }
    else {
        // Unhandled type?
        jassertfalse;
    }
    m_values.set(chan, val);
    return true;
}


int Parameter::getNumChannels() const
{
    return m_values.size();
}



void Parameter::setPossibleValues (Array<var> possibleValues)
{
    m_possibleValues = possibleValues;

    m_possibleValuesObject = convertArrayToString (possibleValues);

    if (possibleValues.size() >= 2)
    {
        m_minValueObject = possibleValues[0];
        m_maxValueObject = possibleValues[1];
    }
}


void Parameter::setEditorDesiredSize (int desiredWidth, int desiredHeight)
{
    setEditorDesiredBounds (m_editorBounds.getX(), m_editorBounds.getY(),
                            desiredWidth, desiredHeight);
}


void Parameter::setEditorDesiredBounds (const Rectangle<int>& desiredBounds)
{
    setEditorDesiredBounds (desiredBounds.getX(), desiredBounds.getY(),
                            desiredBounds.getWidth(), desiredBounds.getHeight());
}


void Parameter::setEditorDesiredBounds (int x, int y, int width, int height)
{
    m_hasCustomEditorBounds = true;

    m_editorBounds.setBounds (x, y, width, height);

    m_desiredXValueObject = m_editorBounds.getX();
    m_desiredYValueObject = m_editorBounds.getY();
    m_desiredWidthValueObject   = m_editorBounds.getWidth();
    m_desiredHeightValueObject  = m_editorBounds.getHeight();
}


void Parameter::valueChanged (Value& valueThatWasChanged)
{
    if (valueThatWasChanged.refersToSameSourceAs (m_desiredXValueObject))
    {
        m_hasCustomEditorBounds = true;
        m_editorBounds.setBounds (valueThatWasChanged.getValue(), m_editorBounds.getY(),
                                  m_editorBounds.getWidth(), m_editorBounds.getHeight());
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_desiredYValueObject))
    {
        m_hasCustomEditorBounds = true;
        m_editorBounds.setBounds (m_editorBounds.getX(), valueThatWasChanged.getValue(),
                                  m_editorBounds.getWidth(), m_editorBounds.getHeight());
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_desiredWidthValueObject))
    {
        m_hasCustomEditorBounds = true;
        m_editorBounds.setBounds (m_editorBounds.getX(), m_editorBounds.getY(),
                                  valueThatWasChanged.getValue(), m_editorBounds.getHeight());
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_desiredHeightValueObject))
    {
        m_hasCustomEditorBounds = true;
        m_editorBounds.setBounds (m_editorBounds.getX(), m_editorBounds.getY(),
                                  m_editorBounds.getWidth(), valueThatWasChanged.getValue());
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_minValueObject))
    {
        if (m_possibleValues.size() >= 2)
            m_possibleValues.getReference (0) = valueThatWasChanged.getValue();
        else
            jassertfalse;
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_maxValueObject))
    {
        if (m_possibleValues.size() >= 2)
            m_possibleValues.getReference (1) = valueThatWasChanged.getValue();
        else
            jassertfalse;
    }
    else if (valueThatWasChanged.refersToSameSourceAs (m_possibleValuesObject))
    {
        m_possibleValues.clear();

        Array<var> possibleValues (createArrayFromString<int> (valueThatWasChanged.toString(), ","));
        m_possibleValues = possibleValues;
    }

    m_listeners.call (&Parameter::Listener::parameterValueChanged, valueThatWasChanged);
}


Parameter* ParameterFactory::createEmptyParameter (Parameter::ParameterType parameterType, int parameterId)
{
    switch (parameterType)
    {
        case Parameter::PARAMETER_TYPE_BOOLEAN:
        {
            auto parameter = new Parameter ("Empty", false, parameterId);
            return parameter;
        }

        case Parameter::PARAMETER_TYPE_CONTINUOUS:
        {
            auto parameter = new Parameter ("Empty", -1.f, 1.f, 0.f, parameterId);
            return parameter;
        }

        case Parameter::PARAMETER_TYPE_DISCRETE:
        {
            Array<var> possibleValues;
            possibleValues.add (0);

            auto parameter = new Parameter ("Empty", possibleValues, 0, parameterId);
            return parameter;
        }

        case Parameter::PARAMETER_TYPE_NUMERICAL:
        {
            auto parameter = new Parameter ("Empty", "Empty", -1.0, 1.0, 0.0, parameterId);
            return parameter;
        }

        default:
            return nullptr;
    };
}


ValueTree Parameter::createValueTreeForParameter (Parameter* parameter)
{
    ValueTree parameterNode ("PARAMETER");
    parameterNode.setProperty (Ids::OpenEphysParameter::ID,                 parameter->getID(), nullptr);
    parameterNode.setProperty (Ids::OpenEphysParameter::NAME,               parameter->getName(), nullptr);
    parameterNode.setProperty (Ids::OpenEphysParameter::TYPE,               parameter->getParameterTypeString(), nullptr);
    parameterNode.setProperty (Ids::OpenEphysParameter::DEFAULT_VALUE,      parameter->getDefaultValue(), nullptr);
    parameterNode.setProperty (Ids::OpenEphysParameter::HAS_CUSTOM_BOUNDS,  parameter->hasCustomEditorBounds(), nullptr);
    parameterNode.setProperty (Ids::OpenEphysParameter::DESIRED_BOUNDS,     parameter->getEditorDesiredBounds().toString(), nullptr);

    if (parameter->isContinuous() || parameter->isDiscrete() || parameter->isNumerical())
        parameterNode.setProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES, convertArrayToString (parameter->getPossibleValues()), nullptr);

    parameterNode.setProperty (Ids::OpenEphysParameter::DESCRIPTION, parameter->getDescription(), nullptr);

    return parameterNode;
}


Parameter* Parameter::createParameterFromValueTree (ValueTree parameterValueTree)
{
    if (! parameterValueTree.isValid())
        return nullptr;

    Parameter::ParameterType parameterType
        = Parameter::getParameterTypeFromString (parameterValueTree.getProperty (Ids::OpenEphysParameter::TYPE));

    auto id     = (int)parameterValueTree.getProperty   (Ids::OpenEphysParameter::ID);
    auto name   = parameterValueTree.getProperty        (Ids::OpenEphysParameter::NAME).toString();

    Parameter* parameter = nullptr;
    // Boolean parameter
    if (parameterType == Parameter::PARAMETER_TYPE_BOOLEAN)
    {
        auto defaultValue = (bool) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, defaultValue, id);
    }
    // Continuous parameter
    else if (parameterType == Parameter::PARAMETER_TYPE_CONTINUOUS)
    {
        String arrayString = parameterValueTree.getProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES);
        Array<var> possibleValues (createArrayFromString<float> (arrayString, ","));

        auto minPossibleValue = float (possibleValues[0]);
        auto maxPossibleValue = float (possibleValues[1]);
        auto defaultValue = (float) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, minPossibleValue, maxPossibleValue, defaultValue, id);
    }
    // Discrete parameter
    else if (parameterType == Parameter::PARAMETER_TYPE_DISCRETE)
    {
        String arrayString = parameterValueTree.getProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES);
        Array<var> possibleValues (createArrayFromString<int> (arrayString, ","));

        auto defaultValue = (int) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, possibleValues, defaultValue, id);
    }
    // Numerical parameter
    else
    {
        String arrayString = parameterValueTree.getProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES);
        Array<var> possibleValues (createArrayFromString<double> (arrayString, ","));

        auto minPossibleValue = double (possibleValues[0]);
        auto maxPossibleValue = double (possibleValues[1]);
        auto defaultValue = (double) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, name, minPossibleValue, maxPossibleValue, defaultValue, id);
    }

    // Set custom bounds if needed
    auto hasCustomEditorBounds = (bool) parameterValueTree.getProperty (Ids::OpenEphysParameter::HAS_CUSTOM_BOUNDS);
    if (hasCustomEditorBounds)
    {
        auto desiredBounds = Rectangle<int>::fromString (parameterValueTree.getProperty (Ids::OpenEphysParameter::DESIRED_BOUNDS).toString());
        parameter->setEditorDesiredBounds (desiredBounds);
    }

    auto description = parameterValueTree.getProperty (Ids::OpenEphysParameter::DESCRIPTION).toString();
    parameter->setDescription (description);

    return parameter;
}*/


