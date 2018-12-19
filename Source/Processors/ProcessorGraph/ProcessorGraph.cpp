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

#include <stdio.h>

#include "ProcessorGraph.h"
#include "../GenericProcessor/GenericProcessor.h"

#include "../AudioNode/AudioNode.h"
#include "../RecordNode/RecordNode.h"
#include "../MessageCenter/MessageCenter.h"
#include "../Merger/Merger.h"
#include "../Splitter/Splitter.h"
#include "../../UI/UIComponent.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/TimestampSourceSelection.h"

#include "../ProcessorManager/ProcessorManager.h"
    
ProcessorGraph::ProcessorGraph() : currentNodeId(100)
{

    // The ProcessorGraph will always have 0 inputs (all content is generated within graph)
    // but it will have N outputs, where N is the number of channels for the audio monitor
    setPlayConfigDetails(0, // number of inputs
                         2, // number of outputs
                         44100.0, // sampleRate
                         1024);    // blockSize

}

ProcessorGraph::~ProcessorGraph()
{

}

void ProcessorGraph::createDefaultNodes()
{

    // add output node -- sends output to the audio card
    AudioProcessorGraph::AudioGraphIOProcessor* on =
        new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

    // add record node -- sends output to disk
    RecordNode* recn = new RecordNode();
    recn->setNodeId(RECORD_NODE_ID);

    // add audio node -- takes all inputs and selects those to be used for audio monitoring
    AudioNode* an = new AudioNode();
    an->setNodeId(AUDIO_NODE_ID);

    // add message center
    MessageCenter* msgCenter = new MessageCenter();
    msgCenter->setNodeId(MESSAGE_CENTER_ID);

    addNode(on, OUTPUT_NODE_ID);
    addNode(recn, RECORD_NODE_ID);
    addNode(an, AUDIO_NODE_ID);
    addNode(msgCenter, MESSAGE_CENTER_ID);

}

void ProcessorGraph::updatePointers()
{
    getAudioNode()->updateBufferSize();
}

void* ProcessorGraph::createNewProcessor(Array<var>& description, int id)//,
{
	GenericProcessor* processor = 0;
	try {// Try/catch block added by Michael Borisov
		processor = createProcessorFromDescription(description);
	}
	catch (std::exception& e) {
		NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "OpenEphys", e.what());
	}

	// int id = currentNodeId++;

	if (processor != 0)
	{
		processor->setNodeId(id); // identifier within processor graph
		std::cout << "  Adding node to graph with ID number " << id << std::endl;
		std::cout << std::endl;
		std::cout << std::endl;
		addNode(processor,id); // have to add it so it can be deleted by the graph

		if (processor->isSource())
		{
			// by default, all source nodes record automatically
			processor->setAllChannelsToRecord();
			if (processor->isGeneratesTimestamps())
			{ //If there are no source processors and we add one, set it as default for global timestamps and samplerates
				m_validTimestampSources.add(processor);
				if (m_timestampSource == nullptr)
				{
					m_timestampSource = processor;
					m_timestampSourceSubIdx = 0;
				}
				if (m_timestampWindow)
					m_timestampWindow->updateProcessorList();
			}
		}
		return processor->createEditor();
	}
	else
	{
		CoreServices::sendStatusMessage("Not a valid processor type.");
		return 0;
	}
}

void ProcessorGraph::clearSignalChain()
{

    Array<GenericProcessor*> processors = getListOfProcessors();

    for (int i = 0; i < processors.size(); i++)
    {
        removeProcessor(processors[i]);
    }

}

void ProcessorGraph::changeListenerCallback(ChangeBroadcaster* source)
{
    refreshColors();

}

void ProcessorGraph::refreshColors()
{
    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        int nodeId = node->nodeId;

        if (nodeId != OUTPUT_NODE_ID &&
            nodeId != AUDIO_NODE_ID &&
            nodeId != RECORD_NODE_ID &&
            nodeId != MESSAGE_CENTER_ID)
        {
            GenericProcessor* p =(GenericProcessor*) node->getProcessor();
            GenericEditor* e = (GenericEditor*) p->getEditor();
            e->refreshColors();
        }
    }
}

