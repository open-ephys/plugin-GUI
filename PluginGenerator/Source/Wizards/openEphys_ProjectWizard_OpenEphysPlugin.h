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


// ============================================================================
// ============================================================================
// ============================================================================

struct OpenEphysPluginAppWizard   : public NewProjectWizard
{
    OpenEphysPluginAppWizard()
    {
        modulesFolder = File::getCurrentWorkingDirectory()
                                    .getParentDirectory().getParentDirectory().getParentDirectory()
                                    .getChildFile ("JuceLibraryCode").getChildFile ("modules");
    }

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


    Result getResultsFromConfigPage (const PluginTemplatesPageComponent* configPage)
    {
        m_pluginType    = configPage->getSelectedPluginType();
        m_processorType = configPage->getSelectedProcessorType();

        m_shouldUseVisualizerEditor = configPage->shouldUseVisualizerEditor();

        m_guiTemplateName           = configPage->getSelectedTemplateName();
        m_guiVisualizerTemplateName = configPage->getSelectedVisualizerTemplateName();

        DBG (String ("GUI Template name: ") + m_guiTemplateName);

        if (m_shouldUseVisualizerEditor)
            DBG (String ("GUI Visualizer template name: ") + m_guiVisualizerTemplateName);

        m_contentLookAndFeelClassName = configPage->getSelectedLookAndFeelClassName();
        m_shouldChangeContentLookAndFeel = (m_contentLookAndFeelClassName != "DEFAULT");

        if (m_shouldChangeContentLookAndFeel)
            DBG (String ("Content LookAndFeel name: ") + m_contentLookAndFeelClassName);

        return Result::ok();
    }


    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        if (auto templatesPage = dynamic_cast<PluginTemplatesPageComponent*>
                (ownerWizardComp->findParentComponentOfClass<StartPageComponent>()
                    ->getPluginTemplatesPage()))
        {
            getResultsFromConfigPage (templatesPage);
        }

        String pluginProcessorName  = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "Processor";
        pluginProcessorName         = pluginProcessorName.substring (0, 1).toUpperCase() + pluginProcessorName.substring (1);

        const String pluginEditorName     = pluginProcessorName + "Editor";
        const String processorType        = getProcessorTypeString (m_processorType);
        const String pluginFriendlyName   = appTitle;

        DBG (String ("Processor type: ") + processorType);

        const String pluginContentComponentName = pluginProcessorName + "ContentComponent";

        project.getProjectTypeValue() = ProjectType_OpenEphysPlugin::getTypeName();

        project.setPluginType (m_pluginType);
        project.setPluginProcessorType (m_processorType);

        Project::Item sourceGroup (createSourceGroup (project));
        project.getConfigFlag ("JUCE_QUICKTIME") = Project::configFlagDisabled; // disabled because it interferes with RTAS build on PC
        project.shouldBuildVST().setValue (false);
        project.shouldBuildVST3().setValue (false);
		project.shouldBuildOpenEphysPlugin().setValue (true);

        setExecutableNameForAllTargets (project, File::createLegalFileName (appTitle));

        //String appHeaders (CodeHelpers::createIncludeStatement (project.getAppIncludeFile(), filterCppFile));

        generatePluginMakeFile  (project, sourceGroup);
        generatePluginLibFile   (project, sourceGroup, pluginProcessorName, pluginFriendlyName);
        generatePluginProcessorFiles (project, sourceGroup, pluginProcessorName, pluginEditorName, pluginFriendlyName);

        // No need to generate content component files for any plugins except Processors.
        if (m_pluginType != Plugin::PLUGIN_TYPE_PROCESSOR)
            return true;

        generatePluginEditorFiles    (project, sourceGroup, pluginProcessorName, pluginEditorName, pluginFriendlyName, pluginContentComponentName);
        generatePluginEditorContentComponentFiles (project, sourceGroup, pluginProcessorName, pluginEditorName, pluginContentComponentName);

        if (m_shouldUseVisualizerEditor)
        {
            const String visualizerCanvasName                 = pluginProcessorName + "Visualizer";
            const String visualizerCanvasContentComponentName = visualizerCanvasName + "ContentComponent";
            generatePluginVisualizerEditorCanvasFiles (project, sourceGroup,
                                                       pluginProcessorName, visualizerCanvasName,
                                                       visualizerCanvasName, visualizerCanvasContentComponentName);
            generatePluginEditorContentComponentFiles (project, sourceGroup,
                                                       pluginProcessorName, visualizerCanvasName,
                                                       visualizerCanvasContentComponentName,
                                                       true);
        }

