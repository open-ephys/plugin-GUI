// LfpDisplayCanvasElements.cpp

#include "LfpDisplayCanvasElements.h"
#include "LfpDisplay.h"

using namespace LfpDisplayNodeBeta;
// =============================================================


ShowHideOptionsButton::ShowHideOptionsButton(LfpDisplayOptions* options) : Button("Button")
{
    setClickingTogglesState(true);
}
ShowHideOptionsButton::~ShowHideOptionsButton()
{

}

void ShowHideOptionsButton::paintButton(Graphics& g, bool, bool) 
{   
    g.setColour(Colours::white);

    Path p;

    float h = getHeight();
    float w = getWidth();

    if (getToggleState())
    {
        p.addTriangle(0.5f*w, 0.2f*h,
                      0.2f*w, 0.8f*h,
                      0.8f*w, 0.8f*h);
    }
    else
    {
        p.addTriangle(0.8f*w, 0.8f*h,
                      0.2f*w, 0.5f*h,
                      0.8f*w, 0.2f*h);
    }

    PathStrokeType pst = PathStrokeType(1.0f, PathStrokeType::curved, PathStrokeType::rounded);

    g.strokePath(p, pst);
}



// -------------------------------------------------------------

LfpTimescale::LfpTimescale(LfpDisplayCanvas* c) : canvas(c)
{

    font = Font("Default", 16, Font::plain);
}

LfpTimescale::~LfpTimescale()
{

}

void LfpTimescale::paint(Graphics& g)
{

    g.setFont(font);

    g.setColour(Colour(100,100,100));

    g.drawText("ms:",5,0,100,getHeight(),Justification::left, false);

    for (int i = 1; i < 10; i++)
    {
        if (i == 5)
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),3.0f);
        else
            g.drawLine(getWidth()/10*i,0,getWidth()/10*i,getHeight(),1.0f);

        g.drawText(labels[i-1],getWidth()/10*i+3,0,100,getHeight(),Justification::left, false);
    }

}

void LfpTimescale::setTimebase(float t)
{
    timebase = t;

    labels.clear();

    for (float i = 1.0f; i < 10.0; i++)
    {
        String labelString = String(timebase/10.0f*1000.0f*i);

        labels.add(labelString.substring(0,6));
    }

    repaint();

}





// Lfp Viewport -------------------------------------------

LfpViewport::LfpViewport(LfpDisplayCanvas *canvas)
    : Viewport()
{
    this->canvas = canvas;
}

void LfpViewport::visibleAreaChanged(const Rectangle<int>& newVisibleArea)
{
    canvas->fullredraw = true;
    canvas->refresh();
}
