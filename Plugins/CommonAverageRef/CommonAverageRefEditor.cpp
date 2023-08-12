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

#include "CommonAverageRefEditor.h"

CommonAverageRefEditor::CommonAverageRefEditor (GenericProcessor* parentProcessor)
    : GenericEditor (parentProcessor)
{
    
    desiredWidth = 190;

    addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "affected", 10, 35);
    //getParameterEditor("Affected")->setLayout(ParameterEditor::Layout::nameOnTop);
    addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "reference", 10, 65);
    //getParameterEditor("Reference")->setLayout(ParameterEditor::Layout::nameOnTop);
    addSliderParameterEditor(Parameter::STREAM_SCOPE, "gain", 10, 95);
    //getParameterEditor("gain_level")->setLayout(ParameterEditor::Layout::nameOnTop);
    
}
