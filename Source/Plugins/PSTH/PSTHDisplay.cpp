#include "PSTHDisplay.h"

PSTHDisplay::PSTHDisplay( Viewport *p, PSTHCanvas *c) : viewport(p), canvas(c) // PSTHEditor* n, : psthEditor(n),
{

	font = Font("Default", 15, Font::plain);
}

PSTHDisplay::~PSTHDisplay()
{
	for (int k = 0; k < psthPlots.size(); k++)
	{
		delete(psthPlots[k]);
	}
	psthPlots.clear();
}

void PSTHDisplay::refresh()
{
	for (int k = 0; k < psthPlots.size(); k++)
	{
		psthPlots[k]->repaint();
	}

}


void PSTHDisplay::paint(Graphics &g)
{
	g.setColour(Colours::white);
	g.drawRect(0, 0, getWidth(), getHeight());
	/*
	font = Font("Default", 15, Font::plain);

	g.setFont(font);

	g.drawText("Test",10,0,200,20,Justification::left,false);
	*/
}


void PSTHDisplay::setAutoRescale(bool state)
{
	// draw n by m grid
	
	//psthEditor->showAutoRescale = state;

	for (int k = 0; k<psthPlots.size(); k++)
	{
		psthPlots[k]->setAutoRescale(state);
	}

}

void PSTHDisplay::resized()
{
	// draw n by m grid
	for (int k = 0; k<psthPlots.size(); k++)
	{
		if (psthPlots[k]->isFullScreen())
		{
			int newSize = MIN(canvas->screenWidth, canvas->screenHeight);
			setBounds(0, 0, newSize, newSize);
			psthPlots[k]->setBounds(0, 0, newSize - 30, newSize - 30);

		}
		else
		{
			psthPlots[k]->setBounds(psthPlots[k]->getRow() * canvas->widthPerUnit,
				psthPlots[k]->getCol() * canvas->heightPerElectrodePix,
				canvas->widthPerUnit,
				canvas->heightPerElectrodePix);
		}

	}
}


void PSTHDisplay::focusOnPlot(int plotID)
{
	int plotIndex = -1;
	for (int i = 0; i<psthPlots.size(); i++)
	{
		if (psthPlots[i]->getPlotID() == plotID)
		{
			plotIndex = i;
			break;
		}

	}
	if (plotIndex == -1)
		return;
	if (psthPlots[plotIndex]->isFullScreen())
	{

		psthPlots[plotIndex]->toggleFullScreen(false);
		psthPlots[plotIndex]->setBounds(psthPlots[plotIndex]->getRow() * canvas->widthPerUnit,
			psthPlots[plotIndex]->getCol() * canvas->heightPerElectrodePix,
			canvas->widthPerUnit,
			canvas->heightPerElectrodePix);
		// hide all other plots.
		for (int k = 0; k<psthPlots.size(); k++)
		{
			psthPlots[k]->setVisible(true);
			psthPlots[k]->repaint();
		}

	}
	else
	{
		// hide all other plots.
		for (int k = 0; k<psthPlots.size(); k++)
		{
			if (psthPlots[k]->getPlotID() != plotID)
				psthPlots[k]->setVisible(false);
		}
		psthPlots[plotIndex]->toggleFullScreen(true);
		// make sure its rectangular...?
		int newSize = MIN(canvas->screenWidth, canvas->screenHeight);
		setBounds(0, 0, newSize, newSize);
		psthPlots[plotIndex]->setBounds(0, 0, newSize - 30, newSize - 30);
		psthPlots[plotIndex]->repaint();
	}

}