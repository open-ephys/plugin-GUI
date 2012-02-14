/*
  ==============================================================================

    FilterList.cpp
    Created: 4 Feb 2012 6:55:05pm
    Author:  jsiegle

  ==============================================================================
*/


#include "FilterList.h"
#include <stdio.h>

#include "UIComponent.h"


FilterList::FilterList() : isDragging(false)
{

	//setBounds(0,0,225,500);

	itemHeight = 32;
	subItemHeight = 22;
	totalHeight = 800;
	xBuffer = 1;
	yBuffer = 1;

	FilterListItem* sources = new FilterListItem("Sources");
	sources->addSubItem(new FilterListItem("Intan Demo Board"));
	sources->addSubItem(new FilterListItem("Signal Generator"));
	//sources->addSubItem(new FilterListItem("Custom FPGA"));
	//sources->addSubItem(new FilterListItem("File Reader"));

	FilterListItem* filters = new FilterListItem("Filters");
	filters->addSubItem(new FilterListItem("Bandpass Filter"));
	//filters->addSubItem(new FilterListItem("Resampler"));
	//filters->addSubItem(new FilterListItem("Spike Detector"));

	FilterListItem* sinks = new FilterListItem("Sinks");
	sinks->addSubItem(new FilterListItem("LFP Viewer"));
	//sinks->addSubItem(new FilterListItem("Spike Display"));

	//FilterListItem* utilities = new FilterListItem("Utilities");
	//utilities->addSubItem(new FilterListItem("Splitter"));
	//utilities->addSubItem(new FilterListItem("Merger"));

	baseItem = new FilterListItem("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	//baseItem->addSubItem(utilities);

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

	// for (int i = 0; i < nChans; i++)
	// {
	// 	bool isSelected = false;

	// 	if (selectedChan == i)
	// 		isSelected = true;

	// 	if (checkBounds(i)) {
	// 		setViewport(i);
	// 		drawBorder(isSelected);
	// 		drawChannelInfo(i,isSelected);
	// 	}	
	// }
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

					baseItem->getSubItem(n)->getSubItem(m)->parentName = category;

				}
			}			
		}
	}

	//totalHeight -= subItemHeight;//(itemHeight+yBuffer)*(itemNum+1);

}

void FilterList::drawItem(FilterListItem* item)
{
	if (category.startsWith("P"))
	{
		glColor4f(0.23f, 0.23f, 0.23f, 1.0f); // [59 59 59]
		item->color = Colour(int(0.23*255.0f),int(0.23*255.0f),int(0.23*255.0f));
	} else if (category.startsWith("So"))
	{
		glColor4f(0.9f, 0.019f, 0.16f, 1.0f); // [232 5 43]
		item->color = Colour(int(0.9*255.0f),int(0.019*255.0f),int(0.16*255.0f));
	} else if (category.startsWith("F"))
	{
		glColor4f(1.0f, 0.5f, 0.0f, 1.0f);
		item->color = Colour(int(1.0*255.0f),int(0.5*255.0f),int(0.0*255.0f));
	} else if (category.startsWith("Si"))
	{
		glColor4f(0.06f, 0.46f, 0.9f, 1.0f);
		item->color = Colour(int(0.06*255.0f),int(0.46*255.0f),int(0.9*255.0f));
	} else if (category.startsWith("U"))
	{
		glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		item->color = Colour(int(0.7*255.0f),int(0.7*255.0f),int(0.7*255.0f));
	} else {
		glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		item->color = Colour(int(0.7*255.0f),int(0.7*255.0f),int(0.7*255.0f));
	}

	glBegin(GL_POLYGON);
	glVertex2f(0,0);
	glVertex2f(1,0);
	glVertex2f(1,1);
	glVertex2f(0,1);
	glEnd();

	// if (item->isSelected())
	// {
	// 	glColor4f(1.0,1.0,1.0,1.0);
	// 	glLineWidth(3.0);
	// 	glBegin(GL_LINE_STRIP);
	// 	glVertex2f(0,0);
	// 	glVertex2f(1,0);
	// 	glVertex2f(1,1);
	// 	glVertex2f(0,1);
	// 	glEnd();
	// }



	drawItemName(item);

	if (item->hasSubItems())
	{
		drawButton(item->isOpen());
	}

	// glBegin(GL_POLYGON);
	// glVertex2f(0,0);
	// glVertex2f(1,0);
	// glVertex2f(1,1);
	// glVertex2f(0,1);
	// glEnd();

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

	//setBounds(0,0,225,itemHeight + 2*yBuffer);

	

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
				UI->childComponentChanged();
			}
			else
			{
				UI->childComponentChanged();
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

				String b = fli->parentName;
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
						//g.drawRect(4,4,50,10);
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

const String FilterListItem::getName()
{
	return name;
}