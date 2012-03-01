/*
  ==============================================================================

    AccessClass.cpp
    Created: 1 Mar 2012 1:36:31pm
    Author:  jsiegle

  ==============================================================================
*/

#include "AccessClass.h"

#include "UI/UIComponent.h"
#include "UI/FilterViewport.h"
#include "UI/FilterList.h"
#include "UI/DataViewport.h"
#include "UI/Configuration.h"
#include "UI/ControlPanel.h"
#include "UI/MessageCenter.h"
#include "Processors/ProcessorGraph.h"

void AccessClass::setUIComponent(UIComponent* ui_)
{
	ui = ui_;

	fv = ui->getFilterViewport();
	dv = ui->getDataViewport();
	fl = ui->getFilterList();
	pg = ui->getProcessorGraph();
	cp = ui->getControlPanel();
	mc = ui->getMessageCenter();
	cf = ui->getConfiguration();
}
