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

#include "TiledButtonGroupManager.h"
#include "../LookAndFeel/MaterialButtonLookAndFeel.h"

#include <algorithm>

using namespace juce;

TiledButtonGroupManager::TiledButtonGroupManager()
    : m_buttonWidth                 (10)
    , m_buttonHeight                (10)
    , m_minPaddingForButtons        (5)
    , m_firstSelectedButtonIdx      (-1)
    , m_lastSelectedButtonIdx       (-1)
    , m_isToggleOnMode              (true)
    , m_isDraggingMouseNow          (false)
    , m_isSelectButtonsByDragging   (false)
    , m_materialButtonsLookAndFeel  (new MaterialButtonLookAndFeel)
{
    setRadioButtonMode (false);
    setButtonsLookAndFeel (m_materialButtonsLookAndFeel);
}


void TiledButtonGroupManager::resized()
{
    ButtonGroupManager::resized();

    const int width = getWidth();

    if (! width)
        return;

    const int numButtonsInTheRow = jmax (1, width / (m_buttonWidth + m_minPaddingForButtons));
    const int padding = jmax (m_minPaddingForButtons,
                              (width - numButtonsInTheRow * m_buttonWidth) / jmax (numButtonsInTheRow - 1, 1));

    juce::Rectangle<int> buttonBounds (0, 0, m_buttonWidth, m_buttonHeight);
    const int numButtons = m_buttons.size();
    for (int i = 0; i < numButtons; ++i)
    {
        m_buttons[i]->setBounds (buttonBounds);

        // Go to the next row
        if ( (i + 1) % numButtonsInTheRow == 0)
        {
            buttonBounds.setX (0);
            buttonBounds.translate (0, m_buttonHeight + padding);
        }
        // Go to the next column
        else
        {
            buttonBounds.translate (m_buttonWidth + padding, 0);
        }
    }
}


void TiledButtonGroupManager::mouseDown (const MouseEvent& e)
{
}


void TiledButtonGroupManager::mouseUp (const MouseEvent& e)
{
    m_isDraggingMouseNow = false;

    m_firstSelectedButtonIdx = -1;
    m_lastSelectedButtonIdx  = -1;
}


void TiledButtonGroupManager::mouseDrag (const MouseEvent& e)
{
    if (! m_isSelectButtonsByDragging)
        return;

    m_isDraggingMouseNow = true;

    // Is shift + drag mouse action occurs, then we should toggle on buttons;
    // Otherwise - toggle off.
    m_isToggleOnMode = ! e.mods.isShiftDown();

    const int currentButtonIdx = getIndexOfButtonAtPosition (e.getEventRelativeTo (this).getPosition());

    // Remember the first button on which we started dragging
    if (m_firstSelectedButtonIdx == -1
        && m_lastSelectedButtonIdx == -1
        && currentButtonIdx >= 0)
    {
        m_firstSelectedButtonIdx = currentButtonIdx;
    }


    if (currentButtonIdx != -1
        && currentButtonIdx != m_lastSelectedButtonIdx)
    {
        m_lastSelectedButtonIdx = currentButtonIdx;

        // Get the indices of range which we should to select (get FROM index and TO index)
        const int fromIndex = jmin (m_firstSelectedButtonIdx, m_lastSelectedButtonIdx);
        const int toIndex   = jmax (m_firstSelectedButtonIdx, m_lastSelectedButtonIdx);

        for (int i = fromIndex; i <= toIndex; ++i)
        {
            // Trigger click only if the button still isn't selected/deselected (according to the SHIFT holds)
            if (m_buttons[i]->getToggleState() != m_isToggleOnMode)
                // Trigger click in the button, this will call buttonClicked() method, where we will
                m_buttons[i]->triggerClick();
        }
    }
}


void TiledButtonGroupManager::buttonClicked (Button* buttonThatWasClicked)
{
    // Fast selection enabled and we dragging mouse now to select
    if (m_isSelectButtonsByDragging
        && m_isDraggingMouseNow)
    {
        buttonThatWasClicked->setToggleState (m_isToggleOnMode, dontSendNotification);
    }
    // Default selection
    else
    {
        // Do nothing for now, it's toggled on/off by default
        //buttonThatWasClicked->setToggleState (! buttonThatWasClicked->getToggleState(), dontSendNotification);
    }

    // Notify the listener
    ButtonGroupManager::buttonClicked (buttonThatWasClicked);
}


void TiledButtonGroupManager::addButton (Button* newButton, bool useDefaultLookAndFeel)
{
    ButtonGroupManager::addButton (newButton, useDefaultLookAndFeel);

    // Disable default clicking on button
    newButton->addMouseListener (this, false);
}


int TiledButtonGroupManager::getIndexOfButtonAtPosition (juce::Point<int> position) const
{

    const int numButtons = m_buttons.size();
    const int viewPositionX = m_buttonsViewport.getViewPositionX();
    const int viewPositionY = m_buttonsViewport.getViewPositionY();

    //DBG ("Down Y position: " + String (position.translated (viewPositionX, viewPositionY).getY()));

    for (int i = 0; i < numButtons; ++i)
    {
        if (m_buttons[i]->getBounds().contains (position.translated (viewPositionX, viewPositionY)))
            return i;
    }

    return -1;
}


bool TiledButtonGroupManager::isFastSelectionModeEnabled() const
{
    return m_isSelectButtonsByDragging;
}


void TiledButtonGroupManager::setFastSelectionModeEnabled (bool isFastSelectionMode)
{
    m_isSelectButtonsByDragging = isFastSelectionMode;
}


void TiledButtonGroupManager::setButtonSize (int buttonWidth, int buttonHeight)
{
    m_buttonWidth  = buttonWidth;
    m_buttonHeight = buttonHeight;

    std::for_each (m_buttons.begin(), m_buttons.end(),
                   [=] (Button* button) { button->setSize (buttonWidth, buttonHeight); });

    resized();
}


void TiledButtonGroupManager::setMinPaddingBetweenButtons (int minPadding)
{
    m_minPaddingForButtons = minPadding;

    resized();
}
