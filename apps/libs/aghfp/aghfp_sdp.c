/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_hsp_service_record.h"
#include "aghfp_hfp_service_record.h"
#include "aghfp_init.h"
#include "aghfp_sdp.h"
#include "aghfp_slc_handler.h"
#include "aghfp_wbs.h"

#include <panic.h>
#include <service.h>
#include <string.h>
#include <print.h>


/*
   aghfp->supported_features holds the bitmap as used for the BRSF message. This macro
   discards bits 6 - 8 and shifts the remaining bits down in their place
   to match the new format of the SupportedFeatures SDP record bitmap.
*/
#define BRSF_BITMAP_TO_SDP_BITMAP(a)    (((a >> 4) & ~0x1f) | (a & 0x1f))


/* Request the service handle(s) of the HSP service at the AG */
static const uint8 HspServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, UUID_HEADSET >> 8, UUID_HEADSET & 0xff /* 4 byte UUID */
};


/* Request the service handle(s) of the HFP service at the AG */
static const uint8 HfpServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...5 bytes in DataElSeq */
    0x1a, 0x00, 0x00, UUID_HANDSFREE >> 8, UUID_HANDSFREE & 0xff /* 4 byte UUID */
};


/* Request the service handle(s) of the AG service at the AG */
static const uint8 AGServiceRequest [] =
{
    0x35, /* type = DataElSeq */
    0x05, /* size ...3 bytes in DataElSeq */
    0x1a, 0x00, 0x00, UUID_HANDSFREE_AG >> 8, UUID_HANDSFREE_AG & 0xff /* 4 byte UUID */
};


/* Request the RFCOMM channel number of the AG's service */
static const uint8 protocolAttributeRequest [] =
{
    0x35, /* type = DataElSeq */
    0x06, /* size ...3 bytes in DataElSeq */
	0x09, 0x00, 0x04, /* 2 byte UINT attrID ProtocolDescriptorList */
	0x09, 0x00, 0x09 /* 2 byte UINT attrID BluetoothProfileDescriptorList */
};


/* Request the supported features of the HFP AG */
static const uint8 supportedFeaturesAttributeRequest [] =
{
    0x35,               /* 0b00110 101 type=DataElSeq */
    0x03,               /* size = 6 bytes in DataElSeq */
    0x09, 0x03, 0x11    /* 2 byte UINT attrID - SupportedFeatures */
};


/* Find the rfcomm server channel in a service record */
static bool findRfcommServerChannel(const uint8 *ptr, const uint8 *end, Region *value)
{
    ServiceDataType type;
    Region record, protocols, protocol;
    record.begin = ptr;
    record.end   = end;

	while(ServiceFindAttribute(&record, saProtocolDescriptorList, &type, &protocols))
	{
		if(type == sdtSequence)
		{
			while(ServiceGetValue(&protocols, &type, &protocol))
			{
				if(type == sdtSequence
					&& ServiceGetValue(&protocol, &type, value)
					&& type == sdtUUID
					&& RegionMatchesUUID32(value, (uint32) UUID_RFCOMM)
					&& ServiceGetValue(&protocol, &type, value)
					&& type == sdtUnsignedInteger)
				{
					/* Hah! found the server channel field */
					return 1;
				}
			}
		}
	}

    return 0; /* Failed */
}


/* Insert the rfcomm server channel into a service record */
static uint16 insertRfcommServerChannel(const uint8 *ptr, const uint8 *end, uint8 chan)
{
    Region value;

    if(findRfcommServerChannel(ptr, end, &value) && RegionSize(&value) == 1)
    {
		RegionWriteUnsigned(&value, (uint32) chan);
		return 1;
    }

    return 0;
}


/* Retrieve the rfcomm server channel */
static uint16 getRfcommChannelNumber(const uint8 *begin, const uint8 *end, uint16 *chan)
{
    Region value;

    if(findRfcommServerChannel(begin, end, &value))
    {
		*chan = (uint16) RegionReadUnsigned(&value);
		return 1;
    }

    return 0;
}


