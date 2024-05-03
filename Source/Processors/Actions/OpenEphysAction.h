#include <JuceHeader.h>

#include <map>
#include <vector>
#include <string>

#ifndef OPENEPHYSACTION_H_INCLUDED
#define OPENEPHYSACTION_H_INCLUDED

class PLUGIN_API OpenEphysAction : public UndoableAction {
public:
    //OpenEphysAction(GenericProcessor* p) {};
    OpenEphysAction(const std::string& actionKey) : key(actionKey) {
        // Add this action to the static map
        //actionsByKey[key].push_back(this);
    }

    // Implement the UndoableAction methods
    bool perform() override = 0;

    // Implement the UndoableAction methods
    bool undo() override = 0;

    // Example static method to access actions by key
    /*
    static std::vector<OpenEphysAction*>& getActionsByKey(const std::string& actionKey) {
        return actionsByKey[actionKey];
    }
    */
    virtual void restoreOwner(GenericProcessor* p) = 0;

    String getIdentifier() {
        return key;
    }

private:

    GenericProcessor* processor;
    //std::string key;  // Private member to store the action key

    // Static map to store actions by keys
    //static std::map<std::string, std::vector<OpenEphysAction*>> actionsByKey;
    std::string key;
};

#endif  // OPENEPHYSACTION_H_INCLUDED