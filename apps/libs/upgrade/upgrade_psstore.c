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

/****************************************************************************
NAME
    UpgradeSavePSKeys  -  Save our PSKEYS

DESCRIPTION
    Save our PSKEYS into Persistent Storage.

    The existing contents of the key are read first. If they chance not to 
    exist the the value we do not control are set to 0x0000 (deemed safer 
    than panicking or using a marker such as 0xFACE)

    Note that the upgrade library initialisation has guaranteed that the 
    the pskeys fit within the 64 words allowed.
    
    Although not technically part of our API, safest if we allow for the 
    PSKEY to be longer than we use.
*/
void UpgradeSavePSKeys(void)
{
    uint16 keyCache[PSKEY_MAX_STORAGE_LENGTH];
    uint16 min_key_length = UpgradeCtxGet()->upgrade_library_pskeyoffset
                                    +UPGRADE_PRIVATE_PSKEY_USAGE_LENGTH_WORDS;

    /* Find out how long the PSKEY is */
    uint16 actualLength = PsRetrieve(UpgradeCtxGet()->upgrade_library_pskey,NULL,0);
    if (actualLength)
    {
        PsRetrieve(UpgradeCtxGet()->upgrade_library_pskey,keyCache,actualLength);
    }
    else
    {
        if (UpgradeCtxGet()->upgrade_library_pskeyoffset)
        {
            /* Initialise the portion of key before us */
            memset(keyCache,0x0000,sizeof(keyCache));
        }
        actualLength = min_key_length;
    }

    /* Correct for too short a key */
    if (actualLength < min_key_length)
    {
        actualLength = min_key_length;
    }

    memcpy(&keyCache[UpgradeCtxGet()->upgrade_library_pskeyoffset],UpgradeCtxGetPSKeys(),
                UPGRADE_PRIVATE_PSKEY_USAGE_LENGTH_WORDS*sizeof(uint16));
    PsStore(UpgradeCtxGet()->upgrade_library_pskey,keyCache,actualLength);
}

/****************************************************************************
NAME
    UpgradePSSpaceForCriticalOperations

DESCRIPTION

    Checks whether there appears to be sufficient free space in the PSSTORE
    to allow upgrade PSKEY operations to complete.

RETURNS
    FALSE if insufficient space by some metric, TRUE otherwise.
*/
bool UpgradePSSpaceForCriticalOperations(void)
{
    uint16 keySize = UpgradeCtxGet()->upgrade_library_pskeyoffset 
                     + UPGRADE_PRIVATE_PSKEY_USAGE_LENGTH_WORDS;
    return (PsFreeCount(keySize) >= UPGRADE_PS_WRITES_FOR_CRITICAL_OPERATIONS);
}

/****************************************************************************
NAME
    UpgradeGetVersion

DESCRIPTION
    Get the version information from the PS keys.

*/
bool UpgradeGetVersion(uint16 *major, uint16 *minor, uint16 *config)
{
    bool result = FALSE;
    if (UpgradeIsInitialised())
    {
        *major  = UpgradeCtxGetPSKeys()->version.major;
        *minor  = UpgradeCtxGetPSKeys()->version.minor;
        *config = UpgradeCtxGetPSKeys()->config_version;
        result = TRUE;
    }

    return result;
}

/****************************************************************************
NAME
    UpgradeGetInProgressVersion

DESCRIPTION
    Get the "in progress" version information from the PS keys.

*/
bool UpgradeGetInProgressVersion(uint16 *major, uint16 *minor, uint16 *config)
{
    bool result = FALSE;
    if (UpgradeIsInitialised())
    {
        *major  = UpgradeCtxGetPSKeys()->version_in_progress.major;
        *minor  = UpgradeCtxGetPSKeys()->version_in_progress.minor;
        *config = UpgradeCtxGetPSKeys()->config_version_in_progress;
        result = TRUE;
    }

    return result;
}

/****************************************************************************
NAME
    UpgradePartialUpdateInterrupted

DESCRIPTION
    Get the indication of whether a partial upgrade has been interrupted from the PS keys.

*/
bool UpgradePartialUpdateInterrupted(void)
{
    bool interrupted = FALSE;
    if ((UpgradeCtxGetPSKeys()->upgrade_in_progress_key == UPGRADE_RESUME_POINT_START) &&
        (UpgradeCtxGetPSKeys()->state_of_partitions == UPGRADE_PARTITIONS_UPGRADING) &&
        (UpgradeCtxGetPSKeys()->last_closed_partition > 0))
    {
        /* A partial update has been interrupted. Don't erase. */
        interrupted = TRUE;
    }
    return interrupted;
}