/* Find the supported features in a service record */
static bool findHfpSupportedFeatures(const uint8 *begin, const uint8 *end, Region *value)
{
    ServiceDataType type;
	Region record;
	record.begin = begin;
    record.end   = end;

	if (ServiceFindAttribute(&record, saSupportedFeatures, &type, value))
	{
		if(type == sdtUnsignedInteger)
		{
			return 1;
		}
	}

 	return 0;
}


/* Insert the supported features into the HFP service record */
static uint16 insertHfpSupportedFeatures(const uint8 *begin, const uint8 *end, uint8 features)
{
	Region value;

	if (findHfpSupportedFeatures(begin, end, &value) && RegionSize(&value) == 2)
	{
		RegionWriteUnsigned(&value, (uint32) features);
		return 1;
	}

	return 0;
}


/* Find the profile version in a service record */
static bool findHfpProfileVersion(const uint8 *begin, const uint8 *end, Region *value)
{   
    ServiceDataType type;
    Region record, profiles, profile;
    record.begin = begin;
    record.end   = end;

	while(ServiceFindAttribute(&record, saBluetoothProfileDescriptorList, &type, &profiles))
	{
		if(type == sdtSequence)
		{
			while(ServiceGetValue(&profiles, &type, &profile))
			{
				if(type == sdtSequence
					&& ServiceGetValue(&profile, &type, value)
					&& type == sdtUUID
					&& RegionMatchesUUID32(value, (uint32) UUID_HANDSFREE)
					&& ServiceGetValue(&profile, &type, value)
					&& type == sdtUnsignedInteger)
				{
                    AGHFP_DEBUG(("Found Profile version 0x%lx\n", RegionReadUnsigned(value)));
					/* Hah! found the profile version field */
					return 1;
				}
			}
		}
	}

    return 0; /* Failed */
}


/* Insert the profile version into the HFP service record */
static uint16 insertHfpProfile(const uint8 *begin, const uint8 *end, uint16 profile_version)
{
	Region value;

    AGHFP_DEBUG(("Insert Profile version 0x%x\n", profile_version));
    
	if (findHfpProfileVersion(begin, end, &value) && RegionSize(&value) == 2)
	{
		RegionWriteUnsigned(&value, (uint32) profile_version);
		return 1;
	}

	return 0;
}


#if 0
/* Get the supported features from the returned attribute list */
static uint16 getHfpAgSupportedFeatures(const uint8 *begin, const uint8 *end, uint16 *features)
{
 Region value;
    if(findHfpSupportedFeatures(begin, end, &value))
    {
  *features = (uint16) RegionReadUnsigned(&value);
  return 1;
    }
    return 0;
}
#endif


/* Service Search didn't return the required results in order to connect, so return failure to application */
static void aghfpServiceSearchCheckFailed(AGHFP *aghfp, sdp_search_status status)
{
    AGHFP_DEBUG(("SDP Fail 0x%x\n", status));
	/* If we have an established SLC the rfcomm sink will be valid. */
    if (!aghfp->rfcomm_sink)
	{
        aghfp_connect_status return_status = aghfp_connect_sdp_fail;
            
        /* try to get a more detailed reason for why the Service Search failed */
        if (status == l2cap_connect_failed)
            return_status = aghfp_connect_sdp_fail_no_connection;
        if (status == sdp_no_response_data)
            return_status = aghfp_connect_sdp_fail_no_data;            
            
		/* Tell the app the connection attempt has failed. */
		aghfpSendSlcConnectCfmToApp(return_status, aghfp);
	}
}


