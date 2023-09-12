#include "ScrubberInterface.h"

void FullTimeline::paint(Graphics& g) 
{

    /* Draw timeline background */
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
    TimeParameter::TimeValue* start = ((TimeParameter*)fileReader->getParameter("start_time"))->getTimeValue();
    TimeParameter::TimeValue* stop  = ((TimeParameter*)fileReader->getParameter("end_time"))->getTimeValue();
    TimeParameter::TimeValue* duration = new TimeParameter::TimeValue(stop->getTimeInMilliseconds() - start->getTimeInMilliseconds());

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = duration->getTimeInMilliseconds() / 1000.0f * sampleRate;

    int64 startSample = start->getTimeInMilliseconds() / 1000.0f * sampleRate;
    int64 stopSample = stop->getTimeInMilliseconds() / 1000.0f * sampleRate;

    for (auto info : fileReader->getActiveEventInfo())
    {
        for (int i = 0; i < info.timestamps.size(); i++) {
        
            int64 sampleNumber = info.timestamps[i]; //TODO: Update EventInfo object name timestamps -> sampleNumbers
            int16 state = info.channelStates[i];

            if (state && sampleNumber >= startSample && sampleNumber <= stopSample)
            {
                float timelinePos = (sampleNumber - startSample) / float(totalSamples) * getWidth();
                g.setColour(eventChannelColours[info.channels[i]]);
                g.setOpacity(1.0f);
                g.fillRoundedRectangle(timelinePos, 0, 1, this->getHeight() - tickHeight, 0.2);
            }

        }

    }

    /* Draw the 30-second interval */
    g.setColour(Colour(0,0,0));
    g.setOpacity(0.8f);

    if (intervalStartPosition < 0) return;
    setIntervalPosition(intervalStartPosition);

    g.fillRoundedRectangle(intervalStartPosition, 0, 2, this->getHeight(), 2);
    g.fillRoundedRectangle(intervalStartPosition + intervalWidth, 0, 2, this->getHeight(), 2);

    /* Draw the current playback position */
    float timelinePos = (float)(fileReader->getCurrentSample() - startSample) / totalSamples * getWidth();

    g.setOpacity(1.0f);
    g.fillRoundedRectangle(timelinePos, 0, 1, this->getHeight(), 0.2);

    fileReader->getScrubberInterface()->updateZoomTimeLabels();
	
}

void FullTimeline::setIntervalPosition(int pos) 
{

    //TODO: Check if interval is within bounds
    intervalStartPosition = pos;

    TimeParameter::TimeValue* start = ((TimeParameter*)fileReader->getParameter("start_time"))->getTimeValue();
    TimeParameter::TimeValue* stop  = ((TimeParameter*)fileReader->getParameter("end_time"))->getTimeValue();
    TimeParameter::TimeValue* duration = new TimeParameter::TimeValue(stop->getTimeInMilliseconds() - start->getTimeInMilliseconds());

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = duration->getTimeInMilliseconds() / 1000.0f * sampleRate;
    float totalTimeInSeconds = float(totalSamples) / sampleRate;

    intervalWidth = 30.0f / totalTimeInSeconds * float(getWidth());
}

int FullTimeline::getIntervalDurationInSeconds()
{
    TimeParameter::TimeValue* start = ((TimeParameter*)fileReader->getParameter("start_time"))->getTimeValue();
    TimeParameter::TimeValue* stop  = ((TimeParameter*)fileReader->getParameter("end_time"))->getTimeValue();
    TimeParameter::TimeValue* duration = new TimeParameter::TimeValue(stop->getTimeInMilliseconds() - start->getTimeInMilliseconds());

    return duration->getTimeInMilliseconds() / 1000.0f;
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
    fileReader->getScrubberInterface()->updateZoomTimeLabels();
    fileReader->getEditor()->repaint();
}

void FullTimeline::mouseUp(const MouseEvent& event) 
{
    intervalIsSelected = false;

    fileReader->getScrubberInterface()->updatePlaybackTimes();
}

int ZoomTimeline::getIntervalDurationInSeconds()
{
    /* Gets fraction of interval width and converts to nearest second */
    return round ( float( rightSliderPosition + sliderWidth - leftSliderPosition) / getWidth() * widthInSeconds );
}

