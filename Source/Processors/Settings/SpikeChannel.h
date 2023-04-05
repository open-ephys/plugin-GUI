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
        
		Array<int> localChannelIndexes;

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
    
    /** Copy constructor*/
    SpikeChannel(const SpikeChannel& spikeChannel);
    
    /** Sets the DataStream for this spike channel, which sets the global channel indexes (if available)*/
    void setDataStream(DataStream* dataStream, bool addToStream = true) override;

	/* Get the channel type (SINGLE, STEREOTRODE, TETRODE) */
	Type getChannelType() const;
    
    /** Returns true if all continuous channels are available in the incoming stream */
    bool isValid() const;

	/** Returns an array with info about the channels from which the spikes originate */
	const Array<const ContinuousChannel*>& getSourceChannels() const;
    
	/** Sets the source channels from which the spikes originate */
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
    
    /** Holds the current sample index for this electrode*/
    int currentSampleIndex;
    
    /** Holds the current sample index for this electrode*/
    int lastBufferIndex;

    /** Determines whether this electrode should use the overflow buffer*/
    bool useOverflowBuffer;
    
    /** Used to check whether a spike should be triggered */
    std::unique_ptr<Thresholder> thresholder;
    
    /** Resets state after acquisition*/
    void reset();
    
    /** Holds the global channel index for each continuous channel*/
    Array<int> globalChannelIndexes;
    
    /** Holds the local channel index for each continuous channel*/
    Array<int> localChannelIndexes;
    
    /** Find similar stream*/
    DataStream* findSimilarStream(OwnedArray<DataStream>& streams);
    
    /** Determines whether channel sends the full waveform, or just the peak sample*/
    bool sendFullWaveform;
    

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

protected:

	const Type type;

	Array<const ContinuousChannel*> sourceChannels;
	Array<bool> channelIsEnabled;

	unsigned int numPreSamples;
	unsigned int numPostSamples;
    

    uint16 lastStreamId;
    String lastStreamName;
    float lastStreamSampleRate;
    int lastStreamChannelCount;

};

#endif
