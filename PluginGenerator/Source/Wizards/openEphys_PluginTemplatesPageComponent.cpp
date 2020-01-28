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


#include "openEphys_PluginTemplatesPageComponent.h"
#include "openEphys_EditorTemplatesFactory.h"
#include "openEphys_EditorTemplateComponent.h"

#include "../../../Plugins/Headers/AllLookAndFeels.h"
#include "../../../Source/UI/Utils/LinearButtonGroupManager.h"
#include "../../../Source/UI/Utils/TiledButtonGroupManager.h"


const char* PluginTemplatesPageComponent::TEMPLATES_PAGE_CREATE_PROJECT_BUTTON_ID = "templatesPageComponent_createProjectButton";

PluginTemplatesPageComponent::PluginTemplatesPageComponent()
    : m_createProjectButton                 (new TextButton ("Create project...", "Create new project"))
    , m_pluginTypeComboBox                  (new ComboBox ("Plugin type"))
    , m_processorTypeComboBox               (new ComboBox ("Processor type"))
    , m_lookAndFeelsComboBox                (new ComboBox ("LookAndFeel selector"))
    , m_pluginTypeLabel                     (new Label (String::empty, TRANS("Plugin type") + ":"))
    , m_processorTypeLabel                  (new Label (String::empty, TRANS("Processor type") + ":"))
    , m_shouldUseVisualizerEditorButton     (new ToggleButton ("Use visualizer"))
    , m_shouldUseDataThreadButton           (new ToggleButton ("Use separate thread"))
    , m_genericEditorTemplatesManager       (new TiledButtonGroupManager)
    , m_visualizerEditorTemplatesManager    (new TiledButtonGroupManager)
    , m_tabsButtonManager                   (new LinearButtonGroupManager)
{
    // Plugin type combo box
    const String pluginTypeOptions[] =
    {
        TRANS("Processor"),
        TRANS("Record engine"),
        TRANS("File source")
        //TRANS("Data thread"),
    };

    m_pluginTypeComboBox->addItemList (StringArray (pluginTypeOptions,
                                                    numElementsInArray (pluginTypeOptions)),
                                       1);
    m_pluginTypeComboBox->setSelectedId (1, dontSendNotification);
    m_pluginTypeComboBox->addListener (this);
    addAndMakeVisible (m_pluginTypeComboBox);

    m_pluginTypeLabel->setColour (Label::textColourId, Colours::white);
    m_pluginTypeLabel->attachToComponent (m_pluginTypeComboBox, true);

    // Processor type combo box
    const String processorTypeOptions[] =
    {
        TRANS("Filter"),
        TRANS("Source"),
        TRANS("Sink"),
        //TRANS("Splitter"),
        //TRANS("Merger"),
        //TRANS("Utility"),
    };

    m_processorTypeComboBox->addItemList (StringArray (processorTypeOptions,
                                                       numElementsInArray (pluginTypeOptions)),
                                          1);
    m_processorTypeComboBox->setSelectedId (1, dontSendNotification);
    m_processorTypeComboBox->addListener (this);
    addAndMakeVisible (m_processorTypeComboBox);

    m_processorTypeLabel->setColour (Label::textColourId, Colours::white);
    m_processorTypeLabel->attachToComponent (m_processorTypeComboBox, true);

    // Toggle buttons
    m_shouldUseVisualizerEditorButton->addListener (this);
    m_shouldUseVisualizerEditorButton->setColour (ToggleButton::textColourId, Colours::white);
    addAndMakeVisible (m_shouldUseVisualizerEditorButton);

    m_shouldUseDataThreadButton->addListener (this);
    m_shouldUseDataThreadButton->setColour (ToggleButton::textColourId, Colours::white);
    addChildComponent (m_shouldUseDataThreadButton);

    // Create project button
    m_createProjectButton->setComponentID (TEMPLATES_PAGE_CREATE_PROJECT_BUTTON_ID);
    addAndMakeVisible (m_createProjectButton);

    // Tab buttons
    static const Colour TAB_BUTTON_COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));
    static const Colour TAB_BUTTON_COLOUR_ACCENT  (Colour::fromRGB (3, 169, 244));

    TextButton* genericEditorTabButton = new TextButton ("Editor templates",
                                                         "Choose template for main plugin editor");
    genericEditorTabButton->setComponentID (GENERIC_EDITOR_TEMPLATES_TAB_BUTTON_ID);
    genericEditorTabButton->setClickingTogglesState (true);
    genericEditorTabButton->setToggleState (true, dontSendNotification);
    genericEditorTabButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    genericEditorTabButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    genericEditorTabButton->setColour (TextButton::textColourOffId,    TAB_BUTTON_COLOUR_PRIMARY);
    genericEditorTabButton->setColour (TextButton::textColourOnId,     TAB_BUTTON_COLOUR_ACCENT);

    TextButton* visualizerEditorTabButton = new TextButton ("Visualizer templates",
                                                            "Choose template for visualizer editor");
    visualizerEditorTabButton->setComponentID (VISUALIZER_EDITOR_TEMPLATES_TAB_BUTTON_ID);
    visualizerEditorTabButton->setClickingTogglesState (true);
    visualizerEditorTabButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    visualizerEditorTabButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    visualizerEditorTabButton->setColour (TextButton::textColourOffId,    TAB_BUTTON_COLOUR_PRIMARY);
    visualizerEditorTabButton->setColour (TextButton::textColourOnId,     TAB_BUTTON_COLOUR_ACCENT);

    m_tabsButtonManager->addButton (genericEditorTabButton);
    m_tabsButtonManager->addButton (visualizerEditorTabButton);
    m_tabsButtonManager->setRadioButtonMode (true);
    m_tabsButtonManager->setButtonListener (this);
    m_tabsButtonManager->setButtonsLookAndFeel (m_materialButtonLookAndFeel);
    m_tabsButtonManager->setColour (ButtonGroupManager::backgroundColourId,   Colours::white);
    m_tabsButtonManager->setColour (ButtonGroupManager::outlineColourId,      Colour (0x0));
    m_tabsButtonManager->setColour (LinearButtonGroupManager::accentColourId, TAB_BUTTON_COLOUR_ACCENT);
    addAndMakeVisible (m_tabsButtonManager);


    // Templates managers
    const int buttonWidth    = 200;
    const int buttonHeight   = 150;

    m_genericEditorTemplatesManager->setButtonSize (buttonWidth, buttonHeight);
    m_genericEditorTemplatesManager->setColour (TiledButtonGroupManager::outlineColourId, Colour (0x0));
    addAndMakeVisible (m_genericEditorTemplatesManager);

    m_visualizerEditorTemplatesManager->setButtonSize (buttonWidth, buttonHeight);
    m_visualizerEditorTemplatesManager->setColour (TiledButtonGroupManager::outlineColourId, Colour (0x0));
    addChildComponent (m_visualizerEditorTemplatesManager);

    // LookAndFeel comboBox
    fillLookAndFeels();
    m_lookAndFeelsComboBox->addListener (this);
    m_lookAndFeelsComboBox->setSelectedId (1, dontSendNotification);
    addAndMakeVisible (m_lookAndFeelsComboBox);

    // Fill in templates managers with templates
    fillGenericEditorTemplates();
    fillVisualizerEditorTemplates();
}


