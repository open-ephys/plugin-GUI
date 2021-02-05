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
#include <utility>
#include <vector>
#include <map>

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
#include "../../UI/GraphViewer.h"

#include "../ProcessorManager/ProcessorManager.h"

ProcessorGraph::ProcessorGraph() : currentNodeId(100), isLoadingSignalChain(false)
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
    //RecordNode* recn = new RecordNode();
    //recn->setNodeId(RECORD_NODE_ID);

    // add audio node -- takes all inputs and selects those to be used for audio monitoring
    AudioNode* an = new AudioNode();
    an->setNodeId(AUDIO_NODE_ID);

    // add message center
    MessageCenter* msgCenter = new MessageCenter();
    msgCenter->setNodeId(MESSAGE_CENTER_ID);

    addNode(on, OUTPUT_NODE_ID);
    //addNode(recn, RECORD_NODE_ID);
    addNode(an, AUDIO_NODE_ID);
    addNode(msgCenter, MESSAGE_CENTER_ID);

}

void ProcessorGraph::updatePointers()
{
    getAudioNode()->updateBufferSize();
}

void ProcessorGraph::moveProcessor(GenericProcessor* processor,
                                    GenericProcessor* newSource,
                                   GenericProcessor* newDest,
                                   bool moveDownstream)
{
    GenericProcessor* originalSource = processor->getSourceNode();
    GenericProcessor* originalDest = processor->getDestNode();
    
    if (originalSource != nullptr)
    {
        originalSource->setDestNode(originalDest);
    }
        
    if (originalDest != nullptr)
    {
        originalDest->setSourceNode(originalSource);
    }
    
    LOGD("Processor to move: ", processor->getName());
    if (originalSource != nullptr)
    LOGD("Original source: ", originalSource->getName());
    if (originalDest != nullptr)
    LOGD("Original dest: ", originalDest->getName());
    if (newSource != nullptr)
    LOGD("New source: ", newSource->getName());
    if (newDest != nullptr)
    LOGD("New dest: ", newDest->getName());
    
    processor->setSourceNode(nullptr);
    processor->setDestNode(nullptr);
    
    if (newSource != nullptr)
    {
        if (!processor->isSource())
        {
            processor->setSourceNode(newSource);
            newSource->setDestNode(processor);
        } else {
            processor->setSourceNode(nullptr);
            newSource->setDestNode(nullptr);
            //rootNodes.add(newSource);
        }
        
    }
    
    if (newDest != nullptr)
    {
        if (!newDest->isSource())
        {
            processor->setDestNode(newDest);
            newDest->setSourceNode(processor);
        } else {
            processor->setDestNode(nullptr);
        }
    }
    
    checkForNewRootNodes(processor, false, true);
    
    if (moveDownstream) // processor is further down the signal chain, its original dest may have changed
        updateSettings(originalDest);
    else // processor is upstream of its original dest, so we can just update that
        updateSettings(processor);
}

