
/*    ------------------------------------------------------------------

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


#include "Parameter.h"


Parameter::Parameter(const String& name_, bool defaultVal, int ID, bool t)
    : shouldDeactivateDuringAcquisition(t), name(name_), description(""),
      parameterId(ID)
{

    defaultValue = defaultVal;

    possibleValues.add(true);
    possibleValues.add(false);

    isBool = true;
    isCont = false;
    isDisc = false;

}

Parameter::Parameter(const String& name_, float low, float high,
                     float defaultVal, int ID, bool t)
    : shouldDeactivateDuringAcquisition(t), name(name_), description(""),
      parameterId(ID)
{
    defaultValue = defaultVal;

    possibleValues.add(low);
    possibleValues.add(high);

    isCont = true;
    isBool = false;
    isDisc = false;

    // Initialize default value
    values.set (0, defaultValue);
}

Parameter::Parameter(const String& name_, Array<var> a, int defaultVal,
                     int ID, bool t)
    : shouldDeactivateDuringAcquisition(t), name(name_), description(""),
      parameterId(ID)
{
    possibleValues = a;
    defaultValue = defaultVal; //possibleValues[defaultVal];

    isCont = false;
    isDisc = true;
    isBool = false;

}

Parameter::~Parameter() {}

const String& Parameter::getName()
{
	return name;
}

const String& Parameter::getDescription()
{
	return description;
}

void Parameter::addDescription(const String& desc)
{
	description = desc;
}

var Parameter::getDefaultValue()
{
	return defaultValue;
}

int Parameter::getID()
{
	return parameterId;
}

Array<var> Parameter::getPossibleValues()
{
	return possibleValues;
}

void Parameter::setValue(float val, int chan)
{

    if (isBoolean())
    {
        if (val > 0.0f)
            values.set(chan, true);
        else
            values.set(chan, false);
    }
    else if (isContinuous())
    {

        if (val < (float) possibleValues[0])
        {
            values.set(chan, possibleValues[0]);
        }
        else if (val > (float) possibleValues[1])
        {
            values.set(chan, possibleValues[1]);
        }
        else
        {
            values.set(chan, val);
        }

    }
    else
    {
        //int index = (int) val;

        //if (index >= 0 && index < possibleValues.size())
        //{
        values.set(chan, val);
        //}

    }

}

var Parameter::operator[](int chan)
{
	return values[chan];
}

var Parameter::getValue(int chan)
{
	return values[chan];
}


bool Parameter::isBoolean()
{
	return isBool;
}

bool Parameter::isContinuous()
{
	return isCont;
}

bool Parameter::isDiscrete()
{
	return isDisc;
}

// void BooleanParameter::setValue(float val, int chan)
// {

// 	var b = true;
// 	bool c = b;

// 	if (val > 0)
// 		values.set(chan, true);
// 	else
// 		values.set(chan, false);

// }

// void ContinuousParameter::setValue(float val, int chan)
// {
// 	if (val < low)
// 	{
// 		values.set(chan, low);
// 	} else if (val > high) {
// 		values.set(chan, high);
// 	} else {
// 		values.set(chan, val);
// 	}
// }

// void DiscreteParameter::setValue(float val, int chan)
// {
// 	int index = (int) val;

// 	if (index >= 0 && index < possibleValues.size())
// 	{
// 		values.set(chan, possibleValues[index]);
// 	}

// }

// Array<var> BooleanParameter::getPossibleValues()
// {
// 	Array<var> a;
// 	a.add(true);
// 	a.add(false);

// 	return a;

// }

// Array<var> ContinuousParameter::getPossibleValues()
// {
// 	Array<var> a;
// 	a.add(low);
// 	a.add(high);

// 	return a;
// }

// Array<var> DiscreteParameter::getPossibleValues()
// {
// 	return possibleValues;

// }


// void* BooleanParameter::operator[](int chan)
// {
// 	return (void*) values[chan];
// }

// void* ContinuousParameter::operator[](int chan)
// {
// 	return (void*) values[chan];
// }


// void* DiscreteParameter::operator[](int chan)
// {
// 	return (void*) values[chan];
// }

// BooleanParameter::BooleanParameter(const String name_, bool defaultVal) : Parameter(name_)
// {
// 	defaultValue = defaultVal;
// }

// ContinuousParameter::ContinuousParameter(const String name_,
// 										 float low_, float high_, float defaultVal)
// 										 : Parameter(name_)
// {
// 	low = low_;
// 	high = high_;

// 	defaultValue = defaultVal;

// }

// DiscreteParameter::DiscreteParameter(const String name_,
// 									 Array<var> a, int defaultVal)
// 										 : Parameter(name_)
// {
// 	possibleValues = a;

// 	defaultValue = possibleValues[defaultVal];
// }

