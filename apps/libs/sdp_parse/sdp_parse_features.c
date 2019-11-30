/****************************************************************************
Copyright (c) 2006 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_parse_features.c
    
DESCRIPTION
	Contains functions for accessing the supported features field in a service 
	record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>

/************************************ Private *****************************/

/* Find Supported Features */

static bool findSupportedFeatures(const uint8 length, const uint8* begin, Region* value)
{
	ServiceDataType type;
	Region record;
	record.begin = begin;
    record.end   = begin + length;

	if (ServiceFindAttribute(&record, saSupportedFeatures, &type, value))
		if(type == sdtUnsignedInteger)
		{
			/* Found the Supported Features */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

/************************************ Public ******************************/

/* Access Supported Features */

bool SdpParseGetSupportedFeatures(const uint8 size_service_record, const uint8* service_record, uint16* features)
{
	Region value;
    if(findSupportedFeatures(size_service_record, service_record, &value))
    {
		*features = (uint16) RegionReadUnsigned(&value);
		/* Accessed Successfully */
		return TRUE;
    }
	/* Failed */
    return FALSE;
}

/* Insert Supported Features */

bool SdpParseInsertSupportedFeatures(const uint8 size_service_record, const uint8* service_record, uint16 features)
{
	Region value;

	if (findSupportedFeatures(size_service_record, service_record, &value) && RegionSize(&value) == 2)
	{
		RegionWriteUnsigned(&value, (uint32) features);
		/* Inserted Successfully */
		return TRUE;
	}
	/* Failed */
	return FALSE;
}
