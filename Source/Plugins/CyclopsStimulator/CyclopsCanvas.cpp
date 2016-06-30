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

#include "CyclopsCanvas.h"


CyclopsCanvas::CyclopsCanvas(CyclopsProcessor* n) :
    processor(n)
{
    ;
}

CyclopsCanvas::~CyclopsCanvas()
{

}

void CyclopsCanvas::beginAnimation()
{
    std::cout << "CyclopsCanvas beginning animation." << std::endl;

    startCallbacks();
}

void CyclopsCanvas::endAnimation()
{
    std::cout << "CyclopsCanvas ending animation." << std::endl;

    stopCallbacks();
}

void CyclopsCanvas::setParameter(int x, float f)
{

}

void CyclopsCanvas::setParameter(int a, int b, int c, float d)
{
}

void CyclopsCanvas::update()
{
    std::cout << "Updating CyclopsCanvas" << std::endl;
}


void CyclopsCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void CyclopsCanvas::resized()
{
    std::cout << "Resizing CyclopsCanvas" << std::endl;
}

/*
void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}
*/
void CyclopsCanvas::refresh()
{
    repaint();
}

void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::grey);

}

bool CyclopsCanvas::keyPressed(const KeyPress& key)
{

    KeyPress c = KeyPress::createFromDescription("c");

    if (key.isKeyCode(c.getKeyCode())) // C
    {
        std::cout << "Clearing display" << std::endl;
        return true;
    }
    return false;
}


void CyclopsCanvas::saveVisualizerParameters(XmlElement* xml)
{
    ;
}

void CyclopsCanvas::loadVisualizerParameters(XmlElement* xml)
{
    ;
}

// ----------------------------------------------------------------
