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

#include "openEphys_ParametersEditorComponent.h"
#include "../../Utility/openEphys_pluginHelpers.h"


ParametersEditorComponent::ParametersEditorComponent (Project& p)
    : m_project           (p)
    , m_parametersListBox (new ParameterListBox (p))
    , m_propertiesPanel   (new PropertyPanel)
{
    m_parametersListBox->setColour (ListBox::textColourId, Colours::black);
    m_parametersListBox->setRowHeight (30);
    m_parametersListBox->addListener (this);
    addAndMakeVisible (m_parametersListBox);

    addAndMakeVisible (m_propertiesPanel);
}


ParametersEditorComponent::~ParametersEditorComponent()
{
}


void ParametersEditorComponent::paint (Graphics& g)
{
}


void ParametersEditorComponent::resized()
{
    m_parametersListBox->setBounds (0, 40, 300, 200);

    auto localBounds = getLocalBounds();
    m_propertiesPanel->setBounds (localBounds.removeFromRight (250).reduced (0, 16));
}


void ParametersEditorComponent::parameterSelected (Parameter* parameterThatWasSelected, int /*row*/)
{
    if (parameterThatWasSelected == nullptr)
    {
        m_propertiesPanel->clear();
        m_propertiesPanel->setVisible (false);

        return;
    }
    else
    {
        m_propertiesPanel->clear();
        addPropertiesOfParameterToPropertyPanel (parameterThatWasSelected, *m_propertiesPanel);
        m_propertiesPanel->setVisible (true);

        m_project.updateOpenEphysParametersList (m_parametersListBox->getAllParameters());
    }
}
