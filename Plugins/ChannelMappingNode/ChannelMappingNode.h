/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __CHANNELMAPPINGNODE_H_330E50E0__
#define __CHANNELMAPPINGNODE_H_330E50E0__


#include <ProcessorHeaders.h>

/** Holds channel map settings for one data stream*/
class ChannelMapSettings
{
public:
    /** Constructor -- sets default values*/
    ChannelMapSettings();

    /** Destructor */
    ~ChannelMapSettings() {}

    /** Updates the number of channels*/
    void updateNumChannels(int);

    /** Channel order*/
    Array<int> channelOrder;

    /** Enabled channels*/
    Array<bool> isEnabled;

    /** -1 if no reference is used; otherwise holds the index of the reference channel*/
    Array<bool> referenceIndex;

    /** Holds up to 4 reference channels*/
    Array<int> referenceChannels;

    /** Writes settings to XML*/
    void toXml(XmlElement* xml);

    /** Reads settings from XML*/
    void fromXml(XmlElement* xml);

    /** Writes settings to JSON (.prb format)*/
    void toJson(File file);

    /** Reads settings from JSON (.prb format)*/
    void fromJson(File file);

};

/**
    Channel mapping node.

    Allows the user to select a subset of channels, remap their order, and reference them against
    any other channel.

    @see GenericProcessor
*/
class ChannelMappingNode : public GenericProcessor
{
public:

    /** Constructor*/
    ChannelMappingNode();

    /** Destructor*/
    ~ChannelMappingNode() { }

    /** Creates the plugin's editor*/
    AudioProcessorEditor* createEditor() override;

    /** Performs channel remapping and referencing*/
    void process (AudioBuffer<float>& buffer) override;

    /** Used to update parameters during acquisition*/
    void setParameter (int parameterIndex, float newValue) override;

    /** Informs downstream plugins of channel remapping*/
    void updateSettings() override;

    /** Changes the enabled state of a channel*/
    void setChannelEnabled(uint16 streamId, int channelNum, int isEnabled);

    /** Sets the channel order*/
    void setChannelOrder(uint16 streamId, Array<int> order);

    /** Gets the channel order */
    Array<int> getChannelOrder(uint16 streamId);

    /** Gets the channel enabled state */
    Array<bool> getChannelEnabledState(uint16 streamId);

    /** Returns reference channel*/
    int getReferenceChannel(uint16 streamId, int referenceIndex);

    /** Returns channels that use a reference*/
    Array<int> getChannelsForReference(uint16 streamId, int referenceIndex);

    /** Updates the reference channel for a reference group*/
    void setReferenceChannel(uint16 streamId, int referenceNum, int localChannel);

    /** Updates the reference index for a particular channel*/
    void setReferenceIndex(uint16 streamId, int channelNum, int referenceIndex);

    /** Updates the reference channel for a reference group*/
    void saveCustomParametersToXml(XmlElement* xml) override;

    /** Updates the reference index for a particular channel*/
    void loadCustomParametersFromXml(XmlElement* xml) override;
    
    /** Saves current settings to a Prb file*/
    String loadStreamSettings(uint16 streamId, File& file);
    
    /** Reads current settings from a Prb file*/
    String writeStreamSettings(uint16 streamId, File& file);

private:

    /** Holds settings for individual streams*/
    StreamSettings<ChannelMapSettings> settings;

    uint16 currentStream;

    int currentChannel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelMappingNode);
};


#endif  // __CHANNELMAPPINGNODE_H_330E50E0__
