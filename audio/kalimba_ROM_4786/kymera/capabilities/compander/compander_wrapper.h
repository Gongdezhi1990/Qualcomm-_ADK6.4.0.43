/****************************************************************************
  * (c) Qualcomm Technologies International, Ltd. 2015
****************************************************************************/
/**
 * \defgroup compander
 * \file  basic_compander_wrapper.h
 * \ingroup capabilities
 *
 * compander operator private header file. <br>
 *
 */

#ifndef _COMPANDER_WRAPPER_H_
#define _COMPANDER_WRAPPER_H_
/*****************************************e************************************
Include Files
*/
#include "capabilities.h"
#include "compander_wrapper.h"
#include "compander_gen_c.h"
#include "compander_c.h"
#include "compander_wrapper_defs.h"
#include "op_msg_utilities.h"
#include "base_multi_chan_op/base_multi_chan_op.h"
#include "ps/ps.h"

/* channel descriptor specific to DBE capability */
typedef struct compander_channels
{
   MULTI_CHANNEL_CHANNEL_STRUC   common;
   unsigned                      chan_mask;
   t_compander_object           *compander_object;
}compander_channels;

/* capability-specific extra operator data */
typedef struct compander_exop
{
    t_compander_object *compander_object[COMPANDER_CAP_MAX_CHANNELS];
    COMPANDER_PARAMETERS compander_cap_params;                       /**< Pointer to cur params  */
    unsigned sample_rate;
    unsigned ReInitFlag;
    unsigned Cur_mode;
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;
   
    CPS_PARAM_DEF parms_def;

    unsigned lookahead_status; 
    
} COMPANDER_OP_DATA;

/*****************************************************************************
Private Function Definitions
*/
/* ASM processing function */
extern unsigned compander_processing(COMPANDER_OP_DATA *op_data, unsigned samples_to_process, compander_channels *first);
extern void compander_initialize(COMPANDER_OP_DATA *op_data, unsigned chan_mask, compander_channels *first);

/* Message handlers */
extern bool compander_wrapper_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool compander_wrapper_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);


/* Op msg handlers */
extern bool compander_wrapper_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool compander_wrapper_opmsg_set_sample_rate(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

/* Data processing function */
extern void compander_wrapper_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

/* misc */

extern bool compander_channel_create(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr,unsigned chan_idx);
extern void compander_channel_destroy(OPERATOR_DATA *op_data,MULTI_CHANNEL_CHANNEL_STRUC *chan_ptr);

extern bool ups_params_compander(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);

#endif /* _COMPANDER_WRAPPER_H_ */
