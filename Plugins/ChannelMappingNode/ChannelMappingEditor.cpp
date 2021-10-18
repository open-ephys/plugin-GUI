/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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


#include "ChannelMappingEditor.h"
#include "ChannelMappingNode.h"
#include <stdio.h>


ChannelMappingEditor::ChannelMappingEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode)

{
    desiredWidth = 350;

    selectAllButton = new ElectrodeEditorButton("Select All");
    selectAllButton->addListener(this);
    addAndMakeVisible(selectAllButton);
    selectAllButton->setBounds(125,110,110,10);
    selectAllButton->setToggleState(false, dontSendNotification);

    modifyButton = new ElectrodeEditorButton("Remap");
    modifyButton->addListener(this);
    addAndMakeVisible(modifyButton);
    modifyButton->setBounds(220,110,60,10);
    modifyButton->setToggleState(false, dontSendNotification);
    modifyButton->setClickingTogglesState(true);

    resetButton = new ElectrodeEditorButton("Reset");
    resetButton->addListener(this);
    addAndMakeVisible(resetButton);
    resetButton->setBounds(285,110,60,10);
    resetButton->setToggleState(true, dontSendNotification);
    resetButton->setClickingTogglesState(false);
    resetButton->setEnabled(false);
    
    addAndMakeVisible(electrodeButtonViewport = new Viewport());
    electrodeButtonViewport->setBounds(10,30,330,70);
    electrodeButtonViewport->setScrollBarsShown(true,false,true,true);
    electrodeButtonHolder = new Component();
    electrodeButtonViewport->setViewedComponent(electrodeButtonHolder,false);

    loadButton = new LoadButton();
    loadButton->addListener(this);
    loadButton->setBounds(325,5,15,15);
    addAndMakeVisible(loadButton);

    saveButton = new SaveButton();
    saveButton->addListener(this);
    saveButton->setBounds(305,5,15,15);
    addAndMakeVisible(saveButton);

    for (int i = 0 ; i < NUM_REFERENCES; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(i+1);
        referenceButtons.add(button);

        button->setBounds(10+i*30,110,20,15);
        button->setToggleState(false, dontSendNotification);
        button->setRadioGroupId(900);

        addAndMakeVisible(button);
        button->addListener(this);
        button->setButtonText(String::charToString('A'+i));

    }

    reorderActive = false;
    isDragging = false;

}

ChannelMappingEditor::~ChannelMappingEditor()
{

}

void ChannelMappingEditor::updateSettings()
{

    refreshButtonLocations();

    /*if (isConfigured)
    {
        if (getProcessor()->getNumInputs() > previousChannelCount)
        {
            createElectrodeButtons(getProcessor()->getNumInputs(),false);
            previousChannelCount = getProcessor()->getNumInputs();
        }
        if (!reorderActive)
        {
            checkUnusedChannels();
        }

    }
    else if (getProcessor()->getNumInputs() != previousChannelCount)
    {
        createElectrodeButtons(getProcessor()->getNumInputs());
        previousChannelCount = getProcessor()->getNumInputs();
    }
	channelCountArray.clearQuick();
	int size = channelArray.size();
	for (int i = 0; i < size; i++)
	{
		if (enabledChannelArray[channelArray[i]-1])
			channelCountArray.add(channelArray[i]-1);
	}*/
}