GenericProcessor* ProcessorGraph::createProcessor(ProcessorDescription& description,
                                      GenericProcessor* sourceNode,
                                      GenericProcessor* destNode,
                                      bool signalChainIsLoading)
{
	GenericProcessor* processor = nullptr;
    
    LOGD("Creating processor with name: ", description.processorName);
    
    if (sourceNode != nullptr)
        LOGD("Source node: ", sourceNode->getName());
    //else
     //   LOGD("No source node.");
    
    if (destNode != nullptr)
        LOGD("Dest node: ", destNode->getName());
    //else
    //    LOGD("No dest node.");
    
	try {
		processor = createProcessorFromDescription(description);
	}
	catch (std::exception& e) {
		NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "Open Ephys", e.what());
	}

	if (processor != nullptr)
	{

        int id;
        
        if (description.nodeId > 0)
        {
            id = description.nodeId;
            currentNodeId = id >= currentNodeId ? id + 1 : currentNodeId;
        } else {
            id = currentNodeId++;
        }

         // identifier within processor graph
        processor->setNodeId(id);
        addNode(processor, id); // have to add it so it can be deleted by the graph
        GenericEditor* editor = (GenericEditor*) processor->createEditor();

        editor->refreshColors();
        
		if (processor->isSource()) // if we are adding a source processor
        {
            
            if (sourceNode != nullptr)
            {
                // if there's a source feeding into source, form a new signal chain
                processor->setDestNode(sourceNode->getDestNode());
                processor->setSourceNode(nullptr);
                sourceNode->setDestNode(nullptr);
                if (destNode != nullptr)
                    destNode->setSourceNode(processor);
            }
            
            if (sourceNode == nullptr && destNode != nullptr)
            {
                // if we're adding it upstream of another processor
                if (!destNode->isSource())
                {
                    // if it's not a source, connect them
                    processor->setDestNode(destNode);
                    destNode->setSourceNode(processor);
                }
                else
                {
                    // if it's in front of a source, start a new signal chain
                    processor->setDestNode(nullptr);
                }
            }
            
			if (processor->isGeneratesTimestamps())
			{ // If there are no source processors and we add one,
              //  set it as default for global timestamps and sample rates
				m_validTimestampSources.add(processor);
				if (m_timestampSource == nullptr)
				{
					m_timestampSource = processor;
					m_timestampSourceSubIdx = 0;
				}
				if (m_timestampWindow)
					m_timestampWindow->updateProcessorList();
			}
            
        } else {
            // a source node was not dropped
            if (sourceNode != nullptr)
            {
                // if there's a source available, connect them
                processor->setSourceNode(sourceNode);
                sourceNode->setDestNode(processor);
            }
                
            if (destNode != nullptr)
            {
                if (!destNode->isSource())
                {
                    // if it's not behind a source node, connect them
                    processor->setDestNode(destNode);
                    
                    if (!destNode->isMerger())
                        destNode->setSourceNode(processor);
                    else
                        destNode->setMergerSourceNode(processor);
                    
                } else {
                    // if there's a source downstream, start a new signalchain
                    processor->setDestNode(nullptr);
                }
            }
        }
        
        if (!checkForNewRootNodes(processor))
        {
            removeProcessor(processor);
            updateViews(rootNodes.getLast());
            return nullptr;
        }
        
	}
	else
	{
		CoreServices::sendStatusMessage("Not a valid processor.");
	}
    
    if (!signalChainIsLoading)
    {
        updateSettings(processor);
    } else {
        updateViews(processor);
    }
    
    return processor;
}

