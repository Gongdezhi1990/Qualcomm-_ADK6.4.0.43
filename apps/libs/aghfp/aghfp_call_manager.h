/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef AGHFP_CALL_MANAGER_H
#define AGHFP_CALL_MANAGER_H


#include "aghfp.h"


aghfp_call_flags aghfpConvertDisconnectStatusToCallFlag (aghfp_disconnect_status status);
bool aghfpCallManagerActive (AGHFP *aghfp);
bool aghfpCallManagerActiveNotComplete (AGHFP *aghfp);
void aghfpManageCall (AGHFP *aghfp, aghfp_call_event call_event, uint16 event_value);
void aghfpResetCallerIdDetails (AGHFP *aghfp);
void aghfpHandleSetCallerIdDetails (AGHFP *aghfp, AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ_T *req);
void aghfpHandleCallCreateAudio (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ_T *req);
void aghfpHandleCallCreate (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_CREATE_REQ_T *req);
void aghfpHandleCallAnswer (AGHFP *aghfp);
void aghfpHandleCallRemoteAnswered (AGHFP *aghfp);
void aghfpHandleCallTerminate (AGHFP *aghfp, AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ_T *req);
void aghfpHandleHeldCallTerminate(AGHFP *aghfp);
void aghfpHandleHeldCallAccept(AGHFP *aghfp);
void aghfpSendCallCreateCfmToApp (AGHFP *aghfp, aghfp_call_create_status status);

#endif /* AGHFP_CALL_MANAGER_H */
