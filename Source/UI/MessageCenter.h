/*
  ==============================================================================

    MessageCenter.h
    Created: 31 Jul 2011 3:31:01pm
    Author:  jsiegle

  ==============================================================================
*/

#ifndef __MESSAGECENTER_H_2695FC38__
#define __MESSAGECENTER_H_2695FC38__


#include "../../JuceLibraryCode/JuceHeader.h"


class MessageCenter : public Component,
					  public ActionListener

{
public:
    MessageCenter();
    ~MessageCenter();

    void paint (Graphics& g);

private:
   Label* messageDisplayArea;

   void resized();

   void actionListenerCallback(const String& message);

   Colour messageBackground;

};



#endif  // __MESSAGECENTER_H_2695FC38__
