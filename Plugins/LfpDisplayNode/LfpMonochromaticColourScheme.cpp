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

#include "LfpMonochromaticColourScheme.h"
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

#include <math.h>

using namespace LfpViewer;

#pragma  mark - LfpMonochromaticColorScheme

LfpMonochromaticColourScheme::LfpMonochromaticColourScheme(LfpDisplay* display, LfpDisplayCanvas* canvas)
    : LfpChannelColourScheme (8, display, canvas)
    , isBlackAndWhite (false)
    , colourPattern (DOWN_UP)
{
    setName("Monochromatic");
    
    numChannelsLabel = new Label("numChannelsLabel", "Num Color Steps");
    numChannelsLabel->setFont(Font("Default", 13.0f, Font::plain));
    numChannelsLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(numChannelsLabel);
    
    StringArray numChannelsSelectionOptions = {"4", "8", "16"};
    numChannelsSelection = new ComboBox("numChannelsSelection");
    numChannelsSelection->addItemList(numChannelsSelectionOptions, 1);
    numChannelsSelection->setEditableText(true);
    numChannelsSelection->addListener(this);
    numChannelsSelection->setSelectedId(2, dontSendNotification);
    addAndMakeVisible(numChannelsSelection);
    
    baseHueLabel = new Label("baseHue", "Hue");
    baseHueLabel->setFont(Font("Default", 13.0f, Font::plain));
    baseHueLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(baseHueLabel);
    
    baseHueSlider = new Slider;
    baseHueSlider->setRange(0, 1);
    baseHueSlider->setValue(0);
    baseHueSlider->setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
    baseHueSlider->addListener(this);
    addAndMakeVisible(baseHueSlider);
    
    baseHueSlider->addMouseListener(this, true);
    
    colourPatternLabel = new Label("colourPatternLabel", "Pattern");
    colourPatternLabel->setFont(Font("Default", 13.0f, Font::plain));
    colourPatternLabel->setColour(Label::textColourId, Colour(100, 100, 100));
    addAndMakeVisible(colourPatternLabel);
    
    StringArray colourPatternSelectionOptions = {"Down", "Up", "Down-Up", "Up-Down"};
    colourPatternSelection = new ComboBox("colourPatternSelection");
    colourPatternSelection->addItemList(colourPatternSelectionOptions, 1);
    colourPatternSelection->setEditableText(false);
    colourPatternSelection->addListener(this);
    colourPatternSelection->setSelectedId(colourPattern + 1, dontSendNotification);
    addAndMakeVisible(colourPatternSelection);
    
    baseHue = Colour::fromHSV(0, 1, 1, 1);
    swatchHue = baseHue;
    
    calculateColourSeriesFromBaseHue();
}

void LfpMonochromaticColourScheme::paint(Graphics &g)
{
    g.setColour(swatchHue);
    g.fillRect(colourSwatchRect);
}

void LfpMonochromaticColourScheme::resized()
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
    
    colourPatternLabel->setBounds(0, baseHueLabel->getBottom(), 80, 25);
    colourPatternSelection->setBounds(colourPatternLabel->getRight(),
                                      colourPatternLabel->getY(),
                                      numChannelsSelection->getRight() - colourPatternLabel->getRight(),
                                      25);
    
}

void LfpMonochromaticColourScheme::sliderValueChanged(Slider *sl)
{
    swatchHue = Colour::fromHSV(sl->getValue(), 1, 1, 1);
    repaint(colourSwatchRect);
}

void LfpMonochromaticColourScheme::comboBoxChanged(ComboBox *cb)
{
    if (cb == numChannelsSelection)
    {
        int numChannelsColourSpread = 0;
        if (cb->getSelectedId())
        {
            numChannelsColourSpread = cb->getText().getIntValue();
        }
        else
        {
            numChannelsColourSpread = cb->getText().getIntValue();
            if (numChannelsColourSpread < 1) numChannelsColourSpread = 1;
            else if (numChannelsColourSpread > 16) numChannelsColourSpread = 16;
            
            cb->setText(String(numChannelsColourSpread), dontSendNotification);
        }
        
        setNumColourSeriesSteps(numChannelsColourSpread);
    }
    else if (cb == colourPatternSelection)
    {
        setColourPattern((ColourPattern)(cb->getSelectedId() - 1));
    }
    calculateColourSeriesFromBaseHue();
    lfpDisplay->setColors();
//    canvas->fullredraw = true;
    canvas->redraw();
}

void LfpMonochromaticColourScheme::mouseUp(const MouseEvent &e)
{
    if (swatchHue.getARGB() != baseHue.getARGB())
    {
        baseHue = swatchHue;
        calculateColourSeriesFromBaseHue();
        lfpDisplay->setColors();
        canvas->redraw();
    }
}

void LfpMonochromaticColourScheme::setBaseHue(Colour base)
{
    baseHue = base;
    calculateColourSeriesFromBaseHue();
}

const Colour LfpMonochromaticColourScheme::getBaseHue() const
{
    return baseHue;
}

void LfpMonochromaticColourScheme::setNumColourSeriesSteps(int numSteps)
{
    numColourChannels = numSteps;
}

int LfpMonochromaticColourScheme::getNumColourSeriesSteps()
{
    return numColourChannels;
}

const Colour LfpMonochromaticColourScheme::getColourForIndex(int index) const
{
    int colourIdx = (int(index/colourGrouping) % numColourChannels);
    
    // adjust for oscillating patterns
    if (colourPattern == DOWN_UP || colourPattern == UP_DOWN)
    {
        int mid = numColourChannels / 2;
        if (colourIdx > mid)
        {
            if (numColourChannels % 2 == 0)
                colourIdx = numColourChannels - colourIdx;
            else
                colourIdx = (numColourChannels - colourIdx) * 2 - 1;
        }
        else if (numColourChannels % 2 != 0)
        {
            colourIdx *= 2;
        }
    }
    
    // invert if the pattern is UP or UP_DOWN
    if (colourPattern == UP)
        colourIdx = (numColourChannels - 1) - colourIdx;
    else if (colourPattern == UP_DOWN)
        colourIdx = (colourList.size() - 1) - colourIdx;
    
    return colourList[colourIdx];
}

void LfpMonochromaticColourScheme::calculateColourSeriesFromBaseHue()
{
    colourList.clear();
    
    int coloursToCalculate = numColourChannels;
    
    if (numColourChannels % 2 == 0 && (colourPattern == DOWN_UP || colourPattern == UP_DOWN))
    {
        coloursToCalculate = coloursToCalculate / 2 + 1;
    }
    
    for (int i = 0; i < coloursToCalculate; ++i)
    {
        float saturation = 1 - (i / float(coloursToCalculate + 1));
        colourList.add(baseHue.withMultipliedSaturation(saturation));
    }
}

