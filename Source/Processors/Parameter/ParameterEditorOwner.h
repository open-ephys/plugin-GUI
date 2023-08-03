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

#ifndef PARAMETEREDITOROWNER_H_INCLUDED
#define PARAMETEREDITOROWNER_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "Parameter.h"
#include "ParameterEditor.h"

/**
* Holds parameters and provides ways to access and modify those parameters.
* ParameterEditorOwner can be inherited by anything that holds information about specific
* settings. For example, a DataStream might have parameters that store the
* the values of filter cutoffs.
* 
*/
class PLUGIN_API ParameterEditorOwner
{

public:
	/** Destructor */
    virtual ~ParameterEditorOwner();

    /** Custom copy constructor -- set isLocal to false and don't copy parameters*/
    ParameterEditorOwner(const ParameterEditorOwner& other);

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a parameter to this object**/
    void addParameterEditor(ParameterEditor* p, int xPos, int yPos);

    /** Returns a pointer to a parameter with a given name**/
    ParameterEditor* getParameterEditor(String name) const;

	/** Returns true if an object has a parameter with a given name**/
	bool hasParameterEditor(String name) const;

    /** Returns a pointer to a parameter with a given name**/
    Array<ParameterEditor*> getParameterEditors();

    /** Returns the number of parameters for this object*/
    int numParameterEditors() const { return parameterEditors.size(); }
    
    Component* getComponent() const { return ownerComponent; }

protected:
	/** Constructor*/
	ParameterEditorOwner(Component* ownerComponent);

private:

    Component* ownerComponent;
	OwnedArray<ParameterEditor> parameterEditors;

};

#endif
