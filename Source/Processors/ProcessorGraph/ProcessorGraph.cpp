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

#include <map>
#include <stdio.h>
#include <utility>
#include <vector>

#include "../GenericProcessor/GenericProcessor.h"
#include "ProcessorGraph.h"

#include "../../UI/EditorViewport.h"
#include "../../UI/GraphViewer.h"
#include "../../UI/ProcessorList.h"
#include "../../UI/UIComponent.h"
#include "../AudioNode/AudioNode.h"
#include "../Editors/VisualizerEditor.h"
#include "../FileReader/FileReader.h"
#include "../Merger/Merger.h"
#include "../MessageCenter/MessageCenter.h"
#include "../RecordNode/RecordNode.h"
#include "../SourceNode/SourceNode.h"
#include "../Splitter/Splitter.h"

#include "../../AccessClass.h"
#include "../../Audio/AudioComponent.h"
#include "../PluginManager/PluginManager.h"
#include "../ProcessorManager/ProcessorManager.h"

std::map<ChannelKey, bool> ProcessorGraph::bufferLookupMap;

ProcessorGraph::ProcessorGraph (bool isConsoleApp_) : isConsoleApp (isConsoleApp_),
                                                      currentNodeId (100),
                                                      isLoadingSignalChain (false)
{
    // The ProcessorGraph will always have 0 inputs (all content is generated within graph)
    // but it will have N outputs, where N is the number of channels for the audio monitor
    setPlayConfigDetails (0, // number of inputs
                          2, // number of outputs
                          44100.0, // sampleRate
                          1024); // blockSize

    pluginManager = std::make_unique<PluginManager>();
    LOGD ("Created plugin manager");

    undoManager = std::make_unique<UndoManager>();
    LOGD ("Created undo manager");

    createDefaultNodes();

    AccessClass::setProcessorGraph (this);

    pluginManager->loadAllPlugins();
}

ProcessorGraph::~ProcessorGraph()
{
}

void ProcessorGraph::createDefaultNodes()
{
    // add output node -- sends output to the audio card
    auto outputNode = new AudioProcessorGraph::AudioGraphIOProcessor (AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);

    // add audio node -- takes all inputs and selects those to be used for audio monitoring
    auto audioNode = new AudioNode();
    audioNode->setNodeId (AUDIO_NODE_ID);
    LOGD ("Created Audio Node");

    // add message center
    auto msgCenter = new MessageCenter();
    msgCenter->setNodeId (MESSAGE_CENTER_ID);
    LOGD ("Created Message Center");

    addNode (std::unique_ptr<AudioProcessorGraph::AudioGraphIOProcessor> (outputNode), NodeID (OUTPUT_NODE_ID));
    addNode (std::unique_ptr<AudioNode> (audioNode), NodeID (AUDIO_NODE_ID));
    addNode (std::unique_ptr<MessageCenter> (msgCenter), NodeID (MESSAGE_CENTER_ID));
}

void ProcessorGraph::updateBufferSize()
{
    getAudioNode()->updateBufferSize();
}

void ProcessorGraph::moveProcessor (GenericProcessor* processor,
                                    GenericProcessor* newSource,
                                    GenericProcessor* newDest,
                                    bool moveDownstream)
{
    GenericProcessor* originalSource = processor->getSourceNode();
    GenericProcessor* originalDest = processor->getDestNode();

    bool shouldCheckForNewRootNodes = true;

    if (originalSource != nullptr)
    {
        // if processor's original source was an empty processor, remove the empty processor
        if (originalSource->isEmpty())
        {
            if (originalDest == nullptr)
            {
                if (! isConsoleApp)
                {
                    AccessClass::getGraphViewer()->removeNode (originalSource);
                    AccessClass::getEditorViewport()->removeEditor (originalSource->getEditor());
                }
                rootNodes.remove (rootNodes.indexOf (originalSource));
                emptyProcessors.removeObject (originalSource);
                originalSource = nullptr;
            }
            else
            {
                originalSource->setDestNode (originalDest);
                shouldCheckForNewRootNodes = false;
            }
        }
        else
            originalSource->setDestNode (originalDest);
    }

    if (originalDest != nullptr)
    {
        originalDest->setSourceNode (originalSource);
    }

    LOGD ("Processor to move: ", processor->getName());
    if (originalSource != nullptr)
        LOGD ("Original source: ", originalSource->getName());
    if (originalDest != nullptr)
        LOGD ("Original dest: ", originalDest->getName());
    if (newSource != nullptr)
        LOGD ("New source: ", newSource->getName());
    if (newDest != nullptr)
        LOGD ("New dest: ", newDest->getName());

    processor->setSourceNode (nullptr);
    processor->setDestNode (nullptr);

    if (newSource != nullptr)
    {
        if (! processor->isSource())
        {
            processor->setSourceNode (newSource);

            if (newSource->isSplitter())
            {
                Splitter* splitter = (Splitter*) newSource;
                splitter->setSplitterDestNode (processor);
            }
            else
            {
                newSource->setDestNode (processor);
            }
        }
        else
        {
            processor->setSourceNode (nullptr);
            newSource->setDestNode (nullptr);
            //rootNodes.add(newSource);
        }
    }

    if (newDest != nullptr)
    {
        //  if processor is moved downstream of an empty processor, remove the empty processor
        if (newDest->getSourceNode() != nullptr
            && newDest->getSourceNode()->isEmpty())
        {
            if (processor->isSource())
            {
                if (! isConsoleApp)
                {
                    AccessClass::getGraphViewer()->removeNode (newDest->getSourceNode());
                    AccessClass::getEditorViewport()->removeEditor (newDest->getSourceNode()->getEditor());
                }
                rootNodes.remove (rootNodes.indexOf (newDest->getSourceNode()));
                emptyProcessors.removeObject (newDest->getSourceNode());
            }
            else
            {
                GenericProcessor* emptyProc = newDest->getSourceNode();
                emptyProc->setDestNode (processor);
                processor->setSourceNode (emptyProc);
                shouldCheckForNewRootNodes = false;
            }
        }

        if (! newDest->isSource())
        {
            processor->setDestNode (newDest);
            newDest->setSourceNode (processor);
        }
        else
        {
            processor->setDestNode (nullptr);
        }
    }

    if (shouldCheckForNewRootNodes)
        checkForNewRootNodes (processor, false, true);

    if (moveDownstream) // processor is further down the signal chain, its original dest may have changed
        updateSettings (originalDest);
    else // processor is upstream of its original dest, so we can just update that
        updateSettings (processor);
}

