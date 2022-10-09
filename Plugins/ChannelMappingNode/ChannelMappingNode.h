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

    /** Writes settings to XML*/
    void toXml(XmlElement* xml);

    /** Reads settings from XML*/
    void fromXml(XmlElement* xml);

    /** Writes settings to JSON (.prb format)*/
    void toJson(File file);

    /** Reads settings from JSON (.prb format)*/
    void fromJson(File file);

    /** Sets the stream info */
    void setStream(const DataStream* stream);

    /** Reset to defaults */
    void reset();

    int sourceNodeId = -1;
    String streamName = "";
    int numChannels = 0;
    float sampleRate = 0.0f;
    uint16 streamId = 0;

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

    /** Channel remap happens automatically via channel connections; does nothing*/
    void process (AudioBuffer<float>& buffer) override;

    /** Informs downstream plugins of channel remapping*/
    void updateSettings() override;

    /** Changes the enabled state of a channel*/
    void setChannelEnabled(uint16 streamId, int channelNum, int isEnabled);

    /** Sets the channel order*/
    void setChannelOrder(uint16 streamId, Array<int> order);

    /** Resets to default settings*/
    void resetStream(uint16 streamId);

    /** Gets the channel order */
    Array<int> getChannelOrder(uint16 streamId);

    /** Gets the channel enabled state */
    Array<bool> getChannelEnabledState(uint16 streamId);

    /** Updates the reference channel for a reference group*/
    void saveCustomParametersToXml(XmlElement* xml) override;

    /** Updates the reference index for a particular channel*/
    void loadCustomParametersFromXml(XmlElement* xml) override;
    
    /** Saves current settings to a Prb file*/
    String loadStreamSettings(uint16 streamId, File& file);
    
    /** Reads current settings from a Prb file*/
    String writeStreamSettings(uint16 streamId, File& file);

private:

    /** Find previously saved settings from similar streams */
    ChannelMapSettings* findMatchingStreamSettings(ChannelMapSettings* s);

    /** Holds settings for individual streams*/
    StreamSettings<ChannelMapSettings> settings;

    /** Previous stream IDs*/
    Array<uint16> previousStreamIds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelMappingNode);
};


#endif  // __CHANNELMAPPINGNODE_H_330E50E0__
