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

#include "FileReaderEditor.h"

#include "FileReader.h"

#include <stdio.h>

ScrubDrawerButton::ScrubDrawerButton(const String &name) : DrawerButton(name) {}

ScrubDrawerButton::~ScrubDrawerButton() {}

FullTimeline::FullTimeline(FileReader* fr) 
{

    fileReader = fr;

    startTimer(50);
}

FullTimeline::~FullTimeline() {}

void FullTimeline::timerCallback() {
    repaint();
}

void FullTimeline::paint(Graphics& g) 
{

    int tickHeight = 4;
    int borderThickness = 1;

    g.setColour(Colours::black);

    int numTicks = 5;

    for (int i = 0; i < numTicks; i++) {

        float dX = float(i) / numTicks * 450;

        Path tick;
        tick.startNewSubPath(dX, this->getHeight() - tickHeight);
        tick.lineTo(dX, this->getHeight());
        g.strokePath(tick, PathStrokeType(1.0));
    }

	g.fillRect(0, 0, this->getWidth(), this->getHeight()-tickHeight);
	g.setColour(Colours::white);
	g.fillRect(borderThickness, borderThickness, this->getWidth() - 2*borderThickness, this->getHeight() - 2*borderThickness - tickHeight);

    /* Draw a colored vertical bar for each event */
    int64 totalSamples = fileReader->getCurrentNumTotalSamples();

    for (auto info : fileReader->getActiveEventInfo())
    {
        for (int i = 0; i < info.timestamps.size(); i++) {
        
            int64 ts = info.timestamps[i];
            int16 state = info.channelStates[i];

            if (state == 1)
            {
                float timelinePos = ts / float(totalSamples) * getWidth();
                g.setColour(static_cast<FileReaderEditor*>(fileReader->getEditor())->channelColours[info.channels[i]]);
                g.setOpacity(1.0f);
                g.fillRoundedRectangle(timelinePos, 0, 1, this->getHeight() - tickHeight, 0.2);

            }

        }

    }

    /* Draw the 30-second interval */
    g.setColour(Colour(0,0,0));
    g.setOpacity(0.8f);

    if (intervalStartPosition < 0)
        return;

    g.fillRoundedRectangle(intervalStartPosition, 0, 2, this->getHeight(), 2);
    g.fillRoundedRectangle(intervalStartPosition + intervalWidth, 0, 2, this->getHeight(), 2);

    /* Draw the current playback position */
    float timelinePos = (float)fileReader->getCurrentSample() / fileReader->getCurrentNumTotalSamples() * getWidth();

    g.setOpacity(1.0f);
    g.fillRoundedRectangle(timelinePos, 0, 1, this->getHeight(), 0.2);
	
}

void FullTimeline::setIntervalPosition(int min, int max) 
{

    int totalSamples = fileReader->getCurrentNumTotalSamples();
    float sampleRate = fileReader->getCurrentSampleRate();

    float totalTimeInSeconds = float(totalSamples) / sampleRate; 

    intervalStartPosition = min;
    intervalWidth = 30.0f / totalTimeInSeconds * float(getWidth());
}

void FullTimeline::mouseDown(const MouseEvent& event) 
{

    if (event.x >= intervalStartPosition && event.x <= intervalStartPosition + intervalWidth) {
        intervalIsSelected = true;
    }
}

void FullTimeline::mouseDrag(const MouseEvent & event) 
{

    if (intervalIsSelected) {
        if (event.x >= intervalWidth / 2 && event.x < getWidth() - intervalWidth / 2)
            intervalStartPosition = event.x - intervalWidth / 2;
    }

    repaint();
    static_cast<FileReaderEditor*>(fileReader->getEditor())->updateZoomTimeLabels();
    fileReader->getEditor()->repaint();
    
}

void FullTimeline::mouseUp(const MouseEvent& event) 
{

    intervalIsSelected = false;

    static_cast<FileReaderEditor*>(fileReader->getEditor())->updatePlaybackTimes();
    
}

int FullTimeline::getStartInterval() 
{
    return intervalStartPosition;
}

int FullTimeline::getIntervalWidth() 
{
    return intervalWidth;
}

