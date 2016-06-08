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


#include <iostream>
//#include "../../../Source/Processors/GenericProcessor/GenericProcessor.h"
enum PluginProcessorType
{
    PROCESSOR_TYPE_FILTER = 1
    , PROCESSOR_TYPE_SOURCE
    , PROCESSOR_TYPE_SINK
    , PROCESSOR_TYPE_SPLITTER
    , PROCESSOR_TYPE_MERGER
    , PROCESSOR_TYPE_UTILITY
    , PROCESSOR_TYPE_DATA_FORMAT
};

enum PluginType
{
    PLUGIN_TYPE_PROCESSOR = 1
    , PLUGIN_TYPE_RECORD_ENGINE
    , PLUGIN_TYPE_DATA_THREAD
    , PLUGIN_TYPE_FILE_SOURCE
    , NOT_A_PLUGIN_TYPE = -1
};


static ComboBox& createPluginTypeOptionComboBox (Component& setupComp,
                                                 OwnedArray<Component>& itemsCreated,
                                                 const StringArray& fileOptions)
{
    ComboBox* pluginTypeComboBox = new ComboBox;
    itemsCreated.add (pluginTypeComboBox);
    setupComp.addChildAndSetID (pluginTypeComboBox, OpenEphysPluginAppWizard::COMBOBOX_ID_PLUGIN_TYPE);

    pluginTypeComboBox->addItemList (fileOptions, 1);
    pluginTypeComboBox->setSelectedId (1, dontSendNotification);

    Label* l = new Label (String::empty, TRANS("Plugin type") + ":");
    l->attachToComponent (pluginTypeComboBox, true);
    itemsCreated.add (l);

    return *pluginTypeComboBox;
}


static ComboBox& createProcessorTypeOptionComboBox (Component& setupComp,
                                                    OwnedArray<Component>& itemsCreated,
                                                    const StringArray& fileOptions)
{
    ComboBox* processorTypeComboBox = new ComboBox;
    itemsCreated.add (processorTypeComboBox);
    setupComp.addChildAndSetID (processorTypeComboBox, OpenEphysPluginAppWizard::COMBOBOX_ID_PROCESSOR_TYPE);

    processorTypeComboBox->addItemList (fileOptions, 1);
    processorTypeComboBox->setSelectedId (1, dontSendNotification);

    Label* l = new Label (String::empty, TRANS("Processor type") + ":");
    l->attachToComponent (processorTypeComboBox, true);
    itemsCreated.add (l);

    //c->setBounds ("parent.width / 2 + 160, 30, parent.width - 30, top + 22");

    return *processorTypeComboBox;
}


static int getComboResult (WizardComp& setupComp, const String& comboBoxID)
{
    if (ComboBox* cb = dynamic_cast<ComboBox*> (setupComp.findChildWithID (comboBoxID)))
        return cb->getSelectedItemIndex();

    jassertfalse;
    return 0;
}


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
        case PROCESSOR_TYPE_DATA_FORMAT:
            return "PROCESSOR_TYPE_DATA_FORMAT";
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
            return "Plugin::RecordEngine";
        case PLUGIN_TYPE_DATA_THREAD:
            return "Plugin::DatathreadPlugin";
        case PLUGIN_TYPE_FILE_SOURCE:
            return "Plugin::FileSourcePlugin";

        default:
            return "Plugin::NotAPlugin";
        //case PROCESSOR_TYPE_FILTER:
        //case PROCESSOR_TYPE_SINK:
        //case PROCESSOR_TYPE_SOURCE:
        //case PROCESSOR_TYPE_SPLITTER:
        //case PROCESSOR_TYPE_MERGER:
        //case PROCESSOR_TYPE_UTILITY:
        //    // TODO <Kirill A>: Check where data format plugins should be placed.
        //case PROCESSOR_TYPE_DATA_FORMAT:
        //    return "Plugin::ProcessorPlugin";

        //default:
        //    return "Plugin::NotAPlugin";

        //    case anotherCase:
        //        return "RecordEnginePlugin";
        //    case anotherCase2:
        //        return "DatathreadPlugin";
        //    case anotherCase3:
        //        return "FileSourcePlugin";
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
    }
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
            // TODO <Kirill A>: Check where data format plugins should be placed.
        case PROCESSOR_TYPE_DATA_FORMAT:
            return "Plugin::UtilityProcessor";

        default:
            return "Plugin::InvalidProcessor";
    };
}

