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

#include "FileReader.h"
#include "FileReaderEditor.h"
#include "ScrubberInterface.h"

#include <stdio.h>

ScrubDrawerButton::ScrubDrawerButton(const String &name) : DrawerButton(name) {}

ScrubDrawerButton::~ScrubDrawerButton() {}

void ScrubDrawerButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
	g.setColour(Colour(110, 110, 110));
	if (isMouseOver)
		g.setColour(Colour(210, 210, 210));

	g.drawVerticalLine(3, 0.0f, getHeight());
	g.drawVerticalLine(5, 0.0f, getHeight());
	g.drawVerticalLine(7, 0.0f, getHeight());
}

FileReaderEditor::FileReaderEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)
    , fileReader   (static_cast<FileReader*> (parentNode))
    , recTotalTime              (0)
    , m_isFileDragAndDropActive (false)
    , scrubInterfaceVisible (false)
    , scrubInterfaceAvailable(false)
{

    scrubberInterface = new ScrubberInterface(fileReader);
    scrubberInterface->setBounds(0, 0, 420, 140);
    addChildComponent(scrubberInterface);

    scrubDrawerButton = new ScrubDrawerButton(getNameAndId() + " Scrub Drawer Button");
	scrubDrawerButton->setBounds(4, 40, 10, 78);
    scrubDrawerButton->setToggleState(false, dontSendNotification);
	scrubDrawerButton->addListener(this);
    scrubDrawerButton->setEnabled(false);
	addAndMakeVisible(scrubDrawerButton);

    addPathParameterEditor (Parameter::PROCESSOR_SCOPE, "selected_file", 24, 29);
    addSelectedStreamParameterEditor (Parameter::PROCESSOR_SCOPE, "active_stream", 24, 54);
    addTimeParameterEditor (Parameter::PROCESSOR_SCOPE, "start_time", 24, 79);
    addTimeParameterEditor (Parameter::PROCESSOR_SCOPE, "end_time", 24, 104);

    for (auto& p : {"selected_file", "active_stream", "start_time", "end_time"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(ed->getX(), ed->getY(), 2*ed->getWidth(), ed->getHeight());
    }

    /*
    fileButton = new UtilityButton ("F:", Font ("Small Text", 13, Font::plain));
    fileButton->addListener (this);
    fileButton->setBounds (20, 27, 20, 20);
    addAndMakeVisible (fileButton);

    fileNameLabel = new Label ("FileNameLabel", "No file selected.");
    fileNameLabel->setBounds (50, 25, 140, 20);
    addAndMakeVisible (fileNameLabel);

    recordSelector = new ComboBox (getNameAndId() + " Recording Selector");
    recordSelector->setBounds (50, 50, 120, 20);
    recordSelector->addListener (this);
    addAndMakeVisible (recordSelector);

    currentTime = new DualTimeComponent (this, false);
    currentTime->setBounds (20, 80, 175, 20);
    addAndMakeVisible (currentTime);

    timeLimits = new DualTimeComponent (this,true);
    timeLimits->setBounds (20, 105, 175, 20);
    addAndMakeVisible (timeLimits);
    */

    desiredWidth = 280;

    lastFilePath = CoreServices::getDefaultUserSaveDirectory();

}

FileReaderEditor::~FileReaderEditor()
{
}

/*
void FileReaderEditor::setFile (String file, bool shouldUpdateSignalChain)
{
    if (file.equalsIgnoreCase("default"))
    {
        File executable = File::getSpecialLocation(File::currentApplicationFile);
        
#ifdef __APPLE__
        File defaultFile = executable.getChildFile("Contents/Resources/resources").getChildFile("structure.oebin");
#else
        File defaultFile = executable.getParentDirectory().getChildFile("resources").getChildFile("structure.oebin");
#endif
        
        if (defaultFile.exists())
        {
            file = defaultFile.getFullPathName();
        }
    }
    else {
        
        if (!File(file).existsAsFile())
            return;

        lastFilePath = File(file).getParentDirectory();
    }

    File fileToRead (file);

    LOGD("Setting file to ", file);

    if (fileReader->setFile (fileToRead.getFullPathName()))
    {
        fileNameLabel->setText (fileToRead.getFileName(), dontSendNotification);

        scrubDrawerButton->setVisible(false);
        if ( scrubInterfaceVisible )
            showScrubInterface(false);

        if (fileReader->getCurrentNumTotalSamples() / fileReader->getCurrentSampleRate() > 30.0f)
        {
            scrubDrawerButton->setVisible(true);
            scrubInterfaceAvailable = true;
        }
            
    }
    else
    {
        clearEditor();
    }
    if (shouldUpdateSignalChain)
        CoreServices::updateSignalChain (this);

    repaint();
}
*/

void FileReaderEditor::paintOverChildren (Graphics& g)
{
    /* Draws a frame if a file is currently being dragged over the editor */
    if (m_isFileDragAndDropActive)
    {
        g.setColour (Colours::aqua);
        g.drawRect (getLocalBounds(), 2.f);
    }

    scrubberInterface->paintOverChildren(g);

}

void FileReaderEditor::buttonClicked (Button* button)
{
    /*
    if (! acquisitionIsActive)
    {
        if (button == fileButton)
        {
			StringArray extensions = fileReader->getSupportedExtensions();
			String supportedFormats = String();

			int numExtensions = extensions.size();
			for (int i = 0; i < numExtensions; ++i)
			{
				supportedFormats += ("*." + extensions[i]);
				if (i < numExtensions - 1)
					supportedFormats += ";";
			}

            FileChooser chooseFileReaderFile ("Please select the file you want to load...",
                                              lastFilePath,
                                              supportedFormats);

            if (chooseFileReaderFile.browseForFileToOpen())
            {
                setFile (chooseFileReaderFile.getResult().getFullPathName());
            }
        }
    }
    */

    if (button == scrubDrawerButton) {

        showScrubInterface(!scrubInterfaceVisible);

    }

}

void FileReaderEditor::collapsedStateChanged()
{
    /*
    if (!getCollapsedState())
    {
        scrubberInterface->setVisible(scrubInterfaceAvailable);
        scrubDrawerButton->setVisible(scrubInterfaceAvailable);
    }
    */
    
}

ScrubberInterface* FileReaderEditor::getScrubberInterface()
{
    return scrubberInterface;
}

void FileReaderEditor::showScrubInterface(bool show)
{

    scrubberInterface->setVisible(show);

    int dX = scrubberInterface->getWidth();

    if (scrubInterfaceVisible && !show)
        dX = -dX;
    else if (!scrubInterfaceVisible && !show)
       return;

    desiredWidth += dX;

    /* Move all editor components to the right */
    scrubDrawerButton->setBounds(
        scrubDrawerButton->getX() + dX, scrubDrawerButton->getY(),
        scrubDrawerButton->getWidth(), scrubDrawerButton->getHeight()
    );

    for (auto& p : {"selected_file", "active_stream", "start_time", "end_time"})
    {
        auto* ed = getParameterEditor(p);
        ed->setBounds(ed->getX() + dX, ed->getY(), ed->getWidth(), ed->getHeight());
    }

    /*
    fileButton->setBounds(
        fileButton->getX() + dX, fileButton->getY(),
        fileButton->getWidth(), fileButton->getHeight()
    );

    fileNameLabel->setBounds(
        fileNameLabel->getX() + dX, fileNameLabel->getY(),
        fileNameLabel->getWidth(), fileNameLabel->getHeight()
    );

    recordSelector->setBounds(
        recordSelector->getX() + dX, recordSelector->getY(),
        recordSelector->getWidth(), recordSelector->getHeight()
    );

    currentTime->setBounds(
        currentTime->getX() + dX, currentTime->getY(),
        currentTime->getWidth(), currentTime->getHeight()
    );

    timeLimits->setBounds(
        timeLimits->getX() + dX, timeLimits->getY(),
        timeLimits->getWidth(), timeLimits->getHeight()
    );
    */

    /* Show all scrubber interface components */
    // TOFIX: Don't think this is needed anymore...
    /*
    fullTimeline->setVisible(show);
    zoomTimeline->setVisible(show);
    playbackButton->setVisible(show);
    zoomStartTimeLabel->setVisible(show);
    zoomMiddleTimeLabel->setVisible(show);
    zoomEndTimeLabel->setVisible(show);
    fullStartTimeLabel->setVisible(show);
    fullEndTimeLabel->setVisible(show);
    */

    CoreServices::highlightEditor(this);
    deselect();

    scrubInterfaceVisible = show;

}

bool FileReaderEditor::setPlaybackStartTime (unsigned int ms)
{
    if (ms > timeLimits->getTimeMilliseconds (1))
        return false;

    fileReader->setParameter (1, ms);
    return true;
}

bool FileReaderEditor::setPlaybackStopTime (unsigned int ms)
{
    if ( (ms > recTotalTime) 
         || (ms < timeLimits->getTimeMilliseconds (0)))
        return false;

    fileReader->setParameter (2, ms);
    return true;
}

void FileReaderEditor::setTotalTime (unsigned int ms)
{

    timeLimits->setTimeMilliseconds     (0, 0);
    timeLimits->setTimeMilliseconds     (1, ms);
    currentTime->setTimeMilliseconds    (0, 0);
    currentTime->setTimeMilliseconds    (1, ms);

    recTotalTime = ms;

    updateScrubInterface(true);
}

void FileReaderEditor::updateScrubInterface(bool reset)
{

    /* TODO: Move this to ScrrubberInterface */

    /*
    if (reset) {

        // Reset scrubbing interface to show first 30 seconds of recording

        int ms = timeLimits->getTimeMilliseconds(1);

        int msFrac      = 0;
        int secFrac     = 0;
        int minFrac     = 0;
        int hourFrac    = 0;

        msFrac = ms % 1000;
        ms /= 1000;
        secFrac = ms % 60;
        ms /= 60;
        minFrac = ms % 60;
        ms /= 60;
        hourFrac = ms;

        if (msFrac > 0.5)
            secFrac += 1;

        if (secFrac == 60)
        {
            secFrac = 0;
            minFrac += 1;
        }

        if (minFrac == 60) 
        {
            minFrac = 0;
            hourFrac += 1;
        }
        
        String fullTime = String(hourFrac).paddedLeft ('0', 2) + ":" 
                        + String (minFrac).paddedLeft ('0', 2) + ":" 
                        + String (secFrac).paddedLeft ('0', 2);

        fullEndTimeLabel->setText(fullTime, juce::sendNotificationAsync);

        if (recTotalTime / 1000.0f > 30) {

            // Draws a 30 second interval on full timeline
            zoomMiddleTimeLabel->setText("00:15", juce::sendNotificationAsync);
            zoomEndTimeLabel->setText("00:30", juce::sendNotificationAsync);

            fullTimeline->setIntervalPosition(0, 30);

        }
        else
        {
            String halfZoomTime = String (minFrac).paddedLeft ('0', 2) + ":" + String (secFrac / 2).paddedLeft ('0', 2);
            zoomMiddleTimeLabel->setText(halfZoomTime, juce::sendNotificationAsync);
            String fullZoomTime = String (minFrac).paddedLeft ('0', 2) + ":" + String (secFrac).paddedLeft ('0', 2);
            zoomEndTimeLabel->setText(fullZoomTime, juce::sendNotificationAsync);
        }

    }
    else
    {
        //TODO: Update interval selection interface
    }
    */

}

void FileReaderEditor::setRecording(int index)
{
    //recordSelector->setSelectedItemIndex(index, sendNotification);
}

void FileReaderEditor::timerCallback()
{
    /* TOFIX */
    //setCurrentTime(fileReader->samplesToMilliseconds(fileReader->getCurrentSample()));
}

void FileReaderEditor::setCurrentTime (unsigned int ms)
{
    const MessageManagerLock mmLock;
    currentTime->setTimeMilliseconds (0, ms);
}

void FileReaderEditor::comboBoxChanged (ComboBox* combo)
{
    //fileReader->setParameter (0, combo->getSelectedId() - 1);
    CoreServices::updateSignalChain (this);
}

void FileReaderEditor::populateRecordings (FileSource* source)
{

    ComboBox* activeStreamEditor = (ComboBox*)getParameterEditor("active_stream")->getEditor();

    activeStreamEditor->clear(dontSendNotification);
    for (int i = 0; i < source->getNumRecords(); ++i)
        activeStreamEditor->addItem (source->getRecordName (i), i + 1);

    activeStreamEditor->setSelectedId (1, dontSendNotification);
}

void FileReaderEditor::clearEditor()
{
    /*
    fileNameLabel->setText ("No file selected.", dontSendNotification);
    recordSelector->clear (dontSendNotification);

    timeLimits->setTimeMilliseconds     (0, 0);
    timeLimits->setTimeMilliseconds     (1, 0);
    currentTime->setTimeMilliseconds    (0, 0);
    currentTime->setTimeMilliseconds    (1, 0);

    CoreServices::updateSignalChain(this);
    */

}

void FileReaderEditor::startAcquisition()
{
    /* TOFIX */
    //recordSelector->setEnabled (false);
    //timeLimits->setEnable (false);
}

void FileReaderEditor::stopAcquisition()
{
    /* TOFIX */
    //recordSelector->setEnabled (true);
    //timeLimits->setEnable (true);
}

void FileReaderEditor::saveCustomParametersToXml (XmlElement* xml)
{
    /* TOFIX
    XmlElement* childNode = xml->createNewChildElement ("TIME_LIMITS");
    childNode->setAttribute ("start_time",  (double)timeLimits->getTimeMilliseconds (0));
    childNode->setAttribute ("stop_time",   (double)timeLimits->getTimeMilliseconds (1));
    */
}

void FileReaderEditor::loadCustomParametersFromXml (XmlElement* xml)
{
    /* TOFIX
    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName ("TIME_LIMITS"))
        {
            unsigned int time = 0;

            time = (unsigned int)element->getDoubleAttribute ("start_time");
            setPlaybackStartTime (time);
            timeLimits->setTimeMilliseconds (0, time);

            time = (unsigned int)element->getDoubleAttribute ("stop_time");
            setPlaybackStopTime (time);
            timeLimits->setTimeMilliseconds (1, time);
        }
    }
    */
}

bool FileReaderEditor::isInterestedInFileDrag (const StringArray& files)
{
    if (! acquisitionIsActive)
    {
        const bool isExtensionSupported = fileReader->isFileSupported (files[0]);
        m_isFileDragAndDropActive = true;

        return isExtensionSupported;
    }

    return false;
}

void FileReaderEditor::fileDragExit (const StringArray& files)
{
    m_isFileDragAndDropActive = false;

    repaint();
}

void FileReaderEditor::fileDragEnter (const StringArray& files, int x, int y)
{
    m_isFileDragAndDropActive = true;

    repaint();
}

void FileReaderEditor::filesDropped (const StringArray& files, int x, int y)
{
    //TODO: Use parameter 
    //setFile (files[0]);

    m_isFileDragAndDropActive = false;
    repaint();
}

DualTimeComponent::DualTimeComponent (FileReaderEditor* e, bool editable)
    : editor      (e)
    , isEditable  (editable)
{
    Label* l;
    l = new Label (editor->getNameAndId() + " Start Time");
    l->setBounds (0, 0, 75, 20);
    l->setEditable (isEditable);
    l->setFont (Font("Silkscreen", "Regular", 10));
    if (isEditable)
    {
        l->addListener (this);
        l->setColour (Label::backgroundColourId, Colours::lightgrey);
        l->setColour (Label::outlineColourId,    Colours::black);
    }

    addAndMakeVisible (l);
    timeLabel[0] = l;

    l = new Label(editor->getNameAndId() + " End Time");
    l->setBounds (85, 0, 75, 20);
    l->setEditable (isEditable);
    l->setFont(Font("Silkscreen", "Regular", 10));
    if (isEditable)
    {
        l->addListener (this);
        l->setColour (Label::backgroundColourId,    Colours::lightgrey);
        l->setColour (Label::outlineColourId,       Colours::black);
    }

    addAndMakeVisible(l);
    timeLabel[1] = l;

    setTimeMilliseconds (0, 0);
    setTimeMilliseconds (1, 0);
}

DualTimeComponent::~DualTimeComponent()
{
}

void DualTimeComponent::paint (Graphics& g)
{
    String sep;
    if (isEditable)
        sep = "-";
    else
        sep = "/";
    g.setFont(Font("Silkscreen", "Regular", 10));
    g.setColour (Colours::darkgrey);
    g.drawText (sep, 78, 0, 5, 20, Justification::centred, false);
}

void DualTimeComponent::setTimeMilliseconds (unsigned int index, unsigned int time)
{
    if (index > 1)
        return;

    msTime[index] = time;

    int msFrac      = 0;
    int secFrac     = 0;
    int minFrac     = 0;
    int hourFrac    = 0;

    msFrac = time % 1000;
    time /= 1000;
    secFrac = time % 60;
    time /= 60;
    minFrac = time % 60;
    time /= 60;
    hourFrac = time;

    labelText[index] = String (hourFrac).paddedLeft ('0', 2)
                        + ":" + String (minFrac).paddedLeft ('0', 2)
                        + ":" + String (secFrac).paddedLeft ('0', 2)
                        + "." + String (msFrac).paddedLeft  ('0', 3);

    if (editor->acquisitionIsActive)
    {
        triggerAsyncUpdate();
    }
    else
    {
        timeLabel[index]->setText (labelText[index], dontSendNotification);
    }
}

void DualTimeComponent::handleAsyncUpdate()
{
    timeLabel[0]->setText (labelText[0], dontSendNotification);
}

unsigned int DualTimeComponent::getTimeMilliseconds (unsigned int index) const
{
    if (index > 1)
        return 0;

    return msTime[index];
}

void DualTimeComponent::setEnable (bool enable)
{
    timeLabel[0]->setEnabled (enable);
    timeLabel[1]->setEnabled (enable);
}

void DualTimeComponent::labelTextChanged (Label* label)
{
    const int index = (label == timeLabel[0]) ? 0 : 1;

    StringArray elements;
    elements.addTokens (label->getText(), ":.", String());

    unsigned int time = elements[0].getIntValue();
    time = 60   * time + elements[1].getIntValue();
    time = 60   * time + elements[2].getIntValue();
    time = 1000 * time + elements[3].getIntValue();

    bool res = false;
    if (index == 0)
        res = editor->setPlaybackStartTime (time);
    else
        res = editor->setPlaybackStopTime (time);

    if (res)
        setTimeMilliseconds (index,time);
    else
        setTimeMilliseconds (index, getTimeMilliseconds (index));
}
