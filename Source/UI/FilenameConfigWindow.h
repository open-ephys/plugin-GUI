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

#ifndef __FILENAMECONFIGWINDOW_H_F0BD2DD9__
#define __FILENAMECONFIGWINDOW_H_F0BD2DD9__

#include "../../JuceLibraryCode/JuceHeader.h" 

class FilenameFieldComponent : 
    public Component,
    public Label::Listener,
    public Button::Listener
{
public:

    enum Type 
    {
        PREPEND = 0,
        MAIN,
        APPEND
    };
    const std::vector<std::string> types = { "Prepend", "Main", "Append" };

	enum State
	{
		NONE = 0,
		AUTO,
		CUSTOM,
	};
    const std::vector<std::string> states = { "None", "Auto", "Custom" };

    FilenameFieldComponent(int type, int state, String value);
    ~FilenameFieldComponent() {}

    void labelTextChanged(Label* label);
    void buttonClicked(Button*);

    Type type;
    State state;
    String value;

    int index;

    std::unique_ptr<Label> typeLabel;
    std::unique_ptr<Button> stateButton;
    std::unique_ptr<Label> valueLabel;

};


class FilenameConfigWindow : public Component
{

public:
    
    FilenameConfigWindow(Array<std::shared_ptr<FilenameFieldComponent>> _fields) 
    {
        setSize(360, 100);

        for (int i = 0; i < _fields.size(); i++)
        {
            fields.add(_fields[i]);
            fields.getLast()->setBounds(0,i*getHeight()/3,getWidth(),getHeight()/3);
            addAndMakeVisible(fields.getLast().get());
        }

    }

    ~FilenameConfigWindow() {}

    Array<std::shared_ptr<FilenameFieldComponent>> fields;

    /** Save settings. */
    void saveStateToXml(XmlElement*);

    /** Load settings. */
    void loadStateFromXml(XmlElement*);

};


#endif  // __SPIKEDETECTORCONFIGWINDOW_H_F0BD2DD9__
