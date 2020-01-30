// LfpDisplayCanvasElements.h

#ifndef LFPDISPLAYCANVASELEMENTS_H

#define LFPDISPLAYCANVASELEMENTS_H

#include "LfpDisplayCanvas.h"

namespace LfpDisplayNodeBeta {
  
class ShowHideOptionsButton : public Button
{
public:
    ShowHideOptionsButton(LfpDisplayOptions*);
    virtual ~ShowHideOptionsButton();
    void paintButton(Graphics& g, bool, bool);
    LfpDisplayOptions* options;
};



class LfpTimescale : public Component
{
public:
    LfpTimescale(LfpDisplayCanvas*);
    ~LfpTimescale();

    void paint(Graphics& g);

    void setTimebase(float t);

private:

    LfpDisplayCanvas* canvas;

    float timebase;

    Font font;

    StringArray labels;

};

class LfpViewport : public Viewport
{
public:
    LfpViewport(LfpDisplayCanvas* canvas);
    void visibleAreaChanged(const Rectangle<int>& newVisibleArea);

private:
    LfpDisplayCanvas* canvas;
};


  
};

#endif
