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
#include "MetaData.h"

class GenericProcessor;

// ------- Ancilliary objects -------//

/**
Structure with the basic info that identifies a channel
*/
struct SourceChannelInfo
{
	uint16 processorID;
	uint16 subProcessorID;
	uint16 channelIDX;
};

class PLUGIN_API NodeInfoBase
{
	//This field should never be changed by anything except GenericProcessor base code
	friend class GenericProcessor;
public:
    virtual ~NodeInfoBase();
	/** Gets the ID of the processor which currently owns this copy of the info object */
	uint16 getCurrentNodeID() const;
	/** Gets the index of this channel in the processor which currently owns this copy of the info object */
	uint16 getCurrentNodeChannelIdx() const;
	/** Gets the type of the processor which currently owns this copy of the info object */
	String getCurrentNodeType() const;
	/** Gets the name of the processor which currently owns this copy of the info object */
	String getCurrentNodeName() const;
protected:
	NodeInfoBase() = delete;
	NodeInfoBase(uint16 id, uint16 idx, String type, String name);
private:
	uint16 m_nodeID{ 0 };
	uint16 m_nodeIdx{ 0 };
	String m_currentNodeType;
	String m_currentNodeName;
};

/** This class allows creating a string with an historic of all the data a node has gone through */
class PLUGIN_API HistoryObject
{
protected:
	HistoryObject();

public:
    virtual ~HistoryObject();
	/** Returns the historic string */
	String getHistoricString() const;
	/** Adds a new entry in the historic string*/
	void addToHistoricString(String entry);

private:
	String m_historicString;
};

class PLUGIN_API SourceProcessorInfo
{
protected:
	SourceProcessorInfo(const GenericProcessor* source, uint16 subproc = 0);

public:
    virtual ~SourceProcessorInfo();
	/** Gets the ID of the processor which created the channel object */
	uint16 getSourceNodeID() const;

	/** Gets the subprocessor index associated to this channel object*/
	uint16 getSubProcessorIdx() const;

	/** Gets the processor type of the node which created this object */
	String getSourceType() const;

	/** Gets the name of the processor which created this object */
	String getSourceName() const;

	/** Gets the number of subprocessors the source processor has.
	Useful to determine if a processor has multiple subprocessors and label things accordingly*/
	uint16 getSourceSubprocessorCount() const;
 
private:
	SourceProcessorInfo() = delete;
	const uint16 m_sourceNodeID;
	const uint16 m_sourceSubNodeIndex;
	const String m_sourceType;
	const String m_sourceName;
	const uint16 m_sourceSubProcessorCount;
};

class PLUGIN_API NamedInfoObject
{
public:
    virtual ~NamedInfoObject();
	/** Sets the object's name*/
	void setName(String name);

	/** Returns the name of a given object*/
	String getName() const;

	/** Sets the Channel Info Object description, to be stored in most file formats*/
	void setDescription(String description);

	/** Gets the Channel Info Object description */
	String getDescription() const;

	/** Sets a machine-readable data identifier (eg.: sourcedata.continuous ) */
	void setIdentifier(String identifier);

	String getIdentifier() const;
protected:
	NamedInfoObject();
	virtual void setDefaultNameAndDescription() = 0;
private:
	String m_name;
	String m_identifier;
	String m_description;

};

/** Common class for all info objects */
class PLUGIN_API InfoObjectCommon :
	public NodeInfoBase, public SourceProcessorInfo, public NamedInfoObject
{
protected:
	InfoObjectCommon(uint16 idx, uint16 typeidx, float sampleRate, const GenericProcessor* source, uint16 subproc = 0);

public:
    virtual ~InfoObjectCommon();
	enum InfoObjectType
	{
		DATA_CHANNEL,
		EVENT_CHANNEL,
		SPIKE_CHANNEL,
		INVALID = 100
	};

	/** Returns the sample rate value for this channel. */
	float getSampleRate() const;


	/** Gets the position of this channel in the source processor*/
	uint16 getSourceIndex() const;

	/** Gets the position in the source processor of this channel object, relative
	to its subtype (HEADSTAGE, AUX or ADC for data channels, TTL, MESSAGE or BINARY for events, etc...) */
	uint16 getSourceTypeIndex() const;

	virtual InfoObjectType getInfoObjectType() const = 0;

	bool isEqual(const InfoObjectCommon& other) const;
	bool isSimilar(const InfoObjectCommon& other) const;
	bool operator==(const InfoObjectCommon& other) const;

private:
	bool isEqual(const InfoObjectCommon& other, bool similar) const;
	virtual bool checkEqual(const InfoObjectCommon& other, bool similar) const = 0;
	/** Index of the object in the source processor */
	const uint16 m_sourceIndex;
	/** Index of this particular subtype in the source processor */
	const uint16 m_sourceTypeIndex;
	const float m_sampleRate;
};

