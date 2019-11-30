/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_hs_service_record.h
    
DESCRIPTION
    
*/

#include "hfp.h"

#ifndef HSP_SERVICE_RECORD_H_
#define HSP_SERVICE_RECORD_H_


static const uint8 hsp_service_record[] =
{
    /* Service class ID list */
    0x09,0x00,0x01, /* AttrID , ServiceClassIDList */
    0x35,0x09, /* 9 bytes in total DataElSeq */
    0x19,0x11,0x08,/* 2 byte UUID, Service class = HSP */
    0x19,0x11,0x31,/* 2 byte UUID, Service class = Headset - HS */
    0x19,0x12,0x03,/* 2 byte UUID Service class  = GenericAudio */
        
    /* protocol descriptor list */
    0x09,0x00,0x04,/* AttrId ProtocolDescriptorList */
    0x35,0x0c, /* 12 bytes in total DataElSeq */
    0x35,0x03, /*3 bytes in DataElSeq */
    0x19, 0x01,0x00,/* 2 byte UUID, Protocol = L2CAP */
        
    0x35,0x05, /* 5 bytes in DataElSeq */
    0x19, 0x00,0x03,  /* 1 byte UUID Protocol = RFCOMM */
    0x08, HSP_DEFAULT_CHANNEL, /* 1 byte UINT - Channel number overwritten by app if not available */

    /* Language base list */
    0x09, 0x00, 0x06, /* AttrId LanguageBaseAttributeIDList */
    0x35, 0x09,  /* 9 bytes in total DataElSeq */
    0x09, 0x65, 0x6e,  /* 2 byte UINT, Natural language = en (english) */
    0x09, 0x00, 0x6a,  /* 2 byte UINT, Character encoding = UTF-8 */
    0x09, 0x01, 0x00,  /* 2 byte UINT, Base attr id = 0x100 */

    /* profile descriptor list */
    0x09,0x00,0x09, /* AttrId, ProfileDescriptorList */
    0x35,0x08, /* 8 bytes in total DataElSeq */
    0x35,0x06, /* 6 bytes in total DataElSeq */
    0x19, 0x11,0x08, /* 2 byte UUID, Service class = HS */    

    0x09, 0x01,0x02,  /* 2 byte UINT, version = 102 */

    /* service name */
    0x09, 0x01, 0x00, /* AttrId - Service Name */
    0x25, 0x07, /* 7 byte string */
    'H','e','a','d','s','e','t',
        
    /* remote audio volume control */
    0x09, 0x03, 0x02, /* AttrId - remote audio volume control */
    0x28, 0x01 /* boolean - TRUE we do support remote audio volume control */
};

static const uint8 hsp_service_record_2[] =
{
    /* Service class ID list */
    0x09,0x00,0x01, /* AttrID , ServiceClassIDList */
    0x35,0x09, /* 9 bytes in total DataElSeq */
    0x19,0x11,0x08,/* 2 byte UUID, Service class = HSP */
    0x19,0x11,0x31,/* 2 byte UUID, Service class = Headset - HS */
    0x19,0x12,0x03,/* 2 byte UUID Service class  = GenericAudio */
        
    /* protocol descriptor list */
    0x09,0x00,0x04,/* AttrId ProtocolDescriptorList */
    0x35,0x0c, /* 12 bytes in total DataElSeq */
    0x35,0x03, /*3 bytes in DataElSeq */
    0x19, 0x01,0x00,/* 2 byte UUID, Protocol = L2CAP */
        
    0x35,0x05, /* 5 bytes in DataElSeq */
    0x19, 0x00,0x03,  /* 1 byte UUID Protocol = RFCOMM */
    0x08, HSP_DEFAULT_CHANNEL_2, /* 1 byte UINT - Channel number overwritten by app if not available */

    /* Language base list */
    0x09, 0x00, 0x06, /* AttrId LanguageBaseAttributeIDList */
    0x35, 0x09,  /* 9 bytes in total DataElSeq */
    0x09, 0x65, 0x6e,  /* 2 byte UINT, Natural language = en (english) */
    0x09, 0x00, 0x6a,  /* 2 byte UINT, Character encoding = UTF-8 */
    0x09, 0x01, 0x00,  /* 2 byte UINT, Base attr id = 0x100 */

    /* profile descriptor list */
    0x09,0x00,0x09, /* AttrId, ProfileDescriptorList */
    0x35,0x08, /* 8 bytes in total DataElSeq */
    0x35,0x06, /* 6 bytes in total DataElSeq */
    0x19, 0x11,0x08, /* 2 byte UUID, Service class = HS */    

    0x09, 0x01,0x02,  /* 2 byte UINT, version = 102 */

    /* service name */
    0x09, 0x01, 0x00, /* AttrId - Service Name */
    0x25, 0x07, /* 7 byte string */
    'H','e','a','d','s','e','t',
        
    /* remote audio volume control */
    0x09, 0x03, 0x02, /* AttrId - remote audio volume control */
    0x28, 0x01 /* boolean - TRUE we do support remote audio volume control */
};


/* 
Indexes of values we need to insert. Update these if you change the 
service record above.
*/
#define HSP_RFCOMM_IDX          30


#endif /* HSP_SERVICE_RECORD_H_ */

