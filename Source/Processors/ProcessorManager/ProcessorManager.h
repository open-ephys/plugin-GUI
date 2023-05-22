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

#ifndef PROCESSORMANAGER_H_INCLUDED
#define PROCESSORMANAGER_H_INCLUDED

#include "../PluginManager/OpenEphysPlugin.h"

#include "../ProcessorGraph/ProcessorGraph.h"

/**
 Creates processors (including built-in ones)
 */
namespace ProcessorManager
{

    /** Returns the types of plugins that can generate processors*/
    Array<Plugin::Type> getAvailablePluginTypes();

    /** Returns the number of processors available for a given plugin type*/
	int getNumProcessorsForPluginType(Plugin::Type type);
	
    /** Returns info about a plugin at a particular index*/
    Plugin::Description getPluginDescription(Plugin::Type type, int index);

	/** Returns info about a plugin with a particular name*/
	Plugin::Description getPluginDescription(String name);

    /** Creates a new processor from its description*/
    std::unique_ptr<GenericProcessor> createProcessor(Plugin::Description description);

	/** Returns an array of strings of available processor */
	Array<String> getAvailableProcessors();

};



#endif  // PROCESSORMANAGER_H_INCLUDED
