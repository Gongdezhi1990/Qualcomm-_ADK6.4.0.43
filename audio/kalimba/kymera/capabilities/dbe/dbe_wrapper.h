/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup dbe
 * \file  dbe_wrapper.h
 * \ingroup capabilities
 *
 * DBE operator private header file. <br>
 *
 */

#ifndef _DBE_WRAPPER_H_
#define _DBE_WRAPPER_H_
/*****************************************e************************************
Include Files
*/
#include "capabilities.h"
#include "dbe_wrapper.h"
#include "dbe_gen_c.h"
#include "dbe_wrapper_defs.h"
#include "dbe_c.h"
#include "op_msg_utilities.h"
#include "base_multi_chan_op/base_multi_chan_op.h"
#include "ps/ps.h"

/* channel descriptor specific to DBE capability */
typedef struct dbe_channels
{
   MULTI_CHANNEL_CHANNEL_STRUC   common;
   unsigned                      chan_idx;
   t_dbe_object                 *dbe_object;
}dbe_channels;

/* capability-specific extra operator data */
/* MUST BE KEPT ALLIGNED WITH FIELD OFFSETS IN basic_mono_data_processing.asm */
typedef struct dbe_exop
{
    DBE_PARAMETERS dbe_cap_params;                          /**< Pointer to cur params  */
    unsigned sample_rate;
    unsigned ReInitFlag;
    unsigned Cur_mode;
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;

    CPS_PARAM_DEF parms_def;
    unsigned int *hp1_out;
    unsigned int *hp3_out;
    unsigned int *hp2_out;
    unsigned int *ntp_tp_filters_buf;
    unsigned int *high_freq_output_buf;
    unsigned cap_config;                             /* Limits config based on capability ID */

} DBE_OP_DATA;



/*****************************************************************************
Private Function Definitions
*/
/* ASM processing function */
extern void dbe_processing(DBE_OP_DATA *op_data, unsigned samples_to_process,dbe_channels *first);
extern void dbe_initialize(DBE_OP_DATA *op_data,dbe_channels *first);

/* Message handlers */
extern bool dbe_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool dbe_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);


/* Op msg handlers */
extern bool dbe_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
//static bool dbe_wrapper_opmsg_load_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool dbe_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_METADATA
extern bool dbe_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif

/* Data processing function */
extern void dbe_wrapper_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* misc */
//void copy_params(t_peq_object* pobj, t_peq_params* p_coeff_params);
//void copy_params_from_payload(t_peq_params* p_coeff_params, unsigned* payload);
extern bool DBE_CAP_Create(DBE_OP_DATA* p_ext_data);
extern void DBE_CAP_Destroy(OPERATOR_DATA *op_data);

extern bool ups_params_dbe(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);

extern bool dbe_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx);
extern void dbe_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr);

#endif /* _DBE_WRAPPER_H_ */
