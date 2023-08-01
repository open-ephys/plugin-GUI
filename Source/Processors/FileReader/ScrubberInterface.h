#ifndef __SCRUBBERINTERFACE_H_D6EC8B48__
#define __SCRUBBERINTERFACE_H_D6EC8B48__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

#include "FileReader.h"

class FileReaderEditor;

class PlaybackButton : public Button
{
public:
    PlaybackButton(FileReader*);

    ~PlaybackButton();

    bool getState();
    void setState(bool isActive);
    
private:

    FileReader* fileReader;

    bool isActive;
    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown);
};

class FullTimeline : public Component, public Timer
{
public:
    FullTimeline(FileReader*);
    ~FullTimeline();

    void setIntervalPosition(int start, int width);

    int getStartInterval();
    int getIntervalWidth();

private:

    FileReader* fileReader;

    void timerCallback();

    int intervalStartPosition;
    int intervalWidth;
    bool intervalIsSelected;

    void paint(Graphics& g);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);

    bool leftSliderIsSelected;

};

class ZoomTimeline : public Component, public Timer
{
public:
    ZoomTimeline(FileReader*);
    ~ZoomTimeline();

    void updatePlaybackRegion(int min, int max);
    int getStartInterval();
    int getIntervalDurationInSeconds();

private:

    FileReader* fileReader;

    int sliderWidth;
    int widthInSeconds;
    float leftSliderPosition;
    float rightSliderPosition;
    float lastDragXPosition;

    void paint(Graphics& g);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
    void mouseUp(const MouseEvent& event);

    bool leftSliderIsSelected;
    bool rightSliderIsSelected;
    bool playbackRegionIsSelected;

    void timerCallback();

};

class ScrubberInterface : public Component, public Button::Listener
{
public:
    ScrubberInterface(FileReader*);
    ~ScrubberInterface();

    ScopedPointer<Label>                zoomStartTimeLabel;
    ScopedPointer<Label>                zoomMiddleTimeLabel;
    ScopedPointer<Label>                zoomEndTimeLabel;

    ScopedPointer<Label>                fullStartTimeLabel;
    ScopedPointer<Label>                fullEndTimeLabel;

    ScopedPointer<FullTimeline>         fullTimeline;
    ScopedPointer<ZoomTimeline>         zoomTimeline;

    ScopedPointer<PlaybackButton>       playbackButton;

    void buttonClicked (Button* button) override;

    void paintOverChildren(Graphics& g) override;

    void updatePlaybackTimes();
    
    /** Updates the time labels based on current slider positions */
    void updateZoomTimeLabels();

    /** Gets the location of the global start of playback */
    int getFullTimelineStartPosition();

    /** Gets the location of the local start of playback */
    int getZoomTimelineStartPosition();

    /** Updates component when parameters change via FileReader */
    void update();

private:

    FileReader* fileReader;
};

#endif  // __SCRUBBERINTERFACE_H_D6EC8B48__