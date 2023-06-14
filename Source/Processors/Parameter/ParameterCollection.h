/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#ifndef __PARAMETERCOLLECTION_H_62922AE5__
#define __PARAMETERCOLLECTION_H_62922AE5__

#include <JuceHeader.h>
#include "../PluginManager/OpenEphysPlugin.h"

class InfoObject;

class Parameter;

/**
    Class for holding a group of Parameter objects associated with a
         particular DataStream, SpikeChannel, etc.

    @see GenericProcessor, GenericEditor
*/
class PLUGIN_API ParameterCollection
{
public:

    /** Constructor */
    ParameterCollection() { }
    
    /** Initializes parameters from an InfoObject */
    ParameterCollection(InfoObject* object);
    
    /** Destructor */
    virtual ~ParameterCollection();
    
    /** Adds a new parameter to the collection */
    void addParameter(Parameter*);
    
    /** Returns pointers to all parameters in the collection */
    Array<Parameter*> getParameters();

    /** Copies parameter values to an InfoObject with existing Parameters */
    void copyParameterValuesTo(InfoObject* object);
    
    /** Copies parameters to an InfoObject */
    void copyParametersTo(InfoObject* object);
    
    /** Copies parameters from an InfoObject */
    void copyParametersFrom(InfoObject* object);
    
    /** Overload indexing operator */
    Parameter* operator [](String name) const {return parameterMap.at(name) ;}
    
    /** Overload indexing operator */
    Parameter*& operator [](String name) {return parameterMap[name];}

    /** Returns true if a parameter with a given name exists */
    bool contains(String name) const { return parameterMap.find(name) != parameterMap.end(); }
    
    /** Returns the total number of parameters in this collection*/
    int size() const {return parameters.size(); }
    
    /** Removes all parameters from the collection*/
    void clear();

    struct Owner {
        String name = "None";
        uint16 streamId = 0;
        int sourceNodeId = 0;
        float sample_rate = 0.0f;
        int channel_count = 0;
        String deviceName = "None";
    };

    Owner owner;
    
private:
    
    OwnedArray<Parameter> parameters;
    
    std::map<String, Parameter*> parameterMap;
};
    
    

#endif /* __PARAMETERCOLLECTION_H_62922AE5__ */
