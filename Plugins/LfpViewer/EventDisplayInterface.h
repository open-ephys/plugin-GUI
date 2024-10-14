/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2024 Open Ephys

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

#ifndef __EVENTDISPLAYINTERFACE_H__
#define __EVENTDISPLAYINTERFACE_H__

#include <VisualizerWindowHeaders.h>

#include <array>
#include <vector>

#include "LfpDisplayClasses.h"
#include "LfpDisplayNode.h"

namespace LfpViewer
{

/**
    Interface class for Event Display channels.

    Holds a for toggling one channel's event display on and off.

 */
class EventDisplayInterface : public Component,
                              public Button::Listener
{
public:
    /** Constructor */
    EventDisplayInterface (LfpDisplay*, LfpDisplaySplitter*, int chNum);

    /** Destrutor */
    ~EventDisplayInterface();

    /** Renders the background */
    void paintOverChildren (Graphics& g) override;

    /** Responds to button presses */
    void buttonClicked (Button* button) override;

    void resized() override;

    /** Checks whether events should be displayed for this channel*/
    void checkEnabledState();

private:
    int channelNumber;
    bool isEnabled;

    LfpDisplay* display;
    LfpDisplaySplitter* canvasSplit;

    std::unique_ptr<UtilityButton> chButton;
};

}; // namespace LfpViewer
#endif
