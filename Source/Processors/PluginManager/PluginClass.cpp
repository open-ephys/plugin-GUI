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
	libName = String();
	pluginName = String();
	libVersion = -1;
    pluginType = Plugin::INVALID;
}

PluginClass::~PluginClass()
{

}

void PluginClass::setPluginData(Plugin::Type type, int index)
{
    PluginManager* pm = AccessClass::getPluginManager();
    String name;
    pluginType = type;
    pluginIndex = index;
    switch (type)
    {
        case Plugin::PROCESSOR:
        {
            Plugin::ProcessorInfo i = pm->getProcessorInfo(index);
            name = i.name;
            break;
        }

        case Plugin::RECORD_ENGINE:
        {
            Plugin::RecordEngineInfo i = pm->getRecordEngineInfo(index);
            name = i.name;
            break;
        }

        case Plugin::DATA_THREAD:
        {
            Plugin::DataThreadInfo i = pm->getDataThreadInfo(index);
            name = i.name;
            break;
        }

        case Plugin::FILE_SOURCE:
        {
            Plugin::FileSourceInfo i = pm->getFileSourceInfo(index);
            name = i.name;
            break;
        }

        case Plugin::INVALID:
        {
            Plugin::Description description = ProcessorManager::getPluginDescription(Plugin::BUILT_IN, index);
            name = description.name;
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

Plugin::Type PluginClass::getPluginType() const
{
	return pluginType;
}

int PluginClass::getIndex() const
{
	return pluginIndex;
}
