/*!
\copyright  Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Private (internal) kymera header file.

*/

#ifndef AV_HEADSET_KYMERA_PRIVATE_H
#define AV_HEADSET_KYMERA_PRIVATE_H

#include <a2dp.h>
#include <hfp.h>
#include <panic.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <chain.h>

#include "av_headset.h"
#include "av_headset_kymera.h"
#include "av_headset_log.h"
#include "av_headset_chain_roles.h"
#include "av_headset_latency.h"
#include <opmsg_prim.h>

/*! \brief Macro to help getting an operator from chain.
    \param op The returned operator, or INVALID_OPERATOR if the operator was not
           found in the chain.
    \param chain_handle The chain handle.
    \param role The operator role to get from the chain.
    \return TRUE if the operator was found, else FALSE
 */
#define GET_OP_FROM_CHAIN(op, chain_handle, role) \
    (INVALID_OPERATOR != ((op) = ChainGetOperatorByRole((chain_handle), (role))))

/*@{ \name System kick periods, in microseconds */
#define KICK_PERIOD_FAST (2000)
#define KICK_PERIOD_SLOW (7500)

#define KICK_PERIOD_SLAVE_SBC (KICK_PERIOD_SLOW)
#define KICK_PERIOD_SLAVE_APTX (KICK_PERIOD_SLOW)
#define KICK_PERIOD_SLAVE_AAC (KICK_PERIOD_SLOW)
#define KICK_PERIOD_MASTER_SBC (KICK_PERIOD_SLOW)
#define KICK_PERIOD_MASTER_AAC (KICK_PERIOD_SLOW)
#define KICK_PERIOD_MASTER_APTX (KICK_PERIOD_SLOW)
#define KICK_PERIOD_TONES (KICK_PERIOD_SLOW)
#define KICK_PERIOD_VOICE (KICK_PERIOD_FAST) /*!< Use low latency for voice */
/*@} */

/*!@{ \name Defines used in setting up kymera messages
    Kymera operator messages are 3 words long, with the ID in the 2nd word */
#define KYMERA_OP_MSG_LEN (3)
#define KYMERA_OP_MSG_WORD_MSG_ID (1)
#define KYMERA_OP_MSG_ID_TONE_END (0x0001) /*!< Kymera ringtone generator TONE_END message */
/*!@}*/

/*! Kymera requires gain specified in unit of 1/60th db */
#define KYMERA_DB_SCALE (60)

/*! Helper macro to get size of fixed arrays to populate structures */
#define DIMENSION_AND_ADDR_OF(ARRAY) ARRAY_DIM((ARRAY)), (ARRAY)

/*! Maximum sample rate supported by this application */
#define MAX_SAMPLE_RATE (48000)

/*! Maximum codec rate expected by this application */
#define MAX_CODEC_RATE_KBPS (384) /* Stereo aptX */
#define APTX_MONO_CODEC_RATE_KBPS (192) /* Stereo aptX */

/*!:{ \name Macros to calculate buffer sizes required to hold a specific (timed) amount of audio */
#define CODEC_BITS_PER_MEMORY_WORD (16)
#define MS_TO_BUFFER_SIZE_MONO_PCM(time_ms, sample_rate) (((time_ms) * (sample_rate)) / MS_PER_SEC)
#define US_TO_BUFFER_SIZE_MONO_PCM(time_us, sample_rate) (((time_us) * (sample_rate)) / US_PER_SEC)
#define MS_TO_BUFFER_SIZE_CODEC(time_ms, codec_rate_kbps) (((time_ms) * (codec_rate_kbps)) / CODEC_BITS_PER_MEMORY_WORD)
/*!@}*/

/*!@{ \name Buffer sizes required to hold enough audio to achieve the TTP latency */
#define PRE_DECODER_BUFFER_SIZE     (MS_TO_BUFFER_SIZE_CODEC(PRE_DECODER_BUFFER_MS, MAX_CODEC_RATE_KBPS))
#define PCM_LATENCY_BUFFER_SIZE     (MS_TO_BUFFER_SIZE_MONO_PCM(PCM_LATENCY_BUFFER_MS, MAX_SAMPLE_RATE))
#define APTX_LATENCY_BUFFER_SIZE    (MS_TO_BUFFER_SIZE_CODEC(PCM_LATENCY_BUFFER_MS - (appConfigTwsDeadline() / 1000), APTX_MONO_CODEC_RATE_KBPS))
/*!@}*/

