/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2022 Open Ephys

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

#include "SyncControlButton.h"
#include "SyncChannelSelector.h"
#include "Synchronizer.h"

#include "../../CoreServices.h"

SyncControlButton::SyncControlButton(SynchronizingProcessor* node_,
                                     const String& name,
                                     String streamKey_,
                                     int ttlLineCount_)
    : Button(name),
      streamKey(streamKey_),
      node(node_),
      ttlLineCount(ttlLineCount_)
{

    isPrimary = node->isMainDataStream(streamKey);
    LOGD("SyncControlButton::Constructor; Stream: ", streamKey, " is main stream: ", isPrimary);
    startTimer(250);
    
    setTooltip(name);
    
}

SyncControlButton::~SyncControlButton() {}

void SyncControlButton::timerCallback()
{
    repaint();
}

/*
void SyncControlButton::componentBeingDeleted(Component &component)
{
    //Capture button channel states and send back to record node.

    auto* syncChannelSelector = (SyncChannelSelector*) component.getChildComponent(0);
    
    if (!syncChannelSelector->detectedChange)
        return;
    
    if (syncChannelSelector->isPrimary)
    {
        LOGD("Set main stream: {", streamId, "}");
        node->setMainDataStream(streamId);
        isPrimary = true;
    }

    for (int i = 0; i < syncChannelSelector->buttons.size(); i++)
    {
        if (syncChannelSelector->buttons[i]->getToggleState())
        {
            node->setSyncLine(streamId, i);
            break;
        }

    }

    repaint();
}
*/

/*
void SyncControlButton::mouseUp(const MouseEvent &event)
{

    if (!CoreServices::getRecordingStatus() && event.mods.isLeftButtonDown())
    {
        
        int syncLine = node->getSyncLine(streamId);

        SyncChannelSelector* channelSelector = new SyncChannelSelector (ttlLineCount, syncLine, node->isMainDataStream(streamId));

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);

        myBox.addComponentListener(this);
        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);

        return;

    }
}
*/

void SyncControlButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

    g.setColour(Colours::grey);
    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),4);
    
    switch(node->synchronizer.getStatus(streamKey)) {
        
        case SyncStatus::OFF :
        {
            if (isMouseOver)
            {
                //LIGHT GREY
                g.setColour(Colour(150, 150, 150));
            }
            else
            {
                //DARK GREY
                g.setColour(Colour(110, 110, 110));
            }
            break;
        }
        case SyncStatus::SYNCING :
        {

            if (isMouseOver)
            {
                //LIGHT ORANGE
               g.setColour(Colour(255,216,177));
            }
            else
            {
                //DARK ORAN
               g.setColour(Colour(255,165,0));
            }
            break;
        }
        case SyncStatus::SYNCED :
        {

            if (isMouseOver)
            {
                //LIGHT GREEN
                g.setColour(Colour(25, 255, 25));
            }
            else
            {
                //DARK GREEN
                g.setColour(Colour(25, 220, 25));
            }
            break;

        }
    }
    
    g.fillRoundedRectangle(2, 2, getWidth()-4, getHeight()-4, 2);

    if (node->isMainDataStream(streamKey))
    {
        g.setColour(Colour(255,255,255));
        g.setFont(Font(10));
        g.drawText("M", 0, 0, getWidth(), getHeight(), juce::Justification::centred);
    }
    
}

