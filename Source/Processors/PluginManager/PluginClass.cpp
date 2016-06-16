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

#include "PluginClass.h"
#include "PluginManager.h"
#include "../../AccessClass.h"
#include "../ProcessorManager/ProcessorManager.h"

PluginClass::PluginClass()
{
	libName = String::empty;
	pluginName = String::empty;
	libVersion = -1;
    pluginType = Plugin::NOT_A_PLUGIN_TYPE;
}

PluginClass::~PluginClass()
{

}

void PluginClass::setPluginData(Plugin::PluginType type, int index)
{
    PluginManager* pm = AccessClass::getPluginManager();
    String name;
    pluginType = type;
    pluginIndex = index;
    switch (type)
    {
        case Plugin::PLUGIN_TYPE_PROCESSOR:
        {
            Plugin::ProcessorInfo i = pm->getProcessorInfo(index);
            name = i.name;
            break;
        }

        case Plugin::PLUGIN_TYPE_RECORD_ENGINE:
        {
            Plugin::RecordEngineInfo i = pm->getRecordEngineInfo(index);
            name = i.name;
            break;
        }

        case Plugin::PLUGIN_TYPE_DATA_THREAD:
        {
            Plugin::DataThreadInfo i = pm->getDataThreadInfo(index);
            name = i.name;
            break;
        }

        case Plugin::PLUGIN_TYPE_FILE_SOURCE:
        {
            Plugin::FileSourceInfo i = pm->getFileSourceInfo(index);
            name = i.name;
            break;
        }

        case Plugin::NOT_A_PLUGIN_TYPE:
        {
            String pName;
            int pType;
            ProcessorManager::getProcessorNameAndType(BuiltInProcessor, index, pName, pType);
            name = pName;
            break;
        }

        default:
            return;
    }
    pluginName = name;
    libName = pm->getLibraryName(pm->getLibraryIndexFromPlugin(type, index));
    libVersion = pm->getLibraryVersion(pm->getLibraryIndexFromPlugin(type, index));
}

String PluginClass::getLibName() const
{
	return libName;
}

String PluginClass::getPluginName() const
{
	return pluginName;
}

int PluginClass::getLibVersion() const
{
	return libVersion;
}

Plugin::PluginType PluginClass::getPluginType() const
{
	return pluginType;
}

int PluginClass::getIndex() const
{
	return pluginIndex;
}
