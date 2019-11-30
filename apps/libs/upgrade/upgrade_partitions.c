/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_partitions.c

DESCRIPTION

    Implementation of partition management, dealing with the realities
    of mapping logical to physical partitions, tracking double banking,
    and also erase state.


IMPLEMENTATION NOTES

Mapping of logical to physical partition is provided as a table 
UPGRADE_UPGRADABLE_PARTITION_T provided by the VM app at runtime. 
We make the pointer to this static and provide all the access.

The status of partitions comes from a PSKEY. 

When first booted we expect the relevant words to be non-existent
and the PSSTORE module deals with initialising these.
The ORIGINAL state is held in the PSSTORE module in the copy of the
PSKEY.  
The FUTURE state is held in the same place, but is never written
back by this module. The key is being written by the upgrade 
process at the appropriate moments to ensure that the FUTURE will
be saved.

APIs are provided to
* start/resume an upgrade
* mark a partition as affected by upgrade
* commit an upgrade


@todo When we have rebooted for commit... what partitions should we
      return?  The app may be asking for Raw Serials...


*/
    
#include <ps.h>
#include <print.h>
#include <partition.h>
#include <sink.h>
#include <stream.h>

#include "upgrade_private.h"
#include "upgrade_ctx.h"

#include "upgrade_psstore.h"
#include "upgrade_partitions.h"


/****************************************************************************
NAME
    UpgradePartitionsSetMappingTable  -  set table to map logical to physical

DESCRIPTION
    Accept details of a partition mapping table. Check for validity and store
    if legal.

RETURNS
    TRUE if table supplied, and table is a valid length. FALSE otherwise.

*/
bool UpgradePartitionsSetMappingTable(const UPGRADE_UPGRADABLE_PARTITION_T * table,uint16 entries)
{
    if (   !table
        || entries == 0
        || entries > MAX_LOGICAL_PARTITIONS)
    {
        /* Protect us in case of subsequent calls */
        UpgradeCtxGet()->upgradeNumLogicalPartitions = 0;
        return FALSE;
    }
    
    UpgradeCtxGet()->upgradeLogicalPartitions = table;
    UpgradeCtxGet()->upgradeNumLogicalPartitions = entries;

    return TRUE;
}

/****************************************************************************
NAME
    UpgradePartitionsPhysicalPartition  -  get physical partition for logical

DESCRIPTION
    Given a logical partition number, interrogates the logical partition table
    based on the currently mapped partition and returns the requested bank.

*/
UpgradePartitionID UpgradePartitionsPhysicalPartition(uint16 logicalPartition,UpgradePartitionGetPhysicalType request)
{
    if (logicalPartition < UpgradeCtxGet()->upgradeNumLogicalPartitions)
    {
        UPGRADE_LOGICAL_PARTITION_ARRANGEMENT banking = UpgradeCtxGet()->upgradeLogicalPartitions[logicalPartition].banking;

        switch (banking)
        {
        case UPGRADE_LOGICAL_BANKING_SINGLE_ERASE_TO_UPDATE:
        case UPGRADE_LOGICAL_BANKING_SINGLE_KEEP_ERASED:
            return UpgradeCtxGet()->upgradeLogicalPartitions[logicalPartition].bank1;
            
        case UPGRADE_LOGICAL_BANKING_DOUBLE_UNMOUNTED:
        case UPGRADE_LOGICAL_BANKING_DOUBLE_MOUNTED:
            if (request == UpgradePartitionUpgraded)
            {
                /* Checking for an upgraded partition.
                   This will be when the partition bit has changed, in which case we want
                   to return the upgradable partition
                 */
                if (   ReadPartitionBit(UpgradeCtxGetPSKeys()->logical_partition_state.status,logicalPartition)
                    != ReadPartitionBit(UpgradeCtxGetPSKeys()->future_partition_state.status,logicalPartition))
                {
                    request = UpgradePartitionUpgradable;
                }
                else
                {
                    request = UpgradePartitionActive;
                }
            }

            if (   (    ReadPartitionBit(UpgradeCtxGetPSKeys()->logical_partition_state.status,logicalPartition)
                    && (request == UpgradePartitionActive))
                || (   !ReadPartitionBit(UpgradeCtxGetPSKeys()->logical_partition_state.status,logicalPartition)
                    && (request == UpgradePartitionUpgradable)))
            {
                return UpgradeCtxGet()->upgradeLogicalPartitions[logicalPartition].bank2;
            }
            else
            {
                return UpgradeCtxGet()->upgradeLogicalPartitions[logicalPartition].bank1;
            }

        default:
            /* Does nothing, will return appropriate error code below */
            break;
        }
    }
        
    return UPGRADE_PARTITION_NONE_MAPPED;
}


