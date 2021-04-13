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

#include "LfpDisplayCanvas.h"
#include "LfpDisplayNode.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "LfpViewport.h"
#include "ColourSchemes/ChannelColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpDisplayCanvas -

#define MAX_N_SAMP 3000 // used for screen buffer; this could be resized dynamically depending on the display width


LfpDisplayCanvas::LfpDisplayCanvas(LfpDisplayNode* processor_, SplitLayouts sl) :
                  processor(processor_), selectedLayout(sl)
{
    
    juce::TopLevelWindow::getTopLevelWindow(0)->addKeyListener(this);
    
    optionsDrawerIsOpen = false;

    Array<DisplayBuffer*> displayBuffers = processor->getDisplayBuffers();
    Array<LfpDisplaySplitter*> splits;

    for (int i = 0; i < 3; i++) // create 3 split displays
    {
        displaySplits.add(new LfpDisplaySplitter(processor, this, displayBuffers[0], i));
        addChildComponent(displaySplits[i]);
        splits.add(displaySplits[i]);
        
        
        // create options menu per split display
        //options.add(new LfpDisplayOptions(this, 
        //    displaySplits[i], 
        //    displaySplits[i]->timescale, 
        //    displaySplits[i]->lfpDisplay, 
        //    processor));

        addChildComponent(displaySplits[i]->options);
        displaySplits[i]->options->setAlwaysOnTop(true);

        if (i == 0)
            displaySplits[i]->options->setVisible(true);

        //displaySplits[i]->lfpDisplay->options = options[i];
        //displaySplits[i]->lfpDisplay->setNumChannels(displaySplits[i]->nChans);

    }

    processor->setSplitDisplays(splits);

    doubleVerticalSplitRatio = 0.5f;
    doubleHorizontalSplitRatio = 0.5f;

    tripleHorizontalSplitRatio.add(0.33f);
    tripleHorizontalSplitRatio.add(0.66f);

    tripleVerticalSplitRatio.add(0.33f);
    tripleVerticalSplitRatio.add(0.66f);

    setLayout(sl);

    addMouseListener(this, true);

    borderToDrag = -1;
    
}

LfpDisplayCanvas::~LfpDisplayCanvas()
{
    juce::TopLevelWindow::getTopLevelWindow(0)->removeKeyListener(this);
}

void LfpDisplayCanvas::resized()
{

    ///refresh();

    //for (auto split : displaySplits)
   // {
   //     split->setVisible(false);
   // }

    int borderSize = 5;

    if(selectedLayout == SINGLE)
    {
        displaySplits[0]->setVisible(true);

        
        displaySplits[0]->deselect(); // to remove boundary
        displaySplits[0]->options->setVisible(true);
        displaySplits[1]->options->setVisible(false);
        displaySplits[2]->options->setVisible(false);

        displaySplits[1]->setVisible(false);
        displaySplits[2]->setVisible(false);

        displaySplits[0]->setBounds(0, 0, getWidth(), getHeight());
        
    }
    else if(selectedLayout == TWO_VERT)
    {
        displaySplits[0]->setVisible(true);
        displaySplits[1]->setVisible(true);

        displaySplits[2]->setVisible(false);

        displaySplits[0]->setBounds(0, 
            0, 
            getWidth() * doubleVerticalSplitRatio - borderSize,
            getHeight());

        displaySplits[1]->setBounds(getWidth() * doubleVerticalSplitRatio + borderSize,
            0, 
            getWidth() * (1- doubleVerticalSplitRatio) - borderSize,
            getHeight());

        if (!displaySplits[1]->getSelectedState())
            displaySplits[0]->select();
        else
            displaySplits[1]->select();
        
    }
    else if(selectedLayout == THREE_VERT)
    {
        displaySplits[0]->setVisible(true);
        displaySplits[1]->setVisible(true);
        displaySplits[2]->setVisible(true);

        displaySplits[0]->setBounds(0, 
            0, 
            getWidth() * tripleVerticalSplitRatio[0] - borderSize,
            getHeight());

        displaySplits[1]->setBounds(getWidth() * tripleVerticalSplitRatio[0] + borderSize,
            0, 
            getWidth() * (tripleVerticalSplitRatio[1]-tripleVerticalSplitRatio[0]) - borderSize,
            getHeight());

        displaySplits[2]->setBounds(getWidth() * (tripleVerticalSplitRatio[1]) + borderSize,
            0, 
            getWidth() * (1-tripleVerticalSplitRatio[1]) - borderSize,
            getHeight());

        if (!displaySplits[1]->getSelectedState() && !displaySplits[2]->getSelectedState())
            displaySplits[0]->select();

    }
    else if(selectedLayout == TWO_HORZ)
    {
        displaySplits[0]->setVisible(true);
        displaySplits[1]->setVisible(true);

        displaySplits[2]->setVisible(false);

        displaySplits[0]->setBounds(0, 
            0, 
            getWidth(), 
            getHeight() * doubleHorizontalSplitRatio - borderSize);

        displaySplits[1]->setBounds(0, 
            getHeight() * doubleHorizontalSplitRatio + borderSize,
            getWidth(), 
            getHeight() * (1-doubleHorizontalSplitRatio) - borderSize);

        if (!displaySplits[1]->getSelectedState())
            displaySplits[0]->select();
    }
    else{
        
        displaySplits[0]->setVisible(true);
        displaySplits[1]->setVisible(true);
        displaySplits[2]->setVisible(true);

        displaySplits[0]->setBounds(0, 
            0, 
            getWidth(), 
            getHeight() * tripleHorizontalSplitRatio[0] - borderSize);

        displaySplits[1]->setBounds(0, 
            getHeight() * tripleHorizontalSplitRatio[0] + borderSize,
            getWidth(), 
            getHeight() * (tripleHorizontalSplitRatio[1] - tripleHorizontalSplitRatio[0]) - borderSize);

        displaySplits[2]->setBounds(0, 
            getHeight() * (tripleHorizontalSplitRatio[1]) + borderSize,
            getWidth(), 
            getHeight() * (1-tripleHorizontalSplitRatio[1]) - borderSize);

        if (!displaySplits[1]->getSelectedState() && !displaySplits[2]->getSelectedState())
            displaySplits[0]->select();
    }

    syncDisplays();

    for (int i = 0; i < 3; i++)
    {
        if (optionsDrawerIsOpen)
            displaySplits[i]->options->setBounds(0, getHeight() - 200, getWidth(), 200);
        else
            displaySplits[i]->options->setBounds(0, getHeight() - 55, getWidth(), 55);
    }

}