bool ProcessorGraph::checkForNewRootNodes(GenericProcessor* processor,
                                          bool processorBeingAdded, // default = true
                                          bool processorBeingMoved) // default = false
{
    
    if (processorBeingAdded) // adding processor
    {
        LOGDD("Checking ", processor->getName(), " for new root node.")
        
        if (processor->getSourceNode() == nullptr)
        {
            LOGDD("  Has no source node.");
            
            if (processor->getDestNode() != nullptr)
            {
                LOGDD("  Has a dest node.");
                
                if (rootNodes.indexOf(processor->getDestNode()) > -1)
                {
                    
                    LOGDD("  Found dest node in root nodes; swapping.");
                    
                    rootNodes.set(rootNodes.indexOf(processor->getDestNode()), processor);
                    return true;
                } else {
                    
                    LOGDD("  Didn't find dest node; adding a new root.");
                    
                    if (rootNodes.size() == 8)
                    {
                        NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
                                                              "Maximum of 8 signal chains.");
                        return false;
                    } else {
                        rootNodes.add(processor);
                        return true;
                    }
                }
                
                if (processor->getDestNode()->isMerger())
                {
                    LOGDD("  Dest node is merger; adding a new root.");
                    
                    if (rootNodes.size() == 8)
                    {
                        NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
                                                              "Maximum of 8 signal chains.");
                        return false;
                    } else {
                        rootNodes.add(processor);
                        return true;
                    }
                    
                }
            } else {
                
                LOGDD("  Has no dest node; adding.");
                
                if (rootNodes.size() == 8)
                {
                    NativeMessageBox::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
                                                          "Maximum of 8 signal chains.");
                    return false;
                    
                } else {
                    rootNodes.add(processor);
                    return true;
                }
            }
        }
        
        return true;
    }
    
    if (!processorBeingMoved) // deleting processor
    {
        if (rootNodes.indexOf(processor) > -1)
        {
            if (processor->getDestNode() != nullptr)
            {
                if (processor->getDestNode()->isMerger())
                {
                    Merger* merger = (Merger*) processor->getDestNode();
                    
                    GenericProcessor* sourceA = merger->getSourceNode(0);
                    GenericProcessor* sourceB = merger->getSourceNode(1);
                    
                    if (sourceA == nullptr && sourceB == nullptr) // no remaining source nodes
                    {
                        rootNodes.set(rootNodes.indexOf(processor),
                                      processor->getDestNode());
                    } else { // merger still has one source node
                        rootNodes.remove(rootNodes.indexOf(processor));
                    }
                    
                } else {
                    rootNodes.set(rootNodes.indexOf(processor),
                                  processor->getDestNode());
                }
            } else {
                rootNodes.remove(rootNodes.indexOf(processor));
            }
        }
        
        return true;
        
    } else { // processor being moved
        
        LOGDD("Processing being moved.");
        
        for (auto p : getListOfProcessors())
        {
            LOGDD("Checking ", p->getName());
            
            if (p->getSourceNode() == nullptr) // no source node
            {
                LOGDD("  Should be root.");
                
                if (rootNodes.indexOf(p) == -1) // not a root node yet
                {
                    if (!p->isMerger())
                    {
                        rootNodes.add(p); // add it
                        
                        LOGDD("  Adding as root.");

                    } else {
                        
                        Merger* merger = (Merger*) processor->getDestNode();
                        
                        GenericProcessor* sourceA = merger->getSourceNode(0);
                        GenericProcessor* sourceB = merger->getSourceNode(1);
                        
                        if (sourceA == nullptr && sourceB == nullptr) // no remaining source nodes
                        {
                            rootNodes.add(p); // add it
                        }
                    }
                        
                } else {
                    LOGDD("  Already is root.");
                }
            } else {
                
                LOGDD("  Should not be root.");
                
                if (rootNodes.indexOf(p) > -1) // has a source node, but is also a root node
                {
                    rootNodes.remove(rootNodes.indexOf(p)); // remove it
                    LOGDD("  Removing as root.");
                } else {
                    LOGDD("  Not a root.");
                }

            }
        }
    }
    
    return true;
    
}

void ProcessorGraph::updateSettings(GenericProcessor* processor, bool signalChainIsLoading)
{
    // prevents calls from within processors from triggering full update during loading
    if (signalChainIsLoading != isLoadingSignalChain)
    {
        //updateViews(processor);
        return;
    }
        
    GenericProcessor* processorToUpdate = processor;
    
    Array<Splitter*> splitters;
    
    while ((processor != nullptr) || (splitters.size() > 0))
    {
        if (processor != nullptr)
        {
            processor->update();
            
            if (signalChainIsLoading)
            {
                processor->loadFromXml();
                processor->update();
            }
            
            if (processor->getSourceNode() != nullptr)
            {
                if (processor->isMerger())
                {
                    processor->setEnabledState(processor->isEnabled);
                } else {
                    processor->setEnabledState(processor->getSourceNode()->isEnabledState());
                }
            }
                
            else
            {
                if (processor->isSource())
                    processor->setEnabledState(processor->isEnabledState());
                else
                    processor->setEnabledState(false);
            }
                
                
            if (processor->isSplitter())
            {
                splitters.add((Splitter*) processor);
                processor = splitters.getLast()->getDestNode(0); // travel down chain 0 first
            } else {
                processor = processor->getDestNode();
            }
        }
        else {
            Splitter* splitter = splitters.getFirst();
            processor = splitter->getDestNode(1); // then come back to chain 1
            splitters.remove(0);
        }
    }
    
    updateViews(processorToUpdate);
    
}

