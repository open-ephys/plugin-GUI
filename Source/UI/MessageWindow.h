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

#ifndef MESSAGEWINDOW_H_INCLUDED
#define MESSAGEWINDOW_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"

class UIComponent;

/** 

    Allows the user to write a message to disk.
    
    The message is timestamped according to when 
    the window is opened.

*/
class MessageWindowComponent : public Component,
                               public Button::Listener,
                               public Label::Listener,
                               public ComboBox::Listener
{
public:
    /** Constructor */
    MessageWindowComponent();

    /** Destructor */
    ~MessageWindowComponent();

    /** Renders the component*/
    void paint (Graphics& g) override;

    /** Sets sub-component layout*/
    void resized() override;

    /** Responds to button clicks*/
    void buttonClicked (Button*) override;

    /** Responds to label text changes */
    void labelTextChanged (Label*) override {}

    /** Called when label changes to a TextEditor */
    void editorShown (Label*, TextEditor&) override;

    /** Called a new item is selected from the ComboBox */
    void comboBoxChanged (ComboBox*) override;

private:
    /** Converts time in milliseconds to HH:MM:SS format */
    String createTimeString (float timeMilliseconds);

    /** Resets time of next message */
    void resetTime();

    std::unique_ptr<Label> timestampLabel;
    std::unique_ptr<TextButton> timestampResetButton;

    std::unique_ptr<Label> messageLabel;
    std::unique_ptr<TextButton> sendMessageButton;

    std::unique_ptr<ComboBox> savedMessageSelector;
    std::unique_ptr<TextButton> clearSavedMessagesButton;

    int64 messageTimeMillis;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageWindowComponent);
};

class MessageWindow
{
public:
    /** Initializes the MessageWindow, and sets the window boundaries. */
    MessageWindow();

    /** Destroys the MessageWindow. */
    ~MessageWindow();

    /** Shows the MessageWindow */
    void launch();

private:
    WeakReference<Component> messageWindow;

    MessageWindowComponent* messageWindowComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MessageWindow);
};

#endif
