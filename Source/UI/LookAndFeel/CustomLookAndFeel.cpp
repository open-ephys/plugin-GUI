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

#include "CustomLookAndFeel.h"

#include "../../Utils/Utils.h"

CustomLookAndFeel::CustomLookAndFeel()
    : silkscreen (Typeface::createSystemTypefaceFor (BinaryData::SilkscreenRegular_ttf,
                                                     BinaryData::SilkscreenRegular_ttfSize)),

      nimbusSans (Typeface::createSystemTypefaceFor (BinaryData::NimbusSans_otf,
                                                     BinaryData::NimbusSans_otfSize)),

      cpmonoExtraLight (Typeface::createSystemTypefaceFor (BinaryData::CPMonoExtraLight_otf,
                                                           BinaryData::CPMonoExtraLight_otfSize)),
      cpmonoLight (Typeface::createSystemTypefaceFor (BinaryData::CPMonoLight_otf,
                                                      BinaryData::CPMonoLight_otfSize)),
      cpmonoPlain (Typeface::createSystemTypefaceFor (BinaryData::CPMonoPlain_otf,
                                                      BinaryData::CPMonoPlain_otfSize)),
      cpmonoBold (Typeface::createSystemTypefaceFor (BinaryData::CPMonoBold_otf,
                                                     BinaryData::CPMonoBold_otfSize)),

      firaCodeLight (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeLight_ttf,
                                                        BinaryData::FiraCodeLight_ttfSize)),
      firaCodeMedium (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeMedium_ttf,
                                                         BinaryData::FiraCodeMedium_ttfSize)),
      firaCodeRetina (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeRetina_ttf,
                                                         BinaryData::FiraCodeRetina_ttfSize)),
      firaCodeRegular (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeRegular_ttf,
                                                          BinaryData::FiraCodeRegular_ttfSize)),
      firaCodeSemiBold (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeSemiBold_ttf,
                                                           BinaryData::FiraCodeSemiBold_ttfSize)),
      firaCodeBold (Typeface::createSystemTypefaceFor (BinaryData::FiraCodeBold_ttf,
                                                       BinaryData::FiraCodeBold_ttfSize)),

      firaSansExtraLight (Typeface::createSystemTypefaceFor (BinaryData::FiraSansExtraLight_ttf,
                                                             BinaryData::FiraSansExtraLight_ttfSize)),
      firaSansRegular (Typeface::createSystemTypefaceFor (BinaryData::FiraSansRegular_ttf,
                                                          BinaryData::FiraSansRegular_ttfSize)),
      firaSansSemiBold (Typeface::createSystemTypefaceFor (BinaryData::FiraSansSemiBold_ttf,
                                                           BinaryData::FiraSansSemiBold_ttfSize)),

      interBold (Typeface::createSystemTypefaceFor (BinaryData::InterBold_ttf,
                                                    BinaryData::InterBold_ttfSize)),
      interLight (Typeface::createSystemTypefaceFor (BinaryData::InterLight_ttf,
                                                     BinaryData::InterLight_ttfSize)),
      interMedium (Typeface::createSystemTypefaceFor (BinaryData::InterMedium_ttf,
                                                      BinaryData::InterMedium_ttfSize)),
      interRegular (Typeface::createSystemTypefaceFor (BinaryData::InterRegular_ttf,
                                                       BinaryData::InterRegular_ttfSize)),
      interSemiBold (Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_ttf,
                                                        BinaryData::InterSemiBold_ttfSize))
{
    initializeColours();

    setTheme (MEDIUM);
}

CustomLookAndFeel::~CustomLookAndFeel() {}

void CustomLookAndFeel::initializeColours()
{
    // Processor colours are consistent across all themes, but
    // can be customized by right-clicking on the processor list
    setColour (ProcessorColour::IDs::FILTER_COLOUR, Colour (0, 160, 225));
    setColour (ProcessorColour::IDs::SINK_COLOUR, Colour (0, 166, 81));
    setColour (ProcessorColour::IDs::SOURCE_COLOUR, Colour (241, 90, 41));
    setColour (ProcessorColour::IDs::UTILITY_COLOUR, Colour (90, 110, 110));
    setColour (ProcessorColour::IDs::RECORD_COLOUR, Colour (255, 0, 0));
    setColour (ProcessorColour::IDs::AUDIO_COLOUR, Colour (0, 0, 0));
    setColour (ProcessorColour::IDs::SYNC_COLOUR, Colour (255, 165, 0));

    themeColoursMap[MEDIUM] = {
        { ThemeColours::componentBackground, Colour (130, 130, 130) },
        { ThemeColours::componentParentBackground, Colour (58, 58, 58) },
        { ThemeColours::windowBackground, Colour (15, 15, 15) },
        { ThemeColours::widgetBackground, Colour (190, 190, 190) },
        { ThemeColours::controlPanelBackground, Colour (50, 50, 50) },
        { ThemeColours::controlPanelText, Colour (220, 220, 220) },
        { ThemeColours::menuBackground, Colour (140, 140, 140) },
        { ThemeColours::menuHighlightText, Colours::black },
        { ThemeColours::menuHighlightBackground, Colour (244, 148, 32) },
        { ThemeColours::outline, Colours::black },
        { ThemeColours::defaultText, Colours::black },
        { ThemeColours::defaultFill, Colour (95, 95, 95) },
        { ThemeColours::highlightedText, Colours::black },
        { ThemeColours::highlightedFill, Colour (244, 148, 32) },
        { ThemeColours::dropShadowColour, Colours::black.withAlpha (0.5f) }
    };

    themeColoursMap[DARK] = {
        { ThemeColours::componentBackground, Colour (45, 45, 45) },
        { ThemeColours::componentParentBackground, Colour (30, 30, 30).darker (0.3f) },
        { ThemeColours::windowBackground, Colour (15, 15, 15) },
        { ThemeColours::widgetBackground, Colour (40, 40, 40).darker() },
        { ThemeColours::controlPanelBackground, Colour (45, 45, 45) },
        { ThemeColours::controlPanelText, Colour (224, 224, 224) },
        { ThemeColours::menuBackground, Colour (30, 30, 30) },
        { ThemeColours::menuHighlightText, Colour (20, 20, 20) },
        { ThemeColours::menuHighlightBackground, Colour (153, 200, 214) },
        { ThemeColours::outline, Colours::black },
        { ThemeColours::defaultText, Colour (224, 224, 224) },
        { ThemeColours::defaultFill, Colour (80, 80, 80) },
        { ThemeColours::highlightedText, Colour (240, 240, 240) },
        { ThemeColours::highlightedFill, Colour (230, 152, 63) },
        { ThemeColours::dropShadowColour, Colours::black.withAlpha (0.75f) }
    };

    themeColoursMap[LIGHT] = {
        { ThemeColours::componentBackground, Colour (225, 225, 225) },
        { ThemeColours::componentParentBackground, Colour (225, 225, 225).darker() },
        { ThemeColours::windowBackground, Colour (176, 176, 176) },
        { ThemeColours::widgetBackground, Colour (225, 225, 225).brighter (0.6f) },
        { ThemeColours::controlPanelBackground, Colour (225, 225, 225) },
        { ThemeColours::controlPanelText, Colours::black },
        { ThemeColours::menuBackground, Colour (225, 225, 225) },
        { ThemeColours::menuHighlightText, Colours::black },
        { ThemeColours::menuHighlightBackground, Colour (138, 193, 232) },
        { ThemeColours::outline, Colours::black },
        { ThemeColours::defaultText, Colours::black },
        { ThemeColours::defaultFill, Colour (110, 110, 110) },
        { ThemeColours::highlightedText, Colours::black },
        { ThemeColours::highlightedFill, Colour (138, 193, 232) },
        { ThemeColours::dropShadowColour, Colours::black.withAlpha (0.4f) }
    };
}