void ProcessorGraph::updateViews(GenericProcessor* processor)
{
    AccessClass::getGraphViewer()->updateNodes(rootNodes);
    
    int tabIndex;
    
    if (processor == nullptr && rootNodes.size() > 0)
    {
        processor = rootNodes.getFirst();
    }
    
    Array<GenericEditor*> editorArray;
    GenericProcessor* rootProcessor = processor;
    
    if (processor != nullptr)
        LOGDD("Processor to view: ", processor->getName());
    
    while (processor != nullptr)
    {
        rootProcessor = processor;
        processor = processor->getSourceNode();
        
        if (rootProcessor != nullptr)
            LOGDD("  Source: ", rootProcessor->getName());
    }
    
    processor = rootProcessor;

    while (processor != nullptr)
    {
        editorArray.add(processor->getEditor());
        
        LOGDD(" Adding ", processor->getName(), " to editor array.");
        
        if (processor->getDestNode() != nullptr)
        {
            if (processor->getDestNode()->isMerger())
            {
                if (processor->getDestNode()->getSourceNode() != processor)
                {
                    MergerEditor* editor = (MergerEditor*) processor->getDestNode()->getEditor();
                    editor->switchSource();
                }
                    
            }
        }
        
        processor = processor->getDestNode();
    }
    
    AccessClass::getEditorViewport()->updateVisibleEditors(editorArray,
                                                           rootNodes.size(),
                                                           rootNodes.indexOf(rootProcessor));
    
}

void ProcessorGraph::viewSignalChain(int index)
{
    updateViews(rootNodes[index]);
}

void ProcessorGraph::deleteNodes(Array<GenericProcessor*> processorsToDelete)
{
    GenericProcessor* sourceNode = nullptr;
    GenericProcessor* destNode = nullptr;
    
    for (auto processor : processorsToDelete)
    {
        
        sourceNode = processor->getSourceNode();
        destNode = processor->getDestNode();
        
        if (sourceNode != nullptr)
        {
            sourceNode->setDestNode(destNode);
        }
        
        if (destNode != nullptr)
        {
            destNode->setSourceNode(sourceNode);
        }
        
        //i//f (rootNodes.indexOf(processor) > -1)
        //{
        //    if (destNode != nullptr && !destNode->isMerger())
         //       rootNodes.set(rootNodes.indexOf(processor), destNode);
         //   else
        //        rootNodes.remove(rootNodes.indexOf(processor));
        //}
        
        removeProcessor(processor);
    }
    
    if (destNode != nullptr)
    {
        updateSettings(destNode);
    } else {
        updateSettings(sourceNode);
    }
        
}

void ProcessorGraph::clearSignalChain()
{

    Array<GenericProcessor*> processors = getListOfProcessors();

    for (int i = 0; i < processors.size(); i++)
    {
        removeProcessor(processors[i]);
    }
    
    rootNodes.clear();
    currentNodeId = 100;
    
    AccessClass::getGraphViewer()->removeAllNodes();

    updateViews(nullptr);
}

void ProcessorGraph::changeListenerCallback(ChangeBroadcaster* source)
{
    refreshColors();
}

void ProcessorGraph::refreshColors()
{
    for (auto p : getListOfProcessors())
    {
        GenericEditor* e = (GenericEditor*) p->getEditor();
        e->refreshColors();
    }
}

