/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup secure_basic_passthrough
 * \file  secure_basic_passthrough_struct.h
 * \ingroup capabilities
 *
 * Secure basic passthrough operator header file containing type definitions
 * shared between C and asm. <br>
 *
 */

#ifndef _SECURE_BASIC_PASSTHROUGH_STRUCT_H_
#define _SECURE_BASIC_PASSTHROUGH_STRUCT_H_
/*****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"
#include "audio_fadeout.h"
#include "stream/stream_audio_data_format.h"
#include "secure_basic_passthrough_shared_const.h"
#include "ttp/ttp.h"
#include "op_msg_utilities.h"
#include "ps/ps.h"

#ifdef INSTALL_LICENSE_CHECK
#include "third_party_licensing_example.h"
#endif

/****************************************************************************
Public Constant Definitions
*/


/****************************************************************************
Public Type Declarations
*/

/** Basic passthrough per channel capability data */
typedef struct PASSTHROUGH_CHANNEL
{
    /** The buffer at the input terminal for this channel */
    tCbuffer *ip_buffer;

    /** The buffer at the output terminal for this channel */
    tCbuffer *op_buffer;

    /** Fade-out parameters for this channel */
    FADEOUT_PARAMS fadeout_parameters;

    /** The number channel number of this channel as viewed externally to the
     * operator.
     */
    unsigned channel_num;
} PASSTHROUGH_CHANNEL;

/** basic passthrough capability specific extra operator data */
typedef struct SECURE_BASIC_PASSTHROUGH_OP_DATA
{
    /** operator copy function, this varies depending on the type the operator
     * is set to. e.g. Data, Audio, Garg, XOR */
    void *copy_function;

    /** The audio data format configurations of the input terminal */
    AUDIO_DATA_FORMAT ip_format;

    /** The audio data format configurations of the output terminal */
    AUDIO_DATA_FORMAT op_format;

    /** Flag indicating whether only a single channel needs to be considered to
     * determine amount of data and space.
     */
    bool simple_data_test_safe;

#ifdef INSTALL_TTP
    /** Time-to-play context */
    ttp_context *time_to_play;
#endif

    /** Latency buffer size */
    unsigned latency_buffer_size;

    /** Configured sample rate */
    unsigned sample_rate;

    /** The number of channels currently active */
    unsigned num_active_chans;

    /** Bitfield indicates which channels are currently active. */
    unsigned active_chans;

    /** List of connected channels. This list is ordered so those with 
     * both ip and op connected are at the top */
    PASSTHROUGH_CHANNEL *channel[MAX_CHANS];

    /** TRUE if the operator should kick forwards if limited by output space */
    bool kick_on_full_output;

#ifdef INSTALL_METADATA
    /** The input buffer with metadata to transport from */
    tCbuffer *metadata_ip_buffer;
    /** The output buffer with metadata to transport to */
    tCbuffer *metadata_op_buffer;
#endif /* INSTALL_METADATA */

    /** UCID for Parameter PsStore */
    unsigned  ucid;

    unsigned ReInitFlag;

#if defined(INSTALL_LICENSE_CHECK) && defined(LEGACY_LICENSING)
    SECURE_BASIC_PASSTHROUGH_LICENSING lic;
#endif

} SECURE_BASIC_PASSTHROUGH_OP_DATA;

#endif /* _SECURE_BASIC_PASSTHROUGH_STRUCT_H_ */
