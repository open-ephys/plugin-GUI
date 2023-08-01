#include "ScrubberInterface.h"

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
                g.setColour(eventChannelColours[info.channels[i]]);
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
    fileReader->getScrubberInterface()->updateZoomTimeLabels();
    fileReader->getEditor()->repaint();
    
}

void FullTimeline::mouseUp(const MouseEvent& event) 
{

    intervalIsSelected = false;

    fileReader->getScrubberInterface()->updatePlaybackTimes();
    
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

    int intervalStartPos = fileReader->getScrubberInterface()->getFullTimelineStartPosition();

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
                g.setColour(eventChannelColours[info.channels[i]]);
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

    fileReader->getScrubberInterface()->updatePlaybackTimes(); 
    
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

    zoomStartTimeLabel = new Label("ZoomStartTime", "00:00");
    zoomStartTimeLabel->setBounds(10,30,40,10);
    addChildComponent(zoomStartTimeLabel);
    addAndMakeVisible(zoomStartTimeLabel);

    zoomMiddleTimeLabel = new Label("ZoomMidTime", "00:00");
    zoomMiddleTimeLabel->setBounds(0.454*scrubInterfaceWidth,30,40,10);
    addChildComponent(zoomMiddleTimeLabel);
    addAndMakeVisible(zoomMiddleTimeLabel);

    zoomEndTimeLabel = new Label("ZoomEndTime", "00:00");
    zoomEndTimeLabel->setBounds(0.88*scrubInterfaceWidth,30,40,10);
    addChildComponent(zoomEndTimeLabel);
    addAndMakeVisible(zoomEndTimeLabel);

    fullStartTimeLabel = new Label("FullStartTime", "00:00:00");
    fullStartTimeLabel->setBounds(0,100,60,10);
    addChildComponent(fullStartTimeLabel);
    addAndMakeVisible(fullStartTimeLabel);

    fullEndTimeLabel = new Label("FullEndTime", "00:00:00");
    fullEndTimeLabel->setBounds(0.855*scrubInterfaceWidth,100,60,10);
    addChildComponent(fullEndTimeLabel);
    addAndMakeVisible(fullEndTimeLabel);

    int padding = 30;
    zoomTimeline = new ZoomTimeline(fileReader);
    zoomTimeline->setBounds(padding, 46, scrubInterfaceWidth - 2*padding, 20);
    zoomTimeline->updatePlaybackRegion(fileReader->getPlaybackStart(), fileReader->getPlaybackStop());
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

ScrubberInterface::~ScrubberInterface() {}

void ScrubberInterface::updatePlaybackTimes()
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

void ScrubberInterface::paintOverChildren(Graphics& g) {

    //TOFIX: Currently draws rays infinitely to the right
    return;

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

    TimeParameter* tp = static_cast<TimeParameter*>(fileReader->getParameter("end_time"));
    int ms = tp->getTimeValue()->getTimeInMilliseconds();

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

    ms = tp->getTimeValue()->getTimeInMilliseconds();

    if (ms / 1000.0f > 30) {

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

void ScrubberInterface::updateZoomTimeLabels()
{

/*
    int startPos = fullTimeline->getStartInterval();

    float frac = float(startPos) / float(fullTimeline->getWidth());

    for (int i = 0; i < 3; i++) {

        int ms = frac * fileReader->getTimeMilliseconds(1) + i*15000;

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
    */

}
