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

    if (getName().equalsIgnoreCase ("window"))
        setTooltip ("Open this visualizer in its own window");
    else
        setTooltip ("Open this visualizer in a tab");
}


SelectorButton::~SelectorButton()
{
}


void SelectorButton::paintButton (Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour (Colours::white);
    else
        g.setColour (Colours::darkgrey);

    if (isMouseOver)
        g.setColour (Colours::yellow);


    if (getName().equalsIgnoreCase ("window"))
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
    return getName().equalsIgnoreCase ("window");
}


bool SelectorButton::isOpenTabButton() const
{
    return ! isOpenWindowButton();
}


VisualizerEditor::VisualizerEditor (GenericProcessor* parentNode, int width, bool useDefaultParameterEditors)
    : GenericEditor (parentNode, useDefaultParameterEditors)
    , dataWindow    (nullptr)
    , canvas        (nullptr)
    , tabText       ("Tab")
    , isPlaying     (false)
    , tabIndex      (-1)
{
    desiredWidth = width;

    initializeSelectors();
}


VisualizerEditor::VisualizerEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors)
    : GenericEditor (parentNode, useDefaultParameterEditors)
    , dataWindow    (nullptr)
    , canvas        (nullptr)
    , isPlaying     (false)
    , tabIndex      (-1)
{
    desiredWidth = 180;
    initializeSelectors();
}


void VisualizerEditor::initializeSelectors()
{
    windowSelector = new SelectorButton ("window");
    windowSelector->setBounds (desiredWidth - 40, 7, 14, 10);
    windowSelector->setToggleState (false, dontSendNotification);
    windowSelector->addListener (this);
    addAndMakeVisible (windowSelector);

    tabSelector = new SelectorButton ("tab");
    tabSelector->setToggleState (false, dontSendNotification);
    tabSelector->setBounds (desiredWidth - 20, 7, 15, 10);
    tabSelector->addListener (this);
    addAndMakeVisible(tabSelector);
}


VisualizerEditor::~VisualizerEditor()
{
    if (tabIndex > -1)
    {
        AccessClass::getDataViewport()->destroyTab (tabIndex);
    }
    if (dataWindow != nullptr)
        dataWindow->removeListener (this);

    deleteAllChildren();
}


void VisualizerEditor::resized()
{
    GenericEditor::resized();

    windowSelector->setBounds   (desiredWidth - 40, 7, 14, 10);
    tabSelector->setBounds      (desiredWidth - 20, 7, 15, 10);
}


// All additional buttons inside the VisualizerEditor should use this instead of buttonClicked()
void VisualizerEditor::buttonEvent (Button* button) {}


void VisualizerEditor::enable()
{
    LOGD("   Enabling VisualizerEditor");
    if (canvas != 0)
        canvas->beginAnimation();

    isPlaying = true;
}


void VisualizerEditor::disable()
{
    if (canvas != 0)
        canvas->endAnimation();

    isPlaying = false;
}


void VisualizerEditor::updateVisualizer()
{
    if (canvas != 0)
        canvas->update();
}


void VisualizerEditor::windowClosed()
{
}


void VisualizerEditor::editorWasClicked()
{
    if (tabIndex > -1)
    {
        LOGD("Setting tab index to ", tabIndex);
        AccessClass::getDataViewport()->selectTab (tabIndex);
    }
}