void ProcessorGraph::restoreParameters()
{

    std::cout << "Restoring parameters for each processor..." << std::endl;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        int nodeId = node->nodeId;

        if (nodeId != OUTPUT_NODE_ID &&
            nodeId != AUDIO_NODE_ID &&
            nodeId != RECORD_NODE_ID &&
            nodeId != MESSAGE_CENTER_ID)
        {
            GenericProcessor* p =(GenericProcessor*) node->getProcessor();
            p->loadFromXml();
        }
    }

}

Array<GenericProcessor*> ProcessorGraph::getListOfProcessors()
{

    Array<GenericProcessor*> a;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        int nodeId = node->nodeId;

        if (nodeId != OUTPUT_NODE_ID &&
            nodeId != AUDIO_NODE_ID &&
            nodeId != RECORD_NODE_ID &&
            nodeId != MESSAGE_CENTER_ID)
        {
            GenericProcessor* p =(GenericProcessor*) node->getProcessor();
            a.add(p);
        }
    }

    return a;

}

void ProcessorGraph::clearConnections()
{

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);
        int nodeId = node->nodeId;

        if (nodeId != OUTPUT_NODE_ID)
        {

            if (nodeId != RECORD_NODE_ID && nodeId != AUDIO_NODE_ID)
            {
                disconnectNode(node->nodeId);
            }

            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            p->resetConnections();

        }
    }

    // connect audio subnetwork
    for (int n = 0; n < 2; n++)
    {

        addConnection(AUDIO_NODE_ID, n,
                      OUTPUT_NODE_ID, n);

    }

    addConnection(MESSAGE_CENTER_ID, midiChannelIndex,
                  RECORD_NODE_ID, midiChannelIndex);
}


void ProcessorGraph::updateConnections(Array<SignalChainTabButton*, CriticalSection> tabs)
{
    clearConnections(); // clear processor graph

    std::cout << "Updating connections:" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    Array<GenericProcessor*> splitters;
    // GenericProcessor* activeSplitter = nullptr;

    for (int n = 0; n < tabs.size(); n++) // cycle through the tabs
    {
        std::cout << "Signal chain: " << n << std::endl;
        std::cout << std::endl;

        GenericEditor* sourceEditor = (GenericEditor*) tabs[n]->getEditor();
        GenericProcessor* source = (GenericProcessor*) sourceEditor->getProcessor();

        while (source != nullptr)// && destEditor->isEnabled())
        {
            std::cout << "Source node: " << source->getName() << "." << std::endl;
            GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

            if (source->isEnabledState())
            {
                // add the connections to audio and record nodes if necessary
                if (!(source->isSink()     ||
                      source->isSplitter() ||
                      source->isMerger()   ||
                      source->isUtility())
                    && !(source->wasConnected))
                {
                    std::cout << "     Connecting to audio and record nodes." << std::endl;
                    connectProcessorToAudioAndRecordNodes(source);
                }
                else
                {
                    std::cout << "     NOT connecting to audio and record nodes." << std::endl;
                }

                if (dest != nullptr)
                {

                    while (dest->isMerger()) // find the next dest that's not a merger
                    {
                        dest = dest->getDestNode();

                        if (dest == nullptr)
                            break;
                    }

                    if (dest != nullptr)
                    {
                        while (dest->isSplitter())
                        {
                            if (!dest->wasConnected)
                            {
                                if (!splitters.contains(dest))
                                {
                                    splitters.add(dest);
                                    dest->switchIO(0); // go down first path
                                }
                                else
                                {
                                    int splitterIndex = splitters.indexOf(dest);
                                    splitters.remove(splitterIndex);
                                    dest->switchIO(1); // go down second path
                                    dest->wasConnected = true; // make sure we don't re-use this splitter
                                }
                            }

                            dest = dest->getDestNode();

                            if (dest == nullptr)
                                break;
                        }

                        if (dest != nullptr)
                        {

                            if (dest->isEnabledState())
                            {
                                connectProcessors(source, dest);
                            }
                        }

                    }
                    else
                    {
                        std::cout << "     No dest node." << std::endl;
                    }

                }
                else
                {
                    std::cout << "     No dest node." << std::endl;
                }
            }

            std::cout << std::endl;

            source->wasConnected = true;
            source = dest; // switch source and dest

            if (source == nullptr && splitters.size() > 0)
            {

                source = splitters.getLast();
                GenericProcessor* newSource;// = source->getSourceNode();

                while (source->isSplitter() || source->isMerger())
                {
                    newSource = source->getSourceNode();
                    newSource->setPathToProcessor(source);
                    source = newSource;
                }

            }

        } // end while source != 0
    } // end "tabs" for loop
	
	getAudioNode()->updatePlaybackBuffer();
	//Update RecordNode internal channel mappings
	Array<EventChannel*> extraChannels;
	getMessageCenter()->addSpecialProcessorChannels(extraChannels);
	getRecordNode()->addSpecialProcessorChannels(extraChannels);
} // end method

