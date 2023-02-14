/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 /Users/franman/Documents/Programming/GitHub/plugin-GUI/Plugins/BasicSpikeDisplay/SpikeDisplayNode/SpikeDisplayCanvas.h

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include <VisualizerWindowHeaders.h>

#include "SpikeDisplay.h"

#include <vector>

class SpikePlot;
class SpikeDisplayNode;

/**
    Allows spike plot thresholds to be adjusted synchronously
*/
class SpikeThresholdCoordinator
{
public:

    /** Constructor*/
    SpikeThresholdCoordinator();

    /** Destructor*/
    ~SpikeThresholdCoordinator();

    /** Registers a plot to interact with this coordinator*/
    void registerSpikePlot(SpikePlot* sp);

    /** De-registers a plot to interact with this coordinators*/
    void deregisterSpikePlot(SpikePlot* sp);

    /** Sets the lock threshold state*/
    void setLockThresholds(bool en);

    /** Returns the lock threshold state*/
    bool getLockThresholds();

    /** Sets the thresholds of all registered plots*/
    void thresholdChanged(float displayThreshold, float range);

private:

    bool lockThresholds;
    Array<SpikePlot*> registeredPlots;

    WeakReference<SpikeThresholdCoordinator>::Master masterReference;
    friend class WeakReference<SpikeThresholdCoordinator>;

};

/**

  Displays spike waveforms and projections.

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

class SpikeDisplayCanvas : public Visualizer, 
                           public Button::Listener

{
public:

    /** Constructor */
    SpikeDisplayCanvas(SpikeDisplayNode* n);

    /** Destructor */
    ~SpikeDisplayCanvas() { }

    /** Render black background */
    void paint(Graphics& g);

    /** Called instead of "repaint" to avoid redrawing underlying components.*/
    void refresh();

    /** Called when the component's tab becomes visible again*/
    void refreshState();

    /** Creates spike displays for incoming spike channels*/
    void update();

    /** Aligns components*/
    void resized();

    /** Respond to clear / lock thresholds / invert spikes buttons*/
    void buttonClicked(Button* button);

    /** Clears audio monitor selection for all sub-plots*/
    void resetAudioMonitorState();

    /** Sets the scaling facotr for the sub-plots*/
    void setPlotScaleFactor(float scale);

    /** Saves display parameters */
    void saveCustomParametersToXml(XmlElement* xml);

    /** Loads display parameters */
    void loadCustomParametersFromXml(XmlElement* xml);

    /** Pointer to the underlying SpikeDisplayNode*/
    SpikeDisplayNode* processor;

    void cacheDisplaySettings(int electrodeIndex, int channelIndex, double threshold, double range, bool isMonitored);

    bool hasCachedDisplaySettings(int electrodeIndex, int channelIndex);

    void invalidateDisplaySettings(int electrodeIndex);

private:

    std::unique_ptr<SpikeDisplay> spikeDisplay;
    std::unique_ptr<Viewport> viewport;

    bool newSpike;

    int scrollBarThickness;

    std::unique_ptr<UtilityButton> clearButton;
    std::unique_ptr<SpikeThresholdCoordinator> thresholdCoordinator;
    std::unique_ptr<UtilityButton> lockThresholdsButton;
    std::unique_ptr<UtilityButton> invertSpikesButton;

    std::map<int, std::map<int, double>> ranges;
    std::map<int, std::map<int, double>> thresholds;
    std::map<int, bool> monitors;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayCanvas);

};


#endif  // SPIKEDISPLAYCANVAS_H_
