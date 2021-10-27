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

class Parameter;

/**
    Class for holding a group of Parameter objects associated with a
         particular DataStream, SpikeChannel, etc.

    @see GenericProcessor, GenericEditor
*/


class PLUGIN_API ParameterCollection
{
public:

    ParameterCollection() { }
    
    virtual ~ParameterCollection() {}
    
    void addParameter(Parameter*);
    
    Parameter* operator [](String name) const {return parameterMap.at(name) ;}
    
    Parameter*& operator [](String name) {return parameterMap[name];}
    
    int size() const {return parameters.size(); }
    
private:
    
    OwnedArray<Parameter> parameters;
    
    std::map<String, Parameter*> parameterMap;
};
    
    

#endif /* __PARAMETERCOLLECTION_H_62922AE5__ */
