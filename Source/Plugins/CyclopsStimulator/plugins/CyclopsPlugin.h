#ifndef OE_CYCLOPS_PLUGIN_H
#define OE_CYCLOPS_PLUGIN_H

#include "CyclopsAPI.h"
#include "CyclopsPluginInfo.h"

namespace cyclops{

/**
 * @brief      Forward declaration for the enum used by the user's plugin.
 * @details    Allows the user to refer to the sources for this plugin with
 *             meaningful names, rather than plain integer indices.
 *
 *             It's perfectly fine to not define it or use a different enum or
 *             not use any enum at all.
 */
extern enum sourceAlias : int;

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsSource                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

enum operationMode
{
    LOOPBACK,
    ONE_SHOT,
    N_SHOT
};

enum sourceType
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
    enum pluginStatus : int
    {
        CHANNEL_INITIALIZED     = 0,
        SRC_NAMES_INITIALIZED   = 1,
    }
private:
    pluginStatus readiness;
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

    /**
     * @brief      Contructs a Cyclops Plugin.
     * @details    Your code will never call it, don't worry about this
     *             function. This is called by the Cyclops Stimulator node when
     *             you select this plugin from the drop down.
     *
     * @param      info_struct  This holds the channel and source counts, source
     *                          names, etc.
     */
    CyclopsPlugin(CyclopsPluginInfo* info_struct);

    /**
     * @brief      Sets the channel IDs as per those filled in the editor.
     *
     * @param[in]  channelIDs  Sets CyclopsPlugin::Channels
     */
    void setChannels(const int channelIDs[]);

    virtual bool isReady();
    virtual bool enable();
    virtual bool disable();

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