GenericProcessor* ProcessorGraph::createProcessor (Plugin::Description& description,
                                                   GenericProcessor* sourceNode,
                                                   GenericProcessor* destNode,
                                                   bool signalChainIsLoading,
                                                   bool undoingDelete)
{
    std::unique_ptr<GenericProcessor> processor = nullptr;
    GenericProcessor* addedProc = nullptr;

    LOGC ("Creating processor with name: ", description.name);

    if (sourceNode != nullptr)
        LOGDD ("Source node: ", sourceNode->getName());
    //else
    //   LOGD("No source node.");

    if (destNode != nullptr)
        LOGDD ("Dest node: ", destNode->getName());
    //else
    //    LOGD("No dest node.");

    try
    {
        processor = createProcessorFromDescription (description);
        processor->setHeadlessMode (isConsoleApp);
    }
    catch (std::exception& e)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Open Ephys", e.what());
    }

    if (processor != nullptr)
    {
        int id;

        if (description.nodeId > 0)
        {
            id = description.nodeId;
            currentNodeId = id >= currentNodeId ? id + 1 : currentNodeId;
        }
        else
        {
            id = currentNodeId++;
        }

        // identifier within processor graph
        processor->setNodeId (id);
        processor->registerParameters();
        Node* n = addNode (std::move (processor), NodeID (id)); // have to add it so it can be deleted by the graph

        addedProc = (GenericProcessor*) n->getProcessor();

        if (! isConsoleApp)
        {
            GenericEditor* editor = (GenericEditor*) addedProc->createEditor();
        }

        if (! signalChainIsLoading && ! undoingDelete)
        {
            addedProc->initialize (false);
        }

        bool shouldCheckForNewRootNodes = true;

        if (addedProc->isSource()) // if we are adding a source processor
        {
            if (sourceNode != nullptr)
            {
                // if there's a source feeding into source, form a new signal chain
                addedProc->setDestNode (sourceNode->getDestNode());
                addedProc->setSourceNode (nullptr);
                sourceNode->setDestNode (nullptr);
                if (destNode != nullptr)
                    destNode->setSourceNode (addedProc);

                if (rootNodes.size() == 8)
                {
                    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Signal chain error", "Maximum of 8 signal chains.");
                    removeProcessor (addedProc);
                    updateViews (rootNodes.getLast());
                    return nullptr;
                }
                else
                {
                    GenericProcessor* rootSource = sourceNode;

                    if (sourceNode->isMerger())
                    {
                        Merger* merger = (Merger*) sourceNode;
                        rootSource = merger->getSourceNode (1);
                    }

                    while (rootSource->getSourceNode() != nullptr)
                        rootSource = rootSource->getSourceNode();

                    rootNodes.insert (rootNodes.indexOf (rootSource) + 1, addedProc);
                    shouldCheckForNewRootNodes = false;
                }
            }

            if (sourceNode == nullptr && destNode != nullptr)
            {
                // if we're adding it upstream of another processor
                if (! destNode->isSource())
                {
                    // If the dest node has an empty source, remove the empty processor
                    if (destNode->getSourceNode() != nullptr && destNode->getSourceNode()->isEmpty())
                    {
                        if (! isConsoleApp)
                        {
                            AccessClass::getGraphViewer()->removeNode (destNode->getSourceNode());
                            AccessClass::getEditorViewport()->removeEditor (destNode->getSourceNode()->getEditor());
                        }

                        rootNodes.set (rootNodes.indexOf (destNode->getSourceNode()), addedProc);
                        emptyProcessors.removeObject (destNode->getSourceNode());

                        shouldCheckForNewRootNodes = false;
                    }

                    // if it's not a source, connect them
                    addedProc->setDestNode (destNode);
                    destNode->setSourceNode (addedProc);
                }
                else
                {
                    // if it's in front of a source, start a new signal chain
                    addedProc->setDestNode (nullptr);
                }
            }
        }
        else
        {
            // a source node was not dropped
            if (sourceNode != nullptr)
            {
                // if there's a source available, connect them
                addedProc->setSourceNode (sourceNode);
                sourceNode->setDestNode (addedProc);
            }

            if (destNode != nullptr)
            {
                if (! destNode->isSource())
                {
                    if (destNode->getSourceNode() != nullptr && destNode->getSourceNode()->isEmpty())
                    {
                        GenericProcessor* emptyProc = destNode->getSourceNode();
                        emptyProc->setDestNode (addedProc);
                        addedProc->setSourceNode (emptyProc);
                    }
                    // if it's not behind a source node, connect them
                    addedProc->setDestNode (destNode);
                    destNode->setSourceNode (addedProc);
                }
                else
                {
                    // if there's a source downstream, start a new signalchain
                    addedProc->setDestNode (nullptr);
                }
            }
        }

        if (shouldCheckForNewRootNodes && ! checkForNewRootNodes (addedProc))
        {
            removeProcessor (addedProc);
            updateViews (rootNodes.getLast());
            return nullptr;
        }

        String msg = "New " + addedProc->getName() + " created";
        CoreServices::sendStatusMessage (msg);
    }
    else
    {
        CoreServices::sendStatusMessage ("Not a valid processor.");
        updateViews (nullptr);
        return nullptr;
    }

    if (! signalChainIsLoading)
    {
        updateSettings (addedProc);

        if (! isConsoleApp && addedProc->getEditor()->isVisualizerEditor())
        {
            VisualizerEditor* editor = (VisualizerEditor*) addedProc->getEditor();
            editor->addTab();
        }
    }
    else
    {
        updateViews (addedProc);
    }

    return addedProc;
}

