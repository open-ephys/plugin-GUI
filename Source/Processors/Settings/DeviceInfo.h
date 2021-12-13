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

#ifndef DEVICEINFO_H_INCLUDED
#define DEVICEINFO_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "InfoObject.h"

/** 
	Allows device info to propagate through the signal chain

	For example, this could allow downstream plugins to recognize
	the particular kind of Neuropixels probe used for acquisition.

*/
class PLUGIN_API DeviceInfo :
	public InfoObject
{
public:

	friend class GenericProcessor;

	struct Settings {
		String name = "name";
		String description = "description";
		String identifier = "default-device";

		String serial_number = "0000000";
		String manufacturer = "Open Ephys";
	};

	/** Constructor */
	DeviceInfo(Settings settings);

	/** Destructor */
	virtual ~DeviceInfo();

	const String manufacturer;
	const String serial_number;
	
};

#endif