/*! Convert x into 1.31 format */
#define FRACTIONAL(x) ( (int)( (x) * ((1l<<31) - 1) ))

/*!@{ \name Macros to set and clear bits in the lock. */
#define appKymeraSetToneLock(theKymera) (theKymera)->lock |= 1U
#define appKymeraClearToneLock(theKymera) (theKymera)->lock &= ~1U
#define appKymeraSetStartingLock(theKymera) (theKymera)->lock |= 2U
#define appKymeraClearStartingLock(theKymera) (theKymera)->lock &= ~2U
/*!@}*/

/*! Get current state */
#define appKymeraGetState() (appGetKymera()->state)

/*! Kymera ringtone generator has a fixed sample rate of 8 kHz */
#define KYMERA_TONE_GEN_RATE (8000)

/*! \brief The audio data format */
typedef enum
{
    ADF_GENERIC_ENCODED = 0,
    ADF_PCM = 1,
    ADF_16_BIT_WITH_METADATA = 2,
    ADF_GENERIC_ENCODED_32BIT = 13
} audio_data_format;

/*! \brief Switch passthrough consumer states */
typedef enum
{
    PASSTHROUGH_MODE,
    CONSUMER_MODE
} switched_passthrough_states;

/*! \brief Kymera operator UCIDs */
typedef enum kymera_operator_ucids
{
    UCID_AEC_WB = 0,
    UCID_AEC_NB = 1,
    UCID_AEC_SWB = 2,
    UCID_AEC_UWB = 3,
    UCID_CVC_SEND = 0,
    UCID_CVC_RECEIVE = 0,
    UCID_VOLUME_CONTROL = 0,
    UCID_SOURCE_SYNC = 0,
} kymera_operator_ucid_t;


/*! \brief Internal message IDs */
enum app_kymera_internal_message_ids
{
    /*! Internal A2DP start message. */
    KYMERA_INTERNAL_A2DP_START,
    /*! Internal A2DP starting message. */
    KYMERA_INTERNAL_A2DP_STARTING,
    /*! Internal A2DP stop message. */
    KYMERA_INTERNAL_A2DP_STOP,
    /*! Internal A2DP stop forwarding message. */
    KYMERA_INTERNAL_A2DP_STOP_FORWARDING,
    /*! Internal A2DP set volume message. */
    KYMERA_INTERNAL_A2DP_SET_VOL,
    /*! Internal SCO start message, including start of SCO forwarding (if supported). */
    KYMERA_INTERNAL_SCO_START,
    /*! Internal message to start SCO forwarding */
    KYMERA_INTERNAL_SCO_START_FORWARDING_TX,
    /*! Internal message to stop forwarding SCO */
    KYMERA_INTERNAL_SCO_STOP_FORWARDING_TX,
    /*! Internal message to set SCO volume */
    KYMERA_INTERNAL_SCO_SET_VOL,
    /*! Internal SCO stop message. */
    KYMERA_INTERNAL_SCO_STOP,
    /*! Internal SCO microphone mute message. */
    KYMERA_INTERNAL_SCO_MIC_MUTE,
    /*! Internal message indicating that forwarded SCO (incoming) is/will be active */
    KYMERA_INTERNAL_SCO_SLAVE_START,
    /*! Internal message to stop playing forwarded SCO */
    KYMERA_INTERNAL_SCOFWD_RX_STOP,
    /*! Internal tone play message. */
    KYMERA_INTERNAL_TONE_PROMPT_PLAY,
    /*! Select the local microphone(s) for SCO */
    KYMERA_INTERNAL_MICFWD_LOCAL_MIC,
    /*! Select a forwarded microphone for SCO */
    KYMERA_INTERNAL_MICFWD_REMOTE_MIC,
    /*! Internal ANC tuning start message */
    KYMERA_INTERNAL_ANC_TUNING_START,
    /*! Internal ANC tuning stop message */
    KYMERA_INTERNAL_ANC_TUNING_STOP,
    /*! Disable the audio SS (used for the DAC disable) */
    KYMERA_INTERNAL_AUDIO_SS_DISABLE,
    /*! Internal ANC turn on indication use in peer synchronization */
    KYMERA_INTERNAL_ANC_ON,
    /*! Internal ANC turn indication use in peer synchronization */
    KYMERA_INTERNAL_ANC_OFF,
};

