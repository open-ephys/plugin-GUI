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
#include "../PluginManager/OpenEphysPlugin.h"
#include "Parameter.h"
#include <stdio.h>

class ParameterButton;
class ParameterSlider;
class ParameterCheckbox;

class ParameterEditor : public Component
{
public:

    ParameterEditor() { }

    virtual void updateView() = 0;

    void setParameter(Parameter* param_)
    {
        param = param_;
        updateView();
    }

    bool shouldDeactivateDuringAcquisition()
    {
        return param->shouldDeactivateDuringAcquisition();
    }

    const String getParameterName() { return param->getName(); }

protected:
    Parameter* param;
};

class PLUGIN_API IntParameterEditor : public ParameterEditor,
    public Label::Listener
{
public:
    IntParameterEditor(IntParameter* param);
    ~IntParameterEditor() { }

    void labelTextChanged(Label* label);

    virtual void updateView() override;

    virtual void resized();

private:
    ScopedPointer<Label> name;
    ScopedPointer<Label> value;
};


class PLUGIN_API BooleanParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:
    BooleanParameterEditor(BooleanParameter* param);
    ~BooleanParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    virtual void resized();

private:
    ScopedPointer<Label> name;
    ScopedPointer<ToggleButton> value;
};


#endif  // __PARAMETEREDITOR_H_44537DA9__
