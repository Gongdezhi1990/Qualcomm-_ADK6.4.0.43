/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_partitions.h
    
DESCRIPTION
    The upgrade library needs to be able to keep track of physical partitions
    to deal with logical partitions and double banking (upgrade into one, 
    and then swap).

    This header file contains functions to manage this, although the 
    information is maintained in persistent store.

    There are XXXXXXXXXXXXXXXXX sets of APIs.


                        ?????
                       ?     ?
                             ?
                            ?
                           ?
                          ?
                          ?
                          ?

                          ?


*/
#ifndef UPGRADE_PARTITIONS_H_
#define UPGRADE_PARTITIONS_H_

#include <csrtypes.h>
#include <message.h>
#include "upgrade_private.h"

/*! Enumerated type used to request a physical partition.

*/
typedef enum 
{
    UpgradePartitionActive,
    UpgradePartitionUpgradable,
    UpgradePartitionUpgraded
} UpgradePartitionGetPhysicalType;

typedef uint16 UpgradePartitionID;

#define UPGRADE_PARTITION_NONE_MAPPED   ((UpgradePartitionID)0)

#define UPGRADE_PARTITION_PHYSICAL_PARTITION(x) ((x)&0xFF)
#define UPGRADE_PARTITION_PHYSICAL_DEVICE(x)    (((x)&0x3000) >> 12)


/*! Macros for bit array manipulation

   Bit numbering follows bit numbering conventions within words (more efficient
   code wise)
   i.e. 0xF000 0x0001
   Has bits 12-15 and 16 set
*/
#define WritePartitionBit(array,bit,value) do {\
        uint16 word = (bit) / 16; \
        uint16 val = ((uint16)(value) & 0x1) << ((bit) & 0xF); \
        uint16 mask = (uint16)~(1u << ((bit) & 0xF)); \
        array[word] = (array[word] & mask) | val; \
    } while(0)

#define ReadPartitionBit(array,bit) (((array)[(bit)/16]>>((bit)&0xF))&0x1)

/*!
    @brief UpgradePartitionsSetMappingTable

    Initialisation of partition mapping table.

    @param table    Pointer to a table (ideally const) which maps each entry
                    to 1 or 2 partitions, giving the type of mapping.
    @param entries  Number of entries in the table

    @return TRUE if table of valid length supplied, FALSE otherwise
*/
bool UpgradePartitionsSetMappingTable(const UPGRADE_UPGRADABLE_PARTITION_T * table,uint16 entries);


/*!
    @brief UpgradePartitionsPhysicalPartition

    Retrieve a physical partition from the logical partition mapping table, 
    based on the current status of the partitions.

    @param logicalPartition Number of the partition. These are numbered from
                            zero in the mapping table.
    @param request          Which partition to return. Specifies the partition
                            based on state i.e. active, erasable, upgradable...
*/
UpgradePartitionID UpgradePartitionsPhysicalPartition(uint16 logicalPartition,UpgradePartitionGetPhysicalType request);



/*!
    @brief UpgradePartitionsNewPhysical

    Given a physical partition returns the matching, upgraded, partition ID.
    If the partition has not been upgraded, or is not recognised, returns the 
    original value.

    @param currentPartition The physical partition reference. This follows the same
                            numbering scheme as @ref PSKEY_FSTAB.
    @returns The upgraded partition ID, if upgraded, or the same value as passed.
*/
UpgradePartitionID UpgradePartitionsNewPhysical(UpgradePartitionID currentPartition);



/*!
    @brief UpgradePartitionsUpgradeStarted

    Inform the partitions module that an upgrade has been started (or resumed).
*/
void UpgradePartitionsUpgradeStarted(void);
    

/*!
    @brief UpgradePartitionsCommitUpgrade

    Tell the partitions module to commit the state of the partitions.

    @note It is the responsibility of the caller to update the PSKEY 
        - @ref UpgradeSavePSKeys
*/
void UpgradePartitionsCommitUpgrade(void);


/*!
    @brief UpgradePartitionsMarkUpgrading

    Set the partition status bit in our internal (prospective) status
    to be opposite of the PSKEY value (use the other bank).

    Single banked partitions are marked with a '1'
*/
void UpgradePartitionsMarkUpgrading(uint16 logicalPartition);


/*!
    @brief UpgradePartitionsEraseAllManaged

    Erases partitions managed by the upgrade manager in preparation for 
    an upgrade.

    This should be called after an upgrade (so that a background update
    can occur without need to erase a partition).

    @returns UPGRADE_PARTITIONS_ERASED if all partitions erased
             UPGRADE_PARTITIONS_ERROR if an error was received
*/
UpgradePartitionsState UpgradePartitionsEraseAllManaged(void);


#endif /* UPGRADE_PARTITIONS_H_ */

