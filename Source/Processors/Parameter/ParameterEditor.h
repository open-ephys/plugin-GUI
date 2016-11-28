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

class PLUGIN_API ParameterEditor : public Component,
    public Button::Listener,
    public Slider::Listener

{
public:
    ParameterEditor(GenericProcessor* proc, Parameter& p, Font labelFont);
    ~ParameterEditor();


    int desiredWidth;
    int desiredHeight;

    void parentHierarchyChanged();

    void buttonClicked(Button* button);
    void sliderValueChanged(Slider* slider);

	void setChannelSelector(ChannelSelector* ch);

    // for inactivation during acquisition:
    void setEnabled(bool t);
    bool shouldDeactivateDuringAcquisition;
    bool activationState;
    void channelSelectionUI();
private:

    Array<ParameterSlider*> sliderArray;
    Array<ParameterButton*> buttonArray;
    Array<ParameterCheckbox*> checkboxArray;
    Array<int> buttonIdArray;
    Array<int> sliderIdArray;
    Array<int> checkboxIdArray;

    Parameter* parameter;
    GenericProcessor* processor;
    ScopedPointer<ChannelSelector> channelSelector;

    enum
    {
        LEFT,
        MIDDLE,
        RIGHT
    };

};

/**

  Used to edit discrete parameters.

*/

class PLUGIN_API ParameterButton : public Button

{
public:
    ParameterButton(var value, int buttonType, Font labelFont);
	~ParameterButton();

    bool isEnabled;
    //Used to mark if unused, usedByActive, or usedby inactive
    int colorState;
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    void resized();

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

/**

  Used to edit boolean parameters.

*/

class PLUGIN_API ParameterCheckbox : public Button

{
public:
    ParameterCheckbox(bool defaultState);
	~ParameterCheckbox();

    bool isEnabled;

private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);

    ColourGradient selectedGrad;
    ColourGradient selectedOverGrad;
    ColourGradient neutralGrad;
    ColourGradient neutralOverGrad;
    ColourGradient deactivatedGrad;
};

/**

  Used to edit continuous parameters.

*/

class PLUGIN_API ParameterSlider : public Slider

{
public:
    ParameterSlider(float min, float max, float defaultValue, Font f);
	~ParameterSlider();

    bool isEnabled;

private:
    void paint(Graphics& g);//Button(Graphics& g, bool isMouseOver, bool isButtonDown);

    Path makeRotaryPath(double, double, double);

    Font font;
};


#endif  // __PARAMETEREDITOR_H_44537DA9__
