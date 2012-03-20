/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#ifndef SPIKEDISPLAYEDITOR_H_
#define SPIKEDISPLAYEDITOR_H_

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "../../UI/UIComponent.h"
#include "../../UI/DataViewport.h"
#include "../Visualization/DataWindow.h"
#include "../SpikeDisplayNode.h"
#include "../Visualization/SpikeDisplayCanvas.h"
#include "VisualizerEditor.h"

class Visualizer;

class SpikeDisplayEditor : public VisualizerEditor
{
public:
	SpikeDisplayEditor (GenericProcessor*);
	~SpikeDisplayEditor();

	void buttonCallback (Button* button);

	Visualizer* createNewCanvas();

private:	


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpikeDisplayEditor);

};

#endif  // SPIKEDISPLAYEDITOR_H_