ZoomTimeline::ZoomTimeline(FileReader* fr) 
{
    fileReader = fr; 
    sliderWidth = 8;
    widthInSeconds = 30;

    startTimer(50);
}

ZoomTimeline::~ZoomTimeline() {}

void ZoomTimeline::timerCallback()
{
    repaint();
}

void ZoomTimeline::updatePlaybackRegion(int min, int max) 
{
    /* Default zoom slider region to first 10s */
    leftSliderPosition = 0;
    rightSliderPosition = ( getWidth() - sliderWidth )  / 10.0f;
}

int ZoomTimeline::getStartInterval()
{
    return leftSliderPosition;
}

int ZoomTimeline::getIntervalDurationInSeconds()
{
    /* Get fraction of interval width and convert to nearest second */
    return round ( float( rightSliderPosition + sliderWidth - leftSliderPosition) / getWidth() * widthInSeconds );

}

void ZoomTimeline::paint(Graphics& g) 
{
    int tickHeight = 4;
    int borderThickness = 1;

    g.setColour(Colours::black);

    int numTicks = 7;

    for (int i = 0; i < numTicks; i++) {

        float dX = float(i) / numTicks * 420;

        Path tick;
        tick.startNewSubPath(dX,0);
        tick.lineTo(dX, tickHeight);
        g.strokePath(tick, PathStrokeType(1.0));
    }

    g.fillRect(0, tickHeight, this->getWidth(), this->getHeight()-tickHeight);
	g.setColour(Colours::white);
	g.fillRect(borderThickness, tickHeight + borderThickness, this->getWidth() - 2*borderThickness, this->getHeight() - 2*borderThickness - tickHeight);

    /* Draw a colored vertical bar for each event */
    Array<EventInfo> eventInfo = fileReader->getActiveEventInfo();
    int64 totalSamples = fileReader->getCurrentNumTotalSamples();

    int intervalStartPos = static_cast<FileReaderEditor*>(fileReader->getEditor())->getFullTimelineStartPosition();

    int startTimestamp = int(float(intervalStartPos) / float(getWidth()) * float(totalSamples)); 
    int stopTimestamp = startTimestamp + 30 * fileReader->getCurrentSampleRate();

    for (auto info : eventInfo) 
    {

        for (int i = 0; i < info.timestamps.size(); i++) 
        {
            
            int64 ts = info.timestamps[i];
            int16 state = info.channelStates[i];

            if (ts >= startTimestamp && ts <= stopTimestamp)
            {
                float timelinePos = (ts - startTimestamp) / float(stopTimestamp - startTimestamp) * getWidth();
                g.setColour(static_cast<FileReaderEditor*>(fileReader->getEditor())->channelColours[info.channels[i]]);
                g.setOpacity(1.0f);
                g.fillRect(int(timelinePos), tickHeight, 1, this->getHeight() - tickHeight);
            }

        }

    }
 
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(leftSliderPosition, 0, sliderWidth, this->getHeight(), 2);
    g.setColour(Colour(110, 110, 110));
    g.setOpacity(0.8f);
    g.fillRoundedRectangle(leftSliderPosition+1, 1, sliderWidth-2, this->getHeight()-2, 2);

    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(rightSliderPosition, 0, sliderWidth, this->getHeight(), 2);
    g.setColour(Colour(110, 110, 110));
    g.setOpacity(0.8f);
    g.fillRoundedRectangle(rightSliderPosition+1, 1, sliderWidth-2, this->getHeight()-2, 2);

    g.setColour(Colour(110, 110, 110));
    g.setOpacity(0.4f);
 
    g.fillRoundedRectangle(leftSliderPosition, 4, rightSliderPosition + sliderWidth - leftSliderPosition, this->getHeight(), 2);

    g.setColour(Colour(50, 50, 50));
    g.setFont(16);
    g.drawText(
        juce::String(getIntervalDurationInSeconds()),
        ((leftSliderPosition + rightSliderPosition + sliderWidth) / 2) - 10,
        0,
        20,
        this->getHeight() + tickHeight,
        juce::Justification::centred);

    /* Draw the current playback position */
    float timelinePos = (float)(fileReader->getCurrentSample() - startTimestamp) / (stopTimestamp - startTimestamp) * getWidth();
    //LOGD("Timeline pos: ", timelinePos, " current sample: ", fileReader->getCurrentSample(), " start timestamp: ", startTimestamp, " stop timestamp: ", stopTimestamp);
    if (fileReader->playbackIsActive() || (!fileReader->playbackIsActive() && timelinePos < rightSliderPosition + sliderWidth))
    {
        g.setOpacity(1.0f);
        g.fillRoundedRectangle(timelinePos, 0, 1, this->getHeight(), 0.2);
    }


}

