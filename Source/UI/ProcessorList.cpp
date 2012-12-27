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


#include "ProcessorList.h"
#include <stdio.h>

#include "UIComponent.h"


ProcessorList::ProcessorList() : isDragging(false),
                           itemHeight(32),
                           subItemHeight(22),
                           totalHeight(800),
                           xBuffer(1),
                           yBuffer(1)
{


	setColour(PROCESSOR_COLOR, Colour(59, 59, 59));
	setColour(FILTER_COLOR, Colour(41, 76, 158));//Colour(255, 89, 0));
	setColour(SINK_COLOR, Colour(93, 125, 199));//Colour(255, 149, 0));
	setColour(SOURCE_COLOR, Colour(48, 67, 112)); //Colour(255, 0, 0));
	setColour(UTILITY_COLOR, Colour(90, 80, 80));

	ProcessorListItem* sources = new ProcessorListItem("Sources");
	sources->addSubItem(new ProcessorListItem("Intan Demo Board"));
	sources->addSubItem(new ProcessorListItem("Signal Generator"));
	sources->addSubItem(new ProcessorListItem("Custom FPGA"));
	sources->addSubItem(new ProcessorListItem("File Reader"));
	sources->addSubItem(new ProcessorListItem("Event Generator"));

	ProcessorListItem* filters = new ProcessorListItem("Filters");
	filters->addSubItem(new ProcessorListItem("Bandpass Filter"));
	filters->addSubItem(new ProcessorListItem("Event Detector"));
	filters->addSubItem(new ProcessorListItem("Spike Detector"));
	filters->addSubItem(new ProcessorListItem("Resampler"));
	filters->addSubItem(new ProcessorListItem("Phase Detector"));
	filters->addSubItem(new ProcessorListItem("Digital Reference"));

	ProcessorListItem* sinks = new ProcessorListItem("Sinks");
	sinks->addSubItem(new ProcessorListItem("LFP Viewer"));
	sinks->addSubItem(new ProcessorListItem("Spike Viewer"));
	sinks->addSubItem(new ProcessorListItem("WiFi Output"));
	sinks->addSubItem(new ProcessorListItem("Arduino Output"));
	sinks->addSubItem(new ProcessorListItem("FPGA Output"));

	ProcessorListItem* utilities = new ProcessorListItem("Utilities");
	utilities->addSubItem(new ProcessorListItem("Splitter"));
	utilities->addSubItem(new ProcessorListItem("Merger"));
	utilities->addSubItem(new ProcessorListItem("Record Controller"));

	baseItem = new ProcessorListItem("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	baseItem->addSubItem(utilities);

	// set parent names / colors
	baseItem->setParentName("Processors");

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{

		const String category = baseItem->getSubItem(n)->getName();
		baseItem->getSubItem(n)->setParentName(category);

			for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
			{

				baseItem->getSubItem(n)->getSubItem(m)->setParentName(category);// = category;

			}
			
	}

}

ProcessorList::~ProcessorList()
{
	deleteAndZero(baseItem);
}

bool ProcessorList::isOpen()
{
	return baseItem->isOpen();
}


void ProcessorList::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();

	setClearColor(black);

	resized();
}

void ProcessorList::renderOpenGL()
{
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // clear buffers to preset values

	drawItems();
	//drawScrollBars();
	
    //glFlush();
    //swapBuffers();
}


void ProcessorList::drawItems()
{
	int itemNum = 0;
	totalHeight = yBuffer;

	setViewport(true);

	category = baseItem->getName();

	drawItem(baseItem);

	if (baseItem->isOpen())
	{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			setViewport(baseItem->hasSubItems());
			category = baseItem->getSubItem(n)->getName();
			drawItem(baseItem->getSubItem(n));
			
			if (baseItem->getSubItem(n)->isOpen())
			{
				for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
				{

					setViewport(baseItem->
								 getSubItem(n)->
								 getSubItem(m)->
								 hasSubItems());
					drawItem(baseItem->getSubItem(n)->getSubItem(m));

				}
			}			
		}
	}

}

void ProcessorList::drawItem(ProcessorListItem* item)
{

	Colour c = findColour(item->colorId);

	glColor4f(c.getFloatRed(),
		      c.getFloatGreen(),
		      c.getFloatBlue(),
		      1.0f);

	// see if this helps drawing issues on Windows:
	// (draw rectangle below polygon)
	glRectf(0.0, 0.0, 1.0, 1.0);

	glBegin(GL_POLYGON);
	glVertex2f(0,0);
	glVertex2f(1,0);
	glVertex2f(1,1);
	glVertex2f(0,1);
	glEnd();

	drawItemName(item);

	if (item->hasSubItems())
	{
		drawButton(item->isOpen());
	}

}

