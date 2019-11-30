/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_link.h
DESCRIPTION
    Header file for link information and incoming data from remote device.
*/
#ifndef __IAP2_LINK_H__
#define __IAP2_LINK_H__

#include "iap2_private.h"

iap2_link *iap2LinkAllocate(iap2_transport_t transport);
void iap2LinkDeallocate(iap2_link *link);
bool iap2LinkValidate(iap2_link *link);
void iap2HandleInternalLinkEakInd(iap2_link *link, const IAP2_INTERNAL_LINK_EAK_IND_T *ind);
void iap2HandleInternalLinkDetachReq(iap2_link *link);
void iap2HandleMessageMoreData(iap2_link *link, MessageMoreData *message);
void iap2RemoveLinkFromSessionAndMessage(iap2_link *link);


#endif /* __IAP2_LINK_H__ */
