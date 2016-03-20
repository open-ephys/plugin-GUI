/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#include "CAREditor.h"
#include "CAR.h"
#include "../../UI/LookAndFeel/MaterialButtonLookAndFeel.h"


static const Colour COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));
static const Colour COLOUR_ACCENT  (Colour::fromRGB (3, 169, 244));


CAREditor::CAREditor (GenericProcessor* parentProcessor, bool useDefaultParameterEditors)
    : GenericEditor (parentProcessor, useDefaultParameterEditors)
    , m_currentChannelsView          (REFERENCE_CHANNELS)
    , m_channelSelectorButtonManager (new ButtonGroupManager)
{
    TextButton* referenceChannelsButton = new TextButton ("Reference", "Switch to reference channels");
    referenceChannelsButton->setToggleState (true, dontSendNotification);
    referenceChannelsButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    referenceChannelsButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    referenceChannelsButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    referenceChannelsButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    TextButton* affectedChannelsButton  = new TextButton ("Affected", "Switch to affected channels");
    affectedChannelsButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    affectedChannelsButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    affectedChannelsButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    affectedChannelsButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    m_channelSelectorButtonManager->addButton (referenceChannelsButton);
    m_channelSelectorButtonManager->addButton (affectedChannelsButton);
    m_channelSelectorButtonManager->setRadioButtonMode (true);
    m_channelSelectorButtonManager->setButtonListener (this);
    m_channelSelectorButtonManager->setButtonsLookAndFeel (m_materialButtonLookAndFeel);
    m_channelSelectorButtonManager->setBackgroundColour (Colours::white);
    m_channelSelectorButtonManager->setOutlineColour    (Colour (0x0));
    m_channelSelectorButtonManager->setAccentColour     (COLOUR_ACCENT);
    addAndMakeVisible (m_channelSelectorButtonManager);

    channelSelector->paramButtonsToggledByDefault (false);
    channelSelector->addListener (this);

    setDesiredWidth (280);
}


void CAREditor::resized()
{
    m_channelSelectorButtonManager->setBounds (110, 50, 150, 36);

    GenericEditor::resized();
}


void CAREditor::buttonClicked (Button* buttonThatWasClicked)
{
    const String buttonName = buttonThatWasClicked->getName().toLowerCase();

    // "Reference channels" button clicked
    if (buttonName.startsWith ("reference"))
    {
        channelSelector->setActiveChannels (static_cast<CAR*> (getProcessor())->getReferenceChannels());

        m_currentChannelsView = REFERENCE_CHANNELS;
    }
    // "Affected channels" button clicked
    else if (buttonName.startsWith ("affected"))
    {
        channelSelector->setActiveChannels (static_cast<CAR*> (getProcessor())->getAffectedChannels());

        m_currentChannelsView = AFFECTED_CHANNELS;
    }

    GenericEditor::buttonClicked (buttonThatWasClicked);
}


void CAREditor::channelSelectionChanged (int channel, bool newState)
{
    auto processor = static_cast<CAR*> (getProcessor());
    if (m_currentChannelsView == REFERENCE_CHANNELS)
    {
        processor->setReferenceChannelState (channel, newState);
    }
    else
    {
        processor->setAffectedChannelState (channel, newState);
    }
}
