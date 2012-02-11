/*
  ==============================================================================

    CustomLookAndFeel.h
    Created: 26 Jan 2012 8:42:00pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __CUSTOMLOOKANDFEEL_H_6B021009__
#define __CUSTOMLOOKANDFEEL_H_6B021009__

#include "../../JuceLibraryCode/JuceHeader.h"


class CustomLookAndFeel : public LookAndFeel

{
public:
	CustomLookAndFeel();
	~CustomLookAndFeel();

  // ======== custom fonts ================
  Typeface::Ptr Miso;

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

    const Typeface::Ptr getTypefaceForFont (const Font& font);


private:	


};


#endif  // __CUSTOMLOOKANDFEEL_H_6B021009__
