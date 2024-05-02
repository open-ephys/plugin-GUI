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
#include "../LookAndFeel/CustomLookAndFeel.h"

#include <algorithm>


ButtonGroupManager::ButtonGroupManager()
    : m_isRadioButtonMode       (true)
    , m_buttonListener          (nullptr)
    , m_buttonsLookAndFeel      (nullptr)
    , m_componentProxyHandler   (new Component)
{
    addAndMakeVisible (m_buttonsViewport);
    m_buttonsViewport.setViewedComponent (m_componentProxyHandler, false);
    m_buttonsViewport.setScrollBarsShown (false, false, true, false);
}


ButtonGroupManager::~ButtonGroupManager()
{
    m_buttons.clear();
}


void ButtonGroupManager::resized()
{
    const int width  = getWidth();
    const int height = getHeight();

    if (m_buttons.size())
        m_componentProxyHandler->setBounds (0, 0, width, jmax (height,
                                                               m_buttons[m_buttons.size() - 1]->getBounds().getBottom()));
    else
        m_componentProxyHandler->setBounds (0, 0, width, height);

    m_buttonsViewport.setBounds (0, 0, getWidth(), getHeight());
}


void ButtonGroupManager::colourChanged()
{
    repaint();
}


void ButtonGroupManager::buttonClicked (Button* buttonThatWasClicked)
{
    if (m_buttonListener != nullptr)
        m_buttonListener->buttonClicked (buttonThatWasClicked);
}


int ButtonGroupManager::getNumButtons() const
{
    return m_buttons.size();
}


Button* ButtonGroupManager::getButtonAt (int index) const
{
    jassert (index >= 0 && index < m_buttons.size());

    return m_buttons[index];
}


bool ButtonGroupManager::isRadioButtonMode() const
{
    return m_isRadioButtonMode;
}


void ButtonGroupManager::addButton (Button* newButton, bool useDefaultLookAndFeel)
{
    newButton->addListener (this);

    if (useDefaultLookAndFeel)
        newButton->setLookAndFeel (m_buttonsLookAndFeel);

    m_componentProxyHandler->addAndMakeVisible (newButton);
    m_buttons.add (newButton);

    if (m_isRadioButtonMode)
    {
        newButton->setRadioGroupId (1);
    }

    resized();
}


void ButtonGroupManager::removeButton (int index)
{
    if (index < 0
        || index >= m_buttons.size())
    {
        jassertfalse;
        return;
    }

    m_buttons.remove (index);
}


void ButtonGroupManager::removeAllButtons()
{
    m_buttons.clear();
}


void ButtonGroupManager::clear()
{
    removeAllButtons();
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

    std::for_each (m_buttons.begin(), m_buttons.end(), [this] (Button* button)
                    { button->setLookAndFeel (m_buttonsLookAndFeel); });

    repaint();
}


void ButtonGroupManager::setButtonListener (Button::Listener* newListener)
{
    m_buttonListener = newListener;
}
