/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef __INFOLABEL_H_14DA9A62__
#define __INFOLABEL_H_14DA9A62__


#include "../../JuceLibraryCode/JuceHeader.h"

#include "../Processors/Visualization/Visualizer.h"

/**

  Makes it possible to switch between different tabs
  within the "Info" panel.

*/
class InfoLabelTabButton : public Button
{
public:
    /** Constructor*/
    InfoLabelTabButton(const String& name);

private:

    /** Custom button rendering function*/
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
};


/**

  Displays info about the GUI.

  Inhabits a tab in the DataViewport.

  @see UIComponent, DataViewport

*/

class InfoLabel : public Visualizer,
    public Button::Listener

{
public:
    /** Constructor */
    InfoLabel();

    /** Destructor */
    ~InfoLabel();

    /** Fills the background and draws a logo.*/
    void paint(Graphics& g);

    /** Visualizer virtual functions */
    void refresh() { }
    void update() { }
    void refreshState() { }

    /** Resizes text field*/
    void resized();

    /** Called when tab buttons are pressed.*/
    void buttonClicked(Button* button);

private:

    /** Sets the "about" text*/
    void setAboutText();

    /** Sets the "authors" text*/
    void setAuthorsText();

    /** Sets the license text*/
    void setLicenseText();

    /** Checks for mouse hovering over links */
    void mouseMove(const MouseEvent& mouse);

    /** Opens links in a browser window */
    void mouseUp(const MouseEvent& mouse);

    std::unique_ptr<TextEditor> textEditor;
    OwnedArray<InfoLabelTabButton> tabButtons;

    struct Hyperlink
    {
        String url;
        Range<int> position;
    } hyperlink;

    Array<Hyperlink> hyperlinks;

    std::unique_ptr<Drawable> color_logo;
    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<Component> viewedComponent;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoLabel);


};




#endif  // __INFOLABEL_H_14DA9A62__