bool ProcessorGraph::checkForNewRootNodes (GenericProcessor* processor,
                                           bool processorBeingAdded, // default = true
                                           bool processorBeingMoved) // default = false
{
    if (processorBeingAdded) // adding processor
    {
        LOGDD ("Checking ", processor->getName(), " for new root node.")

        if (processor->getSourceNode() == nullptr)
        {
            LOGDD ("  Has no source node.");

            if (processor->getDestNode() != nullptr)
            {
                LOGDD ("  Has a dest node.");

                GenericProcessor* destNode = processor->getDestNode();

                if (rootNodes.indexOf (destNode) > -1)
                {
                    LOGDD ("  Found dest node in root nodes; swapping.");

                    rootNodes.set (rootNodes.indexOf (destNode), processor);
                    return true;
                }
                else
                {
                    LOGDD ("  Didn't find dest node in root nodes; adding a new root.");

                    if (rootNodes.size() == 8)
                    {
                        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Signal chain error", "Maximum of 8 signal chains.");
                        return false;
                    }
                    else
                    {
                        if (processor->isSource())
                        {
                            rootNodes.add (processor);
                        }
                        else
                        {
                            createEmptyProcessor (processor);
                        }

                        if (processor->isMerger())
                        {
                            Merger* merger = (Merger*) processor;
                            int originalPath = merger->getPath();
                            if (merger->getSourceNode (1 - originalPath) == nullptr)
                            {
                                merger->switchIO (1 - originalPath);
                                createEmptyProcessor (processor);
                                merger->switchIO (originalPath);
                            }
                        }

                        return true;
                    }
                }
            }
            else
            {
                LOGDD ("  Has no dest node; adding.");

                if (rootNodes.size() == 8)
                {
                    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Signal chain error", "Maximum of 8 signal chains.");
                    return false;
                }
                else
                {
                    if (processor->isSource())
                        rootNodes.add (processor);
                    else
                        createEmptyProcessor (processor);

                    if (processor->isMerger())
                    {
                        Merger* merger = (Merger*) processor;
                        int originalPath = merger->getPath();
                        if (merger->getSourceNode (1 - originalPath) == nullptr)
                        {
                            merger->switchIO (1 - originalPath);
                            createEmptyProcessor (processor);
                            merger->switchIO (originalPath);
                        }
                    }

                    return true;
                }
            }
        }
        else
        {
            if (processor->isMerger())
            {
                Merger* merger = (Merger*) processor;
                if (merger->getSourceNode (1) == nullptr)
                {
                    merger->switchIO (1);

                    GenericProcessor* sourceA = merger->getSourceNode (0);
                    while (sourceA->getSourceNode() != nullptr)
                    {
                        if (sourceA->isMerger())
                            sourceA = ((Merger*) sourceA)->getSourceNode (1);
                        else
                            sourceA = sourceA->getSourceNode();
                    }

                    createEmptyProcessor (processor, rootNodes.indexOf (sourceA) + 1);

                    merger->switchIO (0);
                }
            }
        }

        return true;
    }

    if (! processorBeingMoved) // deleting processor
    {
        if (processor->getSourceNode() != nullptr && processor->getSourceNode()->isEmpty())
        {
            if (processor->getDestNode() != nullptr && processor->getDestNode()->getSourceNode() == nullptr)
            {
                GenericProcessor* emptyProc = processor->getSourceNode();
                emptyProc->setDestNode (processor->getDestNode());
                processor->getDestNode()->setSourceNode (emptyProc);
            }
            else
            {
                if (! isConsoleApp)
                {
                    AccessClass::getGraphViewer()->removeNode (processor->getSourceNode());
                    AccessClass::getEditorViewport()->removeEditor (processor->getSourceNode()->getEditor());
                }

                rootNodes.remove (rootNodes.indexOf (processor->getSourceNode()));
                emptyProcessors.removeObject (processor->getSourceNode());
            }
        }

        if (processor->isMerger())
        {
            Merger* merger = (Merger*) processor;

            GenericProcessor* anotherSource = merger->getSourceNode (1 - merger->getPath());

            if (anotherSource != nullptr && anotherSource->isEmpty())
            {
                if (! isConsoleApp)
                {
                    AccessClass::getGraphViewer()->removeNode (anotherSource);
                    AccessClass::getEditorViewport()->removeEditor (anotherSource->getEditor());
                }

                rootNodes.remove (rootNodes.indexOf (anotherSource));
                emptyProcessors.removeObject (anotherSource);
            }
        }

        if (rootNodes.indexOf (processor) > -1)
        {
            if (processor->getDestNode() != nullptr)
            {
                createEmptyProcessor (processor->getDestNode(), rootNodes.indexOf (processor), true);
            }
            else
            {
                rootNodes.remove (rootNodes.indexOf (processor));
            }
        }

        return true;
    }
    else
    { // processor being moved

        LOGDD ("Processing being moved.");

        for (auto p : getListOfProcessors())
        {
            LOGDD ("Checking ", p->getName());

            if (p->getSourceNode() == nullptr) // no source node
            {
                LOGDD ("  Should be root.");

                if (rootNodes.indexOf (p) == -1) // not a root node yet
                {
                    if (! p->isMerger())
                    {
                        if (p->isSource())
                            rootNodes.add (p); // add it
                        else
                            createEmptyProcessor (p);

                        LOGDD ("  Adding as root.");
                    }
                    else
                    {
                        Merger* merger = (Merger*) p;

                        GenericProcessor* sourceA = merger->getSourceNode (0);
                        GenericProcessor* sourceB = merger->getSourceNode (1);

                        if (sourceA == nullptr) // no remaining source nodes
                        {
                            merger->switchIO (0);

                            if (sourceB == nullptr)
                                createEmptyProcessor (p);
                            else
                            {
                                while (sourceB->getSourceNode() != nullptr)
                                    sourceB = sourceA->getSourceNode();

                                createEmptyProcessor (p, rootNodes.indexOf (sourceB));
                            }
                        }
                        if (sourceB == nullptr) // no remaining source nodes
                        {
                            merger->switchIO (1);

                            sourceA = merger->getSourceNode (0);

                            while (sourceA->getSourceNode() != nullptr)
                                sourceA = sourceA->getSourceNode();

                            createEmptyProcessor (p, rootNodes.indexOf (sourceA) + 1);
                        }
                    }
                }
                else
                {
                    LOGDD ("  Already is root.");
                }
            }
            else
            {
                LOGDD ("  Should not be root.");

                if (rootNodes.indexOf (p) > -1) // has a source node, but is also a root node
                {
                    rootNodes.remove (rootNodes.indexOf (p)); // remove it
                    LOGDD ("  Removing as root.");
                }
                else
                {
                    LOGDD ("  Not a root.");
                }
            }
        }
    }

    return true;
}

void ProcessorGraph::updateSettings (GenericProcessor* processor, bool signalChainIsLoading)
{
    // prevents calls from within processors from triggering full update during loading
    if (signalChainIsLoading != isLoadingSignalChain)
    {
        //updateViews(processor);
        return;
    }

    getMessageCenter()->addSpecialProcessorChannels();

    GenericProcessor* processorToUpdate = processor;

    if (processorToUpdate != nullptr
        && processorToUpdate->isMerger()
        && processorToUpdate->getSourceNode() != nullptr
        && ! processorToUpdate->getSourceNode()->isEmpty())
    {
        processorToUpdate = processorToUpdate->getSourceNode();
        processor = processorToUpdate;
    }

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
                splitters.add ((Splitter*) processor);
                processor = splitters.getLast()->getDestNode (0); // travel down chain 0 first
            }
            else
            {
                processor = processor->getDestNode();
            }
        }
        else
        {
            Splitter* splitter = splitters.getFirst();
            processor = splitter->getDestNode (1); // then come back to chain 1
            splitters.remove (0);
        }
    }

    updateViews (processorToUpdate, true);

    if (! signalChainIsLoading && ! isConsoleApp)
    {
        CoreServices::saveRecoveryConfig();
    }
}

void ProcessorGraph::updateViews (GenericProcessor* processor, bool updateGraphViewer)
{
    if (updateGraphViewer && ! isConsoleApp)
        AccessClass::getGraphViewer()->updateNodes (processor, rootNodes);

    int tabIndex;

    if (processor == nullptr && rootNodes.size() > 0)
    {
        processor = rootNodes.getFirst();
    }

    processorArray.clear();
    GenericProcessor* rootProcessor = processor;

    if (processor != nullptr)
    {
        LOGD ("Processor to view: ", processor->getName());

        if (processor->isSplitter() && ! isConsoleApp)
        {
            SplitterEditor* spEditor = (SplitterEditor*) processor->getEditor();
            Splitter* spProc = (Splitter*) processor;

            spEditor->switchDest (spProc->getPath());
        }
    }

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
            LOGD ("  Source: ", rootProcessor->getName());
        }

        if (processor != nullptr)
        {
            LOGD ("  Going upstream!! Processor: ", processor->getName());
            if (processor->isSplitter() && ! isConsoleApp)
            {
                SplitterEditor* sp = (SplitterEditor*) processor->getEditor();
                GenericEditor* ed = rootProcessor->getEditor();

                LOGD ("---> Switching splitter to view: ", ed->getName())
                sp->switchDest (sp->getPathForEditor (ed));
            }
        }
    }

    processor = rootProcessor;

    for (int i = 0; i < 99; i++)
    {
        if (processor == nullptr)
            break;

        processorArray.add (processor);

        LOGD (" Adding ", processor->getName(), " to editor array.");

        if (processor->getDestNode() != nullptr)
        {
            if (processor->getDestNode()->isMerger())
            {
                if (processor->getDestNode()->getSourceNode() != processor)
                {
                    Merger* destMerger = (Merger*) processor->getDestNode();
                    destMerger->switchToSourceNode (processor);
                }
            }
        }

        processor = processor->getDestNode();
    }

    if (! isConsoleApp)
    {
        Array<GenericEditor*> editorArray;

        for (auto p : processorArray)
        {
            editorArray.add (p->getEditor());
        }

        AccessClass::getEditorViewport()->updateVisibleEditors (editorArray,
                                                                rootNodes.size(),
                                                                rootNodes.indexOf (rootProcessor));
    }
}