void ProcessorList::drawItemName(ProcessorListItem* item)
{

	String name; 

	glColor4f(1.0f,1.0f,1.0f,1.0f);

	float offsetX, offsetY;

	if (item->getNumSubItems() == 0) 
	{
		if (item->isSelected())
		{
			glRasterPos2f(9.0/getWidth(),0.72);
			getFont(cpmono_plain)->FaceSize(15);
			getFont(cpmono_plain)->Render(">");
		}

		name = item->getName();

		offsetX = 20.0f;

		offsetY = 0.72f;
	}
	else {
		name = item->getName().toUpperCase();
		offsetX = 5.0f;
		offsetY = 0.75f;
	}

	
	glRasterPos2f(offsetX/getWidth(),offsetY);

	if (item->getNumSubItems() == 0) {
		getFont(cpmono_plain)->FaceSize(15);
		getFont(cpmono_plain)->Render(name);
	} else {
		getFont(cpmono_light)->FaceSize(23);
		getFont(cpmono_light)->Render(name);
	}
}

void ProcessorList::drawButton(bool isOpen)
{
	glColor4f(1.0f,1.0f,1.0f,1.0f);
	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);

	if (isOpen)
	{
		glVertex2f(0.875,0.35);
		glVertex2f(0.9,0.65);
	} else {
		glVertex2f(0.925,0.65);
		glVertex2f(0.875,0.5);
	}
	glVertex2f(0.925,0.35);
	glEnd();

}

void ProcessorList::clearSelectionState()
{
	baseItem->setSelected(false);

	for (int n = 0; n < baseItem->getNumSubItems(); n++)
	{
		baseItem->getSubItem(n)->setSelected(false);

		for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
		{
			baseItem->getSubItem(n)->getSubItem(m)->setSelected(false);
		}
	}
}

ProcessorListItem* ProcessorList::getListItemForYPos(int y)
{
	int bottom = (yBuffer + itemHeight) - getScrollAmount();

	//std::cout << "Bottom: " << bottom << std::endl;
	//std::cout << "Y coordinate: " << y << std::endl;

	if (y < bottom)
	{
		return baseItem;

	} else {
		
		if (baseItem->isOpen())
		{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			bottom += (yBuffer + itemHeight);

			if (y < bottom)
			{
				return baseItem->getSubItem(n);
			}
				
			if (baseItem->getSubItem(n)->isOpen())
				{
					for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
					{
						bottom += (yBuffer + subItemHeight);

						if (y < bottom)
						{
							return baseItem->getSubItem(n)->getSubItem(m);
						}

					}
				}			
			}
		}

	}

	return 0;

}

void ProcessorList::setViewport(bool hasSubItems)
{

	int height;

	if (hasSubItems)
	{
		height = itemHeight;
	} else {
		height = subItemHeight;
	}

	glViewport(xBuffer,
			   getHeight()-(totalHeight) - height + getScrollAmount(),
	           getWidth()-2*xBuffer,
	           height);

	totalHeight += yBuffer + height;
}

int ProcessorList::getTotalHeight()
{
 	return totalHeight;
}

void ProcessorList::toggleState()
{
	ProcessorListItem* fli = getListItemForYPos(0);
	fli->reverseOpenState();
	getUIComponent()->childComponentChanged();
	repaint();
}

void ProcessorList::mouseDownInCanvas(const MouseEvent& e) 
{

	isDragging = false;

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	//std::cout << xcoord << " " << ycoord << std::endl;

	ProcessorListItem* fli = getListItemForYPos(ycoord);

	if (fli != 0) 
	{
		//std::cout << "Selecting: " << fli->getName() << std::endl;
		if (!fli->hasSubItems()){
			clearSelectionState();
			fli->setSelected(true);
		}
			
	} else {
		//std::cout << "No selection." << std::endl;
	}

	if (fli != 0) {
		if (xcoord < getWidth() - getScrollBarWidth())
		{
			if (e.mods.isRightButtonDown() || e.mods.isCtrlDown())
			{

				if (fli->getName().equalsIgnoreCase("Sources"))
				{
					currentColor = SOURCE_COLOR;
				} else if (fli->getName().equalsIgnoreCase("Filters"))
				{
					currentColor = FILTER_COLOR;
				} else if (fli->getName().equalsIgnoreCase("Utilities"))
				{
					currentColor = UTILITY_COLOR;
				} else if (fli->getName().equalsIgnoreCase("Sinks"))
				{
					currentColor = SINK_COLOR;
				} else {
					return;
				}

				int options=0;
				options += (0 << 0); // showAlpha
				options += (0 << 1); // showColorAtTop
				options += (0 << 2); // showSliders
				options += (1 << 3); // showColourSpace

		        ColourSelector colourSelector(options);
		        colourSelector.setName ("background");
		        colourSelector.setCurrentColour (findColour (currentColor));
		        colourSelector.addChangeListener (this);
		        colourSelector.addChangeListener (getProcessorGraph());
		        colourSelector.setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
		        colourSelector.setSize (300, 275);

		        CallOutBox callOut (colourSelector, *fli, 0);//*this, 0);
		        callOut.setTopLeftPosition (e.getScreenX(), e.getScreenY());
		        callOut.setArrowSize(0.0f);

		        callOut.runModalLoop();

			} else {
				fli->reverseOpenState();
			}
		}

		if (fli == baseItem)
		{
			if (fli->isOpen()) {
				getUIComponent()->childComponentChanged();
			}
			else
			{
				getUIComponent()->childComponentChanged();
				totalHeight = itemHeight + 2*yBuffer;
			}
			
		}
	}

	repaint();
}

