/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2013 Open Ephys

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

#ifndef PLUGINCLASS_H_INCLUDED
#define PLUGINCLASS_H_INCLUDED

#include "OpenEphysPlugin.h"

/**
 Retrieves information about available plugins
 */
class PLUGIN_API PluginClass
{
public:
    
    /** Constructor */
	PluginClass();
    
    /** Destructor*/
	~PluginClass();
    
    /** Sets the pluginType*/
	void setPluginData(Plugin::Type type, int index);
    
    /** Returns the library name of this plugin*/
	String getLibName() const;
    
    /** Returns the name of this plugin*/
	String getPluginName() const;
    
    /** Returns the library version*/
	String getLibVersion() const;
    
    /** Returns the type of this plugin*/
	Plugin::Type getPluginType() const;
    
    /** Returns the index of this plugin*/
	int getIndex() const;
    
private:
	String libName;
	String pluginName;
	Plugin::Type pluginType;
	String libVersion;
	int pluginIndex;
};



#endif  // PLUGINCLASS_H_INCLUDED
