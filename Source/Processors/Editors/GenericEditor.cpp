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

#include "GenericEditor.h"

#include "../Parameter/ParameterEditor.h"
#include "ChannelSelector.h"
#include "../ProcessorGraph/ProcessorGraph.h"
#include "../RecordNode/RecordNode.h"
#include "../../UI/ProcessorList.h"
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/GraphViewer.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265359
#endif
GenericEditor::GenericEditor(GenericProcessor* owner, bool useDefaultParameterEditors=true)
    : AudioProcessorEditor(owner),
      desiredWidth(150), isFading(false), accumulator(0.0), acquisitionIsActive(false),
      drawerButton(0), drawerWidth(170),
      drawerOpen(false), channelSelector(0), isSelected(false), isEnabled(true), isCollapsed(false), tNum(-1)
{
    constructorInitialize(owner, useDefaultParameterEditors);
}


/*GenericEditor::GenericEditor (GenericProcessor* owner)
: AudioProcessorEditor (owner), isSelected(false),
desiredWidth(150), tNum(-1), isEnabled(true),
accumulator(0.0), isFading(false), drawerButton(0),
channelSelector(0)

{
    bool useDefaultParameterEditors=true;
    constructorInitialize(owner, useDefaultParameterEditors);
}
*/
GenericEditor::~GenericEditor()
{
    deleteAllChildren();
}

void GenericEditor::constructorInitialize(GenericProcessor* owner, bool useDefaultParameterEditors)
{

    name = getAudioProcessor()->getName();
    displayName = name;

    nodeId = owner->getNodeId();

    //MemoryInputStream mis(BinaryData::silkscreenserialized, BinaryData::silkscreenserializedSize, false);
    //Typeface::Ptr typeface = new CustomTypeface(mis);
    titleFont = Font ("Default", 14, Font::bold);

    if (!owner->isMerger() && !owner->isSplitter() && !owner->isUtility())
    {
        // std::cout << "Adding drawer button." << std::endl;

        drawerButton = new DrawerButton("name");
        drawerButton->addListener(this);
        addAndMakeVisible(drawerButton);

        if (!owner->isSink())
        {
            channelSelector = new ChannelSelector (true, titleFont);
        }
        else
        {
            channelSelector = new ChannelSelector (false, titleFont);
        }

        addChildComponent(channelSelector);
        channelSelector->setVisible(false);

        isSplitOrMerge=false;
    }
    else
    {
        isSplitOrMerge=true;
    }

    backgroundGradient = ColourGradient(Colour(190, 190, 190), 0.0f, 0.0f,
                                        Colour(185, 185, 185), 0.0f, 120.0f, false);
    backgroundGradient.addColour(0.2f, Colour(155, 155, 155));

    addParameterEditors(useDefaultParameterEditors);

    backgroundColor = Colour(10,10,10);

    //fadeIn();

}

void GenericEditor::updateName()
{
    nodeId = getProcessor()->getNodeId();
    repaint();
}

void GenericEditor::setDisplayName(const String& string)
{
    displayName = string;
    AccessClass::getGraphViewer()->updateNodeLocations();
    repaint();
}

String GenericEditor::getDisplayName()
{
    return displayName;
}

int GenericEditor::getChannelDisplayNumber(int chan) const
{
	return chan;
}

void GenericEditor::addParameterEditors(bool useDefaultParameterEditors=true)
{
    if (useDefaultParameterEditors)
    {
        const int xPosInitial = 2;
        const int yPosIntiial = 23;

        int xPos = 15;
        int yPos = 30;

        // std::cout << "Adding parameter editors." << std::endl;

        for (int i = 0; i < getProcessor()->getNumParameters(); i++)
        {
            ParameterEditor* p = new ParameterEditor(getProcessor(), getProcessor()->getParameterObject (i), titleFont);
            p->setChannelSelector (channelSelector);

            if (p->hasCustomBounds())
            {
                p->setBounds (p->getDesiredBounds().translated (xPosInitial, yPosIntiial));
            }
            else
            {
                const int dWidth  = p->desiredWidth;
                const int dHeight = p->desiredHeight;

                p->setBounds (xPos, yPos, dWidth, dHeight);

                yPos += dHeight;
                yPos += 10;
            }

            addAndMakeVisible (p);
            parameterEditors.add (p);
        }
    }
}