void LfpDisplayCanvas::beginAnimation()
{

    if (true)
    {
        syncDisplays();

        for (auto split : displaySplits)
        {
            split->beginAnimation();
        }

        startCallbacks();
    }    
}

void LfpDisplayCanvas::endAnimation()
{
    if (true)
    {

        stopCallbacks();
    }
}

void LfpDisplayCanvas::syncDisplays()
{
    for (auto split : displaySplits)
    {
        split->syncDisplay();
    }

}

void LfpDisplayCanvas::update()
{
    // update settings

    for (auto split : displaySplits)
    {
        split->updateSettings();
    }

}

//void LfpDisplayCanvas::setParameter(int param, float val)
//{
    // not used for anything, since LfpDisplayCanvas is not a processor
//
void LfpDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again

    if (true)
    {
        for (auto split : displaySplits)
        {
            split->refreshSplitterState();
        }
    }

}

void LfpDisplayCanvas::select(LfpDisplaySplitter* splitter)
{
    for (auto split : displaySplits)
    {
        if (split != splitter)
        {
            split->deselect();
            split->options->setVisible(false);
        }
        else {
            split->options->setVisible(true);
            
        }
            
    }

    splitter->options->repaint();
}

void LfpDisplayCanvas::setLayout(SplitLayouts sl)
{
    selectedLayout = sl;

    resized();
}

bool LfpDisplayCanvas::makeRoomForOptions(int splitID)
{
    if (selectedLayout == TWO_HORZ)
    {
        if (splitID < 1)
            return false;
    }
    else if (selectedLayout == THREE_HORZ)
    {
        if (splitID < 2)
            return false;
    }

    return true;


}

bool LfpDisplayCanvas::canSelect(int splitID)
{
    if (selectedLayout != SINGLE)
    {
        return true;
    }
    
    return false;
}

void LfpDisplayCanvas::mouseMove(const MouseEvent& e)
{
    MouseEvent event = e.getEventRelativeTo(this);

    int borderSize = 5;

    if (selectedLayout == TWO_VERT)
    {
        int relativeX = event.position.getX();
        //
        if ((relativeX > getWidth() * doubleVerticalSplitRatio - borderSize) &&
            (relativeX < getWidth() * doubleVerticalSplitRatio + borderSize))
        {
            setMouseCursor(MouseCursor::LeftRightResizeCursor);
            //std::cout << "over center border" << std::endl;
        }
        else {
            setMouseCursor(MouseCursor::NormalCursor);
        }
    }
    else if (selectedLayout == THREE_VERT)
    {
        int relativeX = event.position.getX();

        if ((relativeX > getWidth() * tripleVerticalSplitRatio[0] - borderSize) &&
            (relativeX < getWidth() * tripleVerticalSplitRatio[0] + borderSize))
        {
            setMouseCursor(MouseCursor::LeftRightResizeCursor);
        }
        else if (
            (relativeX > getWidth() * tripleVerticalSplitRatio[1] - borderSize) &&
            (relativeX < getWidth() * tripleVerticalSplitRatio[1] + borderSize)
            )
        {
            setMouseCursor(MouseCursor::LeftRightResizeCursor);
        }
        else {
            setMouseCursor(MouseCursor::NormalCursor);
        }
    }  else if (selectedLayout == TWO_HORZ)
    {
        int relativeY = event.position.getY();
        //
        if ((relativeY > getHeight() * doubleHorizontalSplitRatio - borderSize) &&
            (relativeY < getHeight() * doubleHorizontalSplitRatio + borderSize))
        {
            setMouseCursor(MouseCursor::UpDownResizeCursor);
        }
        else {
            setMouseCursor(MouseCursor::NormalCursor);
        }
    }
    else if (selectedLayout == THREE_HORZ)
    {
        int relativeY = event.position.getY();

        if ((relativeY > getHeight() * tripleHorizontalSplitRatio[0] - borderSize) &&
            (relativeY < getHeight() * tripleHorizontalSplitRatio[0] + borderSize))
        {
            setMouseCursor(MouseCursor::UpDownResizeCursor);
        }
        else if (
            (relativeY > getHeight() * tripleHorizontalSplitRatio[1] - borderSize) &&
            (relativeY < getHeight() * tripleHorizontalSplitRatio[1] + borderSize)
            )
        {
            setMouseCursor(MouseCursor::UpDownResizeCursor);
        }
        else {
            setMouseCursor(MouseCursor::NormalCursor);
        }
    }

}