void PluginTemplatesPageComponent::paint (Graphics& g)
{
    if (isProcessorPlugin())
    {
        const auto managerBorderBounds = m_genericEditorTemplatesManager->getBounds().expanded (10, 30).translated (0, 20);
        g.setColour (Colours::orange);
        g.drawRoundedRectangle (managerBorderBounds.toFloat(), 5.f, 2.f);
    }
}


void PluginTemplatesPageComponent::resized()
{
    auto localBounds = getLocalBounds().reduced (10, 0);

    // Top panel
    // ========================================================================
    localBounds.removeFromTop (10);

    auto configComponentsBounds = localBounds.removeFromTop (22);
    configComponentsBounds.removeFromLeft (85);
    const int comboBoxWidth = localBounds.getWidth() * 0.2;
    m_pluginTypeComboBox->setBounds (configComponentsBounds.removeFromLeft (comboBoxWidth));

    const int processorComboBoxWidth = m_processorTypeComboBox->isVisible() ? comboBoxWidth : 0;
    configComponentsBounds.removeFromLeft ((int) processorComboBoxWidth * 0.8);
    m_processorTypeComboBox->setBounds (configComponentsBounds.removeFromLeft (processorComboBoxWidth));

    configComponentsBounds.removeFromLeft (20);
    if (m_shouldUseDataThreadButton->isVisible())
        m_shouldUseDataThreadButton->setBounds (configComponentsBounds.removeFromLeft (160));
    m_shouldUseVisualizerEditorButton->setBounds (configComponentsBounds);

    localBounds.removeFromTop (10);
    // ========================================================================

    m_createProjectButton->setBounds ("right - 125, bottom - 32, parent.width - 20, parent.height - 35");

    localBounds.removeFromBottom (m_createProjectButton->getHeight() + 15);

    auto tabSelectorsRow = localBounds.removeFromTop (36);
    m_tabsButtonManager->setBounds    (tabSelectorsRow.removeFromLeft (400));
    tabSelectorsRow.removeFromLeft (20);
    m_lookAndFeelsComboBox->setBounds (tabSelectorsRow.withHeight (22));

    m_genericEditorTemplatesManager->setBounds      (localBounds.reduced (10, 20));
    m_visualizerEditorTemplatesManager->setBounds   (localBounds.reduced (10, 20));
}


