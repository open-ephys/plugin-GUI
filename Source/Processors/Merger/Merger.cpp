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

#include "Merger.h"
#include "MergerEditor.h"

#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"
#include "../../Utils/Utils.h"

#include "../Settings/ConfigurationObject.h"

#include "../MessageCenter/MessageCenter.h"

Merger::Merger()
    : GenericProcessor ("Merger"),
      mergeEventsA (true),
      mergeContinuousA (true),
      mergeEventsB (true),
      mergeContinuousB (true),
      sourceNodeA (nullptr),
      sourceNodeB (nullptr),
      activePath (0)
{
    sendSampleCount = false;
}

AudioProcessorEditor* Merger::createEditor()
{
    editor = std::make_unique<MergerEditor> (this);
    return editor.get();
}

void Merger::setMergerSourceNode (GenericProcessor* sn)
{
    sourceNode = sn;

    if (activePath == 0)
    {
        LOGD ("Setting source node A.");
        sourceNodeA = sn;
    }
    else
    {
        LOGD ("Setting source node B.");
        sourceNodeB = sn;
    }
}

void Merger::switchIO (int sourceNum)
{
    LOGDD ("Merger switching source path to ", sourceNum);

    if (! headlessMode)
        static_cast<MergerEditor*> (getEditor())->switchSource (sourceNum, false);

    activePath = sourceNum;

    if (sourceNum == 0)
    {
        sourceNode = sourceNodeA;
    }
    else
    {
        sourceNode = sourceNodeB;
    }
}

int Merger::switchToSourceNode (GenericProcessor* sn)
{
    if (sn == sourceNodeA)
    {
        switchIO (0);
        return 0;
    }

    if (sn == sourceNodeB)
    {
        switchIO (1);
        return 1;
    }

    return -1;
}

bool Merger::sendContinuousForSource (GenericProcessor* sourceNode)
{
    if (sourceNode == sourceNodeA)
    {
        return mergeContinuousA;
    }
    else if (sourceNode == sourceNodeB)
    {
        return mergeContinuousB;
    }

    return false;
}

bool Merger::sendEventsForSource (GenericProcessor* sourceNode)
{
    if (sourceNode == sourceNodeA)
    {
        return mergeEventsA;
    }
    else if (sourceNode == sourceNodeB)
    {
        return mergeEventsB;
    }

    return false;
}

