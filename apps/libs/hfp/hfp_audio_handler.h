/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_audio_handler.h
    
DESCRIPTION
    
*/

#ifndef HFP_AUDIO_HANDLER_H_
#define HFP_AUDIO_HANDLER_H_

/* Attempt S4 if supported by both HF and AG exchanged during SLC*/
bool isS4Supported(const hfp_link_data* link);

/* Work out the audio parameters to use */
const hfp_audio_params* hfpAudioGetParams(hfp_link_data* link, sync_pkt_type* packet_type, const hfp_audio_params* audio_params, bool disable_override);

/* Inform the app of the status of the audio (Synchronous) connection */
void sendAudioConnectCfmFailToApp(const hfp_link_data* link, hfp_audio_connect_status status);

/* Inform the app of the status of the audio (Synchronous) disconnection */
void sendAudioDisconnectIndToApp(const hfp_link_data* link, hfp_audio_disconnect_status status);

/****************************************************************************
NAME    
    hfpManageSyncDisconnect

DESCRIPTION
    Used to inform hfp of a synchronous (audio) disconnection.
    
RETURNS
    void
*/
void hfpManageSyncDisconnect(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleSyncConnectInd

DESCRIPTION
    Incoming Synchronous connect notification, accept if we recognise the sink 
    reject otherwise.

RETURNS
    void
*/
void hfpHandleSyncConnectInd(const CL_DM_SYNC_CONNECT_IND_T *ind);


/****************************************************************************
NAME    
    hfpHandleSyncConnectCfm

DESCRIPTION
    Confirmation in response to a Synchronous connect request indicating the 
    outcome of the connect attempt.

RETURNS
    void
*/
void hfpHandleSyncConnectCfm(const CL_DM_SYNC_CONNECT_CFM_T *cfm);


/****************************************************************************
NAME    
    hfpHandleAudioDisconnectReq

DESCRIPTION
    Attempt to disconnect the Synchronous connection.

RETURNS
    void
*/
void hfpHandleAudioDisconnectReq(hfp_link_data* link);


/****************************************************************************
NAME    
    hfpHandleSyncDisconnectInd

DESCRIPTION
    The Synchronous connection has been disconnected 

RETURNS
    void
*/
void hfpHandleSyncDisconnectInd(const CL_DM_SYNC_DISCONNECT_IND_T *ind);


/****************************************************************************
NAME    
    hfpHandleAudioConnectReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void hfpHandleAudioConnectReq(const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req);


/****************************************************************************
NAME    
    hfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
    void
*/
void hfpHandleAudioConnectRes(const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res);


/****************************************************************************
NAME    
    hfpHandleAudioTransferReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void hfpHandleAudioTransferReq(const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req);

#endif /* HFP_AUDIO_HANDLER_H_ */
