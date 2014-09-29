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

#ifndef __SPLITTEREDITOR_H_33F644A8__
#define __SPLITTEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "GenericEditor.h"

/**

  User interface for the Splitter utility.

  @see Splitter

*/

class SplitterEditor : public GenericEditor
{
public:
    SplitterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~SplitterEditor();

    void buttonEvent(Button* button);

    void switchDest(int);
    void switchDest();

    void switchIO(int i);

    int getPathForEditor(GenericEditor* editor);
    Array<GenericEditor*> getConnectedEditors();


private:

    ImageButton* pipelineSelectorA;
    ImageButton* pipelineSelectorB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplitterEditor);

};


#endif  // __SPLITTEREDITOR_H_33F644A8__
