/*
    ------------------------------------------------------------------

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

#ifndef __PARAMETER_H_62922AE5__
#define __PARAMETER_H_62922AE5__

#include "../../../JuceLibraryCode/JuceHeader.h"
// #include "Editors/GenericEditor.h"
// #include "GenericProcessor.h"
// #include "../AccessClass.h"

#include <stdio.h>

/**

  Class for holding user-definable processor parameters.

  Parameters can either hold boolean, categorical, or continuous (float) values.

  Using the Parameter class makes it easier to create a graphical interface for editing
  parameters, because each Parameter has a ParameterEditor that is created automatically.

  @see GenericProcessor, GenericEditor

*/

class Parameter
{
public:

    /** Constructor for boolean parameters.*/
    Parameter(const String& name_, bool defaultVal, int ID, bool t = false);

    /** Constructor for continuous (float) parameters.*/
    Parameter(const String& name_, float low, float high, float defaultVal, int ID, bool t = false);

    /** Constructor for categorical parameters.*/
    Parameter(const String& name_, Array<var> a, int defaultVal, int ID, bool t = false);

    /** Destructor.*/
	~Parameter();

    /** Returns the name of the parameter.*/
	const String& getName();

    /** Returns a description of the parameter.*/
	const String& getDescription();

    /** Sets the description of the parameter.*/
	void addDescription(const String& desc);

    /** Returns the default value of a parameter (can be boolean, int, or float).*/
	var getDefaultValue();

    /** Returns the unique integer ID of a parameter.*/
	int getID();

    /** Returns all the possible values that a parameter can take.*/
	Array<var> getPossibleValues();

    /** Sets the value of a parameter for a given channel.*/
    void setValue(float val, int chan);

    /** Returns the value of a parameter for a given channel.*/
	var operator[](int chan);

    /** Returns the value of a parameter for a given channel.*/
	var getValue(int chan);

    /** Copies a parameter.*/
    Parameter& operator=(const Parameter& other);

    /** Returns true if a parameter is boolean, false otherwise.*/
	bool isBoolean();

    /** Returns true if a parameter is continuous, false otherwise.*/
	bool isContinuous();

    /** Returns true if a parameter is discrete, false otherwise.*/
	bool isDiscrete();

    /** Certain parameters should not be changed while data acquisition is active.

         This variable indicates whether or not these parameters can be edited.*/
    bool shouldDeactivateDuringAcquisition;

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
