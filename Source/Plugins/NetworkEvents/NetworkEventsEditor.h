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


#ifndef __NETWORKEVENTSEDITOR_H_D6EC8B49__
#define __NETWORKEVENTSEDITOR_H_D6EC8B49__

#include <EditorHeaders.h>

class NetworkEvents;


/**

  User interface for the "FileReader" source node.

  @see SourceNode, FileReaderThread

*/

class NetworkEventsEditor : public GenericEditor,public Label::Listener
{
public:
    NetworkEventsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~NetworkEventsEditor();

    void buttonEvent(Button* button);
	void labelTextChanged(juce::Label *);
	void setLabelColor(juce::Colour color);
private:

	ScopedPointer<UtilityButton> restartConnection;
    ScopedPointer<Label> urlLabel;
	ScopedPointer<Label> labelPort;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkEventsEditor);

};



#endif  // __NETWORKEVENTSEDITOR_H_D6EC8B49__
