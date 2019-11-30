/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_msg_fw.h
    
DESCRIPTION

*/
#ifndef UPGRADE_MSG_FW_H_
#define UPGRADE_MSG_FW_H_

#ifndef UPGRADE_FW_MSG_BASE
#define UPGRADE_FW_MSG_BASE 0x100
#endif

typedef enum
{
    UPGRADE_FW_PARTITION_VALIDATION_SUCCESS = UPGRADE_FW_MSG_BASE
    /* TODO: replace with real message */
#ifndef MESSAGE_EXE_FS_VALIDATION_STATUS
    ,MESSAGE_EXE_FS_VALIDATION_STATUS
#endif
} UpgradeMsgFW;

#endif /* UPGRADE_MSG_FW_H_ */
