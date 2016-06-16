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

#ifndef PSTHCOMMON_H_INCLUDED
#define PSTHCOMMON_H_INCLUDED

struct zoom
{
	float xmin, xmax, ymin, ymax;
};


enum xyPlotTypes
{
	SPIKE_PLOT = 0,
	LFP_PLOT = 1,
	EYE_PLOT = 2
};


#ifndef max
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif


#endif