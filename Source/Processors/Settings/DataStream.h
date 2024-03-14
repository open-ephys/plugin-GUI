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

/**
* Holds information about a DataStream, a synchronously 
* sampled block of channels.
* 
* A DataStream must have:
*  - At least one continuous channel
*  - The same sample rate for all continuous channels
*  - A guarantee that all channels will have the same number of
*    samples on each processing block
* 
* Every Source processor must have at least one DataStream,
* and can optionally have multiple DataStreams, if it receives
* data from multiple asynchronous devices.
* 
* Filter processors can add event channels and spike channels
* to a DataStream, to indicate where their timestamps originate.
* They can also add, rearrange, or remove continuous channels.
* 
* Every ContinuousChannel, EventChannel, and SpikeChannel object
* must be associated with one and only one DataStream.
* 
* Each stream has a unique 16-bit ID that is used to find
* its associated timestamps and sample counts. DataStream IDs
* are not meant to be seen by the user, and are re-generated
* each time the GUI is launched.
* 
* A stream will be identifiable to the user based on its name
* and source node ID (100, 101, etc.). If a processor has multiple
* streams, each stream should be given a unique name.
* 
*/
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
	};

	/** Constructor */
	DataStream(Settings settings);

	/** Destructor */
	virtual ~DataStream();

	/** Sets the channel counts to 0.
	*   Used by processors that need to re-arrange or remove
	*   channels within a DataStream.
	*/
	void clearChannels();

	/** Same as above, but for continuous channels only
	*/
	void clearContinuousChannels();

	/** Adds a spike, continuous, or event channel to the DataStream. 
	* Sets the channel's localIndex.
	*/
	void addChannel(InfoObject* channel);

	/** Returns the sample rate for this stream. */
	float getSampleRate() const;

	/** Returns the number of continuous channels in this stream. */
	int getChannelCount() const;

	/** Returns the unique ID for this stream*/
	uint16 getStreamId() const;

	/** Returns true if this DataStream has a device associated with it.*/
	bool hasDevice() const;

	/** Gets all of the continuous channels for this stream.*/
	Array<ContinuousChannel*> getContinuousChannels() const;

	/** Gets all of the event channels for this stream.*/
	Array<EventChannel*> getEventChannels() const;

	/** Gets all of the spike channels for this stream.*/
	Array<SpikeChannel*> getSpikeChannels() const;

	DeviceInfo* device;

private:

	Array<ContinuousChannel*> continuousChannels;
	Array<EventChannel*> eventChannels;
	Array<SpikeChannel*> spikeChannels;

	float m_sample_rate;

	uint16 streamId;
};

/** Template class that simplifies the process of
    keeping track of settings for individual streams.

	Plugins should add: 
	
		StreamSettings<CustomSettingsClass> settings

	as a member, then call:

		settings.update(getDataStreams());

	in the updateSettings() method. This will automatically
	remove unused settings objects, and add new ones
	where necessary.

	Then, settings for a particular stream can be 
	accessed via:

		settings[streamId]
*/

template <class T>
class StreamSettings
{

public:

	StreamSettings<T>() { }

	~StreamSettings<T>() { }

	void update(Array<const DataStream*> streams)
	{

		Array<uint16> currentStreamIds;

		for (auto stream : streams)
		{
			currentStreamIds.add(stream->getStreamId());

			if (!settingsMap.contains(stream->getStreamId()))
			{
				settingsArray.add(new T());
				settingsMap.set(stream->getStreamId(), settingsArray.getLast());
			}
		}
	}
	
	T* operator [](uint16 streamId)
	{
		if (settingsMap.contains(streamId))
			return settingsMap[streamId];

		return nullptr;
	}

private:
	HashMap<uint16, T*> settingsMap;
	OwnedArray<T> settingsArray;
};




#endif
