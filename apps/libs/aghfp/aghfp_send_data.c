/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_send_data.h"
#include "aghfp_common.h"

#include <panic.h>
#include <stdarg.h>
#include <string.h>
#include <util.h>
#include <sink.h>
#include <stream.h>

#ifdef DISPLAY_AT_CMDS
 #include <stdio.h>
#endif


/****************************************************************************
 Send a simple (ie one const char *) AT command by putting it into the RFCOMM
 buffer.
*/
void aghfpSendAtCmd(AGHFP *aghfp, const char *at_cmd)
{
	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
		/* Make sure we aren't already in the middle of a AtCmdBegin section */
	    if (aghfp->mapped_rfcomm_sink == NULL)
	    {
			bool flushSuccess;
			uint8 *dest;
	        uint16 length = strlen(at_cmd);
	
		    uint16 offset = SinkClaim(aghfp->rfcomm_sink, length + 4);
	        AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpSendAtCmd: Invalid sink offset length = %d\n", length));
	
			dest = SinkMap(aghfp->rfcomm_sink);
			AGHFP_DEBUG_ASSERT(dest != NULL, ("aghfpSendAtCmd: Sink map failed\n"));
	
			dest += offset;
			memmove(dest, "\r\n", 2);
			dest += 2;
			memmove(dest, at_cmd, length);
			dest += length;
			memmove(dest, "\r\n", 2);
	
			flushSuccess = SinkFlush(aghfp->rfcomm_sink, length + 4);
			AGHFP_DEBUG_ASSERT(flushSuccess, ("aghfpSendAtCmd: Sink flush failed\n"));
	
#ifdef DISPLAY_AT_CMDS
			{
				const char *c = at_cmd;
				printf("\nsending: ");
				while (*c != '\0')
				{
					if (*c == '\r') 		printf("\\r");
					else if (*c == '\n') 	printf("\\n");
					else					putchar(*c);
					++c;
				}
			}
#endif
	    }
	    else
	    {
	        AGHFP_DEBUG_PANIC(("aghfpSendAtCmd: Already in AtCmdBegin section\n"));
	    }
	}
}


/******************************************************************************
 Send a single string of user data, WITH NO ADDITIONAL FORMATTING. This allows
 the sending of malformed instructions, for testing purposes.
*/
void aghfpSendRawData(AGHFP *aghfp, uint16 length, const uint8 *data)
{
	/* Make sure we aren't already in the middle of a AtCmdBegin section */
	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
	    if (aghfp->mapped_rfcomm_sink == NULL)
	    {
			uint8 *dest;
		    uint16 offset = SinkClaim(aghfp->rfcomm_sink, length);
			AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpSendRawData: Invalid sink offset\n"));
	
			dest = SinkMap(aghfp->rfcomm_sink) + offset;
	        AGHFP_DEBUG_ASSERT(dest != NULL, ("aghfpSendRawData: Invalid sink map\n"));
	
			memmove(dest, data, length);
	
			if (!SinkFlush(aghfp->rfcomm_sink, length))
			{
				AGHFP_DEBUG_PANIC(("aghfpSendRawData: Invalid sink flush\n"));
			}

#ifdef DISPLAY_AT_CMDS
			{
				uint16 i = 0;
				printf("\nsending: ");
				for (i = 0; i < length; ++i)
				{
					char c = data[i];
					if (c == '\r') 			printf("\\r");
					else if (c == '\n') 	printf("\\n");
					else					putchar(c);
				}
			}
#endif
	    }
	    else
	    {
	        AGHFP_DEBUG_PANIC(("aghfpSendAtCmd: Already in AtCmdBegin section\n"));
	    }
	}
}


