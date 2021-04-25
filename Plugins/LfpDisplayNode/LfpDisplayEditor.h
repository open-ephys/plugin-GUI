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

#ifndef __LFPDISPLAYEDITOR_H_Alpha__
#define __LFPDISPLAYEDITOR_H_Alpha__

#include <VisualizerEditorHeaders.h>
#include "LfpDisplayNode.h"
#include "LfpDisplayCanvas.h"

class Visualizer;

namespace LfpViewer {
    
class LfpDisplayNode;
class LfpDisplayCanvas;

class LayoutButton : public Button
{
public:
    LayoutButton(const String& buttonName);
    ~LayoutButton();

private:
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
};

/**

  User interface for the LfpDisplayNode sink.

  @see LfpDisplayNode, LfpDisplayCanvas

*/

class LfpDisplayEditor : public VisualizerEditor
{
public:
    LfpDisplayEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~LfpDisplayEditor();

    // Override VisualEditor behavior to add support for Layout switching
    void buttonClicked(Button* button) override;
    
    // not really being used (yet) ...
    void buttonEvent(Button* button);

    /** Called by the base class VisualizerEditor to display the canvas
        when the user chooses to display one
     
        @see VisualizerEditor::buttonClicked
     */
    Visualizer* createNewCanvas() override;

	void startAcquisition();
	void stopAcquisition();

	void saveVisualizerParameters(XmlElement* xml);
	void loadVisualizerParameters(XmlElement* xml);

    void resized() override;

    void removeBufferForDisplay(int);

private:
        
    LfpDisplayNode* lfpProcessor;

    ScopedPointer<UtilityButton> syncButton;
    
    bool hasNoInputs;

	int defaultSubprocessor;

    ScopedPointer<Label> layoutLabel;
    ScopedPointer<LayoutButton> singleDisplay;
    ScopedPointer<LayoutButton> twoVertDisplay;
    ScopedPointer<LayoutButton> threeVertDisplay;
    ScopedPointer<LayoutButton> twoHoriDisplay;
    ScopedPointer<LayoutButton> threeHoriDisplay;

    SplitLayouts selectedLayout;

    void enableLayoutSelection(bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LfpDisplayEditor);

};
};
#endif  // __LFPDISPLAYEDITOR_H_Alpha__
