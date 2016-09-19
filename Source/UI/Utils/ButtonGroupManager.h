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

#include <JuceHeader.h>
#include "../../Processors/PluginManager/OpenEphysPlugin.h"

/**

    This class provides is a base class for other ButtonGroupManagers.
    It stores set of buttons inside and responsible for positioning of buttons, lookAndFeel, any animations.

    @see LinearButtonGroupManager
*/
class PLUGIN_API ButtonGroupManager : public Component
                                    , public Button::Listener
{
public:
    ButtonGroupManager();
    virtual ~ButtonGroupManager();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the button.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId              = 0x1000100,  /**< The colour used to fill the buttons' manager background. */
        outlineColourId                 = 0x1000101,  /**< The colour used for the outline of button manager. */
    };

    // Component methods
    // ===========================================================
    void paint (Graphics& g)    override;
    void colourChanged()        override;

    /** All component, that inherit ButtonGroupManager should override the resized() method
        in order to control buttons positioning inside it */
    virtual void resized();
    // ===========================================================

    // Button::Listener methods
    // ===========================================================
    void buttonClicked (Button* buttonThatWasClicked) override;

    /** Returns the number of stored buttons. */
    int getNumButtons() const;

    /** Returns the button at the given index */
    Button* getButtonAt (int index) const;

    /** Return whether all managed buttons are in the radiobutton mode now */
    bool isRadioButtonMode() const;

    /** Add button to the array of buttons to manage it.

        This class controls ownership of buttons.

        Can be overriden by other buttons to change behaviour of the way, how buttons are added,
        e.g. use decorators for some buttons, etc.
    */
    virtual void addButton (Button* newButton, bool useDefaultLookAndFeel = true);

    /** Remove button from the manager array buttons. */
    void removeButton (int index);

    /** Removes all buttons from th manager array buttons. */
    void removeAllButtons();

    /** @see removeAllButtons .*/
    void clear();

    /** Set if all buttons will have radiobutton behaviour */
    void setRadioButtonMode (bool isRadioButtonMode);

    /** Sets custom LookAndFeel for each button */
    void setButtonsLookAndFeel (LookAndFeel* newButtonLookAndFeel);

    /** Sets the listener that will receive events from buttons */
    void setButtonListener (Button::Listener* newButtonListener);


protected:
    /** Displays if radiobutton mode is used for each button */
    bool m_isRadioButtonMode;

    Button::Listener* m_buttonListener;

    /** Pointer to the LookAndFeel which will be used for each button */
    LookAndFeel* m_buttonsLookAndFeel;

    /** Viewport whick will show appropriate buttons */
    Viewport m_buttonsViewport;

    /** We will instead all of our buttons to this component instead of current object
        to be sure that we can use it with viewport support - it will be easy to show
        a very big quantity of buttons with scrolling feature */
    ScopedPointer<Component> m_componentProxyHandler;

    /** An array which stores buttons that will be managed by this class */
    OwnedArray<Button> m_buttons;

    // ===========================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonGroupManager)
};



#endif  // BUTTONGROUPMANAGER_H_INCLUDED
