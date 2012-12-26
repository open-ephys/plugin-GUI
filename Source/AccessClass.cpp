 /*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2012 Open Ephys

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

#include "UI/UIComponent.h"
#include "UI/EditorViewport.h"
#include "UI/ProcessorList.h"
#include "UI/DataViewport.h"
#include "UI/ControlPanel.h"
#include "UI/MessageCenter.h"
#include "Audio/AudioComponent.h"
#include "Processors/ProcessorGraph.h"

void AccessClass::setUIComponent(UIComponent* ui_)
{

	ui = ui_;

	ev = ui->getEditorViewport();
	dv = ui->getDataViewport();
	pl = ui->getProcessorList();
	pg = ui->getProcessorGraph();
	cp = ui->getControlPanel();
	mc = ui->getMessageCenter();
	ac = ui->getAudioComponent();

	addActionListener(mc);

    updateChildComponents();
}
