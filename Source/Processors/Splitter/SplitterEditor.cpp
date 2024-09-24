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

#include "SplitterEditor.h"

#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"

#include "../Editors/StreamSelector.h"
#include "../Settings/DataStream.h"

SplitterEditor::SplitterEditor (GenericProcessor* parentNode)
    : GenericEditor (parentNode)

{
    desiredWidth = 90;

    pipelineSelectorA = std::make_unique<ImageButton> ("Pipeline A");

    Image normalImageA = ImageCache::getFromMemory (BinaryData::PipelineB01_png, BinaryData::PipelineB01_pngSize);
    Image downImageA = ImageCache::getFromMemory (BinaryData::PipelineA01_png, BinaryData::PipelineA01_pngSize);
    Image normalImageB = ImageCache::getFromMemory (BinaryData::PipelineA02_png, BinaryData::PipelineA02_pngSize);
    Image downImageB = ImageCache::getFromMemory (BinaryData::PipelineB02_png, BinaryData::PipelineB02_pngSize);

    pipelineSelectorA->setImages (true, true, true, normalImageA, 1.0f, Colours::white.withAlpha (0.0f), normalImageA, 1.0f, Colours::black.withAlpha (0.0f), downImageA, 1.0f, Colours::white.withAlpha (0.0f));

    pipelineSelectorA->addListener (this);
    pipelineSelectorA->setBounds (-10, 25, 95, 50);
    pipelineSelectorA->setToggleState (true, dontSendNotification);
    addAndMakeVisible (pipelineSelectorA.get());

    pipelineSelectorB = std::make_unique<ImageButton> ("Pipeline B");

    pipelineSelectorB->setImages (true, true, true, normalImageB, 1.0f, Colours::white.withAlpha (0.0f), normalImageB, 1.0f, Colours::black.withAlpha (0.0f), downImageB, 1.0f, Colours::white.withAlpha (0.0f));

    pipelineSelectorB->addListener (this);
    pipelineSelectorB->setBounds (-10, 75, 95, 50);
    pipelineSelectorB->setToggleState (false, dontSendNotification);
    addAndMakeVisible (pipelineSelectorB.get());
}

SplitterEditor::~SplitterEditor()
{
}

void SplitterEditor::buttonClicked (Button* button)
{
    if (button == pipelineSelectorA.get())
    {
        AccessClass::getEditorViewport()->switchIO (getProcessor(), 0);
    }
    else if (button == pipelineSelectorB.get())
    {
        AccessClass::getEditorViewport()->switchIO (getProcessor(), 1);
    }
}

void SplitterEditor::switchDest (int dest)
{
    if (dest == 0)
    {
        pipelineSelectorA->setToggleState (true, dontSendNotification);
        pipelineSelectorB->setToggleState (false, dontSendNotification);

        Splitter* processor = (Splitter*) getProcessor();
        processor->switchIO (0);
    }
    else if (dest == 1)
    {
        pipelineSelectorB->setToggleState (true, dontSendNotification);
        pipelineSelectorA->setToggleState (false, dontSendNotification);

        Splitter* processor = (Splitter*) getProcessor();
        processor->switchIO (1);
    }
}

void SplitterEditor::switchIO (int dest)
{
    switchDest (dest);

    select();
}

int SplitterEditor::getPathForEditor (GenericEditor* editor)
{
    Splitter* processor = (Splitter*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        if (processor->getDestNode (pathNum) != nullptr)
        {
            LOGDD (" PATH ", pathNum, " editor: ", processor->getDestNode (pathNum)->getEditor()->getName());

            if (processor->getDestNode (pathNum)->getEditor() == editor)
            {
                LOGDD (" MATCHING PATH: ", pathNum);
                return pathNum;
            }
        }
    }

    return -1;
}

Array<GenericEditor*> SplitterEditor::getConnectedEditors()
{
    Array<GenericEditor*> editors;

    Splitter* processor = (Splitter*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        if (processor->getDestNode (pathNum) != nullptr)
            editors.add (processor->getDestNode (pathNum)->getEditor());
        else
            editors.add (nullptr);
    }

    return editors;
}

void SplitterEditor::switchDest()
{
    Splitter* processor = (Splitter*) getProcessor();
    processor->switchIO();

    int path = processor->getPath();

    if (path == 0)
    {
        pipelineSelectorA->setToggleState (true, dontSendNotification);
        pipelineSelectorB->setToggleState (false, dontSendNotification);
    }
    else if (path == 1)
    {
        pipelineSelectorB->setToggleState (true, dontSendNotification);
        pipelineSelectorA->setToggleState (false, dontSendNotification);
    }
}
