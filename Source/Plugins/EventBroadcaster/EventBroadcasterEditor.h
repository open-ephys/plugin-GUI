/*
  ==============================================================================

    EventBroadcasterEditor.h
    Created: 22 May 2015 3:34:30pm
    Author:  Christopher Stawarz

  ==============================================================================
*/

#ifndef EVENTBROADCASTEREDITOR_H_INCLUDED
#define EVENTBROADCASTEREDITOR_H_INCLUDED

#include <EditorHeaders.h>


/**

 User interface for the "EventBroadcaster" source node.

 @see EventBroadcaster

 */

class EventBroadcasterEditor : public GenericEditor, public Label::Listener
{
public:
    EventBroadcasterEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);

    void buttonEvent(Button* button) override;
    void labelTextChanged(juce::Label* label) override;

    void setDisplayedPort(int port);

private:
    ScopedPointer<UtilityButton> restartConnection;
    ScopedPointer<Label> urlLabel;
    ScopedPointer<Label> portLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EventBroadcasterEditor);

};


#endif  // EVENTBROADCASTEREDITOR_H_INCLUDED
