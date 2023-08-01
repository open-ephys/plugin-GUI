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
#include "../../Utils/Utils.h"

#include "ScrubberInterface.h"

class FileReader;
class FileReaderEditor;
class DualTimeComponent;
class FileSource;

/**

  User interface for the "FileReader" source node.

  @see SourceNode, FileReaderThread

*/

class ScrubDrawerButton : public DrawerButton
{
public:
	ScrubDrawerButton(const String& name);
	~ScrubDrawerButton();
private:
	void paintButton(Graphics& g, bool isMouseOver, bool isButtonDown) override;
};

class FileReaderEditor  : public GenericEditor
                        , public FileDragAndDropTarget
                        , public ComboBox::Listener
                        , public Button::Listener
                        , public Timer
{
public:

    /** Constructor */
    FileReaderEditor (GenericProcessor* parentNode);

    /** Destructor */
    virtual ~FileReaderEditor();

    /** Respond to button clicks */
    void buttonClicked (Button* button) override;

    /** Hides the file scrubbing interface when the editor is collapsed */
    void collapsedStateChanged();

    /* Methods to handle file drag and drop onto editor */
    bool isInterestedInFileDrag (const StringArray& files)  override;
    void fileDragExit           (const StringArray& files)  override;
    void filesDropped           (const StringArray& files, int x, int y)  override;
    void fileDragEnter          (const StringArray& files, int x, int y)  override;

    /** Draws a border indicating a file is being dragged above the editor */
    void paintOverChildren(Graphics& g) override;

    /** Sets the desired playback start time */
    bool setPlaybackStartTime (unsigned int ms);

    /** Sets the desired playback stop time */
    bool setPlaybackStopTime  (unsigned int ms);

    /** Sets the total time of playback */
    void setTotalTime   (unsigned int ms);

    /** Sets the current timestamp of playback */
    void setCurrentTime (unsigned int ms);

    /** Disables changing settings during playback */
	void startAcquisition() override;

    /** Enables changing settings after playback */
	void stopAcquisition()  override;

    /** Sets the active file */
    // void setFile (String file, bool shouldUpdateSignalChain = true);

    /** Responds to combo box selections */
    void comboBoxChanged (ComboBox* combo);

    /** Populates a combo box with all recordings belonging to the active file source */
    void populateRecordings (FileSource* source);

    /** Sets the current recording to playback */
    void setRecording(int index);

    /** Returns a pointer to the ScrubberInterface */
    ScrubberInterface* getScrubberInterface();

    /** Enables/disables the ScrubDrawerButton */
    void enableScrubDrawer(bool enabled) { scrubDrawerButton->setEnabled(enabled); }

    /** Controls whether or not to show the file scrubbing interface */
    void showScrubInterface(bool show);

    /** Updates the file scrubbing interface when changes have been made */
    void updateScrubInterface(bool reset);

    /** Called whenever the scrubbing interface sliders are adjusted */
    void updatePlaybackTimes();

    /** Save File Reader parameters */
    void saveCustomParametersToXml(XmlElement*) override;

    /** Load File Reader parameters */
    void loadCustomParametersFromXml(XmlElement*) override;

private:
    void clearEditor();

    ScopedPointer<UtilityButton>        fileButton;
    ScopedPointer<Label>                fileNameLabel;
    ScopedPointer<ComboBox>             recordSelector;
    ScopedPointer<DualTimeComponent>    currentTime;
    ScopedPointer<DualTimeComponent>    timeLimits;

    ScopedPointer<ScrubDrawerButton>    scrubDrawerButton;

    ScopedPointer<ScrubberInterface>    scrubberInterface;

    FileReader* fileReader;
    unsigned int recTotalTime;

    bool m_isFileDragAndDropActive;
    bool scrubInterfaceVisible;
    bool scrubInterfaceAvailable;

    File lastFilePath;

    void timerCallback();

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
