/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup xover
 * \file  basic_xover_wrapper.h
 * \ingroup capabilities
 *
 * xover operator private header file. <br>
 *
 */

#ifndef _XOVER_WRAPPER_H_
#define _XOVER_WRAPPER_H_
/*****************************************e************************************
Include Files
*/
#include "capabilities.h"
#include "xover_gen_c.h"
#include "xover_c.h"
#include "peq_c.h"
#include "xover_wrapper_defs.h"
#include "op_msg_utilities.h"


/**
 * Internal operator states
 */
typedef enum
{
    OPSTATE_INTERNAL_NOT_CREATED,
    OPSTATE_INTERNAL_CONNECTED,
    OPSTATE_INTERNAL_READY,
    OPSTATE_INTERNAL_RUNNING
}OPSTATE_INTERNAL;

/* capability-specific extra operator data */
typedef struct xover_exop
{
    tCbuffer *ip_buffer[XOVER_CAP_MAX_CHANNELS];             /** The buffer at the input terminal, null if unconnected */
    tCbuffer *op_buffer[2*XOVER_CAP_MAX_CHANNELS];           /** The buffer at the output terminal, null if unconnected */
#ifdef INSTALL_METADATA
    tCbuffer *metadata_ip_buffer;   /** The input buffer with metadata to transport from */
    tCbuffer *metadata_op_buffer;   /** The output buffer with metadata to transport to */
    unsigned metadata_delay;          /** The number of octets delay that the processing introduces */
#endif /* INSTALL_METADATA */
    t_xover_object *xover_object[XOVER_CAP_MAX_CHANNELS];
    XOVER_PARAMETERS *xover_cap_params;                      /**< Pointer to cur params  */
    unsigned sample_rate;
    unsigned ReInitFlag;
    unsigned Cur_mode;
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;
    unsigned pair_count;                                     /* number of input/output pairs */
    unsigned chan_flags;                                     /* flags of input/output pairs, one bit per pair */
    AUDIO_DATA_FORMAT ip_format;                             /** The audio data format configurations of the input terminal */
    AUDIO_DATA_FORMAT op_format;                             /** The audio data format configurations of the output terminal */
    bool op_all_connected;
    bool keep_buffers_full;                                  /** TRUE if the kicking logic should try to keep connected buffers filled */
    
    CPS_PARAM_DEF parms_def;                                 /** Common Parameter Definition */

    unsigned comp_config;
    unsigned input_active_chans; 
    unsigned output_active_chans;
    t_peq_params *peq_coeff_params_low;                      /* pointer to coeff_params of the low_freq peq_data_object */    
    t_peq_params *peq_coeff_params_high;                     /* pointer to coeff_params of the high_freq peq_data_object */
    unsigned *peq_cap_params_low;                            /* pointer to cap_params of the low_freq peq_data_object */
    unsigned *peq_cap_params_high;                           /* pointer to cap_params of the high_freq peq_data_object */

} XOVER_OP_DATA;



/*****************************************************************************
Private Function Definitions
*/
/* ASM processing function */
extern void xover_processing(XOVER_OP_DATA *op_data, unsigned samples_to_process);
extern void xover_initialize(XOVER_OP_DATA *op_data);

#endif /* _XOVER_WRAPPER_H_ */
