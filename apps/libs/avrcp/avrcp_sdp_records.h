/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_sdp_records.h
    
DESCRIPTION
    Header file for AVRCP default SDP records. 
    
*/

#ifndef ACRCP_SDP_RECORDS_H_
#define AVRCP_SDP_RECORDS_H_

#include "avrcp.h"

#define AVRCP_CONTROLLER_CAT_DEFAULT AVRCP_CATEGORY_1

/* Static Data structures for populating Service Records */
static const uint8 serviceClassControllerDefault[] =
{
    0x09, 0x00, 0x01,                           /* ServiceClassIDList(0x0001) */
        0x35, 0x06,                             /* Data Element Sequence - 6 Bytes */
            0x19, 0x11, 0x0E,                   /* UUID -  A/V Remote Control */
            0x19, 0x11, 0x0F,                   /* UUID - A/V Remote Control Controller */   
            
    0x09, 0x00, 0x04,                           /* Protocol Descriptor List */
        0x35, 0x10,                             /* Data Element Sequence - 16 Bytes */
            0x35, 0x06,                         /* Data Element Sequence - 6 Bytes */
               0x19, 0x01, 0x00,                /* L2CAP UUID */
               0x09, 0x00, 0x17,                /* AVCTP PSM Value */
            0x035, 0x06,                        /* Data Element Sequence - 6 Bytes */
               0x19, 0x00, 0x17,                /* AVCTP UUID */
               0x09, 0x01, 0x04,                /* AVCTP version 1.4 */
               
    0x09, 0x00, 0x09,                           /* Profile Descriptor List */
        0x035, 0x08,                            /* Data Element Sequence of 8 */
            0x035, 0x06,                        /* Data Element Sequence of 6 */
               0x19, 0x11, 0x0E,                /* A/V Remote Control UUID */
               0x09, 0x01, 0x06,                /* AVRCP version 1.6 */
    0x09, 0x03 , 0x11,                          /* Supported Features List */
        0x09, 0x00, AVRCP_CONTROLLER_CAT_DEFAULT/* Default - Supports only Cat 1 */
};

#define AVRCP_TARGET_CAT_DEFAULT AVRCP_CATEGORY_2

static const uint8 serviceClassTargetDefault[] =
{
    0x09, 0x00, 0x01,                           /* ServiceClassIDList(0x0001) */
        0x35, 0x03,                             /* Data Element Sequence - 3 Bytes */
            0x19, 0x11, 0x0C,                   /* UUID -  A/V Remote Control Target */
            
    0x09, 0x00, 0x04,                           /* Protocol Descriptor List */
        0x35, 0x10,                             /* Data Element Sequence - 16 Bytes */
            0x35, 0x06,                         /* Data Element Sequence - 6 Bytes */
               0x19, 0x01, 0x00,                /* L2CAP UUID */
               0x09, 0x00, 0x17,                /* AVCTP PSM Value */
            0x035, 0x06,                        /* Data Element Sequence - 6 Bytes */
               0x19, 0x00, 0x17,                /* AVCTP UUID */
               0x09, 0x01, 0x04,                /* AVCTP version 1.4 */
               
    0x09, 0x00, 0x09,                           /* Profile Descriptor List */
        0x035, 0x08,                            /* Data Element Sequence of 8 */
            0x035, 0x06,                        /* Data Element Sequence of 6 */
               0x19, 0x11, 0x0E,                /* A/V Remote Control UUID */
               0x09, 0x01, 0x06,                /* AVRCP version 1.6 */
    0x09, 0x03 , 0x11,                          /* Supported Features List */
        0x09, 0x00, AVRCP_TARGET_CAT_DEFAULT    /* Default - Supports only Cat 2 */
};

#endif /* AVRCP_SDP_RECORDS_H_ */
