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


// =================================================================================

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

    processorTypeComboBox->setVisible (false);
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


static void updateOpenEphysWizardComboBoxBounds (const Component& parent)
{
    auto pluginTypeComboBox     = dynamic_cast<ComboBox*> (parent
                                                            .findChildWithID (OpenEphysPluginAppWizard::COMBOBOX_ID_PLUGIN_TYPE));
    auto processorTypeComboBox  = dynamic_cast<ComboBox*> (parent
                                                            .findChildWithID (OpenEphysPluginAppWizard::COMBOBOX_ID_PROCESSOR_TYPE));

    if (pluginTypeComboBox == nullptr
        || processorTypeComboBox == nullptr)
    {
        return;
    }

    const auto parentBounds = parent.getLocalBounds();
    //auto rightSideOfComponent = parent.getLocalBounds().removeFromRight (parent.getWidth() / 2).withHeight (22);
    auto comboBoxBounds = juce::Rectangle<int> (parent.getWidth() / 2 + 95,
                                                30,
                                                parent.getWidth() / 2 - 127,
                                                parent.getY() + 22);

    const int marginBetweenComboBoxes = 140;
    const int comboBoxMinWidth        = (comboBoxBounds.getWidth() - marginBetweenComboBoxes) / 2;

    if (pluginTypeComboBox->getSelectedItemIndex() + 1 == (int)PLUGIN_TYPE_PROCESSOR)
    {
        pluginTypeComboBox->setBounds (comboBoxBounds.removeFromLeft (comboBoxMinWidth));

        comboBoxBounds.removeFromLeft (marginBetweenComboBoxes - 10);

        processorTypeComboBox->setBounds (comboBoxBounds);
        processorTypeComboBox->setVisible (true);
    }
    else
    {
        pluginTypeComboBox->setBounds (comboBoxBounds);
        processorTypeComboBox->setVisible (false);
    }
}