/****************************************************************************
 Register the service record corresponding to the specified profile
*/
void aghfpRegisterServiceRecord(AGHFP *aghfp, aghfp_profile profile, uint8 chan)
{
	uint16 length;
	uint8 *service_record = 0;


	if (supportedProfileIsHsp(profile))
	{
		/* Create a copy of the service record that we can modify */
		length = sizeof(aghfp_hsp_service_record);
		service_record = (uint8 *)PanicUnlessMalloc(length);
		memmove(service_record, aghfp_hsp_service_record, length);
	}
	else if (supportedProfileIsHfp(profile))
	{
         uint16 profile_version = 0x101;
        
        if (aghfp->supported_profile == aghfp_handsfree_15_profile)
            profile_version = 0x105;
        else if (aghfp->supported_profile == aghfp_handsfree_16_profile)
            profile_version = 0x106;
        else if (aghfp->supported_profile == aghfp_handsfree_17_profile)
            profile_version = 0x107;
            
		/* Create a copy of the service record that we can modify */
		length = sizeof(aghfp_hfp_service_record);
		service_record = (uint8 *)PanicUnlessMalloc(length);
		memmove(service_record, aghfp_hfp_service_record, length);

		/* Insert the supported features and profile version into the service record */
		if (!insertHfpSupportedFeatures(service_record, service_record + length, BRSF_BITMAP_TO_SDP_BITMAP(aghfp->supported_features)) ||
            !insertHfpProfile(service_record, service_record + length, profile_version))
		{
			/* Failed to insert the supported features into the service record */
			MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SDP_REGISTER_CFM);
			message->status = aghfp_fail;
			MessageSend(&aghfp->task, AGHFP_INTERNAL_SDP_REGISTER_CFM, message);

			/* Free the allocated memory */
			free(service_record);
			return;
		}
	}
	else
	{
		/* Unknown profile, send an error */
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SDP_REGISTER_CFM);
		message->status = aghfp_fail;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SDP_REGISTER_CFM, message);
		return;
	}

	if (!insertRfcommServerChannel(service_record, service_record + length, chan))
	{
		/* If we fail to insert the rfcomm channel return an error to the app */
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SDP_REGISTER_CFM);
		message->status = aghfp_fail;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_SDP_REGISTER_CFM, message);

		/* Free the allocated memory */
		free(service_record);
	}
	else
	{
		/* Send the service record to the connection lib to be registered with BlueStack */
		ConnectionRegisterServiceRecord(&aghfp->task, length, service_record);
	}
}


/****************************************************************************
	Outcome of SDP service register request.
*/
void aghfpHandleSdpRegisterCfm(AGHFP *aghfp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    /* Ignore pending status */
    if(cfm->status != sds_status_pending)
    {
        MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_SDP_REGISTER_CFM);

        /* Check the outcome of the register request */
        if (cfm->status == success)
        {
            /* Set status to success */
            message->status = aghfp_success;

            /* Store the service handle */
            aghfp->sdp_record_handle = cfm->service_handle;
        }
        else
        {
            /* Set status to fail */
            message->status = aghfp_fail;

            /* Reset the service handle */
            aghfp->sdp_record_handle = 0;
        }

        /* Send the message */
        MessageSend(&aghfp->task, AGHFP_INTERNAL_SDP_REGISTER_CFM, message);
    }
}


/****************************************************************************
 Outcome of SDP service unregister request.
*/
void aghfpHandleSdpUnregisterCfm(AGHFP *aghfp, const CL_SDP_UNREGISTER_CFM_T *cfm)
{
    if (cfm->status == success)
    {
        /* Unregister succeeded reset the service record handle */
        aghfp->sdp_record_handle = 0;
    }
}


/****************************************************************************
	Handle the outcome of the SDP register request if we're initialising the
	HFP profile lib.
*/
void aghfpHandleSdpInternalRegisterInit(AGHFP *aghfp, const AGHFP_INTERNAL_SDP_REGISTER_CFM_T *cfm)
{
	if (cfm->status == aghfp_success)
	{
		/* Service record register succeeded */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_success, 0);
	}
	else
	{
		/* Service record register failed - we don't get more detail than this! */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_sdp_reg_fail, 0);
	}
}


/****************************************************************************
 Handle the outcome of the SDP register request if we have initialised
 the profile lib and are registering a service record during the
 operation of the profile.
*/
void aghfpHandleSdpInternalRegisterCfm(const AGHFP_INTERNAL_SDP_REGISTER_CFM_T *cfm)
{
	AGHFP_DEBUG_ASSERT(cfm->status == aghfp_success, ("SDP register request failed\n"));
}


