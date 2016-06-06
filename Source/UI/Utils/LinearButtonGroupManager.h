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

#ifndef LINEARBUTTONGROUPMANAGER_H_INCLUDED
#define LINEARBUTTONGROUPMANAGER_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Processors/PluginManager/OpenEphysPlugin.h"
#include "ButtonGroupManager.h"

/**

    This class provides possibility to store buttons with radiobutton behaviour in the single
    component - button manager. It is responsible for positioning of buttons, any animations
    during switching, etc.

    All buttons will be stored lineary - either in horizontal or vertical
    box-like component. (only horizontal mode available now)

*/
class PLUGIN_API LinearButtonGroupManager : public ButtonGroupManager
                                          , private Timer
{
public:
    LinearButtonGroupManager();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the button.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        accentColourId                  = 0x1000110,  /**< The colour used as colour of underline which displays
                                                           current selected button. */
        dividersColourId                = 0x1000111   /**< The colour used to draw dividers between buttons. */
    };


    // Component methods
    // ===========================================================
    void paintOverChildren  (Graphics& g)   override;
    void resized()                          override;

    // Button::Listener methods
    // ===========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    /** Sets whether dividers between buttons will be visible or not */
    void setShowDividers (bool isShow);


private:
    // Timer methods
    // ===========================================================
    void timerCallback() override;

    /** Displays if dividers between buttons will be drawn. */
    bool m_isShowDividers;

    /** Stores an index of currently selected button.
        That variable become useless in the non-radiobutton mode.
    */
    int m_selectedButtonIdx;

    float m_desiredButtonLineX;
    float m_currentButtonLineX;
    float m_animationStepX;

    // ===========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinearButtonGroupManager)
};



#endif  // LINEARBUTTONGROUPMANAGER_H_INCLUDED
