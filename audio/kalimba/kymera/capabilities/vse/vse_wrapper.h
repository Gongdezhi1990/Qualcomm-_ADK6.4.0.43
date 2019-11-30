/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup vse
 * \file  vse_wrapper.h
 * \ingroup capabilities
 *
 * VSE operator private header file. <br>
 *
 */

#ifndef _VSE_WRAPPER_H_
#define _VSE_WRAPPER_H_
/*****************************************e************************************
Include Files
*/
#include "capabilities.h"
#include "vse_wrapper.h"
#include "vse_gen_c.h"
#include "vse_wrapper_defs.h"
#include "vse_c.h"
#include "op_msg_utilities.h"
#include "base_multi_chan_op/base_multi_chan_op.h"
#include "ps/ps.h"

/* channel descriptor specific to VSE capability */
typedef struct vse_channels
{
   MULTI_CHANNEL_CHANNEL_STRUC   common;
   unsigned                      chan_idx;
   t_vse_object                 *vse_object;
}vse_channels;

/* capability-specific extra operator data */
/* MUST BE KEPT ALLIGNED WITH FIELD OFFSETS IN basic_mono_data_processing.asm */
typedef struct vse_exop
{
    VSE_PARAMETERS vse_cap_params;                      /**< Pointer to cur params  */
    unsigned sample_rate;
    unsigned ReInitFlag;
  
    unsigned Cur_mode;
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;

    CPS_PARAM_DEF parms_def;
} VSE_OP_DATA;



/*****************************************************************************
Private Function Definitions
*/
/* ASM processing function */
extern void vse_processing(VSE_OP_DATA *op_data, unsigned samples_to_process,vse_channels *channels);
extern void vse_initialize(VSE_OP_DATA *op_data,vse_channels *channels);

/* Message handlers */
extern bool vse_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vse_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool vse_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool vse_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool vse_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_METADATA
extern bool vse_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif

/* Data processing function */
extern void vse_wrapper_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);


extern bool ups_params_vse(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);

extern bool vse_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx);
extern void vse_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr);
#endif /* _VSE_WRAPPER_H_ */
