/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_util.c
    
DESCRIPTION
 Helper functions

*/
#include <string.h>
#include "pbapc_util.h"

/* String for the vCard Listing mime Type */
static const uint8 pbapcVcardlistingType[] =  {"x-bt/vcard-listing"};

/* String for the Phonebook mime Type */
static const uint8 pbapcPhonebookType[] =  {"x-bt/phonebook"};

/* String for the vCard Entry mime Type */
static const uint8 pbapcVcardType[] =  {"x-bt/vcard"};

/* This must match with the enum values of pbapcPhoneBbook in pbapc.h */
static const char* const pbapcPbDir[] = {"telecom","pb","ich","och","mch","cch"};

/* Sim String */
static const uint8 sim1Str[] = "sim1";

#define PBAPC_PB_DIR_SIZE   6

/**************************************************************************
 * NAME:
 *  pbapcFrame2ByteAppParams
 *
 * DESCRIPTION
 *  Frame application parameters of 2 Byte length
 ************************************************************************/
void pbapcFrame2ByteAppParams( uint8* ptr, uint8 param, uint16 value ) 
{
    ptr[0] = param;
    ptr[1] = 2; /* Two byte length */
    ptr[2] = value >> 8;
    ptr[3] = value & 0xFF;
}

/************************************************************************
 * NAME
 *  pbapcFrameByteParam
 *
 * DESCRIPTION
 *  Frame One byte application parameter
 *************************************************************************/
void pbapcFrameByteAppParams( uint8* ptr, uint8 param, uint8 value )
{
    ptr[0] = param;
    ptr[1] = 1; /* one byte length */
    ptr[2] = value;
}

/************************************************************************
 * NAME
 *  pbapcFrame4ByteParam
 *
 * DESCRIPTION
 *  Frame One byte application parameter
 *************************************************************************/
void pbapcFrame4ByteAppParams( uint8* ptr, uint8 param, uint32 value )
{
    ptr[0] = param;
    ptr[1] = 4; /* four byte length */
    ptr[2] = value >> 24 ;
    ptr[3] = (value >> 16) & 0xFF;
    ptr[4] = (value >> 8 ) & 0xFF;
    ptr[5] = value & 0xFF;
}
/**************************************************************************
 * NAME:
 *  pbapcFramevCardListAppParams
 *
 * DESCRIPTION
 * Frame the vCard List app Params. Size of listData must be 
 * validated before calling this function.
 ************************************************************************/
uint16 pbapcFramevCardListAppParams( uint8 order ,
                                     const uint8* srchVal,
                                     uint16 srchValLen,
                                     uint8 srchAttr,
                                     uint8* listData )
{
    uint16 len = 0;

    /* add order */
    if( order )
    {
        pbapcFrameByteAppParams( &listData[len], pbapc_app_order, order );
        len  +=  BYTE_APP_PARAM_LEN;      
    }

    /* Add Search Value */
    if( srchValLen && srchVal )
    {
        listData[ len ] =  pbapc_app_search_value;
        listData[ len + 1 ] =   srchValLen;
        len +=  APP_PARAM_HDR_LEN;
        memmove( &listData[ len ], srchVal, srchValLen );
        len += srchValLen;
    }

    /* Add Search Attribute */
    if( srchAttr )
    {
        pbapcFrameByteAppParams( &listData[len], 
                                 pbapc_app_search_attr,
                                 srchAttr );
        len  +=  BYTE_APP_PARAM_LEN;  
    }

    return len;
}

/**************************************************************************
 * NAME:
 *  pbapcFrameListAppParams
 *
 * DESCRIPTION
 * Frame the common application parameters like max list size 
 * and offset. list data must have space for 6 Bytes.
 ************************************************************************/
uint16 pbapcFrameListAppParams( uint16 maxListCount,
                              uint16 startOffset,
                              uint8* listData )
{
    uint16 len = 0;

   /* Create the application header for 2 headers*/
   pbapcFrame2ByteAppParams( &listData[len], 
                             pbapc_app_max_list_count,
                             maxListCount );
   len += WORD_APP_PARAM_LEN;

    if( startOffset )
    {
        pbapcFrame2ByteAppParams( &listData[len],
                                  pbapc_app_start_offset,
                                  startOffset ); 
        len += WORD_APP_PARAM_LEN;
    }
    /* Length of App params */
    return len;
}

