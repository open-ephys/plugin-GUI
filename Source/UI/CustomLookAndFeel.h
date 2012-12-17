/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef __CUSTOMLOOKANDFEEL_H_6B021009__
#define __CUSTOMLOOKANDFEEL_H_6B021009__

#ifdef WIN32
#include <Windows.h>
#endif
#include "../../JuceLibraryCode/JuceHeader.h"

/**
  
   Used to modify the appearance of the application.

   Currently contains methods for drawing custom tabs, scroll bars, and sliders.
   It also takes care of custom fonts via getTypefaceForFont().

   @see MainWindow

*/

class CustomLookAndFeel : public LookAndFeel
{
public:
    CustomLookAndFeel();
	~CustomLookAndFeel();

	// ======== custom typeface getter: =============================
	const Typeface::Ptr getTypefaceForFont (const Font& font);

	// ======== custom tab methods: ============================= 

	void drawTabButton (Graphics & g, 
						int w, int h, 
						const Colour& preferredColour,
						int tabIndex, const String& text,
						Button& button,
						TabbedButtonBar::Orientation,
						bool isMouseOver,
						bool isMouseDown,
						bool isFrontTab);

	void drawTabButtonText (Graphics& g,
						    int x, int y, int w, int h,
						    const Colour& preferredBackgroundColour,
						    int tabIndex,
						    const String& text,
						    Button& button,
						    TabbedButtonBar::Orientation o,
						    bool isMouseOver,
						    bool isMouseDown,
						    bool isFrontTab);

	int getTabButtonBestWidth (int tabIndex,
                               const String& text,
                               int tabDepth,
                               Button& button);

	int getTabButtonSpaceAroundImage ();

	void drawTabAreaBehindFrontButton (Graphics& g,
									   int w, int h,
									   TabbedButtonBar& tabBar,
									   TabbedButtonBar::Orientation o);

	int getTabButtonOverlap (int tabDepth);

	// ======== custom scroll bar methods: =============================

	void drawScrollbarButton (Graphics& g,
                              ScrollBar& scrollbar,
                              int width, int height,
                              int buttonDirection,
                              bool isScrollBarVertical,
                              bool isMouseOverButton,
                              bool isButtonDown);

    void drawScrollbar (Graphics& g,
                        ScrollBar& scrollbar,
                        int x, int y,
                        int width, int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition,
                        int thumbSize,
                        bool isMouseOver,
                        bool isMouseDown);


   	// ======== custom slider methods: =============================

    void drawLinearSliderThumb (Graphics& g,
                                int x, int y,
                                int width, int height,
                                float sliderPos,
                                float minSliderPos,
                                float maxSliderPos,
                                const Slider::SliderStyle style,
                                Slider& slider);

	void drawLinearSliderBackground (Graphics& g,
                                     int x, int y,
                                     int width, int height,
                                     float /*sliderPos*/,
                                     float /*minSliderPos*/,
                                     float /*maxSliderPos*/,
                                     const Slider::SliderStyle /*style*/,
                                     Slider& slider);


    int getSliderThumbRadius (Slider& slider);

    void drawSliderKnob (Graphics& g,
                         const float x, const float y,
                         const float diameter,
                         const Colour& colour,
                         const float outlineThickness) throw();

    void drawGlassPointer (Graphics& g,
                           const float x, const float y,
                           const float diameter,
                           const Colour& colour, const float outlineThickness,
                           const int direction) throw();

    // ======== custom combo box methods: =============================

    void drawComboBox (Graphics& g, int width, int height,
                       const bool isButtonDown,
                       int buttonX, int buttonY,
                       int buttonW, int buttonH,
                       ComboBox& box);

private:

    // UNCOMMENT AFTER UPDATE
    // this maps strings to customtypeface pointers
    //HashMap<String, Typeface::Ptr> typefaceMap;

    MemoryInputStream
        cpmonoExtraLightStream,
        cpmonoLightStream,
        cpmonoPlainStream,
        cpmonoBoldStream,
        cpmonoBlackStream,
        misoRegularStream,
        silkscreenStream;

    Typeface::Ptr
        cpmonoExtraLight,
        cpmonoLight,
        cpmonoPlain,
        cpmonoBold,
        cpmonoBlack,
        misoRegular,
        silkscreen;

};


#endif  // __CUSTOMLOOKANDFEEL_H_6B021009__
