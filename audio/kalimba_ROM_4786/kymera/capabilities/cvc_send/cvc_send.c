/**
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
* \file  cvc_send.c
* \ingroup  capabilities
*
*  CVC send
*
*/

/****************************************************************************
Include Files
*/
#include "capabilities.h"
#include "cvc_send_cap_c.h"
#include "adaptor/adaptor.h"
#include "mem_utils/dynloader.h"
#include "mem_utils/exported_constants.h"
#include "mem_utils/exported_constant_files.h"
#include "ps/ps.h"
#include "obpm_prim.h"  
#include "op_msg_helpers.h"
#include "cvc_processing_c.h"
#include "platform/pl_fractional.h"

#include "patch/patch.h"
/****************************************************************************

Local Definitions
*/

/* Reference inputs 0-2 */
#define CVC_SEND_NUM_INPUTS_MASK    0x7

/* CVS send terminal mic 0*/
#define CVC_SEND_TERMINAL_MIC0      1

/****************************************************************************
Private Constant Definitions
*/

#ifdef CAPABILITY_DOWNLOAD_BUILD
#define CVCHS1MIC_SEND_NB_CAP_ID           CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_NB
#define CVCHS1MIC_SEND_WB_CAP_ID           CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_WB
#define CVCHS1MIC_SEND_UWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_UWB
#define CVCHS1MIC_SEND_SWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_SWB
#define CVCHS1MIC_SEND_FB_CAP_ID           CAP_ID_DOWNLOAD_CVCHS1MIC_SEND_FB
#define CVCHS2MIC_MONO_SEND_NB_CAP_ID      CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_NB
#define CVCHS2MIC_MONO_SEND_WB_CAP_ID      CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_WB
#define CVCHS2MIC_MONO_SEND_UWB_CAP_ID     CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_UWB
#define CVCHS2MIC_MONO_SEND_SWB_CAP_ID     CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_SWB
#define CVCHS2MIC_MONO_SEND_FB_CAP_ID      CAP_ID_DOWNLOAD_CVCHS2MIC_MONO_SEND_FB
#define CVCHS2MIC_BINAURAL_SEND_NB_CAP_ID  CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_NB
#define CVCHS2MIC_BINAURAL_SEND_WB_CAP_ID  CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_WB
#define CVCHS2MIC_BINAURAL_SEND_UWB_CAP_ID CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_UWB
#define CVCHS2MIC_BINAURAL_SEND_SWB_CAP_ID CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_SWB
#define CVCHS2MIC_BINAURAL_SEND_FB_CAP_ID  CAP_ID_DOWNLOAD_CVCHS2MIC_BINAURAL_SEND_FB
#define CVCHF1MIC_SEND_NB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_NB
#define CVCHF1MIC_SEND_WB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_WB
#define CVCHF1MIC_SEND_UWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_UWB
#define CVCHF1MIC_SEND_SWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_SWB
#define CVCHF1MIC_SEND_FB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF1MIC_SEND_FB
#define CVCHF2MIC_SEND_NB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_NB
#define CVCHF2MIC_SEND_WB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_WB
#define CVCHF2MIC_SEND_UWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_UWB
#define CVCHF2MIC_SEND_SWB_CAP_ID          CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_SWB
#define CVCHF2MIC_SEND_FB_CAP_ID           CAP_ID_DOWNLOAD_CVCHF2MIC_SEND_FB
#define CVCSPKR1MIC_SEND_NB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_NB
#define CVCSPKR1MIC_SEND_WB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_WB
#define CVCSPKR1MIC_SEND_UWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_UWB
#define CVCSPKR1MIC_SEND_SWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_SWB
#define CVCSPKR1MIC_SEND_FB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_FB
#define CVCSPKR2MIC_SEND_NB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_NB
#define CVCSPKR2MIC_SEND_WB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_WB
#define CVCSPKR2MIC_SEND_UWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_UWB
#define CVCSPKR2MIC_SEND_SWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_SWB
#define CVCSPKR2MIC_SEND_FB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR2MIC_SEND_FB
#define CVCSPKR3MIC_SEND_NB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_NB
#define CVCSPKR3MIC_SEND_WB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_WB
#define CVCSPKR3MIC_SEND_UWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_UWB
#define CVCSPKR3MIC_SEND_SWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_SWB
#define CVCSPKR3MIC_SEND_FB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR3MIC_SEND_FB
#define CVCSPKR3MIC_CIRC_SEND_UWB_CAP_ID   CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_UWB
#define CVCSPKR3MIC_CIRC_SEND_SWB_CAP_ID   CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_SWB
#define CVCSPKR3MIC_CIRC_SEND_FB_CAP_ID    CAP_ID_DOWNLOAD_CVCSPKR3MIC_CIRC_SEND_FB
#define CVCSPKR4MIC_SEND_NB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_NB
#define CVCSPKR4MIC_SEND_WB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_WB
#define CVCSPKR4MIC_SEND_UWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_UWB
#define CVCSPKR4MIC_SEND_SWB_CAP_ID        CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_SWB
#define CVCSPKR4MIC_SEND_FB_CAP_ID         CAP_ID_DOWNLOAD_CVCSPKR4MIC_SEND_FB
#define CVCSPKR4MIC_CIRC_SEND_UWB_CAP_ID   CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_UWB
#define CVCSPKR4MIC_CIRC_SEND_SWB_CAP_ID   CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_SWB
#define CVCSPKR4MIC_CIRC_SEND_FB_CAP_ID    CAP_ID_DOWNLOAD_CVCSPKR4MIC_CIRC_SEND_FB
#else
#define CVCHS1MIC_SEND_NB_CAP_ID           CAP_ID_CVCHS1MIC_SEND_NB
#define CVCHS1MIC_SEND_WB_CAP_ID           CAP_ID_CVCHS1MIC_SEND_WB
#define CVCHS1MIC_SEND_UWB_CAP_ID          CAP_ID_CVCHS1MIC_SEND_UWB
#define CVCHS1MIC_SEND_SWB_CAP_ID          CAP_ID_CVCHS1MIC_SEND_SWB
#define CVCHS1MIC_SEND_FB_CAP_ID           CAP_ID_CVCHS1MIC_SEND_FB
#define CVCHS2MIC_MONO_SEND_NB_CAP_ID      CAP_ID_CVCHS2MIC_MONO_SEND_NB
#define CVCHS2MIC_MONO_SEND_WB_CAP_ID      CAP_ID_CVCHS2MIC_MONO_SEND_WB
#define CVCHS2MIC_MONO_SEND_UWB_CAP_ID     CAP_ID_CVCHS2MIC_MONO_SEND_UWB
#define CVCHS2MIC_MONO_SEND_SWB_CAP_ID     CAP_ID_CVCHS2MIC_MONO_SEND_SWB
#define CVCHS2MIC_MONO_SEND_FB_CAP_ID      CAP_ID_CVCHS2MIC_MONO_SEND_FB
#define CVCHS2MIC_BINAURAL_SEND_NB_CAP_ID  CAP_ID_CVCHS2MIC_BINAURAL_SEND_NB
#define CVCHS2MIC_BINAURAL_SEND_WB_CAP_ID  CAP_ID_CVCHS2MIC_BINAURAL_SEND_WB
#define CVCHS2MIC_BINAURAL_SEND_UWB_CAP_ID CAP_ID_CVCHS2MIC_BINAURAL_SEND_UWB
#define CVCHS2MIC_BINAURAL_SEND_SWB_CAP_ID CAP_ID_CVCHS2MIC_BINAURAL_SEND_SWB
#define CVCHS2MIC_BINAURAL_SEND_FB_CAP_ID  CAP_ID_CVCHS2MIC_BINAURAL_SEND_FB
#define CVCHF1MIC_SEND_NB_CAP_ID           CAP_ID_CVCHF1MIC_SEND_NB
#define CVCHF1MIC_SEND_WB_CAP_ID           CAP_ID_CVCHF1MIC_SEND_WB
#define CVCHF1MIC_SEND_UWB_CAP_ID          CAP_ID_CVCHF1MIC_SEND_UWB
#define CVCHF1MIC_SEND_SWB_CAP_ID          CAP_ID_CVCHF1MIC_SEND_SWB
#define CVCHF1MIC_SEND_FB_CAP_ID           CAP_ID_CVCHF1MIC_SEND_FB
#define CVCHF2MIC_SEND_NB_CAP_ID           CAP_ID_CVCHF2MIC_SEND_NB
#define CVCHF2MIC_SEND_WB_CAP_ID           CAP_ID_CVCHF2MIC_SEND_WB
#define CVCHF2MIC_SEND_UWB_CAP_ID          CAP_ID_CVCHF2MIC_SEND_UWB
#define CVCHF2MIC_SEND_SWB_CAP_ID          CAP_ID_CVCHF2MIC_SEND_SWB
#define CVCHF2MIC_SEND_FB_CAP_ID           CAP_ID_CVCHF2MIC_SEND_FB
#define CVCSPKR1MIC_SEND_NB_CAP_ID         CAP_ID_CVCSPKR1MIC_SEND_NB
#define CVCSPKR1MIC_SEND_WB_CAP_ID         CAP_ID_CVCSPKR1MIC_SEND_WB
#define CVCSPKR1MIC_SEND_UWB_CAP_ID        CAP_ID_CVCSPKR1MIC_SEND_UWB
#define CVCSPKR1MIC_SEND_SWB_CAP_ID        CAP_ID_CVCSPKR1MIC_SEND_SWB
#define CVCSPKR1MIC_SEND_FB_CAP_ID         CAP_ID_CVCSPKR1MIC_SEND_FB
#define CVCSPKR2MIC_SEND_NB_CAP_ID         CAP_ID_CVCSPKR2MIC_SEND_NB
#define CVCSPKR2MIC_SEND_WB_CAP_ID         CAP_ID_CVCSPKR2MIC_SEND_WB
#define CVCSPKR2MIC_SEND_UWB_CAP_ID        CAP_ID_CVCSPKR2MIC_SEND_UWB
#define CVCSPKR2MIC_SEND_SWB_CAP_ID        CAP_ID_CVCSPKR2MIC_SEND_SWB
#define CVCSPKR2MIC_SEND_FB_CAP_ID         CAP_ID_CVCSPKR2MIC_SEND_FB
#define CVCSPKR3MIC_SEND_NB_CAP_ID         CAP_ID_CVCSPKR3MIC_SEND_NB
#define CVCSPKR3MIC_SEND_WB_CAP_ID         CAP_ID_CVCSPKR3MIC_SEND_WB
#define CVCSPKR3MIC_SEND_UWB_CAP_ID        CAP_ID_CVCSPKR3MIC_SEND_UWB
#define CVCSPKR3MIC_SEND_SWB_CAP_ID        CAP_ID_CVCSPKR3MIC_SEND_SWB
#define CVCSPKR3MIC_SEND_FB_CAP_ID         CAP_ID_CVCSPKR3MIC_SEND_FB
#define CVCSPKR3MIC_CIRC_SEND_UWB_CAP_ID   CAP_ID_CVCSPKR3MIC_CIRC_SEND_UWB
#define CVCSPKR3MIC_CIRC_SEND_SWB_CAP_ID   CAP_ID_CVCSPKR3MIC_CIRC_SEND_SWB
#define CVCSPKR3MIC_CIRC_SEND_FB_CAP_ID    CAP_ID_CVCSPKR3MIC_CIRC_SEND_FB
#define CVCSPKR4MIC_SEND_NB_CAP_ID         CAP_ID_CVCSPKR4MIC_SEND_NB
#define CVCSPKR4MIC_SEND_WB_CAP_ID         CAP_ID_CVCSPKR4MIC_SEND_WB
#define CVCSPKR4MIC_SEND_UWB_CAP_ID        CAP_ID_CVCSPKR4MIC_SEND_UWB
#define CVCSPKR4MIC_SEND_SWB_CAP_ID        CAP_ID_CVCSPKR4MIC_SEND_SWB
#define CVCSPKR4MIC_SEND_FB_CAP_ID         CAP_ID_CVCSPKR4MIC_SEND_FB
#define CVCSPKR4MIC_CIRC_SEND_UWB_CAP_ID   CAP_ID_CVCSPKR4MIC_CIRC_SEND_UWB
#define CVCSPKR4MIC_CIRC_SEND_SWB_CAP_ID   CAP_ID_CVCSPKR4MIC_CIRC_SEND_SWB
#define CVCSPKR4MIC_CIRC_SEND_FB_CAP_ID    CAP_ID_CVCSPKR4MIC_CIRC_SEND_FB
#endif