void GenericEditor::refreshColors()
{

    //std::cout << getName() << " refreshing colors." << std::endl;

    enum
    {
        PROCESSOR_COLOR = 801,
        FILTER_COLOR = 802,
        SINK_COLOR = 803,
        SOURCE_COLOR = 804,
        UTILITY_COLOR = 805,
    };

    if (getProcessor()->isSource())
        backgroundColor = AccessClass::getProcessorList()->findColour(SOURCE_COLOR);// Colour(255, 0, 0);//Colour(int(0.9*255.0f),int(0.019*255.0f),int(0.16*255.0f));
    else if (getProcessor()->isSink())
        backgroundColor = AccessClass::getProcessorList()->findColour(SINK_COLOR);//Colour(255, 149, 0);//Colour(int(0.06*255.0f),int(0.46*255.0f),int(0.9*255.0f));
    else if (getProcessor()->isSplitter() || getProcessor()->isMerger() || getProcessor()->isUtility())
        backgroundColor = AccessClass::getProcessorList()->findColour(UTILITY_COLOR);//Colour(40, 40, 40);//Colour(int(0.7*255.0f),int(0.7*255.0f),int(0.7*255.0f));
    else
        backgroundColor = AccessClass::getProcessorList()->findColour(FILTER_COLOR);//Colour(255, 89, 0);//Colour(int(1.0*255.0f),int(0.5*255.0f),int(0.0*255.0f));

    repaint();

}


void GenericEditor::resized()
{
    if (! isCollapsed)
    {
        if (drawerButton != 0)
            drawerButton->setBounds (getWidth() - 14, 40, 10, getHeight() - 60);

        if (channelSelector != 0)
            channelSelector->setBounds (desiredWidth - drawerWidth, 30, channelSelector->getDesiredWidth(), getHeight()-45);
    }
}


bool GenericEditor::keyPressed(const KeyPress& key)
{
    return false;
}

void GenericEditor::switchSelectedState()
{
    //std::cout << "Switching selected state" << std::endl;
    isSelected = !isSelected;
    repaint();
}

void GenericEditor::select()
{
    isSelected = true;
    repaint();
    //setWantsKeyboardFocus(true);
    //grabKeyboardFocus();

    editorWasClicked();
}

void GenericEditor::highlight()
{
    isSelected = true;
    repaint();
}

void GenericEditor::makeVisible()
{
    isSelected = true;
    repaint();
    AccessClass::getEditorViewport()->makeEditorVisible(this);
}

bool GenericEditor::getSelectionState()
{
    return isSelected;
}

void GenericEditor::deselect()
{
    isSelected = false;
    repaint();
    //setWantsKeyboardFocus(false);
}

void GenericEditor::enable()
{
    isEnabled = true;
    GenericProcessor* p = (GenericProcessor*) getProcessor();
    p->setEnabledState (true);
}

void GenericEditor::disable()
{
    isEnabled = false;
    GenericProcessor* p = (GenericProcessor*) getProcessor();
    p->setEnabledState (false);
}

bool GenericEditor::getEnabledState()
{
    GenericProcessor* p = (GenericProcessor*) getProcessor();
    return p->isEnabledState();
}

void GenericEditor::setEnabledState(bool t)
{
    GenericProcessor* p = (GenericProcessor*) getProcessor();
    p->setEnabledState(t);
    isEnabled = p->isEnabledState();
}

void GenericEditor::setDesiredWidth (int width)
{
    desiredWidth = width;
    repaint();
}

void GenericEditor::startRecording()
{
//now are disabled on acquisition
  //  if (channelSelector != 0)
  //      channelSelector->inactivateRecButtons();
}

void GenericEditor::stopRecording()
{
  //  if (channelSelector != 0)
  //      channelSelector->activateRecButtons();
}

void GenericEditor::editorStartAcquisition()
{
	startAcquisition();
    //std::cout << "GenericEditor received message to start acquisition." << std::endl;

	if (channelSelector != 0)
	{
		channelSelector->startAcquisition();
		channelSelector->inactivateRecButtons();
	}

    for (int n = 0; n < parameterEditors.size(); n++)
    {

        if (parameterEditors[n]->shouldDeactivateDuringAcquisition)
            parameterEditors[n]->setEnabled(false);

    }

    acquisitionIsActive = true;

}

