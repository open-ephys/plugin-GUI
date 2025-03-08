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

#ifndef __CUSTOMLOOKANDFEEL_H_6B021009__
#define __CUSTOMLOOKANDFEEL_H_6B021009__

#include "../../TestableExport.h"
#include <JuceHeader.h>

namespace ProcessorColour
{
enum IDs
{
    PROCESSOR_COLOUR = 801,
    FILTER_COLOUR = 802,
    SINK_COLOUR = 803,
    SOURCE_COLOUR = 804,
    UTILITY_COLOUR = 805,
    RECORD_COLOUR = 806,
    AUDIO_COLOUR = 807,
    SYNC_COLOUR = 808
};
}

enum ThemeColours
{
    componentBackground = 0x9001,
    componentParentBackground,
    windowBackground,
    widgetBackground,
    controlPanelBackground,
    controlPanelText,
    menuBackground,
    menuHighlightText,
    menuHighlightBackground,
    outline,
    defaultText,
    defaultFill,
    highlightedText,
    highlightedFill,
    dropShadowColour
};

enum ColourTheme
{
    LIGHT,
    MEDIUM,
    DARK
};

/**

   Used to modify the appearance of the application.

   Currently contains methods for drawing custom tabs, scroll bars, and sliders.
   It also takes care of custom fonts via getTypefaceForFont().

   @see MainWindow

*/

class TESTABLE CustomLookAndFeel : public LookAndFeel_V4
{
public:
    /** Constructor */
    CustomLookAndFeel();

    /** Destructor */
    ~CustomLookAndFeel();

    std::map<ColourTheme, std::map<ThemeColours, Colour>> themeColoursMap;

    /** Set colour theme*/
    void setTheme (ColourTheme theme);

    // ======== custom typeface getter: =============================
    Typeface::Ptr getTypefaceForFont (const Font& font) override;

    // ======== custom scroll bar methods: =============================

    void drawScrollbarButton (Graphics& g,
                              ScrollBar& scrollbar,
                              int width,
                              int height,
                              int buttonDirection,
                              bool isScrollBarVertical,
                              bool isMouseOverButton,
                              bool isButtonDown) override;

    void drawScrollbar (Graphics& g,
                        ScrollBar& scrollbar,
                        int x,
                        int y,
                        int width,
                        int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition,
                        int thumbSize,
                        bool isMouseOver,
                        bool isMouseDown) override;

    // ======== custom tooltip methods: ============================
    // juce::Rectangle<int> getTooltipBounds(const String &tipText, Point<int> screenPos, juce::Rectangle<int> parentArea) override;

    // void drawTooltip(Graphics &, const String &text, int width, int height) override;

    // TextLayout layoutTooltipText(const String& text, Colour colour);

    // ======== custom slider methods: =============================

    void drawLinearSlider (Graphics&, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, Slider::SliderStyle, Slider&) override;

    int getSliderThumbRadius (Slider& slider) override;

    void drawPointer (Graphics&, float x, float y, float diameter, const Colour&, int direction) noexcept;

    // ======== custom combo box methods: =============================

    void drawComboBox (Graphics& g, int width, int height, const bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override;

    Font getComboBoxFont (ComboBox& box) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    // ========= custom popup menu & menu bar methods: ===========================

    void drawPopupMenuBackground (Graphics&, int width, int height) override;

    Font getPopupMenuFont() override;

    void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;

    Font getMenuBarFont (MenuBarComponent&, int itemIndex, const String& itemText) override;

    // ========= custom button methods: ===========================

    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown) override;

    void drawButtonText (Graphics& g,
                         TextButton& button,
                         bool isMouseOverButton,
                         bool isButtonDown) override;

    Font getTextButtonFont (TextButton&, int buttonHeight) override;

    // ========= custom toggle button methods: ===========================
    void drawToggleButton (Graphics&, ToggleButton&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTickBox (Graphics&, Component&, float x, float y, float w, float h, bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    Path getTickShape (float height) override;

    // ========= custom progress bar methods: ===========================
    void drawProgressBar (Graphics&, ProgressBar&, int width, int height, double progress, const String& textToShow) override;

    // ========= custom document window methods: ===========================
    Button* createDocumentWindowButton (int) override;
    void positionDocumentWindowButtons (DocumentWindow&, int, int, int, int, Button*, Button*, Button*, bool) override;
    void drawDocumentWindowTitleBar (DocumentWindow&, Graphics&, int, int, int, int, const Image*, bool) override;

    // ========= custom Alert Window methods: ===========================
    AlertWindow* createAlertWindow (const String& title, const String& message, const String& button1, const String& button2, const String& button3, MessageBoxIconType iconType, int numButtons, Component* associatedComponent) override;
    void drawAlertBox (Graphics&, AlertWindow&, const juce::Rectangle<int>& textArea, TextLayout&) override;
    int getAlertWindowButtonHeight() override;
    Font getAlertWindowTitleFont() override;
    Font getAlertWindowMessageFont() override;
    Font getAlertWindowFont() override;

    // ======== custom TabButton methods: ================================
    int getTabButtonSpaceAroundImage() override;
    int getTabButtonOverlap (int tabDepth) override;
    int getTabButtonBestWidth (TabBarButton&, int tabDepth) override;
    juce::Rectangle<int> getTabButtonExtraComponentBounds (const TabBarButton&, juce::Rectangle<int>& textArea, Component& extraComp) override;

    void drawTabButton (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;
    Font getTabButtonFont (TabBarButton&, float height) override;
    void drawTabButtonText (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) override;
    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int w, int h) override;

    void createTabButtonShape (TabBarButton&, Path&, bool isMouseOver, bool isMouseDown) override;
    void fillTabButtonShape (TabBarButton&, Graphics&, const Path&, bool isMouseOver, bool isMouseDown) override;

    // ======== custom CallOutBox method: ================================
    void drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) override;

    // ======== custom TableHeaderComponent methods: ================================
    void drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    void drawTableHeaderColumn (Graphics&, TableHeaderComponent&, const String& columnName, int columnId, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags) override;

    // ======== custom Resizable Component methods: ================================
    void drawResizableFrame (Graphics&, int w, int h, const BorderSize<int>&) override;
    void fillResizableWindowBackground (Graphics&, int w, int h, const BorderSize<int>&, ResizableWindow&) override;
    void drawResizableWindowBorder (Graphics&, int w, int h, const BorderSize<int>& border, ResizableWindow&) override;

private:
    Typeface::Ptr

        cpmonoExtraLight,
        cpmonoLight,
        cpmonoPlain,
        cpmonoBold,

        firaCodeLight,
        firaCodeMedium,
        firaCodeRetina,
        firaCodeRegular,
        firaCodeSemiBold,
        firaCodeBold,

        firaSansExtraLight,
        firaSansRegular,
        firaSansSemiBold,

        silkscreen,

        nimbusSans,

        interBold,
        interLight,
        interMedium,
        interRegular,
        interSemiBold;

    Font getCommonMenuFont();

    void initializeColours();
};

#endif // __CUSTOMLOOKANDFEEL_H_6B021009__
