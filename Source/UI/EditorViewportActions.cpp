/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#include "EditorViewportActions.h"
#include "../Processors/Merger/Merger.h"
#include "../Processors/Splitter/Splitter.h"

AddProcessor::AddProcessor(Plugin::Description description_,
                           GenericProcessor* sourceProcessor,
                           GenericProcessor* destProcessor,
                           EditorViewport* editorViewport_,
                           bool signalChainIsLoading_) :
    description(description_),
    editorViewport(editorViewport_),
    signalChainIsLoading(signalChainIsLoading_)
{
    settings = nullptr;
    
    nodeId = -1;
    
    if (sourceProcessor != nullptr)
        sourceNodeId = sourceProcessor->getNodeId();
    else
        sourceNodeId = -1;
    
    if (destProcessor != nullptr)
        destNodeId = destProcessor->getNodeId();
    else
        destNodeId = -1;
}

AddProcessor::~AddProcessor()
{
    if (settings != nullptr)
        delete settings;
}
   
bool AddProcessor::perform()
{
    LOGDD("Performing ADD for processor ", nodeId);

    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(sourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(destNodeId);
    
    if (nodeId > -1)
        description.nodeId = nodeId;
    
    processor = AccessClass::getProcessorGraph()->createProcessor(description,
                                                      sourceProcessor,
                                                      destProcessor,
                                                      signalChainIsLoading);
    
    if (settings != nullptr && processor != nullptr)
    {
        processor->parametersAsXml = settings;
        processor->loadFromXml();
    }

    if (processor != nullptr && !signalChainIsLoading)
        processor->initialize(false);
    
    if (processor != nullptr)
    {
        nodeId = processor->getNodeId();
        return true;
    }
    else
        return false;
}

bool AddProcessor::undo()
{
    LOGD("Undoing ADD for processor ", nodeId);

    Array<GenericProcessor*> processorToDelete;
    
    processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    processorToDelete.add(processor);
    
    if (settings != nullptr)
        delete settings;
    
    if (processor != nullptr)
    {
        settings = editorViewport->createNodeXml(processor, false);

        AccessClass::getProcessorGraph()->deleteNodes(processorToDelete);
    }
       
    return true;
}
   

PasteProcessors::PasteProcessors(Array<XmlElement*> copyBuffer_,
    int insertionPoint_,
    EditorViewport* editorViewport_) :
    editorViewport(editorViewport_),
    insertionPoint(insertionPoint_),
    copyBuffer(copyBuffer_)
{

}

PasteProcessors::~PasteProcessors()
{

}

bool PasteProcessors::perform()
{
    Array<GenericProcessor*> newProcessors;
    
    for (int i = 0; i < copyBuffer.size(); i++)
    {
        newProcessors.add(editorViewport->createProcessorAtInsertionPoint(copyBuffer.getUnchecked(i),
            insertionPoint++, true));
    }

    for (auto p : newProcessors)
    {
        p->loadFromXml();
        nodeIds.add(p->getNodeId());
    }
        
    AccessClass::getProcessorGraph()->updateSettings(newProcessors[0]);

    // initialize in background thread if necessary
    for (auto p : newProcessors)
        p->initialize(false);

    return true;
}

bool PasteProcessors::undo()
{
    Array<GenericProcessor*> processorsToDelete;

    for (auto nodeId : nodeIds)
    {
        GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
        processorsToDelete.add(processor);
   
    }

    AccessClass::getProcessorGraph()->deleteNodes(processorsToDelete);

    nodeIds.clear();

    return true;
}


DeleteProcessor::DeleteProcessor(GenericProcessor* processor_, EditorViewport* editorViewport_)
{
    
    processor = processor_;
    editorViewport = editorViewport_;
    
    nodeId = processor->getNodeId();
    
    settings = editorViewport->createNodeXml(processor, false);
    
    if (processor->getSourceNode() != nullptr)
        sourceNodeId = processor->getSourceNode()->getNodeId();
    else
        sourceNodeId = -1;
    
    if (processor->getDestNode() != nullptr)
        destNodeId = processor->getDestNode()->getNodeId();
    else
        destNodeId = -1;
}

DeleteProcessor::~DeleteProcessor()
{
    delete settings;
}
   
bool DeleteProcessor::perform()
{
    LOGDD("Peforming DELETE for processor ", nodeId);

    Array<GenericProcessor*> processorToDelete;
    
    processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    processorToDelete.add(processor);
    
    AccessClass::getProcessorGraph()->deleteNodes(processorToDelete);
    
    
    return true;
}

bool DeleteProcessor::undo()
{
    LOGDD("Undoing DELETE for processor ", nodeId);

    Plugin::Description description = editorViewport->getDescriptionFromXml(settings, false);

    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(sourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(destNodeId);
    
    processor = AccessClass::getProcessorGraph()->createProcessor(description,
                                                      sourceProcessor,
                                                      destProcessor,
                                                      false);
    processor->parametersAsXml = settings;
    processor->loadFromXml();
    
    AccessClass::getProcessorGraph()->updateSettings(processor);
    
    if (processor != nullptr)
        return true;
    else
        return false;
}
   

MoveProcessor::MoveProcessor(GenericProcessor* processor,
                             GenericProcessor* sourceNode,
                             GenericProcessor* destNode,
                             bool moveDownstream_)
{
    
    nodeId = processor->getNodeId();
    
    moveDownstream = moveDownstream_;

    if (processor->getSourceNode() != nullptr)
        originalSourceNodeId = processor->getSourceNode()->getNodeId();
    else
        originalSourceNodeId = -1;
    
    if (processor->getDestNode() != nullptr)
    {
        originalDestNodeId = processor->getDestNode()->getNodeId();
        
        if (processor->getDestNode()->getDestNode() != nullptr)
            originalDestNodeDestNodeId = processor->getDestNode()->getDestNode()->getNodeId();
        else
            originalDestNodeDestNodeId = -1;
    }
    else
    {
        originalDestNodeId = -1;
        originalDestNodeDestNodeId = -1;
    }
        
    
    if (sourceNode != nullptr)
        newSourceNodeId = sourceNode->getNodeId();
    else
        newSourceNodeId = -1;
    
    if (destNode != nullptr)
        newDestNodeId = destNode->getNodeId();
    else
        newDestNodeId = -1;
    
    
}

MoveProcessor::~MoveProcessor()
{

}
   
bool MoveProcessor::perform()
{
    LOGD("Peforming MOVE for processor ", nodeId);
    
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    
    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(newSourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(newDestNodeId);
    
    AccessClass::getProcessorGraph()->moveProcessor(processor,
                                                    sourceProcessor,
                                                    destProcessor,
                                                    moveDownstream);
    
    return true;
}

bool MoveProcessor::undo()
{

    LOGD("Undoing MOVE for processor ", nodeId);

    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    
    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalSourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalDestNodeId);
    
    AccessClass::getProcessorGraph()->moveProcessor(processor,
                                                    sourceProcessor,
                                                    destProcessor,
                                                    !moveDownstream);
    
    if (processor->isSource() && originalDestNodeDestNodeId > -1)
    {
        GenericProcessor* originalDest = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalDestNodeDestNodeId);
        
        AccessClass::getProcessorGraph()->moveProcessor(originalDest,
                                                        destProcessor,
                                                        originalDest->getDestNode(),
                                                        !moveDownstream);
    }
    
    return true;
}
   

ClearSignalChain::ClearSignalChain(EditorViewport* editorViewport_)
{
    editorViewport = editorViewport_;
    
    settings = editorViewport->createSettingsXml();
}
    
 
ClearSignalChain::~ClearSignalChain()
{
}
    
bool ClearSignalChain::perform()
{
    LOGDD("Performing clear signal chain.");

    AccessClass::getProcessorGraph()->clearSignalChain();
    
    return true;
}

bool ClearSignalChain::undo()
{
    LOGDD("Undoing clear signal chain.");

    editorViewport->loadStateFromXml(settings.get());
    
    return true;
}



LoadSignalChain::LoadSignalChain(EditorViewport* editorViewport_, std::unique_ptr<XmlElement>& newSettings_)
{
    editorViewport = editorViewport_;
    newSettings = std::move(newSettings_);
    
    oldSettings = editorViewport->createSettingsXml();
}
    
 
LoadSignalChain::~LoadSignalChain()
{
    if (newSettings == nullptr)
        newSettings.release();
}
    
bool LoadSignalChain::perform()
{
    LOGDD("Performing load signal chain.");

    error = editorViewport->loadStateFromXml(newSettings.get());
    
    return true;
}

bool LoadSignalChain::undo()
{
    LOGDD("Undoing load signal chain.");

    error = editorViewport->loadStateFromXml(oldSettings.get());
    
    return true;
}

const String LoadSignalChain::getError()
{
    return String(error);
}

LoadPluginSettings::LoadPluginSettings(EditorViewport* editorViewport_,
                                       GenericProcessor* processor,
                                       XmlElement* newSettings_)
{
    editorViewport = editorViewport_;
    newSettings = newSettings_;
    
    oldSettings = editorViewport->createNodeXml(processor, false);
    
    processorId = processor->getNodeId();
    
}
    
 
LoadPluginSettings::~LoadPluginSettings()
{
    delete oldSettings;
}
    
bool LoadPluginSettings::perform()
{
    
    String oldPluginType = oldSettings->getStringAttribute("name");
    String newPluginType = newSettings->getStringAttribute("name");
    
    if (oldPluginType.equalsIgnoreCase(newPluginType))
    {
        LOGDD("Performing load plugin settings for processor ", processorId);
        LOGDD("Getting processor");
        GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
        
        
        LOGDD("Updating NodeId");
        newSettings->setAttribute("NodeId", processorId);
        LOGDD("Setting parameters");
        processor->parametersAsXml = newSettings;
        LOGDD("Loading parameters");
        processor->loadFromXml();
        
        CoreServices::updateSignalChain(processor->getEditor());
        
        return true;
    } else {
        
        CoreServices::sendStatusMessage("Plugin types do not match.");
        
        return false;
    }
    
}

bool LoadPluginSettings::undo()
{
    LOGDD("Undoing load plugin settings for processor ", processorId);
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->parametersAsXml = oldSettings;
    processor->loadFromXml();
    
    AccessClass::getProcessorGraph()->updateViews(processor);
    
    return true;
}


SwitchIO::SwitchIO(GenericProcessor* processor, int path)
{
    processorId = processor->getNodeId();
    
    originalPath = path;
    
}
 
SwitchIO::~SwitchIO()
{
    
}
    
bool SwitchIO::perform()
{
    LOGDD("Performing SwitchIO for processor ", processorId);

    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->getEditor()->switchIO(originalPath);
    
    AccessClass::getProcessorGraph()->updateViews(processor);
    
    return true;
}
    
bool SwitchIO::undo()
{

    LOGDD("Undoing SwitchIO for processor ", processorId);

    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->getEditor()->switchIO(1 - originalPath);
    
    AccessClass::getProcessorGraph()->updateViews(processor);
    
    return true;
}

