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
#ifndef __LFPMONOCHROMATICCOLOURSCHEME_H__
#define __LFPMONOCHROMATICCOLOURSCHEME_H__

#include <VisualizerWindowHeaders.h>

#include <vector>
#include <array>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"
#include "LfpChannelColourScheme.h"
namespace LfpViewer {
#pragma  mark - LfpMonochromaticColourScheme -
enum ColourPattern
{
    DOWN = 0,
    UP,
    DOWN_UP,
    UP_DOWN
};
    
class LfpMonochromaticColourScheme : public LfpChannelColourScheme,
    public ComboBox::Listener,
    public Slider::Listener
{
public:
    LfpMonochromaticColourScheme(LfpDisplay*, LfpDisplayCanvas*);
    virtual ~LfpMonochromaticColourScheme() {}
    
    void paint(Graphics &g) override;
    void resized() override;
    
    virtual bool hasConfigurableElements() override { return true; };
    
    void sliderValueChanged(Slider* sl);
    void comboBoxChanged(ComboBox *cb);
    
    /** Catches mouseUp to determine whether the base hue has changed. */
    void mouseUp(const MouseEvent &e) override;
    
    void setBaseHue(Colour base);
    const Colour getBaseHue() const;
    
    void setColourPattern(ColourPattern newPattern) { colourPattern = newPattern; }
    ColourPattern getColourPattern() { return colourPattern; }
    
    void setNumColourSeriesSteps(int numSteps);
    int getNumColourSeriesSteps();
    
    virtual const Colour getColourForIndex(int index) const override;
    
protected:
    bool isBlackAndWhite; // Not used yet
    Colour baseHue;
    Colour swatchHue;
    Array<Colour> colourList;
    
    ColourPattern colourPattern;
    
    ScopedPointer<Label> numChannelsLabel;
    ScopedPointer<ComboBox> numChannelsSelection;
    ScopedPointer<Label> baseHueLabel;
    ScopedPointer<Slider> baseHueSlider;
    ScopedPointer<Label> colourPatternLabel;
    ScopedPointer<ComboBox> colourPatternSelection;
    
    Rectangle<int> colourSwatchRect;
    
    virtual void calculateColourSeriesFromBaseHue();
};
    
}; // namespace
#endif
