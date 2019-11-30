/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
*/

#ifndef __hid_control_handler_h
#define __hid_control_handler_h

void hidHandleControlPacket(HID *hid, Source source);
void hidHandleInternalControlRequest(HID *hid, const HID_INTERNAL_CONTROL_REQ_T *req);

#ifdef HID_DEVICE
void hidHandleInternalGetIdleResponse(HID *hid, const HID_INTERNAL_GET_IDLE_RES_T *res);
void hidHandleInternalSetIdleResponse(HID *hid, const HID_INTERNAL_SET_IDLE_RES_T *res);
void hidHandleInternalGetProtocolResponse(HID *hid, const HID_INTERNAL_GET_PROTOCOL_RES_T *res);
void hidHandleInternalSetProtocolResponse(HID *hid, const HID_INTERNAL_SET_PROTOCOL_RES_T *res);
void hidHandleInternalGetReportResponse(HID *hid, const HID_INTERNAL_GET_REPORT_RES_T *res);
void hidHandleInternalSetReportResponse(HID *hid, const HID_INTERNAL_SET_REPORT_RES_T *res);
#endif

void hidHandleInternalRequestTimeout(HID *hid);
void hidRequestFailed(HID *hid, int request, hid_status status);

#endif
