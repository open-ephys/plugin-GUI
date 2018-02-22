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
#include "../../../Source/Processors/Parameter/Parameter.h"
#include "jucer_CodeHelpers.h"

#include <type_traits>

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

        case PROCESSOR_TYPE_SOURCE:
            return "Plugin::SourceProcessor";

        case PROCESSOR_TYPE_UTILITY:
        case PROCESSOR_TYPE_MERGER:
        case PROCESSOR_TYPE_SPLITTER:
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

// Parameter helpers
// ============================================================================
// ============================================================================
// ============================================================================

/** Returns the string reporesentation of an array. */
template<typename ValueType>
static String convertArrayToString (const Array<ValueType>& sourceArray)
{
    // Allow to convert only the arrays which store arithmetic types
    //if (! std::is_arithmetic<ValueType>::value)
    //{
    //    DBG ("Not Arithmetic");
    //    return String::empty;
    //}

    String stringRepr;
    for (auto value: sourceArray)
        stringRepr += value.toString() + ",";

    // Some hardcode - remove last coma at the end of the string
    stringRepr = stringRepr.dropLastCharacters (1);

    return stringRepr;
}


/** Creates and returns the array created from given string. Could be dangerous. */
template <typename ValueType>
static Array<var> createArrayFromString (const String& arrayString, const String& breakCharacters, const String& quoteCharacters = String::empty)
{
    // Allow to convert only the arrays which store arithmetic types
    if (! std::is_arithmetic<ValueType>::value)
       return Array<var> {};

    StringArray valuesStr;
    valuesStr.addTokens (arrayString, breakCharacters, quoteCharacters);

    const bool isBool   = std::is_same<ValueType, bool>::value;
    const bool isInt    = std::is_same<ValueType, int>::value;
    const bool isLong   = std::is_same<ValueType, long>::value;
    const bool isFloat  = std::is_same<ValueType, float>::value;
    const bool isDouble = std::is_same<ValueType, double>::value;

    Array<var> resultArray;
    for (const auto& line: valuesStr)
    {
        if (isBool)
            resultArray.add (line.getFloatValue());
        else if (isInt)
            resultArray.add (line.getIntValue());
        else if (isLong)
            resultArray.add (line.getLargeIntValue());
        else if (isFloat)
            resultArray.add (line.getFloatValue());
        else if (isDouble)
            resultArray.add (line.getDoubleValue());
        // If any another type - try to convert to int
        else
            resultArray.add (line.getIntValue());
    }

    return resultArray;
}


/** Returns the needed C++ code for a given parameter. */
static String generateCodeForParameter (const Parameter& parameter)
{
    String parameterCode;

    const auto id           = parameter.getID();
    const auto name         = parameter.getName();
    const auto defaultValue = parameter.getDefaultValue();

    // Add commentary to the parameter code
    parameterCode += CodeHelpers::indent (String ("// Parameter {PARAMINDEX} code"), 4, true) + newLine;

    // Generate code for file creation
    switch (parameter.getParameterType())
    {
        case Parameter::PARAMETER_TYPE_BOOLEAN:
        {
            const String defaultValueStr = bool (defaultValue) ? "true" : "false";
            String constructorCode = String ("auto parameter{PARAMINDEX} = new Parameter (\"[name]\", [defaultVal], [id]);")
                                       .replace ("[name]", name)
                                       .replace ("[defaultVal]", defaultValueStr)
                                       .replace ("[id]", String (id));
            constructorCode = CodeHelpers::indent (constructorCode, 4, true) + newLine;
            parameterCode << constructorCode;

            break;
        }

        case Parameter::PARAMETER_TYPE_CONTINUOUS:
        {
            Array<var> possibleValues = parameter.getPossibleValues();
            jassert (possibleValues.size() == 2);

            const float minPossibleValue = float (possibleValues[0]);
            const float maxPossibleValue = float (possibleValues[1]);

            String constructorCode = String ("auto parameter{PARAMINDEX} = new Parameter (\"[name]\", [minVal], [maxVal], [defaultVal], [id]);")
                                         .replace ("[name]", name)
                                         .replace ("[minVal]", String (minPossibleValue))
                                         .replace ("[maxVal]", String (maxPossibleValue))
                                         .replace ("[defaultVal]", defaultValue.toString())
                                         .replace ("[id]", String (id));
            constructorCode = CodeHelpers::indent (constructorCode, 4, true) + newLine;
            parameterCode << constructorCode;

            break;
        }

        case Parameter::PARAMETER_TYPE_DISCRETE:
        {
            auto possibleValues = parameter.getPossibleValues();

            auto possibleValuesStr = CodeHelpers::indent ("Array<var> parameter{PARAMINDEX}PossibleValues {"
                                                              + convertArrayToString (possibleValues)
                                                              + "};"
                                                              + newLine,
                                                          4, true);

            String constructorCode = String ("auto parameter{PARAMINDEX} = new Parameter (\"[name]\", [possibleValues], [defaultVal], [id]);")
                                         .replace ("[name]", name)
                                         .replace ("[possibleValues]", "parameter{PARAMINDEX}PossibleValues")
                                         .replace ("[defaultVal]", defaultValue.toString())
                                         .replace ("[id]", String (id));
            constructorCode = CodeHelpers::indent (constructorCode, 4, true) + newLine;
            parameterCode << possibleValuesStr << constructorCode;

            break;
        }
        case Parameter::PARAMETER_TYPE_NUMERICAL:
        {
            Array<var> possibleValues = parameter.getPossibleValues();
            jassert (possibleValues.size() == 2);

            const double minPossibleValue = double (possibleValues[0]);
            const double maxPossibleValue = double (possibleValues[1]);

            String constructorCode = String ("auto parameter{PARAMINDEX} = new Parameter (\"[name]\", \"[name]\", [minVal], [maxVal], [defaultVal], [id]);")
                                         .replace ("[name]", name)
                                         .replace ("[minVal]", String (minPossibleValue))
                                         .replace ("[maxVal]", String (maxPossibleValue))
                                         .replace ("[defaultVal]", defaultValue.toString())
                                         .replace ("[id]", String (id));
            constructorCode = CodeHelpers::indent (constructorCode, 4, true) + newLine;
            parameterCode << constructorCode;

            break;
        }
    };

    if (parameter.hasCustomEditorBounds())
    {
        auto convertBoundsToStr = [](const Rectangle<int>& bounds, const String& breakCharacters, const String& quoteCharacters)
            -> String
        {
            StringArray valuesStr;
            valuesStr.addTokens (bounds.toString(), breakCharacters, quoteCharacters);

            return valuesStr.joinIntoString (",");
        };

        String desiredBoundsCode = String ("parameter{PARAMINDEX}->setEditorDesiredBounds ([desiredBounds]);")
                                        .replace ("[desiredBounds]", convertBoundsToStr (parameter.getEditorDesiredBounds(), " ", String::empty));
        parameterCode << CodeHelpers::indent (desiredBoundsCode, 4, true);
        parameterCode << newLine;
    }

    if (parameter.getDescription().isNotEmpty())
    {
        String description = "String parameter{PARAMINDEX}description = \"" + parameter.getDescription() + "\";" + newLine
                                + String ("parameter{PARAMINDEX}->setDescription (parameter{PARAMINDEX}description);");
        parameterCode << CodeHelpers::indent (description, 4, true);
        parameterCode << newLine;
    }

    parameterCode << CodeHelpers::indent ("parameters.add (parameter{PARAMINDEX});", 4, true);
    parameterCode << newLine << newLine;

    return parameterCode;
}


