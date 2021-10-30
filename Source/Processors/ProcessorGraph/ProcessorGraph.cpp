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
#include "../MessageCenter/MessageCenterEditor.h"
#include "../Merger/Merger.h"
#include "../Splitter/Splitter.h"
#include "../../UI/UIComponent.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/TimestampSourceSelection.h"
#include "../../UI/GraphViewer.h"

#include "../ProcessorManager/ProcessorManager.h"
#include "../../Audio/AudioComponent.h"
#include "../../AccessClass.h"

#include "ProcessorGraphHttpServer.h"

ProcessorGraph::ProcessorGraph() : currentNodeId(100), isLoadingSignalChain(false)
{

    // The ProcessorGraph will always have 0 inputs (all content is generated within graph)
    // but it will have N outputs, where N is the number of channels for the audio monitor
    setPlayConfigDetails(0, // number of inputs
                         2, // number of outputs
                         44100.0, // sampleRate
                         1024);    // blockSize

    http_server_thread = std::make_unique<ProcessorGraphHttpServer>(this);
}

ProcessorGraph::~ProcessorGraph()
{
    if (http_server_thread) {
        http_server_thread->stop();
    }
}


void ProcessorGraph::enableHttpServer() {
    http_server_thread->start();
}

void ProcessorGraph::disableHttpServer() {
    http_server_thread->stop();
}

