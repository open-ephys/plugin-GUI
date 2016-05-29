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


struct OpenEphysPluginAppWizard   : public NewProjectWizard
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

    bool initialiseProject (Project& project) override
    {
        createSourceFolder();

        String filterClassName = CodeHelpers::makeValidIdentifier (appTitle, true, true, false) + "Processor";
        filterClassName = filterClassName.substring (0, 1).toUpperCase() + filterClassName.substring (1);
        String editorClassName = filterClassName + "Editor";

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
        auto newFilterCppFile = sourceFolder.getChildFile (filterClassName + ".cpp");
        auto newFilterHFile   = sourceFolder.getChildFile (filterClassName + ".h");
        auto newEditorCppFile = sourceFolder.getChildFile (editorClassName + ".cpp");
        auto newEditorHFile   = sourceFolder.getChildFile (editorClassName + ".h");
        auto newPluginLibFile  = sourceFolder.getChildFile (pluginLibFile.getFileName());
        auto newPluginMakeFile = sourceFolder.getChildFile ("Makefile");

        // TODO <Kirill A> change getting file template to more versatile method
        //String filterCpp = project.getFileTemplate ("jucer_AudioPluginFilterTemplate_cpp")
        String filterCpp = filterCppFile.loadFileAsString()
                            .replace ("FILTERHEADERS", CodeHelpers::createIncludeStatement (newFilterHFile, newFilterCppFile)
                                                            + newLine + CodeHelpers::createIncludeStatement (newEditorHFile, newFilterCppFile), false)
                            .replace ("FILTERCLASSNAME", filterClassName, false)
                            .replace ("EDITORCLASSNAME", editorClassName, false);

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
                            .replace ("FILTERGUINAME", appTitle, false) // TODO <Kirill A>: set better gui name variable
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
                                .replace ("FILTERGUINAME", "Temporary " + appTitle, false); // TODO <Kirill A>: set library gui name variable

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenEphysPluginAppWizard)
};
