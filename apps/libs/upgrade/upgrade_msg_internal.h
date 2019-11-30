/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_msg_internal.h
    
DESCRIPTION

*/
#ifndef UPGRADE_MSG_INTERNAL_H_
#define UPGRADE_MSG_INTERNAL_H_

#ifndef UPGRADE_INTERNAL_MSG_BASE
#define UPGRADE_INTERNAL_MSG_BASE 0x300
#endif

typedef enum
{
    UPGRADE_INTERNAL_IN_PROGRESS = UPGRADE_INTERNAL_MSG_BASE,
    UPGRADE_INTERNAL_IN_PROGRESS_JOURNAL,
    UPGRADE_INTERNAL_CONTINUE,

    /*! used to prompt the state machine to perform a warm reboot
        after we've asked the VM application for permission */
    UPGRADE_INTERNAL_REBOOT,

    /*! used to prompt the state machine to perform a partition erase
        tasks after we've asked the VM application for permission */
    UPGRADE_INTERNAL_ERASE,

    UPGRADE_INTERNAL_COMMIT,

    /*! internal message to set the state machine to battery low */
    UPGRADE_INTERNAL_BATTERY_LOW,

    /*! send to itself after reboot to commit, it is used to handle no reconnection cases */
    UPGRADE_INTERNAL_RECONNECTION_TIMEOUT

} UpgradeMsgInternal;

#endif /* UPGRADE_MSG_INTERNAL_H_ */
