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

#ifndef __OPEN_EPHYS_PARAMETERSEDITORCOMPONENT_H__
#define __OPEN_EPHYS_PARAMETERSEDITORCOMPONENT_H__

#include <JuceHeader.h>

// Because of the problems with compiling we include implementation file instead of header
#include "openEphys_ParameterListBox.cpp"


class Project;


class ParametersEditorComponent : public Component
                                , public ParameterListBox::Listener
{
public:
    ParametersEditorComponent (Project& project);
    ~ParametersEditorComponent();

    void paint (Graphics& g)    override;
    void resized()              override;

    // ParameterListBox::Listener
    void parameterSelected (Parameter* parameterThatWasSelected, int row) override;


private:
    Project& m_project;

    ScopedPointer<ParameterListBox> m_parametersListBox;
    ScopedPointer<PropertyPanel>    m_propertiesPanel;
};


#endif //__OPEN_EPHYS_PARAMETERSEDITORCOMPONENT_H__