void ProcessorGraph::connectProcessors(GenericProcessor* source, GenericProcessor* dest)
{

    if (source == nullptr || dest == nullptr)
        return;

    std::cout << "     Connecting " << source->getName() << " " << source->getNodeId(); //" channel ";
    std::cout << " to " << dest->getName() << " " << dest->getNodeId() << std::endl;

    bool connectContinuous = true;
    bool connectEvents = true;

    if (source->getDestNode() != nullptr)
    {
        if (source->getDestNode()->isMerger())
        {
            Merger* merger = (Merger*) source->getDestNode();
            connectContinuous = merger->sendContinuousForSource(source);
            connectEvents = merger->sendEventsForSource(source);
        }
    }

    // 1. connect continuous channels
    if (connectContinuous)
    {
        for (int chan = 0; chan < source->getNumOutputs(); chan++)
        {
            //std::cout << chan << " ";

            addConnection(source->getNodeId(),         // sourceNodeID
                          chan,                        // sourceNodeChannelIndex
                          dest->getNodeId(),           // destNodeID
                          dest->getNextChannel(true)); // destNodeChannelIndex
        }
    }

    // 2. connect event channel
    if (connectEvents)
    {
        addConnection(source->getNodeId(),    // sourceNodeID
                      midiChannelIndex,       // sourceNodeChannelIndex
                      dest->getNodeId(),      // destNodeID
                      midiChannelIndex);      // destNodeChannelIndex
    }

}

void ProcessorGraph::connectProcessorToAudioAndRecordNodes(GenericProcessor* source)
{

    if (source == nullptr)
        return;

    getRecordNode()->registerProcessor(source);

    for (int chan = 0; chan < source->getNumOutputs(); chan++)
    {

        getAudioNode()->addInputChannel(source, chan);

        // THIS IS A HACK TO MAKE SURE AUDIO NODE KNOWS WHAT THE SAMPLE RATE SHOULD BE
        // IT CAN CAUSE PROBLEMS IF THE SAMPLE RATE VARIES ACROSS PROCESSORS

		//TODO: See if this causes problems with the newer architectures
        //getAudioNode()->settings.sampleRate = source->getSampleRate();

        addConnection(source->getNodeId(),                   // sourceNodeID
                      chan,                                  // sourceNodeChannelIndex
                      AUDIO_NODE_ID,                         // destNodeID
                      getAudioNode()->getNextChannel(true)); // destNodeChannelIndex

        getRecordNode()->addInputChannel(source, chan);

        addConnection(source->getNodeId(),                    // sourceNodeID
                      chan,                                   // sourceNodeChannelIndex
                      RECORD_NODE_ID,                         // destNodeID
                      getRecordNode()->getNextChannel(true)); // destNodeChannelIndex

    }

    // connect event channel
    addConnection(source->getNodeId(),    // sourceNodeID
                  midiChannelIndex,       // sourceNodeChannelIndex
                  RECORD_NODE_ID,         // destNodeID
                  midiChannelIndex);      // destNodeChannelIndex

    // connect event channel
    addConnection(source->getNodeId(),    // sourceNodeID
                  midiChannelIndex,       // sourceNodeChannelIndex
                  AUDIO_NODE_ID,          // destNodeID
                  midiChannelIndex);      // destNodeChannelIndex


    getRecordNode()->addInputChannel(source, midiChannelIndex);

}

