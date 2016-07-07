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

namespace cyclops {
CyclopsCanvas::CyclopsCanvas(CyclopsProcessor* n) :
    processor(n)
{
    in_a_test = false;
    progress = 0;
    // Add TEST buttons
    for (int i=0; i < 4; i++){
        testButtons.add(new UtilityButton(String("Test") + String(i), Font("Default", 10, Font::bold)));
        testButtons[i]->setBounds(7+(58*i), 104, 40, 20);
        testButtons[i]->addListener(this);
        addAndMakeVisible(testButtons[i]);
    }
    //progressBar = new ProgressBar(progress.var);
    progressBar = new ProgressBar(progress);
    progressBar->setPercentageDisplay(false);
    progressBar->setBounds(2, 106, 236, 16);
    addChildComponent(progressBar);

    // communicate with teensy.
    
    pstep = 0.01;
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

void CyclopsCanvas::disableAllInputWidgets()
{
    // Disable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(false);
}

void CyclopsCanvas::enableAllInputWidgets()
{
    // Reenable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(true);
}


void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    if (!in_a_test)
        progressBar->setVisible(false);
}

void CyclopsCanvas::buttonClicked(Button* button)
{  
    int test_index = -1;
    for (int i=0; i < 4; i++){
        if (button == testButtons[i]){
            test_index = i;
            break;
        }
    }
    if (test_index >= 0){
        disableAllInputWidgets();
        std::cout << "Testing LED channel " << test_index << "\n";
        in_a_test = true;
        //node->testChannel(test_index);
        progressBar->setVisible(true);
        startTimer(20);
        test_index = -1;
        //for (auto& )
    }
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

void CyclopsCanvas::timerCallback()
{
    if (in_a_test){
        progress += pstep;
        if (progress >= 1.0){
            progressBar->setVisible(false);
            progress = 0;
            in_a_test = false;
            stopTimer();
            enableAllInputWidgets();
        }
    }
}

void CyclopsCanvas::saveVisualizerParameters(XmlElement* xml)
{
    ;
}

void CyclopsCanvas::loadVisualizerParameters(XmlElement* xml)
{
    ;
}

} // NAMESPACE cyclops
