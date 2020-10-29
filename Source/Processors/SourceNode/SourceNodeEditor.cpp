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


#include "SourceNodeEditor.h"
#include "../SourceNode/SourceNode.h"
#include <stdio.h>
#include "../../Utils/Utils.h"


SourceNodeEditor::SourceNodeEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 170;

    Image im;

    LOGD("I think my name is: ", getName());

    if (getName().equalsIgnoreCase("Intan Demo Board"))
    {
        im = ImageCache::getFromMemory(BinaryData::IntanIcon_png,
                                       BinaryData::IntanIcon_pngSize);
    }
    else if (getName().equalsIgnoreCase("File Reader"))
    {
        im = ImageCache::getFromMemory(BinaryData::FileReaderIcon_png,
                                       BinaryData::FileReaderIcon_pngSize);


    }
    else if (getName().equalsIgnoreCase("Custom FPGA"))
    {
        im = ImageCache::getFromMemory(BinaryData::OpenEphysBoardLogoGray_png,
                                       BinaryData::OpenEphysBoardLogoGray_pngSize);

    }
    else
    {
        im = ImageCache::getFromMemory(BinaryData::DefaultDataSource_png,
                                       BinaryData::DefaultDataSource_pngSize);
    }




    icon = new ImageIcon(im);
    addAndMakeVisible(icon);
    icon->setBounds(50,40,70,70);

    if (getName().equalsIgnoreCase("Custom FPGA"))
    {
        icon->setBounds(20,15,120,120);
    }

    //Array<int> values;
    //values.add(1); values.add(2), values.add(3);

    //createRadioButtons(10, 25, 100, values);
    
    

}

SourceNodeEditor::~SourceNodeEditor()
{
    deleteAllChildren();
}