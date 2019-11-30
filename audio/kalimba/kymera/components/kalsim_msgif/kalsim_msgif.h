/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup kalsim_msgif kalsim message interface
 * \file kalsim_msgif.h
 *
 * \section kalsim_msgif_h_usage USAGE
 * This is the public header file for the kalsim_msgif module.
 */

 /****************************************************************************
 Include Files
 */
#ifndef KALSIM_MSGIF_H
#define KALSIM_MSGIF_H

#include "accmd/accmd.h"
#include "hydra_types.h"
#include "types.h"

/****************************************************************************
Public Type Declarations
*/
typedef enum pipe_identifier
{
    KCMD_ACCMD_PIPE = 0,
    KCMD_GEN_PURPOSE_PIPE,
    KCMD_FREE_PIPE0,
    KCMD_FREE_PIPE1,
    KCMD_FREE_PIPE2,
} pipe_identifier;

typedef enum service_msg_id
{
    START_SERVICE_REQ = 0x2001,
    STOP_SERVICE_REQ = 0x2002,
    SET_SCO_PARAMS_REQ = 0x2003,
    KICK_SERVICE_CONSUMER = 0x2004,
    PANIC_REPORT = 0x2006,
    FAULT_REPORT = 0x2007,
    AUDIO_DATA_SERVICE_AUX_MSG = 0x2008,
    SCO_PROC_SERVICE_AUX_MSG = 0x2009,
    SSSM_SERVICE_ADVICE_IND = 0x200A,
    FTP_SERV_TO_FW_CTRL = 0x200B,
    FTP_SERV_TO_FW_DATA = 0x200C,
    START_SERVICE_RESP = 0x6001,
    STOP_SERVICE_RESP = 0x6002,
    SET_SCO_PARAMS_RESP = 0x6003,
    FTP_FW_TO_SERV_CTRL = 0x6004,
    FTP_FW_TO_SERV_DATA = 0x6005,
    KALSIM_CCP_SIGNAL_ID_OPERATIONAL_IND = 0x6006
} service_msg_id;

typedef enum data_service_aux_msgs
{
    CAP_DOWNLOAD_KCS_ID = 0x0,
    CAP_DOWNLOAD_ERROR = 0x1,
    EOF_PROCESSED = 0x2
} data_service_aux_msgs;

typedef struct kick_consumer_msg_t
{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    data_buf_amt_data;
    uint16    data_buf_amt_space;
    uint16    meta_buf_amt_data;
    uint16    meta_buf_amt_space;
} kick_consumer_msg_t;

typedef struct aux_msg_t
{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    aux_msg_code;
    uint16    param;
} aux_msg_t;

typedef struct aux_msg_sco_t
{
    uint16    msg_id;
    uint16    serv_tag;
    uint16    direction;
    uint16    param;
} aux_msg_sco_t;

#define AUDIO_DATA_SERV_AUX_MSG_LEN     sizeof(aux_msg_t)/sizeof(uint16)
#define SCO_PROC_SERV_AUX_MSG_LEN       sizeof(aux_msg_sco_t)/sizeof(uint16)
#define KICK_CONSUMER_MSG_LEN           sizeof(kick_consumer_msg_t)/sizeof(uint16)

#define SEND_PIPE_SIZE_OCTETS  256
#define RECV_PIPE_SIZE_OCTETS  256
/****************************************************************************
Public Function Definitions
*/
extern void kalcmd_configure_communication(void);
extern void kalcmd_send_message(pipe_identifier id, uint16 *payload, unsigned len);
extern bool kalsim_ccp_send_pdu_operational_ind(unsigned patch_level);
#endif /* KALSIM_MSGIF_H */