/**************************************************************************
 * NAME:
 *  pbapcFrameFilterFormat
 *
 * DESCRIPTION
 * Frame the common application parameters like filter and format.
 * The buffer must have 13 bytes free space.
 ************************************************************************/
uint16 pbapcFrameFilterFormat( uint32  filterHi,
                               uint32  filterLo,
                               uint8   format,
                               uint8* listData )
{
    uint16 len = 0;

    if( filterLo || filterHi )
    {
        pbapcFrame4ByteAppParams( &listData[len + WORD_APP_PARAM_LEN], 0, 
                                  filterLo );
        pbapcFrame4ByteAppParams( &listData[len], pbapc_app_filter, 
                                  filterHi );
        listData[len+1] = 8; /* Length of filter is 8 */
        len += LONG_LONG_WORD_APP_PARAM_LEN;
    }

    if( format )
    {
        /* Next offset starts at 10 */
        pbapcFrameByteAppParams( &listData[len], pbapc_app_format, format);
        len += BYTE_APP_PARAM_LEN;
    }

    /* filer app param size of 10 + format of 3 */
    return len;
}

/**************************************************************************
 * NAME:
 *  pbapcGetPbNameFromID
 *
 * DESCRIPTION
 *     Returns Pointer to the book name.  
 *  len contains the length of the returned name. 
 ************************************************************************/
const uint8 *pbapcGetPbNameFromID(uint8 book, uint16 *len)
{
    *len = strlen(pbapcPbDir[book]);
    return (const uint8*)pbapcPbDir[book];
}

/**************************************************************************
 * NAME:
 *  pbapcGetSimName
 *
 * DESCRIPTION
 *  Returns the SIM repository name.
 ************************************************************************/
const uint8* pbapcGetSimName( uint16 *len )
{
    *len = sizeof(sim1Str);
    return sim1Str;
}

/**************************************************************************
 * NAME:
 *  pbapcGetPbPathFromID
 *
 * DESCRIPTION
 *  book must be between 0-5. This function does not check that.
 *     Returns Pointer to the book name.  
 *  len contains the length of the returned name. 
 *  The path must be of size 16 bytes for local and 21 for SIM.
 ************************************************************************/
uint16 pbapcGetPbPathFromID( bool  sim,  uint8 book, uint8* path) 
{
    uint16 length = 0;
    uint16* len = &length;
    uint16 offset= 0;

    /* Copy the SIM Path */
    if( sim )
    { 
        memmove(&path[offset], "SIM1/", 5); 
        offset += 5;
    }
    
    /* Get Telecom first */
    memmove(&path[offset], pbapcGetPbNameFromID( 0 , len ), length );
    offset += length;

    if( book )
    {
        path[offset] = '/';
        offset++;
        memmove(&path[offset], pbapcGetPbNameFromID(book, len), length );
        offset += length;
        memmove(&path[offset], ".vcf", 5);
        offset += 5;
    }
    else
    {
        path[offset] = '\0';
        offset++;
    }

    return offset;
}


/**************************************************************************
 * NAME:
 *  pbapcoGetPhonebookMimeType
 *
 * DESCRIPTION
 * Returns Pointer to the string. 
 * len contains the length of the returned string.
 ************************************************************************/
const uint8 *pbapcGetPhonebookMimeType(uint16 *len)
{
    *len = sizeof(pbapcPhonebookType);
    return pbapcPhonebookType;
}

/**************************************************************************
 * NAME:
 *  PbapcGetvCardListingMimeType
 *
 * DESCRIPTION
 * Returns Pointer to the string. 
 * len contains the length of the returned string.
 ************************************************************************/
const uint8 *pbapcGetvCardListingMimeType(uint16 *len)
{
    *len = sizeof( pbapcVcardlistingType );
    return  pbapcVcardlistingType;
}


/**************************************************************************
 * NAME:
 *  PbapcGetvCardMimeType
 *
 * DESCRIPTION
 * Returns Pointer to the string. 
 * len contains the length of the returned string.
 ************************************************************************/
const uint8 *pbapcGetvCardMimeType(uint16 *len)
{
    *len = sizeof( pbapcVcardType );
    return  pbapcVcardType;
}