bool Merger::stillHasSource() const
{
    if (sourceNodeA == 0 || sourceNodeB == 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Merger::switchIO()
{
    LOGDD ("Merger switching source. New path: ", 1 - activePath);

    if (activePath == 0)
    {
        activePath = 1;
        sourceNode = sourceNodeB;
    }
    else
    {
        activePath = 0;
        sourceNode = sourceNodeA;
    }
}

void Merger::lostInput()
{
    // if (sourceNodeA == nullptr && sourceNodeB != nullptr)
    // {
    //     // sourceNodeA = sourceNodeB;
    //     // sourceNodeB = nullptr;

    //     MergerEditor* ed = (MergerEditor*)getEditor();
    //     ed->switchSource(1);
    // }
    // else if (sourceNodeB == nullptr && sourceNodeA != nullptr)
    // {
    //     MergerEditor* ed = (MergerEditor*)getEditor();
    //     ed->switchSource(0);
    // }
}

GenericProcessor* Merger::getSourceNode (int path)
{
    if (path == 0)
    {
        return sourceNodeA;
    }
    else
    {
        return sourceNodeB;
    }
}

int Merger::addSettingsFromSourceNode (GenericProcessor* sn, int continuousChannelGlobalIndex)
{
    for (auto stream : sn->getStreamsForDestNode (this))
    {
        continuousChannelGlobalIndex = copyDataStreamSettings (stream, continuousChannelGlobalIndex);
    }

    for (int i = 0; i < sn->getTotalConfigurationObjects(); i++)
    {
        const ConfigurationObject* sourceChan = sn->getConfigurationObject (i);
        ConfigurationObject* ch = new ConfigurationObject (*sourceChan);
        configurationObjects.add (ch);
    }

    return continuousChannelGlobalIndex;
}

void Merger::updateSettings()
{
    isEnabled = true;

    int continuousChannelGlobalIndex = 0;

    messageChannel.reset();

    if (sourceNodeA != nullptr)
    {
        if (sourceNodeA->isMerger())
        {
            isEnabled = false;
            messageChannel = std::make_unique<EventChannel> (*AccessClass::getMessageCenter()->getMessageChannel());
            messageChannel->addProcessor (this);
            return;
        }

        LOGD ("   Merger source A found.");
        continuousChannelGlobalIndex = addSettingsFromSourceNode (sourceNodeA, continuousChannelGlobalIndex);
        isEnabled &= sourceNodeA->isEnabled;

        if (sourceNodeA->getMessageChannel() != nullptr)
        {
            messageChannel = std::make_unique<EventChannel> (*sourceNodeA->getMessageChannel());
            messageChannel->addProcessor (this);
        }
    }
    else
    {
        mergeEventsA = true;
        mergeContinuousA = true;
    }

    if (sourceNodeB != nullptr)
    {
        if (sourceNodeB->isMerger())
        {
            isEnabled = false;

            if (messageChannel == nullptr)
            {
                messageChannel = std::make_unique<EventChannel> (*AccessClass::getMessageCenter()->getMessageChannel());
                messageChannel->addProcessor (this);
            }
            return;
        }

        LOGD ("   Merger source B found.");
        continuousChannelGlobalIndex = addSettingsFromSourceNode (sourceNodeB, continuousChannelGlobalIndex);
        isEnabled &= sourceNodeB->isEnabled;

        if (messageChannel == nullptr && sourceNodeB->getMessageChannel() != nullptr)
        {
            messageChannel = std::make_unique<EventChannel> (*sourceNodeB->getMessageChannel());
            messageChannel->addProcessor (this);
        }
    }
    else
    {
        mergeEventsB = true;
        mergeContinuousB = true;
    }

    if (sourceNodeA == nullptr && sourceNodeB == nullptr)
        isEnabled = false;

    if (messageChannel == nullptr)
    {
        messageChannel = std::make_unique<EventChannel> (*AccessClass::getMessageCenter()->getMessageChannel());
        messageChannel->addProcessor (this);
    }

    LOGD ("Number of merger outputs: ", getNumInputs());
}

void Merger::saveCustomParametersToXml (XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement ("MERGER");

    if (sourceNodeA != nullptr)
        mainNode->setAttribute ("NodeA", sourceNodeA->getNodeId());
    else
        mainNode->setAttribute ("NodeA", -1);

    if (sourceNodeB != nullptr)
        mainNode->setAttribute ("NodeB", sourceNodeB->getNodeId());
    else
        mainNode->setAttribute ("NodeB", -1);

    mainNode->setAttribute ("activePath", activePath);
}

void Merger::loadCustomParametersFromXml (XmlElement* xml)
{
    if (! headlessMode)
    {
        MergerEditor* me = (MergerEditor*) getEditor();

        me->switchSource (xml->getIntAttribute ("activePath", 0));
    }
}

void Merger::restoreConnections()
{
    LOGDD ("Restoring Merger (", getNodeId(), ") connections.");

    if (parametersAsXml != nullptr)
    {
        for (auto* mainNode : parametersAsXml->getChildIterator())
        {
            if (mainNode->hasTagName ("CUSTOM_PARAMETERS"))
            {
                for (auto* mergerSettings : mainNode->getChildIterator())
                {
                    int nodeIdA = mergerSettings->getIntAttribute ("NodeA");
                    int nodeIdB = mergerSettings->getIntAttribute ("NodeB");

                    ProcessorGraph* graph = AccessClass::getProcessorGraph();
                    Array<GenericProcessor*> p = graph->getListOfProcessors();

                    for (int k = 0; k < p.size(); k++)
                    {
                        if (p[k]->getNodeId() == nodeIdA)
                        {
                            LOGDD ("Setting Merger source A to ", nodeIdA);
                            graph->connectMergerSource (this, p[k], 0);
                        }
                        else if (p[k]->getNodeId() == nodeIdB)
                        {
                            LOGDD ("Setting Merger source B to ", nodeIdB);
                            graph->connectMergerSource (this, p[k], 1);
                        }
                    }

                    if (editor != nullptr)
                        editor->switchSource (mergerSettings->getIntAttribute ("activePath", 0));
                }
            }
        }
    }
}