void ProcessorList::changeListenerCallback(ChangeBroadcaster* source)
{
	ColourSelector* cs = dynamic_cast <ColourSelector*> (source);

    setColour (currentColor, cs->getCurrentColour());

    repaint();

}

void ProcessorList::mouseDragInCanvas(const MouseEvent& e) 
{

	if (e.getMouseDownX() < getWidth()-getScrollBarWidth() && !(isDragging))
	{

		ProcessorListItem* fli = getListItemForYPos(e.getMouseDownY());

		if (fli != 0)
		{

			if (!fli->hasSubItems())
			{
				isDragging = true;

				String b = fli->getParentName();
				b += "/";
				b += fli->getName();

				const String dragDescription = b;

				//std::cout << dragDescription << std::endl;

				if (dragDescription.isNotEmpty())
				{
					DragAndDropContainer* const dragContainer
						= DragAndDropContainer::findParentDragContainerFor (this);

					if (dragContainer != 0)
					{
						//pos.setSize (pos.getWidth(), 10);

						Image dragImage (Image::ARGB, 100, 15, true);

						Graphics g(dragImage);
						g.setColour (findColour(fli->colorId));
						g.fillAll();
						g.setColour(Colours::white);
						g.setFont(14);
						g.drawSingleLineText(fli->getName(),10,12);//,75,15,Justification::centredRight,true);

						dragImage.multiplyAllAlphas(0.6f);

						Point<int> imageOffset (20,10);
						dragContainer->startDragging(dragDescription, this,
											         dragImage, true, &imageOffset);
					}
				}
			}
		}
	}

}

ProcessorListItem::ProcessorListItem(const String& name_) : name(name_), open(true), selected(false)
{
}

ProcessorListItem::~ProcessorListItem()
{ }

bool ProcessorListItem::hasSubItems()
{
	if (subItems.size() > 0)
	{
		return true;
	} else {
		return false;
	}
}

int ProcessorListItem::getNumSubItems()
{
	return subItems.size();
}

ProcessorListItem* ProcessorListItem::getSubItem (int index)
{
	return subItems[index];
}

void ProcessorListItem::clearSubItems()
{
	subItems.clear();
}

void ProcessorListItem::addSubItem (ProcessorListItem* newItem)
{
	subItems.add(newItem);
}

void ProcessorListItem::removeSubItem (int index)
{
	subItems.remove(index);
}

bool ProcessorListItem::isOpen()
{
	return open;
}

void ProcessorListItem::setOpen(bool t)
{
	open = t;
}

const String& ProcessorListItem::getName()
{
	return name;
}


const String& ProcessorListItem::getParentName()
{
	return parentName;
}

void ProcessorListItem::setParentName(const String& name)
{
	parentName = name;

	enum {
		PROCESSOR_COLOR = 801,
		FILTER_COLOR = 802,
		SINK_COLOR = 803,
		SOURCE_COLOR = 804,
		UTILITY_COLOR = 805,
	};

	if (parentName.equalsIgnoreCase("Processors"))
	{
		colorId = PROCESSOR_COLOR;
	} else if (parentName.equalsIgnoreCase("Filters"))
	{
		colorId = FILTER_COLOR;
	} else if (parentName.equalsIgnoreCase("Sinks"))
	{
		colorId = SINK_COLOR;
	} else if (parentName.equalsIgnoreCase("Sources"))
	{
		colorId = SOURCE_COLOR;
	} else {
		colorId = UTILITY_COLOR;
	}
}

	// Blue slate:
	// if (parentName.equalsIgnoreCase("Processors"))
	// {
	// 	color = Colour(59, 59, 59);
	// } else if (parentName.equalsIgnoreCase("Filters"))
	// {
	// 	color = Colour(82, 101, 163);
	// } else if (parentName.equalsIgnoreCase("Sinks"))
	// {
	// 	color = Colour(48, 61, 102);
	// } else if (parentName.equalsIgnoreCase("Sources"))
	// {
	// 	color = Colour(151, 170, 230);

	// } else {
	// 	color = Colour(20, 37, 92);
	// }