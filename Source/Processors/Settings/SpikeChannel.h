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

class PLUGIN_API Thresholder
{
public:
    Thresholder() { }
    virtual ~Thresholder() { }
    
    virtual void setThreshold(int channel, float threshold) = 0;
    
    virtual float getThreshold(int channel) = 0;
    
    virtual Array<float>& getThresholds() = 0;
    
    virtual bool checkSample(int channel, float sample) = 0;
};

class PLUGIN_API SpikeChannel : 
	public ChannelInfoObject, public MetadataEventObject
{
public:
	enum Type
	{
		SINGLE = 1,
		STEREOTRODE,
		TETRODE,
		INVALID = 100
	};

	struct Settings {

		Type type;

		String name;
		String description;
		String identifier;

		DataStream* stream;

		Array<const ContinuousChannel*>& sourceChannels;

		unsigned int numPrePeakSamples = 8;
		unsigned int numPostPeakSamples = 32;
        
        bool sendFullWaveform = true;

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
	const Array<const ContinuousChannel*>& getSourceChannels() const;
    
    /** Returns an array with info about the channels from which the spikes originate */
    void setSourceChannels(Array<const ContinuousChannel*>& sourceChannels);

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
    
    /** Determines whether a particular continuous channel is used to detect spikes*/
    bool detectSpikesOnChannel(int chan) const;

	// ====== STATIC METHODS ========= //

	/** Gets the number of channels associated with a specific electrode type */
	static unsigned int getNumChannels(Type type);

	/** Gets the electrode type from a specific number of channels*/
	static Type typeFromNumChannels(unsigned int nChannels);

	/** Generates a default channel name to use*/
	static String getDefaultChannelPrefix(Type channelType);

	/** Generates a default channel name to use*/
	static String getDescriptionFromType(Type channelType);

	/** Generates a default channel name to use*/
	static String getIdentifierFromType(Type channelType);
    
    /** Holds the current sample index for this electrode*/
    int currentSampleIndex;
    
    /** Holds the current sample index for this electrode*/
    int lastBufferIndex;

    /** Determines whether this electrode should use the overflow buffer*/
    bool useOverflowBuffer;
    
    /** Used to check whether a spike should be triggered */
    Thresholder* thresholder;
    
    /** Resets state after acquisition*/
    void reset();
    
    /** Holds the global channel index for each continuous channel*/
    Array<int> globalChannelIndexes;

private:

	const Type type;

	Array<const ContinuousChannel*> sourceChannels;
	Array<bool> channelIsEnabled;

	unsigned int numPreSamples;
	unsigned int numPostSamples;
    
    bool sendFullWaveform;

};


#endif


/*
/** Updates the source continuous channels for this spike channel */
//void setSourceChannels(Array<const ContinuousChannel*>);

/* Get the channel threshold type (FIXED, STD, DYNAMIC, etc.) */
//ThresholdType getThresholdType() const;

/* Set the channel threshold type (SINGLE, STEREOTRODE, TETRODE) */
//void setThresholdType(ThresholdType);

/* Get the channel threshold value */
//float getThreshold(int channelIndex) const;

/* Set the channel threshold value */
//void setThreshold(int channelIndex, float threshold);

/** Returns true if this channel sends spike objects containing the full waveform */
//bool sendsFullWaveform() const;

/* Sets whether the channel sends spike objects containing the full waveform, or just the peak */
//void shouldSendFullWaveform(bool);

/** Returns true if a sub-channel is enabled for detecting spikes*/
//bool getSourceChannelState(int channelIndex) const;

/* Sets whether a sub-channel is enabled for detecting spikes */
//void setSourceChannelState(int channelIndex, bool state);

/** Sets the number of samples, pre and post peak */
//void setNumSamples(unsigned int preSamples, unsigned int postSamples); 
