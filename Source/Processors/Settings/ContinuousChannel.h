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

#ifndef CONTINUOUSCHANNEL_H_INCLUDED
#define CONTINUOUSCHANNEL_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "InfoObject.h"

class DataStream;

class PLUGIN_API ContinuousChannel
	: public ChannelInfoObject
{
public:

	enum Type
	{
		ELECTRODE = 0,
		AUX = 1,
		ADC = 2,
		INVALID = 100
	};

	struct InputRange
	{
		float min = -5000.0f;
		float max = +5000.0f;
	};

	struct Impedance
	{
		float frequency = -1.0f;
		float value = -1.0f;
		bool measured = false;
	};

	struct Settings
	{
		Type type; 

		String name;
		String description;
		String identifier;
		
		float bitVolts;

		DataStream* stream;
	};

	InputRange inputRange;
	Impedance impedance;

	//--------- CONSTRUCTOR / DESTRUCTOR --------//

	/** Default constructor for creating continuous channels from scratch.
		@param settings - the settings for this channel
	*/
	ContinuousChannel(Settings settings);

	virtual ~ContinuousChannel();

	//--------- DATA GET / SET METHODS --------//

	/** Sets the bitVolts value for this channel. */
	void setBitVolts(float bitVolts);

	/** Returns the bitVolts value for this channel. */
	float getBitVolts() const;

	/** Sets the unit string */
	void setUnits(String unit);
	
	/** Gets the data units*/
	String getUnits() const;

	const Type getChannelType() const;

private:

	const Type m_type;
	
	float m_bitVolts;
	
	String m_units;

	int local_index;

};


#endif