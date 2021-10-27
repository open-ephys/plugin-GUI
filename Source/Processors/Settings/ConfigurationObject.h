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

#ifndef CONFIGOOBJECT_H_INCLUDED
#define CONFIGOOBJECT_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "InfoObject.h"

class ProcessorInfoObject;

/**
	Class for defining extra configuration objects to be shared with processors down the chain not 
	associated with any particular channel or event.

	It does not hold any data by itself, but can be filled with arbitrary metadata
*/
class PLUGIN_API ConfigurationObject : public InfoObject
{
public:
	/**Default constructor
	@param source - The processor from which this object originates
	@param name - The name of this object
	@param description (optional) - The description field for this object
	*/
	ConfigurationObject(ProcessorInfoObject* source, String name, String description = "");

    virtual ~ConfigurationObject();

	const ProcessorInfoObject* getSource() const;

private:

	const ProcessorInfoObject* m_source;

	JUCE_LEAK_DETECTOR(ConfigurationObject);
};

#endif