void PluginTemplatesPageComponent::buttonClicked (Button* buttonThatWasClicked)
{
    const auto& buttonId = buttonThatWasClicked->getComponentID();
    if (buttonId == GENERIC_EDITOR_TEMPLATES_TAB_BUTTON_ID)
    {
        showGenericEditorTemplates();
    }
    else if (buttonId == VISUALIZER_EDITOR_TEMPLATES_TAB_BUTTON_ID)
    {
        showVisualizerEditorTemplates();
    }
    else if (buttonThatWasClicked == m_shouldUseVisualizerEditorButton)
    {
        // Uncomment if we want to disable "VisualizerEditor templates" tab when no need to use visualizer editor
        //setVisualizerTemplatesAvailable (m_shouldUseVisualizerEditor->getToggleState());
    }
    else if (buttonThatWasClicked == m_shouldUseDataThreadButton)
    {
        updateComponentsVisibility();
    }
    else if (auto selectedTemplateComponent = dynamic_cast<EditorTemplateComponent*> (buttonThatWasClicked))
    {
        // The page with templates for generic editor is active
        if (m_genericEditorTemplatesManager->isVisible())
            m_selectedTemplateName = selectedTemplateComponent->getTemplateName();
        // The page with templates for visualizer editor canvas is active
        else
            m_selectedVisualizerTemplateName = selectedTemplateComponent->getTemplateName();
    }
}


void PluginTemplatesPageComponent::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == m_pluginTypeComboBox
        || comboBoxThatHasChanged == m_processorTypeComboBox)
    {
        updateComponentsVisibility();
    }
    else if (comboBoxThatHasChanged == m_lookAndFeelsComboBox)
    {
        const int selectedLookAndFeelIdx = m_lookAndFeelsComboBox->getSelectedItemIndex();
        if (selectedLookAndFeelIdx < 0)
            return;

        // "Default" lookAndFeel option was selected
        if (selectedLookAndFeelIdx == 0)
        {
            // TODO <Kirill A>: Switch back to each template's default lookAndFeel
            return;
        }

        applyLookAndFeel (m_lookAndFeelsList[selectedLookAndFeelIdx]);
    }
}


