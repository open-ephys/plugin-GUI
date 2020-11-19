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


Merger::Merger()
    : GenericProcessor("Merger"),
      mergeEventsA(true), mergeContinuousA(true),
      mergeEventsB(true), mergeContinuousB(true),
      sourceNodeA(nullptr), sourceNodeB(nullptr), activePath(0)
{
    setProcessorType(PROCESSOR_TYPE_MERGER);
    sendSampleCount = false;
}

Merger::~Merger()
{

}

AudioProcessorEditor* Merger::createEditor()
{
    editor = new MergerEditor(this, true);
    //tEditor(editor);

    LOGDD("Creating editor.");
    return editor;
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

    //if (sn != nullptr)
    //{
    //    sn->setDestNode(this);
    //}
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

void Merger::addSettingsFromSourceNode(GenericProcessor* sn)
{

    if (sendContinuousForSource(sn))
    {
        settings.numInputs += sn->getNumOutputs();

        for (int i = 0; i < sn->getTotalDataChannels(); i++)
        {
            const DataChannel* sourceChan = sn->getDataChannel(i);
            DataChannel* ch = new DataChannel(*sourceChan);
            dataChannelArray.add(ch);
        
        }
    }

    if (sendEventsForSource(sn))
    {
        for (int i = 0; i < sn->getTotalEventChannels(); i++)
        {
            const EventChannel* sourceChan = sn->getEventChannel(i);
            EventChannel* ch = new EventChannel(*sourceChan);
            eventChannelArray.add(ch);
        }
		for (int i = 0; i < sn->getTotalSpikeChannels(); i++)
		{
			const SpikeChannel* sourceChan = sn->getSpikeChannel(i);
			SpikeChannel* ch = new SpikeChannel(*sourceChan);
			spikeChannelArray.add(ch);
		}
    }
	for (int i = 0; i < sn->getTotalConfigurationObjects(); i++)
	{
		const ConfigurationObject* sourceChan = sn->getConfigurationObject(i);
		ConfigurationObject* ch = new ConfigurationObject(*sourceChan);
		configurationObjectArray.add(ch);
	}

    settings.originalSource = sn->settings.originalSource;

    settings.numOutputs = settings.numInputs;

}

void Merger::updateSettings()
{

    // default is to get everything from sourceNodeA,
    // but this might not be ideal
    clearSettings();
    
    isEnabled = true;

    if (sourceNodeA != nullptr)
    {
        LOGD("   Merger source A found.");
        addSettingsFromSourceNode(sourceNodeA);
        isEnabled &= sourceNodeA->isEnabled;
    } else {
        mergeEventsA = true;
        mergeContinuousA = true;
    }

    if (sourceNodeB != nullptr)
    {
        LOGD("   Merger source B found.");
        addSettingsFromSourceNode(sourceNodeB);
        isEnabled &= sourceNodeB->isEnabled;
    } else {
        mergeEventsB = true;
        mergeContinuousB = true;
    }

    if (sourceNodeA == 0 && sourceNodeB == 0)
    {
		settings.numOutputs = getNumOutputs();
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

void Merger::loadCustomParametersFromXml()
{
    
}

void Merger::restoreConnections()
{
    
    if (parametersAsXml != nullptr)
    {
        forEachXmlChildElement(*parametersAsXml, mainNode)
        {
            if (mainNode->hasTagName("MERGER"))
            {
               int nodeIdA = mainNode->getIntAttribute("NodeA");
               int nodeIdB = mainNode->getIntAttribute("NodeB");

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

                mergeEventsA = mainNode->getBoolAttribute("MergeEventsA");
                mergeEventsB = mainNode->getBoolAttribute("MergeEventsB");
                mergeContinuousA = mainNode->getBoolAttribute("MergeContinuousA");
                mergeContinuousB = mainNode->getBoolAttribute("MergeContinuousB");
            }
        }
    }
}
