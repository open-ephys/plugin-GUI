#include <JuceHeader.h>

#include <map>
#include <string>
#include <vector>

#ifndef PROCESSORACTION_H_INCLUDED
#define PROCESSORACTION_H_INCLUDED

/**
  Defines an UndoableAction that is specific to a Processor.

  @see UndoableAction, GenericProcessor
*/
class PLUGIN_API ProcessorAction : public UndoableAction
{
public:
    ProcessorAction (const std::string& name_) : name (name_) {}

    /* Defines the perform action */
    bool perform() override = 0;

    /* Defines the undo action */
    bool undo() override = 0;

    /* Restores the action's owner */
    virtual void restoreOwner (GenericProcessor* p) = 0;

private:
    std::string name;
};

#endif // PROCESSORACTION_H_INCLUDED