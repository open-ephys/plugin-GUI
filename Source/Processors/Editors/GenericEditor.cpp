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

#include "../../CoreServices.h"
#include "../GenericProcessor/GenericProcessor.h"

#include "../ProcessorGraph/ProcessorGraph.h"
#include "../RecordNode/RecordNode.h"
#include "../../UI/ProcessorList.h"
#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"
#include "../../UI/GraphViewer.h"
#include "../Settings/InfoObject.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265359
#endif
GenericEditor::GenericEditor(GenericProcessor* owner) : AudioProcessorEditor(owner),
    desiredWidth(150),
    acquisitionIsActive(false),
    drawerWidth(25),
    drawerOpen(false), 
    isSelected(false), 
    isEnabled(true), 
    isCollapsed(false), 
    selectedStream(0),
    tNum(-1),
    drawerButtonListener(this)
{
    
    name = getAudioProcessor()->getName();
    displayName = name;

    nodeId = owner->getNodeId();

    titleFont = Font("CP Mono", "Plain", 14);

    drawerButton = std::make_unique<DrawerButton>(getNameAndId() + " Drawer Button");
    drawerButton->addListener(&drawerButtonListener);

    if (!owner->isSplitter() && !owner->isMerger())
        addAndMakeVisible(drawerButton.get());

    if (!owner->isSplitter())
    {
        streamSelector = std::make_unique<StreamSelector>(this);
        addAndMakeVisible(streamSelector.get());
    }

    backgroundGradient = ColourGradient(Colour(190, 190, 190), 0.0f, 0.0f,
        Colour(185, 185, 185), 0.0f, 120.0f, false);
    backgroundGradient.addColour(0.2f, Colour(155, 155, 155));

    backgroundColor = Colour(10, 10, 10);
}

GenericEditor::~GenericEditor()
{
    
}

void GenericEditor::updateName()
{
    nodeId = getProcessor()->getNodeId();
    repaint();
}

void GenericEditor::setDisplayName(const String& string)
{
    displayName = string;
    
    getProcessor()->updateDisplayName(displayName);

    CoreServices::updateSignalChain(this);
}

String GenericEditor::getDisplayName()
{
    return displayName;
}

int GenericEditor::getChannelDisplayNumber(int chan) const
{
	return chan;
}