void ProcessorGraph::viewSignalChain (int index)
{
    updateViews (rootNodes[index]);
}

void ProcessorGraph::connectMergerSource (GenericProcessor* merger_, GenericProcessor* sourceNode, int path)
{
    Merger* merger = (Merger*) merger_;

    GenericProcessor* anotherSource = merger->getSourceNode (1 - path);

    if (sourceNode == merger->getSourceNode (path))
        return;

    if (merger->getSourceNode (path)->isEmpty())
    {
        if (! isConsoleApp && ! isLoadingSignalChain)
        {
            AccessClass::getGraphViewer()->removeNode (merger->getSourceNode (path));
            AccessClass::getEditorViewport()->removeEditor (merger->getSourceNode (path)->getEditor());
        }
        rootNodes.remove (rootNodes.indexOf (merger->getSourceNode (path)));
        emptyProcessors.removeObject (merger->getSourceNode (path));
    }

    // merger->switchIO(path);
    // merger->setMergerSourceNode(sourceNode);
    // sourceNode->setDestNode(merger);

    while (anotherSource->getSourceNode() != nullptr)
        anotherSource = anotherSource->getSourceNode();

    auto newSourceRoot = sourceNode;
    while (newSourceRoot->getSourceNode() != nullptr)
        newSourceRoot = newSourceRoot->getSourceNode();

    if ((path == 0 && rootNodes.indexOf (newSourceRoot) > rootNodes.indexOf (anotherSource))
        || (path == 1 && rootNodes.indexOf (newSourceRoot) < rootNodes.indexOf (anotherSource)))
    {
        merger->switchIO (path);
        merger->setMergerSourceNode (anotherSource);

        merger->switchIO (1 - path);
        merger->setMergerSourceNode (sourceNode);
        sourceNode->setDestNode (merger);
    }
    else
    {
        merger->switchIO (path);
        merger->setMergerSourceNode (sourceNode);
        sourceNode->setDestNode (merger);
    }
}

void ProcessorGraph::deleteNodes (Array<GenericProcessor*> processorsToDelete)
{
    GenericProcessor* sourceNode = nullptr;
    GenericProcessor* destNode = nullptr;
    bool isSplitter = false;

    for (auto processor : processorsToDelete)
    {
        sourceNode = processor->getSourceNode();

        destNode = processor->getDestNode();

        if (sourceNode != nullptr)
        {
            if (sourceNode->isEmpty())
                sourceNode = nullptr;
            else
                sourceNode->setDestNode (destNode);
        }

        if (destNode != nullptr)
        {
            if (destNode->isMerger())
            {
                Merger* merger = (Merger*) destNode;
                merger->switchToSourceNode (processor);
            }
            destNode->setSourceNode (sourceNode);
        }

        if (processor->isSplitter())
        {
            isSplitter = true;
            Splitter* splitter = (Splitter*) processor;
            GenericProcessor* destNodeA = splitter->getDestNode (0);
            GenericProcessor* destNodeB = splitter->getDestNode (1);

            removeProcessor (processor);

            updateSettings (destNodeA);
            updateSettings (destNodeB);
        }
        else
        {
            isSplitter = false;
            removeProcessor (processor);
        }
    }

    if (! isSplitter)
    {
        if (destNode != nullptr)
        {
            updateSettings (destNode);
        }
        else
        {
            updateSettings (sourceNode);
        }
    }
}

void ProcessorGraph::reconnectProcessors (int sourceNodeId,
                                          int destNodeid)
{
    GenericProcessor* sourceNode = getProcessorWithNodeId (sourceNodeId);
    GenericProcessor* destNode = getProcessorWithNodeId (destNodeid);

    // if dest node is a merger, switch to path that points to the processor being removed
    if (sourceNode != nullptr && destNode != nullptr)
    {
        if (destNode->getSourceNode() != nullptr
            && destNode->getSourceNode()->isEmpty())
        {
            if (! isConsoleApp)
            {
                AccessClass::getGraphViewer()->removeNode (destNode->getSourceNode());
                AccessClass::getEditorViewport()->removeEditor (destNode->getSourceNode()->getEditor());
            }

            rootNodes.remove (rootNodes.indexOf (destNode->getSourceNode()));
            emptyProcessors.removeObject (destNode->getSourceNode());
        }

        sourceNode->setDestNode (destNode);
        destNode->setSourceNode (sourceNode);

        updateSettings (destNode);
    }
}

void ProcessorGraph::clearSignalChain()
{
    for (auto processor : getListOfProcessors())
    {
        NodeID nodeId = NodeID (processor->getNodeId());
        std::unique_ptr<GenericEditor> editor;
        editor.swap (processor->editor);
        editor.reset();
        Node::Ptr node = removeNode (nodeId);
        node.reset();
    }

    rootNodes.clear();
    emptyProcessors.clear();
    currentNodeId = 100;

    if (! isConsoleApp)
        AccessClass::getGraphViewer()->removeAllNodes();

    updateViews (nullptr);
}

void ProcessorGraph::changeListenerCallback (ChangeBroadcaster* source)
{
    refreshColours();
}

void ProcessorGraph::refreshColours()
{
    for (auto p : getListOfProcessors())
    {
        GenericEditor* e = (GenericEditor*) p->getEditor();
        e->refreshColours();
    }

    AccessClass::getGraphViewer()->repaint();
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

void ProcessorGraph::broadcastMessage (String msg)
{
    AccessClass::getMessageCenter()->broadcastMessage (msg);
}

String ProcessorGraph::sendConfigMessage (GenericProcessor* p, String msg)
{
    return p->handleConfigMessage (msg);
}

void ProcessorGraph::restoreParameters()
{
    isLoadingSignalChain = true;

    LOGDD ("Restoring parameters for each processor...");

    // first connect the mergers
    for (auto p : getListOfProcessors())
    {
        if (p->isMerger())
        {
            Merger* m = (Merger*) p;
            m->restoreConnections();
        }
    }

    // Create message center event channel (necessary for DataThreads)
    getMessageCenter()->addSpecialProcessorChannels();

    // load source node parameters
    for (auto p : rootNodes)
    {
        if (p->isEmpty())
            continue;

        if (p->getPluginType() == Plugin::Type::DATA_THREAD)
            p->update();

        p->loadFromXml();
    }

    // update everyone's settings
    for (auto p : rootNodes)
    {
        LOGG ("Updating settings for ", p->getName(), " : ", p->getNodeId());
        updateSettings (p, true);
    }

    isLoadingSignalChain = false;

    for (auto p : getListOfProcessors())
    {
        p->initialize (true);
    }
}

std::vector<ProcessorAction*> ProcessorGraph::getUndoableActions (int nodeId)
{
    return GenericProcessor::getUndoableActions (nodeId);
}

void ProcessorGraph::updateUndoableActions (int nodeId)
{
    for (auto action : getUndoableActions (nodeId))
    {
        GenericProcessor* p = getProcessorWithNodeId (nodeId);
        p->update();
        action->restoreOwner (p);
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

            if (! r->isSynchronized())
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
        Node* node = getNode (i);

        int nodeId = node->nodeID.uid;

        if (nodeId != OUTPUT_NODE_ID && nodeId != AUDIO_NODE_ID && nodeId != MESSAGE_CENTER_ID)
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            allProcessors.add (p);
        }
    }

    return allProcessors;
}

