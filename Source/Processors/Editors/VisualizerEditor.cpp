/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#include "VisualizerEditor.h"
#include "../../AccessClass.h"
#include "../../UI/DataViewport.h"
#include "../../UI/UIComponent.h"

#include "../../Utils/Utils.h"

SelectorButton::SelectorButton (const String& buttonName)
    : Button (buttonName)
{
    setClickingTogglesState (true);

    if (getName().contains ("Window"))
        setTooltip ("Open visualizer in its own window");
    else
        setTooltip ("Open visualizer in a tab");
}

void SelectorButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour (Colours::white);
    else
        g.setColour (Colours::darkgrey);

    if (isMouseOver)
        g.setColour (Colours::yellow);

    if (getName().contains ("Window"))
    {
        // window icon
        g.drawRect (0, 0, getWidth(), getHeight(), 1.0);
        g.fillRect (0, 0, getWidth(), 3.0);
    }
    else
    {
        // tab icon
        g.drawVerticalLine (5, 0, getHeight());
        g.fillRoundedRectangle (5, 2, 4, getHeight() - 4, 4.0f);
        g.fillRect (5, 2, 4, getHeight() - 4);
    }
}

bool SelectorButton::isOpenWindowButton() const
{
    return getName().contains ("Window");
}

bool SelectorButton::isOpenTabButton() const
{
    return ! isOpenWindowButton();
}

VisualizerEditor::VisualizerEditor (GenericProcessor* parentNode, String tabText, int desiredWidth_)
    : GenericEditor (parentNode), dataWindow (nullptr), canvas (nullptr), tabText (tabText), dataWindowButtonListener (this)
{
    desiredWidth = desiredWidth_;

    initializeSelectors();
}

void VisualizerEditor::initializeSelectors()
{
    windowSelector = std::make_unique<SelectorButton> (getNameAndId() + " Visualizer Window Button");
    windowSelector->setBounds (desiredWidth - 40, 7, 14, 10);
    windowSelector->setToggleState (false, dontSendNotification);
    windowSelector->addListener (&dataWindowButtonListener);
    addAndMakeVisible (windowSelector.get());

    tabSelector = std::make_unique<SelectorButton> (getNameAndId() + " Visualizer Tab Button");
    tabSelector->setToggleState (false, dontSendNotification);
    tabSelector->setBounds (desiredWidth - 20, 7, 15, 10);
    tabSelector->addListener (&dataWindowButtonListener);
    addAndMakeVisible (tabSelector.get());
}

VisualizerEditor::~VisualizerEditor()
{
    if (isOpenInTab)
    {
        AccessClass::getDataViewport()->removeTab (nodeId, false);
    }

    if (dataWindow != nullptr)
        dataWindow->removeListener (this);
}

void VisualizerEditor::resized()
{
    GenericEditor::resized();

    windowSelector->setBounds (getTotalWidth() - 40, 7, 14, 10);
    tabSelector->setBounds (getTotalWidth() - 20, 7, 15, 10);
}

void VisualizerEditor::enable()
{
    if (canvas != nullptr)
        canvas->beginAnimation();
}

void VisualizerEditor::disable()
{
    if (canvas != nullptr)
        canvas->endAnimation();
}

void VisualizerEditor::updateVisualizer()
{
    if (canvas != nullptr)
        canvas->update();
}

void VisualizerEditor::editorWasClicked()
{
    if (isOpenInTab)
    {
        LOGD ("Setting tab index to ", nodeId);
        AccessClass::getDataViewport()->selectTab (nodeId);
    }

    if (dataWindow && windowSelector->getToggleState())
        dataWindow->toFront (true);
}

void VisualizerEditor::ButtonResponder::buttonClicked (Button* button)
{
    // Handle the buttons to open the canvas in a tab or window
    editor->checkForCanvas();

    if (button == editor->windowSelector.get())
    {
        if (editor->tabSelector->getToggleState())
        {
            editor->removeTab();
        }

        if (editor->dataWindow == nullptr) // have we created a window already?
        {
            editor->makeNewWindow();

            editor->dataWindow->setContentNonOwned (editor->canvas.get(), true);
            editor->dataWindow->setVisible (true);
            editor->dataWindow->addListener (editor);

            // Set the rendering engine for the window
            auto editorPeer = editor->getPeer();
            if (auto peer = editor->canvas->getPeer())
            {
                auto editorPeer = editor->getPeer();
                if (editorPeer)
                    peer->setCurrentRenderingEngine (editorPeer->getCurrentRenderingEngine());
            }
        }
        else
        {
            if (editor->windowSelector->getToggleState())
            {
                editor->dataWindow->setContentNonOwned (editor->canvas.get(), true);
                // editor->canvas->setBounds (0, 0, editor->canvas->getParentWidth(), editor->canvas->getParentHeight());
            }
            else
            {
                editor->dataWindow->setContentNonOwned (0, false);
            }

            editor->dataWindow->setVisible (editor->windowSelector->getToggleState());
        }
    }
    else if (button == editor->tabSelector.get())
    {
        LOGD ("TAB BUTTON CLICKED");

        if (! editor->isOpenInTab)
        {
            if (editor->windowSelector->getToggleState())
            {
                LOGD ("CLOSING WINDOW");
                editor->dataWindow->setContentNonOwned (0, false);
                editor->windowSelector->setToggleState (false, dontSendNotification);
                editor->dataWindow->setVisible (false);
            }

            LOGD ("ADDING TAB");
            editor->addTab();
        }
        else
        {
            editor->removeTab();
        }
    }
}

