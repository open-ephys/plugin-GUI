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

#ifndef PSTHEDITOR_H_INCLUDED
#define PSTHEDITOR_H_INCLUDED

#include <EditorHeaders.h>
#include <VisualizerEditorHeaders.h>
#include "PSTHCommon.h"
#include "PSTHCanvas.h"

class PSTHCanvas;
/**

This class serves as a template for creating new editors.

If this were a real editor, this comment section would be used to
describe the editor's structure. In this example, the editor will
have a single button which will set a parameter in the processor.

@see GenericEditor

*/

class PSTHEditor : public VisualizerEditor,	public ComboBox::Listener
{
public:
	PSTHEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
	virtual ~PSTHEditor();
	Visualizer* createNewCanvas();
	void comboBoxChanged(ComboBox* comboBox);
	void updateCanvas();
	void buttonEvent(Button* button);
	bool showSortedUnits, showLFP, showCompactView, showSmooth, showAutoRescale, showMatchRange, showRasters;
	int TTLchannelTrialAlignment;
	int smoothingMS;

	void saveVisualizerParameters(XmlElement* xml);
	void loadVisualizerParameters(XmlElement* xml);
	void visualizationMenu();
private:
	PSTHCanvas* psthCanvas;
	Font font;
	ScopedPointer<ComboBox> hardwareTrialAlignment;
	ScopedPointer<UtilityButton> visibleConditions, saveOptions, clearDisplay, visualizationOptions;
	ScopedPointer<Label> hardwareTrigger;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PSTHEditor);

};


#endif