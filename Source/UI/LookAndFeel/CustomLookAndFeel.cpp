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

#include "CustomLookAndFeel.h"
#include "../CustomArrowButton.h"

CustomLookAndFeel::CustomLookAndFeel() :

    // heap allocation is necessary here, because otherwise the typefaces are
    // deleted too soon (there's a singleton typefacecache that holds references
    // to them whenever they're used).

    silkscreenStream(BinaryData::silkscreenserialized,
        BinaryData::silkscreenserializedSize,
        false),

    cpmonoExtraLight(Typeface::createSystemTypefaceFor(BinaryData::CPMonoExtraLight_otf,
        BinaryData::CPMonoExtraLight_otfSize)),
    cpmonoLight(Typeface::createSystemTypefaceFor(BinaryData::CPMonoLight_otf,
        BinaryData::CPMonoLight_otfSize)),
    cpmonoPlain(Typeface::createSystemTypefaceFor(BinaryData::CPMonoPlain_otf,
        BinaryData::CPMonoPlain_otfSize)),
    cpmonoBold(Typeface::createSystemTypefaceFor(BinaryData::CPMonoBold_otf,
        BinaryData::CPMonoBold_otfSize)),

    firaCodeLight(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeLight_ttf,
        BinaryData::FiraCodeLight_ttfSize)),
    firaCodeMedium(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeMedium_ttf,
        BinaryData::FiraCodeMedium_ttfSize)),
    firaCodeRetina(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeRetina_ttf,
        BinaryData::FiraCodeRetina_ttfSize)),
    firaCodeRegular(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeRegular_ttf,
        BinaryData::FiraCodeRegular_ttfSize)),
    firaCodeSemiBold(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeSemiBold_ttf,
        BinaryData::FiraCodeSemiBold_ttfSize)),
    firaCodeBold(Typeface::createSystemTypefaceFor(BinaryData::FiraCodeBold_ttf,
        BinaryData::FiraCodeBold_ttfSize)),

    firaSansExtraLight(Typeface::createSystemTypefaceFor(BinaryData::FiraSansExtraLight_ttf,
        BinaryData::FiraSansExtraLight_ttfSize)),
    firaSansRegular(Typeface::createSystemTypefaceFor(BinaryData::FiraSansRegular_ttf,
        BinaryData::FiraSansRegular_ttfSize)),
    firaSansSemiBold(Typeface::createSystemTypefaceFor(BinaryData::FiraSansSemiBold_ttf,
        BinaryData::FiraSansSemiBold_ttfSize)),
    firaSansExtraBold(Typeface::createSystemTypefaceFor(BinaryData::FiraSansExtraBold_ttf,
        BinaryData::FiraSansExtraBold_ttfSize)),

    misoRegular(Typeface::createSystemTypefaceFor(BinaryData::MisoRegular_ttf,
        BinaryData::MisoRegular_ttfSize)),
    misoLight(Typeface::createSystemTypefaceFor(BinaryData::MisoLight_ttf,
        BinaryData::MisoLight_ttfSize)),
    misoBold(Typeface::createSystemTypefaceFor(BinaryData::MisoBold_ttf,
        BinaryData::MisoBold_ttfSize)),

    nimbusSans(Typeface::createSystemTypefaceFor(BinaryData::NimbusSans_otf,
        BinaryData::NimbusSans_otfSize)),
    nordic(Typeface::createSystemTypefaceFor(BinaryData::Nordic_ttf,
        BinaryData::Nordic_ttfSize)),
    ostrich(Typeface::createSystemTypefaceFor(BinaryData::Ostrich_ttf,
        BinaryData::Ostrich_ttfSize)),
    bebasNeue(Typeface::createSystemTypefaceFor(BinaryData::BebasNeue_otf,
        BinaryData::BebasNeue_otfSize))


