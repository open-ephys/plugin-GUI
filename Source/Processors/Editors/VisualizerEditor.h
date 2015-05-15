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

class SelectorButton : public Button
{
public:
    SelectorButton(const String& name);
    ~SelectorButton();
private:
    void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown);
};


/**

  Base class for creating editors with visualizers.

  @see GenericEditor, Visualizer

*/

class VisualizerEditor : public GenericEditor
{
public:
    VisualizerEditor(GenericProcessor*, int, bool useDefaultParameterEditors);
    VisualizerEditor(GenericProcessor*, bool useDefaultParameterEditors);
    ~VisualizerEditor();

    void buttonEvent(Button* button);
	virtual void buttonCallback(Button* button);

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
