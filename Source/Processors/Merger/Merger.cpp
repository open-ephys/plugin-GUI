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

#include "Merger.h"
#include "MergerEditor.h"

#include "../../UI/EditorViewport.h"
#include "../../AccessClass.h"
#include "../../Utils/Utils.h"

#include "../Settings/ConfigurationObject.h"

#include "../MessageCenter/MessageCenterEditor.h"


Merger::Merger()
    : GenericProcessor("Merger"),
      mergeEventsA(true), mergeContinuousA(true),
      mergeEventsB(true), mergeContinuousB(true),
      sourceNodeA(nullptr), sourceNodeB(nullptr), activePath(0)
{
    sendSampleCount = false;
}

Merger::~Merger()
{

}

AudioProcessorEditor* Merger::createEditor()
{
    editor = std::make_unique<MergerEditor>(this);
    return editor.get();
}

void Merger::setMergerSourceNode(GenericProcessor* sn)
{

    sourceNode = sn;

    if (activePath == 0)
    {
    LOGD("Setting source node A.");
        sourceNodeA = sn;
    }
    else
    {
    LOGD("Setting source node B.");
        sourceNodeB = sn;
    }

}

void Merger::switchIO(int sourceNum)
{

//LOGDD("Switching to source number ", sourceNum);

    activePath = sourceNum;

    if (sourceNum == 0)
    {
        sourceNode = sourceNodeA;
//LOGDD("Source node: ", getSourceNode());
    }
    else
    {
        sourceNode = sourceNodeB;
//LOGDD("Source node: ", getSourceNode());
    }

    // getEditorViewport()->makeEditorVisible((GenericEditor*) getEditor(), false);

}

int Merger::switchToSourceNode(GenericProcessor* sn)
{
    if (sn == sourceNodeA)
    {
        switchIO(0);
        return 0;
    }

    if (sn == sourceNodeB)
    {
        switchIO(1);
        return 1;
    }

    return -1;
}

bool Merger::sendContinuousForSource(GenericProcessor* sourceNode)
{
    if (sourceNode == sourceNodeA)
    {
        return mergeContinuousA;
    } else if (sourceNode == sourceNodeB)
    {
        return mergeContinuousB;
    }

    return false;
}

bool Merger::sendEventsForSource(GenericProcessor* sourceNode)
{
    if (sourceNode == sourceNodeA)
    {
        return mergeEventsA;
    } else if (sourceNode == sourceNodeB)
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

    LOGDD("Merger switching source.");

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

GenericProcessor* Merger::getSourceNode(int path)
{
    if (path == 0)
    {
        return sourceNodeA;
    } else {
        return sourceNodeB;
    }
}

int Merger::addSettingsFromSourceNode(GenericProcessor* sn, int continuousChannelGlobalIndex)
{

    for (auto stream : sn->getStreamsForDestNode(this))
    {
        if (checkStream(stream))
            continuousChannelGlobalIndex = copyDataStreamSettings(stream, continuousChannelGlobalIndex);
    }

    for (int i = 0; i < sn->getTotalConfigurationObjects(); i++)
    {
        const ConfigurationObject* sourceChan = sn->getConfigurationObject(i);
        ConfigurationObject* ch = new ConfigurationObject(*sourceChan);
        configurationObjects.add(ch);
    }
    
    return continuousChannelGlobalIndex;
}

bool Merger::checkStream(const DataStream* stream)
{
    MergerEditor* ed = (MergerEditor*)getEditor();

    return true; // ed->checkStream(stream);
}


void Merger::updateSettings()
{
    
    isEnabled = true;
    
    int continuousChannelGlobalIndex = 0;
    
    messageChannel.reset();

    if (sourceNodeA != nullptr)
    {
        LOGD("   Merger source A found.");
        continuousChannelGlobalIndex = addSettingsFromSourceNode(sourceNodeA, continuousChannelGlobalIndex);
        isEnabled &= sourceNodeA->isEnabled;
        messageChannel = std::make_unique<EventChannel>(*sourceNodeA->getMessageChannel());
        messageChannel->addProcessor(processorInfo.get());
        
    } else {
        mergeEventsA = true;
        mergeContinuousA = true;
    }

    if (sourceNodeB != nullptr)
    {
        LOGD("   Merger source B found.");
        continuousChannelGlobalIndex = addSettingsFromSourceNode(sourceNodeB, continuousChannelGlobalIndex);
        isEnabled &= sourceNodeB->isEnabled;
        
        if (messageChannel == nullptr)
        {
            messageChannel = std::make_unique<EventChannel>(*sourceNode->getMessageChannel());
            messageChannel->addProcessor(processorInfo.get());
        }
    } else {
        mergeEventsB = true;
        mergeContinuousB = true;
    }
    
    if (messageChannel == nullptr)
    {
        messageChannel = std::make_unique<EventChannel>(*AccessClass::getMessageCenter()->messageCenter->getMessageChannel());
        messageChannel->addProcessor(processorInfo.get());
    }

    LOGD("Number of merger outputs: ", getNumInputs());

}

void Merger::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("MERGER");
    if (sourceNodeA!= nullptr)
        mainNode->setAttribute("NodeA",	sourceNodeA->getNodeId());
    else
        mainNode->setAttribute("NodeA",	-1);

    if (sourceNodeB != nullptr)
        mainNode->setAttribute("NodeB",	sourceNodeB->getNodeId());
    else
        mainNode->setAttribute("NodeB",	-1);

    mainNode->setAttribute("MergeEventsA", mergeEventsA);
    mainNode->setAttribute("MergeContinuousA", mergeContinuousA);
    mainNode->setAttribute("MergeEventsB", mergeEventsB);
    mainNode->setAttribute("MergeContinuousB", mergeContinuousB);
}

void Merger::loadCustomParametersFromXml(XmlElement* xml)
{
    
}

void Merger::restoreConnections()
{
    
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("CUSTOM_PARAMETERS"))
            {
                forEachXmlChildElement(*mainNode, mergerSettings)
                {
                   int nodeIdA = mergerSettings->getIntAttribute("NodeA");
                   int nodeIdB = mergerSettings->getIntAttribute("NodeB");

                   ProcessorGraph* gr = AccessClass::getProcessorGraph();
                   Array<GenericProcessor*> p = gr->getListOfProcessors();

                   for (int k = 0; k < p.size(); k++)
                   {
                       if (p[k]->getNodeId() == nodeIdA)
                       {
                          LOGD("Setting Merger source A to ", nodeIdA);
                          switchIO(0);
                          setMergerSourceNode(p[k]);
                          p[k]->setDestNode(this);
                          editor->switchSource(0);
                       }
                       else if (p[k]->getNodeId() == nodeIdB)
                        {
                            LOGD("Setting Merger source B to ", nodeIdB);
                            switchIO(1);
                            setMergerSourceNode(p[k]);
                            p[k]->setDestNode(this);
                            editor->switchSource(1);
                        }
                    }

                    mergeEventsA = mergerSettings->getBoolAttribute("MergeEventsA");
                    mergeEventsB = mergerSettings->getBoolAttribute("MergeEventsB");
                    mergeContinuousA = mergerSettings->getBoolAttribute("MergeContinuousA");
                    mergeContinuousB = mergerSettings->getBoolAttribute("MergeContinuousB");
                }
            }
        }
    }
}