void VisualizerEditor::checkForCanvas()
{
    if (canvas == nullptr)
    {
        canvas.reset (createNewCanvas());

        // Prevents canvas-less interface from crashing GUI on button clicks...
        if (canvas == nullptr)
        {
            LOGD ("Unable to create ", getName(), " canvas.");
            return;
        }

        canvas->update();

        if (acquisitionIsActive)
            canvas->beginAnimation();
    }
}

void VisualizerEditor::saveCustomParametersToXml (XmlElement* xml)
{
    xml->setAttribute ("Type", "Visualizer");

    XmlElement* tabButtonState = xml->createNewChildElement (EDITOR_TAG_TAB);
    tabButtonState->setAttribute ("Active", tabSelector->getToggleState());

    XmlElement* windowButtonState = xml->createNewChildElement (EDITOR_TAG_WINDOW);
    windowButtonState->setAttribute ("Active", windowSelector->getToggleState());

    if (dataWindow != nullptr)
    {
        windowButtonState->setAttribute ("x", dataWindow->getX());
        windowButtonState->setAttribute ("y", dataWindow->getY());
        windowButtonState->setAttribute ("width", dataWindow->getWidth());
        windowButtonState->setAttribute ("height", dataWindow->getHeight());
    }

    saveVisualizerEditorParameters (xml);

    if (canvas != nullptr)
    {
        canvas->saveToXml (xml);
    }
    else
    {
        // if canvas was never created, we don't need to save custom parameters
    }
}

void VisualizerEditor::loadCustomParametersFromXml (XmlElement* xml)
{
    bool canvasHidden = false;

    for (auto* xmlNode : xml->getChildIterator())
    {
        if (xmlNode->hasTagName (EDITOR_TAG_TAB))
        {
            bool tabState = xmlNode->getBoolAttribute ("Active");

            if (tabState)
            {
                /* NB: DataViewport::loadStateFromXml() will call addTab() for us 
                   to maintain tab configuration
                */
                checkForCanvas();

                break;
            }
        }
        else if (xmlNode->hasTagName (EDITOR_TAG_WINDOW))
        {
            bool windowState = xmlNode->getBoolAttribute ("Active");

            if (windowState)
            {
                windowSelector->setToggleState (true, sendNotification);
                if (dataWindow != nullptr)
                {
                    dataWindow->setBounds (xmlNode->getIntAttribute ("x"),
                                           xmlNode->getIntAttribute ("y"),
                                           xmlNode->getIntAttribute ("width"),
                                           xmlNode->getIntAttribute ("height"));
                }
                break;
            }
        }
        else
        {
            canvasHidden = true;
        }
    }

    loadVisualizerEditorParameters (xml);

    if (canvasHidden)
    {
        //Canvas is created on button callback, so open/close tab to simulate a hidden canvas
        tabSelector->setToggleState (true, sendNotification);
        if (canvas != nullptr)
            canvas->loadFromXml (xml);
        tabSelector->setToggleState (false, sendNotification);
    }
    else if (canvas != nullptr)
    {
        canvas->loadFromXml (xml);
    }
}

void VisualizerEditor::makeNewWindow()
{
    dataWindow = std::make_unique<DataWindow> (windowSelector.get(), tabText);
    dataWindow->setLookAndFeel (&getLookAndFeel());
    dataWindow->setBackgroundColour (findColour (ThemeColours::windowBackground));
}

/* static method */
void VisualizerEditor::addWindowListener (DataWindow* dataWindowToUse, DataWindow::Listener* newListener)
{
    if (dataWindowToUse != nullptr && newListener != nullptr)
        dataWindowToUse->addListener (newListener);
}

/* static method */
void VisualizerEditor::removeWindowListener (DataWindow* dataWindowToUse, DataWindow::Listener* oldListener)
{
    if (dataWindowToUse != nullptr && oldListener != nullptr)
        dataWindowToUse->removeListener (oldListener);
}

Component* VisualizerEditor::getActiveTabContentComponent() const
{
    return AccessClass::getDataViewport()->getActiveTabContentComponent();
}

void VisualizerEditor::removeTab()
{
    //std::cout << "Removing tab for " << nodeId << std::endl;
    AccessClass::getDataViewport()->removeTab (nodeId);
}

void VisualizerEditor::tabWasClosed()
{
    tabSelector->setToggleState (false, dontSendNotification);

    isOpenInTab = false;
}

void VisualizerEditor::addTab()
{
    if (isOpenInTab)
        return;

    LOGD ("CREATING CANVAS");
    checkForCanvas();

    LOGD ("ADDING TAB");
    AccessClass::getDataViewport()->addTab (tabText, canvas.get(), nodeId);

    if (! tabSelector->getToggleState())
        tabSelector->setToggleState (true, dontSendNotification);

    isOpenInTab = true;
}
