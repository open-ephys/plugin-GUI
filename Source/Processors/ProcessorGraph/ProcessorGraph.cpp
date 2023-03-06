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
#include "../FileReader/FileReader.h"
#include "../SourceNode/SourceNode.h"
#include "../MessageCenter/MessageCenter.h"
#include "../MessageCenter/MessageCenterEditor.h"
#include "../Merger/Merger.h"
#include "../Splitter/Splitter.h"
#include "../../UI/UIComponent.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/GraphViewer.h"

#include "../ProcessorManager/ProcessorManager.h"
#include "../../Audio/AudioComponent.h"
#include "../../AccessClass.h"

std::map< ChannelKey, bool> ProcessorGraph::bufferLookupMap;

ProcessorGraph::ProcessorGraph() :
    currentNodeId(100),
    isLoadingSignalChain(false)
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
    LOGD("Move downstream: ", moveDownstream);

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
            updateSettings(newDest);
        }
    }

    checkForNewRootNodes(processor, false, true);

    if (moveDownstream) // processor is further down the signal chain, its original dest may have changed
    {
        //LOGD("MOVE: Updating settings for ", originalDest->getNodeId());
        if (originalDest != nullptr)
            updateSettings(originalDest);
        else
            updateSettings(processor);
    }
        
    else // processor is upstream of its original dest, so we can just update that
    {
        //LOGD("MOVE: Updating settings for ", processor->getNodeId());
        updateSettings(processor);
    }
        
}

GenericProcessor* ProcessorGraph::createProcessor(Plugin::Description& description,
                                      GenericProcessor* sourceNode,
                                      GenericProcessor* destNode,
                                      bool signalChainIsLoading)
{
	std::unique_ptr<GenericProcessor> processor = nullptr;
    GenericProcessor* addedProc = nullptr;

    LOGC("Creating processor with name: ", description.name);

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
		AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Open Ephys", e.what());
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

        if (!signalChainIsLoading)
        {
            addedProc->initialize(false);
        }


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
                        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
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
                        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
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
                    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Signal chain error",
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

                        Merger* merger = (Merger*)p;

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

    if(!signalChainIsLoading)
    {
        CoreServices::saveRecoveryConfig();
    }

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
        LOGD("Processor to view: ", processor->getName());

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
            LOGD("  Source: ", rootProcessor->getName());
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

        LOGD(" Adding ", processor->getEditor()->getNameAndId(), " to editor array.");

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

    for (auto processor : getListOfProcessors())
    {
        NodeID nodeId = NodeID(processor->getNodeId());
        std::unique_ptr<GenericEditor> editor;
        editor.swap(processor->editor);
        editor.reset();
        Node::Ptr node = removeNode(nodeId);
        node.reset();
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
        LOGG("Updating settings for ", p->getName(), " : ", p->getNodeId());
        updateSettings(p, true);
    }

    isLoadingSignalChain = false;

    for (auto p : getListOfProcessors())
    {
        p->initialize(true);
    }

}

