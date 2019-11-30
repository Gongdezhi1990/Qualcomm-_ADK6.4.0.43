/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_psstore.c

DESCRIPTION

    Implementation of an interface to Persistent Storage to get
    details of the file system and anything else related to the
    possibilities of upgrade.

NOTES
    Errors. Cause panics. End of. This behaviour in itself is problematic
            but if we are trying to switch applications then an error
            should indicate a reason to restart. We can't really
            delegate this to the VM app. We can't wind back to a previous
            application.
    Caching. Persistent store keys are not cached. There isn't a mechanism
            to subscribe to PSKEY changes. Since we don't actually expect
            to be called that frequently it makes sense to access the keys
            we need when we need them.
*/


#include <stdlib.h>
#include <string.h>
#include <csrtypes.h>
#include <panic.h>
#include <ps.h>

#include <print.h>
#include <upgrade.h>

#include "upgrade_ctx.h"
#include "upgrade_fw_if.h"
#include "upgrade_psstore.h"
#include "upgrade_psstore_priv.h"
#include "upgrade_partitions.h"

#include <imageupgrade.h>

static void loadUpgradeKey(UPGRADE_LIB_PSKEY *key_data, uint16 key, uint16 key_offset);

/*
 * There is no support for PSKEY_FSTAB in CSRA68100.
 */

/****************************************************************************
NAME
    UpgradeLoadPSStore  -  Load PSKEY on boot

DESCRIPTION
    Save the details of the PSKEY and offset that we were passed on 
    initialisation, and retrieve the current values of the key.

    In the unlikely event of the storage not being found, we initialise
    our storage to 0x00 rather than panicking.
*/
void UpgradeLoadPSStore(uint16 dataPskey,uint16 dataPskeyStart)
{
    union {
        uint16      keyCache[PSKEY_MAX_STORAGE_LENGTH];
        FSTAB_COPY  fstab;
        } stack_storage;
    uint16 lengthRead;

    UpgradeCtxGet()->upgrade_library_pskey = dataPskey;
    UpgradeCtxGet()->upgrade_library_pskeyoffset = dataPskeyStart;

    /* Worst case buffer is used, so confident we can read complete key 
     * if it exists. If we limited to what it should be, then a longer key
     * would not be read due to insufficient buffer
     * Need to zero buffer used as the cache is on the stack.
     */
    memset(stack_storage.keyCache,0,sizeof(stack_storage.keyCache));
    lengthRead = PsRetrieve(dataPskey,stack_storage.keyCache,PSKEY_MAX_STORAGE_LENGTH);
    if (lengthRead)
    {
        memcpy(UpgradeCtxGetPSKeys(),&stack_storage.keyCache[dataPskeyStart],
                UPGRADE_PRIVATE_PSKEY_USAGE_LENGTH_WORDS*sizeof(uint16));
    }
    else
    {
        memset(UpgradeCtxGetPSKeys(),0x0000,sizeof(UpgradeCtxGetPSKeys()) * sizeof(uint16));
    }
}

/****************************************************************************
NAME
    UpgradePsRunningNewApplication
    
DESCRIPTION
    See if we are part way through an upgrade. This is done by a combination of
    the ImageUpgradeSwapTryStatus trap and - if that returns TRUE - querying the
    upgrade PS keys in the same manner as the non-CONFIG_HYDRACORE variant.

    This is used by the application during early boot to check if the
    running application is the upgraded one but it hasn't been committed yet.

    Note: This should only to be called during the early init phase, before
          UpgradeInit has been called.
    
RETURNS
    TRUE if the upgraded application is running but hasn't been
    committed yet. FALSE otherwise, or in the case of an error.
*/
bool UpgradePsRunningNewApplication(uint16 dataPskey, uint16 dataPskeyStart)
{
    bool result = ImageUpgradeSwapTryStatus();
    if (result)
    {
        /* We are in the process of upgrading but have not committed, yet */
        UPGRADE_LIB_PSKEY ps_key;

        loadUpgradeKey(&ps_key, dataPskey, dataPskeyStart);

        /* 
         * Don't need to check upgrade_in_progress_key as ImageUpgradeSwapTryStatus
         * having returned TRUE does in its stead.
         * Return true if the running application is newer than the previous one.
         */
        if (ps_key.version_in_progress.major > ps_key.version.major
                || (ps_key.version_in_progress.major == ps_key.version.major
                    && ps_key.version_in_progress.minor > ps_key.version.minor))
        {
            /* The result is already TRUE */
        }
        else
        {
            result = FALSE;
        }
    }
    return result;
}

static void loadUpgradeKey(UPGRADE_LIB_PSKEY *key_data, uint16 key, uint16 key_offset)
{
    uint16 lengthRead;
    uint16 keyCache[PSKEY_MAX_STORAGE_LENGTH];

    /* Worst case buffer is used, so confident we can read complete key 
     * if it exists. If we limited to what it should be, then a longer key
     * would not be read due to insufficient buffer
     * Need to zero buffer used as the cache is on the stack.
     */
    memset(keyCache, 0, sizeof(keyCache));
    lengthRead = PsRetrieve(key, keyCache, PSKEY_MAX_STORAGE_LENGTH);
    if (lengthRead)
    {
        memcpy(key_data, &keyCache[key_offset], sizeof(*key_data));
    }
    else
    {
        memset(key_data, 0x0000, sizeof(*key_data));
    }
}

/****************************************************************************
NAME
    UpgradeSetToTryUpgrades

DESCRIPTION
    Swaps the application entries in the temporary FSTAB so that the upgraded
    partitions are used on the next reboot.

    Ensures that the new temporary FSTAB is otherwise a copy of the
    implementation version.

RETURNS
    TRUE if the temporary FSTAB was updated with changed values.
*/
bool UpgradeSetToTryUpgrades(void)
{
    return TRUE;
}


/****************************************************************************
NAME
    UpgradeCommitUpgrades

DESCRIPTION
    Writes the implementation store entry of the FSTAB so that the correct
    partitions are used after a power cycle.


    @TODO: This code MAY NOT BE good enough to deal with all the errors that can 
            happen.  Now includes synchronisation with the partitions table, but
            ...

RETURNS
    n/a
*/
void UpgradeCommitUpgrades(void)
{
    bool result;
    result = ImageUpgradeSwapCommit();
    UNUSED(result); /* For when PRINT does nothing */
    PRINT(("ImageUpgradeSwapCommit() returns %d\n", result));
    UpgradePartitionsCommitUpgrade();
}


/****************************************************************************
NAME
    UpgradeRevertUpgrades

DESCRIPTION
    Sets the transient store entry of the FSTAB to match the implementation so 
    that the original partitions are used after a warm reset or power cycle.

    Note that the panic in the case of a failure to write will cause a reboot
    in normal operation - and a reboot after panic will clear the transient
    store.

RETURNS
    n/a
*/
void UpgradeRevertUpgrades(void)
{
}

/****************************************************************************
NAME
    UpgradeIsRunningNewImage
    
DESCRIPTION
    See if we are have rebooted for an image upgrade.
    
RETURNS
    What the ImageUpgradeSwapTryStatus trap returns:
    TRUE if we have rebooted to try a new image and that new image is OK, else FALSE.
*/
bool UpgradeIsRunningNewImage(void)
{
    return ImageUpgradeSwapTryStatus();
}