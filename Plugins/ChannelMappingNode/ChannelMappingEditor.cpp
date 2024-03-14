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
    : GenericEditor(parentNode),
      reorderActive(true),
      isDragging(false)

{
    desiredWidth = 335;
    
    electrodeButtonViewport = std::make_unique<Viewport>();

    addAndMakeVisible(electrodeButtonViewport.get());
    electrodeButtonViewport->setBounds(10,30,325,90);
    electrodeButtonViewport->setScrollBarsShown(true,false,true,true);
    electrodeButtonHolder = std::make_unique<Component>();
    electrodeButtonViewport->setViewedComponent(electrodeButtonHolder.get(), false);

    loadButton = std::make_unique<LoadButton>(getNameAndId() + " Load Prb File");
    loadButton->addListener(this);
    loadButton->setBounds(325,4,15,15);
    addAndMakeVisible(loadButton.get());

    saveButton = std::make_unique<SaveButton>(getNameAndId() + " Save Prb File");
    saveButton->addListener(this);
    saveButton->setBounds(305,4,15,15);
    addAndMakeVisible(saveButton.get());

}

ChannelMappingEditor::~ChannelMappingEditor()
{

}

void ChannelMappingEditor::updateSettings()
{
    refreshElectrodeButtons();
}

void ChannelMappingEditor::selectedStreamHasChanged()
{
    refreshElectrodeButtons();
}