{

    silkscreen = new CustomTypeface(silkscreenStream);
    
    enum
    {
        PROCESSOR_COLOR = 0x801,
        FILTER_COLOR = 0x802,
        SINK_COLOR = 0x803,
        SOURCE_COLOR = 0x804,
        UTILITY_COLOR = 0x805,
    };

    setColour(PROCESSOR_COLOR, Colour(59, 59, 59));
    setColour(FILTER_COLOR, Colour(255, 89, 0));
    setColour(SINK_COLOR, Colour(255, 149, 0));
    setColour(SOURCE_COLOR, Colour(255, 0, 0));
    setColour(UTILITY_COLOR, Colour(90, 80, 80));

    setColour(PopupMenu::backgroundColourId, Colours::darkgrey);
    setColour(PopupMenu::textColourId, Colours::white);
    setColour(PopupMenu::highlightedBackgroundColourId, Colours::grey);
    setColour(PopupMenu::highlightedTextColourId, Colours::yellow);

}

CustomLookAndFeel::~CustomLookAndFeel() {}

//==============================================================================
// FONT/TYPEFACE METHODS :
//==============================================================================

Typeface::Ptr CustomLookAndFeel::getTypefaceForFont(const Font& font)
{
    String typefaceName = font.getTypefaceName();
    String typefaceStyle = font.getTypefaceStyle();

    if (typefaceName.equalsIgnoreCase("CP Mono"))
    {
        if (typefaceStyle.equalsIgnoreCase("Plain"))
        {
            return cpmonoPlain;
        }
        else if (typefaceStyle.equalsIgnoreCase("Extra Light"))
        {
            return cpmonoExtraLight;
        } 
        else if (typefaceStyle.equalsIgnoreCase("Light"))
        {
            return cpmonoLight;
        }
        else if (typefaceStyle.equalsIgnoreCase("Bold"))
        {
            return cpmonoBold;
        }
        else {
            return cpmonoPlain; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase("Fira Code"))
    {
        if (typefaceStyle.equalsIgnoreCase("Light"))
        {
            return firaCodeLight;
        }
        else if (typefaceStyle.equalsIgnoreCase("Medium"))
        {
            return firaCodeMedium;
        }
        else if (typefaceStyle.equalsIgnoreCase("Regular"))
        {
            return firaCodeRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase("Retina"))
        {
            return firaCodeRetina;
        }
        else if (typefaceStyle.equalsIgnoreCase("SemiBold"))
        {
            return firaCodeSemiBold;
        }
        else if (typefaceStyle.equalsIgnoreCase("Bold"))
        {
            return firaCodeBold;
        }
        else {
            return firaCodeRegular; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase("Fira Sans"))
    {
        if (typefaceStyle.equalsIgnoreCase("Extra Light"))
        {
            return firaSansExtraLight;
        }
        else if (typefaceStyle.equalsIgnoreCase("Regular"))
        {
            return firaSansRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase("SemiBold"))
        {
            return firaSansSemiBold;
        }
        else if (typefaceStyle.equalsIgnoreCase("Extra Bold"))
        {
            return firaSansExtraBold;
        }
        else {
            return firaSansSemiBold; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase("Miso"))
    {
        if (typefaceStyle.equalsIgnoreCase("Regular"))
        {
            return misoRegular;
        }
        else if (typefaceStyle.equalsIgnoreCase("Bold"))
        {
            return misoBold;
        }
        else if (typefaceStyle.equalsIgnoreCase("Light"))
        {
            return misoLight;
        }
        else {
            return misoRegular; // default weight
        }
    }
    else if (typefaceName.equalsIgnoreCase("Nimbus Sans"))
    {
        return nimbusSans;
    }
    else if (typefaceName.equalsIgnoreCase("Silkscreen"))
    {
        return silkscreen;
    }
    else if (typefaceName.equalsIgnoreCase("Ostrich"))
    {
        return ostrich;
    }
    else if (typefaceName.equalsIgnoreCase("Bebas Neue"))
    {
        return bebasNeue;
    }
    else if (typefaceName.equalsIgnoreCase("Nordic"))
    {
        return nordic;
    }
    else   // default
    {
        return firaCodeRetina;
    }

}

//==================================================================
// SCROLL BAR METHODS :
//==================================================================

void CustomLookAndFeel::drawScrollbarButton(Graphics& g,
                                            ScrollBar& scrollbar,
                                            int width, int height,
                                            int buttonDirection,
                                            bool isScrollBarVertical,
                                            bool isMouseOverButton,
                                            bool isButtonDown)
{

    Path p;

    float w1 = 0.25f;
    float w2 = 0.75f;

    if (buttonDirection == 0)
        p.addTriangle(width * 0.5f, height * 0.2f,
                      width * w1, height * 0.7f,
                      width * w2, height * 0.7f);
    else if (buttonDirection == 1)
        p.addTriangle(width * 0.8f, height * 0.5f,
                      width * 0.3f, height * w1,
                      width * 0.3f, height * w2);
    else if (buttonDirection == 2)
        p.addTriangle(width * 0.5f, height * 0.8f,
                      width * w1, height * 0.3f,
                      width * w2, height * 0.3f);
    else if (buttonDirection == 3)
        p.addTriangle(width * 0.2f, height * 0.5f,
                      width * 0.7f, height * w1,
                      width * 0.7f, height * w2);

    if (isButtonDown)
        g.setColour(Colours::white);
    else
        g.setColour(Colours::darkgrey);

    g.fillPath(p);

    if (isMouseOverButton)
        g.strokePath(p, PathStrokeType(1.0f));


}

void CustomLookAndFeel::drawScrollbar(Graphics& g,
                                      ScrollBar& scrollbar,
                                      int x, int y,
                                      int width, int height,
                                      bool isScrollbarVertical,
                                      int thumbStartPosition,
                                      int thumbSize,
                                      bool isMouseOver,
                                      bool isMouseDown)

{

    Path thumbPath;

    const float slotIndent = jmin(width, height) > 15 ? 1.0f : 0.0f;
    const float thumbIndent = slotIndent + 4.0f;
    const float thumbIndentx2 = thumbIndent * 2.0f;

    if (isScrollbarVertical)
    {

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle(x + thumbIndent,
                                          thumbStartPosition + thumbIndent,
                                          width - thumbIndentx2,
                                          thumbSize - thumbIndentx2,
                                          (width - thumbIndentx2) * 0.3f);
    }
    else
    {

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle(thumbStartPosition + thumbIndent,
                                          y + thumbIndent,
                                          thumbSize - thumbIndentx2,
                                          height - thumbIndentx2,
                                          (height - thumbIndentx2) * 0.3f);

    }

    g.setColour(Colours::darkgrey);
    g.fillPath(thumbPath);

}

//==================================================================
// SLIDER METHODS :
//==================================================================


void CustomLookAndFeel::drawLinearSliderThumb(Graphics& g,
                                              int x, int y,
                                              int width, int height,
                                              float sliderPos,
                                              float minSliderPos,
                                              float maxSliderPos,
                                              const Slider::SliderStyle style,
                                              Slider& slider)
{
    const float sliderRadius = (float)(getSliderThumbRadius(slider) - 2);

    Colour knobColour(Colours::darkgrey); //LookAndFeelHelpers::createBaseColour (slider.findColour (Slider::thumbColourId),
    //                                      slider.hasKeyboardFocus (false) && slider.isEnabled(),
    //                                      slider.isMouseOverOrDragging() && slider.isEnabled(),
    //                                      slider.isMouseButtonDown() && slider.isEnabled()));

    const float outlineThickness = slider.isEnabled() ? 2.0f : 0.5f;

    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        float kx, ky;

        if (style == Slider::LinearVertical)
        {
            kx = x + width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = y + height * 0.5f;
        }

        drawSliderKnob(g,
                       kx - sliderRadius,
                       ky - sliderRadius,
                       sliderRadius * 2.0f,
                       knobColour, outlineThickness);
    }
    else
    {
        if (style == Slider::ThreeValueVertical)
        {
            drawSliderKnob(g, x + width * 0.5f - sliderRadius,
                           sliderPos - sliderRadius,
                           sliderRadius * 2.0f,
                           knobColour, outlineThickness);
        }
        else if (style == Slider::ThreeValueHorizontal)
        {
            drawSliderKnob(g,sliderPos - sliderRadius,
                           y + height * 0.5f - sliderRadius,
                           sliderRadius * 2.0f,
                           knobColour, outlineThickness);
        }

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            const float sr = jmin(sliderRadius, width * 0.4f);

            drawGlassPointer(g, jmax(0.0f, x + width * 0.5f - sliderRadius * 2.0f),
                             minSliderPos - sliderRadius,
                             sliderRadius * 2.0f, knobColour, outlineThickness, 1);

            drawGlassPointer(g, jmin(x + width - sliderRadius * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                             sliderRadius * 2.0f, knobColour, outlineThickness, 3);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            const float sr = jmin(sliderRadius, height * 0.4f);

            drawGlassPointer(g, minSliderPos - sr,
                             jmax(0.0f, y + height * 0.5f - sliderRadius * 2.0f),
                             sliderRadius * 2.0f, knobColour, outlineThickness, 2);

            drawGlassPointer(g, maxSliderPos - sliderRadius,
                             jmin(y + height - sliderRadius * 2.0f, y + height * 0.5f),
                             sliderRadius * 2.0f, knobColour, outlineThickness, 4);
        }
    }
}

void CustomLookAndFeel::drawLinearSliderBackground(Graphics& g,
                                                   int x, int y,
                                                   int width, int height,
                                                   float sliderPos,
                                                   float minSliderPos,
                                                   float maxSliderPos,
                                                   const Slider::SliderStyle /*style*/,
                                                   Slider& slider)
{
    const float sliderRadius = (float)(getSliderThumbRadius(slider) - 2);

    Path indent;
    // Path backgroundPath;

    if (slider.isHorizontal())
    {
        const float iy = y + height * 0.5f - sliderRadius * 0.5f;
        const float ih = sliderRadius;

        indent.addRoundedRectangle(x - sliderRadius * 0.5f, iy,
                                   width + sliderRadius, ih,
                                   5.0f);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;
        const float iw = sliderRadius;
        indent.addRoundedRectangle(ix, y - sliderRadius * 0.5f,
                                   iw, height + sliderRadius,
                                   5.0f);
    }

    g.setColour(Colours::darkgrey);
    g.strokePath(indent, PathStrokeType(0.5f));
}

int CustomLookAndFeel::getSliderThumbRadius(Slider& slider)
{
    return jmin(7,
                slider.getHeight() / 2,
                slider.getWidth() / 2) + 2;
}

void CustomLookAndFeel::drawSliderKnob(Graphics& g,
                                       const float x, const float y,
                                       const float diameter,
                                       const Colour& colour,
                                       const float outlineThickness) throw()
{
    if (diameter <= outlineThickness)
        return;

    g.setColour(Colours::darkgrey);

    g.fillEllipse(x, y, diameter, diameter);

    g.setColour(Colours::black);
    g.drawEllipse(x, y, diameter, diameter, outlineThickness);
}

void CustomLookAndFeel::drawGlassPointer(Graphics& g,
                                         const float x, const float y,
                                         const float diameter,
                                         const Colour& colour, const float outlineThickness,
                                         const int direction) throw()
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.startNewSubPath(x + diameter * 0.5f, y);
    p.lineTo(x + diameter, y + diameter * 0.6f);
    p.lineTo(x + diameter, y + diameter);
    p.lineTo(x, y + diameter);
    p.lineTo(x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform(AffineTransform::rotation(direction * (float_Pi * 0.5f), x + diameter * 0.5f, y + diameter * 0.5f));

    {
        ColourGradient cg(Colours::white.overlaidWith(colour.withMultipliedAlpha(0.3f)), 0, y,
                          Colours::white.overlaidWith(colour.withMultipliedAlpha(0.3f)), 0, y + diameter, false);

        cg.addColour(0.4, Colours::white.overlaidWith(colour));

        g.setGradientFill(cg);
        g.fillPath(p);
    }

    ColourGradient cg(Colours::transparentBlack,
                      x + diameter * 0.5f, y + diameter * 0.5f,
                      Colours::black.withAlpha(0.5f * outlineThickness * colour.getFloatAlpha()),
                      x - diameter * 0.2f, y + diameter * 0.5f, true);

    cg.addColour(0.5, Colours::transparentBlack);
    cg.addColour(0.7, Colours::black.withAlpha(0.07f * outlineThickness));

    g.setGradientFill(cg);
    g.fillPath(p);

    g.setColour(Colours::black.withAlpha(0.5f * colour.getFloatAlpha()));
    g.strokePath(p, PathStrokeType(outlineThickness));
}

Button* CustomLookAndFeel::createSliderButton(Slider& s, bool isIncrement)
{
    return new CustomArrowButton(String(), isIncrement ? 0 : 0.5);
}


/// ------ combo box ---------------///


void CustomLookAndFeel::drawComboBox(Graphics& g, int width, int height,
                                     const bool isButtonDown,
                                     int buttonX, int buttonY,
                                     int buttonW, int buttonH,
                                     ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    Rectangle<int> boxBounds (0, 0, width, height);

    g.setColour (Colours::lightgrey);
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

    if (box.isPopupActive() || box.hasKeyboardFocus(false))
    {
        g.setColour(Colours::darkgrey);
        g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.5f);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    Rectangle<int> arrowZone (buttonX + outlineThickness, buttonY + outlineThickness,  
                              buttonW - outlineThickness, buttonH - outlineThickness);
                                
    Path path;
    path.addTriangle(arrowZone.getCentreX() - 5.0f, arrowZone.getCentreY() - 2.0f,
                     arrowZone.getCentreX(), arrowZone.getCentreY() + 5.0f,
                     arrowZone.getCentreX() + 5.0f, arrowZone.getCentreY() - 2.0f);

    g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.fillPath(path);
}

Font CustomLookAndFeel::getComboBoxFont (ComboBox& box)
{
    return Font("Fira Sans", "Regular", box.getHeight() * 0.75f);
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

void CustomLookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height,
                                            bool, MenuBarComponent& menuBar)
{
    const Colour colour (58, 58, 58);

    Rectangle<int> r (width, height);

    g.setColour (colour.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColourGradient (colour, 0, 0, colour.darker (0.08f), 0, (float) height, false));
    g.fillRect (r);

    if(menuBar.getName().equalsIgnoreCase("MainMenu"))
    {
        g.setColour(Colours::lightgrey);
        String ver = "v" + String(ProjectInfo::versionString);
        g.setFont(getCommonMenuFont());
        int verStrWidth = getCommonMenuFont().getStringWidth(ver);
        g.drawText(ver, width - verStrWidth - 10, 0, verStrWidth, height, Justification::centred);
    }
}

Font CustomLookAndFeel::getMenuBarFont (MenuBarComponent& menuBar, int /*itemIndex*/, const String& /*itemText*/)
{
    return Font(getCommonMenuFont().getTypefaceName(), "SemiBold", menuBar.getHeight() * 0.7f);
}

//==================================================================
// BUTTON METHODS :
//==================================================================


void CustomLookAndFeel::drawButtonBackground (Graphics& g,
                                              Button& button,
                                              const Colour& backgroundColour,
                                              bool isMouseOverButton, bool isButtonDown)
{
    auto cornerSize = 3.0f;
    auto bounds = button.getLocalBounds().toFloat();

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.35f);

    if (isButtonDown || isMouseOverButton)
        baseColour = baseColour.contrasting (isButtonDown ? 0.35f : 0.05f);

    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);
        g.setColour(findColour(ComboBox::outlineColourId));
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);
    }

    if (button.hasKeyboardFocus(false))
    {
        g.setColour(Colours::darkgrey);
        g.drawRoundedRectangle(bounds.reduced(0.5f, 0.5f), cornerSize, 1.5f);
    }
}