void ProcessorGraph::updateBufferMap (int inputNodeId,
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

    bufferLookupMap.insert (std::make_pair (key, isNeededLater));
}

bool ProcessorGraph::isBufferNeededLater (int inputNodeId,
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

    auto search = bufferLookupMap.find (key);

    if (search != bufferLookupMap.end())
    {
        *isValid = true;
        return bufferLookupMap[key];
    }

    *isValid = false;

    return false;
}

int ProcessorGraph::getStreamIdForChannel (Node& node, int channel)
{
    int nodeId = node.nodeID.uid;

    if (nodeId != OUTPUT_NODE_ID && nodeId != AUDIO_NODE_ID && nodeId != MESSAGE_CENTER_ID)
    {
        GenericProcessor* p = (GenericProcessor*) node.getProcessor();

        return p->getContinuousChannel (channel)->getStreamId();
    }
    else
    {
        return nodeId;
    }
}

GenericProcessor* ProcessorGraph::getProcessorWithNodeId (int nodeId)
{
    if (nodeId == -1)
        return nullptr;
    else if (nodeId == AUDIO_NODE_ID)
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
        Node* node = getNode (i);
        int nodeId = node->nodeID.uid;

        if (nodeId != OUTPUT_NODE_ID)
        {
            if (nodeId != AUDIO_NODE_ID)
            {
                disconnectNode (node->nodeID);
            }

            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            p->resetConnections();
        }
    }

    // connect audio subnetwork
    for (int n = 0; n < 2; n++)
    {
        NodeAndChannel src, dest;

        src.nodeID = NodeID (AUDIO_NODE_ID);
        src.channelIndex = n;

        dest.nodeID = NodeID (OUTPUT_NODE_ID);
        dest.channelIndex = n;

        addConnection (Connection (src, dest));
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
        bool operator< (const ConnectionInfo& other) const
        {
            return mergerOrder < other.mergerOrder;
        }

        bool operator== (const ConnectionInfo& other) const
        {
            return mergerOrder == other.mergerOrder;
        }
    };

    // each destination node gets a set of sources, sorted by their order as dictated by mergers
    std::unordered_map<GenericProcessor*, SortedSet<ConnectionInfo>> sourceMap;

    for (auto processor : getListOfProcessors())
    {
        LOGG ("Processor: ", processor->getName(), " ", processor->getNodeId());

        if (processor->isMerger())
            continue;

        if (processor->isSplitter())
            continue;

        if (processor->isSource())
            connectProcessorToMessageCenter (processor);

        if (processor->isAudioMonitor())
            connectAudioMonitorToAudioNode (processor);

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
            LOGG ("  Found Merger: ", destNode->getNodeId());

            Merger* merger = (Merger*) destNode;

            int path = merger->getSourceNode (0) == lastProcessor ? 0 : 1;

            LOGG ("   --> Adding Merger order: ", path);
            conn.mergerOrder.insert (conn.mergerOrder.begin(), path);

            lastProcessor = destNode;
            destNode = destNode->getDestNode();

            if (destNode == nullptr)
                break;

            if (destNode->isSplitter())
            {
                splitters.add ((Splitter*) destNode);
                LOGG ("  Adding Splitter: ", destNode->getNodeId());
            }
            else if (! destNode->isMerger())
            {
                nodesToConnect.add (destNode);
                LOGG ("  Adding node to connect: ", destNode->getNodeId());
            }
        }

        // if there's nothing after the Merger, skip
        if (destNode == nullptr)
            continue;

        // if the next node is a Splitter, we need to connect to both paths
        if (destNode->isSplitter())
        {
            splitters.add ((Splitter*) destNode);
            LOGG ("  Adding Splitter: ", destNode->getNodeId());
        }

        // keep connecting until we've found all possible paths
        while (splitters.size() > 0)
        {
            Splitter* thisSplitter = splitters.getLast();
            splitters.removeLast();

            for (int path = 0; path < 2; path++)
            {
                if (thisSplitter->getDestNode (path) != nullptr)
                {
                    if (thisSplitter->getDestNode (path)->isSplitter())
                    {
                        LOGG ("  Adding Splitter: ", destNode->getNodeId());
                        splitters.add ((Splitter*) thisSplitter->getDestNode (path));
                    }
                    else
                    {
                        if (thisSplitter->getDestNode (path) != nullptr)
                        {
                            LOGG ("  Adding node to connect: ", thisSplitter->getDestNode (path)->getNodeId());
                            nodesToConnect.add (thisSplitter->getDestNode (path));
                        }
                    }
                }
            }
        }

        // if it's not a Splitter or Merger, simply connect
        if (nodesToConnect.size() == 0)
            nodesToConnect.add (destNode);

        // Add all the connections we found
        for (auto node : nodesToConnect)
        {
            sourceMap[node].add (conn);
        }

        // Finally, actually connect sources to each dest processor,
        // in correct order by merger topography
        for (const auto& destSources : sourceMap)
        {
            GenericProcessor* dest = destSources.first;

            for (const ConnectionInfo& conn : destSources.second)
            {
                connectProcessors (conn.source, dest, conn.connectContinuous, conn.connectEvents);
            }
        }
    }
}

void ProcessorGraph::connectProcessors (GenericProcessor* source, GenericProcessor* dest, bool connectContinuous, bool connectEvents)
{
    if (source == nullptr || dest == nullptr)
        return;

    LOGG ("Connecting ", source->getName(), " ", source->getNodeId(), " to ", dest->getName(), " ", dest->getNodeId());

    NodeAndChannel cs, cd;
    cs.nodeID = NodeID (source->getNodeId()); //source
    cd.nodeID = NodeID (dest->getNodeId()); //dest

    // 1. connect continuous channels
    if (connectContinuous)
    {
        for (int chan = 0; chan < source->getNumOutputs(); chan++)
        {
            cs.channelIndex = chan;
            cd.channelIndex = dest->getIndexOfMatchingChannel (source->getContinuousChannel (chan));

            //LOGG("  Source channel: ", cs.channelIndex, ", Dest Channel: ", cd.channelIndex);

            if (cd.channelIndex > -1)
            {
                addConnection (Connection (cs, cd));
            }
        }
    }

    // 2. connect event channel
    if (connectEvents)
    {
        cs.channelIndex = midiChannelIndex;
        cd.channelIndex = midiChannelIndex;
        addConnection (Connection (cs, cd));
    }

    //3. Ensure the RecordNode block size matches the buffer size of Audio Settings
    if (dest->isRecordNode())
    {
        AudioDeviceManager& adm = AccessClass::getAudioComponent()->deviceManager;
        AudioDeviceManager::AudioDeviceSetup ads;
        adm.getAudioDeviceSetup (ads);
        int blockSize = ads.bufferSize;
        ((RecordNode*) dest)->updateBlockSize (blockSize);
    }
}

