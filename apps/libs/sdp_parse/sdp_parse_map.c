/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_parse_map.c
    
DESCRIPTION
	Contains functions for accessing MAP specific fields in a service record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>

/************************************ Private *****************************/

/* Find MAP MAS instance id */

static bool findMapMasInstance(const uint8 size_service_record, 
                               uint8* service_record, Region* value)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;
	
	if (ServiceFindAttribute(&record, saMapMasInstanceId, &type, value))
		if(type == sdtUnsignedInteger)
		{
			/* Found the Attribute Field */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

static bool findMapMasMsgFeature(const uint8 size_service_record,
                                 uint8* service_record, Region* value)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;
	
	if (ServiceFindAttribute(&record, saMapMasMsgFeature, &type, value))
		if(type == sdtUnsignedInteger)
		{
			/* Found the Attribute Field */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

/************************************ Public ******************************/

/****************************************************************************
 * NAME
 *  SdpParseGetMapMasInstance
 * 
 * DESCRIPTION
 * API to access the MAS instance ID 
 *
 * PARAMETERS
 *  Refer sdp_parse.h
 *
 * RETURNS
 *  TRUE on success with instanceId contains the MASInstanceID.
 **************************************************************************/
bool SdpParseGetMapMasInstance( const uint8 size_service_record,
                                uint8* service_record, 
                                uint8* instance_id )
{
	Region value;
	if(findMapMasInstance(size_service_record, service_record, &value))
	{
		*instance_id = RegionReadUnsigned(&value);
		/* Accessed Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}

/****************************************************************************
 * NAME
 *  SdpParseGetMapMasInstance
 * 
 * DESCRIPTION
 * API to access the MAS support message feature. 
 *
 * PARAMETERS
 *  Refer sdp_parse.h
 *
 * RETURNS
 *  TRUE on success with MsgFeature contains the MASInstanceID.
 **************************************************************************/
bool SdpParseGetMapMasMsgFeature( const uint8 size_service_record,
                                  uint8* service_record, 
                                  uint8* msg_feature )
{
	Region value;
	if(findMapMasMsgFeature(size_service_record, service_record, &value))
	{
		*msg_feature = RegionReadUnsigned(&value);
		/* Accessed Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}
