/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

/*
  SDP Service Record generated from subwoofer_service_record.sdp by sdpgen.pl
*/


#ifndef SWAT_SERVICE_REC_H_
#define SWAT_SERVICE_REC_H_

#include "swat.h"

/* Default (example) service record used to register SWAT library with SDP */
static const uint8 swat_service_record[] =
{
    /* Service Class ID List */
    0x09, 0x00, 0x01,   /* AttrID ServiceClassIDList) */
    0x35, 0x11,         /* "Data element sequence" (Sequence length = 0x11) */
    0x1c,               /* (0x1c) Translates to: "UUID" of length 16 bytes */
    /* The SWAT UUID: 0x00001000_d10211e1_9b230002_5b00a5a5 */
    0x00,0x00,0x10,0x00,
    0xd1,0x02,0x11,0xe1,
    0x9b,0x23,0x00,0x02,
    0x5b,0x00,0xa5,0xa5,
        
    /* Protocol Descriptor List */
    0x09, 0x00, 0x04,   /* AttrID ProtocolDescriptorList */
    0x35, 0x08,         /* "Data element sequence" (Sequence length 0x08) */
    0x35, 0x06,         /* "Data element sequence" (Sequence length = 0x06) */
    0x19, 0x01, 0x00,   /* "UUID" (UUID 0x0100 = L2CAP)*/
    0x09, ((SWAT_SIGNALLING_PSM >> 8) & 0xff), (SWAT_SIGNALLING_PSM & 0xff),   /* "Unsigned Integer" (Describes the L2CAP SIGNALLING PSM) */
    
    /* Additional Protocol Descriptor List */
    0x09, 0x00, 0x0d,   /* AttrID Additional Protocol Descriptor List */
    0x35, 0x08,         /* "Data element sequence" (Sequence length = 0x08) */
    0x35, 0x06,         /* "Data element sequence" (Sequence length = 0x06) */
    0x19, 0x01, 0x00,   /* "UUID" (UUID 0x0100 = L2CAP)*/
    0x09, ((SWAT_MEDIA_PSM >> 8) & 0xff), (SWAT_MEDIA_PSM & 0xff),   /* "Unsigned Integer" Describes the L2CAP MEDIA PSM) */
    
    /* Language Base List */
    0x09, 0x00, 0x06,   /* AttrId LanguageBaseAttributeIDList */
    0x35, 0x09,         /* "Data element sequence" (Sequence length = 0x09) */
    0x09, 0x65, 0x6e,   /* "Unsigned Integer" Natural language = en (english) */
    0x09, 0x00, 0x6a,   /* "Unsigned Integer" Character encoding = UTF-8 */
    0x09, 0x01, 0x00,   /* "Unsigned Integer, Base attr id = 0x100 */
    
    /* Service Name */
    0x09, 0x01, 0x00,                       /* AttrID, ServiceName */
    0x25, 0x09,                             /* "Text string" (String length = 0x09) */
    'S','U','B','W','O','O','F','E','R',    /* Attribute value for AttrID, ServiceName */
    
    /* ServiceDescription Attribute */
    0x09, 0x01, 0x01,                                                           /* AttrID, ServiceDescription */
    0x25, 0x12,                                                                 /* "Text string" (String length = 0x16) */
    'W','i','r','e','l','e','s','s',' ','S','u','b','w','o','o','f','e','r',    /* Attribute value for AttrID, ServiceDescription */
    
    /* ProviderName attribute */
    0x09, 0x01, 0x02,   /* AttrID, ProviderName */    
    0x25, 0x03,         /* "Text string" (String length = 0x03) */
    'C','S','R',        /* Attribute value for AttrID, ProviderName */
};


#endif /* SWAT_SERVICE_REC_H_ */