bool ProcessorGraph::allRecordNodesAreSynchronized()
{
    Array<GenericProcessor*> processors = getListOfProcessors();

    for (auto p : processors)
    {
        if (p->isRecordNode())
        {
            RecordNode* r = (RecordNode*) p;
            
            if (!r->isSynchronized())
                return false;
        }
    }
    
    return true;
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

void ProcessorGraph::updateBufferMap(int inputNodeId,
    int inputIndex,
    int outputNodeId,
    int outputIndex,
    bool isNeededLater)
{
    ChannelKey key;
    key.inputNodeId = inputNodeId;
    key.inputIndex = inputIndex;
    key.outputNodeId = outputNodeId;
    key.outputIndex = outputIndex;

    bufferLookupMap.insert(std::make_pair(key, isNeededLater));
}

bool ProcessorGraph::isBufferNeededLater(int inputNodeId,
    int inputIndex,
    int outputNodeId,
    int outputIndex,
    bool* isValid)

{
    //LOGG(inputNodeId, ":", inputIndex, " --> ", outputNodeId, ":", outputIndex);

    ChannelKey key;
    key.inputNodeId = inputNodeId;
    key.inputIndex = inputIndex;
    key.outputNodeId = outputNodeId;
    key.outputIndex = outputIndex;

    auto search = bufferLookupMap.find(key);

    if (search != bufferLookupMap.end())
    {
        *isValid = true;
        return bufferLookupMap[key];
    }

    *isValid = false;

    return false;

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

    bufferLookupMap.clear();

}


void ProcessorGraph::updateConnections()
{

    clearConnections(); // clear processor graph

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

    for (auto processor : getListOfProcessors())
    {

        LOGG("Processor: ", processor->getName(), " ", processor->getNodeId());
            
        if (processor->isMerger())
            continue;
            
        if (processor->isSplitter())
            continue;

       if (processor->isSource())
           connectProcessorToMessageCenter(processor);

       if (processor->isAudioMonitor())
           connectAudioMonitorToAudioNode(processor);

       ConnectionInfo conn;
       conn.source = processor;
       conn.connectContinuous = true;
       conn.connectEvents = true;

       Array<GenericProcessor*> nodesToConnect;

       GenericProcessor* destNode = processor->getDestNode();

       if (destNode == nullptr)
            continue;

       Array<Splitter*> splitters;
       GenericProcessor* lastProcessor = processor;

       // if the next node is a Merger, we actually need to
       // connect to the next non-Merger node
       while (destNode->isMerger())
       {
            LOGG("  Found Merger: ", destNode->getNodeId());

            Merger* merger = (Merger*) destNode;

            int path = merger->getSourceNode(0) == lastProcessor ? 0 : 1;

            LOGG("   --> Adding Merger order: ", path);
            conn.mergerOrder.insert(conn.mergerOrder.begin(), path);

            lastProcessor = destNode;
            destNode = destNode->getDestNode();

            if (destNode == nullptr)
                break;

            if (destNode->isSplitter())
            {
                splitters.add((Splitter*) destNode);
                LOGG("  Adding Splitter: ", destNode->getNodeId());
            } else if (!destNode->isMerger())
            {
                nodesToConnect.add(destNode);
                LOGG("  Adding node to connect: ", destNode->getNodeId());
            }

       }

       // if there's nothing after the Merger, skip
       if (destNode == nullptr)
            continue;

       // if the next node is a Splitter, we need to connect to both paths
       if (destNode->isSplitter())
       {
            splitters.add((Splitter*) destNode);
            LOGG("  Adding Splitter: ", destNode->getNodeId());
       }

       // keep connecting until we've found all possible paths
       while (splitters.size() > 0)
       {
            Splitter* thisSplitter = splitters.getLast();
            splitters.removeLast();

            for (int path = 0; path < 2; path++)
            {
                if (thisSplitter->getDestNode(path) != nullptr)
                {
                    if (thisSplitter->getDestNode(path)->isSplitter())
                    {
                        LOGG("  Adding Splitter: ", destNode->getNodeId());
                        splitters.add((Splitter*) thisSplitter->getDestNode(path));
                    } else {
                        if (thisSplitter->getDestNode(path) != nullptr)
                        {
                            LOGG("  Adding node to connect: ", thisSplitter->getDestNode(path)->getNodeId());
                            nodesToConnect.add(thisSplitter->getDestNode(path));
                        }
                    }
                }
            }
       }

       // if it's not a Splitter or Merger, simply connect
       if (nodesToConnect.size() == 0)
            nodesToConnect.add(destNode);

       // Add all the connections we found
       for (auto node : nodesToConnect)
       {
            sourceMap[node].add(conn);
       }

        // Finally, actually connect sources to each dest processor,
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

            //LOGG("  Source channel: ", cs.channelIndex, ", Dest Channel: ", cd.channelIndex);

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

    //3. Ensure the RecordNode block size matches the buffer size of Audio Settings
    if (dest->isRecordNode())
    {
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

    GenericProcessor* originalSource;

    if (processor->isMerger())
    {
        Merger* merger = (Merger*) processor;

        originalSource = merger->getSourceNode(merger->getPath());
    } else {
        originalSource = processor->getSourceNode();
    }

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
            Merger* merger = (Merger*) originalDest;
            merger->lostInput();
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

    checkForNewRootNodes(processor, false, false);

    AccessClass::getEditorViewport()->removeEditor(processor->editor.get());

    //// need this to prevent editors from remaining after starting acquisition
    std::unique_ptr<GenericEditor> editor;
    editor.swap(processor->editor);
    editor.reset();

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

    int NWBCounter = 0;

    for (int i = 0; i < getNumNodes(); i++)
    {

        Node* node = getNode(i);

        String name = node->getProcessor()->getName();

        // 1. Check that NWB resources are only used by a single processor.
        if (name == "File Reader")
        {
            FileReader* fr = static_cast<FileReader*>(node->getProcessor());

            if (File(fr->getFile()).getFileExtension() == ".nwb")
                NWBCounter += 1;
        }
        else if (name == "Record Node")
        {
            RecordNode* rn = static_cast<RecordNode*>(node->getProcessor());

            if (rn->getEngineId() == "NWB2")
                NWBCounter += 1;
        }

        if (NWBCounter > 1)
        {
            AccessClass::getUIComponent()->disableCallbacks();

            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                "WARNING", "Open Ephys currently does not support multiple processors using NWB format. Please modify the signal chain accordingly to proceed with acquisition.");

            return false;
        }

        // 2. Check that all processors are enabled and ready for acquisition.
        if (node->nodeID != NodeID(OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*)node->getProcessor();

            if (!p->isEnabled)
            {
                LOGD(" ", p->getName(), " is not ready to start acquisition.");
                AccessClass::getUIComponent()->disableCallbacks();
                return false;
            }

            // THIS MAY BE POSSIBLE IN A FUTURE UPDATE TO THE PLUGIN API
            //if (p->isSource())
            //{
            //    SourceNode* s = (SourceNode*)p;
            //    if (!s->isReady())
            //    {
            //        LOGD(" ", p->getName(), " is not ready to start acquisition.");
            //        AccessClass::getUIComponent()->disableCallbacks();
            //        return false;
            //    }
            //}
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


int64 ProcessorGraph::getGlobalTimestamp() const
{

    return CoreServices::getSoftwareTimestamp();

}

float ProcessorGraph::getGlobalSampleRate() const
{

    return CoreServices::getSoftwareSampleRate();

}

String ProcessorGraph::getGlobalTimestampSource() const
{
    return "Milliseconds since midnight Jan 1st 1970 UTC.";
}