/* Set parameters based on XML.
void ProcessorGraph::loadParametersFromXml(GenericProcessor* processor)
{
    // DEPRECATED
    // Should probably do some error checking to make sure XML is valid, depending on how it treats errors (will likely just not update parameters, but error message could be nice.)
    int numberParameters = processor->getNumParameters();
    // Ditto channels. Not sure how to handle different channel sizes when variable sources (file reader etc. change). Maybe I should check number of channels vs source, but that requires hardcoding when source matters.
    //int numChannels=(targetProcessor->channels).size();
    //int numEventChannels=(targetProcessor->eventChannels).size();

    // Sets channel in for loop
    int currentChannel;

    // What the parameter name to change is.
    String parameterNameForXML;
    String parameterValue;
    float parameterFloat;

    forEachXmlChildElementWithTagName(*processor->parametersAsXml,
                                      channelXML,
                                      "CHANNEL")
    {
        currentChannel=channelXML->getIntAttribute("name");

LOGDD("currentChannel:", currentChannel);
        // Sets channel to change parameter on
        processor->setCurrentChannel(currentChannel-1);

        forEachXmlChildElement(*channelXML, parameterXML)
        {

            for (int j = 0; j < numberParameters; ++j)
            {
                parameterNameForXML = processor->getParameterName(j);

                if (parameterXML->getStringAttribute("name")==parameterNameForXML)
                {
                    parameterValue=parameterXML->getAllSubText();
                    parameterFloat=parameterValue.getFloatValue();
                    processor->setParameter(j, parameterFloat);
                    // testGrab=targetProcessor->getParameterVar(j, currentChannel);
LOGD("Channel:", currentChannel, "Parameter:", parameterNameForXML, "Intended Value:", parameterFloat);
                }

            }
        }
    }
}*/


void ProcessorGraph::restoreParameters()
{
    
    isLoadingSignalChain = true;

    LOGD("Restoring parameters for each processor...");
    
    // first connect the mergers
    for (auto p : getListOfProcessors())
    {
        if (p->isMerger())
        {
            Merger* m = (Merger*) p;
            m->restoreConnections();
        }
    }
    
    // then update everyone's settings
    for (auto p : rootNodes)
    {
        updateSettings(p, true);
    }
    
    isLoadingSignalChain = false;
    
    updateViews(rootNodes[0]);
    
}

bool ProcessorGraph::hasRecordNode()
{
    
    Array<GenericProcessor*> processors = getListOfProcessors();
    
    for (auto p : processors)
    {
        if (p->isRecordNode())
        {
            return true;
        }
    }
    return false;
    
}

Array<GenericProcessor*> ProcessorGraph::getListOfProcessors()
{

    Array<GenericProcessor*> allProcessors;

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
            allProcessors.add(p);
        }
    }

    return allProcessors;

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

    for (auto& recordNode : getRecordNodes())
        addConnection(MESSAGE_CENTER_ID, midiChannelIndex,
                  recordNode->getNodeId(), midiChannelIndex);

}


