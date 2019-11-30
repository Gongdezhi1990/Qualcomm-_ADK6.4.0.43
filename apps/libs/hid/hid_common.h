/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_common.h    
DESCRIPTION
	Header file for the common HID functions.
*/

#ifndef HID_COMMON_H_
#define HID_COMMON_H_

#include "hid.h"
#include "hid_private.h"

HID *hidCreate(HID_LIB *hid_lib, const bdaddr *addr);
HID *hidFindFromBddr(HID_LIB *hid_lib, const bdaddr *addr);
HID *hidFindFromSink(HID_LIB *hid_lib, Sink sink);
HID *hidFindFromConnectionId(HID_LIB *hid_lib, uint16 con_id);
void hidDestroy(HID *hid);

void hidSetState(HID *hid, hidState state);
hidState hidGetState(HID *hid);

bool hidConnIsConnected(HID *hid, int psm);
bool hidConnIsDisconnected(HID *hid, int psm);
void hidConnConnecting(HID *hid, int psm, uint16 con_id, uint8 identifier);
void hidConnConnected(HID *hid, int psm, Sink sink, uint16 mtu);
void hidConnDisconnect(HID *hid, int psm);
void hidConnDisconnected(HID *hid, Sink sink);
void hidConnDisconnectedPsm(HID *hid, int psm);

#endif /* HID_COMMON_H_ */

