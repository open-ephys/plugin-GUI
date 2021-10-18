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
#include "../Editors/GenericEditor.h"

#include "../Editors/PopupChannelSelector.h"

#include "../PluginManager/OpenEphysPlugin.h"
#include "Parameter.h"
#include <stdio.h>


class ParameterEditor : public Component
{
public:

    ParameterEditor(Parameter* param_) : param(param_) { }

    virtual ~ParameterEditor() { }

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
    virtual ~IntParameterEditor() { }

    void labelTextChanged(Label* label);

    virtual void updateView() override;

    virtual void resized();

private:
    ScopedPointer<Label> name;
    ScopedPointer<Label> value;
};

class PLUGIN_API CategoricalParameterEditor : public ParameterEditor,
    public ComboBox::Listener
{
public:
    CategoricalParameterEditor(CategoricalParameter* param);
    virtual ~CategoricalParameterEditor() { }

    void comboBoxChanged(ComboBox* comboBox);

    virtual void updateView() override;

    virtual void resized();

private:
    ScopedPointer<Label> name;
    ScopedPointer<ComboBox> comboBox;
};


class PLUGIN_API BooleanParameterEditor : public ParameterEditor,
    public Button::Listener
{
public:
    BooleanParameterEditor(BooleanParameter* param);
    virtual ~BooleanParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    virtual void resized();

private:
    ScopedPointer<Label> name;
    ScopedPointer<ToggleButton> value;
};

class PLUGIN_API SelectedChannelsParameterEditor : public ParameterEditor,
    public Button::Listener,
    public PopupChannelSelector::Listener
{
public:
    SelectedChannelsParameterEditor(SelectedChannelsParameter* param);
    virtual ~SelectedChannelsParameterEditor() { }

    void buttonClicked(Button* label);

    virtual void updateView() override;

    void channelStateChanged(Array<int> selectedChannels);

    virtual void resized();

private:
    std::unique_ptr<UtilityButton> button;
};


#endif  // __PARAMETEREDITOR_H_44537DA9__
