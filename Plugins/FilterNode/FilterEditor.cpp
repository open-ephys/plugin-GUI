/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2021 Open Ephys

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

#include "FilterEditor.h"


FilterEditor::FilterEditor(GenericProcessor* parentNode) : GenericEditor(parentNode)
{
    desiredWidth = 150;

    addTextBoxParameterEditor("low_cut", 10, 22);
    addTextBoxParameterEditor("high_cut", 10, 62);
    addMaskChannelsParameterEditor("Channels", 10, 108);

}
