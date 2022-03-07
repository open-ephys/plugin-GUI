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

#ifndef EVENTS_H_INCLUDED
#define EVENTS_H_INCLUDED

#include <JuceHeader.h>

#include "Event.h"
#include "../Settings/SpikeChannel.h"

#define SPIKE_BASE_SIZE 18

class GenericProcessor;
class Spike;

typedef ScopedPointer<Spike> SpikePtr;

/**
 * Holds data for an individual spike event.
 * 
 * Each spike represents a snippet of continuous data, usually
 * captured in response to a threshold crossing.
 * 
 * Spikes can be captured simultaneously across 1, 2, or 4 channels.
 * 
 * Spike objects can be packaged into MidiBuffers and passed between
 * processors.
 * 
 * ======================
 * Spike Packet Structure
 * ======================
 * 
 * 1 Byte: Event type (Spike)
 * 1 Byte: Electrode type (SE, ST, or TT)
 * 2 Bytes: Source processor ID
 * 2 Bytes: Source stream ID
 * 2 Bytes: Source electrode index
 * 8 Bytes: Timestamp
 * 2 Bytes: Sorted ID (defaults to 0)
 * 4 x N Bytes: Thresholds
 * 4 x N x M Bytes: Data
 * 
 *  N = number of channels
 *  M = number of samples
 * 
 * The Spike class is part of the Open Ephys Plugin API
 *
 */
class PLUGIN_API Spike : public EventBase
{
public:
	/**
	* A simple helper class that holds a buffer for spike event creation.
	* 
	* It becomes invalid once it's fed into a spike event, so it needs to be re-initialized
	* in order to be reused.
	* 
	* Example:
	* Spike::Buffer spikeBuffer(spikeChannelA);
	* //  --> Individual data samples can then be added to the buffer object
	* 
	* Spike* spike = createSpike(spikeChannelA, ..., spikeBuffer);
	* //  --> the spikeBuffer object is no longer valid
	* 
	* // re-initialize the buffer:
	* spikeBuffer = SpikeBuffer(spikeChannelB);
	* 
	*/

	class PLUGIN_API Buffer
	{
		friend Spike;
	public:
		Buffer(const SpikeChannel* channelInfo);
		void set(const int chan, const int samp, const float value);
		void set(const int index, const float value);
		void set(const int chan, const float* source, const int n);
		void set(const int chan, const int start, const float* source, const int n);
		float get(const int chan, const int samp);
		float get(const int index);

		//Caution advised with this method, as the pointer can become inaccessible
		const float* getRawPointer();
        
        const SpikeChannel* spikeChannel;
	private:
		Buffer() = delete;
		HeapBlock<float> m_data;
		const int m_nChans;
		const int m_nSamps;
		bool m_ready{ true };
	};

	/* Copy constructor*/
	Spike(const Spike& other);

	/* Destructor*/
	~Spike();

	/* Prevent move assignment*/
	Spike& operator=(const Spike&) = delete;

	/* Serialize the Spike object to a destination buffer*/
	void serialize(void* destinationBuffer, size_t bufferSize) const override;

	/* Get the SpikeChannel info object associated with this event*/
	const SpikeChannel* getChannelInfo() const;

	/* Get a pointer to the raw data for this spike*/
	const float* getDataPointer() const;

	/* Get a pointer to the raw data for a particular channel*/
	const float* getDataPointer(int channel) const;

	/* Get the threshold used to trigger spike capture on a particular channel*/
	float getThreshold(int chan) const;

	/* Get the sorted ID for this spike*/
	uint16 getSortedId() const;

	/* Create a Spike object*/
	static SpikePtr createSpike(const SpikeChannel* channelInfo, 
		juce::int64 timestamp, 
		Array<float> thresholds, 
		Spike::Buffer& buffer, 
		uint16 sortedID = 0);

	/* Create a Spike object with metadata*/
	static SpikePtr createSpike(const SpikeChannel* channelInfo, 
		juce::int64 timestamp, 
		Array<float> thresholds, 
		Spike::Buffer& buffer,
		const MetadataValueArray& metaData,
		uint16 sortedID = 0);

	/** Allows downstream processor to update the sorted ID 
	   WARNING -- since the original byte buffer has to exist,
	   this should only be done inside the handleSpike() method!!! */
	void setSortedId(uint16 sortedId);

	/* Deserialize a Spike object from an event packet*/
	static SpikePtr deserialize(const EventPacket& packet, const SpikeChannel* channelInfo);

	/* Deserialize a Spike object from a raw byte buffer*/
	static SpikePtr deserialize(const uint8* buffer, const SpikeChannel* channelInfo);
    
    /* The SpikeChannel object associated with this spike */
    const SpikeChannel* spikeChannel;
private:

	/* Prevent initialization of an empty Spike object*/
	Spike() = delete;
	
	/* Constructor*/
	Spike(const SpikeChannel* channelInfo, 
		juce::int64 timestamp, 
		Array<float> thresholds, 
		HeapBlock<float>& data, 
		uint16 sortedID = 0);

	/* Create a basic Spike object*/
	static Spike* createBasicSpike(const SpikeChannel* channelInfo, 
		juce::int64 timestamp, 
		Array<float> threshold, 
		Spike::Buffer& buffer, 
		uint16 sortedID = 0);

	const Array<float> m_thresholds;

	const uint8* buffer;
	
	const uint16 m_sortedID;
	HeapBlock<float> m_data;
	JUCE_LEAK_DETECTOR(Spike);
};

#endif
