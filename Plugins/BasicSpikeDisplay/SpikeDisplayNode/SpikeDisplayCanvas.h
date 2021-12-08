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

#include "SpikeDisplayNode.h"

#include <vector>

enum WaveAxesSubPlots {
    WAVE1 = 0,
    WAVE2 = 1,
    WAVE3 = 2,
    WAVE4 = 3
};

enum Projection {
    PROJ1x2 4,
    PROJ1x3 5,
    PROJ1x4 6,
    PROJ2x3 7,
    PROJ2x4 8,
    PROJ3x4 9
};

enum SpikePlotType {
    WAVE_AXES,
    PROJECTION_AXES
};

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001

class SpikeDisplayNode;

class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
class WaveAxes;
class SpikePlot;
class RecordNode;
class SpikeThresholdCoordinator;

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

    /** Starts animation callbacks*/
    void beginAnimation();

    /** Ends animation callbacks*/
    void endAnimation();

    /** Called when the component's tab becomse visible again*/
    void refreshState();

    /** Creates spike displays for incoming spike channels*/
    void update();

    /** Aligns components*/
    void resized();

    /** Respond to clear / lock thresholds / invert spikes buttons*/
    void buttonClicked(Button* button);

    /** Saves display parameters */
    void saveCustomParametersToXml(XmlElement* xml);

    /** Loads display parameters */
    void loadCustomParametersFromXml(XmlElement* xml);

    /** Pointer to the underlying SpikeDisplayNode*/
    SpikeDisplayNode* processor;

private:

    std::unique_ptr<SpikeDisplay> spikeDisplay;
    std::unique_ptr<Viewport> viewport;

    bool newSpike;

    int scrollBarThickness;

    std::unique_ptr<UtilityButton> clearButton;
    std::unique_ptr<SpikeThresholdCoordinator> thresholdCoordinator;
    std::unique_ptr<UtilityButton> lockThresholdsButton;
    std::unique_ptr<UtilityButton> invertSpikesButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayCanvas);

};


#endif  // SPIKEDISPLAYCANVAS_H_