/****************************************************************************/
void aghfpAtCmdBegin(AGHFP *aghfp)
{
	uint16 offset;
	uint8 *dest;

	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
		/* Make sure we aren't already in the middle of a begin/end section */
	    if (aghfp->mapped_rfcomm_sink != NULL)
	    {
			AGHFP_DEBUG_PANIC(("aghfpAtCmdBegin: Already in AtCmdBegin section\n"));
		}
	
		aghfp->mapped_rfcomm_sink = SinkMap(aghfp->rfcomm_sink);
		AGHFP_DEBUG_ASSERT(aghfp->mapped_rfcomm_sink != NULL, ("aghfpAtCmdBegin: Invalid sink map\n"));
	
		offset = SinkClaim(aghfp->rfcomm_sink, 2);
		AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpAtCmdBegin: Invalid sink offset\n"));
	
		dest = aghfp->mapped_rfcomm_sink + offset;
		memmove(dest, "\r\n", 2);

#ifdef DISPLAY_AT_CMDS
		printf("\nsending: \\r\\n");
#endif
	}
}


/****************************************************************************/
void aghfpAtCmdString(AGHFP *aghfp, const char *string)
{
	uint16 offset;
    uint8 *dest;
    uint16 length = strlen(string);

	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
		/* Make sure aghfpAtCmdBegin has been called */
	    AGHFP_DEBUG_ASSERT(aghfp->mapped_rfcomm_sink != NULL, ("aghfpAtCmdString: aghfpAtCmdBegin not called\n"));
	
		offset = SinkClaim(aghfp->rfcomm_sink, length);
		AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpAtCmdString: Invalid sink offset\n"));
	
		dest = aghfp->mapped_rfcomm_sink + offset;
		memmove(dest, string, length);

#ifdef DISPLAY_AT_CMDS
		{
			const char *c = string;
			while (*c != '\0')
			{
				if (*c == '\r') 		printf("\\r");
				else if (*c == '\n') 	printf("\\n");
				else					putchar(*c);
				++c;
			}
		}
#endif
	}
}


/****************************************************************************/
void aghfpAtCmdData(AGHFP *aghfp, const uint8 *data, uint16 len)
{
	uint16 offset;
    uint8 *dest;

	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
		/* Make sure aghfpAtCmdBegin has been called */
	    AGHFP_DEBUG_ASSERT(aghfp->rfcomm_sink != NULL, ("aghfpAtCmdData: aghfpAtCmdBegin not called\n"));
	
	    offset = SinkClaim(aghfp->rfcomm_sink, len);
		AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpAtCmdData: Invalid sink offset\n"));
	
	    dest = aghfp->mapped_rfcomm_sink + offset;
	    memmove(dest, data, len);

#ifdef DISPLAY_AT_CMDS
		{
			uint16 i;
			for (i = 0; i < len; ++i)
			{
				putchar('X');
			}
		}
#endif
	}
}


/****************************************************************************/
void aghfpAtCmdEnd(AGHFP *aghfp)
{
	uint16 offset;
    uint8 *dest;

	if ( SinkIsValid(aghfp->rfcomm_sink) )
	{
		/* Make sure aghfpAtCmdBegin has been called */
	    if (aghfp->mapped_rfcomm_sink == NULL)
	    {
			AGHFP_DEBUG_PANIC(("aghfpAtCmdEnd: aghfpAtCmdBegin not called\n"));
		}
	
		offset = SinkClaim(aghfp->rfcomm_sink, 2);
		AGHFP_DEBUG_ASSERT(offset != 0xFFFF, ("aghfpAtCmdEnd: Invalid sink offset\n"));
	
		dest = aghfp->mapped_rfcomm_sink + offset;
		memmove(dest, "\r\n", 2);
	
		if (!SinkFlush(aghfp->rfcomm_sink, offset + 2))
		{
			AGHFP_DEBUG_PANIC(("aghfpAtCmdEnd: Invalid sink flush\n"));
		}

#ifdef DISPLAY_AT_CMDS
		printf("\\r\\n");
#endif
	}

	aghfp->mapped_rfcomm_sink = NULL;
}
