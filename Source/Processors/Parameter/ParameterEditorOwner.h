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
* Holds parameter editors and provides ways to access and update them.
* ParameterEditorOwner can be inherited by anything that provides an interface
* for specific settings. For example, a Visualizer compoenent that has a
* combobox for setting voltage range.
*/
class PLUGIN_API ParameterEditorOwner
{

public:
	/** Destructor */
    virtual ~ParameterEditorOwner();

    /** Custom copy constructor*/
    ParameterEditorOwner(const ParameterEditorOwner& other);

    // --------------------------------------------
    //     PARAMETERS
    // --------------------------------------------

    /** Adds a parameter editor to this object**/
    void addParameterEditor(ParameterEditor* p, int xPos, int yPos);

    /** Returns a pointer to a parameter editor with a given name**/
    ParameterEditor* getParameterEditor(String name) const;

	/** Returns true if an object has a parameter editor with a given name**/
	bool hasParameterEditor(String name) const;

    /** Returns array of pointers to all parameter editors **/
    Array<ParameterEditor*> getParameterEditors();

    /** Returns the number of parameter editors for this object*/
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
