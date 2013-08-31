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

#include "InfoLabel.h"

InfoLabel::InfoLabel()
{

    labelFont = Font("Paragraph", 24, Font::plain);

    // MemoryInputStream mis(BinaryData::misoserialized,
    //                       BinaryData::misoserializedSize,
    //                       false);
    // Typeface::Ptr tp = new CustomTypeface(mis);
    // labelFont = Font(tp);
    // labelFont.setHeight(24);

    infoString = "Welcome to the Open Ephys GUI!\n \n"
                 "To get starting using the GUI, drag a processor from the list "
                 "on the left and drop it onto the signal chain. Each processor is "
                 "classified as either a source, a filter, or a sink. "
                 "Sources supply the data to each signal chain, filters process the "
                 "data, and sinks use the data to control outputs. Data always flows "
                 "from left to right.\n \n"
                 "Every signal chain must have one and only one source, and no more "
                 "than one sink. If a source is dropped in the middle of a signal chain, "
                 "it will start its own chain. Likewise, if a sink is dropped in the middle "
                 "of a chain, the processors to its right will form a new chain. "
                 "Multiple sources and sinks can be combined into a single chain through "
                 "the use of splitters and mergers. The current GUI allows you to construct "
                 "as many signal chains as you like, but be prudent.\n \n"
                 "Once the signal chain is in place, you can start and stop acquisition "
                 "by pressing the ""play"" button at the top of the screen. Acquisition will "
                 "only begin if all of the processors in the signal chain are enabled. Disabled "
                 "processors appear gray. Processors may be disabled if they aren't connected to a "
                 "source, or if they receive input from a processor to which they cannot connect."
                 " You'll have to fix these issues before you can acquire data.\n \n"
                 "You can also start acquisition by pressing the record button.\n \n"
                 "The GUI is still being actively developed, so it lacks "
                 "many crucial features and includes more than a few bugs. "
                 "There's a lot we're still planning on adding, and we will "
                 "be glad to have some help down the road. If you'd like to be "
                 "added as a contributor, please get in touch with us at "
                 "http://open-ephys.org/contact\n \n"
                 "DISCLAIMER: This software is still under development. We can't guarantee "
                 "that it will work as expected, so be sure to thoroughly test any features "
                 "you plan on using!"
                 ;

}

InfoLabel::~InfoLabel()
{

}


void InfoLabel::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

    g.setFont(labelFont);

    g.setColour(Colours::black);

    g.drawFittedText(infoString, 10, 10, getWidth()-10, getHeight()-10, Justification::left, 100);

}
