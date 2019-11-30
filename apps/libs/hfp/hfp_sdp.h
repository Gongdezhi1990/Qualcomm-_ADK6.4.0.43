/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_sdp.h
    
DESCRIPTION
    
*/

#ifndef HFP_SDP_H_
#define HFP_SDP_H_


/* Note that when WB-Speech was added a capability bit was added at:

   - Bit 5 in the SupportedFeatures of the SDP record
   - Bit 7 in the BRSF AT command

   Up until this point these bitmaps were identicle.

   hfp->hf_supported_features holds the bitmap as used for the BRSF message. This macro
   discards bits 6 and 7 and shifts the remaining bits down in their place
   to match the new format of the SupportedFeatures SDP record bitmap.
*/
#define BRSF_BITMAP_TO_SDP_BITMAP(a)    (((a >> 2) & ~0x1f) | (a & 0x1f))

#define HFP_1_5_VERSION_NUMBER    (0x0105)
#define HFP_1_7_VERSION_NUMBER    (0x0107)   /* As per Sec5.3 of HFP1.7 specification, 0x107 indicates HFP 1.6
                                               For a device to be HFP1.7 it should mandatory support S4 state */

/****************************************************************************
NAME    
    hfpRegisterServiceRecord

DESCRIPTION
    Register the service record corresponding to the specified service

RETURNS
    void
*/
void hfpRegisterServiceRecord(hfp_service_data* service);


/****************************************************************************
NAME    
    hfpUnregisterServiceRecord

DESCRIPTION
    Unregister the service record corresponding to the specified service

RETURNS
    void
*/
void hfpUnregisterServiceRecord(hfp_service_data* service);


/****************************************************************************
NAME    
    hfpHandleSdpRegisterCfm

DESCRIPTION
    Outcome of SDP service register request.

RETURNS
    void
*/
void hfpHandleSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    handleSdpUnregisterCfm

DESCRIPTION
    Outcome of SDP service unregister request.

RETURNS
    void
*/
void handleSdpUnregisterCfm(const CL_SDP_UNREGISTER_CFM_T *cfm);


/****************************************************************************
NAME    
    hfpGetProfileServerChannel

DESCRIPTION
    Initiate a service search to get the rfcomm server channel of the 
    required service on the remote device. We need this before we can 
    initiate a service level connection.

RETURNS
    void
*/
void hfpGetProfileServerChannel(hfp_link_data* link, hfp_service_data* service, const bdaddr *bd_addr);


/****************************************************************************
NAME    
    hfpHandleServiceSearchAttributeCfm

DESCRIPTION
    Service search has completed, check it has succeeded and get the required
    attrubutes from the returned list.

RETURNS
    void
*/
void hfpHandleServiceSearchAttributeCfm(const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);


/****************************************************************************
NAME    
    hfpGetAgSupportedFeatures

DESCRIPTION
    AG does not support BRSF command so we need to perform an SDP search
    to get its supported features.

RETURNS
    void
*/
void hfpGetAgSupportedFeatures(hfp_link_data* link);



#endif /* HFP_SDP_H_ */

