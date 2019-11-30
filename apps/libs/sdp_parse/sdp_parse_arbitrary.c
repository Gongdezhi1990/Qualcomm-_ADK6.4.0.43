/****************************************************************************
Copyright (c) 2006 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_parse_arbitrary.c
    
DESCRIPTION
	Contains functions for accessing user specified fields in a service record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>

/************************************ Private *****************************/

/* Find Arbitrary Attribute */

static bool findArbitrary(const uint8 size_service_record, const uint8* service_record, ServiceAttributeId id, Region* value)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;
	
	if (ServiceFindAttribute(&record, id, &type, value))
		if(type == sdtUnsignedInteger)
		{
			/* Found the Attribute Field */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

/************************************ Public ******************************/

/* Access Arbitrary Attribute */

bool SdpParseGetArbitrary(const uint8 size_service_record, const uint8* service_record, ServiceAttributeId id, uint32* val)
{
	Region value;
	if(findArbitrary(size_service_record, service_record, id, &value))
	{
		*val = RegionReadUnsigned(&value);
		/* Accessed Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}

/* Insert Arbitrary Attribute */

bool SdpParseInsertArbitrary(const uint8 size_service_record, const uint8* service_record, ServiceAttributeId id, uint32 val)
{
	Region value;
	if(findArbitrary(size_service_record, service_record, id, &value))
	{
		RegionWriteUnsigned(&value, val);
		/* Inserted Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}