void GenericEditor::editorStopAcquisition()
{
	stopAcquisition();

	if (channelSelector != 0)
	{
		channelSelector->stopAcquisition();
		channelSelector->activateRecButtons();
	}

    for (int n = 0; n < parameterEditors.size(); n++)
    {

        if (parameterEditors[n]->shouldDeactivateDuringAcquisition)
            parameterEditors[n]->setEnabled(true);

    }

    acquisitionIsActive = false;

}

void GenericEditor::startAcquisition() {}
void GenericEditor::stopAcquisition() {}

void GenericEditor::fadeIn()
{
    isFading = true;
    startTimer(10);
}

void GenericEditor::paint(Graphics& g)
{
    int offset = 0;

    if (isEnabled)
        g.setColour (backgroundColor);
    else
        g.setColour (Colours::lightgrey);

    // draw colored background
    if (! isCollapsed)
    {
        g.fillRect (1, 1, getWidth() - (2 + offset), getHeight() - 2);
        // draw gray workspace
        g.setGradientFill (backgroundGradient);
        g.fillRect (1, 22, getWidth() - 2, getHeight() - 29);
    }
    else
    {
        g.fillAll();
    }

    g.setFont (titleFont);
    g.setFont (16);

    if (isEnabled)
    {
        g.setColour(Colours::white);
    }
    else
    {
        g.setColour(Colours::grey);
    }

    // draw title
    if (!isCollapsed)
    {
        // if (!getProcessor()->isMerger() && !getProcessor()->isSplitter())
        //      g.drawText(name+" ("+String(nodeId)+")", 6, 5, 500, 15, Justification::left, false);
        // else
        g.drawText (displayName.toUpperCase(), 10, 5, 500, 15, Justification::left, false);

    }
    else
    {
        g.addTransform(AffineTransform::rotation(-M_PI/2.0));
        g.drawText (displayName.toUpperCase(), - getHeight() + 6, 5, 500, 15, Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI/2.0));
    }

    if (isSelected)
    {
        //g.setColour(Colours::yellow);
        //g.drawRect(0,0,getWidth(),getHeight(),1.0);
        g.setColour(Colours::yellow.withAlpha(0.5f));

    }
    else
    {
        g.setColour(Colours::black);
    }

    // draw highlight box
    g.drawRect(0,0,getWidth(),getHeight(),2.0);

    if (isFading)
    {
        g.setColour(Colours::black.withAlpha((float)(10.0-accumulator)/10.0f));
        if (getWidth() > 0 && getHeight() > 0)
            g.fillAll();
    }

}

void GenericEditor::timerCallback()
{
    accumulator++;

    repaint();

    if (accumulator > 10.0)
    {
        stopTimer();
        isFading = false;
    }
}

void GenericEditor::buttonClicked(Button* button)
{

    // std::cout << "Button clicked." << std::endl;

    checkDrawerButton(button);

    buttonEvent(button); // needed to inform subclasses of
    // button event
}


bool GenericEditor::checkDrawerButton(Button* button)
{
    if (button == drawerButton)
    {
        if (drawerButton->getToggleState())
        {

            channelSelector->setVisible(true);

            drawerWidth = channelSelector->getDesiredWidth() + 20;

            desiredWidth += drawerWidth;
            drawerOpen = true;

        }
        else
        {

            channelSelector->setVisible(false);

            desiredWidth -= drawerWidth;
            drawerOpen = false;
        }

        AccessClass::getEditorViewport()->makeEditorVisible(this);

        deselect();

        return true;
    }
    else
    {
        return false;
    }

}

void GenericEditor::sliderValueChanged(Slider* slider)
{
    sliderEvent(slider);
}

