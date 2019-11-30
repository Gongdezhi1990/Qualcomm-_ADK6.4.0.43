/****************************************************************************
Copyright (c) 2006 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_parse_arbitrary.c
    
DESCRIPTION
	Contains functions for accessing the service name in a service record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>
#include <panic.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE_SERVICE_NAME 32

/************************************ Private *****************************/

/* Find Service Name Attribute */

static bool findServiceName(const uint8 size_service_record, const uint8* service_record, ServiceAttributeId id, Region* value)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;
	
	if (ServiceFindAttribute(&record, id, &type, value))
		if(type == sdtTextString)
		{
			/* Found the Attribute Field */
			return TRUE;
		}
	/* Failed */
	return FALSE;
}

/************************************ Public ******************************/

/* Access Service Name Attribute */

bool SdpParseGetServiceName(const uint8 size_service_record, const uint8* service_record, uint8 size_service_name, char** service_name, uint8* length_service_name)
{
	Region value;
	uint8 k;
	
	/* if found service name field */
	if(findServiceName(size_service_record, service_record, saServiceName, &value))
	{	
		/* read in size of service name string */
		value.end = value.begin;
		value.begin--;
		
		/* Set length_service_name to no. of chars. in service name */
		*length_service_name = (uint8)RegionReadUnsigned(&value);
		
		/* if array has been allocated and field length is valid */
		if(*length_service_name && size_service_name)
		{			
			/* Ensure we dont try to use more memory than has been allocated */
			if(*length_service_name > size_service_name - 1)
				*length_service_name = size_service_name - 1;
			
			/* move to first string element */
			value.begin++;
			value.end = value.begin + 1;
			
			/* copy string element into array and move to next */
			for(k = 0; k < (*length_service_name); k++)
			{
				service_name[0][k] = RegionReadUnsigned(&value);
				value.end++;
			}
			
			/* set null char */
			(*length_service_name)++;
			service_name[0][k] = '\0';
						
			/* Accessed Successfully */
			return TRUE;
		}
	}
	
	/* set return string to "Failed" */
	*length_service_name = 0;
	*service_name = NULL;
	
	/* Failed */
	return FALSE;
}

/* Insert Service Name Attribute */

bool SdpParseInsertServiceName(const uint8 size_service_record, const uint8* service_record, char* service_name)
{
	Region value;
	uint8 size_service_name, size_old_service_name, k;
	
	size_service_name= strlen(service_name);
	
	if(size_service_name <= MAX_SIZE_SERVICE_NAME && size_service_name > 0)
	{
		/* if found service name field */
		if(findServiceName(size_service_record, service_record, saServiceName, &value))
		{	
			/* move to size service name value */
			value.end = value.begin;
			value.begin--;
			
			/* read in old value */
			size_old_service_name = RegionReadUnsigned(&value);
			
			if(size_service_name <= size_old_service_name)
			{
				/* move to first string element */
				value.begin++;
				value.end = value.begin + 1;
			
				/* copy string element into array and move to next */
				for(k = 0; k < size_service_name; k++)
				{
					RegionWriteUnsigned(&value, (uint32)service_name[k]);
					value.begin ++;
					value.end ++;
				}
								
				/* fill the rest with spaces */
				for(k = size_service_name; k < size_old_service_name; k++)
				{
					RegionWriteUnsigned(&value, (uint32)' ');
					value.begin++;
					value.end++;
				}
			
				return TRUE;
			}
		}
	}
	/* Failed */
	return FALSE;
}