void ZoomTimeline::mouseDown(const MouseEvent& event)
{
    if (event.x > leftSliderPosition && event.x < leftSliderPosition + sliderWidth) {
        leftSliderIsSelected = true;
    } else if (event.x > rightSliderPosition && event.x < rightSliderPosition + sliderWidth) {
        rightSliderIsSelected = true;
    } else if (event.x > leftSliderPosition && event.x < rightSliderPosition) {
        playbackRegionIsSelected = true;
    }
}

void ZoomTimeline::mouseDrag(const MouseEvent & event) 
{

    float regionWidth = rightSliderPosition - leftSliderPosition;

    if (leftSliderIsSelected) {

        if (event.x > sliderWidth / 2 && event.x < rightSliderPosition - sliderWidth / 2)
        {

            leftSliderPosition = event.x - sliderWidth / 2;

            if (rightSliderPosition < getWidth() - sliderWidth)

                rightSliderPosition = leftSliderPosition + regionWidth;
        }

    } else if (rightSliderIsSelected) {

        if (event.x > leftSliderPosition + 1.5*sliderWidth && event.x < getWidth() - sliderWidth / 2)
            rightSliderPosition = event.x - sliderWidth / 2;

    } else if (playbackRegionIsSelected) {
    
        if (leftSliderPosition >= 0 && rightSliderPosition <= getWidth() - sliderWidth) {

            if (event.x >= regionWidth / 2 + sliderWidth && event.x <= getWidth() - regionWidth / 2) {

                leftSliderPosition = event.x - regionWidth / 2 - sliderWidth;
                rightSliderPosition = event.x + regionWidth / 2 - sliderWidth;

            }
            
        }
    }

    lastDragXPosition = event.x;


    // Prevent slider going out of timeline bounds
    if (leftSliderPosition < 0)
        leftSliderPosition = 0;

    if (rightSliderPosition > getWidth() - sliderWidth)
        rightSliderPosition = getWidth() - sliderWidth;

    repaint();
    
}

void ZoomTimeline::mouseUp(const MouseEvent& event) 
{

    leftSliderIsSelected = false;
    rightSliderIsSelected = false;
    playbackRegionIsSelected = false;

    if (fileReader->playbackIsActive())
    {
        fileReader->switchBuffer();
    }

    static_cast<FileReaderEditor*>(fileReader->getEditor())->updatePlaybackTimes(); 
    
}

PlaybackButton::PlaybackButton(FileReader* fr) : Button ("Playback")
{
    fileReader = fr;
    isActive = true;
}

PlaybackButton::~PlaybackButton() {}

void PlaybackButton::setState(bool isActive)
{

    this->isActive = isActive;

    if (!isActive) // Pressed play
        static_cast<FileReaderEditor*>(fileReader->getEditor())->updatePlaybackTimes();
    else if (!isActive && fileReader->playbackIsActive())
        fileReader->stopAcquisition();
}

bool PlaybackButton::getState()
{
    return isActive;
}

void PlaybackButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown) 
{

    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0,0,getWidth(),getHeight(),0.2*getWidth());

	g.setColour(Colour(110,110,110));
    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

    g.fillRoundedRectangle(1, 1, getWidth() - 2, getHeight() - 2, 0.2 * getWidth());

    int x = getScreenX(); 
    int y = getScreenY(); 
    int width = getWidth(); 
    int height = getHeight(); 

    if (isActive)
    {
        /* Draw pause button */
        int padding = 0.3*width;
        g.setColour(Colour(255,255,255)); 
        g.fillRect(padding, padding, 0.2*width, height - 2*padding);
        g.fillRect(width / 2 + 0.075*width, padding, 0.2*width, height - 2*padding);

    } else {
        
        /* Draw playbutton */
        int padding = 0.3*height;
        g.setColour(Colour(255,255,255)); 
        Path triangle; 
        triangle.addTriangle(padding, padding, padding, height - padding, width - padding, height/2); 
        g.fillPath(triangle);

    }
	
}

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
    , scrubInterfaceWidth(420)
{

    scrubDrawerButton = new ScrubDrawerButton(getNameAndId() + " Scrub Drawer Button");
	scrubDrawerButton->setBounds(4, 40, 10, 78);
    scrubDrawerButton->setToggleState(false, dontSendNotification);
	scrubDrawerButton->addListener(this);
	addChildComponent(scrubDrawerButton);

    zoomStartTimeLabel = new Label("ZoomStartTime", "00:00");
    zoomStartTimeLabel->setBounds(10,30,40,10);
    addChildComponent(zoomStartTimeLabel);

    zoomMiddleTimeLabel = new Label("ZoomMidTime", "00:00");
    zoomMiddleTimeLabel->setBounds(0.454*scrubInterfaceWidth,30,40,10);
    addChildComponent(zoomMiddleTimeLabel);

    zoomEndTimeLabel = new Label("ZoomEndTime", "00:00");
    zoomEndTimeLabel->setBounds(0.88*scrubInterfaceWidth,30,40,10);
    addChildComponent(zoomEndTimeLabel);

    fullStartTimeLabel = new Label("FullStartTime", "00:00:00");
    fullStartTimeLabel->setBounds(0,100,60,10);
    addChildComponent(fullStartTimeLabel);

    fullEndTimeLabel = new Label("FullEndTime", "00:00:00");
    fullEndTimeLabel->setBounds(0.855*scrubInterfaceWidth,100,60,10);
    addChildComponent(fullEndTimeLabel);

    int padding = 30;
    zoomTimeline = new ZoomTimeline(fileReader);
    zoomTimeline->setBounds(padding, 46, scrubInterfaceWidth - 2*padding, 20);
    zoomTimeline->updatePlaybackRegion(fileReader->getPlaybackStart(), fileReader->getPlaybackStop());
    addChildComponent(zoomTimeline);

    fullTimeline = new FullTimeline(fileReader);
    fullTimeline->setBounds(padding, 76, scrubInterfaceWidth - 2*padding, 20);
    addChildComponent(fullTimeline);

    int buttonSize = 24;
    playbackButton = new PlaybackButton(fileReader);
    playbackButton->setState(true);
    playbackButton->setBounds(scrubInterfaceWidth / 2 - buttonSize / 2, 103, buttonSize, buttonSize);
    playbackButton->addListener(this);
    addChildComponent(playbackButton);

    lastFilePath = CoreServices::getDefaultUserSaveDirectory();

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

    /* Event channel colors */
    channelColours.add(Colour(224, 185, 36));
    channelColours.add(Colour(214, 210, 182));
    channelColours.add(Colour(243, 119, 33));
    channelColours.add(Colour(186, 157, 168));
    channelColours.add(Colour(237, 37, 36));
    channelColours.add(Colour(179, 122, 79));
    channelColours.add(Colour(217, 46, 171));
    channelColours.add(Colour(217, 139, 196));
    channelColours.add(Colour(101, 31, 255));
    channelColours.add(Colour(141, 111, 181));
    channelColours.add(Colour(48, 117, 255));
    channelColours.add(Colour(184, 198, 224));
    channelColours.add(Colour(116, 227, 156));
    channelColours.add(Colour(150, 158, 155));
    channelColours.add(Colour(82, 173, 0));
    channelColours.add(Colour(125, 99, 32));

    desiredWidth = 200;

}

FileReaderEditor::~FileReaderEditor()
{
}

int FileReaderEditor::getFullTimelineStartPosition() 
{
    return fullTimeline->getStartInterval();
}

