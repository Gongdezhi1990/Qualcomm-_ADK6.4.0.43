/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup peq
 * \ingroup capabilities
 * \file  basic_peq_wrapper.h
 * \ingroup peq
 *
 * peq operator private header file. <br>
 *
 */

#ifndef _PEQ_WRAPPER_H_
#define _PEQ_WRAPPER_H_
/*****************************************e************************************
Include Files
*/
#include "capabilities.h"
#include "peq_wrapper.h"
#include "peq_gen_c.h"
#include "peq_c.h"
#include "peq_wrapper_defs.h"
#include "op_msg_utilities.h"
#include "base_multi_chan_op/base_multi_chan_op.h"
#include "ps/ps.h"

/* channel descriptor specific to PEQ capability */
typedef struct peq_channels
{
   MULTI_CHANNEL_CHANNEL_STRUC   common;
   t_peq_object                 *peq_object;
}peq_channels;

/* capability-specific extra operator data */
typedef struct peq_exop
{
    PEQ_PARAMETERS peq_cap_params;       /**< Pointer to cur params  */
    unsigned sample_rate;
    unsigned ReInitFlag;
    unsigned coeff_override_flag;
    int      ParameterLoadStatus;

    unsigned Cur_mode;
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;
   
    CPS_PARAM_DEF parms_def;

    void* peq_proc_func; /* process function */

    /** The bit field of connected terminals for kick propagation */
    t_peq_params *peq_coeff_params; /* peq params (ceofficients) object */

    t_peq_params *peq_coeff_background; /* peq params (ceofficients) object - background generation */
} PEQ_OP_DATA;



/*****************************************************************************
Private Function Definitions
*/
/* ASM processing function */
extern void peq_processing(PEQ_OP_DATA *op_data,peq_channels *first,unsigned samples_to_process);
extern void peq_initialize(PEQ_OP_DATA *op_data,peq_channels *first);
extern void peq_compute_coefficients(PEQ_OP_DATA *op_data,t_peq_params *dest);

/* Message handlers */
extern bool peq_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool peq_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool peq_wrapper_start(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Op msg handlers */
extern bool peq_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_override_coeffs(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_load_config(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool peq_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#ifdef INSTALL_METADATA
extern bool peq_wrapper_opmsg_set_metadata_delay(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
#endif

/* Data processing function */
extern void peq_wrapper_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* misc */
extern void PEQ_CAP_Destroy(OPERATOR_DATA *op_data);
extern bool ups_params_peq(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);
extern bool peq_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx);
extern void peq_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr);

extern void peq_coeff_change(PEQ_OP_DATA  *p_ext_data);

#endif /* _PEQ_WRAPPER_H_ */