void ProcessorGraph::updateConnections()
{

    clearConnections(); // clear processor graph

    Array<GenericProcessor*> splitters;

    // keep track of which splitter is currently being explored, in case there's another
    // splitter between the one being explored and its source.
    GenericProcessor* activeSplitter = nullptr;

    // stores the pointer to a source leading into a particular dest node
    // along with a boolean vector indicating the position of this source
    // relative to other sources entering the dest via mergers
    // (when the mergerOrder vectors of all incoming nodes to a dest are
    // lexicographically sorted, the sources will be in the correct order)
    struct ConnectionInfo
    {
        GenericProcessor* source;
        std::vector<int> mergerOrder;
        bool connectContinuous;
        bool connectEvents;

        // for SortedSet sorting:
        bool operator<(const ConnectionInfo& other) const
        {
            return mergerOrder < other.mergerOrder;
        }

        bool operator==(const ConnectionInfo& other) const
        {
            return mergerOrder == other.mergerOrder;
        }
    };

    // each destination node gets a set of sources, sorted by their order as dictated by mergers
    std::unordered_map<GenericProcessor*, SortedSet<ConnectionInfo>> sourceMap;

    for (int n = 0; n < rootNodes.size(); n++) // cycle through the tabs
    {
        LOGD("Signal chain: ", n);
        std::cout << std::endl;

        //GenericEditor* sourceEditor = (GenericEditor*) tabs[n]->getEditor();
        GenericProcessor* source = rootNodes[n];

        while (source != nullptr)// && destEditor->isEnabled())
        {
            LOGD("Source node: ", source->getName(), ".");
            GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

            if (source->isReady())
            {
                //TODO: This is will be removed when probe based audio node added. 
                connectProcessorToAudioNode(source);

                if (source->isRecordNode())
                    connectProcessorToMessageCenter(source);

                // find the next dest that's not a merger or splitter
                GenericProcessor* prev = source;

                ConnectionInfo conn;
                conn.source = source;
                conn.connectContinuous = true;
                conn.connectEvents = true;

                while (dest != nullptr && (dest->isMerger() || dest->isSplitter()))
                {
                    if (dest->isSplitter() && dest != activeSplitter && !splitters.contains(dest))
                    {
                        // add to stack of splitters to explore
                        splitters.add(dest);
                        dest->switchIO(0); // go down first path
                    }
                    else if (dest->isMerger())
                    {
                        auto merger = static_cast<Merger*>(dest);

                        // keep the input aligned with the current path
                        int path = merger->switchToSourceNode(prev);
                        jassert(path != -1); // merger not connected to prev?
                        
                        conn.mergerOrder.insert(conn.mergerOrder.begin(), path);
                        conn.connectContinuous &= merger->sendContinuousForSource(prev);
                        conn.connectEvents &= merger->sendEventsForSource(prev);
                    }

                    prev = dest;
                    dest = dest->getDestNode();
                }

                if (dest != nullptr)
                {
                    if (dest->isReady())
                    {
                        sourceMap[dest].add(conn);
                    }
                }
                else
                {
                    LOGD("     No dest node.");
                }
            }

            std::cout << std::endl;

            source->wasConnected = true;

            if (dest != nullptr && dest->wasConnected)
            {
                // don't bother retraversing downstream of a dest that has already been connected
                // (but if it leads to a splitter that is still in the stack, it may still be
                // used as a source for the unexplored branch.)

                LOGD(dest->getName(), " ", dest->getNodeId(), " has already been connected.");
                dest = nullptr;
            }

            source = dest; // switch source and dest

            if (source == nullptr)
            {
                if (splitters.size() > 0)
                {
                    activeSplitter = splitters.getLast();
                    splitters.removeLast();
                    activeSplitter->switchIO(1);

                    source = activeSplitter;
                    GenericProcessor* newSource;
                    while (source->isSplitter() || source->isMerger())
                    {
                        newSource = source->getSourceNode();
                        newSource->setPathToProcessor(source);
                        source = newSource;
                    }
                }
                else
                {
                    activeSplitter = nullptr;
                }
            }

        } // end while source != 0
    } // end "tabs" for loop

    // actually connect sources to each dest processor,
    // in correct order by merger topography
    for (const auto& destSources : sourceMap)
    {
        GenericProcessor* dest = destSources.first;

        for (const ConnectionInfo& conn : destSources.second)
        {
            connectProcessors(conn.source, dest, conn.connectContinuous, conn.connectEvents);
        }
    }

    //OwnedArray<EventChannel> extraChannels;
    getMessageCenter()->addSpecialProcessorChannels();
	
	getAudioNode()->updatePlaybackBuffer();

    /*
    for (auto& recordNode : getRecordNodes())
        recordNode->addSpecialProcessorChannels(extraChannels);
    */

} // end method

