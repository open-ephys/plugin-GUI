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

#include "AudioEditor.h"


MuteButton::MuteButton()
	: DrawableButton (T("Mute button"), DrawableButton::ImageFitted)
{
	DrawablePath normal, over, down;

	    Path p;
        p.addEllipse (0.0,0.0,20.0,20.0);
        normal.setPath (p);
        normal.setFill (Colours::lightgrey);
        normal.setStrokeThickness (0.0f);

        over.setPath (p);
        over.setFill (Colours::black);
        over.setStrokeFill (Colours::black);
        over.setStrokeThickness (5.0f);

        setImages (&normal, &over, &over);
        setBackgroundColours(Colours::darkgrey, Colours::purple);
        setClickingTogglesState (true);
        setTooltip ("Toggle a state.");



}

MuteButton::~MuteButton()
{
}

AudioEditor::AudioEditor (AudioNode* owner) 
	: AudioProcessorEditor (owner), isSelected(false),
	  desiredWidth(150)

{
	name = getAudioProcessor()->getName();

	nodeId = owner->getNodeId();

	backgroundColor = Colours::lightgrey.withAlpha(0.5f);

	muteButton = new MuteButton();
	muteButton->addListener(this);
	muteButton->setBounds(95,5,15,15);
	muteButton->setToggleState(false,false);
	addAndMakeVisible(muteButton);

}

AudioEditor::~AudioEditor()
{
	//std::cout << "  Generic editor for " << getName() << " being deleted with " << getNumChildComponents() << " children. " << std::endl;
	deleteAllChildren();
	//delete titleFont;
}

//void GenericEditor::setTabbedComponent(TabbedComponent* tc) {
	
//	tabComponent = tc;

//}

bool AudioEditor::keyPressed (const KeyPress& key)
{
	//std::cout << name << " received " << key.getKeyCode() << std::endl;
}

void AudioEditor::switchSelectedState() 
{
	//std::cout << "Switching selected state" << std::endl;
	isSelected = !isSelected;
	repaint();
}

void AudioEditor::select()
{
	isSelected = true;
	repaint();
	setWantsKeyboardFocus(true);
	grabKeyboardFocus();
}

bool AudioEditor::getSelectionState() {
	return isSelected;
}

void AudioEditor::deselect()
{
	isSelected = false;
	repaint();
	setWantsKeyboardFocus(false);
}

void AudioEditor::buttonClicked(Button* button)
{
	if (button == muteButton)
	{
		
		if(muteButton->getToggleState()) {
			getAudioProcessor()->setParameter(1,0.0f);
			std::cout << "Mute on." << std::endl;
	    } else {
	    	getAudioProcessor()->setParameter(1,1.0f);
	    	std::cout << "Mute off." << std::endl;
	    }
	}

}

void AudioEditor::paint (Graphics& g)
{

	//g.addTransform(AffineTransform::rotation( double_Pi/20));

	// g.setColour(Colours::black);
	// g.fillRoundedRectangle(0,0,getWidth(),getHeight(),10.0);

	// if (isSelected) {
	g.setColour(backgroundColor);
	// } else {
	// 	g.setColour(Colours::lightgrey);
	// }
	 g.fillRoundedRectangle(1,1,getWidth()-2,getHeight()-2,9.0);

	// g.setColour(Colours::grey);
	// g.fillRoundedRectangle(4,15,getWidth()-8, getHeight()-19,8.0);
	// g.fillRect(4,15,getWidth()-8, 20);

	

	 g.setColour(Colours::black);

	 Font titleFont = Font(14.0, Font::plain);

	// //titleFont.setTypefaceName(T("Miso"));

	 g.setFont(titleFont);
	 g.drawText("Mute ON/OFF", 15, 10, 100, 7, Justification::left, false);

}
