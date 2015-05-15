/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

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

#ifndef ENGINECONFIGWINDOW_H_INCLUDED
#define ENGINECONFIGWINDOW_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "RecordEngine.h"

class EngineParameterComponent : public Component,
    public Label::Listener, public SettableTooltipClient
{
public:
    EngineParameterComponent(EngineParameter& param);
    ~EngineParameterComponent();

    void paint(Graphics& g);
    void labelTextChanged(Label* lab);

    void saveValue();

private:
    ScopedPointer<Component> control;
    EngineParameter::EngineParameterType type;
    EngineParameter& parameter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineParameterComponent);
};

class EngineConfigComponent : public Component
{
public:
    EngineConfigComponent(RecordEngineManager* man, int height);
    ~EngineConfigComponent();
    void paint(Graphics& g);
    void saveParameters();

private:
    OwnedArray<EngineParameterComponent> parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineConfigComponent);
};

class EngineConfigWindow : public DocumentWindow
{
public:
    EngineConfigWindow(RecordEngineManager* man);
    ~EngineConfigWindow();
    void saveParameters();

private:
    RecordEngineManager* manager;
    void closeButtonPressed();
    ScopedPointer<EngineConfigComponent> ui;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EngineConfigWindow);

};



#endif  // ENGINECONFIGWINDOW_H_INCLUDED
