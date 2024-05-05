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

#include "../../UI/LookAndFeel/CustomLookAndFeel.h"

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
    {

        addAndMakeVisible(drawerButton.get());
        
        streamSelector = std::make_unique<StreamSelectorTable>(this);
        addAndMakeVisible(streamSelector.get());
    }

    backgroundColor = Colour(10, 10, 10);
}

GenericEditor::~GenericEditor()
{
    LOGD("GenericEditor::destructor");
    
    streamSelector.reset();
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

void GenericEditor::addTextBoxParameterEditor(Parameter::ParameterScope scope, 
    const String& parameterName,
    int xPos_,
    int yPos_)
{
    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::TEXTBOX_EDITOR);

    addCustomParameterEditor(new TextBoxParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addToggleParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{

    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::TOGGLE_EDITOR);

    addCustomParameterEditor(new ToggleParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addBoundedValueParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{

    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::BOUNDED_VALUE_EDITOR);

    addCustomParameterEditor(new BoundedValueParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addComboBoxParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{

    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::COMBOBOX_EDITOR);

    addCustomParameterEditor(new ComboBoxParameterEditor(param), xPos_, yPos_);
}


void GenericEditor::addSelectedChannelsParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{
    
    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::SELECTED_CHANNELS_EDITOR);

    addCustomParameterEditor(new SelectedChannelsParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addMaskChannelsParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{
    
    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::MASK_CHANNELS_EDITOR);

    addCustomParameterEditor(new MaskChannelsParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addPathParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{

    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::PATH_EDITOR);
    
    addCustomParameterEditor(new PathParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addSelectedStreamParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{

    Parameter* param = getProcessor()->getParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::SELECTED_STREAM_EDITOR);

    addCustomParameterEditor(new SelectedStreamParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addTimeParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{
    Parameter* param;

    if(scope == Parameter::PROCESSOR_SCOPE)
        param = getProcessor()->getParameter(parameterName);
    else
        param = getProcessor()->getStreamParameter(parameterName);

    param->setParmeterEditorType(Parameter::ParameterEditorType::TIME_EDITOR);
    
    addCustomParameterEditor(new TimeParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addTtlLineParameterEditor(Parameter::ParameterScope scope,
    const String& parameterName,
    int xPos_,
    int yPos_)
{
    jassert(scope == Parameter::ParameterScope::STREAM_SCOPE);

    Parameter* param = getProcessor()->getStreamParameter(parameterName);
    param->setParmeterEditorType(Parameter::ParameterEditorType::TTL_LINE_EDITOR);

    addCustomParameterEditor(new TtlLineParameterEditor(param), xPos_, yPos_);
}

void GenericEditor::addSyncLineParameterEditor(TtlLineParameter* ttlParam,
    SelectedStreamParameter* syncStreamParam,
    int xPos_,
    int yPos_)
{
    ttlParam->setParmeterEditorType(Parameter::ParameterEditorType::TTL_LINE_EDITOR);
    addCustomParameterEditor(new TtlLineParameterEditor(ttlParam, syncStreamParam), xPos_, yPos_);
}

void GenericEditor::addCustomParameterEditor(ParameterEditor* ed, int xPos_, int yPos_)
{
    parameterEditors.add(ed);
    addAndMakeVisible(ed);
    ed->setBounds(xPos_, yPos_, ed->getWidth(), ed->getHeight());
}



void GenericEditor::refreshColors()
{

    // LOGD(getNameAndId(), " refreshing colors.");

    if (getProcessor()->isSource())
        backgroundColor = getLookAndFeel().findColour(ProcessorColor::IDs::SOURCE_COLOR);
    else if (getProcessor()->isSink())
        backgroundColor = getLookAndFeel().findColour(ProcessorColor::IDs::SINK_COLOR);
    else if (getProcessor()->isSplitter() || getProcessor()->isMerger() || getProcessor()->isAudioMonitor() || getProcessor()->isUtility())
        backgroundColor = getLookAndFeel().findColour(ProcessorColor::IDs::UTILITY_COLOR);
    else if (getProcessor()->isRecordNode())
        backgroundColor = getLookAndFeel().findColour(ProcessorColor::IDs::RECORD_COLOR);
    else
        backgroundColor = getLookAndFeel().findColour(ProcessorColor::IDs::FILTER_COLOR);
    
    // loop though all parameter editors and update their parameter's color
    for (auto ed : parameterEditors)
    {
        const String parameterName = ed->getParameterName();
        
        if (getProcessor()->hasParameter(parameterName))
        {
            Parameter* procParam = getProcessor()->getParameter(parameterName);

            if (procParam->getType() == Parameter::ParameterType::SELECTED_CHANNELS_PARAM ||
                procParam->getType() == Parameter::ParameterType::MASK_CHANNELS_PARAM)
                    getProcessor()->setColor(parameterName, backgroundColor);
        }
        else if (selectedStream > 0)
        {
            auto currStream = getProcessor()->getDataStream(selectedStream);
            
            if (currStream->hasParameter(parameterName))
            {
                Parameter* streamParam = currStream->getParameter(parameterName);

                if (streamParam->getType() == Parameter::ParameterType::SELECTED_CHANNELS_PARAM ||
                    streamParam->getType() == Parameter::ParameterType::MASK_CHANNELS_PARAM)
                    for (auto stream : getProcessor()->getDataStreams())
                        getProcessor()->getDataStream(stream->getStreamId())->setColor(parameterName, backgroundColor);
            }
        }
        
    }

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
                streamSelector->setBounds(desiredWidth, 29, 
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

    for (auto param : getProcessor()->getParameters())
    {
        if (param->shouldDeactivateDuringAcquisition())
            param->setEnabled(false);
    }

    for (auto stream : getProcessor()->dataStreams)
    {
        for (auto param : stream->getParameters())
        {
            if (param->shouldDeactivateDuringAcquisition())
                param->setEnabled(false);
        }
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

    for (auto param : getProcessor()->getParameters())
    {
        if (param->shouldDeactivateDuringAcquisition())
            param->setEnabled(true);
    }

    for (auto stream : getProcessor()->dataStreams)
    {
        for (auto param : stream->getParameters())
        {
            if (param->shouldDeactivateDuringAcquisition())
                param->setEnabled(true);
        }
    }

    acquisitionIsActive = false;
}

void GenericEditor::paint(Graphics& g)
{
   
    if (isEnabled)
        g.setColour (backgroundColor);
    else
        g.setColour (findColour(ThemeColors::widgetBackground));

    if (! isCollapsed)
    {
        // Paint titlebar and bottom edge
        Path topBottomRoundedEdge;
        topBottomRoundedEdge.addRoundedRectangle (1, 1,
                                              getWidth() - 2, getHeight() - 2, 
                                              5.0f, 5.0f, 
                                              true, true, 
                                              true, true);

        g.fillPath (topBottomRoundedEdge);
        
        // Paint body
        g.setGradientFill (getBackgroundGradient());
        Path mainBody;
        mainBody.addRectangle (1, 23, getWidth() - 2, getHeight() - 30);
        g.fillPath (mainBody);
    }
    else
    {
        g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 5.0f);
    }

    // draw title
    if (!isCollapsed)
    {
        g.setColour(isEnabled ? Colours::white : findColour(ThemeColors::defaultText).withAlpha(0.5f));
        g.setFont( Font("Mono", "Plain", 12) );
        g.drawText (String(nodeId), 10, 6, 30, 15, Justification::left, false);
        g.setFont (Font("CP Mono", "Plain", 16));
        g.drawText (displayName.toUpperCase(), 35, 5, 500, 15, Justification::left, false);
    }
    else
    {
        g.addTransform(AffineTransform::rotation(-M_PI/2.0));
        g.setColour(isEnabled ? Colours::white : findColour(ThemeColors::defaultText).withAlpha(0.5f));
        g.setFont (Font("CP Mono", "Plain", 14));
        g.drawText (displayName.toUpperCase(), - getHeight() + 6, 5, 500, 15, Justification::left, false);
        g.addTransform(AffineTransform::rotation(M_PI/2.0));
    }

    if (isSelected)
    {
        // draw highlight box
        g.setColour(Colours::yellow);
        g.drawRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 5.0f, 2.0f);

    }

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

void GenericEditor::updateDelayAndTTLMonitors()
{
    for (auto stream : getProcessor()->getDataStreams())
    {
        delayMonitors[stream->getStreamId()] = streamSelector->getDelayMonitor(stream);
        ttlMonitors[stream->getStreamId()] = streamSelector->getTTLMonitor(stream);

        streamSelector->getTTLMonitor(stream)->updateSettings(stream->getEventChannels());

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

            LOGD("Editor ", getNameAndId(), " updating stream ", stream->getName());

            streamSelector->add(stream);
        }

        selectedStream = streamSelector->finishedUpdate();

        updateDelayAndTTLMonitors();

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
    {
        if (ttlMonitors[streamId] != nullptr)
            ttlMonitors[streamId]->setState(bit, state);
    }
}

void GenericEditor::setMeanLatencyMs(uint16 streamId, float latencyMs)
{
    if (delayMonitors.find(streamId) != delayMonitors.end())
    {
        if (delayMonitors[streamId] != nullptr)
            delayMonitors[streamId]->setDelay(latencyMs);
    }
        
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

                if (c == drawerButton.get() && selectedStream == 0)
                    continue;

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
        g.setColour(findColour(ThemeColors::defaultFill).brighter(0.3f));
    else
        g.setColour(findColour(ThemeColors::defaultFill));

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

    repaint();
}

void UtilityButton::setRadius(float r)
{
    radius = r;
}

void UtilityButton::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{

    if (getToggleState())
    {
        g.setColour(findColour(ThemeColors::highlightedFill));
    }
    else
    {
        g.setColour(findColour(ThemeColors::widgetBackground));
    }

    g.fillPath(outlinePath);

    if (isMouseOver || !isEnabled)
        g.setColour(findColour(ThemeColors::outline).withAlpha(0.4f));
    else
        g.setColour(findColour(ThemeColors::outline));

    g.strokePath(outlinePath, PathStrokeType(1.0f));


    //int stringWidth = font.getStringWidth(getName());

    g.setFont(font);

    if (isEnabled || !isButtonDown)
        g.setColour(findColour(ThemeColors::defaultText));
    else
        g.setColour(findColour(ThemeColors::defaultText).withAlpha(0.4f));

    g.drawFittedText(label,0,0,getWidth(),getHeight(),Justification::centred,2,1.0f);
}

void UtilityButton::resized()
{

    outlinePath.clear();

    outlinePath.addRoundedRectangle(1.0f, 1.0f, (float)getWidth() - 2.0f, (float)getHeight() - 2.0f, 
                                    radius, radius, roundUL, roundUR, roundLL, roundLR);

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
    backgroundGradient = ColourGradient(findColour(ThemeColors::componentBackground).darker(0.1f), 0.0f, 0.0f,
                                        findColour(ThemeColors::componentBackground).brighter(0.2f), 0.0f, 120.0f, false);

    return backgroundGradient;
}

void GenericEditor::updateSettings() {}

void GenericEditor::updateView()
{

    const MessageManagerLock mml;
    
    //for (auto ed : parameterEditors)
    //{
    //    ed->updateView();
    //}

}

ParameterEditor* GenericEditor::getParameterEditor(const String& parameterName)
{
    for (auto e : parameterEditors)
    {
        if (e->getParameterName().equalsIgnoreCase(parameterName))
            return e;
    }
    
    return nullptr;
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
        if (ed->shouldUpdateOnSelectedStreamChanged() == false)
            continue;

        const String parameterName = ed->getParameterName();
        
        if (getProcessor()->hasParameter(parameterName))
        {
            ed->setParameter(getProcessor()->getParameter(ed->getParameterName()));
        }
        else
        {
            if (streamAvailable)
            {
               //LOGD("Stream scope");
                auto stream = getProcessor()->getDataStream(streamId);
                
                if (stream->hasParameter(parameterName))
                {
                    Parameter* streamParam = stream->getParameter(parameterName);
                    ed->setParameter(streamParam);
                }
                else
                {
                    continue;
                }
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
       // streamSelector->getStreamInfoView(getProcessor()->getDataStream(streamId))->setEnabled(isEnabled);
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

LevelMonitor::LevelMonitor(GenericProcessor *p) : Button("LevelMonitor")
{
	processor = p;
	updateFreq = 60; //10Hz
	fillPercentage = 0.0;
	lastUpdateTime = 0.0;
	stateChangeSinceLastUpdate = false;
}

LevelMonitor::~LevelMonitor() {}

void LevelMonitor::paintButton(Graphics& g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(findColour(ThemeColors::outline));
	g.drawRoundedRectangle(1, 1, this->getWidth() - 2, this->getHeight() - 2, 3, 1);
	g.setColour(findColour(ThemeColors::widgetBackground));
	g.fillRoundedRectangle(1, 1, this->getWidth() - 2, this->getHeight() - 2, 3);

	if (fillPercentage < 0.7)
		g.setColour(Colours::yellow);
	else if (fillPercentage < 0.9)
		g.setColour(Colours::orange);
	else
		g.setColour(Colours::red);

	float barHeight = (this->getHeight() - 4) * fillPercentage;
	g.fillRoundedRectangle(2, this->getHeight() - 2 - barHeight, this->getWidth() - 4, barHeight, 2);
}

void LevelMonitor::setFillPercentage(float fill_)
{
	fillPercentage = fill_;
	repaint();
}