void ChannelMappingEditor::createElectrodeButtons(int numNeeded, bool clearPrevious)
{
   /* int startButton;

    if (clearPrevious)
    {
        electrodeButtons.clear();

        //referenceArray.clear();
        //channelArray.clear();
        //referenceChannels.clear();
        //enabledChannelArray.clear();

        startButton = 0;
        previousClickedChan = -1;
        previousShiftClickedChan = -1;
        referenceButtons[0]->setToggleState(true, dontSendNotification);
        selectedReference = 0;
        modifyButton->setToggleState(false, dontSendNotification);
        reorderActive = false;
    }
    else
    {
        startButton = electrodeButtons.size();
        if (startButton > numNeeded) return;
    }

    for (int i = startButton; i < numNeeded; i++)
    {
        ElectrodeButton* button = new ElectrodeButton(i+1);
        electrodeButtons.add(button);

        button->setRadioGroupId(0);

        electrodeButtonHolder->addAndMakeVisible(button); // determine visibility in refreshButtonLocations()

        button->addListener(this);

        if (reorderActive)
        {
            button->setToggleState(true,dontSendNotification);
            electrodeButtons[i]->setClickingTogglesState(false);
            electrodeButtons[i]->addMouseListener(this,false);
        }
        else
        {
            button->setToggleState(false,dontSendNotification);
            electrodeButtons[i]->setClickingTogglesState(true);
        }

        //referenceArray.add(-1);

        //getProcessor()->setCurrentChannel(i);
        //getProcessor()->setParameter(0,i); // set channel mapping to standard channel
        //getProcessor()->setParameter(1,-1); // set reference to none
       // getProcessor()->setParameter(3,1); //enable channel

        //channelArray.add(i+1);
        //enabledChannelArray.add(true);

    }

    if (clearPrevious)
    {
        getProcessor()->setCurrentChannel(-1);
        for (int i = 0; i < NUM_REFERENCES; i++)
        {

            getProcessor()->setParameter(2,i); //Clear reference
            referenceChannels.add(-1);
            referenceButtons[i]->setEnabled(true);
        }
    }
    channelSelector->setRadioStatus(true);

    refreshButtonLocations();*/
}

void ChannelMappingEditor::refreshButtonLocations()
{
    ChannelMappingNode* processor = (ChannelMappingNode*) getProcessor();

    electrodeButtons.clear();

    Array<int> buttonOrder = processor->getChannelOrder(getCurrentStream());
    Array<bool> buttonState = processor->getChannelEnabledState(getCurrentStream());

    electrodeButtonViewport->setVisible(!getCollapsedState());

    int width = 19;
    int height = 15;
    int row = 0;
    int column = 0;
    int totalWidth = 0;
    int totalHeight = 0;

    for (int i = 0; i < buttonOrder.size(); i++)
    {
        ElectrodeButton* button = new ElectrodeButton(buttonOrder[i]+1);
        button->setRadioGroupId(0);
        electrodeButtonHolder->addAndMakeVisible(button);
        button->addListener(this);
        button->setBounds(column*width, row*height, width, height);
        totalWidth =  jmax(totalWidth, ++column*width);
        
        if (column == 1)
        {
            totalHeight = jmax(totalHeight, (row + 1)*height);
        }
        else if (column == 16)
        {
            row++;
            column = 0;            
        }

        button->setEnabled(buttonState[i]);

        electrodeButtons.add(button);
    }

    electrodeButtonHolder->setSize(totalWidth,totalHeight);

    newReferenceSelected();
}

void ChannelMappingEditor::collapsedStateChanged()
{
    refreshButtonLocations();
}

void ChannelMappingEditor::newReferenceSelected()
{
    ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();

    Array<int> channelsForReference = processor->getChannelsForReference(getCurrentStream(), selectedReferenceIndex);

    for (int i = 0; i < electrodeButtons.size(); i++)
    {
        electrodeButtons[i]->setToggleState(channelsForReference.contains(i), dontSendNotification);
    }
}

