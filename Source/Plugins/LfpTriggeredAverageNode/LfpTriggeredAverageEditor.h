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

#ifndef __LfpTriggeredAverageEDITOR_H_3438800D__
#define __LfpTriggeredAverageEDITOR_H_3438800D__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "LfpTriggeredAverageNode.h"
#include "LfpTriggeredAverageCanvas.h"
#include "../Editors/VisualizerEditor.h"

class Visualizer;

/**

  User interface for the LfpDisplayNode sink.

  @see LfpDisplayNode, LfpDisplayCanvas

*/

class LfpTriggeredAverageEditor : public VisualizerEditor
{
public:
    LfpTriggeredAverageEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~LfpTriggeredAverageEditor();

    // not really being used (yet) ...
    void buttonEvent(Button* button);

    Visualizer* createNewCanvas();

private:


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpTriggeredAverageEditor);

};

#endif  // __LfpTriggeredAverageEDITOR_H_3438800D__