// ============================================================================
// ============================================================================
// ============================================================================

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

        auto parentComboBoxListener = dynamic_cast<ComboBox::Listener*> (&setupComp);
        jassert (parentComboBoxListener != nullptr);
        pluginTypeComboBox.addListener (parentComboBoxListener);

        const String processorTypeOptions[] =
        {
            TRANS("Filter"),
            TRANS("Source"),
            TRANS("Sink"),
            TRANS("Splitter"),
            TRANS("Merger"),
            TRANS("Utility"),
        };

        auto& processorTypeComboBox = createProcessorTypeOptionComboBox (setupComp, itemsCreated,
                                                                         StringArray (processorTypeOptions,
                                                                                      numElementsInArray (processorTypeOptions)));

        updateOpenEphysWizardComboBoxBounds (setupComp);
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


    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        return;

        // UNUSED NOW
        // 
        //
        //
        //
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

        String pluginProcessorName  = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "Processor";
        pluginProcessorName         = pluginProcessorName.substring (0, 1).toUpperCase() + pluginProcessorName.substring (1);
        String pluginEditorName     = pluginProcessorName + "Editor";
        String processorType        = getProcessorTypeString (m_processorType);
        String pluginFriendlyName   = appTitle;

        // TODO <Kirill A> think about loading files right from the BinaryData
        File templateFilesFolder ("../../../Source/Processors/PluginManager/Templates");

        project.getProjectTypeValue() = ProjectType_OpenEphysPlugin::getTypeName();

        Project::Item sourceGroup (createSourceGroup (project));
        project.getConfigFlag ("JUCE_QUICKTIME") = Project::configFlagDisabled; // disabled because it interferes with RTAS build on PC
        project.shouldBuildVST().setValue (false);
        project.shouldBuildVST3().setValue (false);

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        //String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), filterCppFile));

        auto sourceFolder = getSourceFilesFolder();

        // TODO <Kirill A> change getting file template to more versatile method
        generatePluginMakeFile  (sourceGroup, templateFilesFolder);
        generatePluginLibFile   (sourceGroup, templateFilesFolder, pluginProcessorName, pluginFriendlyName);
        generatePluginProcessorFiles (sourceGroup, templateFilesFolder, pluginProcessorName, pluginEditorName, pluginFriendlyName);
        generatePluginEditorFiles    (sourceGroup, templateFilesFolder, pluginProcessorName, pluginEditorName, pluginFriendlyName);

        return true;
    }


    bool generatePluginLibFile (Project::Item& sourceGroup,
                                const File& templatesFolder,
                                const String& pluginProcessorName,
                                const String& pluginFriendlyName)
    {
        String libPluginType            = getLibPluginTypeString            (m_pluginType);
        String libCreateFunctionName    = getLibPluginCreateFunctionString  (m_pluginType);
        String libPluginProcessorType   = getLibProcessorTypeString         (m_processorType);

        const auto templatePluginLibFile    = templatesFolder.getChildFile ("openEphys_OpenEphysLibTemplate.cpp");
        const auto newPluginLibFile         = getSourceFilesFolder().getChildFile ("OpenEphysLib.cpp");

        String pluginLibCppFileContent = templatePluginLibFile.loadFileAsString()
            .replace ("PROCESSORCLASSNAME", pluginProcessorName, false)
            .replace ("PLUGINLIBRARYNAME", "Temporary " + pluginFriendlyName + " library", false) // TODO <Kirill A>: set library name variable
            .replace ("PLUGINLIBRARYVERSION", "1", false) // TODO <Kirill A>: set library version variable
            .replace ("PLUGINGUINAME", "Temporary " + pluginFriendlyName, false) // TODO <Kirill A>: set library gui name variable
            .replace ("LIBPLUGINTYPE", libPluginType, false)
            .replace ("LIBPLUGINCREATEFUNCTION", libCreateFunctionName, false)
            // TODO <Kirill A>: we should either add or remove appropriate line for plugin processor
            // type, because it should be preset only if we generate processor plugin
            // (Plugin::ProcessorPlugin).
            .replace ("LIBPLUGINPROCESSORTYPE", libPluginProcessorType, false);

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newPluginLibFile, pluginLibCppFileContent))
        {
            failedFiles.add (newPluginLibFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        sourceGroup.addFileAtIndex (newPluginLibFile, -1, true);

        return wasGeneratedSuccessfully;
    }


    bool generatePluginMakeFile (Project::Item& sourceGroup, const File& templatesFolder)
    {
        const auto templatePluginMakeFile = templatesFolder.getChildFile ("openEphys_PluginMakefile.example");
        const auto sourceFolder           = getSourceFilesFolder();

        auto newPluginMakeFile = sourceFolder.getChildFile ("Makefile");

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newPluginMakeFile, templatePluginMakeFile.loadFileAsString()))
        {
            failedFiles.add (newPluginMakeFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        return wasGeneratedSuccessfully;
    }


    bool generatePluginProcessorFiles (Project::Item& sourceGroup,
                                       const File& templatesFolder,
                                       const String& processorName,
                                       const String& editorName,
                                       const String& pluginFriendlyName)
    {
        const auto templateProcessorCppFile = templatesFolder.getChildFile ("openEphys_ProcessorPluginTemplate.cpp");
        const auto templateProcessorHFile   = templateProcessorCppFile.withFileExtension (".h");
        const auto sourceFolder             = getSourceFilesFolder();

        auto newProcessorCppFile  = sourceFolder.getChildFile (processorName + ".cpp");
        auto newProcessorHFile    = sourceFolder.getChildFile (processorName + ".h");
        auto newEditorHFile       = sourceFolder.getChildFile (editorName + ".h");

        String processorType = getProcessorTypeString (m_processorType);

        //String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
        String processorCppFileContent = templateProcessorCppFile.loadFileAsString()
            .replace ("PROCESSORHEADERS", CodeHelpers::createIncludeStatement (newProcessorHFile, newProcessorCppFile)
                      + newLine + CodeHelpers::createIncludeStatement (newEditorHFile, newProcessorCppFile), false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("PLUGINGUINAME",      pluginFriendlyName, false) // TODO <Kirill A>: set better gui name variable
            .replace ("EDITORCLASSNAME",    editorName, false)
            .replace ("PROCESSORTYPE",      processorType,   false);

        //String filterH = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_h")
        String processorHFileConent   = templateProcessorHFile.loadFileAsString()
            //.replace ("APPHEADERS", appHeaders, false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newProcessorHFile), false);

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newProcessorCppFile, processorCppFileContent))
        {
            failedFiles.add (newProcessorCppFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newProcessorHFile, processorHFileConent))
        {
            failedFiles.add (newProcessorHFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        sourceGroup.addFileAtIndex (newProcessorCppFile, -1, true);
        sourceGroup.addFileAtIndex (newProcessorHFile,   -1, false);

        return wasGeneratedSuccessfully;
    }


    bool generatePluginEditorFiles (Project::Item& sourceGroup,
                                    const File& templatesFolder,
                                    const String& processorName,
                                    const String& editorName,
                                    const String& pluginFriendlyName)
    {
        const auto templateEditorCppFile = templatesFolder.getChildFile ("openEphys_ProcessorEditorPluginTemplate.cpp");
        const auto templateEditorHFile   = templateEditorCppFile.withFileExtension (".h");
        const auto sourceFolder          = getSourceFilesFolder();

        auto newEditorCppFile  = sourceFolder.getChildFile (editorName + ".cpp");
        auto newEditorHFile    = sourceFolder.getChildFile (editorName + ".h");

        //String editorCpp = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_cpp")
        String editorCppFileContent = templateEditorCppFile.loadFileAsString()
            //.replace ("EDITORCPPHEADERS", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
            //                                   + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("EDITORCLASSNAME", editorName, false);

        //String editorH = project.getFileTemplate ("jucer_AudioPluginEditorTemplate_h")
        String editorHFileContent   = templateEditorHFile.loadFileAsString()
            //.replace ("EDITORHEADERS", appHeaders + newLine + CodeHelpers::createIncludeStatement (filterHFile, filterCppFile), false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("EDITORCLASSNAME", editorName, false)
            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newEditorHFile), false);

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newEditorCppFile, editorCppFileContent))
        {
            failedFiles.add (newEditorCppFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newEditorHFile, editorHFileContent))
        {
            failedFiles.add (newEditorHFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        sourceGroup.addFileAtIndex (newEditorCppFile, -1, true);
        sourceGroup.addFileAtIndex (newEditorHFile,   -1, false);

        return wasGeneratedSuccessfully;
    }

    static constexpr const char* COMBOBOX_ID_PLUGIN_TYPE    = "pluginTypeComboBox";//    { "pluginTypeComboBox" };
    static constexpr const char* COMBOBOX_ID_PROCESSOR_TYPE = "processorTypeComboBox";// { "processorTypeComboBox" };


private:
    PluginType          m_pluginType    { PLUGIN_TYPE_FILE_SOURCE } ;
    PluginProcessorType m_processorType { PROCESSOR_TYPE_INVALID };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenEphysPluginAppWizard)
};