        return true;
    }


    bool generatePluginLibFile (const Project& project,
                                Project::Item& sourceGroup,
                                const String& pluginProcessorName,
                                const String& pluginFriendlyName)
    {
        String libPluginType            = getLibPluginTypeString            (m_pluginType);
        String libCreateFunctionName    = getLibPluginCreateFunctionString  (m_pluginType);
        String libPluginInfoType        = getLibPluginInfoType              (m_pluginType);

        const auto newPluginLibFile = getSourceFilesFolder().getChildFile ("OpenEphysLib.cpp");
        String libPluginProcessorType = m_pluginType == PLUGIN_TYPE_PROCESSOR
                                        ? ("info->processor.type = " + getLibProcessorTypeString (m_processorType) + ";")
                                        : "";

        String pluginLibCppFileContent  = project.getFileTemplate ("openEphys_OpenEphysLibTemplate_cpp")
            .replace ("PROCESSORCLASSNAME", pluginProcessorName, false)
            .replace ("PLUGINLIBRARYNAME", pluginFriendlyName + " plugin library", false) // TODO <Kirill A>: set library name variable
            .replace ("PLUGINLIBRARYVERSION", "1", false) // TODO <Kirill A>: set library version variable
            .replace ("PLUGINGUINAME", pluginFriendlyName, false) // TODO <Kirill A>: set library gui name variable
            .replace ("LIBPLUGINTYPE", libPluginType, false)
            .replace ("LIBPLUGININFOTYPE", libPluginInfoType, false)
            .replace ("LIBPLUGINCREATEFUNCTION", libCreateFunctionName, false)
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


    bool generatePluginMakeFile (const Project& project, Project::Item& sourceGroup)
    {
        String templatePluginMakeFileContent = project.getFileTemplate ("openEphys_PluginMakefile_example");
        const auto sourceFolder = getSourceFilesFolder();

        auto newPluginMakeFile = sourceFolder.getChildFile ("Makefile");

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newPluginMakeFile, templatePluginMakeFileContent))
        {
            failedFiles.add (newPluginMakeFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        return wasGeneratedSuccessfully;
    }


    bool generatePluginProcessorFiles (const Project& project,
                                       Project::Item& sourceGroup,
                                       const String& processorName,
                                       const String& editorName,
                                       const String& pluginFriendlyName)
    {
        const auto sourceFolder = getSourceFilesFolder();

        auto newProcessorCppFile  = sourceFolder.getChildFile (processorName + ".cpp");
        auto newProcessorHFile    = sourceFolder.getChildFile (processorName + ".h");
        auto newEditorHFile       = sourceFolder.getChildFile (editorName + ".h");

        String processorFileTemplateName = getTemplateProcessorFileName (m_pluginType);
        String processorType             = getProcessorTypeString (m_processorType);

        String processorHeaders = CodeHelpers::createIncludeStatement (newProcessorHFile, newProcessorCppFile) + newLine;

        if (m_pluginType == PLUGIN_TYPE_PROCESSOR)
            processorHeaders += CodeHelpers::createIncludeStatement (newEditorHFile, newProcessorCppFile);

        String processorCppFileContent = project.getFileTemplate (processorFileTemplateName + "_cpp")
            .replace ("PROCESSORHEADERS",   processorHeaders, false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("PLUGINGUINAME",      pluginFriendlyName, false)
            .replace ("EDITORCLASSNAME",    editorName, false)
            .replace ("PROCESSORTYPE",      processorType,   false);

        String processorHFileConent   = project.getFileTemplate (processorFileTemplateName + "_h")
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


    bool generatePluginEditorFiles (const Project& project,
                                    Project::Item& sourceGroup,
                                    const String& processorName,
                                    const String& editorName,
                                    const String& pluginFriendlyName,
                                    const String& contentComponentName)
    {
        const auto sourceFolder = getSourceFilesFolder();

        auto newEditorCppFile  = sourceFolder.getChildFile (editorName + ".cpp");
        auto newEditorHFile    = sourceFolder.getChildFile (editorName + ".h");

        String templateFileNameWithoutExtension = m_shouldUseVisualizerEditor
                                                    ? "openEphys_ProcessorVisualizerEditorPluginTemplate"
                                                    : "openEphys_ProcessorEditorPluginTemplate";

        String editorCppFileContent = project.getFileTemplate (templateFileNameWithoutExtension + "_cpp")
            //.replace ("EDITORCPPHEADERS", CodeHelpers::createIncludeStatement (filterHFile, filterCppFile)
            //                                   + newLine + CodeHelpers::createIncludeStatement (editorHFile, filterCppFile), false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("EDITORCLASSNAME", editorName, false)
            .replace ("PLUGINGUINAME", pluginFriendlyName, false);

        String editorHFileContent   = project.getFileTemplate (templateFileNameWithoutExtension + "_h")
            //.replace ("EDITORHEADERS", appHeaders + newLine + CodeHelpers::createIncludeStatement (filterHFile, filterCppFile), false)
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("EDITORCLASSNAME", editorName, false)
            .replace ("CONTENTCOMPONENTCLASSNAME", contentComponentName, false)
            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newEditorHFile), false);

        if (m_shouldUseVisualizerEditor)
        {
            const String visualizerCanvasClassName = processorName + "Visualizer";
            editorCppFileContent = editorCppFileContent
                                    .replace ("EDITORCANVASCLASSNAME", visualizerCanvasClassName, false)
                                    .replace ("GenericEditor", "VisualizerEditor", false);
        }

        if (m_shouldChangeContentLookAndFeel)
        {
            const auto lookAndFeelDeclarationCode = "ScopedPointer<LookAndFeel> m_contentLookAndFeel;";
            const auto lookAndFeelSetterCode      = "content.setLookAndFeel (m_contentLookAndFeel);";
            const auto lookAndFeelCreationCode    = "m_contentLookAndFeel = new LOOKANDFEELCLASSNAME();";

            editorCppFileContent = editorCppFileContent
                                    // Uncomment some pieces of code
                                    .replace (String ("//") + lookAndFeelCreationCode, lookAndFeelCreationCode, false)
                                    .replace (String ("//") + lookAndFeelSetterCode, lookAndFeelSetterCode, false)
                                    // Change LookAndFeel class name
                                    .replace ("LOOKANDFEELCLASSNAME", m_contentLookAndFeelClassName, false);

            editorHFileContent = editorHFileContent
                                    // Uncomment some pieces of code
                                    .replace (String ("//") + lookAndFeelDeclarationCode, lookAndFeelDeclarationCode, false);
        }

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


    bool generatePluginEditorContentComponentFiles (const Project& project,
                                                    Project::Item& sourceGroup,
                                                    const String& processorName,
                                                    const String& editorName,
                                                    const String& contentComponentName,
                                                    bool useVisualizerEditorTemplates = false)
    {
        const auto sourceFolder = getSourceFilesFolder();

        auto newContentComponentCppFile = sourceFolder.getChildFile (contentComponentName + ".cpp");
        auto newContentComponentHFile   = sourceFolder.getChildFile (contentComponentName + ".h");

        const String guiTemplateName = useVisualizerEditorTemplates
                                            ? m_guiVisualizerTemplateName
                                            : m_guiTemplateName;
        String contentComponentCppFileContent;
        String contentComponentHFileContent;

        if (guiTemplateName == "DEFAULT" || (! isExistsGuiTemplate (guiTemplateName)))
        {
            contentComponentCppFileContent = project.getFileTemplate ("openEphys_ProcessorContentComponentTemplate_cpp")
                .replace ("CONTENTCOMPONENTCLASSNAME", contentComponentName, false);

            contentComponentHFileContent = project.getFileTemplate ("openEphys_ProcessorContentComponentTemplate_h")
                .replace ("CONTENTCOMPONENTCLASSNAME", contentComponentName, false)
                .replace ("EDITORCLASSNAME", editorName, false)
                .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newContentComponentHFile), false);
        }
        else
        {
            const auto guiTemplateClassName = getGUITemplateClassName (guiTemplateName);

            contentComponentCppFileContent = getGUITemplate (guiTemplateName, false)
                .replace (guiTemplateClassName, contentComponentName);

            const auto userVariablesMacro = String ("//[UserVariables]   -- You can add your own custom variables in this section.") + newLine;
            const auto friendClassDefinition = CodeHelpers::indent (String ("friend class ") + editorName, 4, true) + ";" + newLine;
            contentComponentHFileContent = getGUITemplate (guiTemplateName, true)
                .replace (guiTemplateClassName, contentComponentName)
                .replace (userVariablesMacro, userVariablesMacro + friendClassDefinition , false);
        }

        bool wasGeneratedSuccessfully = true;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newContentComponentCppFile, contentComponentCppFileContent))
        {
            failedFiles.add (newContentComponentCppFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newContentComponentHFile, contentComponentHFileContent))
        {
            failedFiles.add (newContentComponentHFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        sourceGroup.addFileAtIndex (newContentComponentCppFile, -1, true);
        sourceGroup.addFileAtIndex (newContentComponentHFile,   -1, false);

        return wasGeneratedSuccessfully;
    }


    bool generatePluginVisualizerEditorCanvasFiles (const Project& project,
                                                    Project::Item& sourceGroup,
                                                    const String& processorName,
                                                    const String& editorName,
                                                    const String& canvasName,
                                                    const String& contentComponentName)
    {
        const auto sourceFolder = getSourceFilesFolder();

        auto newCanvasComponentCppFile = sourceFolder.getChildFile (canvasName + ".cpp");
        auto newCanvasComponentHFile   = sourceFolder.getChildFile (canvasName + ".h");

        String canvasComponentCppFileContent;
        String canvasComponentHFileContent;

        canvasComponentCppFileContent = project.getFileTemplate ("openEphys_ProcessorVisualizerCanvasTemplate_cpp")
            .replace ("PROCESSORCLASSNAME", processorName, false)
            .replace ("EDITORCANVASCLASSNAME", editorName, false);

        canvasComponentHFileContent = project.getFileTemplate ("openEphys_ProcessorVisualizerCanvasTemplate_h")
            .replace ("PROCESSORCLASSNAME", processorName,false)
            .replace ("EDITORCANVASCLASSNAME", editorName, false)
            .replace ("CONTENTCOMPONENTCLASSNAME", contentComponentName, false)
            .replace ("HEADERGUARD", CodeHelpers::makeHeaderGuardName (newCanvasComponentHFile), false);

        bool wasGeneratedSuccessfully = true;

        if (m_shouldChangeContentLookAndFeel)
        {
            const auto lookAndFeelDeclarationCode = "ScopedPointer<LookAndFeel> m_contentLookAndFeel;";
            const auto lookAndFeelSetterCode      = "content.setLookAndFeel (m_contentLookAndFeel);";
            const auto lookAndFeelCreationCode    = "m_contentLookAndFeel = new LOOKANDFEELCLASSNAME();";

            canvasComponentCppFileContent = canvasComponentCppFileContent 
                                    // Uncomment some pieces of code
                                    .replace (String ("//") + lookAndFeelCreationCode, lookAndFeelCreationCode, false)
                                    .replace (String ("//") + lookAndFeelSetterCode, lookAndFeelSetterCode, false)
                                    // Change LookAndFeel class name
                                    .replace ("LOOKANDFEELCLASSNAME", m_contentLookAndFeelClassName, false);

            canvasComponentHFileContent = canvasComponentHFileContent
                                    // Uncomment some pieces of code
                                    .replace (String ("//") + lookAndFeelDeclarationCode, lookAndFeelDeclarationCode, false);
        }

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newCanvasComponentCppFile, canvasComponentCppFileContent))
        {
            failedFiles.add (newCanvasComponentCppFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (newCanvasComponentHFile, canvasComponentHFileContent))
        {
            failedFiles.add (newCanvasComponentHFile.getFullPathName());

            wasGeneratedSuccessfully = false;
        }

        sourceGroup.addFileAtIndex (newCanvasComponentCppFile, -1, true);
        sourceGroup.addFileAtIndex (newCanvasComponentHFile,   -1, false);

        return wasGeneratedSuccessfully;
    }

    const char* COMBOBOX_ID_PLUGIN_TYPE    = "pluginTypeComboBox";//    { "pluginTypeComboBox" };
    const char* COMBOBOX_ID_PROCESSOR_TYPE = "processorTypeComboBox";// { "processorTypeComboBox" };


private:
    PluginType          m_pluginType    { NOT_A_PLUGIN_TYPE } ;
    PluginProcessorType m_processorType { PROCESSOR_TYPE_INVALID };

    /** The name of the GUI template which is used to create content component for plugin. */
    String m_guiTemplateName;

    /** The name of the GUI template for VisualizerEditor's Canvas (if used) to create content component for plugin. */
    String m_guiVisualizerTemplateName;

    /** The class name of the LookAndFeel for contents. If it's equal to "DEFAULT" - this means that user didn't
        selected the new lookAndFeel for component and we have no need to change it..*/
    String m_contentLookAndFeelClassName;

    bool m_shouldUseVisualizerEditor;
    bool m_shouldChangeContentLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenEphysPluginAppWizard)
};


