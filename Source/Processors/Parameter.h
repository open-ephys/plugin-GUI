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
// #include "Editors/GenericEditor.h"
// #include "GenericProcessor.h"
// #include "../AccessClass.h"

#include <stdio.h>

/**
  
  Class for holding user-definable processor parameters.

  @see GenericProcessor, GenericEditor

*/

class Parameter
{
public:

	Parameter(const String& name_, bool defaultVal, int ID);
	Parameter(const String& name_, float low, float high, float defaultVal, int ID);
	Parameter(const String& name_, Array<var> a, int defaultVal, int ID);

	~Parameter() {}

	const String& getName() {return name;}
	const String& getDescription() {return description;}
	void addDescription(const String& desc) {description = desc;}

	var getDefaultValue() {return defaultValue;}

	int getID() {return parameterId;}

	Array<var> getPossibleValues() {return possibleValues;}
	void setValue(float val, int chan);

	var operator[](int chan) {return values[chan];}
	Parameter& operator=(const Parameter& other);

	bool isBoolean() {return isBool;}
	bool isContinuous() {return isCont;}
	bool isDiscrete() {return isDisc;}

private:


	const String name;
	String description;

	int parameterId;

	bool isBool, isCont, isDisc;

	var defaultValue;
	Array<var> values;
	Array<var> possibleValues;

};

// class BooleanParameter : public Parameter
// {
// public:
// 	BooleanParameter(const String name_, bool defaultVal);
// 	~BooleanParameter() {}

// 	Array<var> getPossibleValues();
// 	void setValue(float val, int chan);
// 	void* operator[](int chan);

// 	bool isBoolean() {return true;}

// 	bool defaultValue;

// 	Array<bool> values;

// };

// class ContinuousParameter : public Parameter
// {
// public:
// 	ContinuousParameter(const String name_, float low, float high, float defaultVal);
// 	~ContinuousParameter() {}

// 	Array<var> getPossibleValues();
// 	void setValue(float val, int chan);
// 	void* operator[](int chan);

// 	bool isContinuous() {return true;}

// 	float low, high, defaultValue;

// 	Array<float> values;

// };


// class DiscreteParameter : public Parameter
// {
// public:
// 	DiscreteParameter(const String name_, Array<var> a, int defaultVal);
// 	~DiscreteParameter() {}

// 	Array<var> getPossibleValues();
// 	void setValue(float val, int chan);
// 	void* operator[](int chan);

// 	bool isDiscrete() {return true;}

// 	Array<var> possibleValues;
// 	Array<var> values;

// 	int defaultValue;
// };


// template <class Type>

// class Parameter
// {
// public:
// 	Parameter(const String& name_,
// 			  Type defaultVal,
// 			  Array<var> possibleVals = Array<var>()) :
// 			   name(name_), defaultValue(defaultVal), possibleValues(possibleVals)
// 	{

// 	}

// 	Type operator[](int chan) {return values[chan];}

// private:

// 	const String name;
// 	Type defaultValue;
// 	Array<Type> values;
// 	Array<var> possibleValues;

// };


#endif  // __PARAMETER_H_62922AE5__
