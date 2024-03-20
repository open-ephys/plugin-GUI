// Define the actions available for the ChannelMappingNode plugin.

// =====================================================
#ifndef ChannelMappingNodeActions_h
#define ChannelMappingNodeActions_h

#include <ProcessorHeaders.h>

#include "ChannelMappingNode.h"
#include "ChannelMappingEditor.h"

class MapChannelsAction : public OpenEphysAction
{

public:

    /** Constructor*/
    MapChannelsAction(ChannelMappingNode* processor,
                    DataStream* stream,
                    Array<int> nextChannelOrder);

    /** Destructor */
    ~MapChannelsAction();

    void restoreOwner(GenericProcessor* processor) override;

    /** Perform the action*/
    bool perform();

    /** Undo the action*/
    bool undo();

    XmlElement* settings;

private:

    ChannelMappingNode* channelMapper;
    String streamKey;
    Array<int> prevChannelOrder;
    Array<int> nextChannelOrder;

};

#endif /* ChannelMappingNodeActions_h */