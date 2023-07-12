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
    
    setDesiredWidth (205);

    addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "Affected", 10, 20 + 15);
    //getParameterEditor("Affected")->setLayout(ParameterEditor::Layout::nameOnTop);
    addSelectedChannelsParameterEditor(Parameter::STREAM_SCOPE, "Reference", 10, 55 + 15);
    //getParameterEditor("Reference")->setLayout(ParameterEditor::Layout::nameOnTop);
    addSliderParameterEditor(Parameter::STREAM_SCOPE, "Gain", 10, 90 + 15);
    //getParameterEditor("gain_level")->setLayout(ParameterEditor::Layout::nameOnTop);
    
}