void GenericEditor::update()
{

    //std::cout << "Editor for ";

    GenericProcessor* p = (GenericProcessor*)getProcessor();

    // std::cout << p->getName() << " updating settings." << std::endl;

    updateSettings();

    int numChannels;
    if (!p->isSink())
    {
        numChannels = p->getNumOutputs();
    }
    else
    {
        numChannels = p->getNumInputs();
    }

    if (channelSelector != 0)
    {
        channelSelector->setNumChannels(numChannels);

        for (int i = 0; i < numChannels; i++)
        {
            // std::cout << p->channels[i]->getRecordState() << std::endl;
            channelSelector->setRecordStatus(i, p->getDataChannel(i)->getRecordState());
        }
    }

    if (numChannels == 0)
    {
        if (drawerButton != 0)
            drawerButton->setVisible(false);
    }
    else
    {
        if (drawerButton != 0)
            drawerButton->setVisible(true);
    }



    updateVisualizer(); // does nothing unless this method
    // has been implemented

}

const DataChannel* GenericEditor::getChannel(int chan) const
{
    return getProcessor()->getDataChannel(chan);

}

const EventChannel* GenericEditor::getEventChannel(int chan) const
{
    return getProcessor()->getEventChannel(chan);
}

const SpikeChannel* GenericEditor::getSpikeChannel(int chan) const
{
	return getProcessor()->getSpikeChannel(chan);
}

Array<int> GenericEditor::getActiveChannels()
{
    if (!isSplitOrMerge)
    {
        Array<int> a = channelSelector->getActiveChannels();
        return a;
    }
    else
    {
        Array<int> a;
        return a;
    }
}

bool GenericEditor::getRecordStatus(int chan)
{
    if (!isSplitOrMerge)
    {
        return channelSelector->getRecordStatus(chan);
    }
    else
    {
        return false;
    }
}

Array<bool> GenericEditor::getRecordStatusArray()
{

    Array<bool> recordStatuses;
    recordStatuses.resize(getProcessor()->getNumOutputs());

    for (int i = 0; i < getProcessor()->getNumOutputs(); i++)
    {
        if (channelSelector != nullptr)
            recordStatuses.set(i,channelSelector->getRecordStatus(i));
        else
            recordStatuses.set(i,false);
    }

    return recordStatuses;

}

bool GenericEditor::getAudioStatus(int chan)
{
    if (!isSplitOrMerge)
    {
        return channelSelector->getAudioStatus(chan);
    }
    else
    {
        return false;
    }
}

void GenericEditor::getChannelSelectionState(int chan, bool* p, bool* r, bool* a)
{
    if (!isSplitOrMerge)
    {
        *p = channelSelector->getParamStatus(chan);
        *r = channelSelector->getRecordStatus(chan);
        *a = channelSelector->getAudioStatus(chan);
    }
    else
    {
        *p = false;
        *r = false;
        *a = false;
    }
}

void GenericEditor::setChannelSelectionState(int chan, bool p, bool r, bool a)
{
    if (!isSplitOrMerge)
    {
        channelSelector->setParamStatus(chan, p);
        channelSelector->setRecordStatus(chan, r);
        channelSelector->setAudioStatus(chan, a);
    }
}

bool GenericEditor::getCollapsedState()
{
    return isCollapsed;
}

void GenericEditor::switchCollapsedState()
{

    if (!getProcessor()->isMerger() && !getProcessor()->isSplitter())
    {

        if (isCollapsed)
        {
            // open it up
            desiredWidth = originalWidth;
            isCollapsed = false;

        }
        else
        {
            originalWidth = desiredWidth;
            desiredWidth = 25;
            isCollapsed = true;
        }

        for (int i = 0; i < getNumChildComponents(); i++)
        {
            Component* c = getChildComponent(i);
            c->setVisible(!isCollapsed);
        }

        if (channelSelector != nullptr)
        {
            if (!drawerOpen)
                channelSelector->setVisible(false);
        }

        collapsedStateChanged();

        AccessClass::getEditorViewport()->refreshEditors();
    }
}

void GenericEditor::saveEditorParameters(XmlElement* xml)
{

    xml->setAttribute("isCollapsed", isCollapsed);
    xml->setAttribute("displayName", displayName);

    saveCustomParameters(xml);

}

void GenericEditor::loadEditorParameters(XmlElement* xml)
{

    bool isCollapsed = xml->getBoolAttribute("isCollapsed", false);

    if (isCollapsed)
    {
        switchCollapsedState();
    }

    displayName = xml->getStringAttribute("displayName", name);

    loadCustomParameters(xml);

}

