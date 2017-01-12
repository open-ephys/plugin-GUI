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

class FileReaderEditor  : public GenericEditor
                        , public FileDragAndDropTarget
                        , public ComboBox::Listener
{
public:
    FileReaderEditor (GenericProcessor* parentNode, bool useDefaultParameterEditors);
    virtual ~FileReaderEditor();

    void paintOverChildren (Graphics& g) override;

    void buttonEvent (Button* button) override;

    void saveCustomParameters (XmlElement*) override;
    void loadCustomParameters (XmlElement*) override;

    // FileDragAndDropTarget methods
    // ============================================
    bool isInterestedInFileDrag (const StringArray& files)  override;
    void fileDragExit           (const StringArray& files)  override;
    void filesDropped           (const StringArray& files, int x, int y)  override;
    void fileDragEnter          (const StringArray& files, int x, int y)  override;

    bool setPlaybackStartTime (unsigned int ms);
    bool setPlaybackStopTime  (unsigned int ms);
    void setTotalTime   (unsigned int ms);
    void setCurrentTime (unsigned int ms);

	void startAcquisition() override;
	void stopAcquisition()  override;

    void setFile (String file);

    void comboBoxChanged (ComboBox* combo);
    void populateRecordings (FileSource* source);


private:
    void clearEditor();


    ScopedPointer<UtilityButton>        fileButton;
    ScopedPointer<Label>                fileNameLabel;
    ScopedPointer<ComboBox>             recordSelector;
    ScopedPointer<DualTimeComponent>    currentTime;
    ScopedPointer<DualTimeComponent>    timeLimits;

    FileReader* fileReader;
    unsigned int recTotalTime;

    bool m_isFileDragAndDropActive;

    File lastFilePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FileReaderEditor);
};


class DualTimeComponent : public Component
                        , public Label::Listener
                        , public AsyncUpdater
{
public:
    DualTimeComponent (FileReaderEditor* e, bool isEditable);
    ~DualTimeComponent();

    void paint (Graphics& g) override;

    void labelTextChanged (Label* label) override;

    void handleAsyncUpdate() override;

    void setEnable(bool enable);

    void setTimeMilliseconds (unsigned int index, unsigned int time);
    unsigned int getTimeMilliseconds (unsigned int index) const;


private:
    ScopedPointer<Label> timeLabel[2];
    String labelText[2];
    unsigned int msTime[2];

    FileReaderEditor* editor;
    bool isEditable;
};



#endif  // __FILEREADEREDITOR_H_D6EC8B48__