GenericProcessor* ProcessorGraph::createProcessorFromDescription(Array<var>& description)
{
	GenericProcessor* processor = nullptr;

	bool fromProcessorList = description[0];
	String processorName = description[1];
	int processorType = description[2];
	int processorIndex = description[3];

	if (fromProcessorList)
	{
		String processorCategory = description[4];

		std::cout << "Creating from description..." << std::endl;
		std::cout << processorCategory << "::" << processorName << " (" << processorType << "-" << processorIndex << ")" << std::endl;

		processor = ProcessorManager::createProcessor((ProcessorClasses)processorType, processorIndex);
	}
	else
	{
		String libName = description[4];
		int libVersion = description[5];
		bool isSource = description[6];
		bool isSink = description[7];

		std::cout << "Creating from plugin info..." << std::endl;
		std::cout << libName << "(" << libVersion << ")::" << processorName << std::endl;

		processor = ProcessorManager::createProcessorFromPluginInfo((Plugin::PluginType)processorType, processorIndex, processorName, libName, libVersion, isSource, isSink);
	}
   
	String msg = "New " + processorName + " created";
	CoreServices::sendStatusMessage(msg);

    return processor;
}


bool ProcessorGraph::processorWithSameNameExists(const String& name)
{
    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        if (name.equalsIgnoreCase(node->getProcessor()->getName()))
            return true;

    }

    return false;

}


void ProcessorGraph::removeProcessor(GenericProcessor* processor)
{

    std::cout << "Removing processor with ID " << processor->getNodeId() << std::endl;

    int nodeId = processor->getNodeId();

    disconnectNode(nodeId);
    removeNode(nodeId);

	if (processor->isSource())
	{
		m_validTimestampSources.removeAllInstancesOf(processor);

		if (m_timestampSource == processor)
		{
			const GenericProcessor* newProc = 0;

			//Look for the next source node. If none is found, set the sourceid to 0
			for (int i = 0; i < getNumNodes() && newProc == nullptr; i++)
			{
				if (getNode(i)->nodeId != OUTPUT_NODE_ID)
				{
					GenericProcessor* p = dynamic_cast<GenericProcessor*>(getNode(i)->getProcessor());
					//GenericProcessor* p = static_cast<GenericProcessor*>(getNode(i)->getProcessor());
					if (p && p->isSource() && p->isGeneratesTimestamps())
					{
						newProc = p;
					}
				}
			}
			m_timestampSource = newProc;
			m_timestampSourceSubIdx = 0;
		}
		if (m_timestampWindow)
			m_timestampWindow->updateProcessorList();
	}

}

bool ProcessorGraph::enableProcessors()
{

    updateConnections(AccessClass::getEditorViewport()->requestSignalChain());

    std::cout << "Enabling processors..." << std::endl;

    bool allClear;

    if (getNumNodes() < 5)
    {
        AccessClass::getUIComponent()->disableCallbacks();
        return false;
    }

    for (int i = 0; i < getNumNodes(); i++)
    {

        Node* node = getNode(i);

        if (node->nodeId != OUTPUT_NODE_ID)
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            allClear = p->isReady();

            if (!allClear)
            {
                std::cout << p->getName() << " said it's not OK." << std::endl;
                //	sendActionMessage("Could not initialize acquisition.");
                AccessClass::getUIComponent()->disableCallbacks();
                return false;

            }
        }
    }

    for (int i = 0; i < getNumNodes(); i++)
    {

        Node* node = getNode(i);

        if (node->nodeId != OUTPUT_NODE_ID)
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            p->enableEditor();
            p->enableProcessor();
        }
    }

    AccessClass::getEditorViewport()->signalChainCanBeEdited(false);

	//Update special channels indexes, at the end
	//To change, as many other things, when the probe system is implemented
	getRecordNode()->updateRecordChannelIndexes();
	getAudioNode()->updateRecordChannelIndexes();

    //	sendActionMessage("Acquisition started.");
	m_startSoftTimestamp = Time::getHighResolutionTicks();
	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(true);
    return true;
}