/* Message handlers */

/** The cvc send capability function handler table */
const handler_lookup_struct cvc_send_handler_table =
{
    cvc_send_create,          /* OPCMD_CREATE */
    cvc_send_destroy,         /* OPCMD_DESTROY */
    base_op_start,            /* OPCMD_START */
    base_op_stop,             /* OPCMD_STOP */
    base_op_reset,            /* OPCMD_RESET */
    cvc_send_connect,         /* OPCMD_CONNECT */
    cvc_send_disconnect,      /* OPCMD_DISCONNECT */
    cvc_send_buffer_details,  /* OPCMD_BUFFER_DETAILS */
    base_op_get_data_format,  /* OPCMD_DATA_FORMAT */
    cvc_send_get_sched_info   /* OPCMD_GET_SCHED_INFO */
};

/* Null terminated operator message handler table */

const opmsg_handler_lookup_table_entry cvc_send_opmsg_handler_table[] =
    {{OPMSG_COMMON_ID_GET_CAPABILITY_VERSION,           base_op_opmsg_get_capability_version},
    {OPMSG_COMMON_ID_SET_CONTROL,                       cvc_send_opmsg_obpm_set_control},
    {OPMSG_COMMON_ID_GET_PARAMS,                        cvc_send_opmsg_obpm_get_params},
    {OPMSG_COMMON_ID_GET_DEFAULTS,                      cvc_send_opmsg_obpm_get_defaults},
    {OPMSG_COMMON_ID_SET_PARAMS,                        cvc_send_opmsg_obpm_set_params},
    {OPMSG_COMMON_ID_GET_STATUS,                        cvc_send_opmsg_obpm_get_status},

    {OPMSG_COMMON_ID_SET_UCID,                          cvc_send_opmsg_set_ucid},
    {OPMSG_COMMON_ID_GET_LOGICAL_PS_ID,                 cvc_send_opmsg_get_ps_id},
    {OPMSG_COMMON_GET_VOICE_QUALITY,                    cvc_send_opmsg_get_voice_quality},

    {0, NULL}};