/*! \brief The KYMERA_INTERNAL_A2DP_START and KYMERA_INTERNAL_A2DP_STARTING message content. */
typedef struct
{
    /*! The client's lock. Bits set in lock_mask will be cleared when A2DP is started. */
    uint16 *lock;
    /*! The bits to clear in the client lock. */
    uint16 lock_mask;
    /*! The A2DP codec settings */
    a2dp_codec_settings codec_settings;
    /*! The starting volume */
    uint8 volume;
    /*! The number of times remaining the kymera module will resend this message to
        itself (having entered the locked KYMERA_STATE_A2DP_STARTING) state before
        proceeding to commence starting kymera. Starting will commence when received
        with value 0. Only applies to starting the master. */
    uint8 master_pre_start_delay;
} KYMERA_INTERNAL_A2DP_START_T;


/*! \brief The KYMERA_INTERNAL_A2DP_SET_VOL message content. */
typedef struct
{
    /*! The volume to set. */
    uint16 volume;
} KYMERA_INTERNAL_A2DP_SET_VOL_T;

/*! \brief The KYMERA_INTERNAL_A2DP_STOP and KYMERA_INTERNAL_A2DP_STOP_FORWARDING message content. */
typedef struct
{
    /*! The A2DP seid */
    uint8 seid;
    /*! The media sink */
    Source source;
} KYMERA_INTERNAL_A2DP_STOP_T;

/*! \brief The KYMERA_INTERNAL_SCO_START message content. */
typedef struct
{
    /*! The SCO audio sink. */
    Sink audio_sink;    
    /*! Pointer to SCO chain information. */
    const appKymeraScoChainInfo *sco_info;
    /*! The link Wesco. */
    uint8 wesco;
    /*! The starting volume. */
    uint8 volume;
    /*! The number of times remaining the kymera module will resend this message to
        itself before starting kymera SCO. */
    uint8 pre_start_delay;
} KYMERA_INTERNAL_SCO_START_T;

/*! \brief The KYMERA_INTERNAL_SCOFWD_RX_START message content. */
typedef struct
{
    /*! The audio source from the air */
    Source link_source;
    /*! Pointer to SCO chain information. */
    const appKymeraScoChainInfo *sco_info;
    /*! The starting volume */
    uint8 volume;
    /*! The number of times remaining the kymera module will resend this message to
        itself before starting kymera SCO. */
    uint8 pre_start_delay;
} KYMERA_INTERNAL_SCO_SLAVE_START_T;

/*! \brief The KYMERA_INTERNAL_SCO_START_FORWARDING_TX message content. */
typedef struct
{
    /*! The BT sink to the air */
    Sink forwarding_sink;
    /*! TRUE if microphone forwarding is enabled */
    bool enable_mic_fwd;
} KYMERA_INTERNAL_SCO_START_FORWARDING_TX_T;


/*! \brief The KYMERA_INTERNAL_SCO_SET_VOL message content. */
typedef struct
{
    /*! The volume to set. */
    uint8 volume;
} KYMERA_INTERNAL_SCO_SET_VOL_T;


/*! \brief The KYMERA_INTERNAL_SCO_MIC_MUTE message content. */
typedef struct
{
    /*! TRUE to enable mute, FALSE to disable mute. */
    bool mute;
} KYMERA_INTERNAL_SCO_MIC_MUTE_T;

/*! \brief KYMERA_INTERNAL_TONE_PLAY message content */
typedef struct
{
    /*! Pointer to the ringtone structure to play, NULL for prompt. */
    const ringtone_note *tone;
    /*! The prompt file index to play. FILE_NONE for tone. */
    FILE_INDEX prompt;
    /*! The prompt file format */
    promptFormat prompt_format;
    /*! The tone/prompt sample rate */
    uint32 rate;
    /*! If TRUE, the tone may be interrupted by another event before it is
        completed. If FALSE, the tone may not be interrupted by another event
        and will play to completion. */
    bool interruptible;
    /*! If not NULL, set bits in client_lock_mask will be cleared in client_lock
    when the tone is stopped. */
    uint16 *client_lock;
    /*! The mask of bits to clear in client_lock. */
    uint16 client_lock_mask;
} KYMERA_INTERNAL_TONE_PROMPT_PLAY_T;


