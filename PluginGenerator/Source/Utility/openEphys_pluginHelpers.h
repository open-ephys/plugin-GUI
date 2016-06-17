/*
   ------------------------------------------------------------------

   This file is part of the Open Ephys GUI
   Copyright (C) 2016 Open Ephys

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

#ifndef __OPEN_EPHYS_PLUGIN_HELPERS__
#define __OPEN_EPHYS_PLUGIN_HELPERS__

#include "../../../Source/Processors/PluginManager/PluginIDs.h"

using namespace Plugin;


static String getProcessorTypeString (PluginProcessorType processorType)
{
    switch (processorType)
    {
        case PROCESSOR_TYPE_FILTER:
            return "PROCESSOR_TYPE_FILTER";
        case PROCESSOR_TYPE_SOURCE:
            return "PROCESSOR_TYPE_SOURCE";
        case PROCESSOR_TYPE_SINK:
            return "PROCESSOR_TYPE_SINK";
        case PROCESSOR_TYPE_SPLITTER:
            return "PROCESSOR_TYPE_SPLITTER";
        case PROCESSOR_TYPE_MERGER:
            return "PROCESSOR_TYPE_MERGER";
        case PROCESSOR_TYPE_UTILITY:
            return "PROCESSOR_TYPE_UTILITY";
        default:
            return "InvalidProcessorType";
    };
}


/** Return the string representation of a given plugin type */
static String getLibPluginTypeString (PluginType pluginType)
{
    switch (pluginType)
    {
        case PLUGIN_TYPE_PROCESSOR:
            return "Plugin::PLUGIN_TYPE_PROCESSOR";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "Plugin::PLUGIN_TYPE_RECORD_ENGINE";
        case PLUGIN_TYPE_DATA_THREAD:
            return "Plugin::PLUGIN_TYPE_DATA_THREAD";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "Plugin::PLUGIN_TYPE_FILE_SOURCE";

        default:
            return "Plugin::NOT_A_PLUGIN_TYPE";
    };
}


/** Returns the string representation (name) of the function which is used to create a plugin of a given type. */
static String getLibPluginCreateFunctionString (PluginType pluginType)
{
    switch (pluginType)
    {
        case PLUGIN_TYPE_PROCESSOR:
            return "createProcessor";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "createRecordEngine";
        case PLUGIN_TYPE_DATA_THREAD:
            return "createDataThread";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "createFileSource";
        default:
            return "InvalidFunctionName";
    };
}

/** Returns the string represenatation of a give plugin info type.
    E.g. plugins can have templates like: "info->processor", "info->fileSource" and etc. */
static String getLibPluginInfoType (PluginType pluginType)
{
    switch (pluginType)
    {
        case PLUGIN_TYPE_PROCESSOR:
            return "processor";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "recordEngine";
        case PLUGIN_TYPE_DATA_THREAD:
            return "dataThread";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "fileSource";
        default:
            return "InvalidPluginInfoType";
    };
}


/** Returns the string representation of a given plugin processor type. */
static String getLibProcessorTypeString (PluginProcessorType processorType)
{
    switch (processorType)
    {
        case PROCESSOR_TYPE_FILTER:
            return "Plugin::FilterProcessor";

        case PROCESSOR_TYPE_SINK:
            return "Plugin::SinkProcessor";

        case PROCESSOR_TYPE_UTILITY:
        case PROCESSOR_TYPE_MERGER:
        case PROCESSOR_TYPE_SPLITTER:
        case PROCESSOR_TYPE_SOURCE:
            return "Plugin::UtilityProcessor";

        default:
            return "Plugin::InvalidProcessor";
    };
}


/** Returns the string with the human friendly name for plugin type. */
static String getPluginTypeHumanReadableName (PluginType pluginType)
{
    switch (pluginType)
    {
        case PLUGIN_TYPE_PROCESSOR:
            return "Processor";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "Record Engine";
        case PLUGIN_TYPE_DATA_THREAD:
            return "Data Thread";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "File Source";
        default:
            return "InvalidPluginInfoType";
    };
}


/** Returns the string with the human friendly name for processor type. */
static String getProcessorTypeHumanReadableName (PluginProcessorType processorType)
{
    switch (processorType)
    {
        case PROCESSOR_TYPE_FILTER:
            return "Filter";
        case PROCESSOR_TYPE_SOURCE:
            return "Source";
        case PROCESSOR_TYPE_SINK:
            return "Sink";
        case PROCESSOR_TYPE_UTILITY:
            return "Utility";
        case PROCESSOR_TYPE_MERGER:
            return "Merger";
        case PROCESSOR_TYPE_SPLITTER:
            return "Splitter";
        default:
            return "Invalid processor";
    };
}


// ============================================================================
// ============================================================================
// ============================================================================

/** Returns the name of the file which contains template of the processor of a plugin of given type */
static String getTemplateProcessorFileName (PluginType pluginType)
{
    switch (pluginType)
    {
        case PLUGIN_TYPE_PROCESSOR:
            return "openEphys_ProcessorPluginTemplate";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "openEphys_RecordEnginePluginTemplate";
        case PLUGIN_TYPE_DATA_THREAD:
            return "openEphys_DataThreadPluginTemplate";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "openEphys_FileSourcePluginTemplate";
        default:
            return "InvalidFileName";
    };
}


#endif // __OPEN_EPHYS_PLUGIN_HELPERS__