void CustomLookAndFeel::setTheme (ColourTheme theme)
{
    auto currentThemeColours = themeColoursMap[theme];

    for (const auto& [colourId, colour] : currentThemeColours)
    {
        setColour (colourId, colour);
    }

    const Colour transparent = Colour (0x00000000);

    setColour (ProcessorColour::IDs::PROCESSOR_COLOUR, currentThemeColours[ThemeColours::controlPanelBackground]);

    setColour (TextButton::buttonColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (TextButton::buttonOnColourId, currentThemeColours[ThemeColours::highlightedFill]);
    setColour (TextButton::textColourOnId, currentThemeColours[ThemeColours::defaultText]);
    setColour (TextButton::textColourOffId, currentThemeColours[ThemeColours::defaultText]);

    setColour (ToggleButton::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (ToggleButton::tickColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (ToggleButton::tickDisabledColourId, currentThemeColours[ThemeColours::widgetBackground]);

    setColour (TextEditor::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (TextEditor::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (TextEditor::highlightColourId, currentThemeColours[ThemeColours::widgetBackground].contrasting (0.5f).withAlpha (0.4f));
    setColour (TextEditor::highlightedTextColourId, currentThemeColours[ThemeColours::highlightedText]);
    setColour (TextEditor::outlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (TextEditor::focusedOutlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (TextEditor::shadowColourId, transparent);

    float darkerAmount = theme == LIGHT ? 0.0f : 0.6f;
    setColour (CodeEditorComponent::backgroundColourId, currentThemeColours[ThemeColours::controlPanelBackground].darker (darkerAmount));
    setColour (CodeEditorComponent::defaultTextColourId, currentThemeColours[ThemeColours::controlPanelText]);
    setColour (CodeEditorComponent::highlightColourId, currentThemeColours[ThemeColours::controlPanelBackground].contrasting (0.5f).withAlpha (0.4f));
    setColour (CodeEditorComponent::lineNumberBackgroundId, currentThemeColours[ThemeColours::controlPanelBackground]);
    setColour (CodeEditorComponent::lineNumberTextId, currentThemeColours[ThemeColours::controlPanelText]);

    setColour (CaretComponent::caretColourId, currentThemeColours[ThemeColours::defaultText]);

    setColour (Label::backgroundColourId, transparent);
    setColour (Label::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (Label::outlineColourId, transparent);
    setColour (Label::textWhenEditingColourId, currentThemeColours[ThemeColours::defaultText]);

    setColour (ScrollBar::backgroundColourId, transparent);
    setColour (ScrollBar::thumbColourId, currentThemeColours[ThemeColours::defaultFill]);
    setColour (ScrollBar::trackColourId, transparent);

    setColour (PopupMenu::backgroundColourId, currentThemeColours[ThemeColours::menuBackground]);
    setColour (PopupMenu::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (PopupMenu::headerTextColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (PopupMenu::highlightedTextColourId, currentThemeColours[ThemeColours::menuHighlightText]);
    setColour (PopupMenu::highlightedBackgroundColourId, currentThemeColours[ThemeColours::menuHighlightBackground]);

    setColour (ComboBox::buttonColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (ComboBox::outlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (ComboBox::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (ComboBox::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (ComboBox::arrowColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (ComboBox::focusedOutlineColourId, currentThemeColours[ThemeColours::highlightedFill]);

    setColour (ListBox::backgroundColourId, currentThemeColours[ThemeColours::componentBackground]);
    setColour (ListBox::outlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (ListBox::textColourId, currentThemeColours[ThemeColours::defaultText]);

    setColour (Slider::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (Slider::thumbColourId, currentThemeColours[ThemeColours::defaultFill]);
    setColour (Slider::trackColourId, currentThemeColours[ThemeColours::highlightedFill]);
    setColour (Slider::rotarySliderFillColourId, currentThemeColours[ThemeColours::highlightedFill]);
    setColour (Slider::rotarySliderOutlineColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (Slider::textBoxTextColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (Slider::textBoxBackgroundColourId, transparent);
    setColour (Slider::textBoxHighlightColourId, currentThemeColours[ThemeColours::menuHighlightBackground].withAlpha (0.4f));
    setColour (Slider::textBoxOutlineColourId, currentThemeColours[ThemeColours::outline]);

    setColour (ResizableWindow::backgroundColourId, currentThemeColours[ThemeColours::componentBackground]);

    setColour (DocumentWindow::textColourId, currentThemeColours[ThemeColours::controlPanelText]);

    setColour (AlertWindow::backgroundColourId, currentThemeColours[ThemeColours::componentBackground]);
    setColour (AlertWindow::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (AlertWindow::outlineColourId, currentThemeColours[ThemeColours::outline]);

    setColour (ProgressBar::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (ProgressBar::foregroundColourId, currentThemeColours[ThemeColours::highlightedFill]);

    setColour (TooltipWindow::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (TooltipWindow::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (TooltipWindow::outlineColourId, currentThemeColours[ThemeColours::outline]);

    setColour (TabbedComponent::backgroundColourId, transparent);
    setColour (TabbedComponent::outlineColourId, transparent);
    setColour (TabbedButtonBar::tabOutlineColourId, currentThemeColours[ThemeColours::outline].withAlpha (0.5f));
    setColour (TabbedButtonBar::tabTextColourId, currentThemeColours[ThemeColours::defaultText].withAlpha (0.75f));
    setColour (TabbedButtonBar::frontOutlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (TabbedButtonBar::frontTextColourId, currentThemeColours[ThemeColours::defaultText]);

    setColour (ResizableWindow::backgroundColourId, currentThemeColours[ThemeColours::windowBackground]);

    setColour (TableHeaderComponent::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (TableHeaderComponent::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (TableHeaderComponent::outlineColourId, currentThemeColours[ThemeColours::outline]);
    setColour (TableHeaderComponent::highlightColourId, currentThemeColours[ThemeColours::highlightedFill]);

    setColour (GroupComponent::textColourId, currentThemeColours[ThemeColours::defaultText]);
    setColour (GroupComponent::outlineColourId, currentThemeColours[ThemeColours::defaultText].withAlpha (0.5f));

    setColour (BubbleComponent::backgroundColourId, currentThemeColours[ThemeColours::widgetBackground]);
    setColour (BubbleComponent::outlineColourId, currentThemeColours[ThemeColours::outline]);
}

//==============================================================================
// FONT/TYPEFACE METHODS :
//==============================================================================

Typeface::Ptr CustomLookAndFeel::getTypefaceForFont (const Font& font)
{
    String typefaceName = font.getTypefaceName();
    String typefaceStyle = font.getTypefaceStyle();

    if (typefaceName.equalsIgnoreCase ("CP Mono"))
    {
        if (typefaceStyle.equalsIgnoreCase ("Plain"))
        {
            return cpmonoPlain;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Extra Light"))
        {
            return cpmonoExtraLight;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Light"))
        {
            return cpmonoLight;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Bold"))
        {
            return cpmonoBold;
        }
        else
        {
            return cpmonoPlain; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase ("Fira Code"))
    {
        if (typefaceStyle.equalsIgnoreCase ("Light"))
        {
            return firaCodeLight;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Medium"))
        {
            return firaCodeMedium;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Regular"))
        {
            return firaCodeRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Retina"))
        {
            return firaCodeRetina;
        }
        else if (typefaceStyle.equalsIgnoreCase ("SemiBold"))
        {
            return firaCodeSemiBold;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Bold"))
        {
            return firaCodeBold;
        }
        else
        {
            return firaCodeRegular; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase ("Fira Sans"))
    {
        if (typefaceStyle.equalsIgnoreCase ("Extra Light"))
        {
            return firaSansExtraLight;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Regular"))
        {
            return firaSansRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase ("SemiBold"))
        {
            return firaSansSemiBold;
        }
        else
        {
            return firaSansSemiBold; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase ("Silkscreen"))
    {
        return silkscreen;
    }
    else if (typefaceName.equalsIgnoreCase ("Nimbus Sans"))
    {
        return nimbusSans;
    }
    else if (typefaceName.equalsIgnoreCase ("Inter"))
    {
        if (typefaceStyle.equalsIgnoreCase ("Bold"))
        {
            return interBold;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Light"))
        {
            return interLight;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Medium"))
        {
            return interMedium;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Regular"))
        {
            return interRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase ("Semi Bold"))
        {
            return interSemiBold;
        }
        else
        {
            return interRegular; // default weight
        }
    }
    else // default
    {
#ifdef JUCE_MAC
        return firaCodeRetina;
#else
        return firaCodeRegular;
#endif
    }
}

//==================================================================
// SCROLL BAR METHODS :
//==================================================================

void CustomLookAndFeel::drawScrollbarButton (Graphics& g,
                                             ScrollBar& scrollbar,
                                             int width,
                                             int height,
                                             int buttonDirection,
                                             bool isScrollBarVertical,
                                             bool isMouseOverButton,
                                             bool isButtonDown)
{
    Path p;

    float w1 = 0.25f;
    float w2 = 0.75f;

    if (buttonDirection == 0)
        p.addTriangle (width * 0.5f, height * 0.2f, width * w1, height * 0.7f, width * w2, height * 0.7f);
    else if (buttonDirection == 1)
        p.addTriangle (width * 0.8f, height * 0.5f, width * 0.3f, height * w1, width * 0.3f, height * w2);
    else if (buttonDirection == 2)
        p.addTriangle (width * 0.5f, height * 0.8f, width * w1, height * 0.3f, width * w2, height * 0.3f);
    else if (buttonDirection == 3)
        p.addTriangle (width * 0.2f, height * 0.5f, width * 0.7f, height * w1, width * 0.7f, height * w2);

    if (isButtonDown)
        g.setColour (findColour (ScrollBar::thumbColourId).contrasting (0.2f));
    else
        g.setColour (findColour (ScrollBar::thumbColourId));

    g.fillPath (p);

    if (isMouseOverButton)
        g.strokePath (p, PathStrokeType (1.0f));
}

void CustomLookAndFeel::drawScrollbar (Graphics& g,
                                       ScrollBar& scrollbar,
                                       int x,
                                       int y,
                                       int width,
                                       int height,
                                       bool isScrollbarVertical,
                                       int thumbStartPosition,
                                       int thumbSize,
                                       bool isMouseOver,
                                       bool isMouseDown)

{
    Path thumbPath;

    const float slotIndent = jmin (width, height) > 15 ? 1.0f : 0.0f;
    const float thumbIndent = slotIndent + 3.0f;
    const float thumbIndentx2 = thumbIndent * 2.0f;

    if (isScrollbarVertical)
    {
        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (x + thumbIndent,
                                           thumbStartPosition + thumbIndent,
                                           width - thumbIndentx2,
                                           thumbSize - thumbIndentx2,
                                           (width - thumbIndentx2) * 0.3f);
    }
    else
    {
        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (thumbStartPosition + thumbIndent,
                                           y + thumbIndent,
                                           thumbSize - thumbIndentx2,
                                           height - thumbIndentx2,
                                           (height - thumbIndentx2) * 0.3f);
    }

    g.setColour (findColour (ScrollBar::thumbColourId));
    g.fillPath (thumbPath);
}

//==================================================================
// TOOLTIP METHODS :
//==================================================================

// TextLayout CustomLookAndFeel::layoutTooltipText (const String& text, Colour colour)
// {
//     const float tooltipFontSize = 13.0f;
//     const int maxToolTipWidth = 400;

//     AttributedString s;
//     s.setJustification (Justification::centredTop);
//     s.append (text, Font (tooltipFontSize, Font::bold), colour);

//     TextLayout tl;
//     tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
//     return tl;
// }

// Rectangle<int> CustomLookAndFeel::getTooltipBounds(const String &tipText, Point<int> screenPos, Rectangle<int> parentArea)
// {
//     const TextLayout tl (layoutTooltipText (tipText, Colours::black));

//         auto w = (int) (tl.getWidth() + 14.0f);
//         auto h = (int) (tl.getHeight() + 4.0f);

//         return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 5) : screenPos.x + 10,
//                                screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 3)  : screenPos.y + 3,
//                                w, h)
//                  .constrainedWithin (parentArea);
// }

// void CustomLookAndFeel::drawTooltip(Graphics &g, const String &text, int width, int height)
// {
//     g.fillAll (findColour (TooltipWindow::backgroundColourId));

//    #if ! JUCE_MAC // The mac windows already have a non-optional 1 pix outline, so don't double it here..
//     g.setColour (findColour (TooltipWindow::outlineColourId));
//     g.drawRect (0, 0, width, height, 1);
//    #endif

//     layoutTooltipText (text, findColour (TooltipWindow::textColourId))
//         .draw (g, Rectangle<float> ((float) width, (float) height));
// }

//==================================================================
// SLIDER METHODS :
//==================================================================

void CustomLookAndFeel::drawLinearSlider (Graphics& g, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, const Slider::SliderStyle style, Slider& slider)
{
    if (slider.isBar())
    {
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.fillRect (x, y, width, height);

        g.setColour (slider.findColour (Slider::trackColourId));
        g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), (float) y + 0.5f, sliderPos - (float) x, (float) height - 1.0f)
                                          : Rectangle<float> ((float) x + 0.5f, sliderPos, (float) width - 1.0f, (float) y + ((float) height - sliderPos)));

        drawLinearSliderOutline (g, x, y, width, height, style, slider);
    }
    else
    {
        auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = jmin (6.0f, slider.isHorizontal() ? (float) height * 0.25f : (float) width * 0.25f);

        Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                 slider.isHorizontal() ? (float) y + (float) height * 0.5f : (float) (height + y));

        Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : (float) y);

        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColour (slider.findColour (Slider::backgroundColourId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : (float) width * 0.5f,
                         slider.isHorizontal() ? (float) height * 0.5f : minSliderPos };

            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : (float) width * 0.5f,
                               slider.isHorizontal() ? (float) height * 0.5f : sliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : (float) width * 0.5f,
                         slider.isHorizontal() ? (float) height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : ((float) x + (float) width * 0.5f);
            auto ky = slider.isHorizontal() ? ((float) y + (float) height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius (slider);

        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        g.setColour (slider.findColour (Slider::trackColourId));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        if (! isTwoVal)
        {
            g.setColour (slider.findColour (Slider::thumbColourId));
            g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre (isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? (float) height : (float) width) * 0.4f);
            auto pointerColour = slider.findColour (Slider::thumbColourId);

            if (slider.isHorizontal())
            {
                drawPointer (g, minSliderPos - sr, jmax (0.0f, (float) y + (float) height * 0.5f - trackWidth * 2.0f), trackWidth * 2.0f, pointerColour, 2);

                drawPointer (g, maxSliderPos - trackWidth, jmin ((float) (y + height) - trackWidth * 2.0f, (float) y + (float) height * 0.5f), trackWidth * 2.0f, pointerColour, 4);
            }
            else
            {
                drawPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth * 2.0f), minSliderPos - trackWidth, trackWidth * 2.0f, pointerColour, 1);

                drawPointer (g, jmin ((float) (x + width) - trackWidth * 2.0f, (float) x + (float) width * 0.5f), maxSliderPos - sr, trackWidth * 2.0f, pointerColour, 3);
            }
        }
    }
}

int CustomLookAndFeel::getSliderThumbRadius (Slider& slider)
{
    return jmin (12, slider.isHorizontal() ? static_cast<int> ((float) slider.getHeight() * 0.5f) : static_cast<int> ((float) slider.getWidth() * 0.5f));
}

void CustomLookAndFeel::drawPointer (Graphics& g, const float x, const float y, const float diameter, const Colour& colour, const int direction) noexcept
{
    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation ((float) direction * MathConstants<float>::halfPi,
                                                 x + diameter * 0.5f,
                                                 y + diameter * 0.5f));
    g.setColour (colour);
    g.fillPath (p);
}

/// ------ combo box ---------------///

void CustomLookAndFeel::drawComboBox (Graphics& g, int width, int height, const bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    bool flatonRight = box.findParentComponentOfClass<FilenameComponent>() != nullptr;
    Rectangle<int> boxBounds (0, 0, width, height);
    auto bounds = boxBounds.toFloat().reduced (0.5f, 0.5f);

    auto baseColour = box.findColour (ComboBox::backgroundColourId).withMultipliedSaturation (box.hasKeyboardFocus (true) ? 1.3f : 0.9f).withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f);

    g.setColour (baseColour);

    if (flatonRight)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize, true, false, true, false);
        g.fillPath (path);
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    Rectangle<int> arrowZone (buttonX + outlineThickness, buttonY + outlineThickness, buttonW - outlineThickness, buttonH - outlineThickness);

    Path path;
    path.addTriangle (arrowZone.getCentreX() - 5.0f, arrowZone.getCentreY() - 2.0f, arrowZone.getCentreX(), arrowZone.getCentreY() + 5.0f, arrowZone.getCentreX() + 5.0f, arrowZone.getCentreY() - 2.0f);

    g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.fillPath (path);

    g.setColour (box.findColour (ComboBox::outlineColourId).withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f));
    float lineThickness = 1.0f;

    if (box.isPopupActive() || box.hasKeyboardFocus (false))
    {
        g.setColour (box.findColour (ComboBox::focusedOutlineColourId));
        lineThickness = 1.5f;
    }

    if (flatonRight)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize, true, false, true, false);
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }

    box.colourChanged();
}

Font CustomLookAndFeel::getComboBoxFont (ComboBox& box)
{
    return FontOptions ("Inter", "Regular", box.getHeight() * 0.75);
}

void CustomLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setFont (getComboBoxFont (box));
    label.setBounds (0, 0, box.getWidth() - 20, box.getHeight());
    box.setJustificationType (juce::Justification::left);
}

// ========= Popup Menu Background: ===========================

void CustomLookAndFeel::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    g.fillAll (background);
    g.setColour (background.overlaidWith (Colour (0x2badd8e6)));

#if ! JUCE_MAC
    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
#endif
}

Font CustomLookAndFeel::getPopupMenuFont()
{
    return getCommonMenuFont();
}

void CustomLookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height, bool, MenuBarComponent& menuBar)
{
    const Colour colour (findColour (ThemeColours::menuBackground));

    Rectangle<int> r (0, 0, width, height);

    g.setColour (colour.contrasting (0.15f));
    g.fillRect (r.removeFromBottom (1));

    g.setGradientFill (ColourGradient::vertical (colour, 0, colour.darker (0.2f), (float) height));
    g.fillRect (r);

    if (menuBar.getName().equalsIgnoreCase ("MainMenu"))
    {
        g.setColour (findColour (ThemeColours::defaultText).withAlpha (menuBar.isEnabled() ? 1.0f : 0.5f));
        String ver = "v" + String (ProjectInfo::versionString);
        g.setFont (getCommonMenuFont());
        int verStrWidth = GlyphArrangement::getStringWidthInt (getCommonMenuFont(), ver);
        g.drawText (ver, width - verStrWidth - 10, 0, verStrWidth, height, Justification::centred);
    }
}

Font CustomLookAndFeel::getMenuBarFont (MenuBarComponent& menuBar, int /*itemIndex*/, const String& /*itemText*/)
{
    return Font (FontOptions (getCommonMenuFont().getTypefaceName(), "Medium", menuBar.getHeight() * 0.65f));
}

//==================================================================
// BUTTON METHODS :
//==================================================================

void CustomLookAndFeel::drawButtonBackground (Graphics& g,
                                              Button& button,
                                              const Colour& backgroundColour,
                                              bool isMouseOverButton,
                                              bool isButtonDown)
{
    auto cornerSize = 3.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 1.0f)
                          .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (isButtonDown || isMouseOverButton)
        baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.05f);

    g.setColour (baseColour);

    auto flatOnLeft = button.isConnectedOnLeft();
    auto flatOnRight = button.isConnectedOnRight();
    auto flatOnTop = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize, ! (flatOnLeft || flatOnTop), ! (flatOnRight || flatOnTop), ! (flatOnLeft || flatOnBottom), ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColour (button.findColour (ComboBox::outlineColourId).withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (button.findColour (ComboBox::outlineColourId).withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

void CustomLookAndFeel::drawButtonText (Graphics& g,
                                        TextButton& button,
                                        bool isMouseOverButton,
                                        bool isButtonDown)
{
    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                     .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.65f);
    const int leftIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent,
                          yIndent,
                          textWidth,
                          button.getHeight() - yIndent * 2,
                          Justification::centred,
                          1,
                          0.95f);
}

Font CustomLookAndFeel::getTextButtonFont (TextButton&, int buttonHeight)
{
    return FontOptions (getCommonMenuFont().getTypefaceName(), "Medium", buttonHeight * 0.65f);
}

// ============ Common Font for Menus ================

Font CustomLookAndFeel::getCommonMenuFont()
{
    return FontOptions ("Inter", "Regular", 18.f);
}

//==================================================================
// TOGGLE BUTTON METHODS :
//==================================================================

void CustomLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto fontSize = jmin (18.0f, (float) button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f, tickWidth, tickWidth, button.getToggleState(), button.isEnabled(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (FontOptions ("Inter", "Regular", fontSize));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10).withTrimmedRight (2),
                      Justification::centredLeft,
                      10);
}

void CustomLookAndFeel::drawTickBox (Graphics& g, Component& component, float x, float y, float w, float h, const bool ticked, const bool isEnabled, const bool shouldDrawButtonAsHighlighted, const bool shouldDrawButtonAsDown)
{
    ignoreUnused (isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    Rectangle<float> tickBounds (x, y, w, h);

    g.setColour (component.findColour (ToggleButton::tickDisabledColourId));
    g.fillRoundedRectangle (tickBounds.reduced (0.5f, 0.5f), 3.0f);

    g.setColour (findColour (ThemeColours::outline));
    g.drawRoundedRectangle (tickBounds.reduced (0.5f, 0.5f), 3.0f, 1.0f);

    if (ticked)
    {
        g.setColour (component.findColour (ToggleButton::tickColourId));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
    }
}

Path CustomLookAndFeel::getTickShape (float height)
{
    static const unsigned char pathData[] = { 110, 109, 32, 210, 202, 64, 126, 183, 148, 64, 108, 39, 244, 247, 64, 245, 76, 124, 64, 108, 178, 131, 27, 65, 246, 76, 252, 64, 108, 175, 242, 4, 65, 246, 76, 252, 64, 108, 236, 5, 68, 65, 0, 0, 160, 180, 108, 240, 150, 90, 65, 21, 136, 52, 63, 108, 48, 59, 16, 65, 0, 0, 32, 65, 108, 32, 210, 202, 64, 126, 183, 148, 64, 99, 101, 0, 0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

void CustomLookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar, int width, int height, double progress, const String& textToShow)
{
    switch (progressBar.getResolvedStyle())
    {
        case ProgressBar::Style::linear:
        {
            auto background = progressBar.findColour (ProgressBar::backgroundColourId);
            auto foreground = progressBar.findColour (ProgressBar::foregroundColourId);

            auto barBounds = progressBar.getLocalBounds().toFloat().reduced (1.0f);

            g.setColour (background);
            g.fillRoundedRectangle (barBounds, 5.0f);

            if (progress >= 0.0f && progress <= 1.0f)
            {
                auto fillBounds = barBounds.withWidth (barBounds.getWidth() * (float) progress);
                g.setColour (foreground);
                g.fillRoundedRectangle (fillBounds, 5.0f);
            }
            else
            {
                // spinning bar..
                g.setColour (background);

                auto stripeWidth = height * 2;
                auto position = static_cast<int> (Time::getMillisecondCounter() / 15) % stripeWidth;

                Path p;

                for (auto x = static_cast<float> (-position); x < (float) (width + stripeWidth); x += (float) stripeWidth)
                    p.addQuadrilateral (x, 0.0f, x + (float) stripeWidth * 0.5f, 0.0f, x, static_cast<float> (height), x - (float) stripeWidth * 0.5f, static_cast<float> (height));

                Image im (Image::ARGB, width, height, true);

                {
                    Graphics g2 (im);
                    g2.setColour (foreground);
                    g2.fillRoundedRectangle (barBounds, 5.0f);
                }

                g.setTiledImageFill (im, 0, 0, 0.85f);
                g.fillPath (p);
            }

            if (textToShow.isNotEmpty())
            {
                g.setColour (Colour::contrasting (background, foreground));
                g.setFont (FontOptions ("Inter", "Regular", (float) height * 0.6f));

                g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
            }

            g.setColour (findColour (ThemeColours::outline));
            g.drawRoundedRectangle (barBounds, 5.0f, 1.0f);

            break;
        }

        case ProgressBar::Style::circular:
            LookAndFeel_V4::drawProgressBar (g, progressBar, width, height, progress, textToShow);
            break;
    }
}

//==================================================================
// DOCUMENT WINDOW METHODS :
//==================================================================

class CustomDocumentWindowButton : public Button
{
public:
    CustomDocumentWindowButton (const String& name, Colour c, const Path& normal, const Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto pathColour = findColour (ThemeColours::controlPanelText).withAlpha (isEnabled() ? 1.0f : 0.5f);

        // g.fillAll (background);

        g.setColour ((! isEnabled() || shouldDrawButtonAsDown) ? colour.withAlpha (0.6f)
                                                               : colour);

        if (shouldDrawButtonAsHighlighted)
        {
            g.fillAll();
        }

        auto& p = getToggleState() ? toggledShape : normalShape;

        auto reducedRect = Justification (Justification::centred)
                               .appliedToRectangle (Rectangle<int> (getHeight(), getHeight()), getLocalBounds())
                               .toFloat()
                               .reduced ((float) getHeight() * 0.3f);

        g.setColour (pathColour);
        g.fillPath (p, p.getTransformToScaleToFit (reducedRect, true));
    }

private:
    Colour colour;
    Path normalShape, toggledShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomDocumentWindowButton)
};

Button* CustomLookAndFeel::createDocumentWindowButton (int buttonType)
{
    Path shape;
    auto crossThickness = 0.15f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment ({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);

        return new CustomDocumentWindowButton ("close", Colour (0xffc42b1c), shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        return new CustomDocumentWindowButton ("minimise", findColour (ThemeColours::defaultFill), shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addRectangle (0.0f, 0.0f, 1.0f, 1.0f);
        PathStrokeType (crossThickness).createStrokedPath (shape, shape);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new CustomDocumentWindowButton ("maximise", findColour (ThemeColours::defaultFill), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

void CustomLookAndFeel::positionDocumentWindowButtons (DocumentWindow&,
                                                       int titleBarX,
                                                       int titleBarY,
                                                       int titleBarW,
                                                       int titleBarH,
                                                       Button* minimiseButton,
                                                       Button* maximiseButton,
                                                       Button* closeButton,
                                                       bool positionTitleBarButtonsOnLeft)
{
    auto buttonW = static_cast<int> (titleBarH * 1.2);

    auto x = positionTitleBarButtonsOnLeft ? titleBarX
                                           : titleBarX + titleBarW - buttonW;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimiseButton, maximiseButton);

    if (maximiseButton != nullptr)
    {
        maximiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != nullptr)
        minimiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
}

void CustomLookAndFeel::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g, int w, int h, int titleSpaceX, int titleSpaceW, const Image* icon, bool drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    auto isActive = window.isActiveWindow();

    const Colour colour (findColour (ThemeColours::componentParentBackground));

    g.setGradientFill (ColourGradient::vertical (colour, 0, colour.darker (0.2f), (float) h));
    g.fillRect (0, 0, w, h);

    g.setColour (colour.contrasting (0.15f));
    g.fillRect (0, h - 1, w, 1);

    Font font (withDefaultMetrics (FontOptions ("Inter", "Semi Bold", (float) h * 0.65f)));
    g.setFont (font);

    auto textW = GlyphArrangement::getStringWidthInt (font, window.getName());
    auto iconW = 0;
    auto iconH = 0;

    if (icon != nullptr)
    {
        iconH = static_cast<int> (font.getHeight());
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW + 4);
    auto textX = drawTitleTextOnLeft ? titleSpaceX
                                     : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH, RectanglePlacement::centred, false);
        textX += iconW + 4;
        textW -= iconW;
    }

    if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
        g.setColour (window.findColour (DocumentWindow::textColourId));
    else
        g.setColour (Colours::whitesmoke);

    g.setOpacity (isActive ? 1.0f : 0.6f);
    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

//==================================================================
// ALERT WINDOW METHODS :
//==================================================================

AlertWindow* CustomLookAndFeel::createAlertWindow (const String& title,
                                                   const String& message,
                                                   const String& button1,
                                                   const String& button2,
                                                   const String& button3,
                                                   MessageBoxIconType iconType,
                                                   int numButtons,
                                                   Component* associatedComponent)
{
    auto boundsOffset = 20;

    auto* aw = LookAndFeel_V2::createAlertWindow (title, message, button1, button2, button3, iconType, numButtons, associatedComponent);

    auto bounds = aw->getBounds();
    bounds = bounds.withSizeKeepingCentre (bounds.getWidth() + boundsOffset, bounds.getHeight() + boundsOffset);
    aw->setBounds (bounds);

    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<TextButton*> (child))
            button->setBounds (button->getBounds() + Point<int> (10, 10));

    return aw;
}

void CustomLookAndFeel::drawAlertBox (Graphics& g, AlertWindow& alert, const Rectangle<int>& textArea, TextLayout& textLayout)
{
    auto cornerSize = 0.0f;

    g.setColour (alert.findColour (AlertWindow::outlineColourId));
    g.drawRoundedRectangle (alert.getLocalBounds().toFloat(), cornerSize, 2.0f);

    auto bounds = alert.getLocalBounds().reduced (1);
    g.reduceClipRegion (bounds);

    g.setColour (alert.findColour (AlertWindow::backgroundColourId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    auto iconSpaceUsed = 0;

    auto iconWidth = 50;
    auto iconSize = jmin (iconWidth, bounds.getHeight());

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    juce::Rectangle<int> iconRect (10, 30, iconSize, iconSize);

    if (alert.getAlertType() != MessageBoxIconType::NoIcon)
    {
        Path icon;
        char character;
        Colour colour;

        if (alert.getAlertType() == MessageBoxIconType::WarningIcon)
        {
            character = '!';

            icon.addTriangle ((float) iconRect.getX() + (float) iconRect.getWidth() * 0.5f,
                              (float) iconRect.getY(),
                              static_cast<float> (iconRect.getRight()),
                              static_cast<float> (iconRect.getBottom()),
                              static_cast<float> (iconRect.getX()),
                              static_cast<float> (iconRect.getBottom()));

            icon = icon.createPathWithRoundedCorners (5.0f);
            colour = findColour (ProcessorColour::SOURCE_COLOUR);
        }
        else
        {
            colour = findColour (ProcessorColour::FILTER_COLOUR);
            character = alert.getAlertType() == MessageBoxIconType::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText (withDefaultMetrics (FontOptions { (float) iconRect.getHeight() * 0.9f, Font::bold }),
                          String::charToString ((juce_wchar) (uint8) character),
                          static_cast<float> (iconRect.getX()),
                          static_cast<float> (iconRect.getY()),
                          static_cast<float> (iconRect.getWidth()),
                          static_cast<float> (iconRect.getHeight()),
                          Justification::centred,
                          1);

        g.setColour (Colour (colour));
        g.fillPath (icon);

        g.setColour (Colours::white);
        ga.draw (g);

        iconSpaceUsed = iconWidth + 20;
    }

    juce::Rectangle<int> alertBounds (bounds.getX() + iconSpaceUsed, 30, bounds.getWidth() - iconSpaceUsed, bounds.getHeight() - getAlertWindowButtonHeight() - 50);

    g.setColour (alert.findColour (AlertWindow::textColourId));
    textLayout.draw (g, alertBounds.toFloat());
}

int CustomLookAndFeel::getAlertWindowButtonHeight() { return 30; }
Font CustomLookAndFeel::getAlertWindowTitleFont() { return FontOptions { "Inter", "Bold", 20.f }; }
Font CustomLookAndFeel::getAlertWindowMessageFont() { return FontOptions { "Inter", "Medium", 18.f }; }
Font CustomLookAndFeel::getAlertWindowFont() { return FontOptions { "Inter", "Medium", 16.f }; }

//==================================================================
// TAB BUTTON METHODS :
//==================================================================

int CustomLookAndFeel::getTabButtonOverlap (int tabDepth)
{
    return 1;
}

int CustomLookAndFeel::getTabButtonSpaceAroundImage()
{
    return 4;
}

int CustomLookAndFeel::getTabButtonBestWidth (TabBarButton& button, int tabDepth)
{
    int width = GlyphArrangement::getStringWidth (Font (FontOptions ((float) tabDepth * 0.7f)), (button.getButtonText().trim()))
                + getTabButtonOverlap (tabDepth) * 2 + 15;

    if (auto* extraComponent = button.getExtraComponent())
        width += button.getTabbedButtonBar().isVertical() ? extraComponent->getHeight()
                                                          : extraComponent->getWidth();

    return jlimit (tabDepth * 2, tabDepth * 8, width);
}

Rectangle<int> CustomLookAndFeel::getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp)
{
    Rectangle<int> extraComp;

    auto orientation = button.getTabbedButtonBar().getOrientation();

    if (button.getExtraComponentPlacement() == TabBarButton::beforeText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:
                extraComp = textArea.removeFromLeft (comp.getWidth());
                break;
            case TabbedButtonBar::TabsAtLeft:
                extraComp = textArea.removeFromBottom (comp.getHeight());
                break;
            case TabbedButtonBar::TabsAtRight:
                extraComp = textArea.removeFromTop (comp.getHeight());
                break;
            default:
                jassertfalse;
                break;
        }
    }
    else
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:
                extraComp = textArea.removeFromRight (comp.getWidth());
                break;
            case TabbedButtonBar::TabsAtLeft:
                extraComp = textArea.removeFromTop (comp.getHeight());
                break;
            case TabbedButtonBar::TabsAtRight:
                extraComp = textArea.removeFromBottom (comp.getHeight());
                break;
            default:
                jassertfalse;
                break;
        }
    }

    return extraComp;
}

void CustomLookAndFeel::createTabButtonShape (TabBarButton& button, Path& p, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    auto activeArea = button.getActiveArea();
    auto w = (float) activeArea.getWidth();
    auto h = (float) activeArea.getHeight();

    auto length = w;
    auto depth = h;

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    const float indent = (float) getTabButtonOverlap ((int) depth);
    const float overhang = 4.0f;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            p.startNewSubPath (w, 0.0f);
            p.lineTo (0.0f, indent);
            p.lineTo (0.0f, h - indent);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (w + overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtRight:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (w, indent);
            p.lineTo (w, h - indent);
            p.lineTo (0.0f, h);
            p.lineTo (-overhang, h + overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtBottom:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (indent, h);
            p.lineTo (w - indent, h);
            p.lineTo (w, 0.0f);
            p.lineTo (w + overhang, -overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtTop:
        default:
            p.startNewSubPath (0.0f, h);
            p.lineTo (indent, 0.0f);
            p.lineTo (w - indent, 0.0f);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (-overhang, h + overhang);
            break;
    }

    p.closeSubPath();

    p = p.createPathWithRoundedCorners (4.0f);
}

void CustomLookAndFeel::fillTabButtonShape (TabBarButton& button, Graphics& g, const Path& path, bool isMouseOver, bool isMouseDown)
{
    auto tabBackground = button.getTabBackgroundColour();
    const bool isFrontTab = button.isFrontTab();

    g.setColour (isFrontTab ? tabBackground
                            : tabBackground.withMultipliedAlpha (0.75f));

    g.fillPath (path);

    g.setColour (button.findColour (isFrontTab ? TabbedButtonBar::frontOutlineColourId
                                               : TabbedButtonBar::tabOutlineColourId,
                                    false)
                     .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    g.strokePath (path, PathStrokeType (isFrontTab ? 1.0f : 0.5f));
}

Font CustomLookAndFeel::getTabButtonFont (TabBarButton&, float height)
{
    return FontOptions ("Inter", "Medium", height * 0.65);
}

void CustomLookAndFeel::drawTabButtonText (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    auto area = button.getTextArea().toFloat();

    auto length = area.getWidth();
    auto depth = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    Font font (getTabButtonFont (button, depth));
    font.setUnderline (button.hasKeyboardFocus (false));

    AffineTransform t;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            t = t.rotated (MathConstants<float>::pi * -0.5f).translated (area.getX(), area.getBottom());
            break;
        case TabbedButtonBar::TabsAtRight:
            t = t.rotated (MathConstants<float>::pi * 0.5f).translated (area.getRight(), area.getY());
            break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom:
            t = t.translated (area.getX(), area.getY());
            break;
        default:
            jassertfalse;
            break;
    }

    Colour col;

    if (button.isFrontTab() && (button.isColourSpecified (TabbedButtonBar::frontTextColourId) || isColourSpecified (TabbedButtonBar::frontTextColourId)))
        col = findColour (TabbedButtonBar::frontTextColourId);
    else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId)
             || isColourSpecified (TabbedButtonBar::tabTextColourId))
        col = findColour (TabbedButtonBar::tabTextColourId);
    else
        col = button.getTabBackgroundColour().contrasting();

    auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

    g.setColour (col.withMultipliedAlpha (alpha));
    g.setFont (font);
    g.addTransform (t);

    g.drawFittedText (button.getButtonText().trim(),
                      0,
                      0,
                      (int) length,
                      (int) depth,
                      Justification::centred,
                      jmax (1, ((int) depth) / 12));
}

void CustomLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    Path tabShape;
    createTabButtonShape (button, tabShape, isMouseOver, isMouseDown);

    auto activeArea = button.getActiveArea();
    tabShape.applyTransform (AffineTransform::translation ((float) activeArea.getX(),
                                                           (float) activeArea.getY()));

    DropShadow (findColour (ThemeColours::dropShadowColour), 8, Point<int> (0, 0)).drawForPath (g, tabShape);

    fillTabButtonShape (button, g, tabShape, isMouseOver, isMouseDown);
    drawTabButtonText (button, g, isMouseOver, isMouseDown);
}

void CustomLookAndFeel::drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics& g) {}

void CustomLookAndFeel::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h)
{
    const float shadowSize = 0.15f;

    Rectangle<int> shadowRect, line;
    ColourGradient gradient (Colours::black.withAlpha (bar.isEnabled() ? 0.15f : 0.08f), 0, 0, Colours::transparentBlack, 0, 0, false);

    switch (bar.getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            gradient.point1.x = (float) w;
            gradient.point2.x = (float) w * (1.0f - shadowSize);
            shadowRect.setBounds ((int) gradient.point2.x, 0, w - (int) gradient.point2.x, h);
            line.setBounds (w - 1, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtRight:
            gradient.point2.x = (float) w * shadowSize;
            shadowRect.setBounds (0, 0, (int) gradient.point2.x, h);
            line.setBounds (0, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtTop:
            gradient.point1.y = (float) h;
            gradient.point2.y = (float) h * (1.0f - shadowSize);
            shadowRect.setBounds (0, (int) gradient.point2.y, w, h - (int) gradient.point2.y);
            line.setBounds (0, h - 1, w, 1);
            break;

        case TabbedButtonBar::TabsAtBottom:
            gradient.point2.y = (float) h * shadowSize;
            shadowRect.setBounds (0, 0, w, (int) gradient.point2.y);
            line.setBounds (0, 0, w, 1);
            break;

        default:
            break;
    }

    g.setGradientFill (gradient);
    g.fillRect (shadowRect.expanded (2, 2));
}

void CustomLookAndFeel::drawCallOutBoxBackground (CallOutBox& box, Graphics& g, const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = { Image::ARGB, box.getWidth(), box.getHeight(), true };
        Graphics g2 (cachedImage);

        DropShadow (Colours::black.withAlpha (0.7f), 8, { 0, 2 }).drawForPath (g2, path);
    }

    g.setColour (Colours::black);
    g.drawImageAt (cachedImage, 0, 0);

    g.setColour (findColour (ThemeColours::componentParentBackground).withAlpha (0.9f));
    g.fillPath (path);

    g.setColour (findColour (ThemeColours::outline).withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (2.0f));
}

void CustomLookAndFeel::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    auto r = header.getLocalBounds();
    auto outlineColour = header.findColour (TableHeaderComponent::outlineColourId);

    g.setColour (outlineColour);
    g.fillRect (r.removeFromBottom (1));

    g.setColour (header.findColour (TableHeaderComponent::backgroundColourId));
    g.fillRect (r);

    g.setColour (outlineColour);

    for (int i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

void CustomLookAndFeel::drawTableHeaderColumn (Graphics& g, TableHeaderComponent& header, const String& columnName, int /*columnId*/, int width, int height, bool isMouseOver, bool isMouseDown, int columnFlags)
{
    auto highlightColour = header.findColour (TableHeaderComponent::highlightColourId);

    if (isMouseDown)
        g.fillAll (highlightColour);
    else if (isMouseOver)
        g.fillAll (highlightColour.withMultipliedAlpha (0.625f));

    Rectangle<int> area (width, height);
    area.reduce (4, 0);

    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        Path sortArrow;
        sortArrow.addTriangle (0.0f, 0.0f, 0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f, 1.0f, 0.0f);

        g.setColour (Colour (0x99000000));
        g.fillPath (sortArrow, sortArrow.getTransformToScaleToFit (area.removeFromRight (height / 2).reduced (2).toFloat(), true));
    }

    g.setColour (header.findColour (TableHeaderComponent::textColourId));
    g.setFont (withDefaultMetrics (FontOptions ("Inter", "Semi Bold", (float) height * 0.5f)));
    g.drawFittedText (columnName, area, Justification::centredLeft, 1);
}

void CustomLookAndFeel::drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border)
{
}

void CustomLookAndFeel::fillResizableWindowBackground (Graphics& g, int /*w*/, int /*h*/, const BorderSize<int>& /*border*/, ResizableWindow& window)
{
    g.setColour (window.getBackgroundColour());
    g.fillRect (window.getLocalBounds().toFloat());
}

void CustomLookAndFeel::drawResizableWindowBorder (juce::Graphics& g, int w, int h, const juce::BorderSize<int>& border, juce::ResizableWindow& window)
{
    g.setColour (findColour (ThemeColours::outline).withAlpha (0.5f));
    g.drawRect (window.getLocalBounds().toFloat());
}
