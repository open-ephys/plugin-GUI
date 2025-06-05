/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef SYNCCHANNEL_SELECTOR_H_INCLUDED
#define SYNCCHANNEL_SELECTOR_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../../UI/PopupComponent.h"
#include "../../Utils/Utils.h"

class SyncLineSelector;

/** 

Allows the user to select the TTL line to use for synchronization

*/
class PLUGIN_API SyncChannelButton : public Button
{
public:
    /** Constructor */
    SyncChannelButton (int id, SyncLineSelector* parent);

    /** Destructor */
    ~SyncChannelButton();

    /** Returns the ID for this button's stream*/
    int getId() { return id; };

private:
    int id;
    SyncLineSelector* parent;
    int width;
    int height;
    Colour btnColour;

    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class PLUGIN_API SetPrimaryButton : public Button
{
public:
    /** Constructor */
    SetPrimaryButton (const String& name);

    /** Destructor */
    ~SetPrimaryButton();

private:
    /** Renders the button*/
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class PLUGIN_API SyncLineSelector : public PopupComponent,
                                    public Button::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {}

        // Called when the selected sync line changes
        virtual void selectedLineChanged (int selectedLine) = 0;

        // Called when updating the popup
        virtual int getSelectedLine() = 0;

        // Called when the user sets the primary stream for synchronization
        virtual void primaryStreamChanged() = 0;

        // Called when the updating to popup to reflect changes
        virtual bool isPrimaryStream() = 0;
    };

    /** Constructor */
    SyncLineSelector (Component* parent, Listener* listener, int numChans, int selectedLine, bool isPrimary, bool canSelectNone = false);

    /** Destructor */
    ~SyncLineSelector();

    int getSelectedChannel() { return selectedLine; }

    /** Mouse listener methods*/
    void mouseDown (const MouseEvent& event) override;
    void mouseMove (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;

    /** Responds to button clicks*/
    void buttonClicked (Button*) override;

    /** Popup update triggered by PopupComponent when undoing/redoing */
    void updatePopup() override;

    int nChannels;
    bool isPrimary;

    bool detectedChange;

    int buttonSize;
    int nRows;

    int width;
    int height;

    OwnedArray<SyncChannelButton> buttons;

    Array<Colour> lineColours;

private:
    Listener* listener;

    ScopedPointer<SetPrimaryButton> setPrimaryStreamButton;

    int selectedLine = 0;
    bool canSelectNone = false;
};

#endif // SYNCCHANNEL_SELECTOR_H_INCLUDED