// =================================================================================


struct OpenEphysPluginAppWizard   : public NewProjectWizard
                                  , public ComboBox::Listener
{
    OpenEphysPluginAppWizard()  {}

    String getName() const override         { return TRANS("Open-Ephys Plug-In"); }
    String getDescription() const override  { return TRANS("Creates an Open-Ephys neuro plug-in."); }
    const char* getIcon() const override    { return BinaryData::wizard_AudioPlugin_svg; }

    StringArray getDefaultModules() override
    {
        //StringArray s (NewProjectWizard::getDefaultModules());
        //s.add ("juce_audio_plugin_client");

        // TODO <Kirill A>
        // Just an empty modules for now
        StringArray s;
        return s;
    }

    void addSetupItems (Component& setupComp, OwnedArray<Component>& itemsCreated) override
    {
        //juce::Rectangle<int> comboBoxDefaultBounds ("parent.width / 2 + 160, 30, parent.width - 30, top + 22");
        juce::Rectangle<int> comboBoxDefaultBounds (setupComp.getWidth() / 2 + 95,
                                                    30,
                                                    setupComp.getWidth() / 2 - 30,
                                                    setupComp.getY() + 22);

        const int marginBetweenComboBoxes = 150;
        const int comboBoxMinWidth        = (comboBoxDefaultBounds.getWidth() - marginBetweenComboBoxes) / 2;

        const String pluginTypeOptions[] =
        {
            TRANS("Processor"),
            TRANS("Record engine"),
            TRANS("Data thread"),
            TRANS("File source")
        };

        auto& pluginTypeComboBox = createPluginTypeOptionComboBox (setupComp, itemsCreated,
                                                                   StringArray (pluginTypeOptions,
                                                                                numElementsInArray (pluginTypeOptions)));
        pluginTypeComboBox.setBounds (comboBoxDefaultBounds);
        //pluginTypeComboBox.addListener (this);

        const String processorTypeOptions[] =
        {
            TRANS("Filter"),
            TRANS("Source"),
            TRANS("Sink"),
            TRANS("Splitter"),
            TRANS("Merger"),
            TRANS("Utility"),
            TRANS("Data format")
        };

        auto& processorTypeComboBox = createProcessorTypeOptionComboBox (setupComp, itemsCreated,
                                                                         StringArray (processorTypeOptions,
                                                                                      numElementsInArray (processorTypeOptions)));

        pluginTypeComboBox.setBounds (comboBoxDefaultBounds.removeFromLeft (comboBoxMinWidth));
        comboBoxDefaultBounds.removeFromLeft (marginBetweenComboBoxes - 10);
        comboBoxDefaultBounds.translate (-30, 0);
        processorTypeComboBox.setBounds (comboBoxDefaultBounds.removeFromLeft (70));
        //processorTypeComboBox.setBounds (comboBoxDefaultBounds.removeFromRight (comboBoxMinWidth));

        // Hide processor type now. Make it visible only after plugin type would be selected.
        //processorTypeComboBox.setVisible (false);
    }

    /** Gets the currently selected processor type and applies appropriate action */
    Result processResultsFromSetupItems (WizardComp& setupComp) override
    {
        // TODO <Kirill A>

        const int pluginTypeSelectedComboboxId = getComboResult (setupComp, COMBOBOX_ID_PLUGIN_TYPE) + 1;
        jassert (pluginTypeSelectedComboboxId != 0);

        m_pluginType = static_cast<PluginType> (pluginTypeSelectedComboboxId);

        // Get processor type only if was selected "Processor" type of the plugin
        if (pluginTypeSelectedComboboxId == (int)PLUGIN_TYPE_PROCESSOR)
        {
            const int processorTypeSelectedComboboxId = getComboResult (setupComp, COMBOBOX_ID_PROCESSOR_TYPE) + 1;
            jassert (processorTypeSelectedComboboxId != 0);

            m_processorType = static_cast<PluginProcessorType> (processorTypeSelectedComboboxId);
        }

        return Result::ok();
    }

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged)
    {
        return;
        const auto comboBoxId = comboBoxThatHasChanged->getComponentID();
        if (comboBoxId == COMBOBOX_ID_PLUGIN_TYPE)
        {
            const auto selectedIndex = comboBoxThatHasChanged->getSelectedItemIndex() + 1;

            ComboBox* processorTypeComboBox = dynamic_cast<ComboBox*> (ownerWizardComp->findChildWithID (COMBOBOX_ID_PROCESSOR_TYPE));
            jassert (processorTypeComboBox != nullptr);

            // If was selected "Processor" type of the plugin, then show "Processor type" combobox
            // and reduce size of both ComboBoxes to have equal size.
            if (selectedIndex == (int)PLUGIN_TYPE_PROCESSOR)
            {
                processorTypeComboBox->setVisible (true);

                // Change "Plugin type" combobox's width
                comboBoxThatHasChanged->setBounds (comboBoxThatHasChanged->getBounds()
                                                       .withWidth (processorTypeComboBox->getWidth()));
            }
            else
            {
                processorTypeComboBox->setVisible (false);
            }
        }
    }


    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        String filterClassName = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "Processor";
        filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
        String editorClassName = filterClassName + "Editor";
        String libPluginType            = getLibPluginTypeString            (m_pluginType);
        String libCreateFunctionName    = getLibPluginCreateFunctionString  (m_pluginType);
        String libPluginProcessorType   = getLibProcessorTypeString         (m_processorType);
        String processorType            = getProcessorTypeString            (m_processorType);

        File exampleFilesFolder ("/home/septen/development/open-ephys/plugin-GUI/Source/Plugins/ExampleProcessor");
        File filterCppFile = exampleFilesFolder.getChildFile ("ExampleProcessor.cpp");
        File filterHFile   = filterCppFile.withFileExtension (".h");
        File editorCppFile = exampleFilesFolder.getChildFile ("ExampleEditor.cpp");
        File editorHFile   = editorCppFile.withFileExtension (".h");
        File pluginLibFile  = exampleFilesFolder.getChildFile ("OpenEphysLib.cpp");
        File pluginMakeFile = exampleFilesFolder.getChildFile ("Makefile.example");

        project.getProjectTypeValue() = ProjectType_AudioPlugin::getTypeName();

        Project::Item sourceGroup (createSourceGroup (project));
        project.getConfigFlag ("JUCE_QUICKTIME") = Project::configFlagDisabled; // disabled because it interferes with RTAS build on PC

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), filterCppFile));

        auto sourceFolder = getSourceFilesFolder();
        auto newFilterCppFile  = sourceFolder.getChildFile (filterClassName + ".cpp");
        auto newFilterHFile    = sourceFolder.getChildFile (filterClassName + ".h");
        auto newEditorCppFile  = sourceFolder.getChildFile (editorClassName + ".cpp");
        auto newEditorHFile    = sourceFolder.getChildFile (editorClassName + ".h");
        auto newPluginLibFile  = sourceFolder.getChildFile (pluginLibFile.getFileName());
        auto newPluginMakeFile = sourceFolder.getChildFile ("Makefile");

        // TODO <Kirill A> change getting file template to more versatile method
        //String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
        String filterCpp = filterCppFile.loadFileAsString()
                            .replace ("FILTERHEADERS", CodeHelpers::createIncludeStatement (newFilterHFile, newFilterCppFile)
                                                            + newLine + CodeHelpers::createIncludeStatement (newEditorHFile, newFilterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("FILTERGUINAME", appTitle, false) // TODO <Kirill A>: set better gui name variable
                            .replace ("EDITORCLASSNAME", editorClassName, false)
                            .replace ("PROCESSORTYPE",   processorType,   false);

        //String filterH = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_h")
        String filterH   = filterHFile.loadFileAsString()
                            //.replace ("APPHEADERS", appHeaders, false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newFilterHFile), false);

        //String editorCpp = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_cpp")
        String editorCpp = editorCppFile.loadFileAsString()
                            //.replace ("EDITORCPPHEADERS", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
                            //                                   + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

        //String editorH = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_h")
        String editorH   = editorHFile.loadFileAsString()
                            //.replace ("EDITORHEADERS", appHeaders + newLine + CodeHelpers::createIncludeStatement (filterHFile, filterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false)
                            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newEditorHFile), false);

        String pluginLibCpp = pluginLibFile.loadFileAsString()
                                .replace ("FILTERCLASSNAME", filterClassName, false)
                                .replace ("FILTERLIBRARYNAME", "Temporary " + appTitle + " library", false) // TODO <Kirill A>: set library name variable
                                .replace ("FILTERLIBRARYVERSION", "1", false) // TODO <Kirill A>: set library version variable
                                .replace ("FILTERGUINAME", "Temporary " + appTitle, false) // TODO <Kirill A>: set library gui name variable
                                .replace ("LIBPLUGINTYPE", libPluginType, false)
                                .replace ("LIBPLUGINCREATEFUNCTION", libCreateFunctionName, false)
                                // TODO <Kirill A>: we should either add or remove appropriate line for plugin processor
                                // type, because it should be preset only if we generate processor plugin
                                // (Plugin::ProcessorPlugin).
                                .replace ("LIBPLUGINPROCESSORTYPE", libPluginProcessorType, false);

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newFilterCppFile, filterCpp))
            failedFiles.add (newFilterCppFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newFilterHFile, filterH))
            failedFiles.add (newFilterHFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newEditorCppFile, editorCpp))
            failedFiles.add (newEditorCppFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newEditorHFile, editorH))
            failedFiles.add (newEditorHFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newPluginLibFile, pluginLibCpp))
            failedFiles.add (newPluginLibFile.getFullPathName());

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newPluginMakeFile, pluginMakeFile.loadFileAsString()))
            failedFiles.add (newPluginMakeFile.getFullPathName());

        sourceGroup.addFileAtIndex (newFilterCppFile, -1, true);
        sourceGroup.addFileAtIndex (newFilterHFile,   -1, false);
        sourceGroup.addFileAtIndex (newEditorCppFile, -1, true);
        sourceGroup.addFileAtIndex (newEditorHFile,   -1, false);
        sourceGroup.addFileAtIndex (newPluginLibFile, -1, true);

        return true;
    }

    static constexpr const char* COMBOBOX_ID_PLUGIN_TYPE = "pluginTypeComboBox";//    { "pluginTypeComboBox" };
    static constexpr const char* COMBOBOX_ID_PROCESSOR_TYPE = "processorTypeComboBox";// { "processorTypeComboBox" };


private:
    PluginType          m_pluginType    { PLUGIN_TYPE_FILE_SOURCE } ;
    PluginProcessorType m_processorType { PROCESSOR_TYPE_FILTER };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenEphysPluginAppWizard)
};

//const String OpenEphysPluginAppWizard::COMBOBOX_ID_PLUGIN_TYPE    ("pluginTypeComboBox");
//const String OpenEphysPluginAppWizard::COMBOBOX_ID_PROCESSOR_TYPE ("processorTypeComboBox");