void ChannelMappingEditor::buttonEvent(Button* button)
{

    ChannelMappingNode* processor = (ChannelMappingNode*) getProcessor();

    if (button == selectAllButton)
    {
        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            electrodeButtons[i]->setToggleState(true, dontSendNotification);
            processor->setReferenceIndex(getCurrentStream(), i, selectedReferenceIndex);
        }
        previousClickedChan = -1;

        //setConfigured(true);
    }
    else if (button == resetButton)
    {
		if (acquisitionIsActive)
		{
			CoreServices::sendStatusMessage("Cannot change channel order while acquiring");
			return;
		}

        //processor->resetStream(getCurrentStream());

		CoreServices::updateSignalChain(this);
    }
    else if (button == modifyButton)
    {
		if (acquisitionIsActive)
		{
			CoreServices::sendStatusMessage("Cannot change channel order while acquiring");
			button->setToggleState(false,dontSendNotification);
			return;
		}
        if (reorderActive)
        {

            reorderActive = false;

            Array<int> newOrder;
            Array<bool> enabledState;

            for (int i = 0; i < electrodeButtons.size(); i++)
            {
                newOrder.add(electrodeButtons[i]->getChannelNum() - 1);
                processor->setChannelEnabled(getCurrentStream(), i, electrodeButtons[i]->isEnabled());
            }

            processor->setChannelOrder(getCurrentStream(), newOrder);
            

            for (int i = 0; i < referenceButtons.size(); i++)
            {
                referenceButtons[i]->setEnabled(true);
            }
            referenceButtons[0]->setToggleState(true, dontSendNotification);
            selectAllButton->setEnabled(true);

            newReferenceSelected();

            CoreServices::updateSignalChain(this);
            
        }
        else
        {
            reorderActive = true;

            for (int i = 0; i < referenceButtons.size(); i++)
            {
                referenceButtons[i]->setEnabled(false);
                referenceButtons[i]->setToggleState(false, dontSendNotification);
            }

            Array<bool> isEnabled = processor->getChannelEnabledState(getCurrentStream());

            for (int i = 0; i < electrodeButtons.size(); i++)
            {
                electrodeButtons[i]->setClickingTogglesState(false);
                electrodeButtons[i]->setEnabled(true);

                if (isEnabled[electrodeButtons[i]->getChannelNum()-1])
                {
                    electrodeButtons[i]->setToggleState(true, dontSendNotification);
                }
                else
                {
                    electrodeButtons[i]->setToggleState(false, dontSendNotification);
                }
                electrodeButtons[i]->addMouseListener(this,false);

            }
            selectAllButton->setEnabled(false);
        }
    }
    else if (referenceButtons.contains((ElectrodeButton*)button))
    {
        selectedReferenceIndex = referenceButtons.indexOf((ElectrodeButton*)button);

        newReferenceSelected();

        int referenceChannel = processor->getReferenceChannel(getCurrentStream(), selectedReferenceIndex);

        std::vector<bool> channelStates;

        for (int i = 0; i < electrodeButtons.size(); i++)
        {
            if (i == referenceChannel)
                channelStates.push_back(true);
            else
                channelStates.push_back(false);
        }

        auto* channelSelector = new PopupChannelSelector(this, channelStates);

        channelSelector->setChannelButtonColour(Colour(0, 174, 239));
        channelSelector->setMaximumSelectableChannels(1);

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously(std::unique_ptr<Component>(channelSelector),
                button->getScreenBounds(),
                nullptr);

        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);

    }
    else if (electrodeButtons.contains((ElectrodeButton*)button))
    {
        if (!reorderActive)
        {
            int clickedChan = electrodeButtons.indexOf((ElectrodeButton*)button);

            if (ModifierKeys::getCurrentModifiers().isShiftDown() && (previousClickedChan >= 0))
            {
                int toChanA = 0;
                int toChanD = 0;
                int fromChanA = 0;
                int fromChanD = 0;

                if (previousShiftClickedChan < 0)
                {
                    previousShiftClickedChan = clickedChan;
                    if (clickedChan > previousClickedChan)
                    {
                        toChanA = clickedChan;
                        fromChanA = previousClickedChan;
                    }
                    else
                    {
                        toChanA = previousClickedChan;
                        fromChanA = clickedChan;
                    }
                    for (int i = fromChanA; i <= toChanA; i++)
                    {
                        electrodeButtons[i]->setToggleState(previousClickedState, dontSendNotification);
                        processor->setReferenceIndex(getCurrentStream(), i, selectedReferenceIndex);
                    }
                }
                else
                {
                    if ((clickedChan > previousClickedChan) && (clickedChan > previousShiftClickedChan))
                    {
                        fromChanA = previousShiftClickedChan+1;
                        toChanA = clickedChan;
                        if (previousShiftClickedChan < previousClickedChan)
                        {
                            fromChanD = previousShiftClickedChan;
                            toChanD = previousClickedChan-1;
                        }
                        else
                        {
                            fromChanD = -1;
                        }
                    }
                    else if ((clickedChan > previousClickedChan) && (clickedChan < previousShiftClickedChan))
                    {
                        fromChanA = -1;
                        fromChanD = clickedChan+1;
                        toChanD = previousShiftClickedChan;
                        button->setToggleState(previousClickedState, dontSendNotification); // Do not toggle this button;
                    }
                    else if ((clickedChan < previousClickedChan) && (clickedChan < previousShiftClickedChan))
                    {
                        fromChanA = clickedChan;
                        toChanA = previousShiftClickedChan-1;
                        if (previousShiftClickedChan > previousClickedChan)
                        {
                            fromChanD = previousClickedChan+1;
                            toChanD = previousShiftClickedChan;
                        }
                        else
                        {
                            fromChanD = -1;
                        }
                    }
                    else if ((clickedChan < previousClickedChan) && (clickedChan > previousShiftClickedChan))
                    {
                        fromChanA = -1;
                        fromChanD = previousShiftClickedChan;
                        toChanD = clickedChan - 1;
                        button->setToggleState(previousClickedState, dontSendNotification); // Do not toggle this button;
                    }
                    else if (clickedChan == previousShiftClickedChan)
                    {
                        fromChanA = -1;
                        fromChanD = -1;
                        button->setToggleState(previousClickedState, dontSendNotification); // Do not toggle this button;
                    }
                    else
                    {
                        fromChanA = -1;
                        button->setToggleState(previousClickedState, dontSendNotification); // Do not toggle this button;
                        if (previousShiftClickedChan < previousClickedChan)
                        {
                            fromChanD = previousShiftClickedChan;
                            toChanD = previousClickedChan - 1;
                        }
                        else if (previousShiftClickedChan > previousClickedChan)
                        {
                            fromChanD = previousClickedChan + 1;
                            toChanD = previousShiftClickedChan;
                        }
                        else
                        {
                            fromChanD = -1;
                        }
                    }

                    if (fromChanA >= 0)
                    {
                        for (int i = fromChanA; i <= toChanA; i++)
                        {
                            electrodeButtons[i]->setToggleState(previousClickedState, dontSendNotification);
                            processor->setReferenceIndex(getCurrentStream(), i, selectedReferenceIndex);
                        }
                    }
                    if (fromChanD >= 0)
                    {
                        for (int i = fromChanD; i <= toChanD; i++)
                        {
                            electrodeButtons[i]->setToggleState(!previousClickedState, dontSendNotification);
                            processor->setReferenceIndex(getCurrentStream(), i, selectedReferenceIndex);
                        }
                    }
                }

                previousShiftClickedChan = clickedChan;
            }
            else
            {
                previousClickedChan = clickedChan;
                previousShiftClickedChan = -1;
                processor->setReferenceIndex(getCurrentStream(), clickedChan, selectedReferenceIndex);
                previousClickedState = button->getToggleState();
            }


        }
    } else if (button == saveButton)
    {
        //std::cout << "Save button clicked." << std::endl;

        if (!acquisitionIsActive)
        {
            FileChooser fc("Choose the file name...",
                               CoreServices::getDefaultUserSaveDirectory(),
                               "*",
                               true);

            if (fc.browseForFileToSave(true))
            {
                File fileToSave = fc.getResult();
               //std::cout << fileToSave.getFileName() << std::endl;
               // CoreServices::sendStatusMessage(writePrbFile(fileToSave));
            }
        } else {
			CoreServices::sendStatusMessage("Stop acquisition before saving the channel map.");
        }

        

    } else if (button == loadButton)
    {
        //std::cout << "Load button clicked." << std::endl;

        if (!acquisitionIsActive)
        {
            FileChooser fc("Choose a file to load...",
                               CoreServices::getDefaultUserSaveDirectory(),
                               "*",
                               true);

            if (fc.browseForFileToOpen())
            {
                if (reorderActive)
                    modifyButton->setToggleState(false,sendNotificationSync);
                File fileToOpen = fc.getResult();
                //std::cout << fileToOpen.getFileName() << std::endl;
				//CoreServices::sendStatusMessage(loadPrbFile(fileToOpen));
            }
        } else {
			CoreServices::sendStatusMessage("Stop acquisition before saving the channel map.");
        }
    }
}

