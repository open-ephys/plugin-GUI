/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

/*
This header included all the base classes for data management in Visualizer Windows.
Must be included on Visualizer windows/canvas source files. Note that the specific
graphic representations must be coded using standard Juce methods.
*/

#include "../../JuceLibraryCode/JuceHeader.h"
#include "../../Source/Processors/Editors/GenericEditor.h"
#include "../../Source/Processors/Events/Spike.h"
#include "../../Source/Processors/Visualization/InteractivePlot.h"
#include "../../Source/Processors/Visualization/Visualizer.h"
#include "../../Source/TestableExport.h"
