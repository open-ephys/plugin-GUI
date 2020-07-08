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


#include "ProcessorList.h"
#include <stdio.h>

#include "UIComponent.h"
#include "../AccessClass.h"
#include "../Processors/ProcessorManager/ProcessorManager.h"
#include "../Processors/ProcessorGraph/ProcessorGraph.h"


enum colorIds
{
	PROCESSOR_COLOR = 801,
	FILTER_COLOR = 802,
	SINK_COLOR = 803,
	SOURCE_COLOR = 804,
	UTILITY_COLOR = 805,
	RECORD_COLOR = 806,
	AUDIO_COLOR = 807
};

	ProcessorList::ProcessorList()
: isDragging(false), totalHeight(800), itemHeight(32), subItemHeight(22),
	xBuffer(1), yBuffer(1)
{

	listFontLight = Font("Default Light", 25, Font::plain);
	listFontPlain = Font("Default", 20, Font::plain);

	setColour(PROCESSOR_COLOR, Colour(59, 59, 59));
	setColour(FILTER_COLOR, Colour(0, 174, 239));
	setColour(SINK_COLOR, Colour(0, 166, 81));
	setColour(SOURCE_COLOR, Colour(241, 90, 41));
	setColour(UTILITY_COLOR, Colour(147, 149, 152));
	setColour(RECORD_COLOR, Colour(255, 0, 0));
	setColour(AUDIO_COLOR, Colour(0,0,0));

	ProcessorListItem* sources = new ProcessorListItem("Sources");
	ProcessorListItem* filters = new ProcessorListItem("Filters");
	ProcessorListItem* sinks = new ProcessorListItem("Sinks");
	ProcessorListItem* utilities = new ProcessorListItem("Utilities");
	ProcessorListItem* record = new ProcessorListItem("Recording");
	//TODO:
	//ProcessorListItem* audio = new ProcessorListItem("Audio");



	baseItem = new ProcessorListItem("Processors");
	baseItem->addSubItem(sources);
	baseItem->addSubItem(filters);
	baseItem->addSubItem(sinks);
	baseItem->addSubItem(utilities);
	baseItem->addSubItem(record);
	//TODO:
	//baseItem->addSubItem(audio);

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

}

void ProcessorList::resized()
{
	setBounds(0,0,195,getTotalHeight());
}



bool ProcessorList::isOpen()
{
	return baseItem->isOpen();
}

void ProcessorList::paint(Graphics& g)
{

	drawItems(g);

	///drawButton(g, true);

}


void ProcessorList::drawItems(Graphics& g)
{
	totalHeight = yBuffer + itemHeight;

	category = baseItem->getName();

	drawItem(g, baseItem);

	if (baseItem->isOpen())
	{
		for (int n = 0; n < baseItem->getNumSubItems(); n++)
		{
			setViewport(g, baseItem->hasSubItems());
			category = baseItem->getSubItem(n)->getName();
			drawItem(g, baseItem->getSubItem(n));

			if (baseItem->getSubItem(n)->isOpen())
			{
				for (int m = 0; m < baseItem->getSubItem(n)->getNumSubItems(); m++)
				{

					setViewport(g, baseItem->
							getSubItem(n)->
							getSubItem(m)->
							hasSubItems());
					drawItem(g, baseItem->getSubItem(n)->getSubItem(m));

				}
			}
		}
	}

	if (isOpen())
		setSize(getWidth(),totalHeight);

	//resized();

}

void ProcessorList::drawItem(Graphics& g, ProcessorListItem* item)
{

	Colour c = findColour(item->colorId);

	g.setColour(c);

	if (item->hasSubItems())
		g.fillRect(1.0, 0.0, getWidth()-2, itemHeight);
	else
		g.fillRect(1.0, 10.0, getWidth()-2, subItemHeight);

	drawItemName(g,item);

	if (item->hasSubItems())
	{
		drawButton(g, item->isOpen());
	}
}

