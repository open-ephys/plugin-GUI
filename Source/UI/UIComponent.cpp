/*
  ==============================================================================

    UIComponent.cpp
    Created: 30 Apr 2011 8:33:05pm
    Author:  jsiegle

  ==============================================================================
*/

#include "UIComponent.h"
#include <stdio.h>

UIComponent::UIComponent (ProcessorGraph* pgraph, AudioComponent* audio_) 
	: processorGraph(pgraph), audio(audio_)

{	

	processorGraph->setUIComponent(this);

	infoLabel = new InfoLabel();

	dataViewport = new DataViewport ();
	addChildComponent(dataViewport);
	dataViewport->addTabToDataViewport("Info",infoLabel);

	std::cout << "Created data viewport." << std::endl;

	filterViewport = new FilterViewport(processorGraph, dataViewport);
	processorGraph->setFilterViewport(filterViewport);
	
	addAndMakeVisible(filterViewport);

	std::cout << "Created filter viewport." << std::endl;

	controlPanel = new ControlPanel(processorGraph, audio);
	addAndMakeVisible(controlPanel);

	std::cout << "Created control panel." << std::endl;

	filterList = new FilterList();
	filterList->setUIComponent(this);
	addAndMakeVisible(filterList);

	std::cout << "Created filter list." << std::endl;

	messageCenter = new MessageCenter();
	processorGraph->addActionListener(messageCenter);
	addActionListener(messageCenter);
	addAndMakeVisible(messageCenter);

	std::cout << "Created message center." << std::endl;

	config = new Configuration();
	processorGraph->setConfiguration(config);

	std::cout << "Created configuration object." << std::endl;

	setBounds(0,0,500,400);

	std::cout << "Component width = " << getWidth() << std::endl;
	std::cout << "Component height = " << getHeight() << std::endl;

	std::cout << "Finished UI stuff." << std::endl;

	std::cout << "UI component data viewport: " << dataViewport << std::endl;

	processorGraph->loadState();
	
}

UIComponent::~UIComponent()
{
	deleteAllChildren();

	deleteAndZero(config);
	deleteAndZero(infoLabel);

	processorGraph = 0;
	audio = 0;
}

void UIComponent::resized()
{
	
	int w = getWidth();
	int h = getHeight();
	
	if (dataViewport != 0) {
		if (filterList->isOpen())
			dataViewport->setBounds(202,40,w-207,h-230);
		else 
			dataViewport->setBounds(6,40,w-11,h-230);
	}
	
	if (filterViewport != 0)
		filterViewport->setBounds(10,h-175,w-20,125);

	if (controlPanel != 0)
		controlPanel->setBounds(201,6,w-210,32);

	if (filterList != 0) {
		if (filterList->isOpen())
			filterList->setBounds(5,5,195,h-205);
		else
			filterList->setBounds(5,5,195,34);
	}

	if (messageCenter != 0)
		messageCenter->setBounds(40,h-40,w-160,30);

}

void UIComponent::disableCallbacks()
{
	//sendActionMessage("Data acquisition terminated.");
	controlPanel->disableCallbacks();
}

void UIComponent::filterListOpened()
{
	resized();
}

void UIComponent::filterListClosed()
{
	
	resized();
}