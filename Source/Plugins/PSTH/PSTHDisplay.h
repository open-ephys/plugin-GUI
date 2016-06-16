/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2014 Open Ephys

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

#ifndef PSTHDISPLAY_H_INCLUDED
#define PSTHDISPLAY_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"
#include <EditorHeaders.h>
#include <VisualizerEditorHeaders.h>
#include "PSTHCommon.h"
#include "GenericPlot.h"
#include "PSTHEditor.h"
#include "PSTHCanvas.h"
class PSTHEditor;
class PSTHCanvas;
class GenericPlot;

// this component holds all the individual PSTH plots
class PSTHDisplay : public Component
{
public:
	PSTHDisplay(Viewport* p, PSTHCanvas* c);
	~PSTHDisplay();

	void setAutoRescale(bool state);
	void resized();

	std::vector<GenericPlot*> psthPlots;
	void paint(Graphics& g);
	void refresh();
	void focusOnPlot(int plotIndex);

    PSTHEditor* psthEditor;
	Viewport* viewport;
	PSTHCanvas* canvas;

	juce::Font font;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PSTHDisplay);

};


#endif