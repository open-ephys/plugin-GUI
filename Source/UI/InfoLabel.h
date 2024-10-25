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

#ifndef __INFOLABEL_H_14DA9A62__
#define __INFOLABEL_H_14DA9A62__

#include "../../JuceLibraryCode/JuceHeader.h"

#include "../Processors/Visualization/Visualizer.h"

#include "../TestableExport.h"

/**

  Makes it possible to switch between different tabs
  within the "Info" panel.

*/
class InfoLabelTabButton : public Button
{
public:
    /** Constructor*/
    InfoLabelTabButton (const String& name);

private:
    /** Custom button rendering function*/
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown);
};

/*
    
      Displays the text of the Info tab.
        
      @see InfoLabelTabButton, InfoLabel
    
    
*/
class TextComponent : public Component
{
public:
    /** Constructor */
    TextComponent();

    /** Destructor */
    ~TextComponent() {}

    /** Custom rendering function */
    void paint (Graphics& g) override;

    /** Resizes the text field */
    void resizeForText();

    /** Sets the text to be displayed */
    void setAttributedString (const AttributedString& infoText, bool isLogo = false);

private:
    AttributedString infoText;
    TextLayout infoTextLayout;

    std::unique_ptr<Drawable> colour_logo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextComponent);
};

/**

  Displays info about the GUI.

  Inhabits a tab in the DataViewport.

  @see UIComponent, DataViewport

*/

class TESTABLE InfoLabel : public Visualizer,
                           public Button::Listener

{
public:
    /** Constructor */
    InfoLabel();

    /** Destructor */
    ~InfoLabel();

    /** Fills the background and draws a logo.*/
    void paint (Graphics& g);

    /** Visualizer virtual functions */
    void refresh() {}
    void refreshState() {}

    /** Updates the colours of the text field */
    void colourChanged() override;

    /** Resizes text field*/
    void resized();

    /** Called when tab buttons are pressed.*/
    void buttonClicked (Button* button);

private:
    /** Sets the "about" text*/
    void setAboutText();

    /** Sets the "authors" text*/
    void setAuthorsText();

    /** Sets the license text*/
    void setLicenseText();

    /** Checks for mouse hovering over links */
    void mouseMove (const MouseEvent& mouse);

    /** Opens links in a browser window */
    void mouseUp (const MouseEvent& mouse);

    /** Creates hyperlinks in the text */
    void createHyperlinks();

    OwnedArray<InfoLabelTabButton> tabButtons;
    AttributedString aboutText;
    AttributedString authorsText;
    AttributedString licenseText;

    struct Hyperlink
    {
        String url = "";
        Range<int> positionX = Range<int> (0, 0);
        Range<int> positionY = Range<int> (0, 0);
    } hyperlink;

    Array<Hyperlink> hyperlinks;

    std::unique_ptr<Viewport> viewport;
    std::unique_ptr<TextComponent> viewedComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InfoLabel);
};

#endif // __INFOLABEL_H_14DA9A62__
