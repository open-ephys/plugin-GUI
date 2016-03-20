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

#ifndef BUTTONGROUPMANAGER_H_INCLUDED
#define BUTTONGROUPMANAGER_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../Processors/PluginManager/OpenEphysPlugin.h"

/**

    This class provides possibility to store buttons with radiobutton behaviour in the single
    component - button manager. It is responsible for positioning of buttons, any animations
    during switching, etc.

    All buttons will be stored in horizontal (for now) box - like component.

*/
class PLUGIN_API ButtonGroupManager : public Component
                                    , public Button::Listener
                                    , private Timer
{
public:
    ButtonGroupManager();

    // Component methods
    // ===========================================================
    void paint              (Graphics& g) override;
    void paintOverChildren  (Graphics& g) override;
    void resized() override;

    // Button::Listener methods
    // ===========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    /** Returns the number of stored buttons. */
    int getNumButtons() const { return m_buttons.size(); }

    /** Return whether all managed buttons are in the radiobutton mode now */
    bool isRadioButtonMode() const { return m_isRadioButtonMode; }

    /** Add button to the array of buttons to manage it.

        This class controls ownership of buttons.
    */
    void addButton (TextButton* newButton);

    /** Remove button from the manager array buttons */
    void removeButton (int index);

    /** Set if all buttons will have radiobutton behaviour */
    void setRadioButtonMode (bool isRadioButtonMode);

    /** Sets custom LookAndFeel for each button */
    void setButtonsLookAndFeel (LookAndFeel* newButtonLookAndFeel);

    /** Sets whether dividers between buttons will be visible or not */
    void setShowDividers (bool isShow);

    /** Sets the listener that will receive events from buttons */
    void setButtonListener (Button::Listener* newButtonListener);

    /** Sets the background colour for component */
    void setBackgroundColour (Colour bgColour);

    /** Sets the accent colour for component which will be used
        as colour of underline which displays current selected button */
    void setAccentColour (Colour accentColour);

    /** Sets the outline colour for component */
    void setOutlineColour (Colour outlineColour);


private:
    // Timer methods
    // ===========================================================
    void timerCallback() override;

    /** Displays if radiobutton mode is used for each button */
    bool m_isRadioButtonMode;
    bool m_isShowDividers;

    /** Stores an index of currently selected button.
        That variable become useless in the non-radiobutton mode.
    */
    int m_selectedButtonIdx;
    float m_desiredButtonLineX;
    float m_currentButtonLineX;
    float m_animationStepX;

    Button::Listener* m_buttonListener;

    Colour m_backgroundColour;
    Colour m_accentColour;
    Colour m_outlineColour;

    /** Pointer to the LookAndFeel which will be used for each button */
    LookAndFeel* m_buttonsLookAndFeel;

    /** An array which stores buttons that will be managed by this class */
    OwnedArray<TextButton> m_buttons;
};



#endif  // BUTTONGROUPMANAGER_H_INCLUDED
