/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_ctx.h
    
DESCRIPTION
    Header file for the Upgrade context.
*/

#ifndef UPGRADE_CTX_H_
#define UPGRADE_CTX_H_

#include <message.h>

#include "imageupgrade.h"

#include "upgrade_partition_data_priv.h"
#include "upgrade_fw_if_priv.h"
#include "upgrade_sm.h"
#include "upgrade_private.h"
#include "upgrade.h"
#include "upgrade_psstore_priv.h"

typedef struct
{
    TaskData smTaskData;
    Task mainTask;
    Task clientTask;
    Task transportTask;
    /* An indication of whether the transport needs a UPGRADE_TRANSPORT_DATA_CFM
     * or not.
     */
    bool need_data_cfm;
    /* An indication of whether the transport can handle a request for multiple
     * blocks at a time, or not.
     */
    bool request_multiple_blocks;
    UpgradeState smState;
    UpgradePartitionDataCtx *partitionData;
    UpgradeFWIFCtx fwCtx;
    uint16 partitionValidationNextPartition;
    const UPGRADE_UPGRADABLE_PARTITION_T *upgradeLogicalPartitions;
    uint16 upgradeNumLogicalPartitions;
    /* Storage for PSKEY management.
     *
     * The library uses a (portion of) PSKEY supplied by the application
     * so needs to remember the details of the PSKEY.
     */
    uint16 upgrade_library_pskey;
    uint16 upgrade_library_pskeyoffset;
    uint32 partitionDataBlockSize;
    /*! Storage for upgrade library information maintained in PSKEYS.
     *
     * The values are only @b read from PSKEY on boot.
     * The local copy can be read/written at any time, but should be written
     * @b to storage as little as possible.
     *
     * Since this PSKEY storage structure has multiple uses be aware that
     * any value you write to the local copy may be committed at any time.
     *
     * The values will always be initialised, to 0x00 at worst.
     */
    UPGRADE_LIB_PSKEY UpgradePSKeys;

    /* Current level of permission granted the upgrade library by the VM
     * application */
    upgrade_permission_t perms;

    /*! The current power management mode (disabled/battery powered) */
    upgrade_power_management_t power_mode;

    /*! The current power management state informed by the VM app */
    upgrade_power_state_t power_state;

    /*! device variant */
    char dev_variant[UPGRADE_HOST_VARIANT_CFM_BYTE_SIZE+1];
    
    /* P0 Hash context */
    hash_context_t vctx;
    
    /* CSR Valid message received flag */
    bool isCsrValidDoneReqReceived;
    
} UpgradeCtx;

void UpgradeCtxSet(UpgradeCtx *ctx);
UpgradeCtx *UpgradeCtxGet(void);

/*!
    @brief Set the partition data context in the upgrade context.
*/
void UpgradeCtxSetPartitionData(UpgradePartitionDataCtx *ctx);

/*! @brief Get the partition data context.

    @return Pointer to the partition data context. It may be
            NULL if it has not been set.
*/
UpgradePartitionDataCtx *UpgradeCtxGetPartitionData(void);

UpgradeFWIFCtx *UpgradeCtxGetFW(void);
UPGRADE_LIB_PSKEY *UpgradeCtxGetPSKeys(void);

#endif /* UPGRADE_CTX_H_ */
