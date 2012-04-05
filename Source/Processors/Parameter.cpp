/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

bool Parameter::setValue(var val, int chan)
{
	if (isBoolean())
	{
		if (val.isBool()) {
			values.set(chan, val);

		} else if (val.isInt()) {

			if (int(val) > 0)
				values.set(chan, true);
			else
				values.set(chan, false);

		} else if (val.isDouble()) {

			if (double(val) > 0.0f)
				values.set(chan, true);
			else
				values.set(chan, false);

		} else {
			return false;
		}

	} else if (isContinuous()) {
		
		if (val.isDouble())
		{
			if (double(val) < double(possibleValues[0]))
			{
				values.set(chan, possibleValues[0]);
			} else if (double(val) > double(possibleValues[1]))
			{
				values.set(chan, possibleValues[1]);
			} else {
				values.set(chan, val);
			}
		} else {
			return false;
		}

	} else if (isDiscrete()) {

		if (val.isInt())
		{
			if (int(val) >= 0 && int(val) < possibleValues.size())
			{
				values.set(chan, possibleValues[val]);
			} else {
				return false;
			}
		} else {
			return false;
		}

	}

	return true;

}

const var& Parameter::getValue(int chan)
{
	return values[chan];
}

const var& Parameter::operator[](int chan)
{
	return values[chan];
}

BooleanParameter::BooleanParameter(const String& name_, bool& defaultVal) : Parameter(name_)
{
	possibleValues.add(true);
	possibleValues.add(false);

	defaultValue = defaultVal;
}

ContinuousParameter::ContinuousParameter(const String& name_,
										 double low, double high, double& defaultVal)
										 : Parameter(name_)
{
	possibleValues.add(low);
	possibleValues.add(high);

	defaultValue = defaultVal;

}

DiscreteParameter::DiscreteParameter(const String& name_,
									 Array<var> a, int defaultVal)
										 : Parameter(name_)
{
	possibleValues = a;

	defaultValue = possibleValues[defaultVal];
}

