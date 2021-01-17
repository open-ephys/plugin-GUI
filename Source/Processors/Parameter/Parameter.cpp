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
#include "ParameterHelpers.h"


Parameter::Parameter (const String& name, bool defaultValue, int ID, bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_parameterType                   (PARAMETER_TYPE_BOOLEAN)
    , m_nameValueObject                 (name)
    , m_descriptionValueObject          (String::empty)
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
    , m_descriptionValueObject          (String::empty)
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
    , m_descriptionValueObject          (String::empty)
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
    , m_descriptionValueObject          (String::empty)
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
    return String::empty;
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
}


