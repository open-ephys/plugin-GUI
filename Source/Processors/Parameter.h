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

#ifndef __PARAMETER_H_62922AE5__
#define __PARAMETER_H_62922AE5__

#include "../../JuceLibraryCode/JuceHeader.h"
#include "Editors/GenericEditor.h"
#include "../AccessClass.h"

#include <stdio.h>

/**
  
  Class for holding user-definable processor parameters.

  @see GenericProcessor, GenericEditor

*/

class Parameter :
{
public:

	Parameter(const String name_);
	~Parameter();

	const String getName() {return name;}

	virtual Array<var> getPossibleValues() {return possibleValues}
	virtual void setValue(var val, int chan);

	var getValue(int chan);
	var operator[](int chan);

	virtual bool isBoolean() {return false;}
	virtual bool isContinuous() {return false;}
	virtual bool isDiscrete() {return false;}
	virtual bool isString() {return false;}

protected:

	const String name;

	GenericProcessor* processor;
	GenericEditor* editor;
	Array<Channel*> channels;

	Array<var> possibleValues;
	Array<var> values;

};

class BooleanParameter : public Parameter
{
public:
	BooleanParameter(const String name_);
	~BooleanParameter();

	Array<var> getPossibleValues()
	{
		Array<var> possibleValues;
		possibleValues.add(true);
		possibleValues.add(false);

		return possibleValues;
	}

	bool isBoolean() {return true;}

};

class ContinuousParameter : public Parameter
{
public:
	ContinuousParameter(const String name_, double low, double high);
	~ContinuousParameter();

	Array<var> getPossibleValues()
	{
		Array<var> possibleValues;
		possibleValues.add(low);
		possibleValues.add(high);

		return possibleValues;
	}

	bool isContinuous() {return true;}

private:
	double low, high;

};


class DiscreteParameter : public Parameter
{
public:
	DiscreteParameter(const String name_, Array<var> possibleValues);
	~DiscreteParameter();

	Array<var> getPossibleValues()
	{
		return possibleValues;
	}

	bool isContinuous() {return true;}

private:
	
	Array<var> possibleValues;

};

#endif  // __PARAMETER_H_62922AE5__