void ProcessorGraph::connectAudioMonitorToAudioNode (GenericProcessor* source)
{
    LOGG ("Connecting Audio Monitor ", source->getNodeId(), " to Audio Node");

    NodeAndChannel cs, cd;
    cs.nodeID = NodeID (source->getNodeId()); //source
    cd.nodeID = NodeID (AUDIO_NODE_ID); // dest

    int numOutputs = source->getNumOutputs();

    for (int chan = 0; chan < 2; chan++)
    {
        cs.channelIndex = numOutputs + chan;
        cd.channelIndex = chan;

        LOGG ("  Source channel: ", cs.channelIndex, ", Dest Channel: ", cd.channelIndex);

        addConnection (Connection (cs, cd));
    }

    getAudioNode()->registerProcessor (source);
}

void ProcessorGraph::connectProcessorToMessageCenter (GenericProcessor* source)
{
    // connect event channel only
    NodeAndChannel cs, cd;

    cs.nodeID = NodeID (getMessageCenter()->getNodeId()); //source
    cs.channelIndex = midiChannelIndex;

    cd.nodeID = NodeID (source->getNodeId()); // dest
    cd.channelIndex = midiChannelIndex;

    addConnection (Connection (cs, cd));

    LOGD ("Connecting ", source->getName(), " (", source->getNodeId(), ") to Message Center");
}

std::unique_ptr<GenericProcessor> ProcessorGraph::createProcessorFromDescription (Plugin::Description& description)
{
    std::unique_ptr<GenericProcessor> processor = nullptr;

    processor = std::move (ProcessorManager::createProcessor (description));

    if (processor != nullptr && processor->isEmpty() && ! isConsoleApp)
    {
        processor->setNodeId (-1);
        processor->createEditor();
    }

    return processor;
}

bool ProcessorGraph::processorWithSameNameExists (const String& name)
{
    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        if (name.equalsIgnoreCase (node->getProcessor()->getName()))
            return true;
    }

    return false;
}

void ProcessorGraph::removeProcessor (GenericProcessor* processor)
{
    GenericProcessor* originalSource;

    if (processor->isMerger())
    {
        Merger* merger = (Merger*) processor;

        originalSource = merger->getSourceNode (merger->getPath());
    }
    else
    {
        originalSource = processor->getSourceNode();
    }

    GenericProcessor* originalDest = processor->getDestNode();

    if (originalSource != nullptr)
    {
        if (originalSource->isEmpty())
            originalSource = nullptr;
        else
            originalSource->setDestNode (originalDest);
    }

    if (originalDest != nullptr)
    {
        if (! processor->isMerger())
        {
            originalDest->setSourceNode (originalSource);
        }
        else
        {
            Merger* merger = (Merger*) processor;

            GenericProcessor* sourceA = merger->getSourceNode (0);
            sourceA->isEmpty() ? sourceA = nullptr : sourceA;
            GenericProcessor* sourceB = merger->getSourceNode (1);
            sourceB->isEmpty() ? sourceB = nullptr : sourceB;

            if (sourceA != nullptr && sourceB == nullptr)
            {
                originalDest->setSourceNode (sourceA);
                sourceA->setDestNode (originalDest);
            }
            else if (sourceA == nullptr && sourceB != nullptr)
            {
                originalDest->setSourceNode (sourceB);
                sourceB->setDestNode (originalDest);
            }
            else if (sourceA != nullptr && sourceB != nullptr)
            {
                originalDest->setSourceNode (originalSource);
                originalSource->setDestNode (originalDest);

                if (sourceA == originalSource)
                    sourceB->setDestNode (nullptr);
                else
                    sourceA->setDestNode (nullptr);
            }
            else
            {
                originalDest->setSourceNode (nullptr);
            }
        }
    }
    else
    {
        if (processor->isMerger())
        {
            Merger* merger = (Merger*) processor;

            GenericProcessor* sourceA = merger->getSourceNode (0);
            GenericProcessor* sourceB = merger->getSourceNode (1);

            if (sourceA != nullptr)
                sourceA->setDestNode (nullptr);

            if (sourceB != nullptr)
                sourceB->setDestNode (nullptr);
        }
    }

    if (processor->isSplitter())
    {
        processor->switchIO();
        GenericProcessor* alternateDest = processor->getDestNode();
        if (alternateDest != nullptr)
        {
            alternateDest->setSourceNode (nullptr);
            checkForNewRootNodes (alternateDest);
        }
    }

    LOGD ("Removing processor with ID ", processor->getNodeId());

    NodeID nodeId = NodeID (processor->getNodeId());

    checkForNewRootNodes (processor, false, false);

    if (! isConsoleApp)
    {
        AccessClass::getGraphViewer()->removeNode (processor);

        AccessClass::getEditorViewport()->removeEditor (processor->editor.get());

        //// need this to prevent editors from remaining after starting acquisition
        std::unique_ptr<GenericEditor> editor;
        editor.swap (processor->editor);
        editor.reset();
    }

    Node::Ptr node = removeNode (nodeId);
    node.reset();
}

bool ProcessorGraph::isReady()
{
    LOGD ("ProcessorGraph checking for all valid parameters...");

    //Iterate through all the active nodes in the signal chain
    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        if (node->nodeID != NodeID (OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();

            for (auto param : p->getParameters())
            {
                if (! param->isValid())
                {
                    CoreServices::sendStatusMessage ("Parameter " + param->getKey() + " is not valid.");
                    AccessClass::getControlPanel()->disableCallbacks();
                    return false;
                }
            }
        }
    }

    LOGD ("All parameters are valid.");

    LOGD ("ProcessorGraph checking minimum number of nodes...");

    if (getNumNodes() < 4)
    {
        LOGD ("Not enough processors in signal chain to acquire data");
        AccessClass::getControlPanel()->disableCallbacks();
        return false;
    }

    LOGD ("Checking that all processors are enabled...");

    int NWBCounter = 0;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        String name = node->getProcessor()->getName();

        // 1. Check that NWB resources are only used by a single processor.
        if (name == "File Reader")
        {
            FileReader* fr = static_cast<FileReader*> (node->getProcessor());

            if (File (fr->getFile()).getFileExtension() == ".nwb")
                NWBCounter += 1;
        }
        else if (name == "Record Node")
        {
            RecordNode* rn = static_cast<RecordNode*> (node->getProcessor());

            if (rn->getEngineId() == "NWB2")
                NWBCounter += 1;
        }

        if (NWBCounter > 1)
        {
            AccessClass::getControlPanel()->disableCallbacks();

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "WARNING",
                                              "Open Ephys currently does not support multiple processors using NWB format. Please modify the signal chain accordingly to proceed with acquisition.");

            return false;
        }

        // 2. Check that all processors are enabled and ready for acquisition.
        if (node->nodeID != NodeID (OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();

            if (! p->isEnabled)
            {
                LOGD (" ", p->getName(), " is not ready to start acquisition.");
                AccessClass::getControlPanel()->disableCallbacks();
                return false;
            }

            if (! p->isReady())
            {
                CoreServices::sendStatusMessage (p->getName() + " is not ready to start acquisition!");
                AccessClass::getControlPanel()->disableCallbacks();
                return false;
            }
        }
    }

    return true;
}