void ChannelMappingEditor::refreshElectrodeButtons()
{
    ChannelMappingNode* processor = (ChannelMappingNode*) getProcessor();

    electrodeButtons.clear();

    uint16 streamId = getCurrentStream();

    if (streamId == 0) // no inputs available
        return;

    Array<int> buttonOrder = processor->getChannelOrder(getCurrentStream());
    Array<bool> buttonState = processor->getChannelEnabledState(getCurrentStream());

    electrodeButtonViewport->setVisible(!getCollapsedState());

    int width = 19;
    int height = 15;
    int row = 0;
    int column = 0;
    int totalWidth = 0;
    int totalHeight = 0;
    
    const int totalButtons = buttonOrder.size();

    for (int i = 0; i < totalButtons; i++)
    {
        
        float hue = float(buttonOrder[i] % 16) / 16.0f * 0.2f + 0.05f;
        
        Colour buttonColour = Colour(hue, 0.90f, 0.90f, 1.0f);
        ElectrodeButton* button = new ElectrodeButton(buttonOrder[i]+1, buttonColour);
        button->setRadioGroupId(0);
        button->setClickingTogglesState(false);
        button->setToggleState(true, dontSendNotification);
        electrodeButtonHolder->addAndMakeVisible(button);
        button->addMouseListener(this, true);
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

}

void ChannelMappingEditor::collapsedStateChanged()
{
    refreshElectrodeButtons();
}


void ChannelMappingEditor::startAcquisition()
{
    reorderActive = false;

    //for (auto button : electrodeButtons)
    //{
    //    button->setEnabled(false);
    //}
}


void ChannelMappingEditor::stopAcquisition()
{
    reorderActive = true;

    //for (auto button : electrodeButtons)
    //{
    //    button->setEnabled(true);
    //}
}



void ChannelMappingEditor::buttonClicked(Button* button)
{

    if (button == saveButton.get())
    {

        if (!acquisitionIsActive)
        {
            FileChooser fc("Choose the file name...",
                               CoreServices::getDefaultUserSaveDirectory(),
                               "*",
                               true);

            if (fc.browseForFileToSave(true))
            {
                File fileToSave = fc.getResult();
                writePrbFile(fileToSave);
            }
        } else {
			CoreServices::sendStatusMessage("Stop acquisition before saving the channel map.");
        }

        

    } else if (button == loadButton.get())
    {

        if (!acquisitionIsActive)
        {
            FileChooser fc("Choose a file to load...",
                               CoreServices::getDefaultUserSaveDirectory(),
                               "*",
                               true);

            if (fc.browseForFileToOpen())
            {
                File fileToOpen = fc.getResult();
				loadPrbFile(fileToOpen);
            }
        } else {
			CoreServices::sendStatusMessage("Stop acquisition before saving the channel map.");
        }
    }
}

void ChannelMappingEditor::writePrbFile(File& file)
{
    ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();

    CoreServices::sendStatusMessage(processor->writeStreamSettings(getCurrentStream(), file));
}

void ChannelMappingEditor::loadPrbFile(File& file)
{
    ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();

    CoreServices::sendStatusMessage(processor->loadStreamSettings(getCurrentStream(), file));
    
    CoreServices::updateSignalChain(this);
}


void ChannelMappingEditor::mouseDown(const MouseEvent& e)
{

    if (e.mods.isRightButtonDown() && !acquisitionIsActive)
    {

        if (electrodeButtons.contains((ElectrodeButton*) e.originalComponent))
        {
            ElectrodeButton* button = (ElectrodeButton*) e.originalComponent;
            
            PopupMenu menu;

            int buttonIndex = electrodeButtons.indexOf(button);

            ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();
            Array<bool> buttonStates = processor->getChannelEnabledState(getCurrentStream());

            bool isActive = buttonStates[buttonIndex];

            String displayString;

            if (isActive)
            {
                displayString = "Disable Input Channel "; 
            }
            else {
                displayString = "Enable Input Channel ";
            }

            displayString += String(button->getChannelNum());

            menu.addItem(2,     // index
                "Reset stream settings",  // message
                true);          // isSelectable

            menu.addItem(1,     // index
                displayString,  // message
                true);          // isSelectable

            const int result = menu.show(); // returns 0 if nothing is selected
            
            PopupMenu::dismissAllActiveMenus();
            
            if (result == 1)
            {
                processor->setChannelEnabled(getCurrentStream(), buttonIndex, !isActive);
                CoreServices::updateSignalChain(this);
            }
            else if (result == 2)
            {
                bool response = AlertWindow::showOkCancelBox(AlertWindow::NoIcon,
                    "Reset Channel Map",
                    "Are you sure you want to reset the channel map for the current stream?",
                    "OK", "Cancel", 0, 0);

                if (response)
                {
                    processor->resetStream(getCurrentStream());
                    CoreServices::updateSignalChain(this);
                }
                
            }
            
        }

        
    }
}

void ChannelMappingEditor::mouseDrag(const MouseEvent& e)
{

    if (reorderActive)
    {
        if (!isDragging) 
        {
            if (electrodeButtons.contains((ElectrodeButton*) e.originalComponent))
            {
                ElectrodeButton* button = (ElectrodeButton*)e.originalComponent;
                isDragging = true;

                String desc = "EditorDrag/MAP/";
                desc += button->getChannelNum();

                const String dragDescription = desc;

                Image dragImage(Image::ARGB, 20, 15, true);

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
                g.drawText(String(button->getChannelNum()), 0, 0, 20, 15, Justification::centred, true);

                dragImage.multiplyAllAlphas(0.6f);

                startDragging(dragDescription, this, dragImage, false);
                button->setVisible(false);
                initialDraggedButton = electrodeButtons.indexOf(button);
                lastHoverButton = initialDraggedButton;
                draggingChannel = button->getChannelNum();
            }
            
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

        Array<int> order;

        for (auto button : electrodeButtons)
        {
            order.add(button->getChannelNum() - 1);
        }

        ChannelMappingNode* processor = (ChannelMappingNode*)getProcessor();

        processor->setChannelOrder(getCurrentStream(), order);

        CoreServices::updateSignalChain(this);
        
        /*int from, to;
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
        }*/

        //for (int i=from; i <= to; i++)
        //{
        //    setChannelPosition(i,electrodeButtons[i]->getChannelNum());
        //}
       // setConfigured(true);
		//CoreServices::updateSignalChain(this);
    }
}




