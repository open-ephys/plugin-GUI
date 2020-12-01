#include "SyncChannelSelector.h"
#include <string>
#include <vector>

SyncChannelButton::SyncChannelButton(int _id, SyncChannelSelector* _parent) : Button(String(_id)), id(_id), parent(_parent) {
    //setClickingTogglesState(true);
}


SyncChannelButton::~SyncChannelButton() {}

/*
void SyncChannelButton::mouseDown(const MouseEvent &event)
{
    this->mouseDown(event);
    parent->mouseDown(event);
}

void SyncChannelButton::mouseUp(const MouseEvent &event)
{
    parent->mouseUp(event);
}
*/

void SyncChannelButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{

	g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
	{
		if (getToggleState())
			g.setColour(Colour(255, 200, 0));
		else
			g.setColour(Colour(210, 210, 210));
	}
	else 
	{
		if (getToggleState())
			g.setColour(Colour(255, 128, 0));
		else
			g.setColour(Colour(110, 110, 110));
	}
	g.fillRoundedRectangle(1,1,getWidth()-2,getHeight()-2,0.001*getWidth());

    //Draw text string in middle of button
	g.setColour(Colour(255,255,255));
	g.setFont(10);
	g.drawText (String(id), 0,0, getWidth(), getHeight(), Justification::centred); 

}

SetButton::SetButton(const String& name) : Button(name) {
	//setClickingTogglesState(true);
}

SetButton::~SetButton() {}

void SetButton::paintButton(Graphics &g, bool isMouseOver, bool isButtonDown)
{
    g.setColour(Colour(0,0,0));
    g.fillRoundedRectangle (0.0f, 0.0f, getWidth(), getHeight(), 0.001*getWidth());

    if (isMouseOver)
    {
        g.setColour(Colour(220, 220, 220));
    }
    else
    {
        if (getToggleState())
            g.setColour(Colour(200, 200, 200));
        else
            g.setColour(Colour(110, 110, 110));
    }
    g.fillRoundedRectangle(0.0f, 0.0f, getWidth(), getHeight(), 0.01*getWidth());
    
	g.setColour(Colour(255,255,255));
	g.setFont(12);
	g.drawText (String(getName()), 0, 0, getWidth(), getHeight(), Justification::centred);
}

SyncChannelSelector::SyncChannelSelector(int nChans, int selectedIdx, bool isMaster_) 
    : Component(), 
    nChannels(nChans),
    selectedId(selectedIdx),
    isMaster(isMaster_)
{

    width = 368; //can use any multiples of 16 here for dynamic resizing

    int nColumns = 16;
    nRows = nChannels / nColumns + (int)(!(nChannels % nColumns == 0));
    buttonSize = width / 16;
    height = buttonSize * nRows;

	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nColumns; j++)
		{
            if (nColumns*i+j < nChannels)
            {
                buttons.add(new SyncChannelButton(nColumns*i+j+1, this));
                buttons.getLast()->setBounds(width/nColumns*j, height/nRows*i, buttonSize, buttonSize);
                buttons.getLast()->setToggleState(selectedIdx == nColumns*i+j, NotificationType::dontSendNotification);
                buttons.getLast()->addListener(this);
                addChildAndSetID(buttons.getLast(), String(nColumns*i+1));
            }
			
		}
	}
    
    if (!isMaster)
    {
        setMasterSubprocessorButton = new SetButton("Set as Master Subprocessor");
        setMasterSubprocessorButton->setBounds(0, height, 0.5*width, width / nColumns);
        setMasterSubprocessorButton->addListener(this);
        addChildAndSetID(setMasterSubprocessorButton,"SETMASTER");
    }
    else
    {
        height = buttonSize * (nRows - 1);
    }
    
    
    if (nChannels <= 8)
        width /= 2;

	setSize (width, height + buttonSize);
	setColour(ColourSelector::backgroundColourId, Colours::transparentBlack);

}

SyncChannelSelector::~SyncChannelSelector() {}

void SyncChannelSelector::mouseMove(const MouseEvent &event)
{

};

void SyncChannelSelector::mouseDown(const MouseEvent &event)
{

    LOGD("Got mouse down event in parent");

};


void SyncChannelSelector::mouseUp(const MouseEvent &event)
{
    //TODO
}

void SyncChannelSelector::buttonClicked(Button* button)
{
    //TODO
    if (button->getComponentID() == "SETMASTER")
    {
        setSize (width, buttonSize * nRows);
        height = buttonSize * (nRows);
        isMaster = true;
        findParentComponentOfClass<CallOutBox>()->exitModalState(0);
    }
    else
    {
        for (int i = 0; i < buttons.size(); i++)
        {
            buttons[i]->setToggleState(false, false);
            repaint();
        }
        button->setToggleState(true, false);

    }
    
}