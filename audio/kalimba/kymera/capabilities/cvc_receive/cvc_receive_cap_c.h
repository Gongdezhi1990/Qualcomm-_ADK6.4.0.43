/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  cvc_receive_cap_c.h
* \ingroup  capabilities
*
*  CVC receive
*
*/

#ifndef CVC_RCV_CAP_C_H
#define CVC_RCV_CAP_C_H

#include "cvc_recv_gen_c.h"
#include "cvc_receive.h"
#include "frame_proc/frame_proc_c.h"
#include "opmgr/opmgr_for_ops.h"
#include "volume/shared_volume_control.h"
#include "op_msg_utilities.h"
#include "ps/ps.h"

/* Capability Version */
#define CVC_RECIEVE_CAP_VERSION_MINOR            2
#if defined(INSTALL_METADATA) && defined(INSTALL_CVC_RECEIVE_SUPPORT_METADATA)
#define CVC_RECEIVE_SUPPORT_METADATA
#endif

#define RCV_VARIANT_NB   0
#define RCV_VARIANT_FE   1
#define RCV_VARIANT_WB   2
#define RCV_VARIANT_UWB  3
#define RCV_VARIANT_SWB  4
#define RCV_VARIANT_FB   5  

/**
 * Extended data stucture for CVC receive capability  
 */
typedef struct
{
    unsigned *inst_alloc;             /**< Pointer to allocated persistent memory  */
    unsigned *scratch_alloc;          /**< Pointer to allocated scratch memory  */
    tFrmBuffer *input_stream;         /**< Pointer to input Frame Sync Stream Map  */
    tFrmBuffer *output_stream;        /**< Pointer to output Frame Synn Stream Map  */

    unsigned *mode_table;             /**< Pointer to mode table  */
    unsigned *init_table;             /**< Pointer to initialization table  */
    unsigned *status_table;           /**< Pointer to status table  */
    unsigned *modules;                /**< Pointer to modules table  */
    CVC_RECV_PARAMETERS *params;      /**< Pointer to params  */
    unsigned *upsampler;              /**< Pointer to upsampler  */
    unsigned *cvclib_table;           /**< Pointer to cvclib table */
    unsigned *vad_dcb_coeffs;         /**< Pointer to vad_dcb coefficients  */
    unsigned *oms_config_ptr;         /**< Pointer to OMS config pointer  */
    unsigned *anal_fb_config;         /**< Pointer to analysis filterbank config  */
    unsigned *synth_fb_config;        /**< Pointer to synthesis filterbank config  */
    unsigned *fb_split;               /**< Pointer to FB split */
    unsigned *cur_mode_ptr;           /**< Pointer to share object between capabilities */  
    SHARED_VOLUME_CONTROL_OBJ *shared_volume_ptr;         /**< NDVC noise level */
    /** additionally used fields */
    bool     op_all_connected;
    unsigned cap_id;
    unsigned Cur_mode;      
    unsigned Host_mode;
    unsigned Obpm_mode;
    unsigned Ovr_Control;
    unsigned ReInitFlag;
    unsigned agc_state;

    unsigned apply_resample;
    unsigned *downsample_config;
    unsigned *upsample_config;

    unsigned frame_size_in;
    unsigned frame_size_out;
    unsigned sample_rate;

    CPS_PARAM_DEF parms_def;
} CVC_RECEIVE_OP_DATA;

/****************************************************************************
Private Function Definitions
*/

/* ASM processing function */
extern unsigned CVC_RCV_CAP_Create(CVC_RECEIVE_OP_DATA *op_extra_data);
extern void CVC_RCV_CAP_Destroy(CVC_RECEIVE_OP_DATA *op_extra_data);
extern void CVC_RCV_CAP_Process(CVC_RECEIVE_OP_DATA *op_extra_data);



/* Receive capability handler functions declarations */
extern bool cvc_receive_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool cvc_receive_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool cvc_receive_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool cvc_receive_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool cvc_receive_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);
extern bool cvc_receive_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data);

/* Data processing function */
extern void cvc_receive_process_data(OPERATOR_DATA*, TOUCHED_TERMINALS*);

#ifdef CVC_RECEIVE_SUPPORT_METADATA
/* metadata transfer function */
extern void cvc_receive_transport_metadata(CVC_RECEIVE_OP_DATA   *op_extra_data);
#endif

/* Operator message handlers */
extern bool cvc_receive_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool cvc_receive_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool cvc_receive_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool cvc_receive_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool cvc_receive_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern bool cvc_receive_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);
extern bool cvc_receive_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data);

extern void* MakeStandardResponse_RCV(unsigned id);
extern bool cvc_receive_ups_set_state(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, STATUS_KYMERA status, uint16 extra_status_info);
extern bool ups_state_rcv(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank, uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);
extern bool ups_params_rcv(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank, uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info);
extern void cvc_receive_release_constants(OPERATOR_DATA *op_data);

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
void cvc_receive_create_pending_cb(OPERATOR_DATA *op_data, uint16 cmd_id,void *msg_body, tRoutingInfo *rinfo,unsigned cb_value);
#endif
#endif  /* CVC_RCV_CAP_C_H */
