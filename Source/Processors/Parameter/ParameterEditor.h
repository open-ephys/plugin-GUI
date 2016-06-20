/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

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

#ifndef __PARAMETEREDITOR_H_44537DA9__
#define __PARAMETEREDITOR_H_44537DA9__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../GenericProcessor/GenericProcessor.h"
#include "../Editors/ChannelSelector.h"
#include "../PluginManager/OpenEphysPlugin.h"
#include "Parameter.h"
#include <stdio.h>

class ParameterButton;
class ParameterSlider;
class ParameterCheckbox;


/**
    Automatically creates an interactive editor for a particular
    parameter.

    @see GenericEditor, GenericProcessor, Parameter
*/
class PLUGIN_API ParameterEditor : public Component
                                 , public Button::Listener
                                 , public Slider::Listener
{
public:
    ParameterEditor (GenericProcessor* proccessor, Parameter& parameter, Font labelFont);

    void parentHierarchyChanged() override;

    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderWhichValueHasChanged) override;

    void setChannelSelector (ChannelSelector* channelSelector);

    // for inactivation during acquisition:
    void setEnabled (bool isEnabled);
    void updateChannelSelectionUI();

    int desiredWidth;
    int desiredHeight;

    bool shouldDeactivateDuringAcquisition;


private:
    bool m_activationState;

    GenericProcessor* m_processor;
    Parameter* m_parameter;
    ScopedPointer<ChannelSelector> m_channelSelector;

    OwnedArray<ParameterSlider>     m_sliderArray;
    OwnedArray<ParameterButton>     m_buttonArray;
    OwnedArray<ParameterCheckbox>   m_checkboxArray;

    Array<int> m_buttonIdArray;
    Array<int> m_sliderIdArray;
    Array<int> m_checkboxIdArray;

    enum
    {
        LEFT,
        MIDDLE,
        RIGHT
    };
};


/** Used to edit discrete parameters.  */
class PLUGIN_API ParameterButton : public Button
{
public:
    ParameterButton (var value, int buttonType, Font labelFont);

    bool isEnabled;
    //Used to mark if unused, usedByActive, or usedby inactive
    int colorState;


private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown) override;

    void resized() override;

    int type;

    Path outlinePath;

    const String valueString;

    Font font;

    ColourGradient selectedGrad;
    ColourGradient selectedOverGrad;
    ColourGradient usedByNonActiveGrad;
    ColourGradient usedByNonActiveOverGrad;
    ColourGradient neutralGrad;
    ColourGradient neutralOverGrad;
    ColourGradient deactivatedGrad;

    enum
    {
        LEFT,
        MIDDLE,
        RIGHT
    };
};


/** Used to edit boolean parameters.  */
class PLUGIN_API ParameterCheckbox : public Button
{
public:
    ParameterCheckbox (bool defaultState);

    bool isEnabled;


private:
    void paintButton (Graphics& g, bool isMouseOver, bool isButtonDown);

    ColourGradient selectedGrad;
    ColourGradient selectedOverGrad;
    ColourGradient neutralGrad;
    ColourGradient neutralOverGrad;
    ColourGradient deactivatedGrad;
};


/** Used to edit continuous parameters.  */
class PLUGIN_API ParameterSlider : public Slider
{
public:
    ParameterSlider (float minValue, float maxValue, float defaultValue, Font f);

    bool isEnabled;


private:
    void paint (Graphics& g);//Button(Graphics& g, bool isMouseOver, bool isButtonDown);

    Path makeRotaryPath (double minValue, double maxValue, double value);

    Font font;
};


#endif  // __PARAMETEREDITOR_H_44537DA9__