void PluginTemplatesPageComponent::updateComponentsVisibility()
{
    const int selectedPluginType = getSelectedPluginType();
    const bool isProcessorPlugin    = selectedPluginType == (int)Plugin::PLUGIN_TYPE_PROCESSOR;
    const bool isDataThreadPlugin   = selectedPluginType == (int)Plugin::PLUGIN_TYPE_DATA_THREAD;

    const int selectedProcessorType = getSelectedProcessorType();
    const bool isSourcePlugin = selectedProcessorType == (int)Plugin::PROCESSOR_TYPE_SOURCE;

    // These components should be visible only if the Processor plugin type was selected
    m_processorTypeComboBox->setVisible            (isProcessorPlugin || isDataThreadPlugin);
    m_shouldUseVisualizerEditorButton->setVisible  (isProcessorPlugin);
    m_lookAndFeelsComboBox->setVisible             (isProcessorPlugin);
    m_tabsButtonManager->setVisible                (isProcessorPlugin);

    const bool isGenericTemplatesActive = m_tabsButtonManager->getButtonAt (0)->getToggleState();
    m_genericEditorTemplatesManager->setVisible    (isProcessorPlugin && isGenericTemplatesActive);
    m_visualizerEditorTemplatesManager->setVisible (isProcessorPlugin && ! isGenericTemplatesActive);

    // These components should be visible only if the File Source plugin type was selected
    m_shouldUseDataThreadButton->setVisible (isDataThreadPlugin || isSourcePlugin);

    resized();
    repaint();
}


void PluginTemplatesPageComponent::setButtonListenerForCreateProjectButton (Button::Listener* buttonListener)
{
    m_createProjectButton->addListener (buttonListener);
}


void PluginTemplatesPageComponent::showGenericEditorTemplates()
{
    m_visualizerEditorTemplatesManager->setVisible (false);
    m_genericEditorTemplatesManager->setVisible (true);
}


void PluginTemplatesPageComponent::showVisualizerEditorTemplates()
{
    m_genericEditorTemplatesManager->setVisible (false);
    m_visualizerEditorTemplatesManager->setVisible (true);
}


void PluginTemplatesPageComponent::fillGenericEditorTemplates()
{
    m_genericEditorTemplatesManager->clear();

    ScopedPointer<Array<EditorTemplateComponent*>> templateComponents = EditorTemplatesFactory::createGenericEditorTemplates();
    for (auto templateComponent : *templateComponents)
    {
        templateComponent->setClickingTogglesState (true);
        templateComponent->addListener (this);
        templateComponent->setRadioGroupId (1, dontSendNotification);

        m_genericEditorTemplatesManager->addButton (templateComponent, false);
    }
}


void PluginTemplatesPageComponent::fillVisualizerEditorTemplates()
{
    m_visualizerEditorTemplatesManager->clear();

    ScopedPointer<Array<EditorTemplateComponent*>> templateComponents = EditorTemplatesFactory::createVisualizerEditorTemplates();
    for (auto templateComponent : *templateComponents)
    {
        templateComponent->setClickingTogglesState (true);
        templateComponent->addListener (this);
        templateComponent->setRadioGroupId (1, dontSendNotification);

        m_visualizerEditorTemplatesManager->addButton (templateComponent, false);
    }
}


void PluginTemplatesPageComponent::setVisualizerTemplatesAvailable (bool areAvailable)
{
    // Force to show generic editor templates
    if (! areAvailable)
        m_tabsButtonManager->getButtonAt (1)->setToggleState (false, sendNotification);

    m_tabsButtonManager->getButtonAt (1)->setEnabled (areAvailable);
}


bool PluginTemplatesPageComponent::isProcessorSourcePlugin() const noexcept
{
    const int selectedPluginType    = m_pluginTypeComboBox->getSelectedItemIndex() + 1;
    const int selectedProcessorType = m_processorTypeComboBox->getSelectedItemIndex() + 1;

    const bool isSourcePlugin = (selectedPluginType == (int)Plugin::PLUGIN_TYPE_PROCESSOR)
                                    && (selectedProcessorType == (int)Plugin::PROCESSOR_TYPE_SOURCE);

    return isSourcePlugin;
}