void ChannelMappingEditor::channelStateChanged(Array<int> selectedChannels)
{

    int referenceChannel = selectedChannels.size() > 0 ? selectedChannels[0] : -1;

    ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();

    processor->setReferenceChannel(getCurrentStream(), selectedReferenceIndex, referenceChannel);

}

void ChannelMappingEditor::mouseDrag(const MouseEvent& e)
{
    if (reorderActive)
    {
        if ((!isDragging) && (electrodeButtons.contains((ElectrodeButton*) e.originalComponent)))
        {
            ElectrodeButton* button = (ElectrodeButton*)e.originalComponent;
            isDragging = true;

            String desc = "EditorDrag/MAP/";
            desc += button->getChannelNum();

            const String dragDescription = desc;

            Image dragImage(Image::ARGB,20,15,true);

            Graphics g(dragImage);
            if (button->getToggleState())
            {
                g.setColour(Colours::orange);
            }
            else
            {
                g.setColour(Colours::darkgrey);
            }
            g.fillAll();
            g.setColour(Colours::black);
            g.drawText(String(button->getChannelNum()),0,0,20,15,Justification::centred,true);

            dragImage.multiplyAllAlphas(0.6f);

            startDragging(dragDescription,this,dragImage,false);
            button->setVisible(false);
            initialDraggedButton = electrodeButtons.indexOf(button);
            lastHoverButton = initialDraggedButton;
            draggingChannel = button->getChannelNum();
        }
        else if (isDragging)
        {
            MouseEvent ev = e.getEventRelativeTo(this);
            int mouseDownY = ev.getMouseDownY()-30;
            int mouseDownX = ev.getMouseDownX()-10;
            juce::Point<int> viewPosition =electrodeButtonViewport->getViewPosition();
            
            int distanceY = ev.getDistanceFromDragStartY();
            int distanceX = ev.getDistanceFromDragStartX();
            
            int newPosY = viewPosition.getY()+ mouseDownY + distanceY;
            int newPosX = viewPosition.getX()+ mouseDownX + distanceX;
            if ( mouseDownY + distanceY > 70){
                electrodeButtonViewport->setViewPosition(viewPosition.getX(),newPosY);
            }else if( mouseDownY + distanceY < 0 ){
                electrodeButtonViewport->setViewPosition(viewPosition.getX(),newPosY);
            }
            
            
            int col = (newPosX / 19);
            if (col < 0) col = 0;
            else if (col > 16) col = 16;
            int row = (newPosY / 15);
            if (row < 0) row = 0;

            int hoverButton = row*16+col;

            if (hoverButton >= electrodeButtons.size())
            {
                hoverButton = electrodeButtons.size() -1;
            }

            if (hoverButton != lastHoverButton)
            {

                electrodeButtons[lastHoverButton]->setVisible(true);
                electrodeButtons[hoverButton]->setVisible(false);

                if (lastHoverButton > hoverButton)
                {
                    for (int i = lastHoverButton; i > hoverButton; i--)
                    {
                        electrodeButtons[i]->setChannelNum(electrodeButtons[i-1]->getChannelNum());
                        
                        if (electrodeButtons[i]->isEnabled())
                        {
                            electrodeButtons[i]->setToggleState(true, dontSendNotification);
                        }
                        else
                        {
                            electrodeButtons[i]->setToggleState(false, dontSendNotification);
                        }
                    }
                }
                else
                {
                    for (int i = lastHoverButton; i < hoverButton; i++)
                    {
                        electrodeButtons[i]->setChannelNum(electrodeButtons[i+1]->getChannelNum());
                        if (electrodeButtons[i]->isEnabled())
                        {
                            electrodeButtons[i]->setToggleState(true, dontSendNotification);
                        }
                        else
                        {
                            electrodeButtons[i]->setToggleState(false, dontSendNotification);
                        }
                    }
                }
                electrodeButtons[hoverButton]->setChannelNum(draggingChannel);
                electrodeButtons[hoverButton]->setToggleState(electrodeButtons[draggingChannel - 1]->isEnabled(), dontSendNotification);

                lastHoverButton = hoverButton;
                repaint();
            }

        }

    }
}

