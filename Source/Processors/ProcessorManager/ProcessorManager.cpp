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

#include "../PlaceholderProcessor/PlaceholderProcessor.h"

/** Total number of builtin processors **/
#define BUILTIN_PROCESSORS 3

namespace ProcessorManager
{
	using namespace Plugin;
	/** Built-in processor names **/
	void getBuiltInProcessorNameAndType(int index, String& name, int& type)
	{
		switch (index)
		{
		case -1:
			name = "Placeholder processor";
			type = UtilityProcessor;
			break;
		case 0:
			name = "Merger";
			type = UtilityProcessor;
			break;
		case 1:
			name = "Splitter";
			type = UtilityProcessor;
			break;
		case 2:
			name = "File Reader";
			type = SourceProcessor;
			break;
		default:
			name = String::empty;
			type = -1;
			break;
		}
	}

	/** Built-in constructors **/
	GenericProcessor* createBuiltInProcessor(int index)
	{
		GenericProcessor* proc;
		switch (index)
		{
		case -1:
			proc = new PlaceholderProcessor("Empty placeholder", "Undefined", 0, false, false);
			break;
		case 0:
			proc = new Merger();
			break;
		case 1:
			proc = new Splitter();
			break;
		case 2:
			proc = new FileReader();
			break;
		default:
			return nullptr;
		}
		proc->setPluginData(Plugin::NOT_A_PLUGIN_TYPE, index);
		return proc;
	}

	int getNumProcessors(ProcessorClasses pClass)
	{
		switch (pClass)
		{
		case BuiltInProcessor:
			return BUILTIN_PROCESSORS;
			break;
		case PluginProcessor:
			return AccessClass::getPluginManager()->getNumProcessors();
			break;
		case DataThreadProcessor:
			return AccessClass::getPluginManager()->getNumDataThreads();
		default:
			return 0;
			break;
		}
	}

	void getProcessorNameAndType(ProcessorClasses pClass, int index, String& name, int& type)
	{
		switch (pClass)
		{
		case BuiltInProcessor:
			getBuiltInProcessorNameAndType(index, name, type);
			break;
		case PluginProcessor:
			{
				Plugin::ProcessorInfo info = AccessClass::getPluginManager()->getProcessorInfo(index);
				name = info.name;
				type = info.type;
			}
			break;
		case DataThreadProcessor:
		{
			Plugin::DataThreadInfo info = AccessClass::getPluginManager()->getDataThreadInfo(index);
			name = info.name;
			type = SourceProcessor;
			break;
		}
		default:
			name = String::empty;
			type = -1;
			break;
		}
	}

	GenericProcessor* createProcessor(ProcessorClasses pClass, int index)
	{
		switch (pClass)
		{
		case BuiltInProcessor:
			return createBuiltInProcessor(index);
			break;
		case PluginProcessor:
			{
				Plugin::ProcessorInfo info = AccessClass::getPluginManager()->getProcessorInfo(index);
				GenericProcessor* proc = info.creator();
				proc->setPluginData(Plugin::PLUGIN_TYPE_PROCESSOR, index);
				return proc;
				break;
			}
		case DataThreadProcessor:
		{
			Plugin::DataThreadInfo info = AccessClass::getPluginManager()->getDataThreadInfo(index);
			GenericProcessor* proc = new SourceNode(info.name, info.creator);
			proc->setPluginData(Plugin::PLUGIN_TYPE_DATA_THREAD, index);
			return proc;
			break;
		}
				
		default:
			return nullptr;
		}
	}

	GenericProcessor* createProcessorFromPluginInfo(Plugin::PluginType type, int index, String procName, String libName, int libVersion, bool source, bool sink)
	{
		PluginManager* pm = AccessClass::getPluginManager();
		GenericProcessor* proc = nullptr;
		if (index > -1)
		{
			if (type == Plugin::NOT_A_PLUGIN_TYPE)
			{
				return createBuiltInProcessor(index);
			}
			else if (type == Plugin::PLUGIN_TYPE_PROCESSOR)
			{
				for (int i = 0; i < pm->getNumProcessors(); i++)
				{
					Plugin::ProcessorInfo info = pm->getProcessorInfo(i);
					if (procName.equalsIgnoreCase(info.name))
					{
						int libIndex = pm->getLibraryIndexFromPlugin(Plugin::PLUGIN_TYPE_PROCESSOR, i);
						if (libName.equalsIgnoreCase(pm->getLibraryName(libIndex)) && libVersion == pm->getLibraryVersion(libIndex))
						{
							proc = info.creator();
							proc->setPluginData(Plugin::PLUGIN_TYPE_PROCESSOR, i);
							return proc;
						}
					}
				}
			}
			else if (type == Plugin::PLUGIN_TYPE_DATA_THREAD)
			{
				for (int i = 0; i < pm->getNumDataThreads(); i++)
				{
					Plugin::DataThreadInfo info = pm->getDataThreadInfo(i);
					if (procName.equalsIgnoreCase(info.name))
					{
						int libIndex = pm->getLibraryIndexFromPlugin(Plugin::PLUGIN_TYPE_DATA_THREAD, i);
						if (libName.equalsIgnoreCase(pm->getLibraryName(libIndex)) && libVersion == pm->getLibraryVersion(libIndex))
						{
							proc = new SourceNode(info.name, info.creator);
							proc->setPluginData(Plugin::PLUGIN_TYPE_DATA_THREAD, i);
							return proc;
						}
					}
				}
			}
		}		
		proc = new PlaceholderProcessor(procName, libName, libVersion, source, sink);
		proc->setPluginData(Plugin::NOT_A_PLUGIN_TYPE, -1);
		return proc;
	}
};
