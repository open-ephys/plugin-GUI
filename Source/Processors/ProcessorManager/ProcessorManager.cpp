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
		case 0:
			name = "File Reader";
			type = SourceProcessor;
			break;
		case 1:
			name = "Merger";
			type = UtilityProcessor;
			break;
		case 2:
			name = "Splitter";
			type = UtilityProcessor;
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
		switch (index)
		{
		case 0:
			return new FileReader();
			break;
		case 1:
			return new Merger();
			break;
		case 2:
			return new Splitter();
			break;
		default:
			return nullptr;
			break;
		}
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
				return info.creator();
				break;
			}
		case DataThreadProcessor:
		{
			Plugin::DataThreadInfo info = AccessClass::getPluginManager()->getDataThreadInfo(index);
			return new SourceNode(info.name, info.creator);
			break;
		}
				
		default:
			return nullptr;
		}
	}
};