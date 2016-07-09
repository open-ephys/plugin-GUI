#ifndef OE_CYCLOPS_PLUGIN_H
#define OE_CYCLOPS_PLUGIN_H

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "../CyclopsAPI/CyclopsAPI.h"

namespace cyclops{

class CyclopsPluginInfo;

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsSource                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

enum class operationMode
{
    LOOPBACK,
    ONE_SHOT,
    N_SHOT
};

enum class sourceType
{
    STORED,
    GENERATED,
    SQUARE
};

/**
 * @brief      A mirror of the Source Object on the teensy
 */
struct CyclopsSource
{
    operationMode    opMode;
    const int        src_id;
    const sourceType type;
};

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsPlugin                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

class CyclopsPlugin
{
    enum class pluginStatus : int
    {
        CHANNEL_INITIALIZED     = 0,
        SRC_NAMES_INITIALIZED   = 1
    };
private:
    int readiness;
    CyclopsPluginInfo* info;

public:
    /**
     * Suppose you want to control N ( < 4) channels. In the plugin code, you
     * can refer to these channels by integer indices. But, for an experiment,
     * you might want plugin-channel-0 to _physically_ mean ``CH2`` on the
     * Cyclops Board.
     *
     * This array defines a mapping from _plugin-code-channel_ to
     * _physical-cyclops-channel_, and is filled automatically by the GUI
     * according to your inputs on the editor window.
     */
    Array<int> Channels;

    CyclopsPlugin();
    /**
     * @brief      Contructs a Cyclops Plugin.
     * @details    Your code will never call it, don't worry about this
     *             function. This is called by the Cyclops Stimulator node when
     *             you select this plugin from the drop down.
     *
     * @param      info_struct  This holds the channel and source counts, source
     *                          names, etc.
     */

    /**
     * @brief      Sets the channel IDs as per those filled in the editor.
     *
     * @param[in]  channelIDs  Sets CyclopsPlugin::Channels
     */
    void setChannels(const int channelIDs[]);

    bool isReady();
    bool enable();
    bool disable();

    /**
     * @brief      This is where your CL-Plugin must read the event buffer and
     *             process events.
     *
     * @param[in]  eventType       The event type
     * @param      event           The event buffer
     * @param[in]  samplePosition  The sample position
     */
    virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0) = 0;
};

} // NAMESPACE cyclops

#endif