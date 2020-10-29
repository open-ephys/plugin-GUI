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

#include "LinearButtonGroupManager.h"

#include <algorithm>

#include "../../Utils/Utils.h"


static const Colour COLOUR_DIVIDER (Colours::black.withAlpha (0.12f));
static const Colour COLOUR_BORDER  (Colour::fromRGB (189, 189, 189));
static const Colour COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));


static bool inline areEqualValues (float p1, float p2)
{
    const float epsilon = 0.001f;
    return fabs (p2 - p1) < epsilon;
}


LinearButtonGroupManager::LinearButtonGroupManager()
    : m_isShowDividers      (true)
    , m_selectedButtonIdx   (0)
    , m_desiredButtonLineX  (0.f)
    , m_currentButtonLineX  (0.f)
    , m_animationStepX      (0.f)
{
    setColour (accentColourId,      Colours::white);
    setColour (dividersColourId,    COLOUR_DIVIDER);
}


void LinearButtonGroupManager::paintOverChildren (Graphics& g)
{
    const int height = getHeight();

    // Draw dividers between buttons
    if (m_isShowDividers)
    {
        g.setColour (findColour (dividersColourId));

        const int dividerOffsetY = 5;
        const int numDividers    = m_buttons.size() - 1;

        for (int i = 0; i < numDividers; ++i)
        {
            int dividerX = m_buttons[i]->getRight();
            g.drawVerticalLine (dividerX, dividerOffsetY, height - dividerOffsetY);
        }
    }

    // Draw line which displays current selected button
    g.setColour (findColour (accentColourId));
    if (m_isRadioButtonMode)
    {
        const int lineWidth  = m_buttons[m_selectedButtonIdx]->getBounds().getWidth();
        const int lineHeight = 3;
        g.fillRect ( (int)m_currentButtonLineX, height - lineHeight - 1, lineWidth, lineHeight);
    }
}


void LinearButtonGroupManager::resized()
{
    ButtonGroupManager::resized();

    const int width     = getWidth();
    const int height    = getHeight();

    const int numButtons = m_buttons.size();
    const int buttonWidth = numButtons != 0
                                ? width / numButtons
                                : 0;

    // Set bounds for each button
    juce::Rectangle<int> buttonBounds (0, 0, buttonWidth, height);
    for (int i = 0; i < numButtons; ++i)
    {
        m_buttons[i]->setBounds (buttonBounds);

        buttonBounds.translate (buttonWidth, 0);
    }
}


void LinearButtonGroupManager::buttonClicked (Button* buttonThatWasClicked)
{
    ButtonGroupManager::buttonClicked (buttonThatWasClicked);

    if (m_isRadioButtonMode)
    {
        stopTimer();
        m_desiredButtonLineX = buttonThatWasClicked->getBounds().getX();
        m_selectedButtonIdx  = m_buttons.indexOf (static_cast<Button*> (buttonThatWasClicked));

        const float numAnimationSteps = 10.f;
        m_animationStepX = (m_desiredButtonLineX - m_currentButtonLineX) / numAnimationSteps;
LOGDD("== Calculation: Animation step: ", m_animationStepX);

        startTimer (30);
    }
}


void LinearButtonGroupManager::timerCallback()
{
    // Move line which displays current selected button
    if (! areEqualValues (m_currentButtonLineX, m_desiredButtonLineX))
    {
LOGDD("Current X: ", m_currentButtonLineX);
LOGDD("Desiredd X: ", m_desiredButtonLineX);
        m_currentButtonLineX += m_animationStepX;
    }
    else
    {
LOGDD("Timer stopped");
        m_animationStepX = 0;
        stopTimer();
    }

    repaint();
}


void LinearButtonGroupManager::setShowDividers (bool isShow)
{
    if (isShow == m_isShowDividers)
        return;

    m_isShowDividers = isShow;
    repaint();
}