void GenericEditor::addTextBoxParameterEditor(const String& parameterName, int xPos_, int yPos_)
{

    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new TextBoxParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addCheckBoxParameterEditor(const String& parameterName, int xPos_, int yPos_)
{

    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new CheckBoxParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addSliderParameterEditor(const String& parameterName, int xPos_, int yPos_)
{
    
    //std::cout << "CREATING EDITOR: " << parameterName << std::endl;

    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new SliderParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addComboBoxParameterEditor(const String& parameterName, int xPos_, int yPos_)
{

    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new ComboBoxParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addSelectedChannelsParameterEditor(const String& parameterName, int xPos_, int yPos_)
{

    //std::cout << "CREATING EDITOR: " << parameterName << std::endl;
    
    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new SelectedChannelsParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addMaskChannelsParameterEditor(const String& parameterName, int xPos_, int yPos_)
{

    //std::cout << "CREATING EDITOR: " << parameterName << std::endl;
    
    Parameter* param = getProcessor()->getParameter(parameterName);

    addCustomParameterEditor(new MaskChannelsParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addCustomParameterEditor(ParameterEditor* ed, int xPos_, int yPos_)
{
    parameterEditors.add(ed);
    addAndMakeVisible(ed);
    ed->setBounds(xPos_, yPos_, ed->getWidth(), ed->getHeight());
}



void GenericEditor::refreshColors()
{

    LOGDD(getNameAndId(), " refreshing colors.");

    enum
    {
        PROCESSOR_COLOR = 801,
        FILTER_COLOR = 802,
        SINK_COLOR = 803,
        SOURCE_COLOR = 804,
        UTILITY_COLOR = 805,
        RECORD_COLOR = 806
    };

    if (getProcessor()->isSource())
        backgroundColor = AccessClass::getProcessorList()->findColour(SOURCE_COLOR);
    else if (getProcessor()->isSink())
        backgroundColor = AccessClass::getProcessorList()->findColour(SINK_COLOR);
    else if (getProcessor()->isSplitter() || getProcessor()->isMerger() || getProcessor()->isAudioMonitor() || getProcessor()->isUtility())
        backgroundColor = AccessClass::getProcessorList()->findColour(UTILITY_COLOR);
    else if (getProcessor()->isRecordNode())
        backgroundColor = AccessClass::getProcessorList()->findColour(RECORD_COLOR);
    else
        backgroundColor = AccessClass::getProcessorList()->findColour(FILTER_COLOR);

    repaint();

}

int GenericEditor::getTotalWidth()
{
    if (isCollapsed)
        return 25;

    if (drawerButton->getToggleState())
        return desiredWidth + streamSelector->getDesiredWidth() + 14;

    return desiredWidth + 14;

}


void GenericEditor::resized()
{
    if (! isCollapsed)
    {

        if (streamSelector != 0)
        {
            if (drawerOpen)
            {
                streamSelector->setBounds(desiredWidth, 25, 
                                          streamSelector->getDesiredWidth(), 
                                          getHeight() - 35);
                streamSelector->setVisible(true);
            }
            else {
                streamSelector->setVisible(false);
            }
            
        }

        if (drawerButton != 0)
            drawerButton->setBounds(getTotalWidth() - 14, 40, 10, getHeight() - 60);
            
    }
}


bool GenericEditor::keyPressed(const KeyPress& key)
{
    return false;
}

void GenericEditor::switchSelectedState()
{
    LOGDD(getNameAndId(), " switching selected state");
    isSelected = !isSelected;
    repaint();
}

void GenericEditor::select()
{
    isSelected = true;
    repaint();

    LOGD(getNameAndId(), " editor selected");

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
}

void GenericEditor::setDesiredWidth (int width)
{
    desiredWidth = width;
    repaint();
}

void GenericEditor::editorStartAcquisition()
{
    
    LOGDD(getNameAndId(), " received message to start acquisition.");

	startAcquisition();

	if (streamSelector != 0)
	{
		streamSelector->startAcquisition();
	}

    for (int n = 0; n < parameterEditors.size(); n++)
    {

        if (parameterEditors[n]->shouldDeactivateDuringAcquisition())
            parameterEditors[n]->setEnabled(false);

    }

    acquisitionIsActive = true;

}

void GenericEditor::editorStopAcquisition()
{

    LOGDD(getNameAndId(), " received message to stop acquisition.");

	stopAcquisition();

    if (streamSelector != 0)
    {
        streamSelector->stopAcquisition();
    }

    for (int n = 0; n < parameterEditors.size(); n++)
    {

        if (parameterEditors[n]->shouldDeactivateDuringAcquisition())
            parameterEditors[n]->setEnabled(true);

    }

    acquisitionIsActive = false;
}

void GenericEditor::paint(Graphics& g)
{
    int offset = 0;

    if (isEnabled)
        g.setColour (backgroundColor);
    else
        g.setColour (Colours::lightgrey);

    if (! isCollapsed)
    {
        g.fillRect (1, 1, getWidth() - (2 + offset), getHeight() - 2);
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
        g.setColour(Colours::yellow.withAlpha(0.5f));

    }
    else
    {
        g.setColour(Colours::black);
    }

    // draw highlight box
    g.drawRect(0,0,getWidth(),getHeight(),2.0);

}

void GenericEditor::ButtonResponder::buttonClicked(Button* button)
{
    editor->checkDrawerButton(button);
}


bool GenericEditor::checkDrawerButton(Button* button)
{
    if (button == drawerButton.get())   
    {
        drawerOpen = drawerButton->getToggleState();

        AccessClass::getEditorViewport()->refreshEditors();

        return true;
    }
    else
    {
        return false;
    }

}

void GenericEditor::update(bool isEnabled_)
{
    isEnabled = isEnabled_;

    GenericProcessor* p = getProcessor();

    LOGDD(getNameAndId(), " editor updating settings");

    int numChannels;

    if (!p->isSink())
    {
        numChannels = p->getNumOutputs();
    }
    else
    {
        numChannels = p->getNumInputs();
    }

    if (p->getDataStreams().size() > 0)
        selectedStream = p->getDataStreams().getFirst()->getStreamId();
    else
        selectedStream = 0;

    if (streamSelector != nullptr)
    {
        streamSelector->beginUpdate();

        delayMonitors.clear();
        ttlMonitors.clear();

        for (auto stream : p->getDataStreams())
        {

            streamSelector->add(stream);
            delayMonitors[stream->getStreamId()] = streamSelector->getDelayMonitor(stream);
            ttlMonitors[stream->getStreamId()] = streamSelector->getTTLMonitor(stream);

            streamSelector->getTTLMonitor(stream)->updateSettings(stream->getEventChannels());

        }

        selectedStream = streamSelector->finishedUpdate();

        if (numChannels == 0)
        {
            if (drawerButton != nullptr)
                drawerButton->setVisible(false);
        }
        else
        {
            if (drawerButton != nullptr)
                drawerButton->setVisible(true);
        }
    }

    updateSettings(); // update custom settings

    updateSelectedStream(getCurrentStream());
    
    updateVisualizer(); // does nothing unless this method
                        // has been implemented
    
}

void GenericEditor::setTTLState(uint16 streamId, int bit, bool state)
{
    if (ttlMonitors.find(streamId) != ttlMonitors.end())
        ttlMonitors[streamId]->setState(bit, state);
}

void GenericEditor::setMeanLatencyMs(uint16 streamId, float latencyMs)
{
    if (delayMonitors.find(streamId) != delayMonitors.end())
        delayMonitors[streamId]->setDelay(latencyMs);
}

bool GenericEditor::getCollapsedState()
{
    return isCollapsed;
}

void GenericEditor::switchCollapsedState()
{
    setCollapsedState(!isCollapsed);
}

void GenericEditor::setCollapsedState(bool state)
{

    if (!getProcessor()->isMerger() && !getProcessor()->isSplitter())
    {

        if (!state && isCollapsed)
        {
            isCollapsed = false;
            
            for (int i = 0; i < getNumChildComponents(); i++)
            {
                Component* c = getChildComponent(i);
                c->setVisible(true);
            }

        }
        else if (state && !isCollapsed)
        {
            isCollapsed = true;
            
            for (int i = 0; i < getNumChildComponents(); i++)
            {
                Component* c = getChildComponent(i);
                c->setVisible(false);
            }
        }

        collapsedStateChanged();

        AccessClass::getEditorViewport()->refreshEditors();
    }
}

void GenericEditor::saveToXml(XmlElement* xml)
{

    xml->setAttribute("isCollapsed", isCollapsed);
    xml->setAttribute("isDrawerOpen", drawerOpen);
    xml->setAttribute("displayName", displayName);
    
    if (streamSelector != nullptr)
        xml->setAttribute("activeStream", streamSelector->getViewedIndex());

    saveCustomParametersToXml(xml);

}

void GenericEditor::loadFromXml(XmlElement* xml)
{

    setCollapsedState(xml->getBoolAttribute("isCollapsed", false));

    drawerOpen = xml->getBoolAttribute("isDrawerOpen", false);
    drawerButton->setToggleState(drawerOpen, dontSendNotification);

    displayName = xml->getStringAttribute("displayName", name);
    getProcessor()->updateDisplayName(displayName);
    
    loadCustomParametersFromXml(xml);
    
    if (streamSelector != nullptr)
        streamSelector->setViewedIndex(xml->getIntAttribute("activeStream", 0));

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

LoadButton::LoadButton(const String& name) : ImageButton(name)
{

    Image icon = ImageCache::getFromMemory(BinaryData::upload_png,
                                           BinaryData::upload_pngSize);

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

SaveButton::SaveButton(const String& name) : ImageButton(name)
{
    Image icon = ImageCache::getFromMemory(BinaryData::floppy_png,
                                           BinaryData::floppy_pngSize);

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


String GenericEditor::getName()
{
    return name;
}

String GenericEditor::getNameAndId()
{
    return name + " (" + String(getProcessor()->getNodeId()) + ")";
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
    return (GenericProcessor*) getAudioProcessor();
}

void GenericEditor::switchDest() { }


void GenericEditor::switchIO(int) { }

int GenericEditor::getPathForEditor(GenericEditor* editor)
{
    return -1;
}

void GenericEditor::editorWasClicked() {}

Colour GenericEditor::getBackgroundColor()
{
    if (isEnabled)
        return backgroundColor;
    else
        return Colours::grey;
}


void GenericEditor::setBackgroundColor(Colour c)
{
    backgroundColor = c;

    repaint();
}

ColourGradient GenericEditor::getBackgroundGradient()
{
    return backgroundGradient;
}

void GenericEditor::updateSettings() {}

void GenericEditor::updateView()
{

    const MessageManagerLock mml;
    
    for (auto ed : parameterEditors)
    {
        ed->updateView();
    }
}

void GenericEditor::updateCustomView() {}

void GenericEditor::updateVisualizer() {}

void GenericEditor::saveCustomParametersToXml(XmlElement* xml) { }

void GenericEditor::loadCustomParametersFromXml(XmlElement* xml) { }

void GenericEditor::collapsedStateChanged() {}

Array<GenericEditor*> GenericEditor::getConnectedEditors()
{
    Array<GenericEditor*> a;
    return a;
}

void GenericEditor::updateSelectedStream(uint16 streamId) 
{

    LOGD(getNameAndId(), " updating selected stream to ", streamId);

    selectedStream = streamId;

    bool streamAvailable = streamId > 0 ? true : false;

    for (auto ed : parameterEditors)
    {
        const String parameterName = ed->getParameterName();
        
        Parameter* param = getProcessor()->getParameter(parameterName);
        
        if (param == nullptr)
            continue;

        //LOGD("Parameter: ", param->getName());
        
        if (param->getScope() == Parameter::GLOBAL_SCOPE)
        {
            //LOGD("Global scope");
            ed->setParameter(getProcessor()->getParameter(ed->getParameterName()));
        }
        else if (param->getScope() == Parameter::STREAM_SCOPE)
        {
            if (streamAvailable)
            {
               //LOGD("Stream scope");
                Parameter* p2 = getProcessor()->getDataStream(streamId)->getParameter(param->getName());
                ed->setParameter(p2);
            }
            else
            {
                //LOGD("Stream not available");
                ed->setParameter(nullptr);
            }
                
        }
        
        ed->updateView();
    }

    selectedStreamHasChanged();

}

void GenericEditor::selectedStreamHasChanged() { }

void GenericEditor::streamEnabledStateChanged(uint16 streamId, bool isEnabled, bool isLoading)
{
    
    if (streamSelector != nullptr)
        streamSelector->setStreamEnabledState(streamId, isEnabled);
    
    getProcessor()->setStreamEnabled(streamId, isEnabled);

    if (!isLoading)
        CoreServices::updateSignalChain(this);
    else
    {
        streamSelector->getStreamInfoView(getProcessor()->getDataStream(streamId))->setEnabled(isEnabled);
    }

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
	double range = 0;
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