void CustomLookAndFeel::drawButtonText (Graphics& g,
                                        TextButton& button,
                                        bool isMouseOverButton, bool isButtonDown)
{
    auto textColour = button.getToggleState()
                        ? button.findColour (TextButton::textColourOnId)
                        : button.findColour (TextButton::textColourOffId);

    g.setColour (textColour.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / 2);
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / 2);
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 1);
}

Font CustomLookAndFeel::getTextButtonFont (TextButton&, int buttonHeight)
{
    return Font(getCommonMenuFont().getTypefaceName(), "Regular", buttonHeight * 0.65f);
}

// ============ Common Font for Menus ================

Font CustomLookAndFeel::getCommonMenuFont()
{
    return Font ("Fira Sans", "Regular", 20.f);
}

//==================================================================
// TOGGLE BUTTON METHODS :
//==================================================================

void CustomLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted, 
                                          bool shouldDrawButtonAsDown)
{
    auto fontSize = jmin (18.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 shouldDrawButtonAsHighlighted,
                 shouldDrawButtonAsDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                                             .withTrimmedRight (2),
                      Justification::centredLeft, 10);
}

void CustomLookAndFeel::drawTickBox (Graphics& g, Component& component,
                                     float x, float y, float w, float h,
                                     const bool ticked,
                                     const bool isEnabled,
                                     const bool shouldDrawButtonAsHighlighted,
                                     const bool shouldDrawButtonAsDown)
{
    ignoreUnused (isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    Rectangle<float> tickBounds (x, y, w, h);

    g.setColour (component.findColour (ToggleButton::tickDisabledColourId));
    g.fillRoundedRectangle (tickBounds.reduced(0.5f, 0.5f), 3.0f);

    if (ticked)
    {
        g.setColour (component.findColour (ToggleButton::tickColourId));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
    }
}

Path CustomLookAndFeel::getTickShape (float height)
{
    static const unsigned char pathData[] = { 110,109,32,210,202,64,126,183,148,64,108,39,244,247,64,245,76,124,64,108,178,131,27,65,246,76,252,64,108,175,242,4,65,246,76,252,
        64,108,236,5,68,65,0,0,160,180,108,240,150,90,65,21,136,52,63,108,48,59,16,65,0,0,32,65,108,32,210,202,64,126,183,148,64, 99,101,0,0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

void CustomLookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar, 
                                         int width, int height, 
                                         double progress, const String& textToShow)
{
    auto background = Colour(Colours::lightgrey);
    auto foreground = Colour(Colours::yellow);

    auto barBounds = progressBar.getLocalBounds().toFloat();

    g.setColour (background);
    g.fillRoundedRectangle (barBounds, progressBar.getHeight() * 0.15f);

    Path p;
    p.addRoundedRectangle (barBounds, progressBar.getHeight() * 0.15f);
    g.reduceClipRegion (p);

    barBounds.setWidth (barBounds.getWidth() * (float) progress);
    g.setColour (foreground);
    g.fillRoundedRectangle (barBounds, progressBar.getHeight() * 0.15f);

    if (textToShow.isNotEmpty())
    {
        g.setColour (Colours::black);
        g.setFont (height * 0.7f);

        g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
    }
}

//==================================================================
// DOCUMENT WINDOW METHODS :
//==================================================================

class CustomDocumentWindowButton   : public Button
{
public:
    CustomDocumentWindowButton (const String& name, Colour c, const Path& normal, const Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
    {
    }

    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto background = Colours::darkgrey;

        g.fillAll (background);

        g.setColour ((! isEnabled() || shouldDrawButtonAsDown) ? colour.withAlpha (0.6f)
                                                     : colour);

        if (shouldDrawButtonAsHighlighted)
        {
            g.fillAll();
            g.setColour (background);
        }

        auto& p = getToggleState() ? toggledShape : normalShape;

        auto reducedRect = Justification (Justification::centred)
                              .appliedToRectangle (Rectangle<int> (getHeight(), getHeight()), getLocalBounds())
                              .toFloat()
                              .reduced ((float) getHeight() * 0.3f);

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

        return new CustomDocumentWindowButton ("close", Colours::white, shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        return new CustomDocumentWindowButton ("minimise", Colour (0xffaa8811), shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment ({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new CustomDocumentWindowButton ("maximise", Colour (0xff0A830A), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}


void CustomLookAndFeel::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                                 int w, int h, int titleSpaceX, int titleSpaceW,
                                                 const Image* icon, bool drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    auto isActive = window.isActiveWindow();

    g.setColour (Colours::darkgrey);
    g.fillAll();

    Font font ((float) h * 0.65f, Font::plain);
    g.setFont (font);

    auto textW = font.getStringWidth (window.getName());
    auto iconW = 0;
    auto iconH = 0;

    if (icon != nullptr)
    {
        iconH = static_cast<int> (font.getHeight());
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    auto textX = drawTitleTextOnLeft ? titleSpaceX
                                     : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centred, false);
        textX += iconW;
        textW -= iconW;
    }

    if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
        g.setColour (window.findColour (DocumentWindow::textColourId));
    else
        g.setColour (Colours::whitesmoke);

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

Font CustomLookAndFeel::getAlertWindowTitleFont()      { return { "Fira Sans", "SemiBold", 20.f }; }
Font CustomLookAndFeel::getAlertWindowMessageFont()    { return { "Fira Sans", "Regular", 18.f }; }
Font CustomLookAndFeel::getAlertWindowFont()           { return { "Fira Sans", "Regular", 16.f }; }