int FileReaderEditor::getZoomTimelineStartPosition()
{
    return zoomTimeline->getStartInterval();
}

void FileReaderEditor::updateZoomTimeLabels()
{

    int startPos = fullTimeline->getStartInterval();

    float frac = float(startPos) / float(fullTimeline->getWidth());

    for (int i = 0; i < 3; i++) {

        int ms = frac * timeLimits->getTimeMilliseconds(1) + i*15000;

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
        
        String timeString;
        
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

        if (hourFrac > 0)
            timeString += String(hourFrac).paddedLeft ('0', 2) + ":";

        timeString += String (minFrac).paddedLeft ('0', 2) + ":" + String (secFrac).paddedLeft ('0', 2);

        if (i == 0)
            zoomStartTimeLabel->setText(timeString, juce::sendNotificationAsync);
        else if (i == 1)
            zoomMiddleTimeLabel->setText(timeString, juce::sendNotificationAsync);
        else
            zoomEndTimeLabel->setText(timeString, juce::sendNotificationAsync);


    }

}

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

        /* Only show scrubber interface if recording > 30s */
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

void FileReaderEditor::paintOverChildren (Graphics& g)
{
    /* Draws a frame if a file is currently being dragged over the editor */
    if (m_isFileDragAndDropActive)
    {
        g.setColour (Colours::aqua);
        g.drawRect (getLocalBounds(), 2.f);
    }

    if (scrubInterfaceVisible)
    {
        int leftRay = fullTimeline->getStartInterval();
        int rightRay = leftRay + fullTimeline->getIntervalWidth();

        g.drawLine(zoomTimeline->getX(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + leftRay, fullTimeline->getY());
        g.drawLine(zoomTimeline->getX()+zoomTimeline->getWidth(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + rightRay, fullTimeline->getY());
    }

}

void FileReaderEditor::buttonClicked (Button* button)
{

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

    if (button == scrubDrawerButton) {

        showScrubInterface(!scrubInterfaceVisible);

    } else if (button == playbackButton) {

        playbackButton->setState(!playbackButton->getState());
        updatePlaybackTimes();
        fileReader->togglePlayback();

    }

}

void FileReaderEditor::updatePlaybackTimes()
{

    //fileReader->switchBuffer();

    int64 startTimestamp = float(getFullTimelineStartPosition()) / fullTimeline->getWidth() * fileReader->getCurrentNumTotalSamples();
    startTimestamp += float(getZoomTimelineStartPosition()) / zoomTimeline->getWidth() * fileReader->getCurrentSampleRate() * 30.0f;
    fileReader->setPlaybackStart(startTimestamp);

    if (playbackButton->getState())
    {
        fileReader->setPlaybackStop(fileReader->getCurrentNumTotalSamples());
    }
    else
    {
        int64 stopTimestamp = startTimestamp + zoomTimeline->getIntervalDurationInSeconds() * fileReader->getCurrentSampleRate();
        fileReader->setPlaybackStop(stopTimestamp);
    }

}

void FileReaderEditor::collapsedStateChanged()
{
    if (!getCollapsedState())
    {

        fullTimeline->setVisible(scrubInterfaceVisible);
        zoomTimeline->setVisible(scrubInterfaceVisible);
        playbackButton->setVisible(scrubInterfaceVisible);
        zoomStartTimeLabel->setVisible(scrubInterfaceVisible);
        zoomMiddleTimeLabel->setVisible(scrubInterfaceVisible);
        zoomEndTimeLabel->setVisible(scrubInterfaceVisible);
        fullStartTimeLabel->setVisible(scrubInterfaceVisible);
        fullEndTimeLabel->setVisible(scrubInterfaceVisible);

        scrubDrawerButton->setVisible(scrubInterfaceAvailable);
    }
    
}

void FileReaderEditor::showScrubInterface(bool show)
{

    scrubInterfaceVisible = show;

    int dX = scrubInterfaceWidth;
    dX = show ? dX : -dX;
    desiredWidth += dX;

    /* Move all editor components to the right */
    scrubDrawerButton->setBounds(
        scrubDrawerButton->getX() + dX, scrubDrawerButton->getY(),
        scrubDrawerButton->getWidth(), scrubDrawerButton->getHeight()
    );

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

    /* Show all scrubber interface components */
    fullTimeline->setVisible(show);
    zoomTimeline->setVisible(show);
    playbackButton->setVisible(show);
    zoomStartTimeLabel->setVisible(show);
    zoomMiddleTimeLabel->setVisible(show);
    zoomEndTimeLabel->setVisible(show);
    fullStartTimeLabel->setVisible(show);
    fullEndTimeLabel->setVisible(show);

    CoreServices::highlightEditor(this);
    deselect();

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

    if (reset) {

        /* Reset scrubbing interface to show first 30 seconds of recording */
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

            /* Draws a 30 second interval on full timeline */
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
    

}

void FileReaderEditor::setRecording(int index)
{
    recordSelector->setSelectedItemIndex(index, sendNotification);
}

void FileReaderEditor::timerCallback()
{
    setCurrentTime(fileReader->samplesToMilliseconds(fileReader->getCurrentSample()));
}

void FileReaderEditor::setCurrentTime (unsigned int ms)
{
    const MessageManagerLock mmLock;
    currentTime->setTimeMilliseconds (0, ms);
}

void FileReaderEditor::comboBoxChanged (ComboBox* combo)
{
    fileReader->setParameter (0, combo->getSelectedId() - 1);
    CoreServices::updateSignalChain (this);
}

void FileReaderEditor::populateRecordings (FileSource* source)
{
    recordSelector->clear (dontSendNotification);

    const int numRecords = source->getNumRecords();

    for (int i = 0; i < numRecords; ++i)
    {
        recordSelector->addItem (source->getRecordName (i), i + 1);
    }

    recordSelector->setSelectedId (1, dontSendNotification);
}

void FileReaderEditor::clearEditor()
{
    fileNameLabel->setText ("No file selected.", dontSendNotification);
    recordSelector->clear (dontSendNotification);

    timeLimits->setTimeMilliseconds     (0, 0);
    timeLimits->setTimeMilliseconds     (1, 0);
    currentTime->setTimeMilliseconds    (0, 0);
    currentTime->setTimeMilliseconds    (1, 0);

    CoreServices::updateSignalChain(this);

}

void FileReaderEditor::startAcquisition()
{
    recordSelector->setEnabled (false);
    timeLimits->setEnable (false);
}

void FileReaderEditor::stopAcquisition()
{
    recordSelector->setEnabled (true);
    timeLimits->setEnable (true);
}

void FileReaderEditor::saveCustomParametersToXml (XmlElement* xml)
{
    xml->setAttribute ("Type", "FileReader");

    XmlElement* childNode = xml->createNewChildElement ("FILENAME");

    String file = fileReader->getFile();

    File executable = File::getSpecialLocation(File::currentApplicationFile);
#ifdef __APPLE__
    File defaultFile = executable.getChildFile("Contents/Resources/resources").getChildFile("structure.oebin");
#else
    File defaultFile = executable.getParentDirectory().getChildFile("resources").getChildFile("structure.oebin");
#endif

    if (file.equalsIgnoreCase(defaultFile.getFullPathName()))
        childNode->setAttribute("path", "default");
    else
        childNode->setAttribute("path", fileReader->getFile());

    childNode->setAttribute ("recording", recordSelector->getSelectedId());

    childNode = xml->createNewChildElement ("TIME_LIMITS");
    childNode->setAttribute ("start_time",  (double)timeLimits->getTimeMilliseconds (0));
    childNode->setAttribute ("stop_time",   (double)timeLimits->getTimeMilliseconds (1));
}

void FileReaderEditor::loadCustomParametersFromXml (XmlElement* xml)
{
    for (auto* element : xml->getChildIterator())
    {
        if (element->hasTagName ("FILENAME"))
        {
            String filepath = element->getStringAttribute ("path");
            setFile (filepath, false);

            int recording = element->getIntAttribute ("recording");
            recordSelector->setSelectedId (recording,sendNotificationSync);
        }
        else if (element->hasTagName ("TIME_LIMITS"))
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
    setFile (files[0]);

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