void ProcessorGraph::startAcquisition()
{
    LOGD ("ProcessorGraph starting acquisition...");

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        if (node->nodeID != NodeID (OUTPUT_NODE_ID))
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
    LOGD ("ProcessorGraph stopping acquisition...");

    bool allClear;

    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        if (node->nodeID != NodeID (OUTPUT_NODE_ID))
        {
            GenericProcessor* p = (GenericProcessor*) node->getProcessor();
            LOGD ("Disabling ", p->getName());

            if (p->getEditor() != nullptr)
            {
                p->getEditor()->editorStopAcquisition();
            }

            allClear = p->stopAcquisition();
        }
    }
}

void ProcessorGraph::setRecordState (bool isRecording)
{
    for (int i = 0; i < getNumNodes(); i++)
    {
        Node* node = getNode (i);

        if (node->nodeID != NodeID (OUTPUT_NODE_ID))
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
    Node* node = getNodeForId (NodeID (AUDIO_NODE_ID));
    return (AudioNode*) node->getProcessor();
}

Array<RecordNode*> ProcessorGraph::getRecordNodes()
{
    Array<RecordNode*> recordNodes;

    Array<GenericProcessor*> processors = getListOfProcessors();

    for (int i = 0; i < processors.size(); i++)
    {
        if (processors[i]->isRecordNode())
            recordNodes.add ((RecordNode*) processors[i]);
    }

    return recordNodes;
}

MessageCenter* ProcessorGraph::getMessageCenter()
{
    Node* node = getNodeForId (NodeID (MESSAGE_CENTER_ID));
    return (MessageCenter*) node->getProcessor();
}

int64 ProcessorGraph::getGlobalTimestamp() const
{
    return CoreServices::getSystemTime();
}

float ProcessorGraph::getGlobalSampleRate() const
{
    return 1000.0f;
}

String ProcessorGraph::getGlobalTimestampSource() const
{
    return "Milliseconds since midnight Jan 1st 1970 UTC.";
}

void ProcessorGraph::saveToXml (XmlElement* xml)
{
    Array<GenericProcessor*> splitPoints;
    Array<GenericProcessor*> allSplitters;
    Array<int> splitterStates;
    /** Used to reset saveOrder at end, to allow saving the same processor multiple times*/
    Array<GenericProcessor*> allProcessors;

    int saveOrder = 0;

    XmlElement* info = xml->createNewChildElement ("INFO");

    XmlElement* version = info->createNewChildElement ("VERSION");
    version->addTextElement (JUCEApplication::getInstance()->getApplicationVersion());

    XmlElement* pluginAPIVersion = info->createNewChildElement ("PLUGIN_API_VERSION");
    pluginAPIVersion->addTextElement (String (PLUGIN_API_VER));

    Time currentTime = Time::getCurrentTime();

    info->createNewChildElement ("DATE")->addTextElement (currentTime.toString (true, true, true, true));
    info->createNewChildElement ("OS")->addTextElement (SystemStats::getOperatingSystemName());

    XmlElement* machine = info->createNewChildElement ("MACHINE");
    machine->setAttribute ("name", SystemStats::getComputerName());
    machine->setAttribute ("cpu_model", SystemStats::getCpuModel());
    machine->setAttribute ("cpu_num_cores", SystemStats::getNumCpus());

    Array<GenericProcessor*> rootNodes = getRootNodes();

    for (int i = 0; i < rootNodes.size(); i++)
    {
        XmlElement* signalChain = new XmlElement ("SIGNALCHAIN");

        bool isStartOfSignalChain = true;

        GenericProcessor* processor = rootNodes[i];

        if (rootNodes[i]->isEmpty())
            processor = rootNodes[i]->getDestNode();

        while (processor != nullptr)
        {
            if (processor->saveOrder < 0)
            {
                // create a new XML element
                signalChain->addChildElement (createNodeXml (processor, isStartOfSignalChain));
                processor->saveOrder = saveOrder;
                allProcessors.addIfNotAlreadyThere (processor);
                saveOrder++;

                if (processor->isSplitter())
                {
                    // add to list of splitters to come back to
                    splitPoints.add (processor);

                    //keep track of all splitters and their inital states
                    allSplitters.add (processor);
                    Splitter* sp = (Splitter*) processor;
                    splitterStates.add (sp->getPath());

                    processor->switchIO (0);
                }
            }

            // continue until the end of the chain
            LOGDD ("  Moving forward along signal chain.");
            processor = processor->getDestNode();
            isStartOfSignalChain = false;

            if (processor == nullptr)
            {
                if (splitPoints.size() > 0)
                {
                    LOGDD ("  Going back to first unswitched splitter.");

                    processor = splitPoints.getFirst();
                    splitPoints.remove (0);

                    processor->switchIO (1);

                    XmlElement* e = new XmlElement ("SWITCH");

                    e->setAttribute ("number", processor->saveOrder);

                    signalChain->addChildElement (e);
                }
                else
                {
                    LOGDD ("  End of chain.");
                }
            }
        }

        xml->addChildElement (signalChain);
    }

    // Loop through all splitters and reset their states to original values
    for (int i = 0; i < allSplitters.size(); i++)
    {
        allSplitters[i]->switchIO (splitterStates[i]);
    }

    AccessClass::getControlPanel()->saveStateToXml (xml); // save the control panel settings

    if (! isConsoleApp)
    {
        AccessClass::getEditorViewport()->saveEditorViewportSettingsToXml (xml);
        AccessClass::getGraphViewer()->saveStateToXml (xml); // save the graph viewer settings
        AccessClass::getDataViewport()->saveStateToXml (xml); // save the data viewport settings
        AccessClass::getProcessorList()->saveStateToXml (xml);
        AccessClass::getUIComponent()->saveStateToXml (xml); // save the UI settings
    }

    XmlElement* audioSettings = new XmlElement ("AUDIO");
    AccessClass::getAudioComponent()->saveStateToXml (audioSettings);
    xml->addChildElement (audioSettings);

    XmlElement* messageSettings = new XmlElement ("MESSAGES");
    AccessClass::getMessageCenter()->saveStateToXml (messageSettings);
    xml->addChildElement (messageSettings);

    //Resets Save Order for processors, allowing them to be saved again without omitting themselves from the order.
    int allProcessorSize = allProcessors.size();
    for (int i = 0; i < allProcessorSize; i++)
    {
        allProcessors.operator[] (i)->saveOrder = -1;
    }
}

XmlElement* ProcessorGraph::createNodeXml (GenericProcessor* processor, bool isStartOfSignalChain)
{
    XmlElement* xml = new XmlElement ("PROCESSOR");

    if (! isConsoleApp)
        xml->setAttribute ("name", processor->getEditor()->getName());

    if (isStartOfSignalChain)
        xml->setAttribute ("insertionPoint", 0);
    else
        xml->setAttribute ("insertionPoint", 1);
    xml->setAttribute ("pluginName", processor->getPluginName());
    xml->setAttribute ("type", (int) (processor->getPluginType()));
    xml->setAttribute ("index", processor->getIndex());
    xml->setAttribute ("libraryName", processor->getLibName());
    xml->setAttribute ("libraryVersion", processor->getLibVersion());
    xml->setAttribute ("processorType", (int) processor->getProcessorType());

    /** Saves individual processor parameters to XML */
    processor->saveToXml (xml);

    return xml;
}

void ProcessorGraph::loadFromXml (XmlElement* xml)
{
    Array<GenericProcessor*> splitPoints;

    if (! isConsoleApp)
    {
        MouseCursor::showWaitCursor();

        AccessClass::getUIComponent()->loadStateFromXml (xml); // load the UI settings first
        AccessClass::getProcessorList()->loadStateFromXml (xml); // load the processor list settings (may override theme colours)
    }

    clearSignalChain();

    isLoadingSignalChain = true; //Indicate config is being loaded into the GUI
    String description; // = " ";
    int loadOrder = 0;

    GenericProcessor* p;

    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName ("SIGNALCHAIN"))
        {
            processorArray.clear();
            for (auto* processor : element->getChildIterator())
            {
                if (processor->hasTagName ("PROCESSOR"))
                {
                    String pName = processor->getStringAttribute ("pluginName");

                    if (! isConsoleApp)
                    {
                        auto loadedPlugins = AccessClass::getProcessorList()->getItemList();

                        if (! loadedPlugins.contains (pName))
                        {
                            LOGC (pName, " plugin not found in Processor List! Looking for it on Artifactory...");

                            String libName = processor->getStringAttribute ("libraryName");
                            String libVer = processor->getStringAttribute ("libraryVersion");
                            libVer = libVer.isEmpty() ? "" : libVer + "-API" + String (PLUGIN_API_VER);

                            CoreServices::PluginInstaller::installPlugin (libName, libVer);
                        }
                    }

                    int insertionPt = processor->getIntAttribute ("insertionPoint");

                    LOGD ("Creating processor: ", pName);
                    p = createProcessorAtInsertionPoint (processor, insertionPt, false);
                    p->loadOrder = loadOrder++;

                    if (p->isSplitter())
                    {
                        splitPoints.add (p);
                    }
                }
                else if (processor->hasTagName ("SWITCH"))
                {
                    int processorNum = processor->getIntAttribute ("number");

                    LOGDD ("SWITCHING number ", processorNum);

                    for (int n = 0; n < splitPoints.size(); n++)
                    {
                        LOGDD ("Trying split point ", n, ", load order: ", splitPoints[n]->loadOrder);

                        if (splitPoints[n]->loadOrder == processorNum)
                        {
                            LOGDD ("Switching splitter destination.");
                            SplitterEditor* editor = (SplitterEditor*) splitPoints[n]->getEditor();
                            editor->switchDest (1);
                            AccessClass::getProcessorGraph()->updateViews (splitPoints[n]);

                            splitPoints.remove (n);
                        }
                    }
                }
            }
        }
        else if (element->hasTagName ("AUDIO"))
        {
            AccessClass::getAudioComponent()->loadStateFromXml (element);
            AccessClass::getControlPanel()->loadStateFromXml (xml); // load the control panel settings after the audio settings
        }
        else if (element->hasTagName ("MESSAGES"))
        {
            AccessClass::getMessageCenter()->loadStateFromXml (element);
        }
    }

    restoreParameters(); // loads the processor graph settings

    if (! isConsoleApp)
    {
        auto editorViewportXml = xml->getChildByName ("EDITORVIEWPORT");
        if (editorViewportXml != nullptr)
            AccessClass::getEditorViewport()->loadEditorViewportSettingsFromXml (editorViewportXml);

        refreshColours(); // refresh editor colours
        AccessClass::getDataViewport()->loadStateFromXml (xml);

        // load the graph viewer settings
        auto graphViewerXml = xml->getChildByName ("GRAPHVIEWER");
        if (graphViewerXml != nullptr)
            AccessClass::getGraphViewer()->loadStateFromXml (graphViewerXml);

        MouseCursor::hideWaitCursor();
    }

    isLoadingSignalChain = false;
}

