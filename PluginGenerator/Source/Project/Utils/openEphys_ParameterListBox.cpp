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

#include "openEphys_ParameterListBox.h"
#include "../../../../Source/Processors/Parameter/Parameter.h"
#include "../jucer_Project.h"

ParameterListBox::ParameterListBox (Project& project)
    : m_project (project)
{
    setModel (this);
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
    // TODO <Kirill A> add different colours for selected and unselected state
    g.setColour (findColour (textColourId));
    // Border
    g.drawRoundedRectangle (0.f, 0.f, (float)width, (float)height, 3.f, 1.f);

    const int sideMargin = 5;
    g.drawText (getItemText (row), sideMargin, 0, width - sideMargin * 2, height, Justification::centredLeft);
}


void ParameterListBox::listBoxItemClicked (int row, const MouseEvent& e)
{
    // Add or delete parameter from popup menu when right clicking
    if (e.mods.isPopupMenu())
    {
        PopupMenu menu;
        menu.addItem (1, "Boolean");
        menu.addItem (2, "Continuous");
        menu.addItem (3, "Discrete");

        const int parameterId = m_parameters.size();
        Parameter* newParameter = nullptr;
        switch (menu.show())
        {
            case 1:
            {
                newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_BOOLEAN, parameterId);
                break;
            }

            case 2:
            {
                newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_CONTINUOUS, parameterId);
                break;
            }

            case 3:
            {
                newParameter = ParameterFactory::createEmptyParameter (Parameter::PARAMETER_TYPE_DISCRETE, parameterId);
                break;
            }

            default:
                return;
        };

        m_parameters.add (newParameter);
        updateContent();
        repaint();

        m_project.updateOpenEphysParametersList (getAllParameters());
    }
    // Show side menu with properties for each parameter
    else
    {
        PopupMenu menu;

        menu.addItem (1, "Edit");
        menu.addSeparator();
        menu.addItem (2, "Delete");
    }
}


String ParameterListBox::getItemText (int row) const noexcept
{
    jassert (row >= 0);

    if (row >= 0 && row < m_parameters.size())
    {
        const auto& parameter = m_parameters[row];
        String prefix = parameter->isBoolean() ? "[B]" : parameter->isContinuous() ? "[C]" : "[D]";
        prefix += " ";

        return prefix + parameter->getName();
    }
    else
    {
        return "---";
    }
}


const OwnedArray<Parameter>& ParameterListBox::getAllParameters() const noexcept
{
    return m_parameters;
}
