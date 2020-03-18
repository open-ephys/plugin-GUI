/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2013 Open Ephys

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

#include "LfpGradientColourScheme.h"
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"
#include "ShowHideOptionsButton.h"
#include "LfpDisplayOptions.h"
#include "LfpTimescale.h"
#include "LfpDisplay.h"
#include "LfpChannelDisplay.h"
#include "LfpChannelDisplayInfo.h"
#include "EventDisplayInterface.h"
#include "LfpViewport.h"
#include "LfpBitmapPlotterInfo.h"
#include "LfpBitmapPlotter.h"
#include "PerPixelBitmapPlotter.h"
#include "SupersampledBitmapPlotter.h"
#include "LfpChannelColourScheme.h"
#include "LfpMonochromaticColourScheme.h"

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpGradientColourScheme

LfpGradientColourScheme::LfpGradientColourScheme(LfpDisplay * display, LfpDisplayCanvas * canvas)
    : LfpMonochromaticColourScheme(display, canvas)
{
    setName("Gradient");
    
    baseHueLabel->setName("baseHueA");
    baseHueLabel->setText("Hue A", dontSendNotification);
    
    baseHueLabelB = new Label("baseHueB", "Hue B");
    baseHueLabelB->setFont(Font("Default", 13.0f, Font::plain));
    baseHueLabelB->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(baseHueLabelB);
    
    baseHueSliderB = new Slider;
    baseHueSliderB->setRange(0, 1);
    baseHueSliderB->setValue(0.5);
    baseHueSliderB->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    baseHueSliderB->addListener(this);
    addAndMakeVisible(baseHueSliderB);
    
    baseHueSliderB->addMouseListener(this, true);
    
    baseHueB = Colour::fromHSV(0.5, 1.0, 1.0, 1.0);
    swatchHueB = baseHueB;
    
    calculateColourSeriesFromBaseHue();
}

void LfpGradientColourScheme::paint(Graphics &g)
{
    g.setColour(swatchHue);
    g.fillRect(colourSwatchRect);
    
    g.setColour(swatchHueB);
    g.fillRect(colourSwatchRectB);
}

void LfpGradientColourScheme::resized()
{
    numChannelsLabel->setBounds(0, 5, 120, 25);
    numChannelsSelection->setBounds(numChannelsLabel->getRight(),
                                    numChannelsLabel->getY(),
                                    60,
                                    25);
    
    baseHueLabel->setBounds(0, numChannelsLabel->getBottom(), 35, 25);
    baseHueSlider->setBounds(baseHueLabel->getRight(),
                             baseHueLabel->getY(),
                             numChannelsSelection->getRight() - baseHueLabel->getRight() - 20,
                             25);
    
    colourSwatchRect.setBounds(baseHueSlider->getRight() + 5, baseHueSlider->getY() + 5, 15, baseHueSlider->getHeight() - 10);
    
    baseHueLabelB->setBounds(0, baseHueLabel->getBottom(), 35, 25);
    baseHueSliderB->setBounds(baseHueLabelB->getRight(),
                             baseHueLabelB->getY(),
                             numChannelsSelection->getRight() - baseHueLabelB->getRight() - 20,
                             25);
    
    colourSwatchRectB.setBounds(baseHueSliderB->getRight() + 5, baseHueSliderB->getY() + 5, 15, baseHueSliderB->getHeight() - 10);
    
    colourPatternLabel->setBounds(0, baseHueLabelB->getBottom(), 80, 25);
    colourPatternSelection->setBounds(colourPatternLabel->getRight(),
                                      colourPatternLabel->getY(),
                                      numChannelsSelection->getRight() - colourPatternLabel->getRight(),
                                      25);
}

void LfpGradientColourScheme::sliderValueChanged(Slider *sl)
{
    if (sl == baseHueSlider)
    {
        swatchHue = Colour::fromHSV(sl->getValue(), 1, 1, 1);
        repaint(colourSwatchRect);
    }
    else
    {
        swatchHueB = Colour::fromHSV(sl->getValue(), 1, 1, 1);
        repaint(colourSwatchRectB);
    }
}

void LfpGradientColourScheme::mouseUp(const MouseEvent &e)
{
    if (e.originalComponent == baseHueSlider)
    {
        if (swatchHue.getARGB() != baseHue.getARGB())
        {
            baseHue = swatchHue;
            calculateColourSeriesFromBaseHue();
            lfpDisplay->setColors();
            canvas->redraw();
        }
    }
    else
    {
        if (swatchHueB.getARGB() != baseHueB.getARGB())
        {
            baseHueB = swatchHueB;
            calculateColourSeriesFromBaseHue();
            lfpDisplay->setColors();
            canvas->redraw();
        }
    }
}

void LfpGradientColourScheme::calculateColourSeriesFromBaseHue()
{
    colourList.clear();
    
    int coloursToCalculate = numColourChannels;
    
    if (numColourChannels % 2 == 0 && (colourPattern == DOWN_UP || colourPattern == UP_DOWN))
    {
        coloursToCalculate = coloursToCalculate / 2 + 1;
    }
    
    for (int i = 0; i < coloursToCalculate; ++i)
    {
        float hue = (baseHueB.getHue() - baseHue.getHue()) * i / float(coloursToCalculate - 1);
        colourList.add(baseHue.withRotatedHue(hue));
    }
}