typedef struct
{
    uint32 usb_rate;
} KYMERA_INTERNAL_ANC_TUNING_START_T;

void appKymeraSetState(appKymeraState state);

/*! \brief Set an operator's terminal buffer sizes.
    \param op The operator.
    \param rate The sample rate is Hz.
    \param buffer_size_ms The amount of audio the buffer should contain in ms.
    \param input_terminals The input terminals for which the buffer size should be set (bitmask).
    \param output_terminals The output terminals for which the buffer size should be set (bitmask).
*/
void appKymeraSetTerminalBufferSize(Operator op, uint32 rate, uint32 buffer_size_ms,
                                           uint16 input_terminals, uint16 output_terminals);


const appKymeraScoChainInfo *appKymeraScoFindChain(const appKymeraScoChainInfo *info, appKymeraScoMode mode, bool sco_fwd, bool mic_fwd, bool cvc_2_mic);

/*! \brief Immediately stop playing the tone or prompt */
void appKymeraTonePromptStop(void);

/*! \brief Create and configure the audio output chain.
    \param rate The sample rate in Hz.
    \param kick_period The kymera kick period.
    \param buffer_size The PCM buffer size.
    \param volume The start volume.

    \note If buffer_size is zero, the buffer size is not configured.
*/
void appKymeraCreateOutputChain(uint32 rate, unsigned kick_period, unsigned buffer_size, uint8 volume);

/*! \brief Configure the audio output chain.
    \param chain The chain containing the audio output chain operators.
    \sample_rate The sample rate to configure.
    \param kick_period The kymera kick period.
    \buffer_size The PCM buffer size.
    \volume The start volume.

    \note Configures the source sync, volume and latency buffer operators in the chain.

    \note Requires the chain to use standard roles for the operators, namely
    #OPR_SOURCE_SYNC,  #OPR_VOLUME_CONTROL and #OPR_LATENCY_BUFFER.

    \note If buffer_size is zero, the buffer size is not configured.

 */
void appKymeraConfigureOutputChainOperators(kymera_chain_handle_t chain,
                                            uint32 sample_rate, unsigned kick_period,
                                            unsigned buffer_size, uint8 volume);

/*! \brief Set the main kymera volume.
    \param chain The chain containing the volume control operator.
    \param volume The volume to set. Set in the range 0-127, 0 mutes the volume.
*/
void appKymeraSetMainVolume(kymera_chain_handle_t chain, uint16 volume);

/*! \brief Set the main and auxilliary kymera volume.
    \param chain The chain containing the volume control operator.
    \param volume The volume to set. Set in the range 0-127, 0 mutes the volume.
*/
void appKymeraSetVolume(kymera_chain_handle_t chain, uint16 volume);

/*! \brief Setup an external amplifier. */
void appKymeraExternalAmpSetup(void);

/*! \brief Enable or disable an external amplifier.
    \param enable TRUE to enable, FALSE to disable.
*/
void appKymeraExternalAmpControl(bool enable);

/*! \brief Configure power mode and clock frequencies of the DSP for the lowest
           power consumption possible based on the current state / codec.
   \param tone_playing Set to TRUE if a tone is currently playing.

   \note Calling this function with chains already started may cause audible
   glitches if using I2S output.
*/
void appKymeraConfigureDspPowerMode(bool tone_playing);

/*! \brief Convert volume on 0..127 scale to MIN_VOLUME_DB..MAX_VOLUME_DB and
    return the KYMERA_DB_SCALE value.
    \param volume Set to 0 to mute the output.
 */
int32 volTo60thDbGain(uint16 volume);

/*! \brief Handle request to play a tone or prompt.
    \param msg The request message.
*/
void appKymeraHandleInternalTonePromptPlay(const KYMERA_INTERNAL_TONE_PROMPT_PLAY_T *msg);

/*! \brief Handle request to start A2DP.
    \param msg The request message.
    \return TRUE if A2DP start is complete. FALSE if A2DP start is incomplete.
*/
bool appKymeraHandleInternalA2dpStart(const KYMERA_INTERNAL_A2DP_START_T *msg);