void LfpDisplayCanvas::mouseDrag(const MouseEvent& e)
{
    MouseEvent event = e.getEventRelativeTo(this);

    int borderSize = 5;

    if (selectedLayout == TWO_VERT)
    {
        int relativeX = event.getMouseDownX();

        //std::cout << relativeX << std::endl;

        if ((relativeX > getWidth() * doubleVerticalSplitRatio - borderSize) &&
            (relativeX < getWidth() * doubleVerticalSplitRatio + borderSize))
        {
            borderToDrag = 0;
        }

        if (borderToDrag == 0)
        {

            doubleVerticalSplitRatio = float(event.position.getX()) / float(getWidth());

            if (doubleVerticalSplitRatio < 0.15)
                doubleVerticalSplitRatio = 0.15;
            else if (doubleVerticalSplitRatio > 0.85)
                doubleVerticalSplitRatio = 0.85;

            // std::cout << doubleVerticalSplitRatio << std::endl;

        }

    } else if (selectedLayout == THREE_VERT)
    {
        int relativeX = event.getMouseDownX();

        //std::cout << relativeX << std::endl;

        if ((relativeX > getWidth() * tripleVerticalSplitRatio[0] - borderSize) &&
            (relativeX < getWidth() * tripleVerticalSplitRatio[0] + borderSize))
        {
            borderToDrag = 0;
        } else if ((relativeX > getWidth() * tripleVerticalSplitRatio[1] - borderSize) &&
            (relativeX < getWidth() * tripleVerticalSplitRatio[1] + borderSize))
        {
            borderToDrag = 1;
        }

        if (borderToDrag == 0)
        {

            tripleVerticalSplitRatio.set(0, float(event.position.getX()) / float(getWidth()));

            if (tripleVerticalSplitRatio[0] < 0.15)
                tripleVerticalSplitRatio.set(0, 0.15);
            else if (tripleVerticalSplitRatio[0] > tripleVerticalSplitRatio[1] - 0.15)
                tripleVerticalSplitRatio.set(0, tripleVerticalSplitRatio[1] - 0.15);

           // std::cout << doubleVerticalSplitRatio << std::endl;

        }

        else if (borderToDrag == 1)
        {

            tripleVerticalSplitRatio.set(1, float(event.position.getX()) / float(getWidth()));

            if (tripleVerticalSplitRatio[1] < tripleVerticalSplitRatio[0] + 0.15)
                tripleVerticalSplitRatio.set(1, tripleVerticalSplitRatio[0] + 0.15);
            else if (tripleVerticalSplitRatio[1] > 0.85)
                tripleVerticalSplitRatio.set(1, 0.85);

            // std::cout << doubleVerticalSplitRatio << std::endl;

        }

    } else if (selectedLayout == TWO_HORZ)
    {
        int relativeY = event.getMouseDownY();
        //
        if ((relativeY > getHeight() * doubleHorizontalSplitRatio - borderSize) &&
            (relativeY < getHeight() * doubleHorizontalSplitRatio + borderSize))

        {
            borderToDrag = 0;
        }

        if (borderToDrag == 0)
        {

            doubleHorizontalSplitRatio = float(event.position.getY()) / float(getHeight());

            if (doubleHorizontalSplitRatio < 0.15)
                doubleHorizontalSplitRatio = 0.15;
            else if (doubleHorizontalSplitRatio > 0.85)
                doubleHorizontalSplitRatio = 0.85;

          //  std::cout << doubleHorizontalSplitRatio << std::endl;
        }
        
    }
    else if (selectedLayout == THREE_HORZ)
    {
        int relativeY = event.getMouseDownY();

        //std::cout << relativeX << std::endl;

        if ((relativeY > getHeight() * tripleHorizontalSplitRatio[0] - borderSize) &&
            (relativeY < getHeight() * tripleHorizontalSplitRatio[0] + borderSize))
        {
            borderToDrag = 0;
        }
        else if ((relativeY > getHeight() * tripleHorizontalSplitRatio[1] - borderSize) &&
            (relativeY < getHeight() * tripleHorizontalSplitRatio[1] + borderSize))
        {
            borderToDrag = 1;
        }

        if (borderToDrag == 0)
        {

            tripleHorizontalSplitRatio.set(0, float(event.position.getY()) / float(getHeight()));

            if (tripleHorizontalSplitRatio[0] < 0.15)
                tripleHorizontalSplitRatio.set(0, 0.15);
            else if (tripleHorizontalSplitRatio[0] > tripleHorizontalSplitRatio[1] - 0.15)
                tripleHorizontalSplitRatio.set(0, tripleHorizontalSplitRatio[1] - 0.15);

            // std::cout << doubleVerticalSplitRatio << std::endl;

        }

        else if (borderToDrag == 1)
        {

            tripleHorizontalSplitRatio.set(1, float(event.position.getY()) / float(getHeight()));

            if (tripleHorizontalSplitRatio[1] < tripleHorizontalSplitRatio[0] + 0.15)
                tripleHorizontalSplitRatio.set(1, tripleHorizontalSplitRatio[0] + 0.15);
            else if (tripleHorizontalSplitRatio[1] > 0.85)
                tripleHorizontalSplitRatio.set(1, 0.85);

            // std::cout << doubleVerticalSplitRatio << std::endl;

        }

    }

}

void LfpDisplayCanvas::mouseUp(const MouseEvent& e)
{
    if (borderToDrag >= 0)
    {
        std::cout << "Mouse up" << std::endl;

        resized();
        borderToDrag = -1;

        
    }
}

void LfpDisplayCanvas::toggleOptionsDrawer(bool isOpen)
{
    optionsDrawerIsOpen = isOpen;
    
    for (int i = 0; i < 3; i++)
    {
        if (optionsDrawerIsOpen)
            displaySplits[i]->options->setBounds(0, getHeight() - 200, getWidth(), 200);
        else
            displaySplits[i]->options->setBounds(0, getHeight() - 55, getWidth(), 55);
    }
}

