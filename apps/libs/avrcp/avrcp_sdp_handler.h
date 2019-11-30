/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_sdp_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SDP_HANDLER_H_
#define AVRCP_SDP_HANDLER_H_

#include "avrcp_init.h"
#include "avrcp_common.h"

/* Offset to access the protocol descriptor ID in protocolDescriptorList */
#define AVRCP_PDL_ID_OFFSET             2 

/* offset to access the PSM value in protocolDescriptorList */
#define AVRCP_PDL_PSM_OFFSET            12

/* Additional protocol list ID Value */
#define SDP_ADDITIONAL_PDL_ID           0x0D

/* Offset to access version in the protocol Descriptor list */
#define AVRCP_PROTOCOL_VERSION_OFFSET   20

/* Offset to access version in the profile Descriptor list */
#define AVRCP_PDL_VERSION_OFFSET        12

/* Offset to access the supported features */
#define AVRCP_SUPPORTED_FEATURES_OFFSET 18

/* Last Octet of Profile Version values used in Profile Descriptor List */
#define AVRCP_PDL_VERSION_10                    0x00
#define AVRCP_PDL_VERSION_13                    0x03
#define AVRCP_PDL_VERSION_14                    0x04
#define AVRCP_PDL_VERSION_15                    0x05
#define AVRCP_PDL_VERSION_16                    0x06

/* Both Octets of the Version Values */
#define AVRCP_PDL_FULL_VERSION_13               0x103
#define AVRCP_PDL_FULL_VERSION_14               0x104
#define AVRCP_PDL_FULL_VERSION_15               0x105
#define AVRCP_PDL_FULL_VERSION_16               0x106

/* Last Octet of Protocol version values used in Protocol Descriptor List */
#define AVRCP_PROTOCOL_VERSION_10           0x00
#define AVRCP_PROTOCOL_VERSION_12           0x02
#define AVRCP_PROTOCOL_VERSION_13           0x03
#define AVRCP_PROTOCOL_VERSION_14           0x04

/****************************************************************************
NAME    
    avrcpSendGetSupportedFeaturesCfm

DESCRIPTION
    Send GetSupportedFeatures confirmation message to the application task.
*/
void avrcpSendGetSupportedFeaturesCfm(AVRCP            *avrcp, 
                                      avrcp_status_code status, 
                                      uint16            features);

/****************************************************************************
NAME    
    avrcpSendGetExtensionsCfm

DESCRIPTION
    Send GetExtensions confirmation message to the application task.
*/
void avrcpSendGetExtensionsCfm(AVRCP             *avrcp, 
                               avrcp_status_code status, 
                               uint16            extensions);

/****************************************************************************
NAME    
    avrcpRegisterServiceRecord

DESCRIPTION
    Attempt to register a service record for the RCP service.
*/
void avrcpRegisterServiceRecord( AvrcpDeviceTask *avrcp);

/****************************************************************************
NAME    
    avrcpHandleSdpRegisterCfm

DESCRIPTION
    This function is called on receipt of an CL_SDP_REGISTER_CFM
    indicating the outcome of registering a service record.
*/
void avrcpHandleSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm);
                              

/*****************************************************************************
 NAME    
   avrcpSdpStateOnConnect
 
 DESCRIPTION
   This function is called on establishing the L2CAP connection 
   for AVRCP to notify the SDP state machine to set its state on 
   AVRCP connection. 
*/
void avrcpSdpStateOnConnect(AVRCP *avrcp);

/****************************************************************************
NAME    
    avrcpHandleServiceSearchAttributeCfm

DESCRIPTION
    Handle result of service/attribute search.
*/
void avrcpHandleServiceSearchAttributeCfm(AVRCP *avrcp, 
                             const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);

/*****************************************************************************
NAME    
  avrcpGetSupportedFeatures

DESCRIPTION
    Send SDP search to find profile version at remote end.
*/
void avrcpGetProfileVersion(AVRCP *avrcp);

/*****************************************************************************
NAME    
  avrcpGetSupportedFeatures

DESCRIPTION
   Send SDP search to find supported features at remote end.
*/
void avrcpGetSupportedFeatures(AVRCP *avrcp);

/*****************************************************************************
NAME    
  avrcpSdpSearchRequest

DESCRIPTION
  Send a SDP search request to get the profile version and supported features.
  
*/
void avrcpSdpSearchRequest(AVRCP           *avrcp, 
                           const bdaddr    *addr, 
                           avrcp_sdp_search mode);


#endif /* AVRCP_SDP_HANDLER_H_ */