void ProcessorList::drawItemName(Graphics& g, ProcessorListItem* item)
{

	String name;

	g.setColour(Colours::white);
	g.setFont(listFontPlain);

	float offsetX, offsetY;

	if (item->getNumSubItems() == 0)
	{
		if (item->isSelected())
		{
			g.drawText(">", 5, 5, getWidth()-9, itemHeight, Justification::left, false);
			// glRasterPos2f(9.0/getWidth(),0.72);
			// getFont(cpmono_plain)->FaceSize(15);
			// getFont(cpmono_plain)->Render(">");
		}

		name = item->getName();

		offsetX = 20.0f;

		offsetY = 0.72f;
	}
	else
	{
		name = item->getName().toUpperCase();
		offsetX = 5.0f;
		offsetY = 0.75f;
	}

	if (item->getNumSubItems() == 0)
	{
		g.setFont(listFontPlain);
		g.drawText(name, offsetX, 5, getWidth()-offsetX, itemHeight, Justification::left, false);

	}
	else
	{
		g.setFont(listFontLight);
		g.drawText(name, offsetX, 0, getWidth()-offsetX, itemHeight, Justification::left, false);
	}


}

void ProcessorList::drawButton(Graphics& g, bool isOpen)
{



	// glColor4f(1.0f,1.0f,1.0f,1.0f);
	// glLineWidth(1.0f);
	// glBegin(GL_LINE_LOOP);

	// if (isOpen)
	// {
	// 	glVertex2f(0.875,0.35);
	// 	glVertex2f(0.9,0.65);
	// } else {
	// 	glVertex2f(0.925,0.65);
	// 	glVertex2f(0.875,0.5);
	// }
	// glVertex2f(0.925,0.35);
	// glEnd();

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
	int bottom = (yBuffer + itemHeight); // - getScrollAmount();

	//std::cout << "Bottom: " << bottom << std::endl;
	//std::cout << "Y coordinate: " << y << std::endl;

	if (y < bottom)
	{
		return baseItem;

	}
	else
	{

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

void ProcessorList::setViewport(Graphics& g, bool hasSubItems)
{

	int height;

	if (hasSubItems)
	{
		height = itemHeight;
	}
	else
	{
		height = subItemHeight;
	}

	g.setOrigin(0, yBuffer + height); //xBuffer, getHeight()-(totalHeight) - height + getScrollAmount());


	totalHeight += yBuffer + height;

	//std::cout << totalHeight << std::endl;
}

int ProcessorList::getTotalHeight()
{
	return totalHeight;
}

void ProcessorList::toggleState()
{
	ProcessorListItem* fli = getListItemForYPos(0);
	fli->reverseOpenState();
	AccessClass::getUIComponent()->childComponentChanged();
	repaint();
}

void ProcessorList::mouseDown(const MouseEvent& e)
{

	isDragging = false;

	juce::Point<int> pos = e.getPosition();
	int xcoord = pos.getX();
	int ycoord = pos.getY();

	//std::cout << xcoord << " " << ycoord << std::endl;

	ProcessorListItem* listItem = getListItemForYPos(ycoord);

	if (listItem != 0)
	{
		//std::cout << "Selecting: " << fli->getName() << std::endl;
		if (!listItem->hasSubItems())
		{
			clearSelectionState();
			listItem->setSelected(true);
		}

	}
	else
	{
		//std::cout << "No selection." << std::endl;
	}

	if (listItem != 0)
	{
		if (xcoord < getWidth())
		{
			if (e.mods.isRightButtonDown() || e.mods.isCtrlDown())
			{

				if (listItem->getName().equalsIgnoreCase("Sources"))
				{
					currentColor = SOURCE_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Filters"))
				{
					currentColor = FILTER_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Utilities"))
				{
					currentColor = UTILITY_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Sinks"))
				{
					currentColor = SINK_COLOR;
				}
				else if (listItem->getName().equalsIgnoreCase("Record Node"))
				{
					currentColor = RECORD_COLOR;
				}
				else
				{
					return;
				}

				int options=0;
				options += (0 << 0); // showAlpha
				options += (0 << 1); // showColorAtTop
				options += (0 << 2); // showSliders
				options += (1 << 3); // showColourSpace

				ColourSelector colourSelector(options);
				colourSelector.setName("background");
				colourSelector.setCurrentColour(findColour(currentColor));
				colourSelector.addChangeListener(this);
				colourSelector.addChangeListener(AccessClass::getProcessorGraph());
				colourSelector.setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);
				colourSelector.setSize(300, 275);

				juce::Rectangle<int> rect = juce::Rectangle<int>(0,0,10,10);

				CallOutBox callOut(colourSelector, rect, nullptr);
				callOut.setTopLeftPosition(e.getScreenX(), e.getScreenY());
				callOut.setArrowSize(0.0f);

				callOut.runModalLoop();

			}
			else
			{
				listItem->reverseOpenState();
			}
		}

		if (listItem == baseItem)
		{
			if (listItem->isOpen())
			{
				AccessClass::getUIComponent()->childComponentChanged();
			}
			else
			{
				AccessClass::getUIComponent()->childComponentChanged();
				// totalHeight = itemHeight + 2*yBuffer;
			}

		}
	}

	repaint();
}

void ProcessorList::changeListenerCallback(ChangeBroadcaster* source)
{
	ColourSelector* cs = dynamic_cast <ColourSelector*>(source);

	setColour(currentColor, cs->getCurrentColour());

	repaint();

}

void ProcessorList::mouseDrag(const MouseEvent& e)
{

	if (e.getMouseDownX() < getWidth() && !(isDragging))
	{

		ProcessorListItem* listItem = getListItemForYPos(e.getMouseDownY());

		if (listItem != 0)
		{

			if (!listItem->hasSubItems())
			{
				isDragging = true;

				String b = listItem->getName();

				const String dragDescription = b;

				//std::cout << dragDescription << std::endl;

				if (dragDescription.isNotEmpty())
				{
					DragAndDropContainer* const dragContainer
						= DragAndDropContainer::findParentDragContainerFor(this);

					if (dragContainer != 0)
					{
						//pos.setSize (pos.getWidth(), 10);

						Image dragImage(Image::ARGB, 100, 15, true);

						Graphics g(dragImage);
						g.setColour(findColour(listItem->colorId));
						g.fillAll();
						g.setColour(Colours::white);
						g.setFont(14);
						g.drawSingleLineText(listItem->getName(),10,12);//,75,15,Justification::centredRight,true);

						dragImage.multiplyAllAlphas(0.6f);

						juce::Point<int> imageOffset(20,10);

						//See ProcessorGraph::createProcesorFromDescription for description info
						Array<var> dragData;
						dragData.add(true);
						dragData.add(dragDescription);
						dragData.add(listItem->processorType);
						dragData.add(listItem->processorId);
						dragData.add(listItem->getParentName());

						dragContainer->startDragging(dragData, this,
								dragImage, true, &imageOffset);
					}
				}
			}
		}
	}

}

void ProcessorList::saveStateToXml(XmlElement* xml)
{
	XmlElement* processorListState = xml->createNewChildElement("PROCESSORLIST");

	for (int i = 0; i < 7; i++)
	{
		XmlElement* colorState = processorListState->createNewChildElement("COLOR");

		int id;

		switch (i)
		{
			case 0:
				id = PROCESSOR_COLOR;
				break;
			case 1:
				id = SOURCE_COLOR;
				break;
			case 2:
				id = FILTER_COLOR;
				break;
			case 3:
				id = SINK_COLOR;
				break;
			case 4:
				id = UTILITY_COLOR;
				break;
			case 5: 
				id = RECORD_COLOR;
				break;
			case 6:
				id = AUDIO_COLOR;
				break;
			default:
				// do nothing
				;
		}

		Colour c = findColour(id);

		colorState->setAttribute("ID", (int) id);
		colorState->setAttribute("R", (int) c.getRed());
		colorState->setAttribute("G", (int) c.getGreen());
		colorState->setAttribute("B", (int) c.getBlue());

	}
}

void ProcessorList::loadStateFromXml(XmlElement* xml)
{
	forEachXmlChildElement(*xml, xmlNode)
	{
		if (xmlNode->hasTagName("PROCESSORLIST"))
		{
			forEachXmlChildElement(*xmlNode, colorNode)
			{
				setColour(colorNode->getIntAttribute("ID"),
						Colour(
							colorNode->getIntAttribute("R"),
							colorNode->getIntAttribute("G"),
							colorNode->getIntAttribute("B")));
			}
		}
	}

	repaint();

	AccessClass::getProcessorGraph()->refreshColors();
}

Array<Colour> ProcessorList::getColours()
{
	Array<Colour> c;

	c.add(findColour(PROCESSOR_COLOR));
	c.add(findColour(SOURCE_COLOR));
	c.add(findColour(FILTER_COLOR));
	c.add(findColour(SINK_COLOR));
	c.add(findColour(UTILITY_COLOR));
	c.add(findColour(RECORD_COLOR));
	c.add(findColour(AUDIO_COLOR));
	return c;
}

void ProcessorList::setColours(Array<Colour> c)
{
	for (int i = 0; i < c.size(); i++)
	{
		switch (i)
		{
			case 0:
				setColour(PROCESSOR_COLOR, c[i]);
				break;
			case 1:
				setColour(SOURCE_COLOR, c[i]);
				break;
			case 2:
				setColour(FILTER_COLOR, c[i]);
				break;
			case 3:
				setColour(SINK_COLOR, c[i]);
				break;
			case 4:
				setColour(UTILITY_COLOR, c[i]);
				break;
			case 5: 
				setColour(RECORD_COLOR, c[i]);
				break;
			case 6: 
				setColour(AUDIO_COLOR, c[i]);
			default:
				;// do nothing
		}
	}
}

void ProcessorList::fillItemList()
{
	int num;
	baseItem->getSubItem(0)->clearSubItems(); //Sources
	baseItem->getSubItem(1)->clearSubItems(); //Filters
	baseItem->getSubItem(2)->clearSubItems(); //sinks
	baseItem->getSubItem(3)->clearSubItems(); //Utilities
	baseItem->getSubItem(4)->clearSubItems(); //Record
	//baseItem->getSubItem(5)->clearSubItems(); //Audio

	for (int pClass = 0; pClass < 3; pClass++)
	{
		num = ProcessorManager::getNumProcessors((ProcessorClasses)pClass);
		for (int i = 0; i < num; i++)
		{
			String name;
			int type = -1;
			ProcessorManager::getProcessorNameAndType((ProcessorClasses)pClass, i, name, type);
			if (type > -1 && type < 4)
			{
				if (name == "Record Node")
				{
					baseItem->getSubItem(4)->addSubItem(new ProcessorListItem(name, i, pClass));
				}
				else
				{
					baseItem->getSubItem(type)->addSubItem(new ProcessorListItem(name, i, pClass));
				}

			}
		}
	}


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

// ===================================================================

	ProcessorListItem::ProcessorListItem(const String& name_, int pid, int ptype)
		: processorId(pid), processorType(ptype), selected(false), open(true), name(name_)
{
}

ProcessorListItem::~ProcessorListItem()
{ }

bool ProcessorListItem::hasSubItems()
{
	if (subItems.size() > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

int ProcessorListItem::getNumSubItems()
{
	return subItems.size();
}

ProcessorListItem* ProcessorListItem::getSubItem(int index)
{
	return subItems[index];
}

void ProcessorListItem::clearSubItems()
{
	subItems.clear();
}

void ProcessorListItem::addSubItem(ProcessorListItem* newItem)
{
	subItems.add(newItem);
}

void ProcessorListItem::removeSubItem(int index)
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

	if (parentName.equalsIgnoreCase("Processors"))
	{
		colorId = PROCESSOR_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Filters"))
	{
		colorId = FILTER_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Sinks"))
	{
		colorId = SINK_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Sources"))
	{
		colorId = SOURCE_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Recording"))
	{
		colorId = RECORD_COLOR;
	}
	else if (parentName.equalsIgnoreCase("Audio"))
	{
		colorId = AUDIO_COLOR;
	}
	else
	{
		colorId = UTILITY_COLOR;
	}
}