/** Creates needed properties for a given parameter. */
static void createPropertiesForParameter (Parameter* parameter, Array<PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (parameter->getValueObjectForName(),         "Name", 50, false));
    props.add (new TextPropertyComponent (parameter->getValueObjectForDescription(),  "Description", 50, true));
    props.add (new TextPropertyComponent (parameter->getValueObjectForID(),           "Id", 6, false));
    props.add (new TextPropertyComponent (parameter->getValueObjectForDefaultValue(), "Default value", 6, false));

    if (parameter->isContinuous() || parameter->isNumerical())
    {
        props.add (new TextPropertyComponent (parameter->getValueObjectForMinValue(), "Minimum value", 6, false));
        props.add (new TextPropertyComponent (parameter->getValueObjectForMaxValue(), "Maximum value", 6, false));
    }
    else if (parameter->isDiscrete())
    {
        props.add (new TextPropertyComponent (parameter->getValueObjectForPossibleValues(), "Possible values", 500, true));
    }

    props.add (new TextPropertyComponent (parameter->getValueObjectForDesiredX(), "x", 4, false));
    props.add (new TextPropertyComponent (parameter->getValueObjectForDesiredY(), "y", 4, false));
    props.add (new TextPropertyComponent (parameter->getValueObjectForDesiredWidth(),  "width",  4, false));
    props.add (new TextPropertyComponent (parameter->getValueObjectForDesiredHeight(), "height", 4, false));
}


/** Adds needed properties for a given parameter to the appropritate properties panel. */
static void addPropertiesOfParameterToPropertyPanel (Parameter* parameter, PropertyPanel& propsPanel)
{
    jassert (parameter != nullptr);
    if (parameter == nullptr)
        return;

    Array<PropertyComponent*> props;
    createPropertiesForParameter (parameter, props);

    propsPanel.addSection ("Parameter", props);
}




// GUI Templates helpers
// ============================================================================
// ============================================================================
// ============================================================================

/** Returns the class name of the given template. */
static String getGUITemplateClassName (const String& templateName)
{
    return "OE_GUI_" + templateName;
}


/** Returns the content of the GUI template file. */
static String getGUITemplate (const String& templateName, bool isHeaderFile)
{
    String fileSuffix = isHeaderFile ? "_h" : "_cpp";
    String templateFileName = getGUITemplateClassName (templateName) + fileSuffix;

    int dataSize;
    const char* data = BinaryData::getNamedResource (templateFileName.toUTF8(), dataSize);

    if (data == nullptr)
    {
        jassertfalse;
        return String::empty;
    }

    return String::fromUTF8 (data, dataSize);
}


/** Checks if exists GUI template for the given template name. */
static bool isExistsGuiTemplate (const String& templateName)
{
    // No need to check for cpp file existence
    String fileSuffix = "_h";
    String templateFileName = getGUITemplateClassName (templateName) + fileSuffix;

    int dataSize;
    const char* data = BinaryData::getNamedResource (templateFileName.toUTF8(), dataSize);

    if (data == nullptr)
    {
        jassertfalse;
        return false;
    }

    return true;
}

#endif // __OPEN_EPHYS_PLUGIN_HELPERS__

