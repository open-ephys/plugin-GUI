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

#include "LfpDisplayEditor.h"

using namespace LfpViewer;


LayoutButton::LayoutButton(const String& buttonName)
            : Button(buttonName)
{
    setClickingTogglesState(true);
}

LayoutButton::~LayoutButton()
{
}

void LayoutButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if(getToggleState())
        g.setColour(Colours::orange);
    else
        g.setColour(Colours::grey);

    g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), 3);
    
    if(isMouseOver)
        g.setColour(Colours::white);
    else
        g.setColour(Colours::black);

    juce::Path path;

    if(getName().equalsIgnoreCase("single"))
    {
        g.drawRoundedRectangle(3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
    }
    else if(getName().equalsIgnoreCase("two-vertical"))
    {
        g.drawRoundedRectangle(3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect((float)(getWidth()/2) - 0.5f, 3.0f, 1.0f, (float)(getHeight()-6));
    }
    else if(getName().equalsIgnoreCase("three-vertical"))
    {
        g.drawRoundedRectangle(3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect((float)(getWidth()/3) + 1.0f, 3.0f, 1.0f, (float)(getHeight()-6));
        g.fillRect((float)(2*getWidth()/3) - 1.0f, 3.0f, 1.0f, (float)(getHeight()-6));
    }
    else if(getName().equalsIgnoreCase("two-horizontal"))
    {
        g.drawRoundedRectangle(3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect(3.0f, (float)(getHeight()/2) - 0.5f, (float)(getWidth()-6), 1.0f);
    }
    else
    {
        g.drawRoundedRectangle(3, 3, getWidth() - 6, getHeight() - 6, 2, 1);
        g.fillRect(3.0f, (float)(getHeight()/3) + 1.0f, (float)(getWidth()-6), 1.0f);
        g.fillRect(3.0f, (float)(2*getHeight()/3) - 1.0f, (float)(getWidth()-6), 1.0f);
    }
    

}


LfpDisplayEditor::LfpDisplayEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : VisualizerEditor(parentNode, useDefaultParameterEditors)
, hasNoInputs(true)
{
    lfpProcessor = (LfpDisplayNode*) parentNode;
    tabText = "LFP";

    desiredWidth = 195;

	defaultSubprocessor = 0;

    layoutLabel = new Label("layout", "Layout:");
    addAndMakeVisible(layoutLabel);
    
    singleDisplay = new LayoutButton("single");
    singleDisplay->setToggleState(true, dontSendNotification);
    singleDisplay->setRadioGroupId(201, dontSendNotification);
    singleDisplay->addListener(this);
    addAndMakeVisible(singleDisplay);
    selectedLayout = SplitLayouts::SINGLE;

    twoVertDisplay = new LayoutButton("two-vertical");
    twoVertDisplay->setToggleState(false, dontSendNotification);
    twoVertDisplay->setRadioGroupId(201, dontSendNotification);
    twoVertDisplay->addListener(this);
    addAndMakeVisible(twoVertDisplay);

    threeVertDisplay = new LayoutButton("three-vertical");
    threeVertDisplay->setToggleState(false, dontSendNotification);
    threeVertDisplay->setRadioGroupId(201, dontSendNotification);
    threeVertDisplay->addListener(this);
    addAndMakeVisible(threeVertDisplay);

    twoHoriDisplay = new LayoutButton("two-horizontal");
    twoHoriDisplay->setToggleState(false, dontSendNotification);
    twoHoriDisplay->setRadioGroupId(201, dontSendNotification);
    twoHoriDisplay->addListener(this);
    addAndMakeVisible(twoHoriDisplay);

    threeHoriDisplay = new LayoutButton("three-horizontal");
    threeHoriDisplay->setToggleState(false, dontSendNotification);
    threeHoriDisplay->setRadioGroupId(201, dontSendNotification);
    threeHoriDisplay->addListener(this);
    addAndMakeVisible(threeHoriDisplay);

    syncButton = new UtilityButton("SYNC DISPLAYS", Font("Default", 13.0f, Font::plain)),
    syncButton->addListener(this);
    addAndMakeVisible(syncButton);
}

LfpDisplayEditor::~LfpDisplayEditor()
{
}

void LfpDisplayEditor::startAcquisition()
{
	//enableLayoutSelection(false);
}

void LfpDisplayEditor::stopAcquisition()
{
	//enableLayoutSelection(true);
}

Visualizer* LfpDisplayEditor::createNewCanvas()
{
    canvas = new LfpDisplayCanvas(lfpProcessor, selectedLayout);
    return canvas;
}

void LfpDisplayEditor::buttonClicked(Button* button)
{
    if (button == singleDisplay)
        selectedLayout = SplitLayouts::SINGLE;
    else if (button == twoVertDisplay)
        selectedLayout = SplitLayouts::TWO_VERT;
    else if (button == threeVertDisplay)
        selectedLayout = SplitLayouts::THREE_VERT;
    else if (button == twoHoriDisplay)
        selectedLayout = SplitLayouts::TWO_HORZ;
    else if (button == threeHoriDisplay)
        selectedLayout = SplitLayouts::THREE_HORZ;
    else if (button == syncButton)
    {
        if (canvas != nullptr)
        {
            LfpDisplayCanvas* c = (LfpDisplayCanvas*) canvas.get();
            c->syncDisplays();
        }
            
    }
    else
        VisualizerEditor::buttonClicked(button);
    
    if(button->getRadioGroupId() == 201 && canvas != nullptr)
        static_cast<LfpDisplayCanvas*>(canvas.get())->setLayout(selectedLayout);

}

// not really being used (yet)...
void LfpDisplayEditor::buttonEvent(Button* button)
{


}

void LfpDisplayEditor::enableLayoutSelection(bool state)
{
    singleDisplay->setEnabled(state);
    twoVertDisplay->setEnabled(state);
    threeVertDisplay->setEnabled(state);
    twoHoriDisplay->setEnabled(state);
    threeHoriDisplay->setEnabled(state);
}

void LfpDisplayEditor::resized()
{
    VisualizerEditor::resized();

    layoutLabel->setBounds(5, 40, 50, 20);
    singleDisplay->setBounds(55, 40, 20, 20);
    twoVertDisplay->setBounds(80, 40, 20, 20);
    threeVertDisplay->setBounds(105, 40, 20, 20);
    twoHoriDisplay->setBounds(130, 40, 20, 20);
    threeHoriDisplay->setBounds(155, 40, 20, 20);

    syncButton->setBounds(40, 80, 110, 30);
}

void LfpDisplayEditor::removeBufferForDisplay(int splitID)
{
    if (canvas != nullptr)
    {
        LfpDisplayCanvas* cv = (LfpDisplayCanvas*) canvas.get();

        cv->removeBufferForDisplay(splitID);
    }
        
}


void LfpDisplayEditor::saveVisualizerParameters(XmlElement* xml)
{

	xml->setAttribute("Type", "LfpDisplayEditor");

	XmlElement* values = xml->createNewChildElement("VALUES");
	values->setAttribute("SelectedLayout", static_cast<int>(selectedLayout));
}

void LfpDisplayEditor::loadVisualizerParameters(XmlElement* xml)
{

	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("VALUES"))
		{
			std::cout << "Loading saved layout: " << xmlNode->getIntAttribute("SelectedLayout") << std::endl;
			selectedLayout = static_cast<SplitLayouts>(xmlNode->getIntAttribute("SelectedLayout"));
            static_cast<LfpDisplayCanvas*>(canvas.get())->setLayout(selectedLayout);

            if (selectedLayout == SplitLayouts::SINGLE)
                singleDisplay->setToggleState(true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::TWO_VERT)
                twoVertDisplay->setToggleState(true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::THREE_VERT)
                threeVertDisplay->setToggleState(true, dontSendNotification);
            else if (selectedLayout == SplitLayouts::TWO_HORZ)
                twoHoriDisplay->setToggleState(true, dontSendNotification);
            else
                threeHoriDisplay->setToggleState(true, dontSendNotification);
		}
	}

}