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

#include "ButtonGroupManager.h"

#include <algorithm>


static const Colour COLOUR_DIVIDER (Colours::black.withAlpha (0.12f));
static const Colour COLOUR_BORDER  (Colour::fromRGB (189, 189, 189));
static const Colour COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));


static bool inline areEqualValues (float p1, float p2)
{
    const float epsilon = 0.001f;
    return fabs (p2 - p1) < epsilon;
}


ButtonGroupManager::ButtonGroupManager()
    : m_isRadioButtonMode   (true)
    , m_isShowDividers      (true)
    , m_selectedButtonIdx   (0)
    , m_currentButtonLineX  (0.f)
    , m_desiredButtonLineX  (0.f)
    , m_animationStepX      (0.f)
    , m_buttonListener      (nullptr)
    , m_backgroundColour    (Colour (0x0))
    , m_accentColour        (Colours::white)
    , m_outlineColour       (COLOUR_BORDER)
    , m_buttonsLookAndFeel  (nullptr)
{
}


void ButtonGroupManager::paint (Graphics& g)
{
    auto floatLocalBounds = getLocalBounds().toFloat();
    const float cornerSize = 3.f;

    // Fill background
    g.setColour (m_backgroundColour);
    g.fillRoundedRectangle (floatLocalBounds.reduced (1, 1), cornerSize);

    // Draw border
    g.setColour (m_outlineColour);
    g.drawRoundedRectangle (floatLocalBounds, cornerSize, 1.f);
}


void ButtonGroupManager::paintOverChildren (Graphics& g)
{
    const int width  = getWidth();
    const int height = getHeight();

    // Draw dividers between buttons
    if (m_isShowDividers)
    {
        g.setColour (COLOUR_DIVIDER);

        const int dividerOffsetY = 5;
        const int numDividers    = m_buttons.size() - 1;

        for (int i = 0; i < numDividers; ++i)
        {
            int dividerX = m_buttons[i]->getRight();
            g.drawVerticalLine (dividerX, dividerOffsetY, height - dividerOffsetY);
        }
    }

    // Draw line which displays current selected button
    g.setColour (m_accentColour);
    if (m_isRadioButtonMode)
    {
        const int lineWidth  = m_buttons[m_selectedButtonIdx]->getBounds().getWidth();
        const int lineHeight = 3;
        g.fillRect ( (int)m_currentButtonLineX, height - lineHeight - 1, lineWidth, lineHeight);
    }
}


void ButtonGroupManager::resized()
{
    const int width     = getWidth();
    const int height    = getHeight();

    const int numButtons = m_buttons.size();
    const int buttonWidth = width / numButtons;

    // Set bounds for each button
    juce::Rectangle<int> buttonBounds (0, 0, buttonWidth, height);
    for (int i = 0; i < numButtons; ++i)
    {
        m_buttons[i]->setBounds (buttonBounds);

        buttonBounds.translate (buttonWidth, 0);
    }
}


void ButtonGroupManager::buttonClicked (Button* buttonThatWasClicked)
{
    if (m_buttonListener != nullptr)
        m_buttonListener->buttonClicked (buttonThatWasClicked);

    if (m_isRadioButtonMode)
    {
        stopTimer();
        m_desiredButtonLineX = buttonThatWasClicked->getBounds().getX();
        m_selectedButtonIdx  = m_buttons.indexOf (static_cast<TextButton*> (buttonThatWasClicked));

        const float numAnimationSteps = 10.f;
        m_animationStepX = (m_desiredButtonLineX - m_currentButtonLineX) / numAnimationSteps;
        //std::cout << "== Calculation: Animation step: " << m_animationStepX << std::endl;

        startTimer (30);
    }
}


void ButtonGroupManager::timerCallback()
{
    // Move line which displays current selected button
    if (! areEqualValues (m_currentButtonLineX, m_desiredButtonLineX))
    {
        //std::cout << "Current X: " << m_currentButtonLineX << std::endl;
        //std::cout << "Desiredd X: " << m_desiredButtonLineX << std::endl;
        m_currentButtonLineX += m_animationStepX;
    }
    else
    {
        //std::cout <<"Timer stopped" << std::endl;
        m_animationStepX = 0;
        stopTimer();
    }

    repaint();
}


void ButtonGroupManager::addButton (TextButton* newButton)
{
    newButton->addListener (this);

        newButton->setLookAndFeel (m_buttonsLookAndFeel);

    addAndMakeVisible (newButton);
    m_buttons.add (newButton);

    if (m_isRadioButtonMode)
    {
        newButton->setClickingTogglesState (true);
        newButton->setRadioGroupId (1);
    }

    resized();
}


void ButtonGroupManager::removeButton (int index)
{
    if (index < 0
        || index >= m_buttons.size())
    {
        return;
    }

    m_buttons.remove (index);
}


void ButtonGroupManager::setRadioButtonMode (bool isRadioButtonMode)
{
    m_isRadioButtonMode = isRadioButtonMode;

    const int numButtons = m_buttons.size();
    for (int i = 0; i < numButtons; ++i)
    {
        m_buttons[i]->setRadioGroupId ( (int)isRadioButtonMode);
    }

    repaint();
}


void ButtonGroupManager::setButtonsLookAndFeel (LookAndFeel* newButtonsLookAndFeel)
{
    m_buttonsLookAndFeel = newButtonsLookAndFeel;

    std::for_each (m_buttons.begin(), m_buttons.end(), [this] (TextButton* button)
                    { button->setLookAndFeel (m_buttonsLookAndFeel); });

    repaint();
}


void ButtonGroupManager::setShowDividers (bool isShow)
{
    if (isShow == m_isShowDividers)
        return;

    m_isShowDividers = isShow;
    repaint();
}


void ButtonGroupManager::setButtonListener (Button::Listener* newListener)
{
    m_buttonListener = newListener;
}


void ButtonGroupManager::setBackgroundColour (Colour bgColour)
{
    m_backgroundColour = bgColour;

    repaint();
}


void ButtonGroupManager::setAccentColour (Colour accentColour)
{
    m_accentColour = accentColour;

    repaint();
}


void ButtonGroupManager::setOutlineColour (Colour outlineColour)
{
    m_outlineColour = outlineColour;

    repaint();
}