/*! \brief Handle request to stop A2DP.
    \param msg The request message.
*/
void appKymeraHandleInternalA2dpStop(const KYMERA_INTERNAL_A2DP_STOP_T *msg);

/*! \brief Handle request to set A2DP volume.
    \param volume The requested volume.
*/
void appKymeraHandleInternalA2dpSetVolume(uint16 volume);

/*! \brief Handle request to start SCO.
    \param audio_sink The BT SCO audio sink (source of SCO audio).
    \param codec The HFP codec type.
    \param wesco WESCO parameter.
    \param volume Initial volume.
    \param use_sco_fwd Set to TRUE to use SCO forwarding capable chain.
*/
void appKymeraHandleInternalScoStart(Sink sco_sink, const appKymeraScoChainInfo *info,
                                     uint8 wesco, uint16 volume);

/*! \brief Handle request to stop SCO. */
void appKymeraHandleInternalScoStop(void);

/*! \brief Handle request to set SCO volume.
    \param volume The requested volume.
*/
void appKymeraHandleInternalScoSetVolume(uint8 volume);

/*! \brief Handle request to mute the SCO microphone.
    \param mute Set to TRUE to mute the microphone.
*/
void appKymeraHandleInternalScoMicMute(bool mute);

/*! \brief Handle request to start SCO forwarding transmit.
    \param forwarding_sink The BT L2CAP sink of forwarding audio.
*/
void appKymeraHandleInternalScoForwardingStartTx(Sink forwarding_sink);

/*! \brief Handle request to stop SCO forwarding transmit. */
bool appKymeraHandleInternalScoForwardingStopTx(void);

/*! \brief Helper function to send a message for appKymeraScoFwdStartReceive() 

    \param link_source      The source used for data received from the SCO forwarding link
    \param volume           volume level to use
    \param enable_mic_fwd   Should MIC forwarding be enabled or not.
    \param pre_start_delay  Maximum number of times message is resent before being actioned
*/
void appKymeraScoSlaveStartHelper(Source link_source, uint8 volume, const appKymeraScoChainInfo *info, uint16 delay);

/*! \brief Handle request to start SCO forwarding receive proessing on slave.
 *
    \param start_req Message requesting the start
*/
void appKymeraHandleInternalScoSlaveStart(Source link_src, const appKymeraScoChainInfo *info, uint16 volume);

/*! \brief Handle request to stop SCO forwarding receive processing on slave. */
void appKymeraHandleInternalScoSlaveStop(void);

/*! \brief Switch the microphone source to transmit to the AG. */
void appKymeraSwitchSelectMic(micSelection mic);

/*! \brief Set the SPC mode.
    \param op The SPC operator.
    \param is_consumer Consumer mode if is_consumer is TRUE otherwise passthrough mode. */
void appKymeraConfigureSpcMode(Operator op, bool is_consumer);

/*! \brief Set the SPC data format.
    \param op The SPC operator.
    \param format The data format to set. */
void appKymeraConfigureSpcDataFormat(Operator op, audio_data_format format);

/*! \brief Set the SPC data format for SCO data.
    \param op The SPC operator.
    \param format The data format to set. */
void appKymeraConfigureScoSpcDataFormat(Operator op, audio_data_format format);

/*! \brief Set operator use case ids.
    \param is_sco Set TRUE if the use case is SCO based. Set to FALSE for A2DP use cases.
    \param mode SCO mode: NB, WB, SWB or UWB.
*/
void appKymeraSetOperatorUcids(bool is_sco, appKymeraScoMode mode);

void appKymeraAncTuningCreateChain(uint16 usb_rate);
void appKymeraAncTuningDestroyChain(void);


kymera_chain_handle_t appKymeraGetScoChain(void);
Operator appKymeraGetMicSwitch(void);
void appKymeraSelectSpcSwitchInput(Operator op, micSelection input);
void OperatorsAwbsSetBitpoolValue(Operator op, uint16 bitpool, bool decoder);
unsigned AudioConfigGetMicrophoneBiasVoltage(mic_bias_id id);
void AudioConfigSetRawDacGain(audio_output_t channel, uint32 raw_gain);
#endif // AV_HEADSET_KYMERA_PRIVATE_H
