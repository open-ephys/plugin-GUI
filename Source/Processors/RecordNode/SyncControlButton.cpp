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

SyncControlButton::SyncControlButton(SynchronizingProcessor* _node,
                                     const String& name,
                                     uint16 streamId)
    : Button(name),
      streamId(streamId),
      node(_node)
{

    isPrimary = node->isMainDataStream(streamId);
    LOGD("Stream: ", streamId, " is main stream: ", isPrimary);
    startTimer(100);
}

SyncControlButton::~SyncControlButton() {}

void SyncControlButton::timerCallback()
{
    repaint();
}

void SyncControlButton::componentBeingDeleted(Component &component)
{
    /*Capture button channel states and send back to record node. */

    auto* syncChannelSelector = (SyncChannelSelector*) component.getChildComponent(0);
    
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

void SyncControlButton::mouseUp(const MouseEvent &event)
{

    if (!CoreServices::getRecordingStatus() && event.mods.isLeftButtonDown())
    {

        //const Array<EventChannel*> eventChannels = node->getDataStream(streamId)->getEventChannels();

        int nEvents;

        //if (eventChannels.size() > 0)
        //    nEvents = eventChannels[0]->getMaxTTLBits();
        //else
        //    nEvents = 1;
        
        nEvents = 8;

        int syncLine = node->getSyncLine(streamId);

        SyncChannelSelector* channelSelector = new SyncChannelSelector (nEvents, syncLine, node->isMainDataStream(streamId));

        CallOutBox& myBox
            = CallOutBox::launchAsynchronously (std::unique_ptr<Component>(channelSelector), getScreenBounds(), nullptr);

        myBox.addComponentListener(this);
        myBox.setDismissalMouseClicksAreAlwaysConsumed(true);

        return;

    }
}

void SyncControlButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),0.2*getWidth());

    g.setColour(Colour(110,110,110));
    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

    if (streamId > 0 && CoreServices::getAcquisitionStatus())
    {

        switch(node->synchronizer.getStatus(streamId)) {

            case SyncStatus::OFF :
            {

                if (isMouseOver)
                {
                    //LIGHT GREY
                    g.setColour(Colour(210, 210, 210));
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
                    g.setColour(Colour(0, 255, 0));
                }
                else
                {
                    //DARK GREEN
                    g.setColour(Colour(20, 255, 20));
                }
                break;

            }
        }

    }

    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

    if (node->isMainDataStream(streamId))
    {
        g.setColour(Colour(255,255,255));
        g.setFont(Font(10));
        g.drawText("M", 0, 0, getWidth(), getHeight(), juce::Justification::centred);
    }

}

