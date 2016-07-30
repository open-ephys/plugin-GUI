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

#ifndef __LISTSPICEPARSER_H_
#define __LISTSPICEPARSER_H_

#include "../../JuceLibraryCode/JuceHeader.h"

class ListSliceParser;

/*
ListSliceParser Class: Find out ranges from a string separated by ';' or ','.
The range can be of one of the following type:
1. [ : ]        -> no start and end value. In this case start value is 1 and end value is index of last element
2. [ x : y ]    -> range given. Start from index x and end at index y.
3. [ x : p : y] -> stride parameter p. start from x, select channels at intervals p upto y.
*/
class ListSliceParser
{
public:
    static Array<int> parseStringIntoRange (String textBoxInfo,int rangeValue);


private:
    static int convertStringToInteger (String s);
};



#endif  //__LISTSPICEPARSER_H_
