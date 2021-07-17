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

#ifndef INFOOBJECTS_H_INCLUDED
#define INFOOBJECTS_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "InfoObject.h"


class PLUGIN_API ProcessorInfoObject : public InfoObject
{
public:

	ProcessorInfoObject(GenericProcessor* processor);

    virtual ~ProcessorInfoObject();

	/** Gets the ID of the processor */
	int getNodeId() const;

	/** Gets the processor type of the node which created this object */
	String getType() const;

	/** Gets the name of the processor which created this object */
	String getName() const;
 
private:

	ProcessorInfoObject() = delete;

	int m_nodeId;
	String m_type;
	String m_name;
};


#endif