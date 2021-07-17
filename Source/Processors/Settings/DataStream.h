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

#ifndef STREAMINFO_H_INCLUDED
#define STREAMINFO_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"

#include "InfoObject.h"

class ProcessorInfoObject;
class DeviceInfo;
class ContinuousChannel;
class EventChannel;
class SpikeChannel;

class PLUGIN_API DataStream :
	public InfoObject
{
public:

	static uint16 nextId;

	struct Settings {

		String name;
		String description;
		String identifier;

		float sample_rate;

		ProcessorInfoObject* source;
	};

	DataStream(Settings settings);

	virtual ~DataStream();

	DeviceInfo* device;
	ProcessorInfoObject* source;
	uint16 streamId;

	void addChannel(InfoObject* channel);

	Array<ContinuousChannel*> getContinuousChannels();
	
	Array<EventChannel*> getEventChannels();

	Array<SpikeChannel*> getSpikeChannels();

	/** Returns the sample rate for this stream. */
	float getSampleRate() const;

	/** Returns the number of channels in this stream. */
	int getChannelCount() const;

private:

	Array<ContinuousChannel*> continuousChannels;
	Array<EventChannel*> eventChannels;
	Array<SpikeChannel*> spikeChannels;

	float m_sample_rate;

	int m_channel_count;

};

#endif