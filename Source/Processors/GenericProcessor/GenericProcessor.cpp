/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "GenericProcessor.h"

#include "../../AccessClass.h"
#include "../../Processors/ProcessorGraph/ProcessorGraph.h"
#include "../../Utils/Utils.h"
#include "../Editors/GenericEditor.h"

#include "../Settings/ConfigurationObject.h"
#include "../Settings/DataStream.h"
#include "../Settings/DeviceInfo.h"
#include "../Settings/ProcessorInfo.h"

#include "../Splitter/Splitter.h"

#include "../Merger/Merger.h"

#include "../Events/Event.h"
#include "../Events/Spike.h"

#include "../MessageCenter/MessageCenter.h"

#include <exception>

#define MS_FROM_START Time::highResolutionTicksToSeconds (Time::getHighResolutionTicks() - start) * 1000

const String GenericProcessor::m_unusedNameString ("xxx-UNUSED-OPEN-EPHYS-xxx");

std::map<int, std::vector<ProcessorAction*>> GenericProcessor::undoableActions;

GenericProcessor::GenericProcessor (const String& name, bool headlessMode_)
    : GenericProcessorBase (name), ParameterOwner (ParameterOwner::Type::OTHER), headlessMode (headlessMode_), sourceNode (nullptr), destNode (nullptr), isEnabled (true), wasConnected (false), nextAvailableChannel (0), saveOrder (-1), loadOrder (-1), currentChannel (-1), editor (nullptr), parametersAsXml (nullptr), ttlEventChannel (nullptr), sendSampleCount (true), m_name (name), m_paramsWereLoaded (false)

{
    latencyMeter = std::make_unique<LatencyMeter> (this);

    addBooleanParameter (Parameter::STREAM_SCOPE,
                         "enable_stream",
                         "Enable",
                         "Determines whether or not processing is enabled for a particular stream",
                         true,
                         true);
}

GenericProcessor::~GenericProcessor()
{
    editor.reset(); // remove parameter editors before parameters

    dataStreamParameters.clear (true);
    parametersAsXml = nullptr;
}

AudioProcessorEditor* GenericProcessor::createEditor()
{
    editor = std::make_unique<GenericEditor> (this);

    return editor.get();
}

void GenericProcessor::setNodeId (int id)
{
    nodeId = id;

    if (editor != nullptr)
    {
        editor->updateName();
    }
}

const String GenericProcessor::getName() const
{
    return m_name;
}

Parameter* GenericProcessor::getStreamParameter (const String& name)
{
    for (auto param : dataStreamParameters)
    {
        if (param->getName().equalsIgnoreCase (name))
            return param;
    }

    LOGE ("Could not find parameter named ", name);

    return nullptr;
}

Array<Parameter*> GenericProcessor::getParameters (uint16 streamId)
{
    Array<Parameter*> params;

    if (getDataStream (streamId) != nullptr)
    {
        params = getDataStream (streamId)->getParameters();
    }

    return params;
}

Array<Parameter*> GenericProcessor::getParameters (EventChannel* eventChannel)
{
    Array<Parameter*> params;

    if (eventChannel != nullptr)
        params = eventChannel->getParameters();

    return params;
}

Array<Parameter*> GenericProcessor::getParameters (ContinuousChannel* continuousChannel)
{
    Array<Parameter*> params;

    if (continuousChannel != nullptr)
        params = continuousChannel->getParameters();

    return params;
}

Array<Parameter*> GenericProcessor::getParameters (SpikeChannel* spikeChannel)
{
    Array<Parameter*> params;

    if (spikeChannel != nullptr)
        params = spikeChannel->getParameters();

    return params;
}

