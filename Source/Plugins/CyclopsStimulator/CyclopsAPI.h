#ifndef OE_CYCLOPS_API_H
#define OE_CYCLOPS_API_H

/*
Field       | Bits  | Description
----------- | ----- | -----------
Reserved    | [7]   | Always ``1``.
``channel`` | [6:3] | This is a bitmask and determines if "command" is applied on the Channel ``x``
``command`` | [2:0] | The command field

command[2:0] | Name     | Effect
------------ | -------  | -------------
``000``      | start    | Launch Waveform generation.
``001``      | stop     | Pause Waveform generation.
``010``      | reset    | Reset selected sources. @attention The system is *not* reset to _initial configuration_!
``011``      | swap     | Swap the Cyclops instances of the 2 high ``channel`` bits.
``111``      | identity | Send device description.

Field       | Bits  | Description
----------- | ----- | -----------
Reserved    | [7]   | Always ``0``.
``channel`` | [6:5] | Command is appplied on Channel ``channel[1:0]``
``command`` | [4:0] | The command field.

command[4:0]  | Name               | Size(Bytes) | Effect
------------- | ------------------ | ----------- | --------
``00000``     | change_source_l    | 2           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``LOOPBACK``.
``00001``     | change_source_o    | 2           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``ONE_SHOT``.
``00010``     | change_source_n    | 3           | Changes Source instance to the one reffered by @ref src-id-sec. Mode is set to ``N_SHOT``. \f$N\f$ is set to @ref shot_cycle "shot_cycle".
``00011``     | change_time_period | 5           | Set time period of updates @attention Works only if Source::holdTime is a constant!
``00100``     | time_factor        | 5           | Scale Source::holdTime values by this factor. \f$\in [0, \infty)\f$.
``00101``     | voltage_factor     | 5           | Scale Source::getVoltage values by this factor. \f$\in [0, \infty)\f$.
``00110``     | voltage_offset     | 3           | Add this DC offset level to Source::getVoltage values. \f$\in [0, \infty)\f$.
``00111``     | square_on_time     | 5           | Set squareSource pulse "ON" time.
``01000``     | square_off_time    | 5           | Set squareSource pulse "OFF" time.
``01001``     | square_on_level    | 3           | Set squareSource pulse "ON" voltage.
``01010``     | square_off_level   | 3           | Set squareSource pulse "OFF" voltage.
*/

/**
 * @defgroup ns-cyclops cyclops
 * 
 * @warning
 * Most functions in this namespace assume that the various arguments are going to be valid, ie
 * there is absolutely no error checking. In the worst case, malformed packets might invoke
 * undesired action on the Teensy!
 */
