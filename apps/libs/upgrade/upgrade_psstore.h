/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_psstore.h
    
DESCRIPTION
    The upgrade library needs to be able to keep track of upgrades and the
    state of the device.

    This header file contains ALL functions that access the persistent storage.
    Other header files may manage specific functionality that records state
    in the persistent storage.


    There are two sets of APIs.

    Generic access to upgrade library PSKEY storage
    This provides access to the UPGRADE_LIB_PSKEY structure defined in
    upgrade_private.h. This includes functions to commit the storage.


    Access to the file system table (FSTAB).
    NOTE The original API for this was written for a Zarkov demo which 
    supported VM app upgrade only. This needs to be more complex to deal 
    with upgrading additional file partitions.
*/
#ifndef UPGRADE_PSSTORE_H_
#define UPGRADE_PSSTORE_H_

#include <csrtypes.h>
#include <message.h>
#include "upgrade_private.h"

/*! Number of PSKEY writes we should be able to make to permit a critical
 * operation 
 */
#define UPGRADE_PS_WRITES_FOR_CRITICAL_OPERATIONS   2


/*!
    @brief UpgradeSavePSKeys

    Writes the local copy of PSKEY information to the PS Store. 
    Note that this API will not panic, even if no PS storage has been
    made available.
*/
void UpgradeSavePSKeys(void);

/*!
    @brief UpgradeLoadPSStore

    Initialisation function to load the pskey information upon 
    init of the upgrade library.

    @param dataPskey Number of the user pskey to use for ugprade data.
    @param dataPskeyStart Offset in dataPskey from which the upgrade library can use.

*/
void UpgradeLoadPSStore(uint16 dataPskey,uint16 dataPskeyStart);



/*!
    @brief UpgradeSetToTryUpgrades

    Updates the Persistent Store so that after a reboot, the upgraded
    partitions will be used.
    
    @return TRUE if the values in persistent store were changed.
*/

bool UpgradeSetToTryUpgrades(void);

/*!
    @brief UpgradeCommitUpgrades

    Updates the Persistent Storage so that the current partitions are
    'permanent'. In the event of a storage error a panic will be raised
    leading to a reboot. 

    @note That this could become a vicious circle
    
*/
void UpgradeCommitUpgrades(void);

/*!
    @brief UpgradeRevertUpgrades

    Removes the temporary persistent store entry for the FSTAB so that
    the upgraded partitions will no longer be used after a reboot.
    
*/
void UpgradeRevertUpgrades(void);

/*!
    @brief UpgradePSSpaceForCriticalOperations

    Checks whether there appears to be sufficient free space in the PSSTORE
    to allow upgrade PSKEY operations to complete.

    The upgrade process needs to remember when a partition has been closed
    and without this an attempt to resume an upgrade will fail.

    @note The upgrade library does not know how many more operations are 
    required to complete an upgrade so it is possible that false will be 
    returned even though this is the last partition and we could safely
    write and reboot.

    @returns false if it is considered risky to continue
    
*/
bool UpgradePSSpaceForCriticalOperations(void);

/*!
    @brief UpgradePsRunningNewApplication
    
    Query if we are running an upgraded application, but it hasn't been
    committed yet.

    Note: It is only valid to call this function during the early
    initialisation of the application, before UpgradeInit has been called.

    @param dataPskey Number of the user pskey to use for ugprade data.
    @param dataPskeyStart Offset in dataPskey from which the upgrade library can use.
    
    @return TRUE if partway through an upgrade and running the upgraded
            application; FALSE otherwise.
*/
bool UpgradePsRunningNewApplication(uint16 dataPskey, uint16 dataPskeyStart);

#endif /* UPGRADE_PSSTORE_H_ */
