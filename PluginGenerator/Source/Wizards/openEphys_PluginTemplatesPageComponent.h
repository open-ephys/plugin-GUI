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

#ifndef __OPEN_EPHYS_PLUGIN_TEMPLATES_PAGE_COMPONENT_H__
#define __OPEN_EPHYS_PLUGIN_TEMPLATES_PAGE_COMPONENT_H__


#include <JuceHeader.h>

#include "openEphys_EditorTemplatesFactory.h"
#include "../../../Source/Processors/PluginManager/PluginIDs.h"

class MaterialButtonLookAndFeel;
class LinearButtonGroupManager;
class TiledButtonGroupManager;


class PluginTemplatesPageComponent  : public Component
                                    , public Button::Listener
                                    , public ComboBox::Listener
{
public:
    PluginTemplatesPageComponent();

    void paint (Graphics& g) override;
    void resized()           override;

    void buttonClicked (Button* buttonThatWasClicked) override;

    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    Plugin::PluginType getSelectedPluginType() const noexcept;
    Plugin::PluginProcessorType getSelectedProcessorType() const noexcept;

    bool shouldUseVisualizerEditor() const noexcept;
    bool shouldUseDataThreadSource() const noexcept;

    String getSelectedTemplateName()            const noexcept;
    String getSelectedVisualizerTemplateName()  const noexcept;
    String getSelectedLookAndFeelClassName()    const noexcept;


    /** Used to set listener for the "Create project" button.
        Usuallly we need to delegate this listener to the previous slidingComponent.
        (ProjectWizard_OpenEphysPlugin) which will call appropriate function to create project. */
    void setButtonListenerForCreateProjectButton (Button::Listener* buttonListener);

    void showGenericEditorTemplates();
    void showVisualizerEditorTemplates();

    static const char* TEMPLATES_PAGE_CREATE_PROJECT_BUTTON_ID;


private:
    bool isProcessorSourcePlugin()  const noexcept;
    bool isProcessorPlugin()        const noexcept;

    void updateComponentsVisibility();

    void fillGenericEditorTemplates();
    void fillVisualizerEditorTemplates();

    void setVisualizerTemplatesAvailable (bool areAvailable);

    void fillLookAndFeels();
    void addLookAndFeel (LookAndFeel* newLookAndFeel, const String& name, const String& className);
    void applyLookAndFeel (LookAndFeel* lookAndFeel);

    String m_selectedTemplateName               { "DEFAULT" };
    String m_selectedVisualizerTemplateName     { "DEFAULT" };

    /** Stores list of all available LookAndFeels. */
    OwnedArray<LookAndFeel> m_lookAndFeelsList;
    /** Stores LookAndFeel's name (key) and LookAndFeel's class name (value) in pairs. */
    StringPairArray m_lookAndFeelsNames;

    /** The purpose of this button is to notify previous slidingComponent (ProjectWizard_OpenEphysPlugin)
        about the event and call appropriate "createProject" function defined there.
        It looks not such aesthetic as could be but otherwise we would have needed to change
        much of code related to the current Projucer's business logic. */
    ScopedPointer<TextButton> m_createProjectButton;

    ScopedPointer<ComboBox> m_pluginTypeComboBox;
    ScopedPointer<ComboBox> m_processorTypeComboBox;
    ScopedPointer<ComboBox> m_lookAndFeelsComboBox;

    ScopedPointer<Label> m_pluginTypeLabel;
    ScopedPointer<Label> m_processorTypeLabel;

    ScopedPointer<ToggleButton> m_shouldUseVisualizerEditorButton;
    ScopedPointer<ToggleButton> m_shouldUseDataThreadButton;

    ScopedPointer<TiledButtonGroupManager> m_genericEditorTemplatesManager;
    ScopedPointer<TiledButtonGroupManager> m_visualizerEditorTemplatesManager;

    ScopedPointer<LinearButtonGroupManager> m_tabsButtonManager;

    ScopedPointer<MaterialButtonLookAndFeel> m_materialButtonLookAndFeel;

    // Some constants
    const char* GENERIC_EDITOR_TEMPLATES_TAB_BUTTON_ID     = "genericEditorPluginTemplatesTab";
    const char* VISUALIZER_EDITOR_TEMPLATES_TAB_BUTTON_ID  = "visualizerEditorPluginTemplatesTab";

    // ========================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginTemplatesPageComponent);
};


#endif // __OPEN_EPHYS_PLUGIN_TEMPLATES_PAGE_COMPONENT_H__

