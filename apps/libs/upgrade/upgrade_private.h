/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade_private.h

DESCRIPTION
    Contains any common, private, information for the Upgrade Library.

    This is any information that is shared between the implementation
    modules. Information "owned" by a module can and should be defined
    in the module header.
*/
#ifndef UPGRADE_PRIVATE_H_
#define UPGRADE_PRIVATE_H_

#include <upgrade.h>

/* PSKEYS are intentionally limited to 32 words to save stack. */
#define PSKEY_MAX_STORAGE_LENGTH    32

typedef enum
{
    UPGRADE_PARTITIONS_ERASED = 0,  /* Expected initial state */
    UPGRADE_PARTITIONS_UPGRADING,
    UPGRADE_PARTITIONS_ERROR
} UpgradePartitionsState;


#define MAX_LOGICAL_PARTITIONS  32
typedef struct
{
    uint16 status[(MAX_LOGICAL_PARTITIONS+15)/16];
} UpgradePartitionStatus;

/*!
    @brief Message from the loader.

    0 - message wasn't received
    1 - it is good
    2 - it is bad
*/

typedef enum
{
    UPGRADE_LOADER_MSG_NONE = 0,
    UPGRADE_LOADER_MSG_SUCCESS = 1,
    UPGRADE_LOADER_MSG_ERROR = 2,
    UPGRADE_LOADER_MSG_INVALID = 4
} UpgradeMessageFromTheLoader;

/* Structure used internally to save information about the upgrade across
 * boots in a PSKEY.
 *
 * See upgrade_psstore.h for functions to access / update the values in this
 * structure.
 */
typedef struct upgrade_lib_pskey_struct
{
    /* Members relating to the current active config */
    upgrade_version version;
    uint16 config_version;
    UpgradePartitionStatus logical_partition_state;
    UpgradePartitionsState state_of_partitions;

    /* Members relating to current upgrade version */
    upgrade_version version_in_progress;
    uint16 config_version_in_progress;
    uint32 id_in_progress;

    /* Upgrade status information */
    uint16 upgrade_in_progress_key;
    uint16 last_closed_partition;
    uint16 dfu_partition_num;
    UpgradeMessageFromTheLoader loader_msg;
    UpgradePartitionStatus future_partition_state;

} UPGRADE_LIB_PSKEY;

/* Use sizeof to make sure we get the right size under Windows and final build */
#define UPGRADE_PRIVATE_PSKEY_USAGE_LENGTH_WORDS  (sizeof(UPGRADE_LIB_PSKEY)/sizeof(uint16))

/*!
    @brief Returns the VM application task.
    @return Task VM application task registered with the library.
*/
Task UpgradeGetAppTask(void);

/*!
    @brief Returns the upgrade library main task.
    @return Task Upgrade library main task.
*/
Task UpgradeGetUpgradeTask(void);

#endif /* UPGRADE_PRIVATE_H_ */
