/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/

#ifndef AGHFP_CALL_H_
#define AGHFP_CALL_H_


#include "aghfp.h"


void aghfpSendRingAlert(AGHFP *aghfp);
void aghfpHandleSendRingAlert(AGHFP *aghfp);
void aghfpSendCallerId(AGHFP *aghfp, uint8 type, uint16 size_number, uint16 size_string, uint8 *data);
void aghfpHandleSendCallerId(AGHFP *aghfp, uint8 type, uint16 size_number, uint16 size_string, uint8 *data);
void aghfpHandleAnswer(AGHFP *aghfp);											/* ATA */
void aghfpSendInBandRingToneEnable(AGHFP *aghfp, bool enable);
void aghfpHandleInBandRingToneEnable(AGHFP *aghfp, bool enable);
void aghfpHandleCallHangUpReq(AGHFP *aghfp);									/* AT+CHUP */
void aghfpHandleDialReq(AGHFP *aghfp, uint8 *number, uint16 number_len);
void aghfpHandleMemoryDialReq(AGHFP *aghfp, uint8 *number, uint16 number_len);
void aghfpHandleLastNumberRedialReq(AGHFP *aghfp);								/* AT+BLDN */
void aghfpHandleCallHoldReq(AGHFP *aghfp, uint16 action, uint16 index);	        /* AT+CHLD=d */
void aghfpSendInBandRingTone(AGHFP *aghfp);


#endif /* AGHFP_CALL_H_ */
