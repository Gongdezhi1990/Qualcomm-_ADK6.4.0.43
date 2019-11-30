/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.
*/


#ifndef AGHFP_HFP_SERVICE_RECORD_H_
#define AGHFP_HFP_SERVICE_RECORD_H_


#define UUID_HANDSFREE 0x111e
#define UUID_HANDSFREE_AG 0x111f


static const uint8 aghfp_hfp_service_record[] =
{
    /* Service class ID list */
    0x09, 0x00, 0x01, 	/* AttrID, ServiceClassIDList */
    0x35, 0x06, 		/* 6 bytes in total DataElSeq */
    0x19, UUID_HANDSFREE_AG >> 8, UUID_HANDSFREE_AG & 0xff,	/* 2 byte UUID, Service class = HFP AG */
    0x19, 0x12, 0x03,	/* 2 byte UUID Service class = GenericAudio */
        
    /* Protocol descriptor list */
    0x09, 0x00, 0x04,	/* AttrId ProtocolDescriptorList */
    0x35, 0x0c, 		/* 11 bytes in total DataElSeq */
    0x35, 0x03, 		/*3 bytes in DataElSeq */
    0x19, 0x01, 0x00,	/* 2 byte UUID, Protocol = L2CAP */
        
    0x35, 0x05, 		/* 5 bytes in DataElSeq */
    0x19, 0x00, 0x03,  	/* 1 byte UUID Protocol = RFCOMM */
    0x08, 0x00, 		/* 1 byte UINT - server channel template value 0 - to be filled in by app */

#if 0
	/* Language base list */
0x09, 0x00, 0x06, 	/* AttrId LanguageBaseAttributeIDList */
0x35, 0x09,  		/* 9 bytes in total DataElSeq */
0x09, 0x65, 0x6e,  	/* 2 byte UINT, Natural language = en (English) */
0x09, 0x00, 0x6a,  	/* 2 byte UINT, Character encoding = UTF-8 */
0x09, 0x01, 0x00,  	/* 2 byte UINT, Base attr id = 0x100 */
#endif

    /* Profile descriptor list */
    0x09, 0x00, 0x09, 	/* AttrId, ProfileDescriptorList */
    0x35, 0x08, 		/* 8 bytes in total DataElSeq */
    0x35, 0x06, 		/* 6 bytes in total DataElSeq */
    0x19, UUID_HANDSFREE >> 8, UUID_HANDSFREE & 0xff, 	/* 2 byte UUID, Service class = HF */

	0x09, 0x01, 0x01,  	/* 2 byte UINT, version = 101 */

    /* Service name */
    0x09, 0x01, 0x00, 	/* AttrId - Service Name */
    0x25, 13, 		    /* 13 byte string */
    'V','o','i','c','e',' ','G', 'a', 't', 'e', 'w', 'a', 'y',
        
    /* Network */
    0x09, 0x03, 0x01, 
    0x08, 0x01,			/* Ability to reject a call */
        
    /* Supported features - passed in from the application */    
    0x09, 0x03, 0x11,   /* AttrId - Supported Features */
    0x09, 0x00, 0x00    /* 2 byte UINT - supported features inserted by app */
};


#endif /* AGHFP_SERVICE_RECORD_H_ */
