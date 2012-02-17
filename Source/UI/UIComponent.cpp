/*
  ==============================================================================

    UIComponent.cpp
    Created: 30 Apr 2011 8:33:05pm
    Author:  jsiegle

  ==============================================================================
*/

#include "UIComponent.h"
#include <stdio.h>

UIComponent::UIComponent (MainWindow* mainWindow_, ProcessorGraph* pgraph, AudioComponent* audio_) 
	: processorGraph(pgraph), audio(audio_), mainWindow(mainWindow_)

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

	filterViewportButton = new FilterViewportButton(this);
	addAndMakeVisible(filterViewportButton);

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
		if (filterList->isOpen() && filterViewportButton->isOpen())
			dataViewport->setBounds(202,40,w-207,h-235);
		else if (!filterList->isOpen() && filterViewportButton->isOpen())
			dataViewport->setBounds(6,40,w-11,h-235);
		else if (filterList->isOpen() && !filterViewportButton->isOpen())
			dataViewport->setBounds(202,40,w-207,h-85);
		else	
			dataViewport->setBounds(6,40,w-11,h-85);
	}

	if (filterViewportButton != 0)
	{
		filterViewportButton->setBounds(w-230, h-40, 225, 35);
	}
	
	if (filterViewport != 0) {
		if (filterViewportButton->isOpen() && !filterViewport->isVisible())
			filterViewport->setVisible(true);
		else if (!filterViewportButton->isOpen() && filterViewport->isVisible())
			filterViewport->setVisible(false);

		filterViewport->setBounds(6,h-190,w-11,150);
	}

	if (controlPanel != 0)
		controlPanel->setBounds(201,6,w-210,32);

	if (filterList != 0) {
		if (filterList->isOpen())
			if (filterViewportButton->isOpen())
				filterList->setBounds(5,5,195,h-200);
			else
				filterList->setBounds(5,5,195,h-50);
		else
			filterList->setBounds(5,5,195,34);
	}

	if (messageCenter != 0)
		messageCenter->setBounds(6,h-35,w-241,30);

}

void UIComponent::disableCallbacks()
{
	//sendActionMessage("Data acquisition terminated.");
	controlPanel->disableCallbacks();
}

void UIComponent::childComponentChanged()
{
	resized();
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// MENU BAR METHODS

const StringArray UIComponent::getMenuBarNames() {

	const char* const names[] = { "File", "Edit", "Help" };

    return StringArray (names);

}

const PopupMenu UIComponent::getMenuForIndex(int menuIndex, const String& menuName)
{
	 //ApplicationCommandManager* commandManager = &(mainWindow->commandManager);

     PopupMenu menu;

     if (menuIndex == 0)
     {
     	menu.addItem (0, "Load configuration");
     	menu.addItem (1, "Save configuration");
     } else if (menuIndex == 1)
     {
     	menu.addItem (0, "Clear signal chain");
     } else if (menuIndex == 2)
     {
     	menu.addItem (0, "Show help...");
     }

     return menu;

}

void UIComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
	
 //
}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

FilterViewportButton::FilterViewportButton(UIComponent* ui) : UI(ui)
{
	open = true;

	const unsigned char* buffer = reinterpret_cast<const unsigned char*>(BinaryData::cpmono_light_otf);
	size_t bufferSize = BinaryData::cpmono_light_otfSize;

	font = new FTPixmapFont(buffer, bufferSize);
	
}

FilterViewportButton::~FilterViewportButton()
{
	
}

void FilterViewportButton::newOpenGLContextCreated()
{
	
	glMatrixMode (GL_PROJECTION);

	glLoadIdentity();
	glOrtho (0, 1, 1, 0, 0, 1);
	glMatrixMode (GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);

	glClearColor(0.23f, 0.23f, 0.23f, 1.0f); 

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void FilterViewportButton::renderOpenGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	drawName();
	drawButton();
}

void FilterViewportButton::drawName()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glRasterPos2f(8.0/getWidth(),0.75f);
	font->FaceSize(23);
	font->Render("SIGNAL CHAIN");
	

	
}

void FilterViewportButton::drawButton()
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glLineWidth(1.0f);

	glBegin(GL_LINE_LOOP);

	if (open)
	{
		glVertex2f(0.90,0.65);
		glVertex2f(0.925,0.35);
	} else {
		glVertex2f(0.95,0.35);
		glVertex2f(0.90,0.5);
	}
	glVertex2f(0.95,0.65);
	glEnd();

}

void FilterViewportButton::mouseDown(const MouseEvent& e)
{
	open = !open;
	UI->childComponentChanged();
	repaint();

}