/****************************************************************************
NAME
    UpgradePartitionsNewPhysical

DESCRIPTION
    Given a physical partition returns the matching, upgraded, partition ID.

    Scans our partition mapping table for a match. Checks every logical partition 
    using existing function.
*/
UpgradePartitionID UpgradePartitionsNewPhysical(UpgradePartitionID currentPartition)
{
uint16 logical;
    
    for (logical = 0;logical < UpgradeCtxGet()->upgradeNumLogicalPartitions;logical++)
    {
        if (currentPartition == UpgradePartitionsPhysicalPartition(logical,UpgradePartitionActive))
        {
            return UpgradePartitionsPhysicalPartition(logical,UpgradePartitionUpgraded);
        }
    }

    return currentPartition;
}


/****************************************************************************
NAME
    UpgradePartitionsUpgradeStarted  -  update copy of boot state of device

DESCRIPTION
    We've been told that a new, or resumed, upgrade is starting. Reset our 
    "future" partition status.
*/

void UpgradePartitionsUpgradeStarted(void)
{
    UpgradeCtxGetPSKeys()->future_partition_state = UpgradeCtxGetPSKeys()->logical_partition_state;
}

/****************************************************************************
NAME
    UpgradePartitionsCommitUpgrade  -  update partition status

DESCRIPTION
    We've been told to commit an upgrade. Update the partition status. Do NOT
    write back the PSKEY, but rely on our caller to do so.
*/

void UpgradePartitionsCommitUpgrade(void)
{
    UpgradeCtxGetPSKeys()->logical_partition_state = UpgradeCtxGetPSKeys()->future_partition_state;
}


/****************************************************************************
NAME
    UpgradePartitionsMarkUpgrading  -  Mark a logical partition as being upgraded

DESCRIPTION
    Toggle the bank, or mark partition as dirty.
*/

void UpgradePartitionsMarkUpgrading(uint16 logicalPartition)
{
    if (logicalPartition < UpgradeCtxGet()->upgradeNumLogicalPartitions)
    {
        uint16 newBit = 1;
        UPGRADE_LOGICAL_PARTITION_ARRANGEMENT banking = UpgradeCtxGet()->upgradeLogicalPartitions[logicalPartition].banking;

        switch (banking)
        {
        case UPGRADE_LOGICAL_BANKING_SINGLE_KEEP_ERASED:
        case UPGRADE_LOGICAL_BANKING_SINGLE_ERASE_TO_UPDATE:
            /* Leave the default, of setting the bit */
            break;
            
        case UPGRADE_LOGICAL_BANKING_DOUBLE_UNMOUNTED:
        case UPGRADE_LOGICAL_BANKING_DOUBLE_MOUNTED:
            newBit = ReadPartitionBit(UpgradeCtxGetPSKeys()->logical_partition_state.status,logicalPartition) ^ 0x01;
            break;

        default:
            PRINT(("UpgradePartitionsMarkUpgrading - unexpected default"));
            break;
        }

        WritePartitionBit(UpgradeCtxGetPSKeys()->future_partition_state.status,
                            logicalPartition,newBit);
        
        UpgradeCtxGetPSKeys()->state_of_partitions = UPGRADE_PARTITIONS_UPGRADING;
    }
}

