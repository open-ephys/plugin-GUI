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

#include "VisualizerEditor.h"
#include "../../AccessClass.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"

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
        g.drawRect(0,0,getWidth(),getHeight(),1.0);
        g.fillRect(0,0,getWidth(),3.0);
    }
    else
    {
        // tab icon
        g.drawVerticalLine(5,0,getHeight());
        g.fillRoundedRectangle(5,2,4,getHeight()-4,4.0f);
        g.fillRect(5,2,4,getHeight()-4);
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
    : GenericEditor (parentNode)
    , dataWindow    (nullptr)
    , canvas        (nullptr)
    , tabText       (tabText)
    , isPlaying     (false)
    , tabIndex      (-1)
    , dataWindowButtonListener(this)
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
    addAndMakeVisible(tabSelector.get());
}


VisualizerEditor::~VisualizerEditor()
{
    if (tabIndex > -1)
    {
        AccessClass::getDataViewport()->destroyTab (tabIndex);
    }
    
    if (dataWindow != nullptr)
        dataWindow->removeListener (this);

}


void VisualizerEditor::resized()
{
    GenericEditor::resized();

    windowSelector->setBounds   (getTotalWidth() - 40, 7, 14, 10);
    tabSelector->setBounds      (getTotalWidth() - 20, 7, 15, 10);
}


void VisualizerEditor::enable()
{

    if (canvas != nullptr)
        canvas->beginAnimation();

    isPlaying = true;
}


void VisualizerEditor::disable()
{
    if (canvas != nullptr)
        canvas->endAnimation();

    isPlaying = false;
}


void VisualizerEditor::updateVisualizer()
{
    if (canvas != nullptr)
        canvas->update();
}


void VisualizerEditor::editorWasClicked()
{
    if (tabIndex > -1)
    {
        LOGD("Setting tab index to ", tabIndex);
        AccessClass::getDataViewport()->selectTab (tabIndex);
    }

    if (dataWindow && windowSelector->getToggleState())
        dataWindow->toFront(true);
}


void VisualizerEditor::ButtonResponder::buttonClicked (Button* button)
{

    // Handle the buttons to open the canvas in a tab or window
    editor->checkForCanvas();

    if (button == editor->windowSelector.get())
    {
        if (editor->tabSelector->getToggleState() && editor->windowSelector->getToggleState())
        {
            editor->tabSelector->setToggleState (false, dontSendNotification);
            editor->removeTab (editor->tabIndex);
        }

        if (editor->dataWindow == nullptr) // have we created a window already?
        {
            editor->makeNewWindow();

            editor->dataWindow->setContentNonOwned (editor->canvas.get(), false);
            editor->dataWindow->setVisible (true);
            editor->dataWindow->addListener (editor);
        }
        else
        {
            editor->dataWindow->setVisible (editor->windowSelector->getToggleState());

            if (editor->windowSelector->getToggleState())
            {
                editor->dataWindow->setContentNonOwned (editor->canvas.get(), false);
                editor->canvas->setBounds (0, 0, editor->canvas->getParentWidth(), editor->canvas->getParentHeight());
            }
            else
            {
                editor->dataWindow->setContentNonOwned (0, false);
            }
        }
    }
    else if (button == editor->tabSelector.get())
    {
        if (editor->tabSelector->getToggleState() && editor->tabIndex < 0)
        {
            if (editor->windowSelector->getToggleState())
            {
                editor->dataWindow->setContentNonOwned (0, false);
                editor->windowSelector->setToggleState (false, dontSendNotification);
                editor->dataWindow->setVisible (false);
            }

            editor->addTab (editor->tabText, editor->canvas.get());
        }
        else if (!editor->tabSelector->getToggleState() && editor->tabIndex > -1)
        {
            editor->removeTab (editor->tabIndex);
        }
    }

}


void VisualizerEditor::checkForCanvas()
{
    if (canvas == nullptr)
    {
        canvas.reset(createNewCanvas());
        
        // Prevents canvas-less interface from crashing GUI on button clicks...
        if (canvas == nullptr)
        {
            LOGD("Unable to create ", getName()," canvas.");
            return;
        }

        canvas->update();

        if (isPlaying)
            canvas->beginAnimation();
    }
}


void VisualizerEditor::saveCustomParametersToXml (XmlElement* xml)
{
    xml->setAttribute ("Type", "Visualizer");

    XmlElement* tabButtonState = xml->createNewChildElement (EDITOR_TAG_TAB);
    tabButtonState->setAttribute ("Active", tabSelector->getToggleState());
    tabButtonState->setAttribute ("Index", tabIndex);

    XmlElement* windowButtonState = xml->createNewChildElement (EDITOR_TAG_WINDOW);
    windowButtonState->setAttribute ("Active", windowSelector->getToggleState());

    if (dataWindow != nullptr)
    {
        windowButtonState->setAttribute ("x",       dataWindow->getX());
        windowButtonState->setAttribute ("y",       dataWindow->getY());
        windowButtonState->setAttribute ("width",   dataWindow->getWidth());
        windowButtonState->setAttribute ("height",  dataWindow->getHeight());
    }

    saveVisualizerEditorParameters(xml);

    if (canvas != nullptr)
    {
        canvas->saveCustomParametersToXml(xml);
    }
    else {
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
            int newIndex = xmlNode->getIntAttribute ("Index", -1);

            if (tabState)
            {
                tabSelector->setToggleState(true, dontSendNotification);
                
                checkForCanvas();
                
                if(newIndex == -1)
                {
                    addTab(tabText, canvas.get());
                }
                else
                {
                    tabIndex = newIndex;
                    AccessClass::getDataViewport()->addTabAtIndex(tabIndex, tabText, canvas.get());
                }

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

    loadVisualizerEditorParameters(xml);

    if (canvasHidden)
    {
        //Canvas is created on button callback, so open/close tab to simulate a hidden canvas
        tabSelector->setToggleState(true, sendNotification);
        if (canvas != nullptr)
            canvas->loadCustomParametersFromXml(xml);
        tabSelector->setToggleState(false, sendNotification);
    }
    else if (canvas != nullptr)
    {
        canvas->loadCustomParametersFromXml(xml);
    }

}


void VisualizerEditor::makeNewWindow()
{
    dataWindow = std::make_unique<DataWindow> (windowSelector.get(), tabText);
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
    return AccessClass::getDataViewport()->getCurrentContentComponent();
}


void VisualizerEditor::setActiveTabId (int tindex)
{
    AccessClass::getDataViewport()->selectTab (tindex);
}


void VisualizerEditor::removeTab (int tindex)
{

    //std::cout << "Removing tab for " << nodeId << std::endl;
    AccessClass::getDataViewport()->destroyTab (tindex);
    tabIndex = -1;
}


int VisualizerEditor::addTab (String textOfTab, Visualizer* contentComponent)
{
    tabText  = textOfTab;
    tabIndex = AccessClass::getDataViewport()->addTabToDataViewport (textOfTab, contentComponent);

    //std::cout << "Adding tab for " << nodeId << " at " << tabIndex << std::endl;

    return tabIndex;
}
