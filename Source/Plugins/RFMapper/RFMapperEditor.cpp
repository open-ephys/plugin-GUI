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

#include "RFMapperEditor.h"

RFMapperEditor::RFMapperEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
: VisualizerEditor(parentNode, useDefaultParameterEditors)

{
    
    tabText = "Mapper";
    
    desiredWidth = 180;
    
}

RFMapperEditor::~RFMapperEditor()
{
}


Visualizer* RFMapperEditor::createNewCanvas()
{
    
    RFMapper* processor = (RFMapper*) getProcessor();
    return new RFMapperCanvas(processor);
    
}

void RFMapperEditor::buttonCallback(Button* button)
{
    
    int gId = button->getRadioGroupId();
    
    if (gId > 0)
    {
        if (canvas != 0)
        {
            canvas->setParameter(gId-1, button->getName().getFloatValue());
        }
        
    }
    
}



RFMapperCanvas::RFMapperCanvas(RFMapper* sr) : processor(sr)
{
    
}


RFMapperCanvas::~RFMapperCanvas(){
    
}
    
void RFMapperCanvas::beginAnimation()
{
    
}

void RFMapperCanvas::endAnimation()
{
    
}
    
void RFMapperCanvas::refreshState()
{
}

void RFMapperCanvas::update()
{
    
}
    
void RFMapperCanvas::setParameter(int, float) {}

void RFMapperCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::orange);
}
    
void RFMapperCanvas::refresh()
{
    
}
    
void RFMapperCanvas::resized()
{
    
}
    
