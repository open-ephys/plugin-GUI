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

#include <stdio.h>

#include "CommonAverageRef.h"
#include "CommonAverageRefEditor.h"


CARSettings::CARSettings()
{
    m_avgBuffer = AudioBuffer<float>(1, 10000); // 1-dimensional buffer to hold the avg
    m_gainLevel.setValue(1.0f);
}

CommonAverageRef::CommonAverageRef()
    : GenericProcessor ("Common Avg Ref") 
{
    setProcessorType (PROCESSOR_TYPE_FILTER);
}


CommonAverageRef::~CommonAverageRef()
{
}


AudioProcessorEditor* CommonAverageRef::createEditor()
{
    editor = std::make_unique<CommonAverageRefEditor> (this, true);
    return editor.get();
}


void CommonAverageRef::updateSettings()
{
    settings.update(getDataStreams());
    
}

float CommonAverageRef::getGainLevel(uint16 streamId)
{
    settings[streamId]->m_gainLevel.updateTarget();

    return settings[streamId]->m_gainLevel.getNextValue();
}


void CommonAverageRef::setGainLevel (uint16 streamId, float newGain)
{
    settings[streamId]->m_gainLevel.setValue (newGain);
}


void CommonAverageRef::process (AudioSampleBuffer& buffer)
{

    for (auto stream : getDataStreams())
    {
        CARSettings* settings_ = settings[stream->getStreamId()];

        const int numSamples = getNumSourceSamples(stream->getStreamId());
        const int numReferenceChannels = settings_->m_referenceChannels.size();
        const int numAffectedChannels = settings_->m_affectedChannels.size();

        // There is no need to do any processing if either number of reference or affected channels is zero.
        if (!numReferenceChannels
            || !numAffectedChannels)
        {
            return;
        }

        settings_->m_avgBuffer.clear();

        for (int i = 0; i < numReferenceChannels; ++i)
        {
            settings_->m_avgBuffer.addFrom(0,       // destChannel
                0,                                  // destStartSample
                buffer,                             // source
                settings_->m_referenceChannels[i],  // sourceChannel
                0,                                  // sourceStartSample
                numSamples,                         // numSamples
                1.0f);                              // gain to apply
        }

        settings_->m_avgBuffer.applyGain(1.0f / float(numReferenceChannels));

        settings_->m_gainLevel.updateTarget();
        const float gain = -1.0f * settings_->m_gainLevel.getNextValue() / 100.f;

        for (int i = 0; i < numAffectedChannels; ++i)
        {
            buffer.addFrom(settings_->m_affectedChannels[i],  // destChannel
                0,                                            // destStartSample
                settings_->m_avgBuffer,                       // source
                0,                                            // sourceChannel
                0,                                            // sourceStartSample
                numSamples,                                   // numSamples
                gain);                                        // gain to apply
        }
    }

   
}


void CommonAverageRef::setReferenceChannels (uint16 streamId, const Array<int>& newReferenceChannels)
{
    const ScopedLock myScopedLock (objectLock);

    settings[streamId]->m_referenceChannels = Array<int> (newReferenceChannels);
}


void CommonAverageRef::setAffectedChannels (uint16 streamId, const Array<int>& newAffectedChannels)
{
    const ScopedLock myScopedLock (objectLock);

    settings[streamId]->m_affectedChannels = Array<int> (newAffectedChannels);
}


void CommonAverageRef::setReferenceChannelState (uint16 streamId, int channel, bool newState)
{

    const ScopedLock myScopedLock(objectLock);

    if (! newState)
        settings[streamId]->m_referenceChannels.removeFirstMatchingValue (channel);
    else
        settings[streamId]->m_referenceChannels.addIfNotAlreadyThere (channel);
}


void CommonAverageRef::setAffectedChannelState (uint16 streamId, int channel, bool newState)
{

    const ScopedLock myScopedLock(objectLock);

    if (! newState)
        settings[streamId]->m_affectedChannels.removeFirstMatchingValue (channel);
    else
        settings[streamId]->m_affectedChannels.addIfNotAlreadyThere (channel);
}


/*void CommonAverageRef::saveCustomChannelParametersToXml(XmlElement* channelElement,
    int channelNumber, InfoObject::Type channelType)
{
    if (channelType == InfoObject::Type::CONTINUOUS_CHANNEL)
    {
        XmlElement* groupState = channelElement->createNewChildElement("GROUPSTATE");
        
        const Array<int>& referenceChannels = getReferenceChannels();
        bool isReferenceChannel = referenceChannels.contains(channelNumber);
        groupState->setAttribute("reference", isReferenceChannel);

        const Array<int>& affectedChannels = getAffectedChannels();
        bool isAffectedChannel = affectedChannels.contains(channelNumber);
        groupState->setAttribute("affected", isAffectedChannel);
    }
}

void CommonAverageRef::loadCustomChannelParametersFromXml(XmlElement* channelElement,
    InfoObject::Type channelType)
{
    if (channelType == InfoObject::Type::CONTINUOUS_CHANNEL)
    {
        int channelNumber = channelElement->getIntAttribute("number");

        forEachXmlChildElementWithTagName(*channelElement, groupState, "GROUPSTATE")
        {
            if (groupState->hasAttribute("reference"))
            {
                bool isReferenceChannel = groupState->getBoolAttribute("reference");
                setReferenceChannelState(channelNumber, isReferenceChannel);
            }

            if (groupState->hasAttribute("affected"))
            {
                bool isAffectedChannel = groupState->getBoolAttribute("affected");
                setAffectedChannelState(channelNumber, isAffectedChannel);
            }
        }
    }
}*/