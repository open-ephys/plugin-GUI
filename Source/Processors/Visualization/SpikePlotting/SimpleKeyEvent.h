#ifndef SIMPLE_KEY_EVENT_H
#define SIMPLE_KEY_EVENT_H

/**

  Struct containing keypress information not handled by JUCE.

*/

struct SimpleKeyEvent{
    
    int key;
    bool shift;
    bool ctrl;
    bool alt;

};

#endif