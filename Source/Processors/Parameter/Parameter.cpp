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
#include "../../../PluginGenerator/Source/Utility/openEphys_pluginHelpers.h"


Parameter::Parameter (const String& name, bool defaultValue, int ID, bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_name                            (name)
    , m_description                     ("")
    , m_parameterId                     (ID)
    , m_parameterType                   (PARAMETER_TYPE_BOOLEAN)
    , m_defaultValue                    (defaultValue)
{
    m_possibleValues.add (true);
    m_possibleValues.add (false);
}


Parameter::Parameter (const String& name,
                      float minPossibleValue, float maxPossibleValue, float defaultValue,
                      int ID,
                      bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_name                            (name)
    , m_description                     ("")
    , m_parameterId                     (ID)
    , m_parameterType                   (PARAMETER_TYPE_CONTINUOUS)
    , m_defaultValue                    (defaultValue)
{
    m_possibleValues.add (minPossibleValue);
    m_possibleValues.add (maxPossibleValue);

    // Initialize default value
    m_values.set (0, m_defaultValue);
}


Parameter::Parameter (const String& name,
                      Array<var> a,
                      int defaultValue, int ID,
                      bool deactivateDuringAcquisition)
    : shouldDeactivateDuringAcquisition (deactivateDuringAcquisition)
    , m_name                            (name)
    , m_description                     ("")
    , m_parameterId                     (ID)
    , m_parameterType                   (PARAMETER_TYPE_DISCRETE)
    , m_defaultValue                    (defaultValue)
{
    m_possibleValues = a;
}


const String& Parameter::getName()          const noexcept { return m_name; }
const String& Parameter::getDescription()   const noexcept { return m_description; }

int Parameter::getID() const noexcept { return m_parameterId; }

var Parameter::getDefaultValue() const noexcept             { return m_defaultValue; }
const Array<var>& Parameter::getPossibleValues() const { return m_possibleValues; }

var Parameter::getValue   (int channel)   const { return m_values[channel]; }
var Parameter::operator[] (int channel)   const { return m_values[channel]; }

Parameter::ParameterType Parameter::getParameterType() const noexcept { return m_parameterType; }

bool Parameter::isBoolean()     const noexcept { return m_parameterType == PARAMETER_TYPE_BOOLEAN; }
bool Parameter::isContinuous()  const noexcept { return m_parameterType == PARAMETER_TYPE_CONTINUOUS; }
bool Parameter::isDiscrete()    const noexcept { return m_parameterType == PARAMETER_TYPE_DISCRETE; }

bool Parameter::hasCustomEditorBounds() const noexcept { return m_hasCustomEditorBounds; }

const Rectangle<int>& Parameter::getEditorDesiredBounds() const noexcept { return m_editorBounds; }


String Parameter::getParameterTypeString() const noexcept
{
    if (isBoolean())
        return "Boolean";
    else if (isContinuous())
        return "Continuous";
    else if (isDiscrete())
        return "Discrete";

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
    else
        return Parameter::PARAMETER_TYPE_DISCRETE;
}


int Parameter::getEditorRecommendedWidth() const noexcept
{
    if (isBoolean())
        return 120;
    else if (isContinuous())
        return 80;
    else if (isDiscrete())
        return 35 * getPossibleValues().size();
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
    else
        return 0;
}


void Parameter::setName (const String& newName)
{
    m_name = newName;
}


void Parameter::setDescription (const String& description)
{
    m_description = description;
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
    else
    {
        m_values.set (channel, value);
    }
}


void Parameter::setPossibleValues (Array<var> possibleValues)
{
    m_possibleValues = possibleValues;
}


void Parameter::setEditorDesiredSize (int desiredWidth, int desiredHeight)
{
    m_hasCustomEditorBounds = true;

    m_editorBounds.setSize (desiredWidth, desiredHeight);
}


void Parameter::setEditorDesiredBounds (int x, int y, int width, int height)
{
    m_hasCustomEditorBounds = true;

    m_editorBounds.setBounds (x, y, width, height);
}


void Parameter::setEditorDesiredBounds (const Rectangle<int>& desiredBounds)
{
    m_hasCustomEditorBounds = true;

    m_editorBounds = desiredBounds;
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

    if (parameter->isContinuous() || parameter->isDiscrete())
        parameterNode.setProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES, convertArrayToString (parameter->getPossibleValues()), nullptr);

    return parameterNode;
}


Parameter* Parameter::createParameterFromValueTree (ValueTree parameterValueTree)
{
    if (! parameterValueTree.isValid())
        return nullptr;

    Parameter::ParameterType parameterType
        = Parameter::getParameterTypeFromString (parameterValueTree.getProperty (Ids::OpenEphysParameter::TYPE));

    auto id   = (int) parameterValueTree.getProperty (Ids::OpenEphysParameter::ID);
    auto name = parameterValueTree.getProperty (Ids::OpenEphysParameter::NAME).toString();

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
        //Array<var> possibleValues = { 0, 1 };
        String arrayString = parameterValueTree.getProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES);
        Array<var> possibleValues (createArrayFromString<float> (arrayString, ","));

        auto minPossibleValue = float (possibleValues[0]);
        auto maxPossibleValue = float (possibleValues[1]);
        auto defaultValue = (float) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, minPossibleValue, maxPossibleValue, defaultValue, id);
    }
    // Discrete parameter
    else
    {
        //Array<var> possibleValues;
        String arrayString = parameterValueTree.getProperty (Ids::OpenEphysParameter::POSSIBLE_VALUES);
        Array<var> possibleValues (createArrayFromString<int> (arrayString, ","));

        auto defaultValue = (int) parameterValueTree.getProperty (Ids::OpenEphysParameter::DEFAULT_VALUE);

        parameter = new Parameter (name, possibleValues, defaultValue, id);
    }

    // Set custom bounds if needed
    auto hasCustomEditorBounds = (bool) parameterValueTree.getProperty (Ids::OpenEphysParameter::HAS_CUSTOM_BOUNDS);
    if (hasCustomEditorBounds)
    {
        auto desiredBounds = Rectangle<int>::fromString (parameterValueTree.getProperty (Ids::OpenEphysParameter::DESIRED_BOUNDS).toString());
        parameter->setEditorDesiredBounds (desiredBounds);
    }

    return parameter;
}


