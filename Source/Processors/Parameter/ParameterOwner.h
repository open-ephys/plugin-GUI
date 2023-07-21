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

#ifndef PARAMETEROWNER_H_INCLUDED
#define PARAMETEROWNER_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "ParameterCollection.h"
#include "Parameter.h"
#include "ParameterEditor.h"

class ParameterCollection;

/**
* Holds parameters and provides ways to access and modify those parameters.
* ParameterOwner can be inherited by anything that holds information about specific
* settings. For example, a DataStream might have parameters that store the
* the values of filter cutoffs.
* 
*/
class PLUGIN_API ParameterOwner
{

public:
	/** Destructor */
    virtual ~ParameterOwner();

    /** Custom copy constructor -- set isLocal to false and don't copy parameters*/
    ParameterOwner(const ParameterOwner& other);

	/** ParameterOwner::Type*/
	enum Type
	{
		// A channel that's sampled at regular intervals
		CONTINUOUS_CHANNEL,

		// A channel that sends events at arbitrary times
		EVENT_CHANNEL,

		// A channel that sends spike events
		SPIKE_CHANNEL,

		// A collection of synchronously sampled continuous channels
		DATASTREAM,

		// Processor, Visualizer, or any other global parameter owners
		OTHER
	};

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a parameter to this object**/
    void addParameter(Parameter* p);

    /** Returns a pointer to a parameter with a given name**/
    Parameter* getParameter(String name) const { return parameters[name]; }

	/** Returns true if an object has a parameter with a given name**/
	bool hasParameter(String name) const { return parameters.contains(name); }

    /** Returns a pointer to a parameter with a given name**/
    Array<Parameter*> getParameters() { return parameters.getParameters(); }

	/** Returns a pointer to a parameter with a given name**/
	Array<String> getParameterNames() const;

	/*Bracket operator returns the value of a parameter*/
    var operator [](String name) const {return parameters[name]->getValue();}

    /** Copies parameters from another ParameterOwner and clears the original ParameterCollection*/
    void copyParameters(ParameterOwner* object);

    /** Returns the number of parameters for this object*/
    int numParameters() const { return parameters.size(); }

	/** Initiates parameter value update */
    virtual void parameterChangeRequest(Parameter*) { }

    /** Called when a parameter value is updated, to allow plugin-specific responses*/
    virtual void parameterValueChanged(Parameter*) { }

	/** Creates a simple editor for modifying this object's parameters */
	virtual Array<ParameterEditor*> createDefaultEditor();
	

	// --------------------------------------------
	//    OTHER METHODS
	// --------------------------------------------
	
	/** Returns the type of the ParameterOwner*/
	const Type getType() const;

	/** Holds the parameters for this object */
    ParameterCollection parameters;


protected:
	/** Constructor*/
	ParameterOwner(Type type);

private:

	const Type m_type;
};

#endif
