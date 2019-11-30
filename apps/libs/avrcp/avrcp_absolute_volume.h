/****************************************************************************
Copyright (c) 2009 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_absolute_volume.h
    
DESCRIPTION
    Header file for Absolute Volume feature.
    
*/

#ifndef  AVRCP_ABSOLUTE_VOLUME_H_
#define  AVRCP_ABSOLUTE_VOLUME_H_

#include "avrcp_common.h"

/* Preprocessor definitions */
#define AVRCP_MAX_VOL_MASK      0x7F

/* Internal function declarations */
#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
void avrcpSendAbsoluteVolumeCfm(AVRCP               *avrcp,
                                avrcp_status_code   status,
                                const uint8         *data);
#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only Lib */
void avrcpHandleInternalAbsoluteVolumeRsp(AVRCP                   *avrcp,
                    const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T   *res);

void avrcpHandleSetAbsoluteVolumeCommand(AVRCP *avrcp, uint8 volume);
#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /*  AVRCP_ABSOLUTE_VOLUME_H_*/

