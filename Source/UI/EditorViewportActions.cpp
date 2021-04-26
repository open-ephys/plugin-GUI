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

AddProcessor::AddProcessor(ProcessorDescription description_,
                           GenericProcessor* sourceProcessor,
                           GenericProcessor* destProcessor,
                           EditorViewport* editorViewport_)
{
    description = description_;
    
    settings = nullptr;
    
    editorViewport = editorViewport_;
    
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
    LOGDD("Performing add processor.");
    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(sourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(destNodeId);
    
    if (nodeId > -1)
        description.nodeId = nodeId;
    
    processor = AccessClass::getProcessorGraph()->createProcessor(description,
                                                      sourceProcessor,
                                                      destProcessor,
                                                      false);
    
    nodeId = processor->getNodeId();
    
    if (settings != nullptr && processor != nullptr)
    {
        processor->parametersAsXml = settings;
        processor->loadFromXml();
    }
    
    if (processor != nullptr)
        return true;
    else
        return false;
}

bool AddProcessor::undo()
{
    LOGDD("Undoing add processor.");
    Array<GenericProcessor*> processorToDelete;
    
    processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    processorToDelete.add(processor);
    
    if (settings != nullptr)
        delete settings;
    
    settings = editorViewport->createNodeXml(processor, false);
    
    AccessClass::getProcessorGraph()->deleteNodes(processorToDelete);
    
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
    LOGDD("Peforming delete processor.");
    Array<GenericProcessor*> processorToDelete;
    
    processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    processorToDelete.add(processor);
    
    AccessClass::getProcessorGraph()->deleteNodes(processorToDelete);
    
    return true;
}

bool DeleteProcessor::undo()
{
    LOGDD("Undoing delete processor.");
    ProcessorDescription description = editorViewport->getDescriptionFromXml(settings, false, false);

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
    LOGDD("Peforming move processor.");
    
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
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(nodeId);
    
    LOGDD("Undoing move processor.");
    GenericProcessor* sourceProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalSourceNodeId);
    
    GenericProcessor* destProcessor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalDestNodeId);
    
    AccessClass::getProcessorGraph()->moveProcessor(processor,
                                                    sourceProcessor,
                                                    destProcessor,
                                                    moveDownstream);
    
    if (processor->isSource() && originalDestNodeDestNodeId > -1)
    {
        GenericProcessor* originalDest = AccessClass::getProcessorGraph()->getProcessorWithNodeId(originalDestNodeDestNodeId);
        
        AccessClass::getProcessorGraph()->moveProcessor(originalDest,
                                                        destProcessor,
                                                        originalDest->getDestNode(),
                                                        moveDownstream);
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
    delete settings;
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
    editorViewport->loadStateFromXml(settings);
    
    return true;
}



LoadSignalChain::LoadSignalChain(EditorViewport* editorViewport_, XmlElement* newSettings_)
{
    editorViewport = editorViewport_;
    newSettings = newSettings_;
    
    oldSettings = editorViewport->createSettingsXml();
}
    
 
LoadSignalChain::~LoadSignalChain()
{
    delete oldSettings;
    delete newSettings;
}
    
bool LoadSignalChain::perform()
{
    LOGDD("Performing load signal chain.");
    error = editorViewport->loadStateFromXml(newSettings);
    
    return true;
}

bool LoadSignalChain::undo()
{
    LOGDD("Undoing load signal chain.");
    error = editorViewport->loadStateFromXml(oldSettings);
    
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
    delete newSettings;
}
    
bool LoadPluginSettings::perform()
{
    
    String oldPluginType = oldSettings->getStringAttribute("name");
    String newPluginType = newSettings->getStringAttribute("name");
    
    if (oldPluginType.equalsIgnoreCase(newPluginType))
    {
        LOGDD("Performing load plugin settings.");
        LOGDD("Getting processor");
        GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
        
        
        LOGDD("Updating NodeId");
        newSettings->setAttribute("NodeId", processorId);
        LOGDD("Setting parameters");
        processor->parametersAsXml = newSettings;
        LOGDD("Loading parameters");
        processor->loadCustomParametersFromXml();

        // need to replicate internals of loadFromXml(), because this can't be called twice for the
        // the same processor
        
        // load editor parameters
        forEachXmlChildElement(*newSettings, xmlNode)
        {
            if (xmlNode->hasTagName("EDITOR"))
            {
                processor->getEditor()->loadEditorParameters(xmlNode);
            }
        }

        forEachXmlChildElement(*newSettings, xmlNode)
        {
            if (xmlNode->hasTagName("CHANNEL"))
            {
                processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::DATA_CHANNEL);
            }
            else if (xmlNode->hasTagName("EVENTCHANNEL"))
            {
                processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::EVENT_CHANNEL);
            }
            else if (xmlNode->hasTagName("SPIKECHANNEL"))
            {
                processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::SPIKE_CHANNEL);
            }
        }
        
        AccessClass::getProcessorGraph()->updateViews(processor);
        
        return true;
    } else {
        
        CoreServices::sendStatusMessage("Plugin types do not match.");
        
        return false;
    }
    
}

bool LoadPluginSettings::undo()
{
    LOGDD("Undoing load plugin settings.");
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->parametersAsXml = oldSettings;
    processor->loadCustomParametersFromXml();

    // load editor parameters
    forEachXmlChildElement(*oldSettings, xmlNode)
    {
        if (xmlNode->hasTagName("EDITOR"))
        {
            processor->getEditor()->loadEditorParameters(xmlNode);
        }
    }

    forEachXmlChildElement(*oldSettings, xmlNode)
    {
        if (xmlNode->hasTagName("CHANNEL"))
        {
            processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::DATA_CHANNEL);
        }
        else if (xmlNode->hasTagName("EVENTCHANNEL"))
        {
            processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::EVENT_CHANNEL);
        }
        else if (xmlNode->hasTagName("SPIKECHANNEL"))
        {
            processor->loadChannelParametersFromXml(xmlNode, InfoObjectCommon::SPIKE_CHANNEL);
        }
    }
    
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
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->getEditor()->switchIO(originalPath);
    
    AccessClass::getProcessorGraph()->updateViews(processor);
    
    return true;
}
    
bool SwitchIO::undo()
{
    GenericProcessor* processor = AccessClass::getProcessorGraph()->getProcessorWithNodeId(processorId);
    
    processor->getEditor()->switchIO(1 - originalPath);
    
    AccessClass::getProcessorGraph()->updateViews(processor);
    
    return true;
}

