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

#include "../Editors/StreamSelectorButton.h"
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

    viewportA = std::make_unique<Viewport>();
    addAndMakeVisible(viewportA.get());
    viewportA->setBounds(100, 22, 140, 110);
    viewportA->setScrollBarsShown(true, false);
    viewportA->setScrollBarThickness(10);

    streamButtonHolderA = std::make_unique<StreamButtonHolder>();
    viewportA->setViewedComponent(streamButtonHolderA.get(), false);

    streamButtonHolderA->setBounds(0, 0, 125, 0);
    streamButtonHolderA->setVisible(true);

    viewportB = std::make_unique<Viewport>();
    addChildComponent(viewportB.get());
    viewportB->setBounds(100, 22, 140, 110);
    viewportB->setScrollBarsShown(true, false);
    viewportB->setScrollBarThickness(10);

    streamButtonHolderB = std::make_unique<StreamButtonHolder>();
    viewportB->setViewedComponent(streamButtonHolderB.get(), false);

    streamButtonHolderB->setBounds(0, 0, 125, 0);
    streamButtonHolderB->setVisible(true);


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
    } else if (streamButtonsA.contains((StreamSelectorButton*) button)
        || streamButtonsB.contains((StreamSelectorButton*)button))
    {
        CoreServices::updateSignalChain(this);
    }
}

void SplitterEditor::switchDest(int dest)
{
    if (dest == 0)
    {
        pipelineSelectorA->setToggleState(true, dontSendNotification);
        pipelineSelectorB->setToggleState(false, dontSendNotification);
        
        viewportA->setVisible(true);
        viewportB->setVisible(false);

        Splitter* processor = (Splitter*) getProcessor();
        processor->switchIO(0);
        

    }
    else if (dest == 1)
    {
        pipelineSelectorB->setToggleState(true, dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

        viewportB->setVisible(true);
        viewportA->setVisible(false);

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

        viewportA->setVisible(true);
        viewportB->setVisible(false);

    }
    else if (path == 1)
    {
        pipelineSelectorB->setToggleState(true,dontSendNotification);
        pipelineSelectorA->setToggleState(false, dontSendNotification);

        viewportA->setVisible(false);
        viewportB->setVisible(true);

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
    if (output == Splitter::Output::OUTPUT_A)
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

        StreamSelectorButton* newButtonA = new StreamSelectorButton(stream);
        std::cout << "Creating new stream button for output 0" << std::endl;

        streamButtonsA.add(newButtonA);
        newButtonA->addListener(this);
        streamButtonHolderA->add(newButtonA);

        StreamSelectorButton* newButtonB = new StreamSelectorButton(stream);
        std::cout << "Creating new stream button for output 1" << std::endl;

        streamButtonsB.add(newButtonB);
        newButtonB->addListener(this);
        streamButtonHolderB->add(newButtonB);
    }

}


void SplitterEditor::updateSettings()
{

    std::cout << "Splitter editor updating settings" << std::endl;

    for (auto streamId : incomingStreams)
    {
        std::cout << "  Found stream " << streamId << std::endl;
    }

    for (auto button : streamButtonsA)
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
    }

}