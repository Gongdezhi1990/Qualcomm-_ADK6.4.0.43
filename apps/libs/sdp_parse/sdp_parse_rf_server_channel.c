/****************************************************************************
Copyright (c) 2006 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_parse_rf_server_channel.c

DESCRIPTION
	Contains functions for accessing the rfcomm server channel field in a service
	record
*/

#include "sdp_parse.h"
#include <service.h>
#include <region.h>
#include <panic.h>
#include <string.h>
#include <stdlib.h>

/************************************ Private *****************************/

/* Find PDL */

static bool findProtocolDescriptorList(const uint8 size_service_record, const uint8* service_record, Region* protocols)
{
	ServiceDataType type;
    Region record;
    record.begin = service_record;
    record.end   = service_record + size_service_record;

	/* Move protocols to Protocol Descriptor List */
    while(ServiceFindAttribute(&record, saProtocolDescriptorList, &type, protocols))
		if(type == sdtSequence)
			/* Success */
			return TRUE;
	/* Failed */
	return FALSE;
}

/* Find Server Channel */

static bool findServerChannel(Region* protocols, uint32 channel_id, Region* value)
{
	Region protocol;
	ServiceDataType type;

	/* Move protocol through pdl sections */
	while(ServiceGetValue(protocols, &type, &protocol))
	{
		/* Move to first section element (if no element - move to next section) */
 		if(ServiceGetValue(&protocol, &type, value))
		{
			/* Move value through section elements until channel id found */
			while(!RegionMatchesUUID32(value, channel_id))
			{
				/* If no more values to read in - next section */
				if(!ServiceGetValue(&protocol, &type, value))
				{
					break;
				}
			}
			/* Check we broke due to finding the channel id, not reaching the end of the pdl section */
			if(value->end != protocol.end)
			{
				/* Assume profile version follows channel id, so read + return */
				if(ServiceGetValue(&protocol, &type, value) && (type == sdtUnsignedInteger))
		      	  	return TRUE;
			}
			else
			{
				/* If we did reach the end of the pdl section, move on */
			}
		}
	}

	/* Failed */
    return FALSE;
}

/************************************ Public ******************************/

/* Insert RFCOMM Server Channel */

bool SdpParseInsertRfcommServerChannel(const uint8 size_service_record, const uint8* service_record, uint8 chan)
{
	Region protocols,value;
    if(findProtocolDescriptorList(size_service_record, service_record, &protocols))
    	if(findServerChannel(&protocols, (uint32) UUID_RFCOMM, &value))
		{
			RegionWriteUnsigned(&value, (uint32) chan);
			/* Inserted Successfully */
			return TRUE;
    	}
	/* Failed */
    return FALSE;
}

/* Access All RFCOMM Server Channels */

bool SdpParseGetMultipleRfcommServerChannels(const uint8 size_service_record, const uint8* service_record, uint8 size_chans, uint8** chans, uint8* chans_found)
{
	Region protocols, value;
	*chans_found = 0;

	if(findProtocolDescriptorList(size_service_record, service_record, &protocols))
		while(findServerChannel(&protocols, (uint32) UUID_RFCOMM, &value))
		{
			/* reset protocols.begin so next search starts from after the channel we just found */
			protocols.begin = value.end;

			/* if space in the array then read in value, else break */
			if((*chans_found) < size_chans)
			{
				/* Read in found channel */
				chans[0][*chans_found] = (uint8)RegionReadUnsigned(&value);
				(*chans_found)++;
			}
			else
			{
				break;
			}
    	}

	/* If we found any rfcomm channels */
	if(*chans_found)
	{
		/* Accessed Successfully */
		return TRUE;
	}
	else
	{
		/* Failed */
    	return FALSE;
	}
}