void ZoomTimeline::paint(Graphics& g) 
{
    /* Draw timeline background */
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

    /* Draw all events within this 30-second interval */
    TimeParameter::TimeValue* start = ((TimeParameter*)fileReader->getParameter("start_time"))->getTimeValue();
    TimeParameter::TimeValue* stop  = ((TimeParameter*)fileReader->getParameter("end_time"))->getTimeValue();
    TimeParameter::TimeValue* duration = new TimeParameter::TimeValue(stop->getTimeInMilliseconds() - start->getTimeInMilliseconds());

    float sampleRate = fileReader->getCurrentSampleRate();
    int64 totalSamples = duration->getTimeInMilliseconds() / 1000.0f * sampleRate;
    int64 intervalSamples = 30 * sampleRate;

    int intervalStartPos = fileReader->getScrubberInterface()->getFullTimelineStartPosition();

    int64 offset = float(intervalStartPos) / float(getWidth()) * totalSamples;

    int64 startSampleNumber = float(start->getTimeInMilliseconds()) / 1000.0f * sampleRate + offset;
    int64 stopSampleNumber = startSampleNumber + intervalSamples;

    for (auto info : fileReader->getActiveEventInfo()) 
    {
        for (int i = 0; i < info.timestamps.size(); i++) 
        {            
            int64 sampleNumber = info.timestamps[i];
            int16 state = info.channelStates[i];

            if (state && sampleNumber >= startSampleNumber && sampleNumber <= stopSampleNumber)
            {
                float timelinePos = (sampleNumber - startSampleNumber) / float(intervalSamples) * getWidth();
                g.setColour(eventChannelColours[info.channels[i]]);
                g.setOpacity(1.0f);
                g.fillRect(int(timelinePos), tickHeight, 1, this->getHeight() - tickHeight);
            }

        }

    }
    
    /* Draw the scrubber interval */
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
    float timelinePos = (float)(fileReader->getCurrentSample() - startSampleNumber) / (stopSampleNumber - startSampleNumber) * getWidth();
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

    fileReader->getScrubberInterface()->updatePlaybackTimes(); 
}