namespace cyclops
{

static const int RPC_HEADER_SZ = 1;
static const int RPC_MAX_ARGS  = 4;

struct CyclopsRPC
{
    uint8_t message[RPC_HEADER_SZ + RPC_MAX_ARGS];
    int length;
};

enum singleByteCommands
{
    START    = 0,
    STOP     = 1,
    RESET    = 2,
    SWAP     = 3,
    IDENTITY = 7
};

enum multiByteCommands
{
    CHANGE_SOURCE_LOOP = 0,
    CHANGE_SOURCE_ONE  = 1,
    CHANGE_SOURCE_N    = 2,
    CHANGE_TIME_PERIOD = 3,
    TIME_FACTOR        = 4,
    VOLTAGE_FACTOR     = 5,
    VOLTAGE_OFFSET     = 6,
    SQUARE_ON_TIME     = 7,
    SQUARE_OFF_TIME    = 8,
    SQUARE_ON_LEVEL    = 9,
    SQUARE_OFF_LEVEL   = 10,
};

enum multiByteLength
{
    CHANGE_SOURCE_LOOP = 2,
    CHANGE_SOURCE_ONE  = 2,
    CHANGE_SOURCE_N    = 3,
    CHANGE_TIME_PERIOD = 5,
    TIME_FACTOR        = 5,
    VOLTAGE_FACTOR     = 5,
    VOLTAGE_OFFSET     = 3,
    SQUARE_ON_TIME     = 5,
    SQUARE_OFF_TIME    = 5,
    SQUARE_ON_LEVEL    = 3,
    SQUARE_OFF_LEVEL   = 3
};

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 SINGLE BYTE COMMANDS                          |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

/**
 * @brief      Start (aka reset) the selected Channel Signals
 * @ingroup    ns-cyclops
 */
bool start (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Pause (aka FREEZE) the selected Channel Signals
 * @ingroup    ns-cyclops
 */
bool stop (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Reset (aka start) the selected Channel Signals
 * @ingroup    ns-cyclops
 */
bool reset (CyclopsRPC *rpc, const int *channels, int channelCount);

/**
 * @brief      Swap the physical cyclops channel of the two selected Channel Signals
 * @ingroup    ns-cyclops
 */
bool swap (CyclopsRPC *rpc, int c1, int c2);

/**
 * @brief      Query the Cyclops Board for connected channels, firmware version etc.
 * @ingroup    ns-cyclops
 */
bool identify (CyclopsRPC *rpc);

/*
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 MULTI BYTE COMMANDS                           |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/


/**
 * @brief      Change the ``source`` object for the selected channel, in LOOPBACK mode.
 * @ingroup    ns-cyclops
 * @param[in]  srcID    The source identifier on the teensy.
 */
bool change_source_loop (CyclopsRPC *rpc, int channel, int srcID);

/**
 * @brief      Change the ``source`` object for the selected channel, in ONE_SHOT mode.
 * @ingroup    ns-cyclops
 * @param[in]  srcID    The source identifier on the teensy.
 */
bool change_source_one_shot (CyclopsRPC *rpc, int channel, int srcID);

/**
 * @brief      Change the ``source`` object for the selected channel, in N_SHOT
 *             mode.
 * @ingroup    ns-cyclops
 * @param[in]  srcID       The source identifier on the teensy.
 * @param[in]  shot_cycle  The shot cycle (1 by default)
 */
bool change_source_n_shot (CyclopsRPC *rpc, int channel, int srcID, int shot_cycle = 1);

/**
 * @brief      Set a new period of update for ``generatedSource``.
 * @note       Works only if the active source on the selected channel is of type ``generatedSource``
 *             AND was been configured with a ``const`` holdTime.
 * @ingroup    ns-cyclops
 */
bool change_time_period (CyclopsRPC *rpc, int channel, uint32_t new_period);

/**
 * @brief      Set the time-scale factor for the source on the selected channel.
 * @ingroup    ns-cyclops
 */
bool time_factor (CyclopsRPC *rpc, int channel, float tFactor);

/**
 * @brief      Set the voltage-scale factor for the source on the selected channel.
 * @ingroup    ns-cyclops
 */
bool voltage_factor (CyclopsRPC *rpc, int channel, float vFactor);

/**
 * @brief      Set the DC voltage offset for the source on the selected channel.
 * @note       This number is a signed integer!
 * @ingroup    ns-cyclops
 */
bool voltage_offset (CyclopsRPC *rpc, int channel, int16_t vOffset);

/**
 * @brief      Set the "ON" time for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    ns-cyclops
 */
bool square_on_time (CyclopsRPC *rpc, int channel, uint32_t onTime);

/**
 * @brief      Set the "OFF" time for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    ns-cyclops
 */
bool square_off_time (CyclopsRPC *rpc, int channel, uint32_t offTime);

/**
 * @brief      Set the "ON" voltage level for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    ns-cyclops
 */
bool square_on_level (CyclopsRPC *rpc, int channel, uint16_t onLevel);

/**
 * @brief      Set the "OFF" voltage level for the square wave.
 * @note       Works only if the active source on the selected channel is of type ``squareSource``!
 * @ingroup    ns-cyclops
 */
bool square_off_level (CyclopsRPC *rpc, int channel, uint16_t offLevel);

} // NAMESPACE cyclops

#endif