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

// Hardcode
#ifndef __OPEN_EPHYS_PARAMETERLISTBOX_CPP__
#define __OPEN_EPHYS_PARAMETERLISTBOX_CPP__

#include "openEphys_ParameterListBox.h"
#include "../jucer_Project.h"



ParameterListBox::ParameterListBox (Project& project)
    : m_project (project)
{
    setModel (this);

    updateParametersFromProject();
}


int ParameterListBox::getNumRows()
{
    //const int numParametersInTree = project.getOpenEphysConfigNode().getNumChildren();
    const int numParameters = m_parameters.size();

    // Always have at least 1 free row for parameters
    return jmax (5, numParameters + 1);
}


void ParameterListBox::paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected)
{
    // BG
    g.setColour (findColour (backgroundColourId));
    g.fillRect (0, 0, width, height);

    // Border
    g.setColour (findColour (outlineColourId));
    g.drawRoundedRectangle (0.f, 0.f, (float)width, (float)height, 3.f, 1.f);

    const int sideMargin = 5;
    g.setFont (14.f);
    auto textColourDefault = findColour (textColourId);
    g.setColour (rowIsSelected ? textColourDefault.brighter() : textColourDefault);
    g.drawText (getItemText (row), sideMargin, 0, width - sideMargin * 2, height, Justification::centredLeft);
}


void ParameterListBox::listBoxItemClicked (int row, const MouseEvent& e)
{
    // Add or delete parameter from popup menu when right clicking
    if (e.mods.isPopupMenu())
    {
        const bool isEmptyRow = ! isExistsParameterForRow (row);

        if (isEmptyRow)
        {
            PopupMenu menu;
            menu.addSectionHeader ("Select parameter type");
            menu.addItem (1, "Boolean");
            menu.addItem (2, "Continuous");
            menu.addItem (3, "Discrete");
            menu.addItem (4, "Numerical");

            const int parameterId = m_parameters.size();
            Parameter* newParameter = nullptr;
            // TODO <Kirill A>: Refactor
            switch (menu.show() - 1)
            {
                case Parameter::PARAMETER_TYPE_BOOLEAN:
                {
                    newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_BOOLEAN, parameterId);
                    break;
                }

                case Parameter::PARAMETER_TYPE_CONTINUOUS:
                {
                    newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_CONTINUOUS, parameterId);
                    break;
                }

                case Parameter::PARAMETER_TYPE_DISCRETE:
                {
                    newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_DISCRETE, parameterId);
                    break;
                }

                case Parameter::PARAMETER_TYPE_NUMERICAL:
                {
                    newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_NUMERICAL, parameterId);
                    break;
                }

                default:
                    return;
            };

            m_parameters.add (newParameter);
            newParameter->addListener (this);

            updateContent();
            repaint();

            m_project.updateOpenEphysParametersList (getAllParameters());
        }
        // Parameter already exist for current row
        else
        {
            PopupMenu menu;

            menu.addItem (1, "Delete");

            switch (menu.show())
            {
                case 1:
                {
                    m_parameters.remove (row);
                    updateContent();
                    repaint();

                    m_project.updateOpenEphysParametersList (getAllParameters());
                }

                default:
                    return;
            }
        }
    }
    else
    {
        Parameter* parameterToReturn = (row >= m_parameters.size())
                                            ? nullptr
                                            : m_parameters[row];
        m_listeners.call (&ParameterListBox::Listener::parameterSelected, parameterToReturn, row);
    }
}


bool ParameterListBox::isExistsParameterForRow (int row) const noexcept
{
    jassert (row >= 0);

    return row >= 0 && row < m_parameters.size();
}


String ParameterListBox::getItemText (int row) const noexcept
{
    if (isExistsParameterForRow (row))
    {
        const auto& parameter = m_parameters[row];
        String prefix = parameter->isBoolean() ? "[B]" : parameter->isContinuous() ? "[C]" : "[D]";
        prefix += " ";

        return prefix + parameter->getName();
    }
    else
    {
        return "Right click to add parameter...";
    }
}


void ParameterListBox::parameterValueChanged (Value& valueThatWasChanged)
{
    updateContent();
    repaint();

    m_project.updateOpenEphysParametersList (getAllParameters());
}


const OwnedArray<Parameter>& ParameterListBox::getAllParameters() const noexcept
{
    return m_parameters;
}


void ParameterListBox::updateParametersFromProject()
{
    m_parameters.clear();

    auto pluginConfigNode = m_project.getOpenEphysConfigNode();
    const int numParameters = pluginConfigNode.getNumChildren();

    for (int i = 0; i < numParameters; ++i)
    {
        Parameter* parameter = Parameter::createParameterFromValueTree (pluginConfigNode.getChild (i));
        if (parameter == nullptr)
        {
            jassert ("Can't create parameter from value tree");
            continue;
        }

        parameter->addListener (this);

        m_parameters.add (parameter);
    }

    updateContent();
}

#endif // __OPEN_EPHYS_PARAMETERLISTBOX_CPP__
