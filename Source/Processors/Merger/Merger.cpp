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


Merger::Merger()
    : GenericProcessor("Merger"),
      mergeEventsA(true), mergeContinuousA(true),
      mergeEventsB(true), mergeContinuousB(true),
      sourceNodeA(0), sourceNodeB(0), activePath(0)
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

    //std::cout << "Creating editor." << std::endl;
    return editor;
}

void Merger::setMergerSourceNode(GenericProcessor* sn)
{

    sourceNode = sn;

    if (activePath == 0)
    {
        std::cout << "Setting source node A." << std::endl;
        sourceNodeA = sn;
    }
    else
    {
        std::cout << "Setting source node B." << std::endl;
        sourceNodeB = sn;
    }

    if (sn != nullptr)
    {
        sn->setDestNode(this);
    }
}

void Merger::switchIO(int sourceNum)
{

    //std::cout << "Switching to source number " << sourceNum << std::endl;

    activePath = sourceNum;

    if (sourceNum == 0)
    {
        sourceNode = sourceNodeA;
        //std::cout << "Source node: " << getSourceNode() << std::endl;
    }
    else
    {
        sourceNode = sourceNodeB;
        //std::cout << "Source node: " << getSourceNode() << std::endl;
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

    //std::cout << "Merger switching source." << std::endl;

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

    if (sourceNodeA != 0)
    {
        std::cout << "   Merger source A found." << std::endl;
        addSettingsFromSourceNode(sourceNodeA);
    } else {
        mergeEventsA = true;
        mergeContinuousA = true;
    }

    if (sourceNodeB != 0)
    {
        std::cout << "   Merger source B found." << std::endl;
        addSettingsFromSourceNode(sourceNodeB);
    } else {
        mergeEventsB = true;
        mergeContinuousB = true;
    }

    if (sourceNodeA == 0 && sourceNodeB == 0)
    {
		settings.numOutputs = getNumOutputs();
    }

    std::cout << "Number of merger outputs: " << getNumInputs() << std::endl;

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
    if (1)
    {
        if (parametersAsXml != nullptr)
        {
            forEachXmlChildElement(*parametersAsXml, mainNode)
            {
                if (mainNode->hasTagName("MERGER"))
                {
                    int NodeAid = mainNode->getIntAttribute("NodeA");
                    int NodeBid = mainNode->getIntAttribute("NodeB");

					ProcessorGraph* gr = AccessClass::getProcessorGraph();
                    Array<GenericProcessor*> p = gr->getListOfProcessors();

                    for (int k = 0; k < p.size(); k++)
                    {
                        if (p[k]->getNodeId() == NodeAid)
                        {
                            std::cout << "Setting Merger source A to " << NodeAid << std::endl;
                            switchIO(0);
                            setMergerSourceNode(p[k]);
                        }
                        if (p[k]->getNodeId() == NodeBid)
                        {
                            std::cout << "Setting Merger source B to " << NodeBid << std::endl;
                            switchIO(1);
                            setMergerSourceNode(p[k]);
                        }
                    }

                    mergeEventsA = mainNode->getBoolAttribute("MergeEventsA");
                    mergeEventsB = mainNode->getBoolAttribute("MergeEventsB");
                    mergeContinuousA = mainNode->getBoolAttribute("MergeContinuousA");
                    mergeContinuousB = mainNode->getBoolAttribute("MergeContinuousB");

                    updateSettings();
                }
            }
        }
    }
}

// void Merger::setNumOutputs(int /*outputs*/)
// {
// 	numOutputs = 0;

// 	if (sourceNodeA != 0)
// 	{
// 		std::cout << "   Merger source A found." << std::endl;
// 		numOutputs += sourceNodeA->getNumOutputs();
// 	}
// 	if (sourceNodeB != 0)
// 	{
// 		std::cout << "   Merger source B found." << std::endl;
// 		numOutputs += sourceNodeB->getNumOutputs();
// 	}

// 	std::cout << "Number of merger outputs: " << getNumOutputs() << std::endl;

// }

// void Merger::tabNumber(int t)
// {
// 	if (tabA == -1)
// 		tabA = t;
// 	else
// 		tabB = t;

// }