int LfpDisplayCanvas::getTotalSplits()
{
    return displaySplits.size();
}

void LfpDisplayCanvas::paint(Graphics& g)
{
    
    //std::cout << "Painting" << std::endl;
    //g.setColour(Colour(0,0,0)); // for high-precision per-pixel density display, make background black for better visibility

}

void LfpDisplayCanvas::refresh()
{
    if (true)
    { 
        for (auto split : displaySplits)
        {
            if (split->isVisible())
                 split->refresh();
        }
    }
}

void LfpDisplayCanvas::comboBoxChanged(ComboBox* cb)
{
    if(cb == displaySelection)
    {
        int id = displaySelection->getSelectedId();

       // options = optionsList[id-1];
        //this->repaint();
        //options->repaint();
    }
}

bool LfpDisplayCanvas::keyPressed(const KeyPress& key)
{
    if (key.getKeyCode() == key.spaceKey)
    {
        for (auto split : displaySplits)
        {
            split->handleSpaceKeyPauseEvent();
        }
        
        return true;
    }

    return false;
}

bool LfpDisplayCanvas::keyPressed(const KeyPress& key, Component* orig)
{
    if (getTopLevelComponent() == orig && isVisible())
    {
        return keyPressed(key);
    }
    return false;
}

void LfpDisplayCanvas::saveVisualizerParameters(XmlElement* xml)
{
    for (int i = 0; i < 3; i++)
    {
        displaySplits[i]->options->saveParameters(xml);
    }

    XmlElement* xmlNode = xml->createNewChildElement("CANVAS");
    xmlNode->setAttribute("doubleVerticalSplitRatio", doubleVerticalSplitRatio);
    xmlNode->setAttribute("doubleHorizontalSplitRatio", doubleHorizontalSplitRatio);
    xmlNode->setAttribute("tripleHorizontalSplitRatio", String(tripleHorizontalSplitRatio[0]) 
                                                      + "," + String(tripleHorizontalSplitRatio[1]));
    xmlNode->setAttribute("tripleVerticalSplitRatio", String(tripleVerticalSplitRatio[0]) 
                                                      + "," + String(tripleVerticalSplitRatio[1]));

    LfpDisplayEditor* ed = (LfpDisplayEditor*) processor->getEditor();
    ed->saveVisualizerParameters(xml);
}

void LfpDisplayCanvas::loadVisualizerParameters(XmlElement* xml)
{
    for (int i = 0; i < 3; i++)
    {
        displaySplits[i]->options->loadParameters(xml);
    }

    forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("CANVAS"))
		{
			doubleHorizontalSplitRatio = xmlNode->getStringAttribute("doubleHorizontalSplitRatio").getFloatValue();
            doubleVerticalSplitRatio = xmlNode->getStringAttribute("doubleVerticalSplitRatio").getFloatValue ();

            juce::StringArray values;

            values.addTokens(xmlNode->getStringAttribute("tripleHorizontalSplitRatio"), ",", String::empty);
            tripleHorizontalSplitRatio.set(0, values[0].getFloatValue());
            tripleHorizontalSplitRatio.set(1, values[1].getFloatValue());
            values.clear();

            values.addTokens(xmlNode->getStringAttribute("tripleVerticalSplitRatio"), ",", String::empty);
            tripleVerticalSplitRatio.set(0, values[0].getFloatValue());
            tripleVerticalSplitRatio.set(1, values[1].getFloatValue());

            resized();
		}
	}

    LfpDisplayEditor* ed = (LfpDisplayEditor*) processor->getEditor();
    ed->loadVisualizerParameters(xml);
}


