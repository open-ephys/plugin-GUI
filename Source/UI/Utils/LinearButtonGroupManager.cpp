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
#include "../../UI/LookAndFeel/CustomLookAndFeel.h"


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

}

void LinearButtonGroupManager::setSelectedButtonIndex(int index)
{
    m_selectedButtonIdx = index;
    m_currentButtonLineX = m_buttons[index]->getBounds().getX();
    
    repaint();
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
        //LOGDD("== Calculation: Animation step: ", m_animationStepX);

        startTimer (30);
    }
}


void LinearButtonGroupManager::addButton (Button* newButton, bool useDefaultLookAndFeel)
{
    newButton->addListener (this);

    if (useDefaultLookAndFeel)
        newButton->setLookAndFeel (m_buttonsLookAndFeel);

    m_componentProxyHandler->addAndMakeVisible (newButton);

    if (m_buttons.size() >= 1)
    {    
        if (m_buttons.getLast()->isConnectedOnLeft())
            m_buttons.getLast()->setConnectedEdges(Button::ConnectedOnRight | Button::ConnectedOnLeft);
        else
            m_buttons.getLast()->setConnectedEdges(Button::ConnectedOnRight);

        newButton->setConnectedEdges(Button::ConnectedOnLeft);
    }

    m_buttons.add (newButton);

    if (m_isRadioButtonMode)
    {
        newButton->setRadioGroupId (1);
    }

    resized();
}


void LinearButtonGroupManager::timerCallback()
{
    // Move line which displays current selected button
    if (! areEqualValues (m_currentButtonLineX, m_desiredButtonLineX))
    {
        //LOGDD("Current X: ", m_currentButtonLineX);
        //LOGDD("Desiredd X: ", m_desiredButtonLineX);
        m_currentButtonLineX += m_animationStepX;
    }
    else
    {
        //LOGDD("Timer stopped");
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
