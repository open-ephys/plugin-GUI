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
            return "Plugin::ProcessorPlugin";
        case PLUGIN_TYPE_RECORD_ENGINE:
            return "Plugin::RecordEnginePlugin";
        case PLUGIN_TYPE_DATA_THREAD:
            return "Plugin::DatathreadPlugin";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "Plugin::FileSourcePlugin";

        default:
            return "Plugin::NotAPlugin";
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