Plugin::Description ProcessorGraph::getDescriptionFromXml (XmlElement* settings, bool ignoreNodeId)
{
    Plugin::Description description;

    description.fromProcessorList = false;
    description.name = settings->getStringAttribute ("pluginName");
    description.type = (Plugin::Type) settings->getIntAttribute ("type");
    description.processorType = (Plugin::Processor::Type) settings->getIntAttribute ("processorType");
    description.index = settings->getIntAttribute ("index");
    description.libName = settings->getStringAttribute ("libraryName");
    description.libVersion = settings->getStringAttribute ("libraryVersion");

    if (! ignoreNodeId)
        description.nodeId = settings->getIntAttribute ("nodeId");
    else
        description.nodeId = -1;

    return description;
}

void ProcessorGraph::createEmptyProcessor (GenericProcessor* destProcessor, int rootIndex, bool replace)
{
    Plugin::Description description;

    description.type = Plugin::Type::BUILT_IN;
    description.processorType = Plugin::Processor::EMPTY;
    description.index = -2;

    emptyProcessors.add (createProcessorFromDescription (description));
    emptyProcessors.getLast()->setDestNode (destProcessor);
    destProcessor->setSourceNode (emptyProcessors.getLast());
    emptyProcessors.getLast()->update();

    if (rootIndex > -1)
    {
        if (replace)
            rootNodes.set (rootIndex, emptyProcessors.getLast());
        else
            rootNodes.insert (rootIndex, emptyProcessors.getLast());
    }
    else
        rootNodes.add (emptyProcessors.getLast());
}

PluginManager* ProcessorGraph::getPluginManager() { return pluginManager.get(); }

GenericProcessor* ProcessorGraph::createProcessorAtInsertionPoint (XmlElement* parametersAsXml,
                                                                   int insertionPt,
                                                                   bool ignoreNodeId)
{
    if (isLoadingSignalChain)
    {
        if (insertionPt == 1)
        {
            insertionPoint = processorArray.size();
        }
        else
        {
            insertionPoint = 0;
        }
    }
    else
    {
        insertionPoint = insertionPt;
    }

    Plugin::Description description = getDescriptionFromXml (parametersAsXml, ignoreNodeId);

    GenericProcessor* source = nullptr;
    GenericProcessor* dest = nullptr;

    if (insertionPoint > 0)
    {
        source = processorArray[insertionPoint - 1];
    }

    if (processorArray.size() > insertionPoint)
    {
        dest = processorArray[insertionPoint];
    }

    GenericProcessor* processor = createProcessor (description,
                                                   source,
                                                   dest,
                                                   isLoadingSignalChain);

    if (processor->getPluginType() != Plugin::Type::INVALID)
        processor->parametersAsXml = parametersAsXml;

    return processor;
}