bool ProcessorGraph::disableProcessors()
{

    std::cout << "Disabling processors..." << std::endl;

    bool allClear;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);
        if (node->nodeId != OUTPUT_NODE_ID )
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            std::cout << "Disabling " << p->getName() << std::endl;
			if (node->nodeId != MESSAGE_CENTER_ID)
				p->disableEditor();
            allClear = p->disableProcessor();

            if (!allClear)
            {
                //	sendActionMessage("Could not stop acquisition.");
                return false;
            }
        }
    }

    AccessClass::getEditorViewport()->signalChainCanBeEdited(true);
	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(false);
    //	sendActionMessage("Acquisition ended.");

    return true;
}

void ProcessorGraph::setRecordState(bool isRecording)
{

    // actually start recording
    if (isRecording)
    {
        getRecordNode()->setParameter(1,10.0f);
    }
    else
    {
        getRecordNode()->setParameter(0,10.0f);
    }

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);
        if (node->nodeId != OUTPUT_NODE_ID)
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();

            p->setRecording(isRecording);
        }
    }



}


AudioNode* ProcessorGraph::getAudioNode()
{

    Node* node = getNodeForId(AUDIO_NODE_ID);
    return (AudioNode*) node->getProcessor();

}

RecordNode* ProcessorGraph::getRecordNode()
{

    Node* node = getNodeForId(RECORD_NODE_ID);
    return (RecordNode*) node->getProcessor();

}


MessageCenter* ProcessorGraph::getMessageCenter()
{

    Node* node = getNodeForId(MESSAGE_CENTER_ID);
    return (MessageCenter*) node->getProcessor();

}


void ProcessorGraph::setTimestampSource(int sourceIndex, int subIdx)
{
	m_timestampSource = m_validTimestampSources[sourceIndex];
	if (m_timestampSource)
	{
		m_timestampSourceSubIdx = subIdx;
	}
	else
	{
		m_timestampSourceSubIdx = 0;
	}
}

void ProcessorGraph::getTimestampSources(Array<const GenericProcessor*>& validSources, int& selectedSource, int& selectedSubId) const
{
	validSources = m_validTimestampSources;
	getTimestampSources(selectedSource, selectedSubId);
}

void ProcessorGraph::getTimestampSources(int& selectedSource, int& selectedSubId) const
{
	if (m_timestampSource)
		selectedSource = m_validTimestampSources.indexOf(m_timestampSource);
	else
		selectedSource = -1;
	selectedSubId = m_timestampSourceSubIdx;
}

int64 ProcessorGraph::getGlobalTimestamp(bool softwareOnly) const
{
	if (softwareOnly || !m_timestampSource)
	{
		return (Time::getHighResolutionTicks() - m_startSoftTimestamp);
	}
	else
	{
		return static_cast<int64>((Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - m_timestampSource->getLastProcessedsoftwareTime())
			* m_timestampSource->getSampleRate(m_timestampSourceSubIdx)) + m_timestampSource->getSourceTimestamp(m_timestampSource->getNodeId(), m_timestampSourceSubIdx));
	}
}

float ProcessorGraph::getGlobalSampleRate(bool softwareOnly) const
{
	if (softwareOnly || !m_timestampSource)
	{
		return Time::getHighResolutionTicksPerSecond();
	}
	else
	{
		return m_timestampSource->getSampleRate(m_timestampSourceSubIdx);
	}
}

void ProcessorGraph::setTimestampWindow(TimestampSourceSelectionWindow* window)
{
	m_timestampWindow = window;
}