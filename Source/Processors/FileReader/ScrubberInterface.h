#ifndef __SCRUBBERINTERFACE_H_D6EC8B48__
#define __SCRUBBERINTERFACE_H_D6EC8B48__

#include "../../../JuceLibraryCode/JuceHeader.h"
#include "../Editors/GenericEditor.h"

#include "FileReader.h"
#include "FileReaderEditor.h"
class PlaybackButton : public Button
{
public:
    PlaybackButton(FileReader* reader)
        : juce::Button("PlaybackButton")
        , fileReader(reader)
        , isActive(true) {}
    ~PlaybackButton() {};

    bool getState();
    void setState(bool isActive);

private:
    FileReader* fileReader;
    bool isActive;

    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown) override;
};


class Timeline : public Component, public Timer
{
public:
    Timeline(FileReader* reader) : fileReader(reader) {}
    virtual ~Timeline() {}

    virtual int getStartInterval() = 0;
    virtual int getIntervalWidth() = 0;
    virtual int getIntervalDurationInSeconds() = 0;

    virtual void timerCallback() = 0;

protected:
    FileReader* fileReader;

    void paint(Graphics& g) override = 0;
    void mouseDown(const MouseEvent& event) override = 0;
    void mouseDrag(const MouseEvent& event) override = 0;
    void mouseUp(const MouseEvent& event) override = 0;
};


class FullTimeline : public Timeline
{
public:
    FullTimeline(FileReader* fr)
        : Timeline(fr)
    {
        startTimer(50);
    }
    ~FullTimeline() override {};

    int getStartInterval() { return intervalStartPosition; }
    int getIntervalWidth() { return intervalWidth; }
    int getIntervalDurationInSeconds();
    
    void setIntervalPosition(int pos);

    void timerCallback() override { repaint(); };

private:
    int intervalStartPosition;
    int intervalWidth;
    bool intervalIsSelected;

    void paint(Graphics& g) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

    bool leftSliderIsSelected;
};

class ZoomTimeline : public Timeline
{
public:
    ZoomTimeline(FileReader* fr)
        :   Timeline(fr),
            sliderWidth(8),
            widthInSeconds(30),
            leftSliderPosition(0),
            rightSliderPosition(110)
    {
        startTimer(50);
    }
    ~ZoomTimeline() override {};

    int getStartInterval() { return leftSliderPosition; }
    int getIntervalWidth() { return rightSliderPosition - leftSliderPosition; }
    int getIntervalDurationInSeconds();

    void timerCallback() override { repaint(); };

private:
    int sliderWidth;
    int widthInSeconds;
    float leftSliderPosition;
    float rightSliderPosition;
    float lastDragXPosition;

    void paint(Graphics& g) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;

    bool leftSliderIsSelected;
    bool rightSliderIsSelected;
    bool playbackRegionIsSelected;
};


class ScrubberInterface : public Component, public Button::Listener
{
public:
    ScrubberInterface(FileReader* reader);
    ~ScrubberInterface() override {};

    ScopedPointer<Label> zoomStartTimeLabel;
    ScopedPointer<Label> zoomMiddleTimeLabel;
    ScopedPointer<Label> zoomEndTimeLabel;
    ScopedPointer<Label> fullStartTimeLabel;
    ScopedPointer<Label> fullEndTimeLabel;

    ScopedPointer<Timeline> fullTimeline;
    ScopedPointer<Timeline> zoomTimeline;

    ScopedPointer<PlaybackButton> playbackButton;

    void buttonClicked (Button* button) override;
    void paintOverChildren(Graphics& g) override;
    void updatePlaybackTimes();
    void updateZoomTimeLabels();
    int getFullTimelineStartPosition();
    int getZoomTimelineStartPosition();
    void update();

private:
    FileReader* fileReader;
};


#endif  // __SCRUBBERINTERFACE_H_D6EC8B48__