// ------- Main objects -------//

class PLUGIN_API DataChannel
	: public InfoObjectCommon, public MetaDataInfoObject, public HistoryObject
{
public:

	enum DataChannelTypes
	{
		HEADSTAGE_CHANNEL = 0,
		AUX_CHANNEL = 1,
		ADC_CHANNEL = 2,
		INVALID = 100
	};
	//--------- CONSTRUCTOR / DESTRUCTOR --------//

	/** Default constructor for creating Channels from scratch.
		@param type The type of data this channel represents (HEADSTAGE, ADC, AUX)
		@param sampleRate the sample rate this channel is acquiring data
		@param source A pointer to the source processor
		@param subproc Optional. The source subprocessor index.
	*/
	DataChannel(DataChannelTypes type, float sampleRate, GenericProcessor* source, uint16 subproc = 0);

	/** Copy constructor. */
	DataChannel(const DataChannel& ch);

	virtual ~DataChannel();

	//--------- DATA GET / SET METHODS --------//

	/** Sets the bitVolts value for this channel. */
	void setBitVolts(float bitVolts);

	/** Returns the bitVolts value for this channel. */
	float getBitVolts() const;

	/** Sets the unit string */
	void setDataUnits(String unit);
	
	/** Gets the data units*/
	String getDataUnits() const;

	DataChannelTypes getChannelType() const;

	//--------- STATUS METHODS ----------//
	/** Toggled when a channel is disabled from further processing. */
	bool isEnabled() const;

	/** Toggled when a channel is disabled from further processing. */
	void setEnable(bool e);

	/** Informs whether the channel is being routed to the audio node */
	bool isMonitored() const;

	/** Sets if the channel needs to be routed to the audio node */
	void setMonitored(bool e);

	//Record methods will be phased out with the probe-based record system
	/** Sets whether or not the channel will record. */
	void setRecordState(bool t);

	/** Informs whether or not the channel will record. */
	bool getRecordState() const;

	//---------- OTHER METHODS ------------//
	/** Restores the default settings for a given channel. */
	void reset();

	InfoObjectType getInfoObjectType() const override;
	void setDefaultNameAndDescription() override;
private:
	bool checkEqual(const InfoObjectCommon& other, bool similar) const override;
	const DataChannelTypes m_type;
	float m_bitVolts{ 1.0f };
	bool m_isEnabled{ true };
	bool m_isMonitored{ false };
	bool m_isRecording{ false };
	String m_unitName{ "uV" };

	JUCE_LEAK_DETECTOR(DataChannel);
};

