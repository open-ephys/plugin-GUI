/*    ------------------------------------------------------------------

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


Parameter::~Parameter() {}


const String& Parameter::getName()          const noexcept { return m_name; }
const String& Parameter::getDescription()   const noexcept { return m_description; }

int Parameter::getID() const noexcept { return m_parameterId; }

var Parameter::getDefaultValue() const noexcept             { return m_defaultValue; }
Array<var> Parameter::getPossibleValues() const noexcept    { return m_possibleValues; }

var Parameter::getValue   (int channel)   const { return m_values[channel]; }
var Parameter::operator[] (int channel)   const { return m_values[channel]; }

bool Parameter::isBoolean()     const noexcept { return m_parameterType == PARAMETER_TYPE_BOOLEAN; }
bool Parameter::isContinuous()  const noexcept { return m_parameterType == PARAMETER_TYPE_CONTINUOUS; }
bool Parameter::isDiscrete()    const noexcept { return m_parameterType == PARAMETER_TYPE_DISCRETE; }


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
