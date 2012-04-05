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
#include "GenericProcessor.h"
#include "../AccessClass.h"

#include <stdio.h>

/**
  
  Class for holding user-definable processor parameters.

  @see GenericProcessor, GenericEditor

*/

class GenericProcessor;
class GenericEditor;

class Parameter
{
public:

	Parameter(const String& name_) : name(name_), description("") {}
	~Parameter() {}

	const String& getName() {return name;}
	const String& getDescription() {return description;}

	void addDescription(const String& desc) {description = desc;}

	Array<var> getPossibleValues() {return possibleValues;}
	var getDefaultValue() {return defaultValue;}

	bool setValue(var val, int chan);

	const var& getValue(int chan);
	const var& operator[](int chan);

	virtual bool isBoolean() {return false;}
	virtual bool isContinuous() {return false;}
	virtual bool isDiscrete() {return false;}

protected:

	const String name;
	String description;

	GenericProcessor* processor;
	GenericEditor* editor;
	//Array<Channel*> channels;

	Array<var> possibleValues;
	Array<var> values;

	var defaultValue;

};

class BooleanParameter : public Parameter
{
public:
	BooleanParameter(const String& name_, bool& defaultVal);
	~BooleanParameter() {}

	bool isBoolean() {return true;}

};

class ContinuousParameter : public Parameter
{
public:
	ContinuousParameter(const String& name_, double low, double high, double& defaultVal);
	~ContinuousParameter() {}

	bool isContinuous() {return true;}

};


class DiscreteParameter : public Parameter
{
public:
	DiscreteParameter(const String& name_, Array<var> a, int defaultVal);
	~DiscreteParameter() {}

	bool isDiscrete() {return true;}
};

#endif  // __PARAMETER_H_62922AE5__
