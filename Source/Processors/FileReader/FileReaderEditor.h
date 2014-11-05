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


#ifndef __FILEREADEREDITOR_H_D6EC8B48__
#define __FILEREADEREDITOR_H_D6EC8B48__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

class FileReader;
class DualTimeComponent;
class FileSource;

/**

  User interface for the "FileReader" source node.

  @see SourceNode, FileReaderThread

*/

class FileReaderEditor : public GenericEditor,
    public ComboBox::Listener

{
public:
    FileReaderEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~FileReaderEditor();

    void buttonEvent(Button* button);

    void setFile(String file);

    void saveCustomParameters(XmlElement*);

    void loadCustomParameters(XmlElement*);

    bool setPlaybackStartTime(unsigned int ms);
    bool setPlaybackStopTime(unsigned int ms);
    void setTotalTime(unsigned int ms);
    void setCurrentTime(unsigned int ms);

    void comboBoxChanged(ComboBox* combo);
    void populateRecordings(FileSource* source);

    void startAcquisition();
    void stopAcquisition();

private:

    ScopedPointer<UtilityButton> fileButton;
    ScopedPointer<Label> fileNameLabel;
    ScopedPointer<ComboBox> recordSelector;
    ScopedPointer<DualTimeComponent> currentTime;
    ScopedPointer<DualTimeComponent> timeLimits;

    FileReader* fileReader;
    unsigned int recTotalTime;

    File lastFilePath;

    void clearEditor();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReaderEditor);

};

class DualTimeComponent : public Component,
	public Label::Listener, public AsyncUpdater
{
public:
    DualTimeComponent(FileReaderEditor* e, bool isEditable);
    ~DualTimeComponent();
    void setTimeMilliseconds(unsigned int index, unsigned int time);
    unsigned int getTimeMilliseconds(unsigned int index);
    void paint(Graphics& g);
    void labelTextChanged(Label* label);
    void setEnable(bool enable);
	void handleAsyncUpdate();

private:
    ScopedPointer<Label> timeLabel[2];
    unsigned int msTime[2];
    FileReaderEditor* editor;
    bool editable;
	String labelText[2];

};



#endif  // __FILEREADEREDITOR_H_D6EC8B48__