/****************************************************************************
CVC send capability data declarations
*/
#ifdef INSTALL_OPERATOR_CVC_HEADSET_1MIC
    const CAPABILITY_DATA cvc_send_1mic_nb_hs_cap_data =
        {
            CVCHS1MIC_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_HS_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_1mic_wb_hs_cap_data =
        {
            CVCHS1MIC_SEND_WB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_HS_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_1mic_uwb_hs_cap_data =
        {
            CVCHS1MIC_SEND_UWB_CAP_ID,      /* Capability ID */
            CVC_SEND_1M_HS_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif

#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_1mic_swb_hs_cap_data =
        {
            CVCHS1MIC_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_HS_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_1mic_fb_hs_cap_data =
        {
            CVCHS1MIC_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_HS_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO
    const CAPABILITY_DATA cvc_send_2mic_hs_mono_nb_cap_data =
        {
            CVCHS2MIC_MONO_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSE_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_2mic_hs_mono_wb_cap_data =
        {
            CVCHS2MIC_MONO_SEND_WB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSE_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_2mic_hs_mono_uwb_cap_data =
        {
            CVCHS2MIC_MONO_SEND_UWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSE_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_2mic_hs_mono_swb_cap_data =
        {
            CVCHS2MIC_MONO_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSE_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_2mic_hs_mono_fb_cap_data =
        {
            CVCHS2MIC_MONO_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSE_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_BINAURAL
    const CAPABILITY_DATA cvc_send_2mic_hs_binaural_nb_cap_data =
        {
            CVCHS2MIC_BINAURAL_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSB_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_2mic_hs_binaural_wb_cap_data =
        {
            CVCHS2MIC_BINAURAL_SEND_WB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSB_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_2mic_hs_binaural_uwb_cap_data =
        {
            CVCHS2MIC_BINAURAL_SEND_UWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSB_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_2mic_hs_binaural_swb_cap_data =
        {
            CVCHS2MIC_BINAURAL_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSB_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_2mic_hs_binaural_fb_cap_data =
        {
            CVCHS2MIC_BINAURAL_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_HSB_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_1MIC
    const CAPABILITY_DATA cvc_send_1mic_nb_auto_cap_data =
        {
            CVCHF1MIC_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_AUTO_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_1mic_wb_auto_cap_data =
        {
            CVCHF1MIC_SEND_WB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_AUTO_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_1mic_uwb_auto_cap_data =
        {
            CVCHF1MIC_SEND_UWB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_AUTO_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_1mic_swb_auto_cap_data =
        {
            CVCHF1MIC_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_AUTO_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_1mic_fb_auto_cap_data =
        {
            CVCHF1MIC_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_AUTO_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_2MIC
    const CAPABILITY_DATA cvc_send_2mic_nb_auto_cap_data =
        {
            CVCHF2MIC_SEND_NB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_2M_AUTO_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_2mic_wb_auto_cap_data =
        {
            CVCHF2MIC_SEND_WB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_2M_AUTO_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_2mic_uwb_auto_cap_data =
        {
            CVCHF2MIC_SEND_UWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_2M_AUTO_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_2mic_swb_auto_cap_data =
        {
            CVCHF2MIC_SEND_SWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_2M_AUTO_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_2mic_fb_auto_cap_data =
        {
            CVCHF2MIC_SEND_FB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_2M_AUTO_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_1MIC
    const CAPABILITY_DATA cvc_send_1mic_speaker_nb_cap_data =
        {
            CVCSPKR1MIC_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_SPKR_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_1mic_speaker_wb_cap_data =
        {
            CVCSPKR1MIC_SEND_WB_CAP_ID,          /* Capability ID */
            CVC_SEND_1M_SPKR_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_1mic_speaker_uwb_cap_data =
        {
            CVCSPKR1MIC_SEND_UWB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_SPKR_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_1mic_speaker_swb_cap_data =
        {
            CVCSPKR1MIC_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_SPKR_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_1mic_speaker_fb_cap_data =
        {
            CVCSPKR1MIC_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_1M_SPKR_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            2, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_2MIC
    const CAPABILITY_DATA cvc_send_2mic_speaker_nb_cap_data =
        {
            CVCSPKR2MIC_SEND_NB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_SPKRB_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_2mic_speaker_wb_cap_data =
        {
            CVCSPKR2MIC_SEND_WB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_SPKRB_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_2mic_speaker_uwb_cap_data =
        {
            CVCSPKR2MIC_SEND_UWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_SPKRB_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_2mic_speaker_swb_cap_data =
        {
            CVCSPKR2MIC_SEND_SWB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_SPKRB_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_2mic_speaker_fb_cap_data =
        {
            CVCSPKR2MIC_SEND_FB_CAP_ID,       /* Capability ID */
            CVC_SEND_2M_SPKRB_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            3, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_3MIC
    const CAPABILITY_DATA cvc_send_3mic_speaker_nb_cap_data =
        {
            CVCSPKR3MIC_SEND_NB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRB_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_3mic_speaker_wb_cap_data =
        {
            CVCSPKR3MIC_SEND_WB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRB_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_3mic_speaker_uwb_cap_data =
        {
            CVCSPKR3MIC_SEND_UWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRB_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_3mic_speaker_swb_cap_data =
        {
            CVCSPKR3MIC_SEND_SWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRB_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_3mic_speaker_fb_cap_data =
        {
            CVCSPKR3MIC_SEND_FB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRB_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_3mic_circ_speaker_uwb_cap_data =
        {
            CVCSPKR3MIC_CIRC_SEND_UWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRCIRC_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_3mic_circ_speaker_swb_cap_data =
        {
            CVCSPKR3MIC_CIRC_SEND_SWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRCIRC_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_3mic_circ_speaker_fb_cap_data =
        {
            CVCSPKR3MIC_CIRC_SEND_FB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_3M_SPKRCIRC_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            4, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_4MIC
    const CAPABILITY_DATA cvc_send_4mic_speaker_nb_cap_data =
        {
            CVCSPKR4MIC_SEND_NB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRB_NB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

    const CAPABILITY_DATA cvc_send_4mic_speaker_wb_cap_data =
        {
            CVCSPKR4MIC_SEND_WB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRB_WB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };

#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_4mic_speaker_uwb_cap_data =
        {
            CVCSPKR4MIC_SEND_UWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRB_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_4mic_speaker_swb_cap_data =
        {
            CVCSPKR4MIC_SEND_SWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRB_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_4mic_speaker_fb_cap_data =
        {
            CVCSPKR4MIC_SEND_FB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRB_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_24K
    const CAPABILITY_DATA cvc_send_4mic_circ_speaker_uwb_cap_data =
        {
            CVCSPKR4MIC_CIRC_SEND_UWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRCIRC_UWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_32K
    const CAPABILITY_DATA cvc_send_4mic_circ_speaker_swb_cap_data =
        {
            CVCSPKR4MIC_CIRC_SEND_SWB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRCIRC_SWB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#ifdef INSTALL_OPERATOR_CVC_48K
    const CAPABILITY_DATA cvc_send_4mic_circ_speaker_fb_cap_data =
        {
            CVCSPKR4MIC_CIRC_SEND_FB_CAP_ID,                                                       /* Capability ID */
            CVC_SEND_4M_SPKRCIRC_FB_VERSION_MAJOR, CVC_SEND_CAP_VERSION_MINOR, /* Version information - hi and lo parts */
            5, 1,                            /* Max number of sinks/inputs and sources/outputs */
            &cvc_send_handler_table,      /* Pointer to message handler function table */
            cvc_send_opmsg_handler_table, /* Pointer to operator message handler function table */
            cvc_send_process_data,        /* Pointer to data processing function */
            0,                               /* TODO - Processing time information */
            sizeof(CVC_SEND_OP_DATA)      /* Size of capability-specific per-instance data */
        };
#endif
#endif

typedef void (*cvc_send_config_function_type)(CVC_SEND_OP_DATA *op_extra_data, unsigned data_variant);

typedef struct cvc_send_config_data
{
    cvc_send_config_function_type config_func;
    unsigned cap_ids[NUM_DATA_VARIANTS];
} cvc_send_config_data;

/* 
 *   cVc send capability configuration table:
 *       cap1_config_func(), cap1_id_nb, cap1_id_wb, cap1_id_uwb, cap1_id_swb, cap1_id_fb,
 *       cap2_config_func(), cap2_id_nb, cap2_id_wb, cap2_id_uwb, cap2_id_swb, cap2_id_fb,
 *       ...
 *       0;
 */ 
cvc_send_config_data cvc_send_caps[] = {
#ifdef INSTALL_OPERATOR_CVC_HEADSET_1MIC
    {
        &CVC_SEND_CAP_Config_headset_1mic,
        {
            CAP_ID_CVCHS1MIC_SEND_NB,  CAP_ID_CVCHS1MIC_SEND_WB,
            CAP_ID_CVCHS1MIC_SEND_UWB, CAP_ID_CVCHS1MIC_SEND_SWB, CAP_ID_CVCHS1MIC_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_2MIC_MONO
    {
        &CVC_SEND_CAP_Config_headset_2mic_mono,
        {
            CAP_ID_CVCHS2MIC_MONO_SEND_NB,   CAP_ID_CVCHS2MIC_MONO_SEND_WB,
            CAP_ID_CVCHS2MIC_MONO_SEND_UWB,  CAP_ID_CVCHS2MIC_MONO_SEND_SWB,  CAP_ID_CVCHS2MIC_MONO_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_HEADSET_BINAURAL
    {
        &CVC_SEND_CAP_Config_headset_2mic_binaural,
        {
            CAP_ID_CVCHS2MIC_BINAURAL_SEND_NB,  CAP_ID_CVCHS2MIC_BINAURAL_SEND_WB,
            CAP_ID_CVCHS2MIC_BINAURAL_SEND_UWB, CAP_ID_CVCHS2MIC_BINAURAL_SEND_SWB, CAP_ID_CVCHS2MIC_BINAURAL_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_1MIC
    {
        &CVC_SEND_CAP_Config_speaker_1mic,
        {
            CAP_ID_CVCSPKR1MIC_SEND_NB,   CAP_ID_CVCSPKR1MIC_SEND_WB,
            CAP_ID_CVCSPKR1MIC_SEND_UWB,  CAP_ID_CVCSPKR1MIC_SEND_SWB,  CAP_ID_CVCSPKR1MIC_SEND_FB
        }
    },
#ifdef CAPABILITY_DOWNLOAD_BUILD
    {
        &CVC_SEND_CAP_Config_speaker_1mic,
        {
            0, CAP_ID_DOWNLOAD_CVCSPKR1MIC_SEND_WB,
            0, 0, 0
        }
    },
#endif
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_2MIC
    {
        &CVC_SEND_CAP_Config_speaker_2mic,
        {
            CAP_ID_CVCSPKR2MIC_SEND_NB,   CAP_ID_CVCSPKR2MIC_SEND_WB,
            CAP_ID_CVCSPKR2MIC_SEND_UWB,  CAP_ID_CVCSPKR2MIC_SEND_SWB,  CAP_ID_CVCSPKR2MIC_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_3MIC
    {
        &CVC_SEND_CAP_Config_speaker_3mic,
        {
            CAP_ID_CVCSPKR3MIC_SEND_NB,   CAP_ID_CVCSPKR3MIC_SEND_WB,
            CAP_ID_CVCSPKR3MIC_SEND_UWB,  CAP_ID_CVCSPKR3MIC_SEND_SWB,  CAP_ID_CVCSPKR3MIC_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_SPEAKER_4MIC
    {
        &CVC_SEND_CAP_Config_speaker_4mic,
        {
            CAP_ID_CVCSPKR4MIC_SEND_NB,   CAP_ID_CVCSPKR4MIC_SEND_WB,
            CAP_ID_CVCSPKR4MIC_SEND_UWB,  CAP_ID_CVCSPKR4MIC_SEND_SWB,  CAP_ID_CVCSPKR4MIC_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_1MIC
    {
        &CVC_SEND_CAP_Config_auto_1mic,
        {
            CAP_ID_CVCHF1MIC_SEND_NB,  CAP_ID_CVCHF1MIC_SEND_WB,
            CAP_ID_CVCHF1MIC_SEND_UWB, CAP_ID_CVCHF1MIC_SEND_SWB, CAP_ID_CVCHF1MIC_SEND_FB
        }
    },
#endif

#ifdef INSTALL_OPERATOR_CVC_AUTO_2MIC
    {
        &CVC_SEND_CAP_Config_auto_2mic,
        {
            CAP_ID_CVCHF2MIC_SEND_NB,  CAP_ID_CVCHF2MIC_SEND_WB,
            CAP_ID_CVCHF2MIC_SEND_UWB, CAP_ID_CVCHF2MIC_SEND_SWB, CAP_ID_CVCHF2MIC_SEND_FB
        }
    },
#endif
    {
        /* end of table */
        NULL, {0, 0, 0, 0, 0}
    }
};


/*
 * cvc_send_config(CVC_SEND_OP_DATA *op_extra_data)
 *    Search for op_extra_data->cap_id from cvc_send_caps[] table, if found
 *
 *       set the following field based on specified cap_id:
 *          op_extra_data->data_variant
 *          op_extra_data->major_config
 *          op_extra_data->num_mics
 *          op_extra_data->mic_config
 *          op_extra_data->frame_size
 *          op_extra_data->sample_rate
 *
 *       and initialize other internal fields
 */
static bool cvc_send_config(CVC_SEND_OP_DATA *op_extra_data)
{
    cvc_send_config_data *caps;
    unsigned cap_id = op_extra_data->cap_id;
    unsigned variant = 0;

    for (caps = cvc_send_caps; caps->config_func != NULL; caps++)
    {
        for (variant = DATA_VARIANT_NB; variant <= DATA_VARIANT_FB; variant++)
        {
            if (caps->cap_ids[variant] == cap_id) 
            {
                caps->config_func(op_extra_data, variant);

                switch(op_extra_data->data_variant)
                {
                case DATA_VARIANT_WB:  // 16 kHz
                    op_extra_data->frame_size = 120;
                    op_extra_data->sample_rate = 16000;
                    break;
                case DATA_VARIANT_UWB: // 24 kHz
                    op_extra_data->frame_size = 120;
                    op_extra_data->sample_rate = 24000;
                    break;
                case DATA_VARIANT_SWB: // 32 kHz
                    op_extra_data->frame_size = 240;
                    op_extra_data->sample_rate = 32000;
                    break;
                case DATA_VARIANT_FB:  // 48 kHz
                    op_extra_data->frame_size = 240;
                    op_extra_data->sample_rate = 48000;
                    break;
                case DATA_VARIANT_NB:  // 8 kHz
                default:
                    op_extra_data->frame_size = 60;
                    op_extra_data->sample_rate = 8000;
                    break;          
                }
                op_extra_data->ReInitFlag = 1;
                op_extra_data->Host_mode = CVC_SEND_SYSMODE_FULL;
                op_extra_data->Cur_mode = CVC_SEND_SYSMODE_STANDBY;
                op_extra_data->cur_mode_ptr     = &op_extra_data->Cur_mode;
                op_extra_data->mute_control_ptr = &op_extra_data->Cur_Mute;

                return TRUE;
            }
        }
    }

   return FALSE;
}

/****************************************************************************
Public Function Declarations
*/

void* MakeStandardResponse_SND(unsigned id)
{
    OP_STD_RSP  *std_resp = xpnew(OP_STD_RSP);

    if(!std_resp)
    {
        return(NULL);
    }
    std_resp->op_id = id;
    std_resp->resp_data.err_code = 0;
    std_resp->status = STATUS_CMD_FAILED;
    return((void*)std_resp);
}


/* ********************************** API functions ************************************* */

bool ups_state_snd(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    if((length==2)&&(status==STATUS_OK))
    {
        op_extra_data->mdgc_gain = ((data[0]<<16)&0xFFFF) | (data[1]&0xFFFF);
        /* Set the Reinit flag after setting the paramters */
        op_extra_data->ReInitFlag = 1;
    }

    return(TRUE);
}

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
void cvc_send_create_pending_cb(OPERATOR_DATA *op_data,
                              uint16 cmd_id,void *msg_body,
                              tRoutingInfo *rinfo,unsigned cb_value)
{
    external_constant_callback_when_available((void*)cb_value,mkqid(op_data->task_id,1),cmd_id,msg_body,rinfo);
}

#endif

void cvc_send_release_constants(OPERATOR_DATA *op_data)
{
    UNUSED(op_data);

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
    CVC_SEND_OP_DATA *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;

    external_constant_release(cvclib_dataDynTable_Main,op_data->id);
    external_constant_release(aec510_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(ASF100_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(oms270_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(filter_bank_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(vad400_DynamicMemDynTable_Main,op_data->id);
    external_constant_release(op_extra_data->dyn_main,op_data->id);
#endif
}

bool cvc_send_create(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;

    patch_fn_shared(cvc_send_wrapper);
    /* Setup Response to Creation Request.   Assume Failure*/
    *response_id = OPCMD_CREATE;
    if((*response_data = MakeStandardResponse_SND(op_data->id))==NULL)
    {
        /* We call cvc_send_release_constants as there is a slim chance we fail on
         * the second pass through */
        cvc_send_release_constants(op_data);
        return(FALSE);
    }

    /* Initialize extended data for operator.  Assume intialized to zero*/
    op_extra_data->cap_id = op_data->cap_data->id;

    /* Capability Specific Configuration */
    if (FALSE == cvc_send_config(op_extra_data)) {
        return(TRUE);
    }

#if defined(INSTALL_OPERATOR_CREATE_PENDING) && defined(INSTALL_CAPABILITY_CONSTANT_EXPORT)
    /* Reserve (and request) any dynamic memory tables that may be in external
     * file system.
     * A negative return value indicates a fatal error */
    if (   !external_constant_reserve(cvclib_dataDynTable_Main,op_data->id)
        || !external_constant_reserve(aec510_DynamicMemDynTable_Main,op_data->id)
        || !external_constant_reserve(ASF100_DynamicMemDynTable_Main,op_data->id)
        || !external_constant_reserve(oms270_DynamicMemDynTable_Main,op_data->id)
        || !external_constant_reserve(filter_bank_DynamicMemDynTable_Main,op_data->id)
        || !external_constant_reserve(vad400_DynamicMemDynTable_Main,op_data->id)
        || !external_constant_reserve(op_extra_data->dyn_main,op_data->id))
    {
        L2_DBG_MSG("cvc_send_create failed reserving constants");
        cvc_send_release_constants(op_data);
        return TRUE;
    }

    /* Now see if these tables are available yet */
    if (   !is_external_constant_available(cvclib_dataDynTable_Main,op_data->id)
        || !is_external_constant_available(aec510_DynamicMemDynTable_Main,op_data->id)
        || !is_external_constant_available(ASF100_DynamicMemDynTable_Main,op_data->id)
        || !is_external_constant_available(oms270_DynamicMemDynTable_Main,op_data->id)
        || !is_external_constant_available(filter_bank_DynamicMemDynTable_Main,op_data->id)
        || !is_external_constant_available(vad400_DynamicMemDynTable_Main,op_data->id)
        || !is_external_constant_available(op_extra_data->dyn_main,op_data->id))
    {
        /* Free the response created above, before it gets overwritten with the pending data */
        pdelete(*response_data);

        /* Database isn't available yet. Arrange for a callback 
         * Only need to check on one table */
        *response_id = (unsigned)op_extra_data->dyn_main;
        *response_data = (void*)(pending_operator_cb)cvc_send_create_pending_cb;

        L4_DBG_MSG("cvc_send_create - requesting callback when constants available");
        return (bool)HANDLER_INCOMPLETE;
    }
#endif

    op_extra_data->isWideband = op_extra_data->data_variant; 

    patch_fn_shared(cvc_send_wrapper);


    /*allocate the volume control shared memory*/
    op_extra_data->shared_volume_ptr = allocate_shared_volume_cntrl();
    if(!op_extra_data->shared_volume_ptr)
    {
        cvc_send_release_constants(op_data);
        return(TRUE);
    }

    /* call the "create" assembly function */
    if(CVC_SEND_CAP_Create(op_extra_data))
    {
        /* Free all the scratch memory we reserved */
        CVC_SEND_CAP_Destroy(op_extra_data);
        release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
        op_extra_data->shared_volume_ptr = NULL;
        cvc_send_release_constants(op_data);
        return(TRUE);
    }

    if(!cvc_send_register_component((void*)op_extra_data))
    {
        /* Free all the scratch memory we reserved, exit with fail response msg. Even if it had failed
         * silently, subsequent security checks will fail in lack of a successful registration.
         */
        CVC_SEND_CAP_Destroy(op_extra_data);
        release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
        op_extra_data->shared_volume_ptr = NULL;
        cvc_send_release_constants(op_data);
        return(TRUE);
    }


    if(!cpsInitParameters(&op_extra_data->parms_def,(unsigned*)CVC_SEND_GetDefaults(op_extra_data->cap_id),(unsigned*)op_extra_data->params,sizeof(CVC_SEND_PARAMETERS)))
    {
        /* Free all the scratch memory we reserved, exit with fail response msg. Even if it had failed
         * silently, subsequent security checks will fail in lack of a successful registration.
         */
        CVC_SEND_CAP_Destroy(op_extra_data);
        release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
        op_extra_data->shared_volume_ptr = NULL;
        cvc_send_release_constants(op_data);
        return(TRUE);
    }

	 /* Read state info from UCID 0 */
    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,0,OPMSG_P_STORE_STATE_VARIABLE_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_state_snd);

    /* operator state variable set to not running state. based on base_op.c */
    op_data->state = OP_NOT_RUNNING;

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}

bool cvc_send_ups_set_state(void* instance_data, PS_KEY_TYPE key, PERSISTENCE_RANK rank, STATUS_KYMERA status,
                                     uint16 extra_status_info)
{
    return TRUE;
}

bool cvc_send_destroy(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;

    /* Setup Response to Destroy Request.*/
    if(!base_op_destroy(op_data, message_data, response_id, response_data))
    {
        cvc_send_release_constants(op_data);
        return(FALSE);
    }

    /* calling the "destroy" assembly function - this frees up all the capability-internal memory */
    CVC_SEND_CAP_Destroy(op_extra_data);

    /*free volume control shared memory*/
    release_shared_volume_cntrl(op_extra_data->shared_volume_ptr);
    op_extra_data->shared_volume_ptr = NULL;

    base_op_change_response_status(response_data,STATUS_OK);

	/* Save state info to ucid 0 */
      {
          unsigned key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,0,OPMSG_P_STORE_STATE_VARIABLE_SUB_ID);
          uint16 state_data[2];

          state_data[0] = (op_extra_data->mdgc_gain>>16)&0xFFFF;
		    state_data[1] = op_extra_data->mdgc_gain&0xFFFF;
          ps_entry_write((void*)op_data,key,PERSIST_ANY,2,state_data,cvc_send_ups_set_state);
      }

    cvc_send_release_constants(op_data);

    return(TRUE);
}




bool cvc_send_check_all_connected(CVC_SEND_OP_DATA *op_data)
{
    if(op_data->output_stream->cbuffer == NULL)
    {
        return(FALSE);
    }

    /* AEC Reference*/
    if(op_data->input_stream[0]->cbuffer == NULL)
    {
        return(FALSE);
    }
    /* Input Chan 1 */
    if(op_data->input_stream[1]->cbuffer == NULL)
    {
        return(FALSE);
    }
    /* Input Chan 2 which only gets used whith more than 1 mic. */
    if((op_data->num_mics != 1) && (op_data->input_stream[2]->cbuffer == NULL))
    {
        return(FALSE);
    }
    if((op_data->num_mics > 2) && (op_data->input_stream[3]->cbuffer == NULL))
    {
        return(FALSE);
    }
    if((op_data->num_mics == 4) && (op_data->input_stream[4]->cbuffer == NULL))
    {
        return(FALSE);
    }
    return(TRUE);
}

bool cvc_send_connect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id   = ((unsigned*)message_data)[0];    /* extract the terminal_id */
    tCbuffer* pterminal_buf = (tCbuffer*)(uintptr_t)(((unsigned *)message_data)[1]);

    patch_fn_shared(cvc_send_wrapper);

    /* Setup Response to Connection Request.   Assume Failure*/
    *response_id = OPCMD_CONNECT;
    if((*response_data = MakeStandardResponse_SND(op_data->id))==NULL)
    {
        return(FALSE);
    }

    /* (i)  check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources .  */
    /* (ii) check if we are connecting to the right type . It has to be a buffer pointer and not endpoint connection */
    if( !base_op_is_terminal_valid(op_data, terminal_id) || !pterminal_buf)
    {
        base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* Allow Connect while running.  Disable processing until all are connected */
    op_extra_data->op_all_connected = FALSE;

    /* check if the terminal is already connected and if not , connect the terminal */
    /* set the terminal buffer to the Cbuffer pointer */
    /* set the "connected" field of the terminal to TRUE */
    /* connect the appropriate stream map */
    if(terminal_id & TERMINAL_SINK_MASK)
    {
        if (NULL == op_extra_data->input_stream[terminal_id&CVC_SEND_NUM_INPUTS_MASK]->cbuffer)
        {
            op_extra_data->input_stream[terminal_id&CVC_SEND_NUM_INPUTS_MASK]->cbuffer = pterminal_buf;
        }
        else
        {
            /* Streams should not have reached this point of sending us a
             * connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }
    else
    {
        if (NULL == op_extra_data->output_stream->cbuffer)
        {
            op_extra_data->output_stream->cbuffer = pterminal_buf;
        }
        else
        {
            /* Streams should not have reached this point of sending us a
             * connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }

    /* Allow Connect while running.  Re-enable processing alter all connections are completed */
    op_extra_data->op_all_connected = cvc_send_check_all_connected(op_extra_data);

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}



bool cvc_send_disconnect(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    unsigned terminal_id = *((unsigned*)message_data);

    /* Setup Response to Disconnection Request.   Assume Failure*/
    *response_id = OPCMD_DISCONNECT;
    if((*response_data = MakeStandardResponse_SND(op_data->id))==NULL)
    {
        return(FALSE);
    }

    /* check if the terminal ID is valid . The number has to be less than the maximum number of sinks or sources.  */
    if(!base_op_is_terminal_valid(op_data, terminal_id))
    {
        base_op_change_response_status(response_data,STATUS_INVALID_CMD_PARAMS);
        return TRUE;
    }

    /* Allow Disconnect while running */
    op_extra_data->op_all_connected = FALSE;

    /* check if the terminal is already disconnected and if not , disconnect the terminal */
    if(terminal_id & TERMINAL_SINK_MASK)
    {
        if (NULL != op_extra_data->input_stream[terminal_id&CVC_SEND_NUM_INPUTS_MASK]->cbuffer)
        {
            op_extra_data->input_stream[terminal_id&CVC_SEND_NUM_INPUTS_MASK]->cbuffer = NULL;
        }
        else
        {
            /* Streams should not have reached this point of sending us a
             * connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }
    else
    {
        if (NULL != op_extra_data->output_stream->cbuffer)
        {
            op_extra_data->output_stream->cbuffer = NULL;
        }
        else
        {
            /* Streams should not have reached this point of sending us a
             * connect for already existing connection */
            panic_diatribe(PANIC_AUDIO_ALREADY_CONNECTED_OPERATOR_TERMINAL, op_data->id);
        }
    }

    base_op_change_response_status(response_data,STATUS_OK);
    return TRUE;
}


bool cvc_send_buffer_details(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA   *opx_data;
    opx_data = (CVC_SEND_OP_DATA*)(op_data->extra_op_data);
    if (!base_op_buffer_details(op_data, message_data, response_id, response_data))
    {
        return FALSE;
    }
    /* No inplace support input samples may be dropped */
    ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size= opx_data->frame_size<<1;
    
    if((((unsigned*)message_data)[0])==TERMINAL_SINK_MASK)
    {
       /* Reference buffer needs more space */
       ((OP_BUF_DETAILS_RSP*)*response_data)->b.buffer_size += (opx_data->frame_size>>1);
    }

    return TRUE;
}

bool cvc_send_get_sched_info(OPERATOR_DATA *op_data, void *message_data, unsigned *response_id, void **response_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    OP_SCHED_INFO_RSP* resp;

    if (!base_op_get_sched_info(op_data, message_data, response_id, response_data))
    {
        return base_op_build_std_response(STATUS_CMD_FAILED, op_data->id, response_data);
    }

    /* Populate the response*/
    base_op_change_response_status(response_data, STATUS_OK);
    resp = *response_data;
    resp->op_id = op_data->id;
    /* Same buffer size for sink and source.
     * No additional verification needed.*/
    
    resp->block_size = op_extra_data->frame_size;
    resp->run_period = 0;

    *response_data = resp;
    return TRUE;
}





/* ************************************* Data processing-related functions and wrappers **********************************/

void cvc_send_process_data(OPERATOR_DATA *op_data, TOUCHED_TERMINALS *touched)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    int samples_to_process, stream_amount_data;
    int mic_index;
    int frame_size = op_extra_data->frame_size;

    patch_fn(cvc_send_process_data_patch);

    /* Bypass processing until all streams are connected */
    if(!op_extra_data->op_all_connected)
    {
       return;
    }

    /* number of samples to process at the reference */    
    samples_to_process = cbuffer_calc_amount_data_in_words(op_extra_data->input_stream[0]->cbuffer);   /* Reference */
		/* number of samples to process at the mics */  
    for(mic_index=1; mic_index <= op_extra_data->num_mics; mic_index++)
    {
        stream_amount_data = cbuffer_calc_amount_data_in_words(op_extra_data->input_stream[mic_index]->cbuffer);
        if (stream_amount_data < samples_to_process)
        {
            samples_to_process = stream_amount_data;
        }
    }

    /* Check for sufficient data and space */
    if(samples_to_process < frame_size)
    {
        return;
    }

    if (op_extra_data->Ovr_Control & CVC_SEND_CONTROL_MODE_OVERRIDE)
    {
        op_extra_data->Cur_mode = op_extra_data->Obpm_mode;
    }
    else if (op_extra_data->major_config!=CVC_SEND_CONFIG_HEADSET)
    {
        op_extra_data->Cur_mode = op_extra_data->Host_mode;
    }
    else
    {
        unsigned temp = op_extra_data->Cur_mode;
        if ((temp == CVC_SEND_SYSMODE_FULL) || (temp == CVC_SEND_SYSMODE_LOWVOLUME) )
        {
           /* TODO - need to redefine OFFSET_LVMODE_THRES to dB/60 */
           unsigned vol_level = 15 - (((int)op_extra_data->shared_volume_ptr->current_volume_level)/(-360));

           if (vol_level < op_extra_data->params->OFFSET_LVMODE_THRES)
           {
               op_extra_data->Cur_mode = CVC_SEND_SYSMODE_LOWVOLUME;
           }
           else
           {
               op_extra_data->Cur_mode = CVC_SEND_SYSMODE_FULL;
           }

           if (temp != op_extra_data->Cur_mode)
           {
               op_extra_data->ReInitFlag = 1;
           }
        }
        else
        {
           op_extra_data->Cur_mode = op_extra_data->Host_mode;
        }
    }

    /* call the "process" assembly function */
    CVC_SEND_CAP_Process(op_extra_data);

    /* touched output */
    touched->sources = TOUCHED_SOURCE_0;
}


/* **************************** Operator message handlers ******************************** */



bool cvc_send_opmsg_obpm_set_control(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{

    CVC_SEND_OP_DATA     *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    unsigned            i,num_controls,cntrl_value; 
    CPS_CONTROL_SOURCE  cntrl_src;
    unsigned result = OPMSG_RESULT_STATES_NORMAL_STATE;

    patch_fn(cvc_send_opmsg_obpm_set_control_patch);
    
    if(!cps_control_setup(message_data, resp_length, resp_data,&num_controls))
    {
       return FALSE;
    }

    for(i=0;i<num_controls;i++)
    {
        unsigned  cntrl_id=cps_control_get(message_data,i,&cntrl_value,&cntrl_src);
        /* Only interested in lower 8-bits of value */
        cntrl_value &= 0xFF;
        
        if (cntrl_id == OPMSG_CONTROL_MODE_ID)
        {
            /* Control is Mode */
            if (cntrl_value >= CVC_SEND_SYSMODE_MAX_MODES)
            {
                result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
                break;
            }
            
            if(cntrl_src == CPS_SOURCE_HOST)
            {
               op_extra_data->Host_mode = cntrl_value;
            }
            else
            {
                op_extra_data->Obpm_mode = cntrl_value;

                /* When the override bit in the control id is high, then we override the
                 * OBPM's ability to override the control value. In other words we let the control
                 * value be reset to the host's control value when the OPMSG_CONTROL_OBPM_OVERRIDE
                 * bit is high in the control id.*/
                if (cntrl_src == CPS_SOURCE_OBPM_DISABLE)
                {
                    op_extra_data->Ovr_Control &= ~CVC_SEND_CONTROL_MODE_OVERRIDE;
                }
                else
                {
                    op_extra_data->Ovr_Control |= CVC_SEND_CONTROL_MODE_OVERRIDE;
                }
            }
            op_extra_data->ReInitFlag = 1;
        }
        else if (cntrl_id == OPMSG_CONTROL_MUTE_ID)
        {
            if (cntrl_value > 1)
            {
                result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
                break;
            }
            /* Control is Mute */
            if(cntrl_src == CPS_SOURCE_HOST)
            {
               op_extra_data->host_mute = cntrl_value;
            }
            else
            {
                op_extra_data->obpm_mute = cntrl_value;

                /* When the override bit in the control id is high, then we override the
                 * OBPM's ability to override the control value. In other words we let the control
                 * value be reset to the host's control value when the OPMSG_CONTROL_OBPM_OVERRIDE
                 * bit is high in the control id.*/
                if(cntrl_src == CPS_SOURCE_OBPM_DISABLE)
                {
                    op_extra_data->Ovr_Control &= ~CVC_SEND_CONTROL_MUTE_OVERRIDE;
                }
                else
                {
                    op_extra_data->Ovr_Control |= CVC_SEND_CONTROL_MUTE_OVERRIDE;
                }
            }
            op_extra_data->Cur_Mute = ( op_extra_data->Ovr_Control & CVC_SEND_CONTROL_MUTE_OVERRIDE) ? op_extra_data->obpm_mute : op_extra_data->host_mute;
        }
        else if (cntrl_id == OPMSG_CONTROL_OMNI_ID)
        {
            if(op_extra_data->omni_mode_ptr == NULL)
            {
                result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
                break;
            }
            if (cntrl_value > 1)
            {
                result = OPMSG_RESULT_STATES_INVALID_CONTROL_VALUE;
                break;
            }
            /* Control is Mute */
            if(cntrl_src == CPS_SOURCE_HOST)
            {
                op_extra_data->host_omni = cntrl_value;
            }
            else
            {
                op_extra_data->obpm_omni = cntrl_value;

                /* When the override bit in the control id is high, then we override the
                 * OBPM's ability to override the control value. In other words we let the control
                 * value be reset to the host's control value when the OPMSG_CONTROL_OBPM_OVERRIDE
                 * bit is high in the control id.*/
                if(cntrl_src == CPS_SOURCE_OBPM_DISABLE)
                {
                    op_extra_data->Ovr_Control &= ~CVC_SEND_CONTROL_OMNI_OVERRIDE;
                }
                else
                {
                    op_extra_data->Ovr_Control |= CVC_SEND_CONTROL_OMNI_OVERRIDE;
                }
            }
            *(op_extra_data->omni_mode_ptr) = ( op_extra_data->Ovr_Control & CVC_SEND_CONTROL_OMNI_OVERRIDE) ? op_extra_data->obpm_omni : op_extra_data->host_omni;
        }
        else
        {
            result = OPMSG_RESULT_STATES_UNSUPPORTED_CONTROL;
            break;
        }
    }

    cps_response_set_result(resp_data,result);

    return TRUE;
}

bool cvc_send_opmsg_obpm_get_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;

    return cpsGetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool cvc_send_opmsg_obpm_get_defaults(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetDefaultsMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);
}

bool cvc_send_opmsg_obpm_set_params(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    bool retval;

    patch_fn(cvc_send_opmsg_obpm_set_params_patch);

    retval = cpsSetParameterMsgHandler(&op_extra_data->parms_def ,message_data, resp_length,resp_data);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return retval;
}

bool cvc_send_opmsg_obpm_get_status(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    unsigned  *resp;
    unsigned **stats = (unsigned**)op_extra_data->status_table;

    patch_fn(cvc_send_opmsg_obpm_get_status_patch);

    if(!common_obpm_status_helper(message_data,resp_length,resp_data,sizeof(CVC_SEND_STATISTICS),&resp))
    {
         return FALSE;
    }

    if(resp)
    {
        resp = cpsPackWords(&op_extra_data->Cur_mode,&op_extra_data->Ovr_Control ,resp);
        resp = cpsPackWords(stats[0], stats[1], resp);
        resp = cpsPackWords(stats[2], stats[3], resp);
        /* Reset IN/OUT Peak Detectors*/
        *(stats[1])=0;
        *(stats[2])=0;
        *(stats[3])=0;
        resp = cpsPackWords(stats[4],stats[5] , resp);
        resp = cpsPackWords(stats[6],stats[7] , resp);
        resp = cpsPackWords(stats[8],stats[9] , resp);
        resp = cpsPackWords(stats[10],stats[11] , resp);
        resp = cpsPackWords(op_extra_data->mute_control_ptr,stats[13], resp);
        resp = cpsPackWords(stats[14],stats[15] , resp);
        resp = cpsPackWords(stats[16],stats[17] , resp);
        cpsPackWords(stats[18],NULL , resp);
        /* Reset Peak Detectors AEC_REF/MIC3/MIC4 */
        *(stats[14])=0;
        *(stats[15])=0;
        *(stats[16])=0;
    }

    return TRUE;
}

bool ups_params_snd(void* instance_data,PS_KEY_TYPE key,PERSISTENCE_RANK rank,
                 uint16 length, unsigned* data, STATUS_KYMERA status,uint16 extra_status_info)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)((OPERATOR_DATA*)instance_data)->extra_op_data;

    cpsSetParameterFromPsStore(&op_extra_data->parms_def,length,data,status);

    /* Set the Reinit flag after setting the paramters */
    op_extra_data->ReInitFlag = 1;

    return(TRUE);
}

bool cvc_send_opmsg_set_ucid(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    PS_KEY_TYPE key;
    bool retval;

    retval = cpsSetUcidMsgHandler(&op_extra_data->parms_def,message_data,resp_length,resp_data);

    key = MAP_CAPID_UCID_SBID_TO_PSKEYID(op_extra_data->cap_id,op_extra_data->parms_def.ucid,OPMSG_P_STORE_PARAMETER_SUB_ID);
    ps_entry_read((void*)op_data,key,PERSIST_ANY,ups_params_snd);

    return retval;
}

bool cvc_send_opmsg_get_ps_id(OPERATOR_DATA *op_data, void *message_data, unsigned *resp_length, OP_OPMSG_RSP_PAYLOAD **resp_data)
{
    CVC_SEND_OP_DATA   *op_extra_data = (CVC_SEND_OP_DATA*)op_data->extra_op_data;
    
    return cpsGetUcidMsgHandler(&op_extra_data->parms_def,op_extra_data->cap_id,message_data,resp_length,resp_data);
}
