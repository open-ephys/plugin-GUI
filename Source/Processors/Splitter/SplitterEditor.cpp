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

#include "SplitterEditor.h"

#include "../../AccessClass.h"
#include "../../UI/EditorViewport.h"

#include "../Editors/StreamSelector.h"
#include "../Settings/DataStream.h"


SplitterEditor::SplitterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=true)
    : GenericEditor(parentNode, useDefaultParameterEditors)

{
    desiredWidth = 300;

    pipelineSelectorA = std::make_unique<ImageButton>("Pipeline A");

    Image normalImageA = ImageCache::getFromMemory(BinaryData::PipelineB01_png, BinaryData::PipelineB01_pngSize);
    Image downImageA = ImageCache::getFromMemory(BinaryData::PipelineA01_png, BinaryData::PipelineA01_pngSize);
    Image normalImageB = ImageCache::getFromMemory(BinaryData::PipelineA02_png, BinaryData::PipelineA02_pngSize);
    Image downImageB = ImageCache::getFromMemory(BinaryData::PipelineB02_png, BinaryData::PipelineB02_pngSize);

    pipelineSelectorA->setImages(true, true, true,
                                 normalImageA, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageA, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageA, 1.0f, Colours::white.withAlpha(0.0f));


    pipelineSelectorA->addListener(this);
    pipelineSelectorA->setBounds(-10,25,95,50);
    pipelineSelectorA->setToggleState(true, dontSendNotification);
    addAndMakeVisible(pipelineSelectorA.get());

    pipelineSelectorB = std::make_unique<ImageButton>("Pipeline B");

    pipelineSelectorB->setImages(true, true, true,
                                 normalImageB, 1.0f, Colours::white.withAlpha(0.0f),
                                 normalImageB, 1.0f, Colours::black.withAlpha(0.0f),
                                 downImageB, 1.0f, Colours::white.withAlpha(0.0f));

    pipelineSelectorB->addListener(this);
    pipelineSelectorB->setBounds(-10,75,95,50);
    pipelineSelectorB->setToggleState(false, dontSendNotification);
    addAndMakeVisible(pipelineSelectorB.get());

    streamSelectorA = std::make_unique<StreamSelector>(this);
    streamSelectorA->setBounds(100, 10, 100, 150);
    addAndMakeVisible(streamSelectorA.get());

    streamSelectorB = std::make_unique<StreamSelector>(this);
    streamSelectorB->setBounds(100, 10, 100, 150);
    addChildComponent(streamSelectorB.get());
    streamSelectorB->setVisible(false);

    drawerWidth = 150;
}

SplitterEditor::~SplitterEditor()
{
}

void SplitterEditor::buttonEvent(Button* button)
{
    if (button == pipelineSelectorA.get())
    {
        AccessClass::getEditorViewport()->switchIO(getProcessor(), 0);
    }
    else if (button == pipelineSelectorB.get())
    {
        AccessClass::getEditorViewport()->switchIO(getProcessor(), 1);
    }
}

void SplitterEditor::switchDest(int dest)
{
    if (dest == 0)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);
        
        streamSelectorA->setVisible(true);
        streamSelectorB->setVisible(false);

        Splitter* processor = (Splitter*) getProcessor();
        processor->switchIO(0);

    }
    else if (dest == 1)
    {
        pipelineSelectorB->setToggleState(true, dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

        streamSelectorB->setVisible(true);
        streamSelectorA->setVisible(false);

        Splitter* processor = (Splitter*) getProcessor();
        processor->switchIO(1);

    }

	
}

void SplitterEditor::switchIO(int dest)
{
    switchDest(dest);

    select();
}

int SplitterEditor::getPathForEditor(GenericEditor* editor)
{
    Splitter* processor = (Splitter*) getProcessor();

    for (int pathNum = 0; pathNum < 2; pathNum++)
    {
        if (processor->getDestNode(pathNum) != nullptr)
        {
            LOGDD(" PATH ", pathNum, " editor: ", processor->getDestNode(pathNum)->getEditor()->getName());

            if (processor->getDestNode(pathNum)->getEditor() == editor)
            {
                LOGDD(" MATCHING PATH: ", pathNum);
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
        if (processor->getDestNode(pathNum) != nullptr)
            editors.add(processor->getDestNode(pathNum)->getEditor());
        else
            editors.add(nullptr);
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
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);

        streamSelectorA->setVisible(true);
        streamSelectorB->setVisible(false);

    }
    else if (path == 1)
    {
        pipelineSelectorB->setToggleState(true,dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

        streamSelectorA->setVisible(false);
        streamSelectorB->setVisible(true);

    }
}

void SplitterEditor::startCheck()
{
    incomingStreams.clear();
}

bool SplitterEditor::checkStream(const DataStream* stream, Splitter::Output output)
{
    std::cout << "Splitter checking stream " << stream->getStreamId() << " for output " << output << std::endl;

    if (output == Splitter::Output::OUTPUT_A)
        incomingStreams.add(stream->getStreamId());


    // buttons already exist:
    /*if (output == Splitter::Output::OUTPUT_A)
    {
        for (auto button : streamButtonsA)
        {
            if (button->getStreamId() == stream->getStreamId())
                return button->getToggleState();
        }
    }
    else {
        for (auto button : streamButtonsB)
        {
            if (button->getStreamId() == stream->getStreamId())
                return button->getToggleState();
        }
    }

    // we need to create new buttons (only do it once per stream)
    if (output == Splitter::Output::OUTPUT_A)
    {

        std::cout << "Creating new stream info view for output 0" << std::endl;

        streamSelectorA->add(new StreamInfoView(stream, this));

        std::cout << "Creating new stream button for output 0" << std::endl;
        streamSelectorB->add(new StreamInfoView(stream, this));
    }*/

    return true;

}


void SplitterEditor::updateSettings()
{

    std::cout << "Splitter editor updating settings" << std::endl;

    for (auto streamId : incomingStreams)
    {
        std::cout << "  Found stream " << streamId << std::endl;
    }

    /*for (auto button : streamButtonsA)
    {
        std::cout << "BUTTON A STREAM " << button->getStreamId() << std::endl;

        if (!incomingStreams.contains(button->getStreamId()))
        {
            std::cout << " ...NOT FOUND, REMOVING" << std::endl;
            streamButtonHolderA->remove(button);
            streamButtonsA.removeObject(button);
        }
        else {
            std::cout << " It's ok, keeping" << std::endl;
        }
    }

    for (auto button : streamButtonsB)
    {
        std::cout << "BUTTON B STREAM " << button->getStreamId() << std::endl;


        if (!incomingStreams.contains(button->getStreamId()))
        {
            std::cout << " ...NOT FOUND, REMOVING" << std::endl;
            streamButtonHolderB->remove(button);
            streamButtonsB.removeObject(button);
        }
        else {
            std::cout << " It's ok, keeping" << std::endl;
        }
    }*/

}