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

#ifndef __LFPDISPLAYEDITOR_H_Alpha__
#define __LFPDISPLAYEDITOR_H_Alpha__

#include "LfpDisplayCanvas.h"
#include "LfpDisplayNode.h"
#include <VisualizerEditorHeaders.h>

class Visualizer;

namespace LfpViewer
{

class LfpDisplayNode;
class LfpDisplayCanvas;

class LayoutButton : public Button
{
public:
    LayoutButton (const String& buttonName);
    ~LayoutButton();

private:
    void paintButton (Graphics&, bool isMouseOverButton, bool isButtonDown) override;
};

/**

  User interface for the LfpDisplayNode sink.

  @see LfpDisplayNode, LfpDisplayCanvas

*/

class LfpDisplayEditor : public VisualizerEditor,
                         public Button::Listener
{
public:
    /** Constructor*/
    LfpDisplayEditor (GenericProcessor*);

    /** Destructor */
    ~LfpDisplayEditor() {}

    // Override VisualEditor behavior to add support for Layout switching
    void buttonClicked (Button* button) override;

    /** Called by the base class VisualizerEditor to display the canvas
        when the user chooses to display one
     
        @see VisualizerEditor::buttonClicked
     */
    Visualizer* createNewCanvas() override;

    /** Tells the LFP Viewer whether the signal chain is loading, to prevent unnecessary redraws*/
    void initialize (bool signalChainIsLoading);

    /** Saves layout type */
    void saveVisualizerEditorParameters (XmlElement* xml) override;

    /** Loads layout type*/
    void loadVisualizerEditorParameters (XmlElement* xml) override;

    /** Sets button locations*/
    void resized() override;

    /** Removes buffers for unused streams (called during updateSettings) */
    void removeBufferForDisplay (int);

private:
    LfpDisplayNode* lfpProcessor;

    std::unique_ptr<UtilityButton> syncButton;

    bool hasNoInputs;

    int defaultSubprocessor;

    std::unique_ptr<Label> layoutLabel;
    std::unique_ptr<LayoutButton> singleDisplay;
    std::unique_ptr<LayoutButton> twoVertDisplay;
    std::unique_ptr<LayoutButton> threeVertDisplay;
    std::unique_ptr<LayoutButton> twoHoriDisplay;
    std::unique_ptr<LayoutButton> threeHoriDisplay;

    SplitLayouts selectedLayout;

    bool signalChainIsLoading;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LfpDisplayEditor);
};
}; // namespace LfpViewer
#endif // __LFPDISPLAYEDITOR_H_Alpha__
