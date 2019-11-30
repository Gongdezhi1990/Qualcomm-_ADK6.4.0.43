/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_SDP_H_
#define AGHFP_SDP_H_


/****************************************************************************
	Register the service record corresponding to the specified profile 
*/
void aghfpRegisterServiceRecord(AGHFP *aghfp, aghfp_profile profile, uint8 chan);


/****************************************************************************
	Outcome of SDP service register request.
*/
void aghfpHandleSdpRegisterCfm(AGHFP *aghfp, const CL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
	Handle the outcome of the SDP register request if we're initialising the
	HFP profile lib.
*/
void aghfpHandleSdpInternalRegisterInit(AGHFP *aghfp, const AGHFP_INTERNAL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
	Handle the outcome of the SDP register request if we have initialised
	the profile lib.
*/
void aghfpHandleSdpInternalRegisterCfm(const AGHFP_INTERNAL_SDP_REGISTER_CFM_T *cfm);


/****************************************************************************
	Outcome of SDP service unregister request.
*/
void aghfpHandleSdpUnregisterCfm(AGHFP *aghfp, const CL_SDP_UNREGISTER_CFM_T *cfm);


/****************************************************************************
	Initiate a service search to get the rfcomm server channel of the 
	required service on the remote device. We need this before we can 
	initiate a service level connection.
*/
void aghfpGetProfileServerChannel(AGHFP *aghfp, const bdaddr *addr);


/****************************************************************************
	Service search has completed, check it has succeeded and get the required
	attributes from the returned list.
*/
void aghfpHandleServiceSearchAttributeCfm(AGHFP *aghfp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm);


/****************************************************************************
	Service search has completed. This was to check if the remote device is an AG.
*/
void aghfpHandleServiceSearchCfm(AGHFP *aghfp, const CL_SDP_SERVICE_SEARCH_CFM_T *cfm);


/****************************************************************************
	AG does not support BRSF command so we need to perform an SDP search
	to get its supported features.
*/
void aghfpGetAgSupportedFeatures(AGHFP *aghfp);


#endif /* AGHFP_SDP_H_ */
