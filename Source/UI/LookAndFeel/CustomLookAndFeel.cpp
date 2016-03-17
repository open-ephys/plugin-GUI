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
    silkscreen(new CustomTypeface(silkscreenStream))

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
    else   // default
    {
        return LookAndFeel::getTypefaceForFont(font);
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

    g.fillAll(Colours::lightgrey); //box.findColour (ComboBox::backgroundColourId));

    if (box.isEnabled() && box.hasKeyboardFocus(false))
    {
        g.setColour(Colours::lightgrey); //box.findColour (TextButton::buttonColourId));
        g.drawRect(0, 0, width, height, 2);
    }
    else
    {
        g.setColour(box.findColour(ComboBox::outlineColourId));
        g.drawRect(0, 0, width, height);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    const Colour baseColour(Colours::orange);/*LookAndFeelHelpers::createBaseColour (box.findColour (ComboBox::buttonColourId),
                                                                   box.hasKeyboardFocus (true),
                                                                   false, isButtonDown)
                                .withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f));*/

    juce::LookAndFeel_V1::drawGlassLozenge(g,
                                           buttonX + outlineThickness, buttonY + outlineThickness,
                                           buttonW - outlineThickness * 2.0f, buttonH - outlineThickness * 2.0f,
                                           baseColour, outlineThickness, -1.0f,
                                           true, true, true, true);

    if (box.isEnabled())
    {
        const float arrowX = 0.3f;
        const float arrowH = 0.2f;

        Path p;
        p.addTriangle(buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
                      buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                      buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

        p.addTriangle(buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
                      buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                      buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

        g.setColour(box.findColour(ComboBox::arrowColourId));
        g.fillPath(p);
    }


}