GenericEditor* GenericEditor::getSourceEditor()
{

    GenericProcessor* sourceNode = getProcessor()->getSourceNode();

    if (sourceNode != nullptr)
        return sourceNode->getEditor();
    else
        return nullptr;
}

GenericEditor* GenericEditor::getDestEditor()
{
    GenericProcessor* destNode = getProcessor()->getDestNode();

    if (destNode != nullptr)
        return destNode->getEditor();
    else
        return nullptr;
}

bool GenericEditor::isSplitter()
{
    return getProcessor()->isSplitter();
}

bool GenericEditor::isMerger()
{
    return getProcessor()->isMerger();
}

bool GenericEditor::isUtility()
{
    return getProcessor()->isUtility();
}

void GenericEditor::buttonEvent(Button* button)
{

}


/////////////////////// BUTTONS ///////////////////////////////

DrawerButton::DrawerButton(const String& name) : Button(name)
{
    setClickingTogglesState(true);
}

DrawerButton::~DrawerButton()
{

}

void DrawerButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
    if (isMouseOver)
        g.setColour(Colour(210,210,210));
    else
        g.setColour(Colour(110, 110, 110));

    g.drawVerticalLine(3, 0.0f, getHeight());
    g.drawVerticalLine(5, 0.0f, getHeight());
    g.drawVerticalLine(7, 0.0f, getHeight());

}

UtilityButton::UtilityButton(String label_, Font font_) :
    Button(label_), label(label_), font(font_)
{

    roundUL = true;
    roundUR = true;
    roundLL = true;
    roundLR = true;

    radius = 5.0f;

    font.setHeight(12.0f);

    setEnabledState(true);

}

UtilityButton::~UtilityButton()
{

}

bool UtilityButton::getEnabledState()
{
	return isEnabled;
}

void UtilityButton::setCorners(bool UL, bool UR, bool LL, bool LR)
{
    roundUL = UL;
    roundUR = UR;
    roundLL = LL;
    roundLR = LR;
}

void UtilityButton::setEnabledState(bool state)
{

    isEnabled = state;

    if (state)
    {
        selectedGrad = ColourGradient(Colour(240,179,12),0.0,0.0,
                                      Colour(207,160,33),0.0, 20.0f,
                                      false);
        selectedOverGrad = ColourGradient(Colour(209,162,33),0.0, 5.0f,
                                          Colour(190,150,25),0.0, 0.0f,
                                          false);
        neutralGrad = ColourGradient(Colour(220,220,220),0.0,0.0,
                                     Colour(170,170,170),0.0, 20.0f,
                                     false);
        neutralOverGrad = ColourGradient(Colour(180,180,180),0.0,5.0f,
                                         Colour(150,150,150),0.0, 0.0,
                                         false);
        fontColor = Colours::darkgrey;

    }
    else
    {

        selectedGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
                                      Colour(200,200,200),0.0, 20.0f,
                                      false);
        selectedOverGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
                                          Colour(200,200,200),0.0, 20.0f,
                                          false);
        neutralGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
                                     Colour(200,200,200),0.0, 20.0f,
                                     false);
        neutralOverGrad = ColourGradient(Colour(240,240,240),0.0,0.0,
                                         Colour(200,200,200),0.0, 20.0f,
                                         false);
        fontColor = Colours::white;
    }

    repaint();
}

void UtilityButton::setRadius(float r)
{
    radius = r;
}

void UtilityButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    g.setColour(Colours::grey);
    g.fillPath(outlinePath);

    if (getToggleState())
    {
        if (isMouseOver)
            g.setGradientFill(selectedOverGrad);
        else
            g.setGradientFill(selectedGrad);
    }
    else
    {
        if (isMouseOver)
            g.setGradientFill(neutralOverGrad);
        else
            g.setGradientFill(neutralGrad);
    }

    AffineTransform a = AffineTransform::scale(0.98f, 0.94f, float(getWidth())/2.0f,
                                               float(getHeight())/2.0f);
    g.fillPath(outlinePath, a);


    //int stringWidth = font.getStringWidth(getName());

    g.setFont(font);

    g.setColour(fontColor);
    g.drawText(label,0,0,getWidth(),getHeight(),Justification::centred,true);

    //g.drawSingleLineText(getName(), getWidth()/2 - stringWidth/2, 12);

    // if (getToggleState() == true)
    //       g.setColour(Colours::orange);
    //   else
    //       g.setColour(Colours::darkgrey);

    //   if (isMouseOver)
    //       g.setColour(Colours::white);

    //   g.fillRect(0,0,getWidth(),getHeight());

    //   font.setHeight(10);
    //   g.setFont(font);
    //   g.setColour(Colours::black);

    //   g.drawRect(0,0,getWidth(),getHeight(),1.0);

    //g.drawText(getName(),0,0,getWidth(),getHeight(),Justification::centred,true);
    // if (isButtonDown)
    // {
    //     g.setColour(Colours::white);
    // }

    // int thickness = 1;
    // int offset = 3;

    // g.fillRect(getWidth()/2-thickness,
    //            offset,
    //            thickness*2,
    //            getHeight()-offset*2);

    // g.fillRect(offset,
    //            getHeight()/2-thickness,
    //            getWidth()-offset*2,
    //            thickness*2);
}

void UtilityButton::resized()
{

    outlinePath.clear();

    if (roundUL)
    {
        outlinePath.startNewSubPath(radius, 0);
    }
    else
    {
        outlinePath.startNewSubPath(0, 0);
    }

    if (roundUR)
    {
        outlinePath.lineTo(getWidth()-radius, 0);
        outlinePath.addArc(getWidth()-radius*2, 0, radius*2, radius*2, 0, 0.5*double_Pi);
    }
    else
    {
        outlinePath.lineTo(getWidth(), 0);
    }

    if (roundLR)
    {
        outlinePath.lineTo(getWidth(), getHeight()-radius);
        outlinePath.addArc(getWidth()-radius*2, getHeight()-radius*2, radius*2, radius*2, 0.5*double_Pi, double_Pi);
    }
    else
    {
        outlinePath.lineTo(getWidth(), getHeight());
    }

    if (roundLL)
    {
        outlinePath.lineTo(radius, getHeight());
        outlinePath.addArc(0, getHeight()-radius*2, radius*2, radius*2, double_Pi, 1.5*double_Pi);
    }
    else
    {
        outlinePath.lineTo(0, getHeight());
    }

    if (roundUL)
    {
        outlinePath.lineTo(0, radius);
        outlinePath.addArc(0, 0, radius*2, radius*2, 1.5*double_Pi, 2.0*double_Pi);
    }

    outlinePath.closeSubPath();

}

String UtilityButton::getLabel()
{
    return label;
}

void UtilityButton::setLabel(String label_)
{
    label = label_;
    repaint();
}

TriangleButton::TriangleButton(int direction_) : Button("Arrow")
{
	direction = direction_;
}

TriangleButton::~TriangleButton()
{

}

void TriangleButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    //  g.fillAll(Colours::orange);
    // g.setColour(Colours::black);
    // g.drawRect(0,0,getWidth(),getHeight(),1.0);

    if (isMouseOver)
    {
        g.setColour(Colours::grey);
    }
    else
    {
        g.setColour(Colours::black);
    }

    if (isButtonDown)
    {
        g.setColour(Colours::white);
    }

    int inset = 1;
    int x1, y1, x2, y2, x3;

    x1 = inset;
    x2 = getWidth()/2;
    x3 = getWidth()-inset;

    if (direction == 1) // up
    {
        y1 = getHeight()-inset;
        y2 = inset;

    }
    else if (direction == 2) // down
    {
        y1 = inset;
        y2 = getHeight()-inset;
    }

    g.drawLine(x1, y1, x2, y2);
    g.drawLine(x2, y2, x3, y1);
    g.drawLine(x3, y1, x1, y1);


}

