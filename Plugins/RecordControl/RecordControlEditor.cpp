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

#include "RecordControlEditor.h"
#include "RecordControl.h"
#include <stdio.h>

RecordControlEditor::RecordControlEditor(GenericProcessor* parentNode)
    : GenericEditor(parentNode)
{
    desiredWidth = 170;

    addComboBoxParameterEditor("trigger_type", 10, 20);
    addComboBoxParameterEditor("edge", 10, 50);
    addComboBoxParameterEditor("trigger_bit", 10, 80);
}
