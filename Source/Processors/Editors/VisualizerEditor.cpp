/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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

SelectorButton::SelectorButton(const String& name_)
    : Button(name_)
{
    setClickingTogglesState(true);

    if (getName().equalsIgnoreCase("window"))
        setTooltip("Open this visualizer in its own window");
    else
        setTooltip("Open this visualizer in a tab");

}

SelectorButton::~SelectorButton()
{
}

void SelectorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (getToggleState() == true)
        g.setColour(Colours::white);
    else
        g.setColour(Colours::darkgrey);

    if (isMouseOver)
        g.setColour(Colours::yellow);


    if (getName().equalsIgnoreCase("window"))
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


VisualizerEditor::VisualizerEditor(GenericProcessor* parentNode, int width, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors=true),
      dataWindow(0), canvas(0), tabText("Tab"), isPlaying(false), tabIndex(-1)
{

    desiredWidth = width;

    initializeSelectors();
}


VisualizerEditor::VisualizerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors),
      dataWindow(nullptr), canvas(nullptr), isPlaying(false), tabIndex(-1)
{

    desiredWidth = 180;
    initializeSelectors();

}

void VisualizerEditor::initializeSelectors()
{

    windowSelector = new SelectorButton("window");
    windowSelector->addListener(this);
    windowSelector->setBounds(desiredWidth - 40,7,14,10);

    windowSelector->setToggleState(false, dontSendNotification);
    addAndMakeVisible(windowSelector);

    tabSelector = new SelectorButton("tab");
    tabSelector->addListener(this);
    tabSelector->setBounds(desiredWidth - 20,7,15,10);

    addAndMakeVisible(tabSelector);
    tabSelector->setToggleState(false, dontSendNotification);
}

VisualizerEditor::~VisualizerEditor()
{

    if (tabIndex > -1)
    {
		AccessClass::getDataViewport()->destroyTab(tabIndex);
    }

    deleteAllChildren();

}

void VisualizerEditor::buttonCallback(Button* button) {}

void VisualizerEditor::enable()
{
    std::cout << "   Enabling VisualizerEditor" << std::endl;
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

void VisualizerEditor::editorWasClicked()
{

    if (tabIndex > -1)
    {
        std::cout << "Setting tab index to " << tabIndex << std::endl;
		AccessClass::getDataViewport()->selectTab(tabIndex);
    }

}

void VisualizerEditor::buttonEvent(Button* button)
{

    int gId = button->getRadioGroupId();

    if (gId > 0)
    {
        if (canvas != nullptr)
        {
            canvas->setParameter(gId-1, button->getName().getFloatValue());
        }

    }
    else
    {

        if (canvas == nullptr)
        {

            canvas = createNewCanvas();
            canvas->update();

            if (isPlaying)
                canvas->beginAnimation();
        }

        if (button == windowSelector)
        {

            if (tabSelector->getToggleState() && windowSelector->getToggleState())
            {
                tabSelector->setToggleState(false, dontSendNotification);
				AccessClass::getDataViewport()->destroyTab(tabIndex);
                tabIndex = -1;
            }

            if (dataWindow == nullptr) // have we created a window already?
            {

                dataWindow = new DataWindow(windowSelector, tabText);
                dataWindow->setContentNonOwned(canvas, false);
                dataWindow->setVisible(true);
                //canvas->refreshState();

            }
            else
            {

                dataWindow->setVisible(windowSelector->getToggleState());

                if (windowSelector->getToggleState())
                {
                    dataWindow->setContentNonOwned(canvas, false);
                    canvas->setBounds(0,0,canvas->getParentWidth(), canvas->getParentHeight());
                  //  canvas->refreshState();
                }
                else
                {
                    dataWindow->setContentNonOwned(0, false);
                }

            }

        }
        else if (button == tabSelector)
        {
            if (tabSelector->getToggleState() && tabIndex < 0)
            {

                if (windowSelector->getToggleState())
                {
                    dataWindow->setContentNonOwned(0, false);
                    windowSelector->setToggleState(false, dontSendNotification);
                    dataWindow->setVisible(false);
                }

				tabIndex = AccessClass::getDataViewport()->addTabToDataViewport(tabText, canvas, this);


            }
            else if (!tabSelector->getToggleState() && tabIndex > -1)
            {
				AccessClass::getDataViewport()->destroyTab(tabIndex);
                tabIndex = -1;

            }
        }

    }

    buttonCallback(button);

    if (button == drawerButton)
    {
        std::cout<<"Drawer button clicked"<<std::endl;
        windowSelector->setBounds(desiredWidth - 40,7,14,10);
        tabSelector->setBounds(desiredWidth - 20,7,15,10);

    }

}

void VisualizerEditor::saveCustomParameters(XmlElement* xml)
{

    xml->setAttribute("Type", "Visualizer");

    XmlElement* tabButtonState = xml->createNewChildElement("TAB");
    tabButtonState->setAttribute("Active",tabSelector->getToggleState());

    XmlElement* windowButtonState = xml->createNewChildElement("WINDOW");
    windowButtonState->setAttribute("Active",windowSelector->getToggleState());

    if (dataWindow != nullptr)
    {
        windowButtonState->setAttribute("x",dataWindow->getX());
        windowButtonState->setAttribute("y",dataWindow->getY());
        windowButtonState->setAttribute("width",dataWindow->getWidth());
        windowButtonState->setAttribute("height",dataWindow->getHeight());
    }

    if (canvas != nullptr)
    {
        canvas->saveVisualizerParameters(xml);
    }

}

void VisualizerEditor::loadCustomParameters(XmlElement* xml)
{

    forEachXmlChildElement(*xml, xmlNode)
    {
        if (xmlNode->hasTagName("TAB"))
        {

            bool tabState = xmlNode->getBoolAttribute("Active");

            if (tabState)
                tabSelector->setToggleState(true, sendNotification);


        }
        else if (xmlNode->hasTagName("WINDOW"))
        {

            bool windowState = xmlNode->getBoolAttribute("Active");

            if (windowState)
            {
                windowSelector->setToggleState(true, sendNotification);
                if (dataWindow != nullptr)
                {
                    dataWindow->setBounds(xmlNode->getIntAttribute("x"),
                                          xmlNode->getIntAttribute("y"),
                                          xmlNode->getIntAttribute("width"),
                                          xmlNode->getIntAttribute("height"));
                }

            }

        }
    }

    if (canvas != nullptr)
    {
        canvas->loadVisualizerParameters(xml);
    }
}


void VisualizerEditor::saveVisualizerParameters(XmlElement* xml)
{

}

void VisualizerEditor::loadVisualizerParameters(XmlElement* xml)
{

}