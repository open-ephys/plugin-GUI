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


#include "FilterList.h"
#include <stdio.h>

#include "UIComponent.h"


FilterList::FilterList() : isDragging(false),
                           itemHeight(32),
                           subItemHeight(22),
                           totalHeight(800),
                           xBuffer(1),
                           yBuffer(1)
{

	FilterListItem* sources = new FilterListItem("Sources");
	sources->addSubItem(new FilterListItem("Intan Demo Board"));
	sources->addSubItem(new FilterListItem("Signal Generator"));
	//sources->addSubItem(new FilterListItem("Custom FPGA"));
	//sources->addSubItem(new FilterListItem("File Reader"));
	sources->addSubItem(new FilterListItem("Event Generator"));

	FilterListItem* filters = new FilterListItem("Filters");
	filters->addSubItem(new FilterListItem("Bandpass Filter"));
	//filters->addSubItem(new FilterListItem("Resampler"));
	//filters->addSubItem(new FilterListItem("Spike Detector"));

	FilterListItem* sinks = new FilterListItem("Sinks");
	sinks->addSubItem(new FilterListItem("LFP Viewer"));
	//sinks->addSubItem(new FilterListItem("Spike Display"));
	sinks->addSubItem(new FilterListItem("WiFi Output"));

	//FilterListItem* utilities = new FilterListItem("Utilities");
	//utilities->addSubItem(new FilterListItem("Splitter"));
	//utilities->addSubItem(new FilterListItem("Merger"));

	baseItem = new FilterListItem("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	//baseItem->addSubItem(utilities);

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

FilterList::~FilterList()
{
	deleteAndZero(baseItem);
}

bool FilterList::isOpen()
{
	return baseItem->isOpen();
}


void FilterList::newOpenGLContextCreated()
{

	setUp2DCanvas();
	activateAntiAliasing();

	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	resized();

}

void FilterList::renderOpenGL()
{
	
	glClear(GL_COLOR_BUFFER_BIT); // clear buffers to preset values
	drawItems();
	drawScrollBars();
}


void FilterList::drawItems()
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

void FilterList::drawItem(FilterListItem* item)
{
	
	glColor4f(item->color.getFloatRed(),
		      item->color.getFloatGreen(),
		      item->color.getFloatBlue(),
		      1.0f);

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

void FilterList::drawItemName(FilterListItem* item)
{

	String name; 

	glColor4f(1.0f,1.0f,1.0f,1.0f);

	float offsetX, offsetY;

	if (item->getNumSubItems() == 0) 
	{
		if (item->isSelected())
		{
			glRasterPos2f(9.0/getWidth(),0.72);
			getFont(String("cpmono-plain"))->FaceSize(15);
			getFont(String("cpmono-plain"))->Render(">");
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
		getFont(String("cpmono-plain"))->FaceSize(15);
		getFont(String("cpmono-plain"))->Render(name);
	} else {
		getFont(String("cpmono-light"))->FaceSize(23);
		getFont(String("cpmono-light"))->Render(name);
	}
}

void FilterList::drawButton(bool isOpen)
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

void FilterList::clearSelectionState()
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

FilterListItem* FilterList::getListItemForYPos(int y)
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

void FilterList::setViewport(bool hasSubItems)
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

int FilterList::getTotalHeight()
{
 	return totalHeight;
}

void FilterList::resized() {canvasWasResized();}

void FilterList::mouseDown(const MouseEvent& e) 
{

	isDragging = false;

	Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	//std::cout << xcoord << " " << ycoord << std::endl;

	FilterListItem* fli = getListItemForYPos(ycoord);

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
			fli->reverseOpenState();
		}

		if (fli == baseItem)
		{
			if (fli->isOpen()) {
				getUIComponent()->childComponentChanged();
			}
			else
			{
				getUIComponent()->childComponentChanged();
				//setBounds(0,0,225,itemHeight + 2*yBuffer); 
				totalHeight = itemHeight + 2*yBuffer;
			}
			
		}
	}

	mouseDownInCanvas(e);

	repaint();
}

void FilterList::mouseDrag(const MouseEvent& e) 
{

	if (e.getMouseDownX() < getWidth()-getScrollBarWidth() && !(isDragging))
	{

		FilterListItem* fli = getListItemForYPos(e.getMouseDownY());

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
						g.setColour (fli->color);
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

	mouseDragInCanvas(e);
}

void FilterList::mouseMove(const MouseEvent& e) {mouseMoveInCanvas(e);}
void FilterList::mouseUp(const MouseEvent& e) 	{mouseUpInCanvas(e);}
void FilterList::mouseWheelMove(const MouseEvent& e, float a, float b) {mouseWheelMoveInCanvas(e,a,b);}


FilterListItem::FilterListItem(const String& name_) : name(name_), open(true), selected(false)
{
}

FilterListItem::~FilterListItem()
{ }

bool FilterListItem::hasSubItems()
{
	if (subItems.size() > 0)
	{
		return true;
	} else {
		return false;
	}
}

int FilterListItem::getNumSubItems()
{
	return subItems.size();
}

FilterListItem* FilterListItem::getSubItem (int index)
{
	return subItems[index];
}

void FilterListItem::clearSubItems()
{
	subItems.clear();
}

void FilterListItem::addSubItem (FilterListItem* newItem)
{
	subItems.add(newItem);
}

void FilterListItem::removeSubItem (int index)
{
	subItems.remove(index);
}

bool FilterListItem::isOpen()
{
	return open;
}

void FilterListItem::setOpen(bool t)
{
	open = t;
}

const String& FilterListItem::getName()
{
	return name;
}


const String& FilterListItem::getParentName()
{
	return parentName;
}

void FilterListItem::setParentName(const String& name)
{
	parentName = name;

	if (parentName.equalsIgnoreCase("Processors"))
	{
		color = Colour(59, 59, 59);

	} else if (parentName.equalsIgnoreCase("Filters"))
	{
		color = Colour(255, 89, 0);
	} else if (parentName.equalsIgnoreCase("Sinks"))
	{
		color = Colour(255, 149, 0);
	} else if (parentName.equalsIgnoreCase("Sources"))
	{
		color = Colour(255, 0, 0);

	} else {
		color = Colour(90, 80, 80);
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