/*****************************************************/
LfpDisplaySplitter::LfpDisplaySplitter(LfpDisplayNode* node,
                                       LfpDisplayCanvas* canvas_,
                                       DisplayBuffer* db,
                                       int id) :
                    timebase(1.0f), displayGain(1.0f),   timeOffset(0.0f), triggerChannel(-1), trialAveraging(false),
                    splitID(id), processor(node), displayBuffer(db), canvas(canvas_)
{

    isSelected = false;

    viewport = new LfpViewport(this);
    lfpDisplay = new LfpDisplay(this, viewport);
    timescale = new LfpTimescale(this, lfpDisplay);
    options = new LfpDisplayOptions(canvas, this, timescale, lfpDisplay, node);

    subprocessorSelection = new ComboBox("Subprocessor selection");
    subprocessorSelection->addListener(this);

    lfpDisplay->options = options;

    timescale->setTimebase(timebase);

    viewport->setViewedComponent(lfpDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    //isChannelEnabled.insertMultiple(0, true, 10000); // max 10k channels

    addAndMakeVisible(viewport);
    addAndMakeVisible(timescale);
    addAndMakeVisible(subprocessorSelection);

    nChans = 0;

    //resizeSamplesPerPixelBuffer(nChans);

    drawSaturationWarning = false;
    drawClipWarning = false;

    isLoading = true;
    isUpdating = false;

    displayBuffer = nullptr;

}

LfpDisplaySplitter::~LfpDisplaySplitter()
{
    //samplesPerPixel.clear();
}


/*void LfpDisplaySplitter::resizeSamplesPerPixelBuffer(int numCh)
{
    // 3D array: dimensions channels x samples x samples per pixel
    samplesPerPixel.clear();
    samplesPerPixel.resize(numCh);

    sampleCountPerPixel.clear();
    sampleCountPerPixel.insertMultiple(0, 0, MAX_N_SAMP);
}*/

void LfpDisplaySplitter::resized()
{

    const int timescaleHeight = 30;

    timescale->setBounds(leftmargin, 0, getWidth()-scrollBarThickness, timescaleHeight);

    if (canvas->makeRoomForOptions(splitID))
    {
        viewport->setBounds(0, timescaleHeight, getWidth(), getHeight() - 87);
    }
    else
    {
        viewport->setBounds(0, timescaleHeight, getWidth(), getHeight() - 32);
    }

    if (screenBuffer != nullptr)
    {
        if (screenBuffer->getNumSamples() < getWidth())
            refreshScreenBuffer();
    }
       
    if (nChans > 0)
    {
        //std::cout << "Changing view for display " << splitID << std::endl;

        if (lfpDisplay->getSingleChannelState())
            lfpDisplay->setChannelHeight(viewport->getHeight(), false);
 
        lfpDisplay->setBounds(0, 0,
            getWidth()-scrollBarThickness, 
            lfpDisplay->getChannelHeight()*lfpDisplay->drawableChannels.size());

        lfpDisplay->restoreViewPosition();
    }
    else
    {
        lfpDisplay->setBounds(0, 0, getWidth(), getHeight());
    }

    subprocessorSelection->setBounds(4, 4, 140, 22);
}

void LfpDisplaySplitter::resizeToChannels(bool respectViewportPosition)
{
    //std::cout << "Resize to channels!!!" << std::endl;

    lfpDisplay->setBounds(0, 0, 
        getWidth()-scrollBarThickness, 
        lfpDisplay->getChannelHeight()*lfpDisplay->drawableChannels.size());
    
    // if param is flagged, move the viewport scroll back to same relative position before
    // resize took place
    if (!respectViewportPosition) return;
    
    // get viewport scroll position as ratio against lfpDisplay's dims
    // so that we can set scrollbar back to rough position before resize
    // (else viewport scrolls back to top after resize)
    const double yPositionRatio = viewport->getViewPositionY() / (double)lfpDisplay->getHeight();
    const double xPositionRatio = viewport->getViewPositionX() / (double)lfpDisplay->getWidth();
    
    viewport->setViewPosition(lfpDisplay->getWidth() * xPositionRatio,
                              lfpDisplay->getHeight() * yPositionRatio);
}

void LfpDisplaySplitter::beginAnimation()
{

    if (displayBuffer != nullptr)
    {

        displayBuffer->resetIndices();

        displayBufferSize = displayBuffer->getNumSamples();

        numTrials = -1;

    }    
}

void LfpDisplaySplitter::select()
{
    if (canvas->canSelect(splitID))
    {
        isSelected = true;

        canvas->select(this);
    }

    repaint();
}

void LfpDisplaySplitter::deselect()
{
    isSelected = false;

    repaint();
}

void LfpDisplaySplitter::updateSettings()
{

    isUpdating = true;

    Array<DisplayBuffer*> availableBuffers = processor->getDisplayBuffers();

    if (availableBuffers.size() == 0)
        displayBuffer = nullptr;

    subprocessorSelection->clear(dontSendNotification);

    bool foundMatchingBuffer = false;

    for (auto buffer : availableBuffers)
    {
        subprocessorSelection->addItem(buffer->name, buffer->id);

        if (displayBuffer != nullptr)
        {
            if (buffer->id == displayBuffer->id)
                foundMatchingBuffer = true;
        }
    }

    if (!foundMatchingBuffer)
        displayBuffer = nullptr;

    if (displayBuffer == nullptr) // displayBuffer was deleted
    {
        if (availableBuffers.size() > 0)
        {
            displayBuffer = availableBuffers[0];
        }
    }

    if (displayBuffer == nullptr) // no inputs to this processor
    {
        nChans = 0;
        sampleRate = 44100.0f;

        options->setEnabled(true);
    }
    else {
        
        subprocessorSelection->setSelectedId(displayBuffer->id, dontSendNotification);
        displayBufferSize = displayBuffer->getNumSamples();
        nChans = displayBuffer->numChannels;
        //resizeSamplesPerPixelBuffer(nChans);
        sampleRate = displayBuffer->sampleRate;

        options->setEnabled(true);
        channelOverlapFactor = options->selectedOverlapValue.getFloatValue();
    }

    //std::cout << "Sample rate for display " << splitID << ": " << sampleRate << std::endl;
    
    if (screenBuffer == nullptr) // not yet initialized
    {
        screenBuffer = new AudioSampleBuffer(nChans + 1, getWidth());
        screenBufferMin = new AudioSampleBuffer(nChans + 1, getWidth());
        screenBufferMean = new AudioSampleBuffer(nChans + 1, getWidth());
        screenBufferMax = new AudioSampleBuffer(nChans + 1, getWidth());
    }
    else {
        if (nChans != lfpDisplay->getNumChannels()) // new channel count
        {
            refreshScreenBuffer();
        }
    }

   // std::cout << "DISPLAY SPLIT " << splitID << " UPDATING SETTINGS." << std::endl;

    lfpDisplay->setNumChannels(nChans);

    for (int i = 0; i < nChans; i++) // update channel metadata
    {
        lfpDisplay->channels[i]->setName(displayBuffer->channelMetadata[i].name);
        lfpDisplay->channels[i]->setGroup(displayBuffer->channelMetadata[i].group);
        lfpDisplay->channels[i]->setDepth(displayBuffer->channelMetadata[i].ypos);
        lfpDisplay->channels[i]->updateType();

        lfpDisplay->channelInfo[i]->setName(displayBuffer->channelMetadata[i].name);
        lfpDisplay->channelInfo[i]->setGroup(displayBuffer->channelMetadata[i].group);
        lfpDisplay->channelInfo[i]->setDepth(displayBuffer->channelMetadata[i].ypos);
        lfpDisplay->channelInfo[i]->updateType();
    }
        
    lfpDisplay->rebuildDrawableChannelsList();

    isLoading = false;
        
    syncDisplay();

    isUpdating = false;

   // std::cout << " done " << std::endl;
   // std::cout << "   " << std::endl;

}

int LfpDisplaySplitter::getChannelHeight()
{
    return options->getChannelHeight();
}

void LfpDisplaySplitter::setTriggerChannel(int ch)
{
    triggerChannel = ch;
    //displayBuffer->triggerChannel = ch;
}

void LfpDisplaySplitter::setAveraging(bool avg)
{
    if (trialAveraging == false)
    {
        numTrials = -1;
    }

    trialAveraging = avg;
    
}

void LfpDisplaySplitter::resetTrials()
{
    numTrials = -1;
}

void LfpDisplaySplitter::refreshSplitterState()
{
    // called when the component's tab becomes visible again

    if (true)
    {

        //for (int i = 0; i <= displayBufferIndex.size(); i++) // include event channel
        //{
        //    displayBufferIndex.set(i, displayBuffer->displayBufferIndices[i]);
       // }

        syncDisplay();
    }

}


void LfpDisplaySplitter::refreshScreenBuffer()
{
    if (true)
    {

        screenBuffer->setSize(nChans + 1, getWidth());
        screenBufferMin->setSize(nChans + 1, getWidth());
        screenBufferMean->setSize(nChans + 1, getWidth());
        screenBufferMax->setSize(nChans + 1, getWidth());

        screenBuffer->clear();
        screenBufferMin->clear();
        screenBufferMean->clear();
        screenBufferMax->clear();
    }

}

void LfpDisplaySplitter::syncDisplay()
{
   // std::cout << "Synchronizing display " << splitID << std::endl;

    for (int channel = 0; channel <= nChans; channel++)
    {
        screenBufferIndex.set(channel, 0);

        if (displayBuffer != nullptr)
            displayBufferIndex.set(channel, displayBuffer->displayBufferIndices[channel]);
    }


}

void LfpDisplaySplitter::updateScreenBuffer()
{
    if (isVisible() && displayBuffer != nullptr && !isUpdating)
    {
        // copy new samples from the displayBuffer into the screenBuffer
        int maxSamples = lfpDisplay->getWidth() - leftmargin; // leftmargin accounts for the fact that the display doesn't start
                                                              // at the leftmost pixel

        int triggerTime = triggerChannel >=0 
                          ? processor->getLatestTriggerTime(splitID)
                          : -1;

        if (triggerTime > 0)
            processor->acknowledgeTrigger(splitID);
            
                
        for (int channel = 0; channel <= nChans; channel++) // pull one extra channel for event display
        {
            float ratio = sampleRate * timebase / float(getWidth() - leftmargin - scrollBarThickness); // samples / pixel
            // this number is crucial: converting from samples to values (in px) for the screen buffer

            //if (channel == 0)
            //    std::cout << sampleRate[channel] << std::endl;

            if (screenBufferIndex[channel] >= maxSamples) // wrap around if we reached right edge before
            {
                if (triggerChannel >= 0)
                {
                    // we may need to wait for a trigger
                    if (triggerTime >= 0)
                    {
                        

                        const int screenThird = int(maxSamples * ratio / 3);
                        const int dispBufLim = displayBufferSize / 2;

                        int t0 = triggerTime - std::min(screenThird, dispBufLim);

                        if (t0 < 0)
                        {
                            t0 += displayBufferSize;
                        }

                        displayBufferIndex.set(channel, t0); // fast-forward

                        if (channel == 0)
                        {
                            numTrials += 1;
                            std::cout << "Trial number: " << numTrials << std::endl;
                        }

                    } else {
                        return; // don't display right now
                    }
                }
                else {
                    numTrials = 0;
                }
                
                screenBufferIndex.set(channel, 0);
            }
            // hold these values locally for each channel
            int sbi = screenBufferIndex[channel];
            int dbi = displayBufferIndex[channel];

            // N samples from display buffer --> M pixels in screen buffer

            lastScreenBufferIndex.set(channel, sbi);

            int index;
            {
                ScopedLock displayLock(*displayBuffer->getMutex());
                index = displayBuffer->displayBufferIndices[channel]; // get the latest value from the display buffer
            }

            int nSamples = index - dbi; // N new samples (not pixels) to be added to displayBufferIndex

            if (nSamples < 0)
            {
                nSamples += displayBufferSize;
                //  std::cout << "nsamples 0 " ;
            }

            //if (channel == 15 || channel == 16)
            //     std::cout << channel << " " << sbi << " " << dbi << " " << nSamples << std::endl;

            int valuesNeeded = (int) float(nSamples) / ratio; // M pixels to update

            if (sbi + valuesNeeded > maxSamples)  // crop number of samples to fit canvas width (this could lead to desynchronization)
            {
                valuesNeeded = maxSamples - sbi;
            }
            float subSampleOffset = 0.0;

           /* if (channel == 0 || channel == 1 || channel == 2)
                std::cout << "Channel "
                << channel << " : "
                << sbi << " : "
                << index << " : "
                << dbi << " : "
                << valuesNeeded << " : "
                << ratio
                << std::endl;*/

            //if (channel == 2)
            //    std::cout << std::endl;

            if (valuesNeeded > 0 && valuesNeeded < 1000000)
            {

                for (int i = 0; i < valuesNeeded; i++) // also fill one extra sample for line drawing interpolation to match across draws
                {
                    //If paused don't update screen buffers, but update all indexes as needed
                    if (!lfpDisplay->isPaused)
                    {
                        float gain = 1.0;

                        
                        if (triggerChannel < 0 || numTrials == 0 || trialAveraging == false || channel == nChans)
                        {
                            screenBuffer->clear(channel, sbi, 1);
                            screenBufferMean->clear(channel, sbi, 1);
                            screenBufferMin->clear(channel, sbi, 1);
                            screenBufferMax->clear(channel, sbi, 1);
                        }
                        else {
                            screenBuffer->applyGain(channel, sbi, 1, numTrials);
                            screenBufferMean->applyGain(channel, sbi, 1, numTrials);
                            screenBufferMin->applyGain(channel, sbi, 1, numTrials);
                            screenBufferMax->applyGain(channel, sbi, 1, numTrials);
                        }
                        
                        // update continuous data channels
                        int nextpix = dbi + int(ceil(ratio));
                        if (nextpix > displayBufferSize)
                            nextpix = displayBufferSize;

                        /*if (nextpix - dbi > 1) 
                        {
                            // multiple samples, calculate average
                            float sum = 0;
                            for (int j = dbi; j < nextpix; j++)
                                sum += displayBuffer->getSample(channel, j);

                            screenBuffer->addSample(channel, sbi, sum*gain / (nextpix - dbi));
                        } 
                        else
                        {
                            // interpolate between two samples with invAlpha and alpha
                            // This is only reasonable if there are more pixels
                            // than samples. Otherwise, we should calculate average. 
                            float alpha = (float) subSampleOffset;
                            float invAlpha = 1.0f - alpha;

                            float val0 = displayBuffer->getSample(channel, dbi);
                            float val1 = displayBuffer->getSample(channel, (dbi+1)%displayBufferSize);

                            float val = invAlpha * val0  + alpha * val1;

                            screenBuffer->addSample(channel, sbi, val*gain);
                        }*/
                        

                        // same thing again, but this time add the min,mean, and max of all samples in current pixel
                        float sample_min = 10000000;
                        float sample_max = -10000000;
                        float sample_sum = 0;
                        int sampleCount = 0;

                        for (int j = dbi; j < nextpix; j++)
                        {

                            float sample_current = displayBuffer->getSample(channel, j);
                            sample_sum = sample_sum + sample_current;

                            if (sample_min > sample_current)
                            {
                                sample_min = sample_current;
                            }

                            if (sample_max < sample_current)
                            {
                                sample_max = sample_current;
                            }

                            sampleCount++;

                        }

                        screenBufferMean->addSample(channel, sbi, sample_sum / sampleCount);
                        screenBufferMin->addSample(channel, sbi, sample_min);
                        screenBufferMax->addSample(channel, sbi, sample_max);

                        // update event channel
                        if (channel == nChans)
                        {
                            //std::cout << sample_max << std::endl;
                            screenBuffer->setSample(channel, sbi, sample_max);
                        }

                        // similarly, for each pixel on the screen, we want a list of all values so we can draw a histogram later
                        // for simplicity, we'll just do this as 2d array, samplesPerPixel[px][samples]
                        // with an additional array sampleCountPerPixel[px] that holds the N samples per pixel

                        //if (channel < nChans) // we're looping over one 'extra' channel for events above, so make sure not to loop over that one here
                       // {
                            // this is for fancy drawing -- not used in new LFP Viewer
                            /*int c = 0;
                            for (int j = dbi; j < nextpix && c < MAX_N_SAMP_PER_PIXEL; j++)
                            {
                                float sample_current = displayBuffer->getSample(channel, j);
                                samplesPerPixel[channel][sbi][c] = sample_current;
                                c++;
                            }
                            if (c > 0){
                                sampleCountPerPixel.set(sbi, c - 1); // save count of samples for this pixel
                            }
                            else{
                                sampleCountPerPixel.set(sbi, 0);
                            }*/
                            //sample_mean = sample_mean / c;
                            
                     //   }

                        if (triggerChannel >= 0 && trialAveraging == true && channel != nChans)
                        {

                            screenBuffer->applyGain(channel, sbi, 1, 1 / (numTrials + 1));
                            screenBufferMean->applyGain(channel, sbi, 1, 1 / (numTrials + 1));
                            screenBufferMin->applyGain(channel, sbi, 1, 1 / (numTrials + 1));
                            screenBufferMax->applyGain(channel, sbi, 1, 1 / (numTrials + 1));
                        }

                        sbi++;
           
                    }

                    subSampleOffset += ratio;

                    int steps(floor(subSampleOffset));
                    dbi = (dbi + steps) % displayBufferSize;
                    subSampleOffset -= steps;

                }

                // update values after we're done
                screenBufferIndex.set(channel, sbi);
                displayBufferIndex.set(channel, dbi); // need to store this locally

               
            }
        }
    }

}

void LfpDisplaySplitter::setTimebase(float t)
{
    timebase = t;

    if (trialAveraging)
    {
        numTrials = -1;
    }

    syncDisplay();
}

const float LfpDisplaySplitter::getXCoord(int chan, int samp)
{
    return samp;
}

const float LfpDisplaySplitter::getYCoord(int chan, int samp)
{
    return *screenBuffer->getReadPointer(chan, samp);
}

const float LfpDisplaySplitter::getYCoordMean(int chan, int samp)
{
    return *screenBufferMean->getReadPointer(chan, samp);
}
const float LfpDisplaySplitter::getYCoordMin(int chan, int samp)
{
    return *screenBufferMin->getReadPointer(chan, samp);
}
const float LfpDisplaySplitter::getYCoordMax(int chan, int samp)
{
    return *screenBufferMax->getReadPointer(chan, samp);
}

int LfpDisplaySplitter::getNumChannels()
{
    return nChans;
}

int LfpDisplaySplitter::getNumChannelsVisible()
{
    return lfpDisplay->drawableChannels.size();
}

int LfpDisplaySplitter::getChannelSubprocessorIdx(int channel)
{
    return processor->getDataChannel(channel)->getSubProcessorIdx();
}

/*std::array<float, MAX_N_SAMP_PER_PIXEL> LfpDisplaySplitter::getSamplesPerPixel(int chan, int px)
{
    return samplesPerPixel[chan][px];
}
const int LfpDisplaySplitter::getSampleCountPerPixel(int px)
{
    return sampleCountPerPixel[px];
}*/

float LfpDisplaySplitter::getMean(int chan)
{
    float total = 0.0f;
    float numPts = 0;

    float sample = 0.0f;
    for (int samp = 0; samp < (lfpDisplay->getWidth() - leftmargin); samp += 10)
    {
        sample = *screenBuffer->getReadPointer(chan, samp);
        total += sample;
        numPts++;
    }

    //std::cout << sample << std::endl;

    return total / numPts;
}

float LfpDisplaySplitter::getStd(int chan)
{
    float std = 0.0f;

    float mean = getMean(chan);
    float numPts = 1;

    for (int samp = 0; samp < (lfpDisplay->getWidth() - leftmargin); samp += 10)
    {
        std += pow((*screenBuffer->getReadPointer(chan, samp) - mean),2);
        numPts++;
    }

    return sqrt(std / numPts);

}

bool LfpDisplaySplitter::getInputInvertedState()
{
    return options->getInputInvertedState(); //invertInputButton->getToggleState();
}

bool LfpDisplaySplitter::getDisplaySpikeRasterizerState()
{
    return options->getDisplaySpikeRasterizerState();
}

bool LfpDisplaySplitter::getDrawMethodState()
{
    
    return options->getDrawMethodState(); //drawMethodButton->getToggleState();
}

void LfpDisplaySplitter::setDrawableSampleRate(float samplerate)
{
//    std::cout << "setting the drawable sample rate in the canvas" << std::endl;
    displayedSampleRate = samplerate;
}

void LfpDisplaySplitter::setDrawableSubprocessor(uint32 sp)
{
   
    subprocessorId = sp;
    displayBuffer = processor->displayBufferMap[sp];

    updateSettings();
}

void LfpDisplaySplitter::redraw()
{
    if (!isLoading)
    {
        fullredraw = true;
        repaint();
        refresh();
    }
    
}

void LfpDisplaySplitter::paint(Graphics& g)
{
    
    //std::cout << "Painting" << std::endl;

    //g.setColour(Colour(0,0,0)); // for high-precision per-pixel density display, make background black for better visibility
    g.setColour(lfpDisplay->getColourSchemePtr()->getBackgroundColour()); //background color
    g.fillRect(0, 0, getWidth(), getHeight());

    Colour borderColour;
    ColourGradient timelineColour;

    if (isSelected)
    {
        borderColour = Colour(252, 210, 0);

        timelineColour = ColourGradient(Colour(252, 210, 0), 0, 0,
            Colour(173, 145, 3), 0, 30,
            false);
    }
    else {

        borderColour = Colour(40, 40, 40);

        timelineColour = ColourGradient(Colour(50, 50, 50), 0, 0,
            Colour(25, 25, 25), 0, 30,
            false);
    }

    g.setColour(borderColour);

    if (!canvas->makeRoomForOptions(splitID))
        g.drawRect(0, 0, getWidth(), getHeight(), 2);
    else
        g.drawRect(0, 0, getWidth(), getHeight() - 55, 2);
    
    g.setGradientFill(timelineColour);
    g.fillRect(2, 2, getWidth()-4, 28);

  //  g.setColour(Colours::black);

  //  g.drawLine(0,30,getWidth()-scrollBarThickness,30);

    /*
    g.setColour(Colour(25,25,60)); // timing grid color

    int w = getWidth()-scrollBarThickness-leftmargin;

    for (int i = 0; i < 10; i++)
    {
        if (i == 5 || i == 0)
            g.drawLine(w/10*i+leftmargin,timescale->getHeight(),w/10*i+leftmargin,getHeight()-60-timescale->getHeight(),3.0f);
        else
            g.drawLine(w/10*i+leftmargin,timescale->getHeight(),w/10*i+leftmargin,getHeight()-60-timescale->getHeight(),1.0f);
    }

    g.drawLine(0,getHeight()-60,getWidth(),getHeight()-60,3.0f);
    */

}

void LfpDisplaySplitter::refresh()
{
    if (true)
    { 
       updateScreenBuffer();

       lfpDisplay->refresh(); // redraws only the new part of the screen buffer, unless fullredraw is set to true
    }
}

void LfpDisplaySplitter::comboBoxChanged(juce::ComboBox *comboBox)
{
    if (comboBox == subprocessorSelection)
    {
        //std::cout << "Setting subprocessor for Display #"<< splitID << " to " << comboBox->getSelectedId() << std::endl;
        //uint32 subproc = processor->inputSubprocessors[cb->getSelectedId() - 1];

        //processor->setSubprocessor(subproc, splitID);
        setDrawableSubprocessor(comboBox->getSelectedId());

        select();
    }
}

void LfpDisplaySplitter::handleSpaceKeyPauseEvent()
{
    options->togglePauseButton();
}