bool PluginTemplatesPageComponent::isProcessorPlugin() const noexcept
{
    return getSelectedPluginType() == (int)Plugin::PLUGIN_TYPE_PROCESSOR;
}


Plugin::PluginType PluginTemplatesPageComponent::getSelectedPluginType() const noexcept
{
    // If selected plugin type is Processor type and "Use thread type" is selected
    // Then it means, that we should create DataThread plugin.
    if (isProcessorSourcePlugin() && m_shouldUseDataThreadButton->getToggleState())
        return Plugin::PLUGIN_TYPE_DATA_THREAD;

    return Plugin::PluginType (m_pluginTypeComboBox->getSelectedItemIndex() + 1);
}


Plugin::PluginProcessorType PluginTemplatesPageComponent::getSelectedProcessorType() const noexcept
{
    if (getSelectedPluginType() != Plugin::PLUGIN_TYPE_PROCESSOR)
        return Plugin::PROCESSOR_TYPE_INVALID;

    return Plugin::PluginProcessorType (m_processorTypeComboBox->getSelectedItemIndex() + 1);
}


bool PluginTemplatesPageComponent::shouldUseVisualizerEditor() const noexcept
{
    const bool isProcessorPlugin = getSelectedPluginType() == Plugin::PLUGIN_TYPE_PROCESSOR;

    return isProcessorPlugin && m_shouldUseVisualizerEditorButton->getToggleState();
}


bool PluginTemplatesPageComponent::shouldUseDataThreadSource() const noexcept
{
    return isProcessorSourcePlugin() && m_shouldUseDataThreadButton->getToggleState();
}


String PluginTemplatesPageComponent::getSelectedTemplateName() const noexcept
{
    return m_selectedTemplateName;
}


String PluginTemplatesPageComponent::getSelectedVisualizerTemplateName() const noexcept
{
    return m_selectedVisualizerTemplateName;
}


String PluginTemplatesPageComponent::getSelectedLookAndFeelClassName() const noexcept
{
    const String defaultValue = "DEFAULT";
    const int selectedLnfIndex = m_lookAndFeelsComboBox->getSelectedItemIndex();

    return m_lookAndFeelsNames.getValue (m_lookAndFeelsComboBox->getItemText (selectedLnfIndex), defaultValue);
}


void PluginTemplatesPageComponent::addLookAndFeel (LookAndFeel* newLookAndFeel, const String& name, const String& className)
{
    m_lookAndFeelsNames.set (name, className);
    m_lookAndFeelsList.add (newLookAndFeel);

    m_lookAndFeelsComboBox->addItem (name, m_lookAndFeelsComboBox->getNumItems() + 1);
}


void PluginTemplatesPageComponent::applyLookAndFeel (LookAndFeel* lookAndFeel)
{
    const int numEditorTempaltes = m_genericEditorTemplatesManager->getNumButtons();
    for (int i = 0; i < numEditorTempaltes; ++i)
    {
        auto editorTemplateComponent = dynamic_cast<EditorTemplateComponent*> (m_genericEditorTemplatesManager->getButtonAt (i));
        editorTemplateComponent->setContentLookAndFeel (lookAndFeel);
    }
}


void PluginTemplatesPageComponent::fillLookAndFeels()
{
    addLookAndFeel (new LookAndFeel_V2(), "Default", "DEFAULT");

    addLookAndFeel (new LookAndFeel_V1(), "LookAndFeel_V1", "LookAndFeel_V1");
    addLookAndFeel (new LookAndFeel_V2(), "LookAndFeel_V2", "LookAndFeel_V2");
    addLookAndFeel (new LookAndFeel_V3(), "LookAndFeel_V3", "LookAndFeel_V3");

    addLookAndFeel (new FlatRoundLookAndFeel(), "Flat round LookAndFeel", "FlatRoundLookAndFeel");
    addLookAndFeel (new SquareLookAndFeel(),    "Square LookAndFeel", "SquareLookAndFeel");
}