void ProcessorGraph::createDefaultNodes()
{

    // add output node -- sends output to the audio card
    auto outputNode = new AudioProcessorGraph::AudioGraphIOProcessor(AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

    // add audio node -- takes all inputs and selects those to be used for audio monitoring
    auto audioNode = new AudioNode();
    audioNode->setNodeId(AUDIO_NODE_ID);

    // add message center
    auto msgCenter = new MessageCenter();
    msgCenter->setNodeId(MESSAGE_CENTER_ID);

    addNode(std::unique_ptr< AudioProcessorGraph::AudioGraphIOProcessor>(outputNode), NodeID(OUTPUT_NODE_ID));
    addNode(std::unique_ptr<AudioNode>(audioNode), NodeID(AUDIO_NODE_ID));
    addNode(std::unique_ptr<MessageCenter>(msgCenter), NodeID(MESSAGE_CENTER_ID));

}

void ProcessorGraph::updateBufferSize()
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
            
            if (newSource->isSplitter())
            {
                Splitter* splitter = (Splitter*) newSource;
                splitter->setSplitterDestNode(processor);
            }
            else
            {
                newSource->setDestNode(processor);
            }
                
            
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

GenericProcessor* ProcessorGraph::createProcessor(Plugin::Description& description,
                                      GenericProcessor* sourceNode,
                                      GenericProcessor* destNode,
                                      bool signalChainIsLoading)
{
	std::unique_ptr<GenericProcessor> processor = nullptr;
    GenericProcessor* addedProc = nullptr;
    
    LOGD("Creating processor with name: ", description.name);
    
    if (sourceNode != nullptr)
        LOGDD("Source node: ", sourceNode->getName());
    //else
     //   LOGD("No source node.");
    
    if (destNode != nullptr)
        LOGDD("Dest node: ", destNode->getName());
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
        Node* n = addNode(std::move(processor), NodeID(id)); // have to add it so it can be deleted by the graph

        addedProc = (GenericProcessor*)n->getProcessor();
        
        GenericEditor* editor = (GenericEditor*)addedProc->createEditor();

        editor->refreshColors();
        
		if (addedProc->isSource()) // if we are adding a source processor
        {
            
            if (sourceNode != nullptr)
            {
                // if there's a source feeding into source, form a new signal chain
                addedProc->setDestNode(sourceNode->getDestNode());
                addedProc->setSourceNode(nullptr);
                sourceNode->setDestNode(nullptr);
                if (destNode != nullptr)
                    destNode->setSourceNode(addedProc);
            }
            
            if (sourceNode == nullptr && destNode != nullptr)
            {
                // if we're adding it upstream of another processor
                if (!destNode->isSource())
                {
                    // if it's not a source, connect them
                    addedProc->setDestNode(destNode);
                    destNode->setSourceNode(addedProc);
                }
                else
                {
                    // if it's in front of a source, start a new signal chain
                    addedProc->setDestNode(nullptr);
                }
            }
            
			if (addedProc->generatesTimestamps())
			{ // If there are no source processors and we add one,
              //  set it as default for global timestamps and sample rates
				m_validTimestampSources.add(addedProc);
				if (m_timestampSource == nullptr)
				{
					m_timestampSource = addedProc;
					m_timestampSourceStreamId = 0;
				}
				if (m_timestampWindow)
					m_timestampWindow->updateProcessorList();
			}
            
        } else {
            // a source node was not dropped
            if (sourceNode != nullptr)
            {
                // if there's a source available, connect them
                addedProc->setSourceNode(sourceNode);
                sourceNode->setDestNode(addedProc);
            }
                
            if (destNode != nullptr)
            {
                if (!destNode->isSource())
                {
                    // if it's not behind a source node, connect them
                    addedProc->setDestNode(destNode);
                    
                    if (!destNode->isMerger())
                        destNode->setSourceNode(addedProc);
                    else
                        destNode->setMergerSourceNode(addedProc);
                    
                } else {
                    // if there's a source downstream, start a new signalchain
                    addedProc->setDestNode(nullptr);
                }
            }
        }
        
        if (!checkForNewRootNodes(addedProc))
        {
            removeProcessor(addedProc);
            updateViews(rootNodes.getLast());
            return nullptr;
        }
        
        String msg = "New " + addedProc->getName() + " created";
        CoreServices::sendStatusMessage(msg);
        
	}
	else
	{
		CoreServices::sendStatusMessage("Not a valid processor.");
        updateViews(nullptr);
        return nullptr;
	}
    
    if (!signalChainIsLoading)
    {
        updateSettings(addedProc);
    } else {
        updateViews(addedProc);
    }
    
    return addedProc;
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

    getMessageCenter()->addSpecialProcessorChannels();
        
    GenericProcessor* processorToUpdate = processor;
    
    Array<Splitter*> splitters;
    
    while ((processor != nullptr) || (splitters.size() > 0))
    {
        if (processor != nullptr)
        {
            processor->update();
            
            if (signalChainIsLoading && processor->getSourceNode() != nullptr)
            {
                processor->loadFromXml();
                processor->update();
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
    
    updateViews(processorToUpdate, true);
    
}

void ProcessorGraph::updateViews(GenericProcessor* processor, bool updateGraphViewer)
{

    if (updateGraphViewer)
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
    
    for (int i = 0; i < 99; i++)
    {
        if (processor == nullptr)
            break;
        
        rootProcessor = processor;
        processor = processor->getSourceNode();
        
        if (rootProcessor == processor)
            break;
        
        if (rootProcessor != nullptr)
        {
            LOGDD("  Source: ", rootProcessor->getName());
        }

        if (processor != nullptr)
        {
            if (processor->isSplitter())
            {
                SplitterEditor* sp = (SplitterEditor*)processor->getEditor();
                GenericEditor* ed = rootProcessor->getEditor();

                LOGD("---> Switching splitter to view: ", ed->getName())
                sp->switchDest(sp->getPathForEditor(ed));
            }
        }
    }
    
    processor = rootProcessor;

    for (int i = 0; i < 99; i++)
    {
        if (processor == nullptr)
            break;
        
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

void ProcessorGraph::broadcastMessage(String msg)
{
    AccessClass::getMessageCenter()->broadcastMessage(msg);
}

String ProcessorGraph::sendConfigMessage(GenericProcessor* p, String msg)
{
    return p->handleConfigMessage(msg);
}

void ProcessorGraph::restoreParameters()
{
    
    isLoadingSignalChain = true;

    LOGDD("Restoring parameters for each processor...");
    
    // first connect the mergers
    for (auto p : getListOfProcessors())
    {
        if (p->isMerger())
        {
            Merger* m = (Merger*) p;
            m->restoreConnections();
        }
    }
    
    // load source node parameters
    for (auto p : rootNodes)
    {
        p->loadFromXml();
        
    }

    // update everyone's settings
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

        int nodeId = node->nodeID.uid;

        if (nodeId != OUTPUT_NODE_ID &&
            nodeId != AUDIO_NODE_ID &&
            nodeId != MESSAGE_CENTER_ID)
        {
            GenericProcessor* p =(GenericProcessor*) node->getProcessor();
            allProcessors.add(p);
        }
    }

    return allProcessors;

}

bool ProcessorGraph::isBufferNeededLater(int inputNodeId,
    int inputIndex,
    int outputNodeId,
    int outputIndex)

{
    LOGG(inputNodeId, ":", inputIndex, " --> ", outputNodeId, ":", outputIndex);

    GenericProcessor* inputProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(inputNodeId);
    GenericProcessor* outputProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(outputNodeId);


    if (inputNodeId == AUDIO_NODE_ID)
    {
        if (outputIndex == midiChannelIndex)
        {
            if (outputNodeId < AUDIO_NODE_ID)
            {
                if (outputProcessor->getDestNode() != nullptr)
                {
                    return true;
                        
                }
                else {
                    return false;
                }
            }
            else
                 return false;
        }
            
        else {
            if (outputNodeId < AUDIO_NODE_ID)
            {
                if (inputIndex == -1)
                    return true;
                else
                    return false;
            }
            else
            {
                return true;
            }
                
        }
            
    }
    else if (inputNodeId == OUTPUT_NODE_ID)
    {
        if (outputNodeId == AUDIO_NODE_ID)
            return true;
        else
        {
            if (outputNodeId < AUDIO_NODE_ID)
            {
                if (outputProcessor->getDestNode() != nullptr)
                {

                    return true;
                    //if (outputProcessor->getDestNode()->isSplitter())
                    //{
                    //    if (outputProcessor->isAudioMonitor() 
                    //        && outputIndex >= outputProcessor->getNumInputChannels()
                    //        && outputIndex != midiChannelIndex)
                    //        return false;
                    //    else
                    //        return true;
                    //}
                        
                   // else
                   // {
                   //     return false;
                   // }
                       
                }
                else {
                    return false;
                }
            }

            return false;
        }
            
    }
    else if (inputNodeId == MESSAGE_CENTER_ID)
    {
        if (outputIndex == midiChannelIndex)
            return true;
        else
            return false;
    }
    else {
        
        if (outputNodeId == MESSAGE_CENTER_ID)
        {
            if (inputProcessor->getDestNode() != nullptr)
            {
                if (inputProcessor->getDestNode()->isMerger())
                {
                    Merger* merger = (Merger*)inputProcessor->getDestNode();

                    if (merger->getSourceNode(0) == inputProcessor)
                    {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else {

                    if (inputProcessor->isMerger())
                        return true;
                    else
                        return false;
                }
            }
            
        }

        if (inputIndex == -1)
        {

            if (outputProcessor->isMerger() || outputProcessor->isSplitter())
                return false;

            if (outputProcessor->isAudioMonitor())
            {
                if (outputIndex >= outputProcessor->getNumInputChannels() && outputIndex != midiChannelIndex)
                    return true;
            }
                
            if (outputProcessor->getDestNode() == nullptr)
                return false;
            else
            {
                if (outputProcessor->getDestNode()->isSplitter())
                {
                    Splitter* splitter = (Splitter*)outputProcessor->getDestNode();

                    if (splitter->getDestNode(0) == nullptr || splitter->getDestNode(1) == nullptr)
                        return false;
                }

                return true;
            }
        }
        else {

            if (outputProcessor->getDestNode() != nullptr)
            {
                if (!outputProcessor->getDestNode()->isSplitter())
                {
                    return false;
                }
                else
                {
                    Splitter* splitter = (Splitter*)outputProcessor->getDestNode();

                    //std::cout << "DESTNODE0: " << splitter->getDestNode(0)->getNodeId() << std::endl;
                    //std::cout << "DESTNODE1: " << splitter->getDestNode(1)->getNodeId() << std::endl;

                    if (splitter->getDestNode(0) == nullptr || splitter->getDestNode(1) == nullptr)
                        return false;

                    if (splitter->getDestNode(0) == inputProcessor)
                        return true;
                    else
                        return false;
                }
                    
            }

            if (inputProcessor->getDestNode() == nullptr)
                return false;

            if (outputProcessor->isAudioMonitor())
            {
                if (outputIndex >= outputProcessor->getNumInputChannels() && outputIndex != midiChannelIndex)
                    return false;
            }

        }
    }

    return true;
}

int ProcessorGraph::getStreamIdForChannel(Node& node, int channel)
{

    int nodeId = node.nodeID.uid;

    if (nodeId != OUTPUT_NODE_ID &&
        nodeId != AUDIO_NODE_ID &&
        nodeId != MESSAGE_CENTER_ID)
    {
        GenericProcessor* p = (GenericProcessor*) node.getProcessor();
        
        return p->getContinuousChannel(channel)->getStreamId();
    }
    else {
        return nodeId;
    }
}

GenericProcessor* ProcessorGraph::getProcessorWithNodeId(int nodeId)
{

    if (nodeId == AUDIO_NODE_ID)
        return getAudioNode();
    else if (nodeId == MESSAGE_CENTER_ID)
        return getMessageCenter();

    for (auto processor : getListOfProcessors())
    {
        if (processor->getNodeId() == nodeId)
        {
            return processor;
        }
    }
    
    return nullptr;
}

void ProcessorGraph::clearConnections()
{

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);
        int nodeId = node->nodeID.uid;

        if (nodeId != OUTPUT_NODE_ID)
        {

            if (nodeId != AUDIO_NODE_ID)
            {
                disconnectNode(node->nodeID);
            }

            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            p->resetConnections();

        }
    }

    // connect audio subnetwork
    for (int n = 0; n < 2; n++)
    {
        NodeAndChannel src, dest;

        src.nodeID = NodeID(AUDIO_NODE_ID);
        src.channelIndex = n;

        dest.nodeID = NodeID(OUTPUT_NODE_ID);
        dest.channelIndex = n;

        addConnection(Connection(src, dest));

    }

}


void ProcessorGraph::updateConnections()
{

    clearConnections(); // clear processor graph

    Array<GenericProcessor*> splitters;
    Array<GenericProcessor*> splitters2;
    Array<int> splitterStates;

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

        LOGG("\nConnecting signal chain: ", n);

        GenericProcessor* source = rootNodes[n];

        while (source != nullptr)
        {
            LOGG("  Node: ", source->getName(), ".");
            GenericProcessor* dest = (GenericProcessor*) source->getDestNode();

            if(source->isAudioMonitor())
                connectAudioMonitorToAudioNode(source);

            if (source->isSource())
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
                        
                    Splitter* splitter = (Splitter*) dest;
                    splitterStates.add(splitter->getPath());
            
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
                sourceMap[dest].add(conn);
            }
            else
            {
                LOGG("     No dest node.");
            }

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
                    splitters2.insert(0, activeSplitter);
            
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
        
    // restore splitters to their initial state
    for (int i = 0; i < splitters2.size(); i++)
    {
        splitters2[i]->switchIO(splitterStates[i]);
    }

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

}

void ProcessorGraph::connectProcessors(GenericProcessor* source, GenericProcessor* dest,
    bool connectContinuous, bool connectEvents)
{

    if (source == nullptr || dest == nullptr)
        return;

    LOGG("Connecting ", source->getName(), " ", source->getNodeId(), " to ", dest->getName(), " ", dest->getNodeId());

    NodeAndChannel cs, cd;
    cs.nodeID = NodeID(source->getNodeId()); //source
    cd.nodeID = NodeID(dest->getNodeId()); //dest

    // 1. connect continuous channels
    if (connectContinuous)
    {        
        for (int chan = 0; chan < source->getNumOutputs(); chan++)
        {

            cs.channelIndex = chan;
            cd.channelIndex = dest->getIndexOfMatchingChannel(source->getContinuousChannel(chan));

            LOGG("  Source channel: ",", Dest Channel: ", cd.channelIndex);

            if (cd.channelIndex > -1)
            {
                addConnection(Connection(cs, cd));
            }  
        }
    }

    // 2. connect event channel
    if (connectEvents)
    {
        cs.channelIndex = midiChannelIndex;
        cd.channelIndex = midiChannelIndex;
        addConnection(Connection(cs, cd));
    }

    //3. If dest is a record node, register the processor and 
    //ensure the RecordNode block size matches the buffer size of Audio Settings
    if (dest->isRecordNode())
    {
        ((RecordNode*)dest)->registerProcessor(source);

        AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
        AudioDeviceManager::AudioDeviceSetup ads;
        adm.getAudioDeviceSetup(ads);
        int blockSize = ads.bufferSize;
        ((RecordNode*)dest)->updateBlockSize(blockSize);
    }

}

void ProcessorGraph::connectAudioMonitorToAudioNode(GenericProcessor* source)
{

    LOGG("Connecting Audio Monitor ", source->getNodeId(), " to Audio Node");

    NodeAndChannel cs, cd;
    cs.nodeID = NodeID(source->getNodeId()); //source
    cd.nodeID = NodeID(AUDIO_NODE_ID); // dest

    int numOutputs = source->getNumOutputs();
    
    for (int chan = 0; chan < 2; chan++)
    {

        cs.channelIndex = numOutputs + chan;
        cd.channelIndex = chan;

        LOGG("  Source channel: ", cs.channelIndex, ", Dest Channel: ", cd.channelIndex);

        addConnection(Connection(cs, cd));
    }

    // connect event channel (disable for now)
    //cs.channelIndex = midiChannelIndex;
   // cd.channelIndex = midiChannelIndex;
    //addConnection(Connection(cs, cd));

    getAudioNode()->registerProcessor(source);

}


void ProcessorGraph::connectProcessorToMessageCenter(GenericProcessor* source)
{

    // connect event channel only
    NodeAndChannel cs, cd;

    cs.nodeID = NodeID(getMessageCenter()->getNodeId()); //source
    cs.channelIndex = midiChannelIndex;

    cd.nodeID = NodeID(source->getNodeId()); // dest
    cd.channelIndex = midiChannelIndex;

    addConnection(Connection(cs, cd));

    LOGD("Connecting ", source->getName(), " (", source->getNodeId(), ") to Message Center");

}


std::unique_ptr<GenericProcessor> ProcessorGraph::createProcessorFromDescription(Plugin::Description& description)
{
	std::unique_ptr<GenericProcessor> processor = nullptr;
            
    processor = std::move(ProcessorManager::createProcessor(description));

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

    LOGD("Removing processor with ID ", processor->getNodeId());

    NodeID nodeId = NodeID(processor->getNodeId());

	if (processor->isSource())
	{
		m_validTimestampSources.removeAllInstancesOf(processor);

		if (m_timestampSource == processor)
		{
			const GenericProcessor* newProc = 0;

			//Look for the next source node. If none is found, set the sourceid to 0
			for (int i = 0; i < getNumNodes() && newProc == nullptr; i++)
			{
				if (getNode(i)->nodeID != NodeID(OUTPUT_NODE_ID))
				{
					GenericProcessor* p = dynamic_cast<GenericProcessor*>(getNode(i)->getProcessor());

					if (p && p->isSource() && p->generatesTimestamps())
					{
						newProc = p;
					}
				}
			}
			m_timestampSource = newProc;
			m_timestampSourceStreamId = 0;
		}
		if (m_timestampWindow)
			m_timestampWindow->updateProcessorList();
	}

    checkForNewRootNodes(processor, false, false);

    // need this in order to prevent double-deletion of plugin editors
    // (not entirely sure why)
    std::unique_ptr<GenericEditor> editor;
    editor.swap(processor->editor);

    Node::Ptr node = removeNode(nodeId);
    node.reset();

}

bool ProcessorGraph::isReady()
{

    LOGD("ProcessorGraph::enableProcessors()");

    if (getNumNodes() < 4)
    {
        LOGD("Not enough processors in signal chain to acquire data");
        AccessClass::getUIComponent()->disableCallbacks();
        return false;
    }

    LOGD("Checking that all processors are enabled...");
    
    for (int i = 0; i < getNumNodes(); i++)
    {

        Node* node = getNode(i);

        if (node->nodeID != NodeID(OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*)node->getProcessor();

            if (!p->isEnabled)
            {
                LOGD(" ", p->getName(), " is not ready to start acquisition.");
                AccessClass::getUIComponent()->disableCallbacks();
                return false;
            }
        }
    }

    return true;
}

void ProcessorGraph::startAcquisition()
{

    LOGD("ProcessorGraph starting acquisition...");

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        if (node->nodeID != NodeID(OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            p->startAcquisition();

            if (p->getEditor() != nullptr)
            {
                p->getEditor()->editorStartAcquisition();
            }
        }
    }

    //buildRenderingSequence(); // use handleAsyncUpdate() call for now

	m_startSoftTimestamp = Time::getHighResolutionTicks();

	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(true);

}

void ProcessorGraph::stopAcquisition()
{

    LOGD("ProcessorGraph stopping acquisition...");

    bool allClear;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        if (node->nodeID != NodeID(OUTPUT_NODE_ID) )
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            LOGD("Disabling ", p->getName());

            if (p->getEditor() != nullptr)
            {                
                p->getEditor()->editorStopAcquisition();
            }

            allClear = p->stopAcquisition();
				

        }
    }

	if (m_timestampWindow)
		m_timestampWindow->setAcquisitionState(false);

}

void ProcessorGraph::setRecordState(bool isRecording)
{

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode(i);

        if (node->nodeID != NodeID(OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();

            if (isRecording)
                p->startRecording();
            else
                p->stopRecording();
            
        }
    }

}


AudioNode* ProcessorGraph::getAudioNode()
{

    Node* node = getNodeForId(NodeID(AUDIO_NODE_ID));
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

    Node* node = getNodeForId(NodeID(MESSAGE_CENTER_ID));
    return (MessageCenter*) node->getProcessor();

}


void ProcessorGraph::setTimestampSource(int sourceIndex, int streamId)
{
	m_timestampSource = m_validTimestampSources[sourceIndex];

	if (m_timestampSource)
	{
        m_timestampSourceStreamId = streamId;
	}
	else
	{
        m_timestampSourceStreamId = 0;
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

	selectedSubId = m_timestampSourceStreamId;
}

int64 ProcessorGraph::getGlobalTimestamp() const
{
	if (!m_timestampSource)
	{
        return CoreServices::getSoftwareTimestamp();
	}
	else
	{
		//return static_cast<int64>((Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks() - m_timestampSource->getLastProcessedsoftwareTime())
		//	* m_timestampSource->getSampleRate(m_timestampSourceStreamId)) + m_timestampSource->getSourceTimestamp(m_timestampSourceStreamId));
	}
}

float ProcessorGraph::getGlobalSampleRate() const
{
	if (!m_timestampSource)
	{
		return CoreServices::getSoftwareSampleRate();
	}
	else
	{
		return m_timestampSource->getSampleRate(m_timestampSourceStreamId);
	}
}

String ProcessorGraph::getGlobalTimestampSource() const
{
    if (!m_timestampSource)
    {
        return "Milliseconds since midnight Jan 1st 1970 UTC.";
    }
    else
    {
        return m_timestampSource->getName() + " - " + String(m_timestampSource->getNodeId());
    }
}

void ProcessorGraph::setTimestampWindow(TimestampSourceSelectionWindow* window)
{
    m_timestampWindow = window;
}


/*uint32 ProcessorGraph::getGlobalTimestampSourceFullId() const
{
	if (!m_timestampSource)
		return 0;

	return GenericProcessor::getProcessorFullId(m_timestampSource->getNodeId(), m_timestampSourceSubIdx);
}*/

