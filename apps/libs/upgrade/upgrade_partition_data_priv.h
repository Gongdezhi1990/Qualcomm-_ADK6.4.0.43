/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_partition_data_priv.h
    
DESCRIPTION
    Definition of partition data processing state datatypes.
*/

#ifndef UPGRADE_PARTITION_DATA_PRIV_H_
#define UPGRADE_PARTITION_DATA_PRIV_H_

#include "upgrade_fw_if.h"
#include "upgrade.h"


#define UPGRADE_PARTITION_DATA_BLOCK_SIZE(CTX) (((CTX)->partitionDataBlockSize) ? ((CTX)->partitionDataBlockSize) : UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE )

#define PREFETCH_UPGRADE_BLOCKS 3

typedef enum
{
    UPGRADE_PARTITION_DATA_STATE_GENERIC_1ST_PART,
    UPGRADE_PARTITION_DATA_STATE_HEADER,
    UPGRADE_PARTITION_DATA_STATE_DATA_HEADER,
    UPGRADE_PARTITION_DATA_STATE_DATA,
    UPGRADE_PARTITION_DATA_STATE_FOOTER
} UpgradePartitionDataState;

typedef struct {
    bool inProgress;
    uint32 size;
    uint8 data[UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE];
} UpgradePartitionDataIncompleteData;

typedef struct {
    uint32 nextReqSize;
    uint32 nextOffset;
    uint32 bigReqSize;
    uint32 partitionLength;
    uint32 prefetchSize;
    uint32 requestedSize;
    UpgradePartitionDataState state;
    UpgradeFWIFPartitionHdl partitionHdl;
    uint8 *signature;
    uint16 signatureReceived;
    UpgradePartitionDataIncompleteData incompleteData;
    bool openNextPartition;
} UpgradePartitionDataCtx;

#endif /* UPGRADE_PARTITION_DATA_PRIV_H_ */
