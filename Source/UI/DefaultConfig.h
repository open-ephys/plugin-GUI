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

#ifndef DEFAULTCONFIG_H_INCLUDED
#define DEFAULTCONFIG_H_INCLUDED

#include "../../JuceLibraryCode/JuceHeader.h"

class MainWindow;

/** 

    Allows the user to select one of several different
    default signal chains.

*/
class DefaultConfigComponent : public Component,
                               public Button::Listener,
                               public KeyListener
{
public:
    /** Constructor */
    DefaultConfigComponent();

    /** Destructor */
    ~DefaultConfigComponent();

    /** Renders the component*/
    void paint (Graphics& g) override;

    /** Sets sub-component layout*/
    void resized() override;

    /** Responds to button clicks*/
    void buttonClicked (Button*) override;

    /** Responds to key presses*/
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;

private:
    std::unique_ptr<Label> configLabel;
    std::unique_ptr<ComboBox> configSelector;
    std::unique_ptr<TextButton> goButton;

    std::unique_ptr<ImageButton> acqBoardButton;
    std::unique_ptr<Label> acqBoardLabel;

    std::unique_ptr<ImageButton> fileReaderButton;
    std::unique_ptr<Label> fileReaderLabel;

    std::unique_ptr<ImageButton> neuropixelsButton;
    std::unique_ptr<Label> neuropixelsLabel;

    FontOptions configFont;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DefaultConfigComponent);
};

class DefaultConfigWindow
{
public:
    /** Initializes the DefaultConfigWindow, and sets the window boundaries. */
    DefaultConfigWindow();

    /** Destroys the DefaultConfigWindow. */
    ~DefaultConfigWindow();

    /** Shows the DefaultConfigWindow */
    void launchWindow();

private:
    WeakReference<Component> configWindow;

    DefaultConfigComponent* configComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DefaultConfigWindow);
};

#endif
