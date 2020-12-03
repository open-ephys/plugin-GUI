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
    // third argument to MIS means don't copy the binary data to make a new stream
    cpmonoExtraLightStream(BinaryData::cpmonoextralightserialized,
                           BinaryData::cpmonoextralightserializedSize,
                           false),
    cpmonoLightStream(BinaryData::cpmonolightserialized,
                      BinaryData::cpmonolightserializedSize,
                      false),
    cpmonoPlainStream(BinaryData::cpmonoplainserialized,
                      BinaryData::cpmonoplainserializedSize,
                      false),
    cpmonoBoldStream(BinaryData::cpmonoboldserialized,
                     BinaryData::cpmonoboldserializedSize,
                     false),
    cpmonoBlackStream(BinaryData::cpmonoblackserialized,
                      BinaryData::cpmonoblackserializedSize,
                      false),
    misoRegularStream(BinaryData::misoserialized,
                      BinaryData::misoserializedSize,
                      false),
    silkscreenStream(BinaryData::silkscreenserialized,
                     BinaryData::silkscreenserializedSize,
                     false),
    // heap allocation is necessary here, because otherwise the typefaces are
    // deleted too soon (there's a singleton typefacecache that holds references
    // to them whenever they're used).
    cpmonoExtraLight(new CustomTypeface(cpmonoExtraLightStream)),
    cpmonoLight(new CustomTypeface(cpmonoLightStream)),
    cpmonoPlain(new CustomTypeface(cpmonoPlainStream)),
    cpmonoBold(new CustomTypeface(cpmonoBoldStream)),
    cpmonoBlack(new CustomTypeface(cpmonoBlackStream)),
    misoRegular(new CustomTypeface(misoRegularStream)),
    silkscreen(new CustomTypeface(silkscreenStream)),
    firasansExtraLight(Typeface::createSystemTypefaceFor(BinaryData::FiraSansExtraLight_ttf, 
                                                         BinaryData::FiraSansExtraLight_ttfSize)),
    firasansRegular(Typeface::createSystemTypefaceFor(BinaryData::FiraSansRegular_ttf, 
                                                      BinaryData::FiraSansRegular_ttfSize)),
    firasansSemiBold(Typeface::createSystemTypefaceFor(BinaryData::FiraSansSemiBold_ttf, 
                                                       BinaryData::FiraSansSemiBold_ttfSize)),
    firasansExtraBold(Typeface::createSystemTypefaceFor(BinaryData::FiraSansExtraBold_ttf, 
                                                        BinaryData::FiraSansExtraBold_ttfSize))

{

    // UNCOMMENT AFTER UPDATE
    // typefaceMap.set(String("Default Extra Light"), cpmonoExtraLight);
    // typefaceMap.set(String("Default Light"), cpmonoLight);
    // typefaceMap.set(String("Default"), cpmonoPlain);
    // typefaceMap.set(String("Default Bold"), cpmonoBold);
    // typefaceMap.set(String("Default Black"), cpmonoBlack);
    // typefaceMap.set(String("Paragraph"), misoRegular);
    // typefaceMap.set(String("Silkscreen"), silkscreen);

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

    // some of these names might be unnecessary, and there may be good ones
    // missing.  adjust as needed
    if (typefaceName.equalsIgnoreCase("Default Extra Light"))
    {
        return cpmonoExtraLight;
    }
    else if (typefaceName.equalsIgnoreCase("Default Light"))
    {
        return cpmonoLight;
    }
    else if (typefaceName.equalsIgnoreCase("Default"))
    {
        return cpmonoPlain;
    }
    else if (typefaceName.equalsIgnoreCase("Default Bold"))
    {
        return cpmonoBold;
    }
    else if (typefaceName.equalsIgnoreCase("Default Black"))
    {
        return cpmonoBlack;
    }
    else if (typefaceName.equalsIgnoreCase("Paragraph"))
    {
        return misoRegular;
    }
    else if (typefaceName.equalsIgnoreCase("Small Text"))
    {
        return silkscreen;
    }
    else if (typefaceName.equalsIgnoreCase("FiraSans Light"))
    {
        return firasansExtraLight;
    }
    else if (typefaceName.equalsIgnoreCase("FiraSans"))
    {
        return firasansRegular;
    }
    else if (typefaceName.equalsIgnoreCase("FiraSans Bold"))
    {
        return firasansSemiBold;
    }
    else if (typefaceName.equalsIgnoreCase("FiraSans Extra Bold"))
    {
        return firasansExtraBold;
    }
    else   // default
    {
        return firasansSemiBold;
    }

    // UNCOMMENT AFTER UPDATE
    // if (typefaceMap.contains(typefaceName))
    //     return typefaceMap[typefaceName];
    // else
    //     return LookAndFeel::getTypefaceForFont(font);
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

        //   backgroundPath.addRoundedRectangle (x - sliderRadius * 0.5f, iy,
        //                              (width + sliderRadius)*minSliderPos, ih,
        //                              5.0f);

        //  g.setColour(Colours::orange);
        //  g.fillPath (backgroundPath);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;
        const float iw = sliderRadius;
        indent.addRoundedRectangle(ix, y - sliderRadius * 0.5f,
                                   iw, height + sliderRadius,
                                   5.0f);

        //   backgroundPath.addRoundedRectangle (ix, y - sliderRadius * 0.5f,
        //                              iw, (height + sliderRadius)*sliderPos,
        //                              5.0f);

        //  g.setColour(Colours::orange);
        //  g.fillPath (backgroundPath);
        //g.fillPath (indent);
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
    return new CustomArrowButton(String::empty, isIncrement ? 0 : 0.5);
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
    return Font(getCommonMenuFont().getTypefaceName(), box.getHeight() * 0.75f, Font::plain);
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
        g.setFont(getPopupMenuFont());
        int verStrWidth = getPopupMenuFont().getStringWidth(ver);
        g.drawText(ver, width - verStrWidth - 10, 0, verStrWidth, height, Justification::centred);
    }
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
    return Font(getCommonMenuFont().getTypefaceName(), buttonHeight * 0.65f, Font::plain);
}

// ============ Common Font for Menus ================

Font CustomLookAndFeel::getCommonMenuFont()
{
    return Font ("FiraSans", 20.f, Font::plain);
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