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

#ifndef __SPLITTEREDITOR_H_33F644A8__
#define __SPLITTEREDITOR_H_33F644A8__


#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"
#include "Splitter.h"

class StreamInfoView;
class StreamSelector;

/**

  User interface for the Splitter utility.

  @see Splitter

*/

class SplitterEditor : public GenericEditor,
    public Button::Listener
{
public:
    
    /** Constructor */
    SplitterEditor(GenericProcessor* parentNode);

    /** Destructor */
    virtual ~SplitterEditor();

    /** Respond to clicks on path buttons */
    void buttonClicked(Button* button);

    /** Switch to dest path 0 or 1*/
    void switchDest(int);

    /** Switch to the opposite dest path */
    void switchDest();

    /** Alias for switchDest */
    void switchIO(int i);

    /** Returns the path that leads to a given editor (0 or 1) */
    int getPathForEditor(GenericEditor* editor);

    /** Checks whether a stream should be sent down a particular output path */
    bool checkStream(const DataStream* stream, Splitter::Output output);

    /** Returns all the editors directly downstream of this splitter */
    Array<GenericEditor*> getConnectedEditors();

    /** Called when an output stream is enabled or disabled */
    void streamEnabledStateChanged(uint16 streamId, bool isEnabled, bool isLoading) override;

    /** Updates settings for this editor */
    void updateSettings() override;

private:

    std::unique_ptr<ImageButton> pipelineSelectorA;
    std::unique_ptr<ImageButton> pipelineSelectorB;

    std::unique_ptr<StreamSelector> streamSelectorA;
    std::unique_ptr<StreamSelector> streamSelectorB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SplitterEditor);

};


#endif  // __SPLITTEREDITOR_H_33F644A8__
