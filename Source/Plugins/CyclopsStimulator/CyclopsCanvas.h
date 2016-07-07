/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

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

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include <VisualizerWindowHeaders.h>
#include "CyclopsProcessor.h"

namespace cyclops {
class CyclopsProcessor;

/**
 * @brief      Holds UI widgets for Cyclops.
 */

class CyclopsCanvas : public Visualizer
                    , public Button::Listener
{
public:
    CyclopsCanvas(CyclopsProcessor* n);
    ~CyclopsCanvas();

    /** Called when the component's tab becomes visible again.*/
    virtual void refreshState();

    /** Called when parameters of underlying data processor are changed.*/
    virtual void update();

    /** Called instead of "repaint" to avoid redrawing underlying components if not necessary.*/
    virtual void refresh();

    /** Disables all input widgets on the editor. */
    void disableAllInputWidgets();
    /** Enables all input widgets on the editor. */
    void enableAllInputWidgets();
    
    void paint(Graphics& g);

    /** Called when data acquisition is active.*/
    virtual void beginAnimation();

    /** Called when data acquisition ends.*/
    virtual void endAnimation();

    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, float);

    /** Called by an editor to initiate a parameter change.*/
    virtual void setParameter(int, int, int, float);

    void resized();

    /** Starts the timer callbacks. */
    //void startCallbacks();

    /** Stops the timer callbacks. */
    //void stopCallbacks();

    /** Called whenever the timer is triggered. */
    //void timerCallback();

    /** Refresh rate in Hz. */
    float refreshRate;

    void buttonClicked(Button* button);

    bool keyPressed(const KeyPress& key);

    void timerCallback();
    
    /** Saves parameters as XML */
    virtual void saveVisualizerParameters(XmlElement* xml);

    /** Loads parameters from XML */
    virtual void loadVisualizerParameters(XmlElement* xml);

    CyclopsProcessor* processor;

private:
    // TEST Buttons
    OwnedArray<UtilityButton> testButtons;
    ScopedPointer<ProgressBar> progressBar;
    // Some state vars for "TEST" UI
    double progress, pstep;
    bool in_a_test;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsCanvas);

};

} // NAMESPACE cyclops
#endif