// This method is used to open the visualizer in a tab or window; do not use for sub-classes of VisualizerEditor
// Use VisualizerEditor::buttonEvent instead
void VisualizerEditor::buttonClicked (Button* button)
{

    // To handle default buttons, like the Channel Selector Drawer.
    GenericEditor::buttonClicked (button);

    // Handle the buttons to open the canvas in a tab or window
    if (canvas == nullptr)
    {
        canvas = createNewCanvas();
        //TODO: Temporary hack to prevent canvas-less interface from crashing GUI on button clicks...
        if (canvas == nullptr)
        {
            return;
        }
        canvas->update();

        if (isPlaying)
            canvas->beginAnimation();
    }

    if (button == windowSelector)
    {
        if (tabSelector->getToggleState() && windowSelector->getToggleState())
        {
            tabSelector->setToggleState (false, dontSendNotification);
            // AccessClass::getDataViewport()->destroyTab(tabIndex);
            // tabIndex = -1;
            removeTab (tabIndex);
        }

        if (dataWindow == nullptr) // have we created a window already?
        {
            makeNewWindow();

            dataWindow->setContentNonOwned (canvas, false);
            dataWindow->setVisible (true);
            // enable windowClosed() callback
            dataWindow->addListener (this);
            //canvas->refreshState();
        }
        else
        {
            dataWindow->setVisible (windowSelector->getToggleState());

            if (windowSelector->getToggleState())
            {
                dataWindow->setContentNonOwned (canvas, false);
                canvas->setBounds (0, 0, canvas->getParentWidth(), canvas->getParentHeight());
                //  canvas->refreshState();
            }
            else
            {
                dataWindow->setContentNonOwned (0, false);
            }
        }
    }
    else if (button == tabSelector)
    {
        if (tabSelector->getToggleState() && tabIndex < 0)
        {
            if (windowSelector->getToggleState())
            {
                dataWindow->setContentNonOwned (0, false);
                windowSelector->setToggleState (false, dontSendNotification);
                dataWindow->setVisible (false);
            }

            // tabIndex = AccessClass::getDataViewport()->addTabToDataViewport(tabText, canvas, this);
            addTab (tabText, canvas);
        }
        else if (! tabSelector->getToggleState() && tabIndex > -1)
        {
            removeTab (tabIndex);
        }
    }

 
}


void VisualizerEditor::saveCustomParameters (XmlElement* xml)
{
    xml->setAttribute ("Type", "Visualizer");

    XmlElement* tabButtonState = xml->createNewChildElement (EDITOR_TAG_TAB);
    tabButtonState->setAttribute ("Active",tabSelector->getToggleState());

    XmlElement* windowButtonState = xml->createNewChildElement (EDITOR_TAG_WINDOW);
    windowButtonState->setAttribute ("Active",windowSelector->getToggleState());

    if (dataWindow != nullptr)
    {
        windowButtonState->setAttribute ("x",       dataWindow->getX());
        windowButtonState->setAttribute ("y",       dataWindow->getY());
        windowButtonState->setAttribute ("width",   dataWindow->getWidth());
        windowButtonState->setAttribute ("height",  dataWindow->getHeight());
    }

    if (canvas != nullptr)
    {
        canvas->saveVisualizerParameters (xml);
    }
}


void VisualizerEditor::loadCustomParameters (XmlElement* xml)
{

    bool canvasHidden = false;

    forEachXmlChildElement (*xml, xmlNode)
    {
        if (xmlNode->hasTagName (EDITOR_TAG_TAB))
        {
            bool tabState = xmlNode->getBoolAttribute ("Active");

            if (tabState)
            {
                tabSelector->setToggleState(true, sendNotification);
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

    if (canvasHidden)
    {
        //Canvas is created on button callback, so open/close tab to simulate a hidden canvas
        tabSelector->setToggleState(true, sendNotification);
        canvas->loadVisualizerParameters(xml);
        tabSelector->setToggleState(false, sendNotification);
    }
    else if (canvas != nullptr)
    {
        canvas->loadVisualizerParameters (xml);
    }

}


void VisualizerEditor::makeNewWindow()
{
    dataWindow = new DataWindow (windowSelector, tabText);
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
    AccessClass::getDataViewport()->destroyTab (tindex);
    tabIndex = -1;
}


int VisualizerEditor::addTab (String textOfTab, Visualizer* contentComponent)
{
    tabText  = textOfTab;
    tabIndex = AccessClass::getDataViewport()->addTabToDataViewport (textOfTab, contentComponent, this);

    return tabIndex;
}


void VisualizerEditor::saveVisualizerParameters (XmlElement* xml)
{
}


void VisualizerEditor::loadVisualizerParameters (XmlElement* xml)
{
}