void GenericProcessor::addBooleanParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool defaultValue,
    bool deactivateDuringAcquisition)
{
    BooleanParameter* p = new BooleanParameter (
        scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
        scope,
        name,
        displayName,
        description,
        defaultValue,
        deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addCategoricalParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> categories,
    int defaultIndex,
    bool deactivateDuringAcquisition)
{
    CategoricalParameter* p = new CategoricalParameter (
        scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
        scope,
        name,
        displayName,
        description,
        categories,
        defaultIndex,
        deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addIntParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int defaultValue,
    int minValue,
    int maxValue,
    bool deactivateDuringAcquisition)
{
    IntParameter* p =
        new IntParameter (
            scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
            scope,
            name,
            displayName,
            description,
            defaultValue,
            minValue,
            maxValue,
            deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addStringParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    String defaultValue,
    bool deactivateDuringAcquisition)
{
    StringParameter* p =
        new StringParameter (
            scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
            scope,
            name,
            displayName,
            description,
            defaultValue,
            deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addFloatParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& unit,
    float defaultValue,
    float minValue,
    float maxValue,
    float stepSize,
    bool deactivateDuringAcquisition)
{
    FloatParameter* p =
        new FloatParameter (
            scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
            scope,
            name,
            displayName,
            description,
            unit,
            defaultValue,
            minValue,
            maxValue,
            stepSize,
            deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addMaskChannelsParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
{
    Array<var> defaultValue;

    MaskChannelsParameter* p =
        new MaskChannelsParameter (
            scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
            scope,
            name,
            displayName,
            description,
            deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addSelectedChannelsParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int maxSelectedChannels,
    bool deactivateDuringAcquisition)
{
    Array<var> defaultValue;

    SelectedChannelsParameter* p =
        new SelectedChannelsParameter (
            scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
            scope,
            name,
            displayName,
            description,
            defaultValue,
            maxSelectedChannels,
            deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addPathParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    const StringArray& validFileExtensions,
    bool isDirectory,
    bool deactivateDuringAcquisition)
{
    PathParameter* p =
        new PathParameter (this,
                           scope,
                           name,
                           displayName,
                           description,
                           defaultValue,
                           validFileExtensions,
                           isDirectory,
                           deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addSelectedStreamParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    Array<String> streamNames,
    const int defaultIndex,
    bool deactivateDuringAcquisition)
{
    SelectedStreamParameter* p =
        new SelectedStreamParameter (this,
                                     scope,
                                     name,
                                     displayName,
                                     description,
                                     streamNames,
                                     defaultIndex,
                                     deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addTimeParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    const String& defaultValue,
    bool deactivateDuringAcquisition)
{
    TimeParameter* p =
        new TimeParameter (this,
                           scope,
                           name,
                           displayName,
                           description,
                           defaultValue,
                           deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addNotificationParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    bool deactivateDuringAcquisition)
{
    NotificationParameter* p = new NotificationParameter (
        scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
        scope,
        name,
        displayName,
        description,
        deactivateDuringAcquisition);

    if (scope == Parameter::PROCESSOR_SCOPE)
        addParameter (p);
    else if (scope == Parameter::STREAM_SCOPE)
        dataStreamParameters.add (p);
}

void GenericProcessor::addTtlLineParameter (
    Parameter::ParameterScope scope,
    const String& name,
    const String& displayName,
    const String& description,
    int maxTtlLines,
    bool syncMode,
    bool canSelectNone,
    bool deactivateDuringAcquisition)
{
    TtlLineParameter* p = new TtlLineParameter (
        scope == Parameter::PROCESSOR_SCOPE ? this : nullptr,
        scope,
        name,
        displayName,
        description,
        maxTtlLines,
        syncMode,
        canSelectNone,
        deactivateDuringAcquisition);

    dataStreamParameters.add (p);
}

void GenericProcessor::parameterChangeRequest (Parameter* param)
{
    currentParameter = param;

    setParameter (-1, 0.0f);

    if (! headlessMode)
        getEditor()->updateView();
}

void GenericProcessor::setParameter (int parameterIndex, float newValue)
{
    if (currentParameter != nullptr)
    {
        currentParameter->updateValue();
        parameterValueChanged (currentParameter);
    }
}

int GenericProcessor::getNextChannel (bool increment)
{
    int chan = nextAvailableChannel;

    //LOGDD("Next channel: ", chan, ", num inputs: ", getNumInputs());

    if (increment)
        nextAvailableChannel++;

    if (chan < getNumInputs())
        return chan;
    else
        return -1;
}

void GenericProcessor::resetConnections()
{
    nextAvailableChannel = 0;

    wasConnected = false;
}

void GenericProcessor::setSourceNode (GenericProcessor* sn)
{
    if (this->isMerger())
    {
        setMergerSourceNode (sn);
    }
    else
    {
        sourceNode = sn;
    }
}

void GenericProcessor::setDestNode (GenericProcessor* dn)
{
    if (isSplitter())
    {
        setSplitterDestNode (dn);
    }
    else
    {
        destNode = dn;
    }
}

void GenericProcessor::clearSettings()
{
    LOGDD ("Clearing settings for ", getName());

    Array<ContinuousChannel*> continuousChannelsToKeep;

    for (auto obj : continuousChannels)
    {
        //std::cout << obj->getName() << std::endl;
        if (! obj->isLocal())
            delete obj;
        else
            continuousChannelsToKeep.add (obj);
    }

    continuousChannels.clearQuick (false);
    continuousChannels.addArray (continuousChannelsToKeep);

    Array<EventChannel*> eventChannelsToKeep;

    for (auto obj : eventChannels)
    {
        if (! obj->isLocal())
            delete obj;
        else
            eventChannelsToKeep.add (obj);
    }

    eventChannels.clearQuick (false);
    eventChannels.addArray (eventChannelsToKeep);

    Array<SpikeChannel*> spikeChannelsToKeep;

    for (auto obj : spikeChannels)
    {
        if (! obj->isLocal())
            delete obj;
        else
            spikeChannelsToKeep.add (obj);
    }

    spikeChannels.clearQuick (false);
    spikeChannels.addArray (spikeChannelsToKeep);

    Array<ConfigurationObject*> configurationObjectsToKeep;

    for (auto obj : configurationObjects)
    {
        if (! obj->isLocal())
            delete obj;
        else
            configurationObjectsToKeep.add (obj);
    }

    configurationObjects.clearQuick (false);
    configurationObjects.addArray (configurationObjectsToKeep);

    Array<DataStream*> dataStreamsToDelete;

    for (auto obj : dataStreams)
    {
        if (! obj->isLocal())
        {
            savedDataStreamParameters.add (new ParameterCollection());

            //std::cout << "SAVING STREAM PARAMETERS" << std::endl;
            savedDataStreamParameters.getLast()->copyParametersFrom (obj);

            dataStreamsToDelete.add (obj);
        }
    }

    for (auto obj : dataStreamsToDelete)
        dataStreams.removeObject (obj, true);

    ttlEventChannel = nullptr;

    startTimestampsForBlock.clear();
    startSamplesForBlock.clear();
    syncStreamIds.clear();
    numSamplesInBlock.clear();
    processStartTimes.clear();
}

void GenericProcessor::setStreamEnabled (uint16 streamId, bool isEnabled)
{
    getDataStream (streamId)->getParameter ("enable_stream")->setNextValue (isEnabled);
}

int GenericProcessor::findMatchingStreamParameters (DataStream* stream)
{
    // LOGDD("Finding best matching saved parameters for ", stream->getName(), " (", stream->getNodeId(), ")");

    for (int i = 0; i < savedDataStreamParameters.size(); i++)
    {
        ParameterCollection* params = savedDataStreamParameters[i];

        // matching ID --> perfect match
        if (params->owner.streamId == stream->getStreamId())
        {
            // std::cout << "Found matching ID: " << params->owner.streamId << std::endl;
            return i;
        }
    }

    for (int i = 0; i < savedDataStreamParameters.size(); i++)
    {
        ParameterCollection* params = savedDataStreamParameters[i];

        //std::cout << params->owner.name << std::endl;

        // matching name --> this is a good sign
        if (params->owner.name.equalsIgnoreCase (stream->getName()))
        {
            //std::cout << "Found matching name." << std::endl;

            bool betterMatch = false;

            for (auto otherStream : dataStreams)
            {
                if (otherStream != stream && params->owner.streamId == otherStream->getStreamId())
                {
                    betterMatch = true;
                    // std::cout << "...but found another stream with matching ID" << std::endl;
                }
            }

            if (! betterMatch)
            {
                //std::cout << "And it's the best match." << std::endl;
                return i;
            }
            else
                continue;
        }
    }

    for (int i = 0; i < savedDataStreamParameters.size(); i++)
    {
        ParameterCollection* params = savedDataStreamParameters[i];

        //std::cout << params->owner.name << std::endl;

        if (! stream->hasDevice())
            continue;

        // matching name --> this is a good sign
        if (params->owner.deviceName.equalsIgnoreCase (stream->device->getName()))
        {
            // std::cout << "Found matching device." << std::endl;

            bool betterMatch = false;

            for (auto otherStream : dataStreams)
            {
                if (otherStream != stream && params->owner.name.equalsIgnoreCase (otherStream->getName()))
                {
                    betterMatch = true;
                    // std::cout << "...but found another stream with matching name" << std::endl;
                }
            }

            if (! betterMatch)
            {
                //std::cout << "And it's the best match." << std::endl;
                return i;
            }
            else
                continue;
        }
    }

    int candidate = -1;

    for (int i = 0; i < savedDataStreamParameters.size(); i++)
    {
        ParameterCollection* params = savedDataStreamParameters[i];

        // matching channels and sample rate --> this is pretty good
        if (params->owner.sample_rate == stream->getSampleRate() && params->owner.channel_count == stream->getChannelCount())
        {
            // std::cout << "Found matching sample rate + channel count." << std::endl;

            bool betterMatch = false;

            for (auto otherStream : dataStreams)
            {
                if (otherStream != stream && otherStream->getName().equalsIgnoreCase (params->owner.name))
                {
                    betterMatch = true;
                    //std::cout << "...but found another stream with matching name" << std::endl;
                }
            }

            if (! betterMatch)
            {
                //std::cout << "And it's the best match." << std::endl;
                return i;
            }
            else
                continue;
        }
    }

    for (int i = 0; i < savedDataStreamParameters.size(); i++)
    {
        ParameterCollection* params = savedDataStreamParameters[i];

        // only sample rate match --> still use it
        if (params->owner.sample_rate == stream->getSampleRate())
        {
            //std::cout << "Found matching sample rate." << std::endl;

            bool betterMatch = false;

            for (auto otherStream : dataStreams)
            {
                if (otherStream != stream && params->owner.sample_rate == otherStream->getSampleRate() && params->owner.channel_count == otherStream->getChannelCount())
                {
                    betterMatch = true;
                    //std::cout << "...but found another stream with matching sample rate and channel count" << std::endl;
                }
            }

            if (! betterMatch)
            {
                //std::cout << "And it's the best match." << std::endl;
                return i;
            }
            else
                continue;
        }
    }

    // no match found

    return -1;
}

int GenericProcessor::copyDataStreamSettings (const DataStream* stream, int continuousChannelGlobalIndex)
{
    if (false)
    {
        std::cout << getName() << " " << getNodeId() << std::endl;
        std::cout << "Copying stream " << stream->getName() << ":" << std::endl;
        std::cout << "  Source Node ID: " << stream->getSourceNodeId() << std::endl;
        std::cout << "  Source Node Name: " << stream->getSourceNodeName() << std::endl;
        std::cout << "  Last Node ID: " << stream->getNodeId() << std::endl;
        std::cout << "  Last Node Name: " << stream->getNodeName() << std::endl;
        std::cout << "  Name: " << stream->getName() << std::endl;
        std::cout << "  Stream ID: " << stream->getStreamId() << std::endl;
        std::cout << "  Sample rate: " << stream->getSampleRate() << std::endl;
        std::cout << "  Channel count: " << stream->getChannelCount() << std::endl;
        std::cout << "  " << std::endl;
    }

    dataStreams.add (new DataStream (*stream));

    dataStreams.getLast()->clearChannels();
    dataStreams.getLast()->addProcessor (this);

    for (auto continuousChannel : stream->getContinuousChannels())
    {
        if (false)
        {
            std::cout << "Copying continuous channel: " << std::endl;
            std::cout << "  Source Node ID: " << continuousChannel->getSourceNodeId() << std::endl;
            std::cout << "  Source Node Name: " << continuousChannel->getSourceNodeName() << std::endl;
            std::cout << "  Last Node ID: " << continuousChannel->getNodeId() << std::endl;
            std::cout << "  Last Node Name: " << continuousChannel->getNodeName() << std::endl;
            std::cout << "  Name: " << continuousChannel->getName() << std::endl;
            std::cout << "  Stream ID: " << continuousChannel->getStreamId() << std::endl;
            std::cout << "  Sample rate: " << continuousChannel->getSampleRate() << std::endl;
        }

        continuousChannels.add (new ContinuousChannel (*continuousChannel));
        continuousChannels.getLast()->addProcessor (this);
        continuousChannels.getLast()->setDataStream (dataStreams.getLast(), true);
        continuousChannels.getLast()->setGlobalIndex (continuousChannelGlobalIndex++);
    }

    for (auto eventChannel : stream->getEventChannels())
    {
        if (false)
        {
            std::cout << "Copying event channel: " << std::endl;
            std::cout << "  Source Node ID: " << eventChannel->getSourceNodeId() << std::endl;
            std::cout << "  Source Node Name: " << eventChannel->getSourceNodeName() << std::endl;
            std::cout << "  Last Node ID: " << eventChannel->getNodeId() << std::endl;
            std::cout << "  Last Node Name: " << eventChannel->getNodeName() << std::endl;
            std::cout << "  Name: " << eventChannel->getName() << std::endl;
            std::cout << "  ID: " << eventChannel->getStreamId() << std::endl;
            std::cout << "  Sample rate: " << eventChannel->getSampleRate() << std::endl;
        }

        eventChannels.add (new EventChannel (*eventChannel));
        eventChannels.getLast()->addProcessor (this);
        eventChannels.getLast()->setDataStream (dataStreams.getLast(), true);
    }

    for (auto spikeChannel : stream->getSpikeChannels())
    {
        if (false)
        {
            std::cout << "Copying spike channel: " << std::endl;
            std::cout << "  Source Node ID: " << spikeChannel->getSourceNodeId() << std::endl;
            std::cout << "  Source Node Name: " << spikeChannel->getSourceNodeName() << std::endl;
            std::cout << "  Last Node ID: " << spikeChannel->getNodeId() << std::endl;
            std::cout << "  Last Node Name: " << spikeChannel->getNodeName() << std::endl;
            std::cout << "  Name: " << spikeChannel->getName() << std::endl;
            std::cout << "  ID: " << spikeChannel->getStreamId() << std::endl;
            std::cout << "  Sample rate: " << spikeChannel->getSampleRate() << std::endl;
        }

        spikeChannels.add (new SpikeChannel (*spikeChannel));
        spikeChannels.getLast()->addProcessor (this);
        spikeChannels.getLast()->setDataStream (dataStreams.getLast(), true);
    }

    return continuousChannelGlobalIndex;
}

void GenericProcessor::updateDisplayName (String name)
{
    m_name = name;
}

void GenericProcessor::update()
{
    LOGD ("Updating settings for ", getName(), " (", getNodeId(), ")");

    int64 start = Time::getHighResolutionTicks();

    clearSettings();

    // processorInfo.reset();
    // processorInfo = std::unique_ptr<ProcessorInfoObject>(new ProcessorInfoObject(this));

    if (! isMerger()) // only has one source
    {
        if (sourceNode != nullptr && ! sourceNode->isEmpty())
        {
            int continuousChannelGlobalIndex = 0;

            // copy settings from source node
            messageChannel.reset();
            messageChannel = std::make_unique<EventChannel> (*sourceNode->getMessageChannel());
            messageChannel->addProcessor (this);
            messageChannel->setDataStream (AccessClass::getMessageCenter()->getMessageStream());

            if (sourceNode->isSplitter())
            {
                Splitter* splitter = (Splitter*) sourceNode;

                for (auto stream : splitter->getStreamsForDestNode (this))
                {
                    continuousChannelGlobalIndex = copyDataStreamSettings (stream, continuousChannelGlobalIndex);

                    if (splitter->getDestNode (0) == this)
                    {
                        dataStreams.getLast()->setName (stream->getName() + "-A");
                    }
                    else
                    {
                        dataStreams.getLast()->setName (stream->getName() + "-B");
                    }
                }
            }
            else
            {
                for (auto stream : sourceNode->getStreamsForDestNode (this))
                {
                    continuousChannelGlobalIndex = copyDataStreamSettings (stream, continuousChannelGlobalIndex);
                }
            }

            for (auto configurationObject : sourceNode->configurationObjects)
            {
                configurationObjects.add (new ConfigurationObject (*configurationObject));
            }

            isEnabled = sourceNode->isEnabled;

            if (continuousChannelGlobalIndex == 0)
                isEnabled = false;
        }
        else if (! isEmpty())
        {
            // connect first processor in signal chain to message center
            // messageChannel.reset();
            const EventChannel* originalChannel = AccessClass::getMessageCenter()->getMessageChannel();
            EventChannel* newChannel = new EventChannel (*originalChannel);
            messageChannel.reset (newChannel);
            // messageChannel = std::make_unique<EventChannel>(originalChannel);
            messageChannel->addProcessor (this);
            messageChannel->setDataStream (AccessClass::getMessageCenter()->getMessageStream());

            // If Source call updateSettings immediately
            if (isSource())
                updateSettings();
            else
                isEnabled = false;

            LOGD (getNodeId(), " connected to Message Center");
        }
    }
    else
    {
        updateSettings(); // only for Merger
    }

    updateChannelIndexMaps();

    LOGD ("    Copied upstream settings in ", MS_FROM_START, " milliseconds");

    /// UPDATE PROCESSOR PARAMETERS
    if (! isSource() && numParameters() > 0)
    {
        for (auto param : getParameters())
        {
            if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
            {
                Array<String> streamNames;
                for (auto stream : dataStreams)
                    streamNames.add (stream->getKey());
                SelectedStreamParameter* p = (SelectedStreamParameter*) param;
                p->setStreamNames (streamNames);
                parameterValueChanged (p);
            }
        }
    }

    /// UPDATE PARAMETERS FOR STREAMS
    for (auto stream : dataStreams)
    {
        LOGD ("Stream ", stream->getStreamId(), " - ", stream->getName(), " num channels: ", stream->getChannelCount(), " num parameters: ", stream->numParameters());
        LOGD ("Number of saved params: ", savedDataStreamParameters.size());

        if (stream->numParameters() == 0)
        {
            //std::cout << "No parameters found, adding..." << std::endl;

            for (auto param : dataStreamParameters)
            {
                if (param->getType() == Parameter::BOOLEAN_PARAM)
                {
                    BooleanParameter* p = (BooleanParameter*) param;
                    stream->addParameter (new BooleanParameter (*p));
                }
                else if (param->getType() == Parameter::STRING_PARAM)
                {
                    StringParameter* p = (StringParameter*) param;
                    stream->addParameter (new StringParameter (*p));
                }
                else if (param->getType() == Parameter::INT_PARAM)
                {
                    IntParameter* p = (IntParameter*) param;
                    stream->addParameter (new IntParameter (*p));
                }
                else if (param->getType() == Parameter::FLOAT_PARAM)
                {
                    FloatParameter* p = (FloatParameter*) param;
                    stream->addParameter (new FloatParameter (*p));
                }
                else if (param->getType() == Parameter::CATEGORICAL_PARAM)
                {
                    CategoricalParameter* p = (CategoricalParameter*) param;
                    stream->addParameter (new CategoricalParameter (*p));
                }
                else if (param->getType() == Parameter::SELECTED_CHANNELS_PARAM)
                {
                    SelectedChannelsParameter* p = (SelectedChannelsParameter*) param;
                    SelectedChannelsParameter* p2 = new SelectedChannelsParameter (*p);
                    stream->addParameter (p2);
                    p2->setChannelCount (stream->getChannelCount());
                }
                else if (param->getType() == Parameter::MASK_CHANNELS_PARAM)
                {
                    MaskChannelsParameter* p = (MaskChannelsParameter*) param;
                    MaskChannelsParameter* p2 = new MaskChannelsParameter (*p);
                    stream->addParameter (p2);
                    p2->setChannelCount (stream->getChannelCount());
                }
                else if (param->getType() == Parameter::PATH_PARAM)
                {
                    PathParameter* p = (PathParameter*) param;
                    stream->addParameter (new PathParameter (*p));
                }
                else if (param->getType() == Parameter::SELECTED_STREAM_PARAM)
                {
                    SelectedStreamParameter* p = (SelectedStreamParameter*) param;
                    stream->addParameter (new SelectedStreamParameter (*p));
                }
                else if (param->getType() == Parameter::TIME_PARAM)
                {
                    TimeParameter* p = (TimeParameter*) param;
                    stream->addParameter (new TimeParameter (*p));
                }
                else if (param->getType() == Parameter::NOTIFICATION_PARAM)
                {
                    NotificationParameter* p = (NotificationParameter*) param;
                    stream->addParameter (new NotificationParameter (*p));
                }
                else if (param->getType() == Parameter::TTL_LINE_PARAM)
                {
                    TtlLineParameter* p = (TtlLineParameter*) param;
                    stream->addParameter (new TtlLineParameter (*p));
                }
            }
        }
        else
        {
            for (auto param : dataStreamParameters)
            {
                if (param->getType() == Parameter::SELECTED_CHANNELS_PARAM)
                {
                    SelectedChannelsParameter* p = (SelectedChannelsParameter*) stream->getParameter (param->getName());
                    p->setChannelCount (stream->getChannelCount());
                    LOGD ("GenericProcessor::update() Setting SelectedChannelsParameter channel count for ", stream->getStreamId(), " to ", stream->getChannelCount(), " channels");
                }
                else if (param->getType() == Parameter::MASK_CHANNELS_PARAM)
                {
                    MaskChannelsParameter* p = (MaskChannelsParameter*) stream->getParameter (param->getName());
                    p->setChannelCount (stream->getChannelCount());
                    LOGD ("GenericProcessor::update() Setting MaskChannelsParameter channel count for ", stream->getStreamId(), " to ", stream->getChannelCount(), " channels");
                }
            }
        }

        // LOGC( "Stream ", stream->getStreamId(), " - ", stream->getName(), " num channels: ", stream->getChannelCount(), " num parameters: ", stream->numParameters());

        if (savedDataStreamParameters.size() > 0)
        {
            int index = findMatchingStreamParameters (stream);

            if (index > -1)
            {
                LOGD ("GenericProcessor::update() Copying savedDataStreamParameters for ", stream->getStreamId());
                LOGD ("COPYING STREAM PARAMETERS TO");
                savedDataStreamParameters[index]->copyParametersTo (stream);
                savedDataStreamParameters.remove (index);
            }
        }
    }

    /// UPDATE PARAMETERS FOR SPIKE CHANNELS
    for (auto spikeChannel : spikeChannels)
    {
        if (spikeChannel->isLocal())
        {
            DataStream* similarStream = spikeChannel->findSimilarStream (dataStreams);

            if (similarStream != nullptr)
            {
                if (! (*similarStream)["enable_stream"])
                    continue;
            }

            spikeChannel->setDataStream (similarStream, true);

            int channelCount = similarStream != nullptr ? similarStream->getChannelCount() : 0;

            for (auto param : spikeChannel->getParameters())
            {
                if (param->getType() == Parameter::SELECTED_CHANNELS_PARAM)
                {
                    SelectedChannelsParameter* p = (SelectedChannelsParameter*) spikeChannel->getParameter (param->getName());

                    p->setChannelCount (channelCount);
                }
            }
        }
    }

    LOGG ("    Copied parameters in ", MS_FROM_START, " milliseconds");

    if (! isMerger() && ! isSource())
        updateSettings(); // allow processors to change custom settings,
            // including creation of streams / channels and
            // setting isEnabled variable

    LOGG ("    Updated custom settings in ", MS_FROM_START, " milliseconds");

    updateChannelIndexMaps();

    m_needsToSendTimestampMessages.clear();
    for (auto stream : getDataStreams())
        m_needsToSendTimestampMessages[stream->getStreamId()] = true;

    // required for the ProcessorGraph to know the
    // details of this processor:
    setPlayConfigDetails (getNumInputs(), // numIns
                          getNumOutputs(), // numOuts
                          44100.0, // sampleRate (always 44100 Hz, default audio card rate)
                          128); // blockSize

    if (editor != nullptr)
        editor->update (isEnabled); // allow the editor to update its settings

    LOGG ("    TOTAL TIME: ", MS_FROM_START, " milliseconds");
}

void GenericProcessor::updateChannelIndexMaps()
{
    continuousChannelMap.clear();
    eventChannelMap.clear();
    spikeChannelMap.clear();
    dataStreamMap.clear();

    if (dataStreams.size() == 0)
        return;

    for (int i = 0; i < continuousChannels.size(); i++)
    {
        ContinuousChannel* chan = continuousChannels[i];
        chan->setGlobalIndex (i);

        uint16 processorId = chan->getSourceNodeId();
        uint16 streamId = chan->getStreamId();
        uint16 localIndex = chan->getLocalIndex();

        continuousChannelMap[processorId][streamId][localIndex] = chan;
    }

    for (int i = 0; i < eventChannels.size(); i++)
    {
        EventChannel* chan = eventChannels[i];
        chan->setGlobalIndex (i);

        uint16 processorId = chan->getSourceNodeId();
        uint16 streamId = chan->getStreamId();
        uint16 localIndex = chan->getLocalIndex();

        eventChannelMap[processorId][streamId][localIndex] = chan;
    }

    for (int i = 0; i < spikeChannels.size(); i++)
    {
        SpikeChannel* chan = spikeChannels[i];
        chan->setGlobalIndex (i);

        uint16 processorId = chan->getSourceNodeId();
        uint16 streamId = chan->getStreamId();
        uint16 localIndex = chan->getLocalIndex();

        spikeChannelMap[processorId][streamId][localIndex] = chan;
    }

    for (int i = 0; i < dataStreams.size(); i++)
    {
        DataStream* stream = dataStreams[i];

        uint16 streamId = stream->getStreamId();

        dataStreamMap[streamId] = stream;
    }

    if (latencyMeter != nullptr)
        latencyMeter->update (getDataStreams());
}

String GenericProcessor::handleConfigMessage (const String& msg)
{
    return "";
}

void GenericProcessor::getDefaultEventInfo (Array<DefaultEventInfo>& events, int subproc) const
{
    events.clear();
}

uint32 GenericProcessor::getNumSamplesInBlock (uint16 streamId) const
{
    return numSamplesInBlock.at (streamId);
}

int64 GenericProcessor::getFirstSampleNumberForBlock (uint16 streamId) const
{
    return startSamplesForBlock.at (streamId);
}

double GenericProcessor::getFirstTimestampForBlock (uint16 streamId) const
{
    return startTimestampsForBlock.at (streamId);
}

void GenericProcessor::setTimestampAndSamples (int64 sampleNumber,
                                               double timestamp,
                                               uint32 nSamples,
                                               uint16 streamId,
                                               uint16 syncStreamId)
{
    HeapBlock<char> data;
    size_t dataSize = SystemEvent::fillTimestampAndSamplesData (data,
                                                                this,
                                                                streamId,
                                                                sampleNumber,
                                                                timestamp,
                                                                nSamples,
                                                                m_initialProcessTime,
                                                                syncStreamId);

    m_currentMidiBuffer->addEvent (data, int(dataSize), 0);

    //since the processor generating the timestamp won't get the event, add it to the map
    startTimestampsForBlock[streamId] = timestamp;
    startSamplesForBlock[streamId] = sampleNumber;
    syncStreamIds[streamId] = syncStreamId;
    processStartTimes[streamId] = m_initialProcessTime;
}

std::optional<std::pair<int64, double>> GenericProcessor::getReferenceSampleForBlock (uint16 streamId)
{
    if (referenceSamplesForBlock.find (streamId) != referenceSamplesForBlock.end())
    {
        return referenceSamplesForBlock.at (streamId);
    }
    else
    {
        return std::nullopt;
    }
}

void GenericProcessor::setReferenceSample (uint16 streamId,
                                           double timestamp,
                                           int64 sampleIndex)
{
    //Check last referenceSample and return if this sample is equal
    if (referenceSamplesForBlock.find (streamId) != referenceSamplesForBlock.end())
    {
        std::optional<std::pair<int64, double>> lastReferenceSample = referenceSamplesForBlock.at (streamId);
        if (lastReferenceSample.has_value())
        {
            if (sampleIndex == lastReferenceSample.value().first && timestamp == lastReferenceSample.value().second)
            {
                return;
            }
        }
    }

    HeapBlock<char> data;
    size_t dataSize = SystemEvent::fillReferenceSampleEvent (data,
                                                             this,
                                                             streamId,
                                                             sampleIndex,
                                                             timestamp);

    m_currentMidiBuffer->addEvent (data, dataSize, 0);
    referenceSamplesForBlock[streamId] = { sampleIndex, timestamp };
}

int GenericProcessor::getGlobalChannelIndex (uint16 streamId, int localIndex) const
{
    return getDataStream (streamId)->getContinuousChannels()[localIndex]->getGlobalIndex();
}

int GenericProcessor::processEventBuffer()
{
    //
    // This loops through all events in the buffer, and uses the BUFFER_SIZE
    // events to determine the number of samples in the current buffer. If
    // there are multiple such events, the one with the highest number of
    // samples will be used.
    // This approach is not ideal, as it will become a problem if we allow
    // the sample rate to change at different points in the signal chain.
    //
    int numRead = 0;

    if (m_currentMidiBuffer->getNumEvents() > 0)
    {
        for (const auto meta : *m_currentMidiBuffer)
        {
            const uint8* dataptr = meta.data;

            if (static_cast<Event::Type> (*dataptr) == Event::Type::SYSTEM_EVENT
                && static_cast<SystemEvent::Type> (*(dataptr + 1) == SystemEvent::Type::TIMESTAMP_AND_SAMPLES))
            {
                uint16 sourceProcessorId = *reinterpret_cast<const uint16*> (dataptr + 2);
                uint16 sourceStreamId = *reinterpret_cast<const uint16*> (dataptr + 4);
                uint16 syncStreamId = *reinterpret_cast<const uint16*> (dataptr + 6);

                int64 startSample = *reinterpret_cast<const int64*> (dataptr + 8);
                double startTimestamp = *reinterpret_cast<const double*> (dataptr + 16);
                uint32 nSamples = *reinterpret_cast<const uint32*> (dataptr + 24);
                int64 initialTicks = *reinterpret_cast<const int64*> (dataptr + 28);

                // if (startSamplesForBlock[sourceStreamId] > startSample)
                //    std::cout << "GET: " << getNodeId() << " " << sourceStreamId << " " << startSamplesForBlock[sourceStreamId] << " " << startSample << std::endl;

                startSamplesForBlock[sourceStreamId] = startSample;
                startTimestampsForBlock[sourceStreamId] = startTimestamp;
                syncStreamIds[sourceStreamId] = syncStreamId;
                numSamplesInBlock[sourceStreamId] = nSamples;
                processStartTimes[sourceStreamId] = initialTicks;
            }
            else if (static_cast<Event::Type> (*dataptr) == Event::Type::PROCESSOR_EVENT
                     && static_cast<EventChannel::Type> (*(dataptr + 1) == EventChannel::Type::TTL))
            {
                uint16 sourceStreamId = *reinterpret_cast<const uint16*> (dataptr + 4);
                uint8 eventBit = *reinterpret_cast<const uint8*> (dataptr + 24);
                bool eventState = *reinterpret_cast<const bool*> (dataptr + 25);

                if (! headlessMode)
                {
                    getEditor()->setTTLState (sourceStreamId, eventBit, eventState);
                }
            }
            else if (static_cast<Event::Type> (*dataptr) == Event::Type::PROCESSOR_EVENT
                     && static_cast<EventChannel::Type> (*(dataptr + 1) == EventChannel::Type::TEXT))
            {
                TextEventPtr textEvent = TextEvent::deserialize (dataptr, getMessageChannel());

                handleBroadcastMessage (textEvent->getText(), textEvent->getSampleNumber());
            }
        }
    }

    return numRead;
}

int GenericProcessor::checkForEvents (bool checkForSpikes)
{
    if (m_currentMidiBuffer->getNumEvents() > 0)
    {
        /** Since adding events to the buffer inside this loop could be dangerous, create a temporary event buffer
		    so any call to addEvent will operate on it; */
        MidiBuffer temporaryEventBuffer;
        MidiBuffer* originalEventBuffer = m_currentMidiBuffer;
        m_currentMidiBuffer = &temporaryEventBuffer;

        for (const auto meta : *originalEventBuffer)
        {
            uint16 sourceProcessorId = EventBase::getProcessorId (meta.data);
            uint16 sourceStreamId = EventBase::getStreamId (meta.data);
            uint16 sourceChannelIdx = EventBase::getChannelIndex (meta.data);

            if (EventBase::getBaseType (meta.data) == Event::Type::PROCESSOR_EVENT)
            {
                if (static_cast<EventChannel::Type> (*(meta.data + 1) != EventChannel::Type::TEXT))
                {
                    const EventChannel* eventChannel = getEventChannel (sourceProcessorId, sourceStreamId, sourceChannelIdx);

                    if (eventChannel != nullptr)
                    {
                        handleTTLEvent (TTLEvent::deserialize (meta.data, eventChannel));
                    }
                }
            }
            else if (checkForSpikes && EventBase::getBaseType (meta.data) == Event::Type::SPIKE_EVENT)
            {
                const SpikeChannel* spikeChannel = getSpikeChannel (sourceProcessorId, sourceStreamId, sourceChannelIdx);

                if (spikeChannel != nullptr)
                {
                    handleSpike (Spike::deserialize (meta.data, spikeChannel));
                }
            }
        }
        // Restore the original buffer pointer and, if some new events have
        // been added here, copy them to the original buffer
        m_currentMidiBuffer = originalEventBuffer;

        if (temporaryEventBuffer.getNumEvents() > 0)
        {
            m_currentMidiBuffer->addEvents (temporaryEventBuffer, 0, -1, 0);
        }

        return 0;
    }

    return -1;
}

void GenericProcessor::addEvent (const Event* event, int sampleNum)
{
    size_t size = event->getChannelInfo()->getDataSize() + event->getChannelInfo()->getTotalEventMetadataSize() + EVENT_BASE_SIZE;

    HeapBlock<char> buffer (size);

    event->serialize (buffer, size);

    m_currentMidiBuffer->addEvent (buffer, int(size), sampleNum >= 0 ? sampleNum : 0);

    if (event->getBaseType() == Event::Type::PROCESSOR_EVENT)
    {
        if (event->getEventType() == EventChannel::Type::TTL)
        {
            const uint8* dataptr = reinterpret_cast<const uint8*> (event->getRawDataPointer());

            if (! headlessMode)
                getEditor()->setTTLState (event->getStreamId(),
                                          *(dataptr),
                                          *(dataptr + 1));
        }
    }
}

void GenericProcessor::addTTLChannel (String name)
{
    if (dataStreams.size() == 0)
    {
        return;
    }

    if (ttlEventChannel == nullptr)
    {
        EventChannel::Settings settings {
            EventChannel::Type::TTL,
            name,
            "Default TTL event channel",
            "ttl.events",
            dataStreams[0]
        };

        eventChannels.add (new EventChannel (settings));
        ttlEventChannel = eventChannels.getLast();

        ttlLineStates.clear();

        for (int i = 0; i < 8; i++)
            ttlLineStates.add (false);
    }
    else
    {
        jassert (false); // this shouldn't be called twice in updateSettings()
    }
}

void GenericProcessor::flipTTLState (int sampleIndex, int lineIndex)
{
    if (lineIndex < 0 || lineIndex >= 8)
        return;

    bool currentState = ttlLineStates[lineIndex];
    ttlLineStates.set (lineIndex, ! currentState);

    int64 startSample = startSamplesForBlock[ttlEventChannel->getStreamId()] + sampleIndex;

    TTLEventPtr eventPtr = TTLEvent::createTTLEvent (ttlEventChannel, startSample, lineIndex, ! currentState);

    addEvent (eventPtr, sampleIndex);
}

void GenericProcessor::setTTLState (int sampleIndex, int lineIndex, bool state)
{
    if (lineIndex < 0 || lineIndex >= 8)
        return;

    ttlLineStates.set (lineIndex, state);

    int64 startSample = startSamplesForBlock[ttlEventChannel->getStreamId()] + sampleIndex;

    TTLEventPtr eventPtr = TTLEvent::createTTLEvent (ttlEventChannel, startSample, lineIndex, state);

    addEvent (eventPtr, sampleIndex);
}

bool GenericProcessor::getTTLState (int lineIndex)
{
    if (lineIndex < 0 || lineIndex >= 8)
        return false;

    return ttlLineStates[lineIndex];
}

void GenericProcessor::broadcastMessage (String msg)
{
    AccessClass::getMessageCenter()->broadcastMessage (msg);
}

void GenericProcessor::sendConfigMessage (GenericProcessor* destination, String message)
{
    AccessClass::getProcessorGraph()->sendConfigMessage (destination, message);
}

void GenericProcessor::addSpike (const Spike* spike)
{
    size_t size = SPIKE_BASE_SIZE
                  + spike->spikeChannel->getDataSize()
                  + spike->spikeChannel->getTotalEventMetadataSize()
                  + spike->spikeChannel->getNumChannels() * sizeof (float);

    HeapBlock<char> buffer (size);

    spike->serialize (buffer, size);

    m_currentMidiBuffer->addEvent (buffer, int(size), 0);
}

void GenericProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& eventBuffer)
{
    if (isSource())
        m_initialProcessTime = Time::getHighResolutionTicks();

    m_currentMidiBuffer = &eventBuffer;

    processEventBuffer(); // extract buffer sizes and timestamps,

    process (buffer);

    if (! headlessMode)
        latencyMeter->setLatestLatency (processStartTimes);
}

Array<const EventChannel*> GenericProcessor::getEventChannels()
{
    Array<const EventChannel*> channels;

    for (int i = 0; i < eventChannels.size(); i++)
    {
        channels.add (eventChannels[i]);
    }

    return channels;
}

Array<const DataStream*> GenericProcessor::getStreamsForDestNode (GenericProcessor* p)
{
    Array<const DataStream*> streams;

    for (int i = 0; i < dataStreams.size(); i++)
    {
        streams.add (dataStreams[i]);
    }

    return streams;
}

Array<const DataStream*> GenericProcessor::getDataStreams() const
{
    Array<const DataStream*> streams;

    for (int i = 0; i < dataStreams.size(); i++)
    {
        streams.add (dataStreams[i]);
    }

    return streams;
}

const ContinuousChannel* GenericProcessor::getContinuousChannel (uint16 processorId, uint16 streamId, uint16 localIndex) const
{
    return continuousChannelMap.at (processorId).at (streamId).at (localIndex);
}

int GenericProcessor::getIndexOfMatchingChannel (const ContinuousChannel* channel) const
{
    for (int index = 0; index < continuousChannels.size(); index++)
    {
        if (*continuousChannels[index] == *channel) // check for matching Uuid
        {
            return index;
        }
    }

    return -1;
}

int GenericProcessor::getIndexOfMatchingChannel (const EventChannel* channel) const
{
    for (int index = 0; index < eventChannels.size(); index++)
    {
        if (*eventChannels[index] == *channel) // check for matching Uuid
        {
            return index;
        }
    }

    return -1;
}

int GenericProcessor::getIndexOfMatchingChannel (const SpikeChannel* channel) const
{
    for (int index = 0; index < spikeChannels.size(); index++)
    {
        if (*spikeChannels[index] == *channel) // check for matching Uuid
        {
            return index;
        }
    }

    return -1;
}

const EventChannel* GenericProcessor::getEventChannel (uint16 processorId, uint16 streamId, uint16 localIndex) const
{
    if (eventChannelMap.find (processorId) != eventChannelMap.end())
        return eventChannelMap.at (processorId).at (streamId).at (localIndex);

    return getMessageChannel();
}

const EventChannel* GenericProcessor::getMessageChannel() const
{
    return messageChannel.get();
}

const SpikeChannel* GenericProcessor::getSpikeChannel (uint16 processorId, uint16 streamId, uint16 localIndex) const
{
    return spikeChannelMap.at (processorId).at (streamId).at (localIndex);
}

DataStream* GenericProcessor::getDataStream (uint16 streamId) const
{
    return dataStreamMap.at (streamId);
}

DataStream* GenericProcessor::getDataStream (String streamKey) const
{
    for (auto stream : dataStreams)
    {
        if (stream->getKey() == streamKey)
            return stream;
    }
    return nullptr;
}

uint16 GenericProcessor::findSimilarStream (int sourceNodeId, String name, float sample_rate, bool sourceNodeIdMustMatch)
{
    if (dataStreams.size() > 0)
    {
        if (sourceNodeId == 0) // previously empty stream
            return dataStreams[0]->getStreamId(); // add to first stream

        for (auto stream : dataStreams)
        {
            if (stream->getSourceNodeId() == sourceNodeId
                && stream->getName() == name
                && stream->getSampleRate() == sample_rate)
            {
                // perfect match
                return stream->getStreamId();
            }
        }

        if (! sourceNodeIdMustMatch)
        {
            for (auto stream : dataStreams)
            {
                if (stream->getName() == name
                    && stream->getSampleRate() == sample_rate)
                {
                    // name and sample rate match
                    return stream->getStreamId();
                }
            }

            for (auto stream : dataStreams)
            {
                if (stream->getSampleRate() == sample_rate)
                {
                    // sample rate match
                    return stream->getStreamId();
                }
            }
        }
    }

    // no streams with any matching characteristics
    return 0;
}

const ConfigurationObject* GenericProcessor::getConfigurationObject (int index) const
{
    return configurationObjects[index];
}

int GenericProcessor::getTotalContinuousChannels() const
{
    return continuousChannels.size();
}

int GenericProcessor::getTotalEventChannels() const
{
    return eventChannels.size();
}

int GenericProcessor::getTotalSpikeChannels() const
{
    return spikeChannels.size();
}

int GenericProcessor::getTotalConfigurationObjects() const
{
    return configurationObjects.size();
}

const ContinuousChannel* GenericProcessor::getContinuousChannel (int globalIndex) const
{
    if (globalIndex < continuousChannels.size())
        return continuousChannels[globalIndex];
    else
        return nullptr;
}

const SpikeChannel* GenericProcessor::getSpikeChannel (int globalIndex) const
{
    if (globalIndex < spikeChannels.size())
        return spikeChannels[globalIndex];
    else
        return nullptr;
}

const EventChannel* GenericProcessor::getEventChannel (int globalIndex) const
{
    if (globalIndex < eventChannels.size())
        return eventChannels[globalIndex];
    else
        return nullptr;
}

void GenericProcessor::saveToXml (XmlElement* xml)
{
    xml->setAttribute ("nodeId", nodeId);

    XmlElement* paramsXml = xml->createNewChildElement ("PROCESSOR_PARAMETERS");

    for (auto param : getParameters())
    {
        param->toXml (paramsXml);
    }

    for (auto stream : dataStreams)
    {
        XmlElement* streamXml = xml->createNewChildElement ("STREAM");

        streamXml->setAttribute ("name", stream->getName());
        streamXml->setAttribute ("description", stream->getDescription());
        streamXml->setAttribute ("sample_rate", stream->getSampleRate());
        streamXml->setAttribute ("channel_count", stream->getChannelCount());

        if (stream->hasDevice())
            streamXml->setAttribute ("device_name", stream->device->getName());

        XmlElement* streamParamsXml = streamXml->createNewChildElement ("PARAMETERS");

        for (auto param : stream->getParameters())
            param->toXml (streamParamsXml);

        /*for (auto eventChannel : stream->getEventChannels())
        {
            if (eventChannel->numParameters() > 0)
            {
                XmlElement* eventParamsXml = streamXml->createNewChildElement("EVENT_CHANNEL");
                eventParamsXml->setAttribute("name",eventChannel->getName());
                eventParamsXml->setAttribute("description", eventChannel->getDescription());
                
                for (auto param : eventChannel->getParameters())
                    param->toXml(eventParamsXml);
            }
        }
        
        for (auto continuousChannel : stream->getContinuousChannels())
        {
            if (continuousChannel->numParameters() > 0)
            {
                XmlElement* continuousParamsXml = streamXml->createNewChildElement("CONTINUOUS_CHANNEL");
                continuousParamsXml->setAttribute("name",continuousChannel->getName());
                continuousParamsXml->setAttribute("description", continuousChannel->getDescription());
                
                for (auto param : continuousChannel->getParameters())
                    param->toXml(continuousParamsXml);
            }
        }*/
    }

    saveCustomParametersToXml (xml->createNewChildElement ("CUSTOM_PARAMETERS"));

    if (! headlessMode)
        getEditor()->saveToXml (xml->createNewChildElement ("EDITOR"));
}

void GenericProcessor::saveCustomParametersToXml (XmlElement* parentElement)
{
}

void GenericProcessor::loadFromXml()
{
    if (parametersAsXml != nullptr)
    {
        LOGD ("Loading parameters for ", getName(), " (", getNodeId(), ")");

        int64 start = Time::getHighResolutionTicks();

        for (auto* xmlNode : parametersAsXml->getChildIterator())
        {
            // Check for procesor-scoped parameter attributes
            if (xmlNode->hasTagName ("PROCESSOR_PARAMETERS") || xmlNode->hasTagName ("GLOBAL_PARAMETERS"))
            {
                for (int i = 0; i < xmlNode->getNumAttributes(); i++)
                {
                    auto param = getParameter (xmlNode->getAttributeName (i));
                    param->fromXml (xmlNode);
                    parameterValueChanged (param);
                }
            }

            if (xmlNode->hasTagName ("STREAM") && dataStreams.size() > 0)
            {
                ParameterCollection* parameterCollection = new ParameterCollection();

                parameterCollection->owner.channel_count = xmlNode->getIntAttribute ("channel_count");
                parameterCollection->owner.name = xmlNode->getStringAttribute ("name");
                parameterCollection->owner.sample_rate = xmlNode->getDoubleAttribute ("sample_rate");

                if (xmlNode->hasAttribute ("device_name"))
                    parameterCollection->owner.deviceName = xmlNode->getStringAttribute ("device_name");

                for (auto* streamParams : xmlNode->getChildIterator())
                {
                    if (streamParams->hasTagName ("PARAMETERS"))
                    {
                        for (int i = 0; i < streamParams->getNumAttributes(); i++)
                        {
                            Parameter* parameter;

                            String name = streamParams->getAttributeName (i);

                            if (dataStreams[0]->hasParameter (name))
                            {
                                parameter = dataStreams[0]->getParameter (name);

                                if (parameter->getType() == Parameter::INT_PARAM)
                                {
                                    IntParameter* p = (IntParameter*) parameter;
                                    IntParameter* p2 = new IntParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::BOOLEAN_PARAM)
                                {
                                    BooleanParameter* p = (BooleanParameter*) parameter;
                                    BooleanParameter* p2 = new BooleanParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::STRING_PARAM)
                                {
                                    StringParameter* p = (StringParameter*) parameter;
                                    StringParameter* p2 = new StringParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::SELECTED_CHANNELS_PARAM)
                                {
                                    SelectedChannelsParameter* p = (SelectedChannelsParameter*) parameter;

                                    SelectedChannelsParameter* p2 = new SelectedChannelsParameter (*p);
                                    p2->setChannelCount (parameterCollection->owner.channel_count);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::MASK_CHANNELS_PARAM)
                                {
                                    MaskChannelsParameter* p = (MaskChannelsParameter*) parameter;
                                    MaskChannelsParameter* p2 = new MaskChannelsParameter (*p);
                                    p2->setChannelCount (parameterCollection->owner.channel_count);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::CATEGORICAL_PARAM)
                                {
                                    CategoricalParameter* p = (CategoricalParameter*) parameter;
                                    CategoricalParameter* p2 = new CategoricalParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::FLOAT_PARAM)
                                {
                                    FloatParameter* p = (FloatParameter*) parameter;
                                    FloatParameter* p2 = new FloatParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::PATH_PARAM)
                                {
                                    LOGD ("Loading path parameter: ", name);
                                    PathParameter* p = (PathParameter*) parameter;
                                    PathParameter* p2 = new PathParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::SELECTED_STREAM_PARAM)
                                {
                                    SelectedStreamParameter* p = (SelectedStreamParameter*) parameter;
                                    SelectedStreamParameter* p2 = new SelectedStreamParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::TIME_PARAM)
                                {
                                    TimeParameter* p = (TimeParameter*) parameter;
                                    TimeParameter* p2 = new TimeParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else if (parameter->getType() == Parameter::TTL_LINE_PARAM)
                                {
                                    TtlLineParameter* p = (TtlLineParameter*) parameter;
                                    TtlLineParameter* p2 = new TtlLineParameter (*p);
                                    p2->fromXml (streamParams);
                                    parameterCollection->addParameter (p2);
                                }
                                else
                                {
                                    jassertfalse;
                                }
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }
                }

                savedDataStreamParameters.add (parameterCollection);
            }
        }

        for (auto stream : dataStreams)
        {
            if (savedDataStreamParameters.size() > 0)
            {
                int index = findMatchingStreamParameters (stream);

                if (index > -1)
                {
                    savedDataStreamParameters[index]->copyParameterValuesTo (stream);

                    for (auto param : stream->getParameters())
                    {
                        // TODO: This is necessary to prevent crash when undoing DeleteProcessor
                        //
                        if (param->getOwner() == nullptr)
                            continue;

                        parameterValueChanged (param);

                        //LOGD("Stream: ", stream->getName());
                        /*
                        if (param->getName() == "enable_stream" && isFilter())
                        {
                            if(!headlessMode) {
                                getEditor()->streamEnabledStateChanged(stream->getStreamId(),
                                                                       (bool)param->getValue(),
                                                                       true);
                            }
                        }
                         */
                    }

                    savedDataStreamParameters.remove (index);
                }
            }
        }

        savedDataStreamParameters.clear();

        LOGG ("    Loaded stream parameters in ", MS_FROM_START, " milliseconds");

        for (auto* xmlNode : parametersAsXml->getChildWithTagNameIterator ("CUSTOM_PARAMETERS"))
        {
            loadCustomParametersFromXml (xmlNode);
        }

        LOGG ("    Loaded custom parameters in ", MS_FROM_START, " milliseconds");

        for (auto* xmlNode : parametersAsXml->getChildWithTagNameIterator ("EDITOR"))
        {
            if (editor != nullptr)
                getEditor()->loadFromXml (xmlNode);

            LOGG ("    Loaded editor parameters in ", MS_FROM_START, " milliseconds");
        }
    }

    m_paramsWereLoaded = true;
}

void GenericProcessor::loadCustomParametersFromXml (XmlElement*) {}

void GenericProcessor::setCurrentChannel (int chan)
{
    currentChannel = chan;
}

void GenericProcessor::setProcessorType (Plugin::Processor::Type processorType)
{
    m_processorType = processorType;
}

bool GenericProcessor::canSendSignalTo (GenericProcessor*) const { return true; }

bool GenericProcessor::generatesTimestamps() const { return false; }

bool GenericProcessor::isFilter() const { return getProcessorType() == Plugin::Processor::FILTER; }
bool GenericProcessor::isSource() const { return getProcessorType() == Plugin::Processor::SOURCE; }
bool GenericProcessor::isSink() const { return getProcessorType() == Plugin::Processor::SINK; }
bool GenericProcessor::isSplitter() const { return getProcessorType() == Plugin::Processor::SPLITTER; }
bool GenericProcessor::isMerger() const { return getProcessorType() == Plugin::Processor::MERGER; }
bool GenericProcessor::isAudioMonitor() const { return getProcessorType() == Plugin::Processor::AUDIO_MONITOR; }
bool GenericProcessor::isUtility() const { return getProcessorType() == Plugin::Processor::UTILITY; }
bool GenericProcessor::isRecordNode() const { return getProcessorType() == Plugin::Processor::RECORD_NODE; }
bool GenericProcessor::isEmpty() const { return getProcessorType() == Plugin::Processor::EMPTY; }

Plugin::Processor::Type GenericProcessor::getProcessorType() const
{
    return m_processorType;
}

String GenericProcessor::getProcessorTypeString() const
{
    if (isSource())
        return "Source";
    else if (isSink())
        return "Sink";
    else if (isFilter())
        return "Filter";
    else if (isUtility())
        return "Utility";
    else
        return "Empty";
}

Plugin::Processor::Type GenericProcessor::typeFromString (String typeName)
{
    if (typeName.equalsIgnoreCase ("Source"))
        return Plugin::Processor::SOURCE;
    else if (typeName.equalsIgnoreCase ("Filter"))
        return Plugin::Processor::FILTER;
    else if (typeName.equalsIgnoreCase ("Sink"))
        return Plugin::Processor::SINK;
    else if (typeName.equalsIgnoreCase ("Utility"))
        return Plugin::Processor::UTILITY;
    else
        return Plugin::Processor::INVALID;
}

int GenericProcessor::getNumInputs() const
{
    if (sourceNode != nullptr)
    {
        return continuousChannels.size();
    }
    else
    {
        return 0;
    }
}

int GenericProcessor::getNumOutputs() const
{
    return continuousChannels.size();
}

int GenericProcessor::getNumOutputsForStream (int streamIdx) const
{
    return dataStreams[streamIdx]->getChannelCount();
}

int GenericProcessor::getNodeId() const { return nodeId; }

float GenericProcessor::getDefaultSampleRate() const { return 44100.0; }
float GenericProcessor::getSampleRate (int) const { return getDefaultSampleRate(); }

GenericProcessor* GenericProcessor::getSourceNode() const { return sourceNode; }
GenericProcessor* GenericProcessor::getDestNode() const { return destNode; }

int GenericProcessor::getNumDataStreams() const { return dataStreams.size(); }

GenericEditor* GenericProcessor::getEditor() const { return editor.get(); }

AudioBuffer<float>* GenericProcessor::getContinuousBuffer() const { return 0; }
MidiBuffer* GenericProcessor::getEventBuffer() const { return 0; }

void GenericProcessor::switchIO (int) {}
void GenericProcessor::switchIO() {}

void GenericProcessor::setPathToProcessor (GenericProcessor* p) {}
void GenericProcessor::setMergerSourceNode (GenericProcessor* sn) {}
void GenericProcessor::setSplitterDestNode (GenericProcessor* dn) {}

bool GenericProcessor::startAcquisition() { return true; }
bool GenericProcessor::stopAcquisition() { return true; }

GenericProcessor::DefaultEventInfo::DefaultEventInfo (EventChannel::Type t, unsigned int c, unsigned int l, float s)
    : type (t),
      nChannels (c),
      length (l),
      sampleRate (s)
{
}

GenericProcessor::DefaultEventInfo::DefaultEventInfo()
    : type (EventChannel::INVALID),
      nChannels (0),
      length (0),
      sampleRate (44100)
{
}

LatencyMeter::LatencyMeter (GenericProcessor* processor_)
    : processor (processor_),
      counter (0)
{
}

void LatencyMeter::update (Array<const DataStream*> dataStreams)
{
    latencies.clear();

    for (auto dataStream : dataStreams)
        latencies[dataStream->getStreamId()].insertMultiple (0, 0, 5);
}

void LatencyMeter::setLatestLatency (std::map<uint16, juce::int64>& processStartTimes)
{
    if (counter % 10 == 0) // update latency estimate every 10 process blocks
    {
        std::map<uint16, juce::int64>::iterator it = processStartTimes.begin();

        int64 currentTime = Time::getHighResolutionTicks();

        while (it != processStartTimes.end())
        {
            latencies[it->first].set (counter % 5, int(currentTime - it->second));
            it++;
        }

        if (counter % 50 == 0) // compute mean latency every 50 process blocks
        {
            std::map<uint16, juce::int64>::iterator it = processStartTimes.begin();

            while (it != processStartTimes.end())
            {
                float totalLatency = 0.0f;

                for (int i = 0; i < 10; i++)
                    totalLatency += float (latencies[it->first][i]);

                totalLatency = totalLatency
                               / float (Time::getHighResolutionTicksPerSecond())
                               * 1000.0f;

                processor->getEditor()->setMeanLatencyMs (it->first, totalLatency);

                it++;
            }
        }
    }

    counter++;
}
