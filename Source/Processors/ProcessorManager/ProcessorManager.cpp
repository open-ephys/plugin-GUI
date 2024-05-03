/*
	 ------------------------------------------------------------------

	 This file is part of the Open Ephys GUI
	 Copyright (C) 2013 Open Ephys

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

#include "ProcessorManager.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../../AccessClass.h"
#include "../PluginManager/PluginManager.h"
#include "../SourceNode/SourceNode.h"

/** Includes for builtin processors **/
#include "../FileReader/FileReader.h"
#include "../Merger/Merger.h"
#include "../Splitter/Splitter.h"
#include "../AudioMonitor/AudioMonitor.h"
#include "../RecordNode/RecordNode.h"
#include "../EventTranslator/EventTranslator.h"

#include "../PlaceholderProcessor/PlaceholderProcessor.h"
#include "../EmptyProcessor/EmptyProcessor.h"

/** Total number of built-in processors **/
#define BUILT_IN_PROCESSOR_COUNT 6

namespace ProcessorManager
{

    Array<Plugin::Type> getAvailablePluginTypes()
    {
        return {Plugin::BUILT_IN,
                Plugin::PROCESSOR,
                Plugin::DATA_THREAD};
    }

	/** Built-in processor names **/
	Plugin::Description getBuiltInPluginDescription(int index)
	{
        Plugin::Description description;
        
        description.type = Plugin::BUILT_IN;
        
		switch (index)
		{
		case -2:
			description.name = "Empty Processor";
			description.processorType = Plugin::Processor::EMPTY;
			break;
        case -1:
			description.name = "Placeholder Processor";
			description.processorType = Plugin::Processor::UTILITY;
			break;
		case 0:
			description.name = "Merger";
			description.processorType = Plugin::Processor::UTILITY;
			break;
		case 1:
			description.name = "Splitter";
			description.processorType = Plugin::Processor::UTILITY;
			break;
		case 2:
			description.name = "File Reader";
			description.processorType = Plugin::Processor::SOURCE;
			break;
		case 3:
			description.name = "Record Node";
			description.processorType = Plugin::Processor::RECORD_NODE;
			break;
		case 4:
			description.name = "Audio Monitor";
			description.processorType = Plugin::Processor::UTILITY;
			break;
        case 5:
            description.name = "Event Translator";
            description.processorType = Plugin::Processor::UTILITY;
            break;
		default:
			description.name = String();
			description.processorType = Plugin::Processor::INVALID;
			break;
		}
        
        return description;
	}

	/** Built-in constructors **/
	std::unique_ptr<GenericProcessor> createBuiltInProcessor(int index)
	{
		GenericProcessor* proc;
		switch (index)
		{
        case -2:
			proc = new EmptyProcessor();
            proc->setProcessorType(Plugin::Processor::EMPTY);
			break;
		case -1:
			proc = new PlaceholderProcessor("Empty placeholder", "Undefined", "0.0.0");
            proc->setProcessorType(Plugin::Processor::INVALID);
			break;
		case 0:
			proc = new Merger();
            proc->setProcessorType(Plugin::Processor::MERGER);
			break;
		case 1:
			proc = new Splitter();
            proc->setProcessorType(Plugin::Processor::SPLITTER);
			break;
		case 2:
			proc = new FileReader();
            proc->setProcessorType(Plugin::Processor::SOURCE);
			break;
		case 3:
			proc = new RecordNode();
            proc->setProcessorType(Plugin::Processor::RECORD_NODE);
			break;
		case 4:
			proc = new AudioMonitor();
            proc->setProcessorType(Plugin::Processor::AUDIO_MONITOR);
			break;
        case 5:
            proc = new EventTranslator();
            proc->setProcessorType(Plugin::Processor::UTILITY);
            break;
		default:
			return nullptr;
		}
		proc->setPluginData(Plugin::BUILT_IN, index);
        
		return std::unique_ptr<GenericProcessor>(proc);
	}

	int getNumProcessorsForPluginType(Plugin::Type type)
	{
		switch (type)
		{
        case Plugin::BUILT_IN:
			return BUILT_IN_PROCESSOR_COUNT;
			break;
        case Plugin::PROCESSOR:
			return AccessClass::getPluginManager()->getNumProcessors();
			break;
        case Plugin::DATA_THREAD:
			return AccessClass::getPluginManager()->getNumDataThreads();
		default:
			return 0;
			break;
		}
	}

	Plugin::Description getPluginDescription(Plugin::Type type,
                                             int index)
	{
        
        Plugin::Description description;
        description.type = type;
        
		switch (type)
		{
            case Plugin::BUILT_IN:
            {
                description = getBuiltInPluginDescription(index);
                break;
            }
            case Plugin::PROCESSOR:
            {
                Plugin::ProcessorInfo info = AccessClass::getPluginManager()->getProcessorInfo(index);
                description.name = info.name;
                description.processorType = info.type;
                break;
            }
            case Plugin::DATA_THREAD:
            {
                Plugin::DataThreadInfo info = AccessClass::getPluginManager()->getDataThreadInfo(index);
                description.name = info.name;
                description.processorType = Plugin::Processor::SOURCE;
                break;
            }
            default:
            {
                description.name = String();
                description.type = Plugin::INVALID;
                break;
            }
		}

        if (description.type != Plugin::INVALID)
        {
            description.fromProcessorList = true;
            description.index = index;
        }
        
        return description;
	}

