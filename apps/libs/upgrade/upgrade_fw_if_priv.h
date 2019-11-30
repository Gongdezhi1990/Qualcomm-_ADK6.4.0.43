/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_fw_if_priv.h
    
DESCRIPTION
    Definition of FW I/F state datatype.
*/

#ifndef UPGRADE_FW_IF_PRIV_H_
#define UPGRADE_FW_IF_PRIV_H_


typedef struct
{
    /*
     * Used by CONFIG_HYDRACORE and CONFIG_QCC300X to store a platform
     * specific validation context.
     *
     * It needs to be defined in this shared header so that the size and
     * layout of UpgradeFWIFCtx is consistent in both the general upgrade
     * code and the platform specifc bits.
     */
    void *vctx;

    uint16 partitionNum;

} UpgradeFWIFCtx;

#endif /* UPGRADE_FW_IF_PRIV_H_ */