/****************************************************************************
 Initiate a service search to get the rfcomm server channel of the
 required service on the remote device. We need this before we can
 initiate a service level connection.
*/
void aghfpGetProfileServerChannel(AGHFP *aghfp, const bdaddr *addr)
{
	uint16 sp_len;
	uint8 *sp_ptr;

	sp_len = 0;
	sp_ptr = 0;

	/* Check which profile we support so we can device which search to use */
	if (supportedProfileIsHsp(aghfp->supported_profile))
	{
		/* This task supports the HSP */
		sp_ptr = (uint8 *) HspServiceRequest;
		sp_len = sizeof(HspServiceRequest);
	}
	else if (supportedProfileIsHfp(aghfp->supported_profile))
	{
		/* This task supports the HFP */
		sp_ptr = (uint8 *)HfpServiceRequest;
		sp_len = sizeof(HfpServiceRequest);
	}
	else
	{
		/* This should never happen */
		Panic();
	}

	/*  Issue the search request to the connection lib. The max number of attribute bytes
	is set to an arbitrary number, however the aim is to set it to a value so that
	if the remote end returns this many bytes we still have a block big enough to
	copy the data into.
	*/
	ConnectionSdpServiceSearchAttributeRequest(
		&aghfp->task, addr, 0x32,
        sp_len, sp_ptr,
        sizeof(protocolAttributeRequest),
        protocolAttributeRequest);
}


/****************************************************************************
 Service search has completed, check it has succeeded and get the required
 attributes from the returned list.
*/
void aghfpHandleServiceSearchAttributeCfm(AGHFP *aghfp, const CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *cfm)
{
	/* Check the outcome of the service search */
	if (cfm->status == sdp_response_success)
	{
		uint16 sdp_data = 0;
        
        if (getRfcommChannelNumber(cfm->attributes,
			cfm->attributes + cfm->size_attributes,
			&sdp_data))
		{        
            if (supportedProfileIsHfp(aghfp->supported_profile))
            {
                /* check this isn't an AG before trying to connect */
                aghfp->rfc_channel = sdp_data;
                ConnectionSdpServiceSearchRequest(&aghfp->task, &cfm->bd_addr, 0x32, sizeof(AGServiceRequest), AGServiceRequest);
            }
            else
            {
		    	/* We have an rfcomm channel we can proceed with the connection establishment */
		    	MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_RFCOMM_CONNECT_REQ);
		    	message->addr = cfm->bd_addr;
		    	message->rfc_channel = sdp_data;
		    	MessageSend(&aghfp->task, AGHFP_INTERNAL_RFCOMM_CONNECT_REQ, message);
		    }
        }
		else
		{
			/*  We have received data we don't know what to do with.
				This shouldn't happen since we're issuing the search and should
				know what we're looking for so for the moment just ignore.
			*/
		}
	}
	else
	{
        aghfpServiceSearchCheckFailed(aghfp, cfm->status);
	}
}


/****************************************************************************
 Service search has completed. This was to check if the remote device is an AG.
*/
void aghfpHandleServiceSearchCfm(AGHFP *aghfp, const CL_SDP_SERVICE_SEARCH_CFM_T *cfm)
{
	/* Check the outcome of the service search */
	if (cfm->status == sdp_response_success)
	{
        /* Remote device is an AG, don't connect */
        aghfpServiceSearchCheckFailed(aghfp, cfm->status);
    }
    else
    {
        /* Remote device isn't an AG, can connect now */
		MAKE_AGHFP_MESSAGE(AGHFP_INTERNAL_RFCOMM_CONNECT_REQ);
		message->addr = cfm->bd_addr;
		message->rfc_channel = aghfp->rfc_channel;
		MessageSend(&aghfp->task, AGHFP_INTERNAL_RFCOMM_CONNECT_REQ, message);
    }
}
