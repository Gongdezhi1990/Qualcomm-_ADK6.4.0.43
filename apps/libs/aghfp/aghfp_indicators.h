/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef AGHFP_INDICATORS_H_
#define AGHFP_INDICATORS_H_


#include "aghfp.h"


void aghfpHandleSendCindDetails(AGHFP *aghfp);
void aghfpHandleSendServiceIndicator(AGHFP *aghfp, aghfp_service_availability);
void aghfpSendCallIndicator(AGHFP *aghfp, aghfp_call_status status);
void aghfpHandleSendCallIndicator(AGHFP *aghfp, aghfp_call_status);
void aghfpSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status);
void aghfpHandleSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status);
void aghfpHandleSendCallHeldIndicator(AGHFP *aghfp, aghfp_call_held_status status);
void aghfpHandleSendSignalIndicator(AGHFP *aghfp, uint16 level);
void aghfpHandleSendRoamIndicator(AGHFP *aghfp, aghfp_roam_status status);
void aghfpHandleSendBattChgIndicator(AGHFP *aghfp, uint16 level);
void aghfpHandleCallerIdSetupReq(AGHFP *aghfp, bool enable);
void aghfpHandleCallWaitingSetupReq(AGHFP *aghfp, bool enable);
void aghfpHandleSendCallWaitingNotification(AGHFP *aghfp, uint8 type, uint16 size_number, uint8 *number, uint16 size_string, uint8 *string);
void aghfpHandleSetServiceState(AGHFP *aghfp, bool service_state);
void aghfpHandleIndicatorsActivationRequest(AGHFP * aghfp, AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ_T* req);


#endif /* AGHFP_INDICATORS_H_ */