class PLUGIN_API EventChannel :
	public InfoObjectCommon, public MetaDataInfoObject, public MetaDataEventObject
{
public:
	enum EventChannelTypes
	{
		//Numeration kept to maintain compatibility with old code
		TTL = 3,
		TEXT = 5,
		//generic binary types. These will be treated by the majority of record engines as simple binary blobs,
		//while having strict typing helps creating stabler plugins
		INT8_ARRAY = 10,
		UINT8_ARRAY,
		INT16_ARRAY,
		UINT16_ARRAY,
		INT32_ARRAY,
		UINT32_ARRAY,
		INT64_ARRAY,
		UINT64_ARRAY,
		FLOAT_ARRAY,
		DOUBLE_ARRAY,
		//For error checking
		INVALID,
		//Alias for checking binary types
		BINARY_BASE_VALUE = 10
	};

	/** Default constructor
	@param type The type of event this channel represents (TTL, TEXT, BYINARY_MSG)
	@param numChannels The number of virtual channels
	@param dataLength The length of the event payload
	@param sampleRate the sample rate this channel timestamps are referred to
	@param source A pointer to the source processor
	@param subproc Optional. The source subprocessor index.

	The virtual channels mean:
	-For TTL signals, it must be the number of bits in the TTL word.
	-For other events, this might be used to differentiate between different origins within the same processor

	The event length mean:
	-For TTL signals, this method will do nothing, as the size is fixed by the number of ttl channels
	-For message events, the length of the string in characters
	-For typed array events, the number of elements

	*/
	EventChannel(EventChannelTypes type, unsigned int numChannels, unsigned int dataLength, float sampleRate, GenericProcessor* source, uint16 subproc = 0);

	virtual ~EventChannel();

	EventChannelTypes getChannelType() const;

	/** Gets the number of virtual channels
	*/
	unsigned int getNumChannels() const;

	/** Gets the size of the event payload
		-For TTL, it returns the number of bytes that form the full TTL word, same as getByteDataSize()
		-For message events, the number of characters
		-For typed array events, the number of elements.
	*/
	unsigned int getLength() const;

	/** Gets the size of the event payload in bytes*/
	size_t getDataSize() const;
	
	/** Sets if the event should be recorded or not.
		Note that this option does not prevent the event from actually being recorded, it simply states
		a processor's developer opinion on how the event should be treated, but can be overrided by the user.

		Meant for events that represent runtime changes in the state of a processor that can be useful to
		other processors but hold no actual meaning for the experiment.
	*/
	void setShouldBeRecorded(bool status);

	/** Gets the event preference about being recorded */
	bool getShouldBeRecorded() const;

	/** Gets the size in bytes of an element depending of the type*/
	static size_t getTypeByteSize(EventChannelTypes type);

	/** Handy method to get an equivalente metadata value type for the main event data*/
	static BaseType getEquivalentMetaDataType(const EventChannel& ev);

	/** Handy method to get an equivalente metadata value type for the main event data*/
	BaseType getEquivalentMetaDataType() const;

	InfoObjectType getInfoObjectType() const override;
	void setDefaultNameAndDescription() override;
private:
	bool checkEqual(const InfoObjectCommon& other, bool similar) const override;
	const EventChannelTypes m_type;
	unsigned int m_numChannels{ 1 };
	size_t m_dataSize{ 1 };
	unsigned int m_length{ 1 };
	bool m_shouldBeRecorded{ true };

	JUCE_LEAK_DETECTOR(EventChannel);
};

class PLUGIN_API SpikeChannel :
	public InfoObjectCommon, public MetaDataInfoObject, public MetaDataEventObject
{
public:
	enum ElectrodeTypes
	{
		SINGLE,
		STEREOTRODE,
		TETRODE,
		INVALID = 100
	};

	/** Default constructor 
		@param type The type of electrode this channel represents (SINGLE, STEREOTRODE, TETRODE)
		@param source A pointer to the source processor
		@param souceChannels An array containing const pointers to the channels that originate the data for this spike electrode
		@param subproc Optional. The source subprocessor index.
	*/
	SpikeChannel(ElectrodeTypes type, GenericProcessor* source, const Array<const DataChannel*>& sourceChannels, uint16 subproc = 0);

	virtual ~SpikeChannel();

	ElectrodeTypes getChannelType() const;

	/** Returns an array with info about the channels from which the spikes originate */
	Array<SourceChannelInfo> getSourceChannelInfo() const;

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
	static unsigned int getNumChannels(ElectrodeTypes type);

	/** Gets the electrode type from a specific number of channels*/
	static ElectrodeTypes typeFromNumChannels(unsigned int nChannels);

	InfoObjectType getInfoObjectType() const override;
	void setDefaultNameAndDescription() override;
private:
	bool checkEqual(const InfoObjectCommon& other, bool similar) const override;
	const ElectrodeTypes m_type;
	Array<SourceChannelInfo> m_sourceInfo;
	unsigned int m_numPreSamples{ 8 };
	unsigned int m_numPostSamples{ 32 };
	Array<float> m_channelBitVolts;

	JUCE_LEAK_DETECTOR(DataChannel);
};

/**
	Class for defining extra configuration objects to be shared with processors down the chain not associated with any particular channel or event.
	It does not hold any data by itself, but can be filled with metadata fields to form any structure
*/
class PLUGIN_API ConfigurationObject :
	public SourceProcessorInfo, public NamedInfoObject, public MetaDataInfoObject
{
public:
	/**Default constructor
	@param identifier the identifier field of NamedInfoObject, required for config objects
	@param source The source processor
	@param subproc Optional. The source subprocessor index
	*/
	ConfigurationObject(String identifier, GenericProcessor* source, uint16 subproc = 0);
    virtual ~ConfigurationObject();

	/** Sets if the configuration should be recorded or not.
	Similar to the events, this does not prevent the configuration data to be recorded, but rather states
	a preference by the source developer.
	*/
	void setShouldBeRecorded(bool status);

	/** Gets the config preference about being recorded */
	bool getShouldBeRecorded() const;

	void setDefaultNameAndDescription() override;
private:
	bool m_shouldBeRecorded{ true };
	JUCE_LEAK_DETECTOR(ConfigurationObject);
};



#endif