    Plugin::Description getPluginDescription(String name)
    {
        Plugin::Description description;

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::BUILT_IN); i++)
        {
            description = getPluginDescription(Plugin::BUILT_IN, i);
            if (description.name.equalsIgnoreCase(name))
            {
                return description;
            }
        }

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::PROCESSOR); i++)
        {
            description = getPluginDescription(Plugin::PROCESSOR, i);
            if (description.name.equalsIgnoreCase(name))
            {
                return description;
            }
        }

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::DATA_THREAD); i++)
        {
            description = getPluginDescription(Plugin::DATA_THREAD, i);
            if (description.name.equalsIgnoreCase(name))
            {
                return description;
            }
        }

        description.name = String();
        description.type = Plugin::INVALID;
        return description;
    }

	std::unique_ptr<GenericProcessor> createProcessor(Plugin::Description description)
	{
        
        if (false)
        {
            std::cout << "DESCRIPTION: " << std::endl;
            std::cout << "From processor list: " << description.fromProcessorList << std::endl;
            std::cout << "Name: " << description.name << std::endl;
            std::cout << "Index: " << description.index << std::endl;
            std::cout << "Type: " << description.type << std::endl;
            std::cout << "ProcessorType: " << description.processorType << std::endl;
            std::cout << "NodeId: " << description.nodeId << std::endl;
            std::cout << "LibName: " << description.libName << std::endl;
            std::cout << "LibVersion: " << description.libVersion << std::endl;
        }
                
        if (description.fromProcessorList)
        {
            switch (description.type)
            {
            case Plugin::BUILT_IN:
            {
                return createBuiltInProcessor(description.index);
            }
            case Plugin::PROCESSOR:
            {
                Plugin::ProcessorInfo info = AccessClass::getPluginManager()->getProcessorInfo(description.index);
                GenericProcessor* proc = info.creator();
                proc->setPluginData(Plugin::PROCESSOR, description.index);
                proc->setProcessorType(description.processorType);
                return std::unique_ptr<GenericProcessor>(proc);
            }
            case Plugin::DATA_THREAD:
            {
                Plugin::DataThreadInfo info = AccessClass::getPluginManager()->getDataThreadInfo(description.index);
                GenericProcessor* proc = new SourceNode(info.name, info.creator);
                proc->setPluginData(Plugin::DATA_THREAD, description.index);
                proc->setProcessorType(Plugin::Processor::SOURCE);
                return std::unique_ptr<GenericProcessor>(proc);
            }
            default:
                return nullptr;
            }
                
        } // if (description.fromProcessorList)
            
        PluginManager* pm = AccessClass::getPluginManager();
        
        GenericProcessor* proc = nullptr;
        
        if (description.index > -1)
        {
            if (description.type == Plugin::BUILT_IN)
            {
                return createBuiltInProcessor(description.index);
            }
            else if (description.type == Plugin::PROCESSOR)
            {
                for (int i = 0; i < pm->getNumProcessors(); i++)
                {
                    Plugin::ProcessorInfo info = pm->getProcessorInfo(i);
                    
                    if (description.name.equalsIgnoreCase(info.name))
                    {
                        int libIndex = pm->getLibraryIndexFromPlugin(Plugin::PROCESSOR, i);
                        
                        if (description.libName.equalsIgnoreCase(pm->getLibraryName(libIndex)))
                        {
                            proc = info.creator();
                            proc->setPluginData(Plugin::PROCESSOR, i);
                            proc->setProcessorType(description.processorType);
                            return std::unique_ptr<GenericProcessor>(proc);
                        }
                    }
                }
            }
            else if (description.type == Plugin::DATA_THREAD)
            {
                for (int i = 0; i < pm->getNumDataThreads(); i++)
                {
                    Plugin::DataThreadInfo info = pm->getDataThreadInfo(i);
                    if (description.name.equalsIgnoreCase(info.name))
                    {
                        int libIndex = pm->getLibraryIndexFromPlugin(Plugin::DATA_THREAD, i);
                        if (description.libName.equalsIgnoreCase(pm->getLibraryName(libIndex)))
                        {
                            proc = new SourceNode(info.name, info.creator);
                            proc->setPluginData(Plugin::DATA_THREAD, i);
                            return std::unique_ptr<GenericProcessor>(proc);
                        }
                    }
                }
            }
        } // if (description.index > -1)
        else if (description.index == -1)
        {
            proc = new PlaceholderProcessor(description.name,
                                            description.libName,
                                            description.libVersion);
            proc->setPluginData(Plugin::INVALID, -1);
            proc->setProcessorType(description.processorType);
            return std::unique_ptr<GenericProcessor>(proc);
        }
        else if (description.index == -2)
        {
            proc = new EmptyProcessor();
            proc->setPluginData(description.type, description.index);
            proc->setProcessorType(Plugin::Processor::EMPTY);
            return std::unique_ptr<GenericProcessor>(proc);
        }
        
        return std::unique_ptr<GenericProcessor>(proc);

	} // createProcessor(Plugin::Description description)

    Array<String> getAvailableProcessors()
    {
        Array<String> availableProcessors;

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::BUILT_IN); i++)
        {
            Plugin::Description description = getPluginDescription(Plugin::BUILT_IN, i);
            availableProcessors.add(description.name);
        }

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::PROCESSOR); i++)
        {
            Plugin::Description description = getPluginDescription(Plugin::PROCESSOR, i);
            availableProcessors.add(description.name);
        }

        for (int i = 0; i < getNumProcessorsForPluginType(Plugin::DATA_THREAD); i++)
        {
            Plugin::Description description = getPluginDescription(Plugin::DATA_THREAD, i);
            availableProcessors.add(description.name);
        }

        return availableProcessors;
    }

}; // namespace