void ChannelMappingEditor::mouseUp(const MouseEvent& e)
{
    if (isDragging)
    {
        isDragging = false;
        electrodeButtons[lastHoverButton]->setVisible(true);
        int from, to;
        if (lastHoverButton == initialDraggedButton)
        {
            return;
        }
        else if (lastHoverButton < initialDraggedButton)
        {
            from = lastHoverButton;
            to = initialDraggedButton;
        }
        else
        {
            from = initialDraggedButton;
            to = lastHoverButton;
        }

        //for (int i=from; i <= to; i++)
        //{
        //    setChannelPosition(i,electrodeButtons[i]->getChannelNum());
        //}
       // setConfigured(true);
		//CoreServices::updateSignalChain(this);
    }
}

void ChannelMappingEditor::setChannelPosition(int position, int channel)
{
    //getProcessor()->setCurrentChannel(position);
    //getProcessor()->setParameter(0,channel-1);
    //channelArray.set(position,channel);
}

void ChannelMappingEditor::mouseDoubleClick(const MouseEvent& e)
{
    if ((reorderActive) && electrodeButtons.contains((ElectrodeButton*)e.originalComponent))
    {
        setConfigured(true);
        ElectrodeButton* button = (ElectrodeButton*)e.originalComponent;
        if (button->getToggleState())
        {
            button->setToggleState(false, dontSendNotification);
            button->setEnabled(false);
            getProcessor()->setCurrentChannel(button->getChannelNum()-1);
            getProcessor()->setParameter(3,0);
        }
        else
        {
            button->setToggleState(true, dontSendNotification);
            button->setEnabled(true);
            getProcessor()->setCurrentChannel(button->getChannelNum()-1);
            getProcessor()->setParameter(3,1);
        }
		CoreServices::updateSignalChain(this);
    }
}

void ChannelMappingEditor::checkUnusedChannels()
{
    /*for (int i = 0; i < electrodeButtons.size(); i++)
    {
        if (electrodeButtons[i]->getChannelNum() > getProcessor()->getNumInputs())
        {
            electrodeButtons[i]->setEnabled(false);
        }
        else
        {
            if (enabledChannelArray[electrodeButtons[i]->getChannelNum()-1])
            {
                electrodeButtons[i]->setEnabled(true);
            }
            else
            {
                electrodeButtons[i]->setEnabled(false);
            }
        }
    }*/
}
void ChannelMappingEditor::setConfigured(bool state)
{
    //isConfigured = state;
    //resetButton->setEnabled(state);
   // resetButton->setToggleState(!state, dontSendNotification);
    //getProcessor()->setParameter(4,state?1:0);
}

void ChannelMappingEditor::startAcquisition()
{
	if (reorderActive)
		modifyButton->setToggleState(false, sendNotificationSync);
}

//int ChannelMappingEditor::getChannelDisplayNumber(int chan) const
//{
	//if (channelCountArray.size() > chan)
	//{
	//	return channelCountArray[chan];
	//}
	//else
	//	return chan;
//}

