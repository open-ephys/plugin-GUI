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

#ifndef SPIKECHANNEL_H_INCLUDED
#define SPIKECHANNEL_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "Metadata.h"
#include "InfoObject.h"

class ContinuousChannel;

class PLUGIN_API SpikeChannel : public InfoObject, public MetadataEventObject
{
public:
	enum Type
	{
		SINGLE,
		STEREOTRODE,
		TETRODE,
		INVALID = 100
	};

	struct Settings {

		Type type;
		String name;
		String description;

		int prePeakSamples;
		int postPeakSamples;

		const Array<const ContinuousChannel*>& sourceChannels;
	};

	/** Default constructor 
		@param settings - the settings for this channel
	*/
	SpikeChannel(Settings settings);

	/* Destructor*/
	virtual ~SpikeChannel();

	/* Get the channel type (SINGLE, STEREOTRODE, TETRODE) */
	Type getChannelType() const;

	/** Returns an array with info about the channels from which the spikes originate */
	const Array<const ContinuousChannel*> getSourceChannels() const;

	/** Sets the number of samples, pre and post peak */
	void setNumSamples(unsigned int preSamples, unsigned int postSamples);

	/** Gets the number of pre peak samples */
	unsigned int getPrePeakSamples() const;

	/** Gets the number of post peak samples */
	unsigned int getPostPeakSamples() const;

	/** Gets the total number of samples */
	unsigned int getTotalSamples() const;

	/** Gets the number of channels associated with the electrode type */
	unsigned int getNumChannels() const;

	/** Gets the bitVolt value of one of the source channels*/
	float getChannelBitVolts(int chan) const;

	/** Gets the total size in bytes for a spike object */
	size_t getDataSize() const;

	/** Gets the size in bytes of one channel of the spike object*/
	size_t getChannelDataSize() const;

	/** Gets the number of channels associated with a specific electrode type */
	static unsigned int getNumChannels(Type type);

	/** Gets the electrode type from a specific number of channels*/
	static Type typeFromNumChannels(unsigned int nChannels);

private:

	const Type m_type;
	Array<const ContinuousChannel*> m_channelInfo;
	unsigned int m_numPreSamples{ 8 };
	unsigned int m_numPostSamples{ 32 };

};


#endif