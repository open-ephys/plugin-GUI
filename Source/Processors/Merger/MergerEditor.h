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

#ifndef __MERGEREDITOR_H_33F644A8__
#define __MERGEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

/**

  User interface for the Merger utility.

  @see Merger

*/

class MergerEditor : public GenericEditor

{
public:
    
    /** Constructor*/
    MergerEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    
    /** Destructor*/
    virtual ~MergerEditor();

    /** Called whenever the pathway selector button is pressed.*/
    virtual void buttonEvent(Button* button);

    /** Changes the active pathway to 0 or 1 */
    void switchSource(int);
    
    /** Swaps the active pathway*/
    void switchSource();

    /** Changes the active pathway to 0 or 1, and selects the editor */
    void switchIO(int);
    
    /** Called for mouse events in the editor's title bar */
    void mouseDown(const MouseEvent& event);

    /** Returns the pathway (0 or 1) for a particular editor*/
    int getPathForEditor(GenericEditor* editor);

    /** Returns an array of the editors that feed into the merger*/
    Array<GenericEditor*> getConnectedEditors();

private:
    
    String getNameString(GenericProcessor*);
    Array<GenericProcessor*> getSelectableProcessors();
    
    ImageButton* pipelineSelectorA;
    ImageButton* pipelineSelectorB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MergerEditor);

};


#endif  // __MERGEREDITOR_H_33F644A8__
