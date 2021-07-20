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

#include "Splitter.h"
#include "SplitterEditor.h"

#include "../../UI/EditorViewport.h"

#include "../Settings/ConfigurationObject.h"
#include "../Settings/DataStream.h"

Splitter::Splitter()
    : GenericProcessor("Splitter"),
      destNodeA(nullptr), destNodeB(nullptr), activePath(0)
{
    setProcessorType(PROCESSOR_TYPE_SPLITTER);
    sendSampleCount = false;
}

Splitter::~Splitter()
{

}

AudioProcessorEditor* Splitter::createEditor()
{
    editor = std::make_unique<SplitterEditor>(this, true);

    LOGDD("Creating Splitter editor.");
    return editor.get();
}

void Splitter::updateSettings()
{

    streamsForPathA.clear();
    streamsForPathB.clear();

    isEnabled = false;

    if (sourceNode != nullptr)
    {
        // copy settings from source node

        Array<DataStream*>* streamsToCopy;

        if (sourceNode->isSplitter())
        {
            Splitter* splitter = (Splitter*)sourceNode;
            streamsToCopy = &splitter->getStreamsForDestNode(this);
        }
        else {
            streamsToCopy = &sourceNode->streams;
        }

        for (auto stream : *streamsToCopy)
        {
            if (checkStream(stream, OUTPUT_A))
                streamsForPathA.add(stream);

            if (checkStream(stream, OUTPUT_B))
                streamsForPathB.add(stream);
        }

        for (auto configurationObject : sourceNode->configurationObjects)
        {
            configurationObjects.add(new ConfigurationObject(*configurationObject));
        }

        isEnabled = sourceNode->isEnabled;
    }
}

bool Splitter::checkStream(DataStream* stream, Splitter::Output output)
{
    return true;
}

void Splitter::setPathToProcessor(GenericProcessor* p)
{

    if (destNodeA == p)
    {
        switchIO(0);

    }
    else if (destNodeB == p)
    {
        switchIO(1);
    }
}

void Splitter::setSplitterDestNode(GenericProcessor* dn)
{
    destNode = dn;

    if (activePath == 0)
    {
        LOGDD("Setting destination node A.");
        destNodeA = dn;
    }
    else
    {
        destNodeB = dn;
        LOGDD("Setting destination node B.");

    }
}

void Splitter::switchIO(int destNum)
{

    LOGDD("Switching to dest number ", destNum);

    activePath = destNum;

    if (destNum == 0)
    {
        destNode = destNodeA;
        LOGDD("Dest node: ", getDestNode(0));
    }
    else
    {
        destNode = destNodeB;
        LOGDD("Dest node: ", getDestNode(1));
    }
}

void Splitter::switchIO()
{

    LOGDD("Splitter switching source.");

    if (activePath == 0)
    {
        activePath = 1;
        destNode = destNodeB;
    }
    else
    {
        activePath = 0;
        destNode = destNodeA;
    }

}

int Splitter::getPath()
{
    return activePath;
}

GenericProcessor* Splitter::getDestNode(int path)
{
    if (path == 0)
    {
        return destNodeA;
    } else {
        return destNodeB;
    }
}

Array<DataStream*> Splitter::getStreamsForDestNode(GenericProcessor* node)
{
    if (node == destNodeA)
        return streamsForPathA;

    else if (node == destNodeB)
        return streamsForPathB;

    else
    {
        Array<DataStream*> a;
        return a;
    }
}