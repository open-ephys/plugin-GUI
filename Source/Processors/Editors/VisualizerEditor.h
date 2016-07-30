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

#ifndef __VISUALIZEREDITOR_H_17E6D78C__
#define __VISUALIZEREDITOR_H_17E6D78C__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"
#include "../Visualization/DataWindow.h"
#include "../Visualization/Visualizer.h"


class DataWindow;
class Visualizer;

/**

  Button for selecting the location of a visualizer.

  @see VisualizerEditor

*/

class PLUGIN_API SelectorButton : public Button
{
public:
    SelectorButton(const String& name);
    ~SelectorButton();
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
};


/**
  @brief
  Base class for creating editors with visualizers.
  
  @details
  Automatically adds buttons (and their handlers) which open the canvas in a window or
  a tab. Just like any other editor, do not override VisualizerEditor::buttonClicked.
  @see GenericEditor, Visualizer
  
  If you must add buttons to your editor, handle them by overiding VisualizerEditor::buttonEvent
  @sa         RHD2000Editor, PCIeRhythm::RHD2000Editor
*/

class PLUGIN_API VisualizerEditor : public GenericEditor
{
public:
    /**
     * @brief      Prefer this constructor to properly "size" the editor widget.
     * @details    Unlike other editors, setting GenericEditor::desiredWidth
     * @code{cpp}
     *   desiredWidth = <width-that-you-need>;
     * @endcode
     * will not work.
     *
     * @param      processor                   The processor
     * @param[in]  desired_width               The desired width
     * @param[in]  useDefaultParameterEditors  ``true`` if you want a _default_ editor.
     */
    VisualizerEditor(GenericProcessor* processor, int desired_width, bool useDefaultParameterEditors);

    VisualizerEditor(GenericProcessor* processor, bool useDefaultParameterEditors);
    ~VisualizerEditor();

	/**
     * @brief      This method handles the button evnets which open visualizer in a tab or window.
     * @warning    Do not override this function unless you call ``VisualizerEditor::buttonClicked``
     *             somewhere!
     */
    void buttonClicked(Button* button);
	
	/**
     * @brief      All additional buttons that you create _for the editor_ should be handled here.
     */
	virtual void buttonEvent(Button* button);

    /**
     * @brief      Creates a new canvas. This is like a factory method and must be defined in your sub-class.
     */
    virtual Visualizer* createNewCanvas() = 0;

    virtual void enable();
    virtual void disable();

    void editorWasClicked();

    void updateVisualizer();

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

    virtual void saveVisualizerParameters(XmlElement* xml);
    virtual void loadVisualizerParameters(XmlElement* xml);


    ScopedPointer<DataWindow> dataWindow;
    ScopedPointer<Visualizer> canvas;

    String tabText;

private:

    void initializeSelectors();
    bool isPlaying;

    SelectorButton* windowSelector;
    SelectorButton* tabSelector;

    int tabIndex;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerEditor);

};



#endif  // __VISUALIZEREDITOR_H_17E6D78C__
