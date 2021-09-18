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

#include "DelayMonitor.h"


DelayMonitor::DelayMonitor() : delay(0.0f)
{
}

DelayMonitor::~DelayMonitor()
{

}

void DelayMonitor::setDelay(float delayMs)
{
    delay = delayMs;
}

void DelayMonitor::startAcquisition()
{
    startTimer(500);
}

void DelayMonitor::stopAcquisition()
{
    stopTimer();
}

void DelayMonitor::timerCallback()
{
    repaint();
}

void DelayMonitor::paint(Graphics& g)
{
    g.setFont(12);
    g.drawText(String(delay, 2) + " ms", 0, 0, 60, 15, Justification::left);
}