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

#include "AccessClass.h"
#include "Processors/GenericProcessor/GenericProcessor.h"
#include "Processors/MessageCenter/MessageCenterEditor.h"
#include "Processors/ProcessorGraph/ProcessorGraph.h"

#include "UI/UIComponent.h"

namespace AccessClass
{
namespace
{

UIComponent* ui = nullptr;
EditorViewport* ev = nullptr;
ProcessorList* pl = nullptr;
DataViewport* dv = nullptr;
ProcessorGraph* pg = nullptr;
ControlPanel* cp = nullptr;
MessageCenter* mc = nullptr;
AudioComponent* ac = nullptr;
GraphViewer* gv = nullptr;
PluginManager* pm = nullptr;
std::unique_ptr<ActionBroadcaster> bc;
}

void setUIComponent(UIComponent* ui_)
{
    if (ui != nullptr) return;
    ui = ui_;

    ev = ui->getEditorViewport();
    dv = ui->getDataViewport();
    pl = ui->getProcessorList();
    gv = ui->getGraphViewer();
    
}

void setProcessorGraph(ProcessorGraph* pg_)
{
    if (pg != nullptr) return;
        pg = pg_;

    pm = pg->getPluginManager();
    mc = pg->getMessageCenter();
    
    bc = std::make_unique<ActionBroadcaster>();
    bc->addActionListener(mc);
}

void setAudioComponent(AudioComponent* ac_)
{
    if (ac != nullptr) return;
    ac = ac_;
}

void setControlPanel(ControlPanel* cp_)
{
    if (cp != nullptr) return;
    cp = cp_;
}



void shutdownBroadcaster()
{
    bc = nullptr;
}

/** Returns a pointer to the application's EditorViewport. */
EditorViewport* getEditorViewport()
{
    return ev;
}

/** Returns a pointer to the application's DataViewport. */
DataViewport* getDataViewport()
{
    return dv;
}

/** Returns a pointer to the application's ProcessorList. */
ProcessorList* getProcessorList()
{
    return pl;
}

/** Returns a pointer to the application's ProcessorGraph. */
ProcessorGraph* getProcessorGraph()
{
    return pg;
}

/** Returns a pointer to the application's DataViewport. */
ControlPanel* getControlPanel()
{
    return cp;
}

/** Returns a pointer to the application's MessageCenter. */
MessageCenter* getMessageCenter()
{
    return mc;
}

/** Returns a pointer to the application's UIComponent. */
UIComponent* getUIComponent()
{
    return ui;
}

/** Returns a pointer to the application's AudioComponent. */
AudioComponent* getAudioComponent()
{
    return ac;
}

/** Returns a pointer to the application's GraphViewer. */
GraphViewer* getGraphViewer()
{
    return gv;
}

PluginManager* getPluginManager()
{
    return pm;
}

ActionBroadcaster* getBroadcaster()
{
    return bc.get();
}

void clearAccessClassStateForTesting() {
    ui = nullptr;
    ev = nullptr;
    pl = nullptr;
    dv = nullptr;
    pg = nullptr;
    cp = nullptr;
    mc = nullptr;
    ac = nullptr;
    gv = nullptr;
    pm = nullptr;
    bc.reset();
}


MidiBuffer* ExternalProcessorAccessor::getMidiBuffer(GenericProcessor* proc)
{
	return proc->m_currentMidiBuffer;
}


void ExternalProcessorAccessor::injectNumSamples(GenericProcessor *proc, uint16_t dataStream, uint32_t numSamples) {
    proc->numSamplesInBlock[dataStream] = numSamples;
}

//**Set the MessageCenter for testing only**//
void setMessageCenter(MessageCenter * mc_){
    if(pg != nullptr) return;
    mc = mc_;
}

}
