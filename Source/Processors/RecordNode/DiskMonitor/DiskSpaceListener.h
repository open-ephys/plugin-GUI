#ifndef DISKSPACELISTENER_H_INCLUDED
#define DISKSPACELISTENER_H_INCLUDED

#include "../../../../JuceLibraryCode/JuceHeader.h"

class DiskSpaceListener
{
public:
    virtual ~DiskSpaceListener() = default;
    virtual void update (float dataRate, int64 bytesFree, float timeLeft) = 0;
    virtual void updateDiskSpace (float percentage) = 0;
    virtual void directoryInvalid() = 0;
    virtual void lowDiskSpace() = 0;
};

#endif // DISKSPACELISTENER_H_INCLUDED