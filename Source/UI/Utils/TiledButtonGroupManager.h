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

#ifndef TILEDBUTTONGROUPMANAGER_H_INCLUDED
#define TILEDBUTTONGROUPMANAGER_H_INCLUDED

#include "../../Processors/PluginManager/OpenEphysPlugin.h"
#include "ButtonGroupManager.h"


/**

    This class provides possibility to store buttons in several rows having tile-structure.
    It is responsible for positioning of buttons, any animations during resizing.
    It also have "Fast select" feature which allows as to quickly select many buttons
    using mouse drag selection. (mouse drag - to select, shift+mouse drag to deselect)

*/
class PLUGIN_API TiledButtonGroupManager : public ButtonGroupManager
{
public:
    TiledButtonGroupManager();

    // Component methods
    // ===========================================================
    void resized() override;

    void mouseDown (const MouseEvent& e) override;
    void mouseUp   (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    // ===========================================================

    // Button::Listener methods
    // ===========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    /** Returns whether fast selection mode (toggle on or off buttons by dragging mouse) is enabled or not */
    bool isFastSelectionModeEnabled() const;

    /** Sets whether fast selection mode (toggle on or off buttons by dragging mouse) will be enabled */
    void setFastSelectionModeEnabled (bool isFastSelectionMode);

    /** Sets the size of each button */
    void setButtonSize (int buttonWidth, int buttonHeight);

    /** Sets the minimal padding which used will be used around buttons */
    void setMinPaddingBetweenButtons (int minPadding);

    /** Add button to the array of buttons to manage it.

        This class controls ownership of buttons.
    */
    void addButton (Button* newButton, bool useDefaultLookAndFeel = true) override;


private:
    // Returns the index of button at given position.
    // If nothing found at this position - returns -1.
    int getIndexOfButtonAtPosition (juce::Point<int> position) const;

    int m_buttonWidth;
    int m_buttonHeight;
    int m_minPaddingForButtons;

    int m_firstSelectedButtonIdx;
    int m_lastSelectedButtonIdx;
    bool m_isToggleOnMode;
    bool m_isDraggingMouseNow;
    bool m_isSelectButtonsByDragging;

    ScopedPointer<LookAndFeel> m_materialButtonsLookAndFeel;

    // ===========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TiledButtonGroupManager)
};



#endif //TILEDBUTTONGROUPMANAGER_H_INCLUDED