void PlaybackButton::setState(bool isActive)
{

    this->isActive = isActive;

    if (!isActive) // Pressed play
        fileReader->getScrubberInterface()->updatePlaybackTimes();
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

ScrubberInterface::ScrubberInterface(FileReader* fileReader_) {

    fileReader = fileReader_;

    int scrubInterfaceWidth = 420;

    zoomStartTimeLabel = new Label("ZoomStartTime", "00:00:00.000");
    zoomStartTimeLabel->setBounds(0,30,100,10);
    addChildComponent(zoomStartTimeLabel);
    addAndMakeVisible(zoomStartTimeLabel);

    zoomMiddleTimeLabel = new Label("ZoomMidTime", "00:00:15.000");
    zoomMiddleTimeLabel->setBounds(0.39*scrubInterfaceWidth,30,100,10);
    addChildComponent(zoomMiddleTimeLabel);
    addAndMakeVisible(zoomMiddleTimeLabel);

    zoomEndTimeLabel = new Label("ZoomEndTime", "00:00:30.000");
    zoomEndTimeLabel->setBounds(0.75*scrubInterfaceWidth,30,100,10);
    addChildComponent(zoomEndTimeLabel);
    addAndMakeVisible(zoomEndTimeLabel);

    fullStartTimeLabel = new Label("FullStartTime", "00:00:00.000");
    fullStartTimeLabel->setBounds(0,100,100,10);
    addChildComponent(fullStartTimeLabel);
    addAndMakeVisible(fullStartTimeLabel);

    fullEndTimeLabel = new Label("FullEndTime", "00:00:00.000");
    fullEndTimeLabel->setBounds(0.75*scrubInterfaceWidth,100,100,10);
    addChildComponent(fullEndTimeLabel);
    addAndMakeVisible(fullEndTimeLabel);

    int padding = 30;
    zoomTimeline = new ZoomTimeline(fileReader);
    zoomTimeline->setBounds(padding, 46, scrubInterfaceWidth - 2*padding, 20);
    addChildComponent(zoomTimeline);
    addAndMakeVisible(zoomTimeline);

    fullTimeline = new FullTimeline(fileReader);
    fullTimeline->setBounds(padding, 76, scrubInterfaceWidth - 2*padding, 20);
    addChildComponent(fullTimeline);
    addAndMakeVisible(fullTimeline);

    int buttonSize = 24;
    playbackButton = new PlaybackButton(fileReader);
    playbackButton->setState(true);
    playbackButton->setBounds(scrubInterfaceWidth / 2 - buttonSize / 2, 103, buttonSize, buttonSize);
    playbackButton->addListener(this);
    addChildComponent(playbackButton);
    addAndMakeVisible(playbackButton);

    setVisible(false);

}

void ScrubberInterface::updatePlaybackTimes()
{

    int64 totalSamples = fullTimeline->getIntervalDurationInSeconds() * fileReader->getCurrentSampleRate();

    TimeParameter* start = static_cast<TimeParameter*>(fileReader->getParameter("start_time"));

    int64 newStartSample = start->getTimeValue()->getTimeInMilliseconds() / 1000.0f * fileReader->getCurrentSampleRate();
    newStartSample += float(getFullTimelineStartPosition()) / fullTimeline->getWidth() * totalSamples;
    newStartSample += float(getZoomTimelineStartPosition()) / zoomTimeline->getWidth() * fileReader->getCurrentSampleRate() * 30.0f;
    fileReader->setPlaybackStart(newStartSample);

    LOGD("Total samples in full timeline: ", totalSamples);
    LOGD("New start sample: ", newStartSample);

    /* Playback button deprecated
    if (playbackButton->getState())
    {
        fileReader->setPlaybackStop(fileReader->getCurrentNumTotalSamples());
    }
    else
    {
        int64 stopTimestamp = newStartSample + zoomTimeline->getIntervalDurationInSeconds() * fileReader->getCurrentSampleRate();
        fileReader->setPlaybackStop(stopTimestamp);
    }
    */

}

void ScrubberInterface::paintOverChildren(Graphics& g)
{

    int leftRay = fullTimeline->getStartInterval();
    int rightRay = leftRay + fullTimeline->getIntervalWidth();

    g.drawLine(zoomTimeline->getX(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + leftRay, fullTimeline->getY());
    g.drawLine(zoomTimeline->getX()+zoomTimeline->getWidth(), zoomTimeline->getY() + zoomTimeline->getHeight(), zoomTimeline->getX() + rightRay, fullTimeline->getY());

}

void ScrubberInterface::buttonClicked (Button* button) 
{

    playbackButton->setState(!playbackButton->getState());
    updatePlaybackTimes();
    fileReader->togglePlayback();

}

int ScrubberInterface::getFullTimelineStartPosition() 
{
    return fullTimeline->getStartInterval();
}

int ScrubberInterface::getZoomTimelineStartPosition()
{
    return zoomTimeline->getStartInterval();
}

void ScrubberInterface::update()
{

    TimeParameter* start = static_cast<TimeParameter*>(fileReader->getParameter("start_time"));
    fullStartTimeLabel->setText(start->getTimeValue()->toString(), juce::sendNotificationAsync);

    TimeParameter* stop  = static_cast<TimeParameter*>(fileReader->getParameter("end_time"));
    fullEndTimeLabel->setText(stop->getTimeValue()->toString(), juce::sendNotificationAsync);

    int duration = stop->getTimeValue()->getTimeInMilliseconds() - start->getTimeValue()->getTimeInMilliseconds();

    FileReaderEditor* e = static_cast<FileReaderEditor*>(fileReader->getEditor());

    if (duration / 1000.0f < 30) {
        e->showScrubInterface(false);
        e->enableScrubDrawer(false);
    }
    else 
    {
        e->enableScrubDrawer(true);
    }

    e->repaint();

}

void ScrubberInterface::updateZoomTimeLabels()
{

    TimeParameter::TimeValue* start = ((TimeParameter*)fileReader->getParameter("start_time"))->getTimeValue();
    TimeParameter::TimeValue* stop  = ((TimeParameter*)fileReader->getParameter("end_time"))->getTimeValue();

    TimeParameter::TimeValue* duration = new TimeParameter::TimeValue(stop->getTimeInMilliseconds() - start->getTimeInMilliseconds());

    int startPos = fullTimeline->getStartInterval();
    float frac = float(startPos) / float(fullTimeline->getWidth());

    for (int i = 0; i < 3; i++) {

        TimeParameter::TimeValue* time = new TimeParameter::TimeValue(start->getTimeInMilliseconds() + frac * duration->getTimeInMilliseconds() + 15000.0f * i);

        if (i == 0)
            zoomStartTimeLabel->setText(time->toString(), juce::sendNotificationAsync);
        else if (i == 1)
            zoomMiddleTimeLabel->setText(time->toString(), juce::sendNotificationAsync);
        else
            zoomEndTimeLabel->setText(time->toString(), juce::sendNotificationAsync);

    }

}