void ProcessorGraph::connectProcessors(GenericProcessor* source, GenericProcessor* dest,
    bool connectContinuous, bool connectEvents)
{

    if (source == nullptr || dest == nullptr)
        return;

    LOGD("     Connecting ", source->getName(), " ", source->getNodeId()); //" channel ";);
    LOGD("              to ", dest->getName(), " ", dest->getNodeId());

    // 1. connect continuous channels
    if (connectContinuous)
    {
        for (int chan = 0; chan < source->getNumOutputs(); chan++)
        {
            LOGDD(chan, " ");

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

    //3. If dest is a record node, register the processor
    if (dest->isRecordNode())
    {
        ((RecordNode*)dest)->registerProcessor(source);
    }

}

void ProcessorGraph::connectProcessorToAudioNode(GenericProcessor* source)
{

    /*
LOGD("#########SKIPPING CONNECT TO RECORD NODE");

    if (source == nullptr)
        return;
    */

    getAudioNode()->registerProcessor(source);
    //getRecordNode()->registerProcessor(source);

    for (int chan = 0; chan < source->getNumOutputs(); chan++)
    {

        getAudioNode()->addInputChannel(source, chan);

        addConnection(source->getNodeId(),                   // sourceNodeID
                      chan,                                  // sourceNodeChannelIndex
                      AUDIO_NODE_ID,                         // destNodeID
                      getAudioNode()->getNextChannel(true)); // destNodeChannelIndex
   

        /*
        getRecordNode()->addInputChannel(source, chan);

        addConnection(source->getNodeId(),                    // sourceNodeID
                      chan,                                   // sourceNodeChannelIndex
                      RECORD_NODE_ID,                         // destNodeID
                      getRecordNode()->getNextChannel(true)); // destNodeChannelIndex
        */
 
    }

    /*
    // connect event channel
    addConnection(source->getNodeId(),    // sourceNodeID
                  midiChannelIndex,       // sourceNodeChannelIndex
                  RECORD_NODE_ID,         // destNodeID
                  midiChannelIndex);      // destNodeChannelIndex
    */

    // connect event channel
    addConnection(source->getNodeId(),    // sourceNodeID
                  midiChannelIndex,       // sourceNodeChannelIndex
                  AUDIO_NODE_ID,          // destNodeID
                  midiChannelIndex);      // destNodeChannelIndex


    //getRecordNode()->addInputChannel(source, midiChannelIndex);

}


void ProcessorGraph::connectProcessorToMessageCenter(GenericProcessor* source)
{

    // connect event channel
    addConnection(getMessageCenter()->getNodeId(),    // sourceNodeID
                  midiChannelIndex,       // sourceNodeChannelIndex
                  source->getNodeId(),          // destNodeID
                  midiChannelIndex);      // destNodeChannelIndex

}


GenericProcessor* ProcessorGraph::createProcessorFromDescription(ProcessorDescription& description)
{
	GenericProcessor* processor = nullptr;

	if (description.fromProcessorList)
	{

        LOGD("Creating from description...");
        LOGD(description.libName, "::", description.processorName, \
            description.processorType, "-", description.processorIndex,")");

		processor = ProcessorManager::createProcessor((ProcessorClasses) description.processorType,
                                                      description.processorIndex);
	}
	else
	{
        LOGD("Creating from plugin info...");
        LOGD(description.libName, "(", description.libVersion, ")::", description.processorName);

		processor = ProcessorManager::createProcessorFromPluginInfo((Plugin::PluginType)
                                                                    description.processorType,
                                                                    description.processorIndex,
                                                                    description.processorName,
                                                                    description.libName,
                                                                    description.libVersion,
                                                                    description.isSource,
                                                                    description.isSink);
	}

	String msg = "New " + description.processorName + " created";
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
    GenericProcessor* originalSource = processor->getSourceNode();
    GenericProcessor* originalDest = processor->getDestNode();
    
    if (originalSource != nullptr)
    {
        originalSource->setDestNode(originalDest);
    }
    
    if (originalDest != nullptr)
    {
        if (!processor->isMerger())
        {
            originalDest->setSourceNode(originalSource);
        } else
        {
            Merger* merger = (Merger*) processor;
            
            GenericProcessor* sourceA = merger->getSourceNode(0);
            GenericProcessor* sourceB = merger->getSourceNode(1);
            
            if (sourceA != nullptr && sourceB == nullptr)
            {
                originalDest->setSourceNode(sourceA);
                sourceA->setDestNode(originalDest);
            }
            else if (sourceA == nullptr && sourceB != nullptr)
            {
                originalDest->setSourceNode(sourceB);
                sourceB->setDestNode(originalDest);
            }
            else if (sourceA != nullptr && sourceB != nullptr){
            
                originalDest->setSourceNode(originalSource);
                originalSource->setDestNode(originalDest);
            
                if (sourceA == originalSource)
                    sourceB->setDestNode(nullptr);
                else
                    sourceA->setDestNode(nullptr);
                
            } else {
                originalDest->setSourceNode(nullptr);
            }
        }
            
        if (originalDest->isMerger())
        {
            MergerEditor* editor = (MergerEditor*) originalDest->getEditor();
            editor->switchSource();
        }
    } else {
        
        if (processor->isMerger())
        {
            Merger* merger = (Merger*) processor;
            
            GenericProcessor* sourceA = merger->getSourceNode(0);
            GenericProcessor* sourceB = merger->getSourceNode(1);
            
            if (sourceA != nullptr)
                sourceA->setDestNode(nullptr);
            
            if (sourceB != nullptr)
                sourceB->setDestNode(nullptr);
        }
        
    }
    
    if (processor->isSplitter())
    {
        processor->switchIO();
        GenericProcessor* alternateDest = processor->getDestNode();
        if (alternateDest != nullptr)
        {
            alternateDest->setSourceNode(nullptr);
            checkForNewRootNodes(alternateDest);
        }
    }
    
    //if (processor->isMerger())
    //{
    //    processor->switchIO();
     //   GenericProcessor* alternateSource = processor->getSourceNode();
    //    if (alternateSource != nullptr)
    //        alternateSource->setDestNode(nullptr);
    //}

    LOGD("Removing processor with ID ", processor->getNodeId());

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

    checkForNewRootNodes(processor, false, false);
}

bool ProcessorGraph::enableProcessors()
{

    updateConnections();

    LOGD("Enabling processors...");

    bool allClear;

    if (getNumNodes() < 4)
    {
    LOGD("Not enough processors in signal chain to acquire data");
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
                LOGD(p->getName(), " said it's not OK.");
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

	//Update special channels indexes, at the end
	//To change, as many other things, when the probe system is implemented
    for (auto& node : getRecordNodes())
    {
        node->updateRecordChannelIndexes();
    }
	getAudioNode()->updateRecordChannelIndexes();

    //	sendActionMessage("Acquisition started.");
	m_startSoftTimestamp = Time::getHighResolutionTicks();
	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(true);
    return true;
}

bool ProcessorGraph::disableProcessors()
{

    LOGD("Disabling processors...");

    bool allClear;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);
        if (node->nodeId != OUTPUT_NODE_ID )
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            LOGD("Disabling ", p->getName());
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

    //AccessClass::getEditorViewport()->signalChainCanBeEdited(true);
	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(false);
    //	sendActionMessage("Acquisition ended.");

    return true;
}

void ProcessorGraph::setRecordState(bool isRecording)
{

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

Array<RecordNode*> ProcessorGraph::getRecordNodes()
{

    Array<RecordNode*> recordNodes;

    Array<GenericProcessor*> processors = getListOfProcessors();

    for (int i = 0; i < processors.size(); i++)
    {
        if (processors[i]->isRecordNode())
            recordNodes.add((RecordNode*)processors[i]);
    }

    return recordNodes;

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

uint32 ProcessorGraph::getGlobalTimestampSourceFullId() const
{
	if (!m_timestampSource)
		return 0;

	return GenericProcessor::getProcessorFullId(m_timestampSource->getNodeId(), m_timestampSourceSubIdx);
}

void ProcessorGraph::setTimestampWindow(TimestampSourceSelectionWindow* window)
{
	m_timestampWindow = window;
}