LoadButton::LoadButton() : ImageButton("Load")
{

    Image icon = ImageCache::getFromMemory(BinaryData::upload2_png,
                                           BinaryData::upload2_pngSize);

    setImages(false, // resizeButtonNowToFitThisImage
              true,  // rescaleImagesWhenButtonSizeChanges
              true,  // preserveImageProprotions
              icon,  // normalImage
              1.0,   // imageOpacityWhenNormal
              Colours::white, // overlayColourWhenNormal
              icon,  // overImage
              1.0,   // imageOpacityWhenOver
              Colours::yellow, // overlayColourWhenOver
              icon,  // downImage
              1.0,   // imageOpacityWhenDown
              Colours::yellow // overlayColourWhenDown
             );

}

LoadButton::~LoadButton()
{

}

SaveButton::SaveButton() : ImageButton("Save")
{
    Image icon = ImageCache::getFromMemory(BinaryData::floppy5_png,
                                           BinaryData::floppy5_pngSize);

    setImages(false, // resizeButtonNowToFitThisImage
              true,  // rescaleImagesWhenButtonSizeChanges
              true,  // preserveImageProprotions
              icon,  // normalImage
              1.0,   // imageOpacityWhenNormal
              Colours::white, // overlayColourWhenNormal
              icon,  // overImage
              1.0,   // imageOpacityWhenOver
              Colours::yellow, // overlayColourWhenOver
              icon,  // downImage
              1.0,   // imageOpacityWhenDown
              Colours::yellow // overlayColourWhenDown
             );
}

SaveButton::~SaveButton()
{

}


void GenericEditor::updateParameterButtons(int parameterIndex)
{
    if (parameterEditors.size() == 0)
    {
        //Checks if there is a parameter editor, and stops a bug if there isn't.
        //std::cout << "No parameterEditors" << std::endl;
    }
    else
    {
        if (parameterIndex == -1)
        {
            for (int i = 0; i < parameterEditors.size(); ++i)
            {
                parameterEditors[i]->updateChannelSelectionUI();
            }
        }
        else
        {
            parameterEditors[parameterIndex]->updateChannelSelectionUI();
        }
        //std::cout << "updateParameterButtons" << std::endl;
    }
}

String GenericEditor::getName()
{
    return name;
}

void GenericEditor::tabNumber(int t)
{
    tNum = t;
}

int GenericEditor::tabNumber()
{
    return tNum;
}

void GenericEditor::switchSource(int) { }

void GenericEditor::switchSource() { }

GenericProcessor* GenericEditor::getProcessor() const
{
    return (GenericProcessor*)getAudioProcessor();
}

void GenericEditor::switchDest() { }


void GenericEditor::switchIO(int) { }

int GenericEditor::getPathForEditor(GenericEditor* editor)
{
    return -1;
}

void GenericEditor::sliderEvent(Slider* slider) {}

void GenericEditor::editorWasClicked() {}

Colour GenericEditor::getBackgroundColor()
{
    return backgroundColor;
}

ColourGradient GenericEditor::getBackgroundGradient()
{
    return backgroundGradient;
}

void GenericEditor::updateSettings() {}

void GenericEditor::updateVisualizer() {}

void GenericEditor::channelChanged (int channel, bool newState) {}

void GenericEditor::saveCustomParameters(XmlElement* xml) { }

void GenericEditor::loadCustomParameters(XmlElement* xml) { }

void GenericEditor::collapsedStateChanged() {}

Array<GenericEditor*> GenericEditor::getConnectedEditors()
{
    Array<GenericEditor*> a;
    return a;
}


/***************************/
ColorButton::ColorButton(String label_, Font font_) :
    Button(label_), label(label_), font(font_)
{
    userDefinedData = -1;
    fontColor = juce::Colours::white;
    backgroundColor = juce::Colours::darkgrey;
    vert = false;
    setEnabledState(true);
    showEnabledStatus = false;
}

ColorButton::~ColorButton()
{

}

bool ColorButton::getEnabledState()
{
	return isEnabled;
}

void ColorButton::setShowEnabled(bool state)
{
    showEnabledStatus = state;
    repaint();
}

void ColorButton::setEnabledState(bool state)
{

    isEnabled = state;

    repaint();
}

void ColorButton::setUserDefinedData(int d)
{
    userDefinedData = d;
}
int ColorButton::getUserDefinedData()
{
    return userDefinedData;
}

void ColorButton::setVerticalOrientation(bool state)
{
    vert = state;
    repaint();
}

void ColorButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    if (isEnabled)
    {
        g.fillAll(backgroundColor);
    }
    else
    {
        int fac = 3;
        g.fillAll(Colour::fromRGB(backgroundColor.getRed() / fac, backgroundColor.getGreen() / fac, backgroundColor.getBlue() / fac));
    }


    /*
    if (getToggleState())
    {
    if (isMouseOver)
    g.setGradientFill(selectedOverGrad);
    else
    g.setGradientFill(selectedGrad);
    }
    else
    {
    if (isMouseOver)
    g.setGradientFill(neutralOverGrad);
    else
    g.setGradientFill(neutralGrad);
    }
    */

    if (isMouseOver)
    {
        g.setColour(Colours::white);
        g.drawRect(0, 0, getWidth(), getHeight());
    }

    g.setFont(font);
    g.setColour(fontColor);

    if (vert)
    {
        g.addTransform(AffineTransform::rotation(-M_PI / 2.0));
        g.drawText(label, 0, -getHeight(), getHeight(), getWidth(), Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI / 2.0));
    }
    else
    {
        if (showEnabledStatus)
        {
            if (isEnabled)
                g.drawText("[+] " + label, 0, 0, getWidth(), getHeight(), Justification::left, true);
            else
                g.drawText("[-] " + label, 0, 0, getWidth(), getHeight(), Justification::left, true);
        }
        else
            g.drawText(label, 0, 0, getWidth(), getHeight(), Justification::centred, true);

    }

}


String ColorButton::getLabel()
{
    return label;
}

void ColorButton::setColors(Colour foreground, Colour background)
{
    fontColor = foreground;
    backgroundColor = background;
}

void ColorButton::setLabel(String label_)
{
    label = label_;
    repaint();
}


ThresholdSlider::ThresholdSlider(Font f) : Slider("name"), font(f)
{

	setSliderStyle(Slider::Rotary);
	setRange(-400, 400.0f, 10.0f);
	setValue(-20.0f);
	setTextBoxStyle(Slider::NoTextBox, false, 40, 20);

}

ThresholdSlider::~ThresholdSlider()
{

}

void ThresholdSlider::paint(Graphics& g)
{

	ColourGradient grad = ColourGradient(Colour(40, 40, 40), 0.0f, 0.0f,
		Colour(80, 80, 80), 0.0, 40.0f, false);

	Path p;
	p.addPieSegment(3, 3, getWidth() - 6, getHeight() - 6, 5 * double_Pi / 4 - 0.2, 5 * double_Pi / 4 + 3 * double_Pi / 2 + 0.2, 0.5);

	g.setGradientFill(grad);
	g.fillPath(p);

	String valueString;

	if (isActive)
	{
		p = makeRotaryPath(getMinimum(), getMaximum(), getValue());
		g.setColour(Colour(240, 179, 12));
		g.fillPath(p);

		valueString = String((int)getValue());
	}
	else
	{

		valueString = "";

		for (int i = 0; i < valueArray.size(); i++)
		{
			p = makeRotaryPath(getMinimum(), getMaximum(), valueArray[i]);
			g.setColour(Colours::lightgrey.withAlpha(0.4f));
			g.fillPath(p);
			valueString = String((int)valueArray.getLast());
		}

	}

	font.setHeight(9.0);
	g.setFont(font);
	int stringWidth = font.getStringWidth(valueString);

	g.setFont(font);

	g.setColour(Colours::darkgrey);
	g.drawSingleLineText(valueString, getWidth() / 2 - stringWidth / 2, getHeight() / 2 + 3);

}

Path ThresholdSlider::makeRotaryPath(double min, double max, double val)
{

	Path p;

	double start;
	double range;
	if (val > 0)
	{
		start = 0;
		range = (val) / (1.3*max)*double_Pi;
	}
	if (val < 0) {
		start = -(val) / (1.3*min)*double_Pi;
		range = 0;
	}
	p.addPieSegment(6, 6, getWidth() - 12, getHeight() - 12, start, range, 0.65);

	return p;

}

void ThresholdSlider::setActive(bool t)
{
	isActive = t;
	repaint();
}

void ThresholdSlider::setValues(Array<double> v)
{
	valueArray = v;
}
