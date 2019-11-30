/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_upgrade.c

DESCRIPTION
    Interface to the upgrade library.

*/

#ifdef ENABLE_BISTO
#include <sink2bisto.h>
#endif /* ENABLE_BISTO */
#include <config_store.h>
#include <ctype.h>
#include <message.h>
#include <gaia.h>
#include <print.h>
#include <upgrade.h>
#include <vmal.h>

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_config.h"
#include "sink_configmanager.h"
#include "sink_powermanager.h"
#include "sink_volume.h"
#include "audio.h"
#include "sink_upgrade.h"
#include "sink_audio.h"
#include "sink_ble.h"
#include "sink_malloc_debug.h"
#include "sink_gaia.h"
#include "sink_ba.h"

#ifdef ENABLE_UPGRADE

#include "sink_upgrade_config_def.h"


#ifdef DEBUG_UPGRADE
#define UPGRADE_INFO(x) DEBUG(x)
#define UPGRADE_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define UPGRADE_INFO(x)
#define UPGRADE_ERROR(x)
#endif


/* NOTE: Only platforms that have ENABLE_UPGRADE_PARTITIONS_CONFIG set
 *       use the logical-to-physical partition mapping described below.
 *       If it is not set then assume that the partition layout is fixed
 *       at build time. Please refer to the platform specific upgrade
 *       documentation for details of the partition layout.
 *
 * The files sent to the upgrade library work using logical partitions
 * rather than physical ones.
 * In most cases a partition partitions are paired in such a way that
 * upgrades occur into a partition that is not being used and,
 * following a successful upgrade, the old partition can then be erased.
 *
 * The library is passed a table that gives the logical partition
 * structure.
 *
 * The example passed here, matches this partition table.
 *
 * upgrade_demo_ADK.ptn
    0, 512k, RS, (erase)     # Partition for DFU
    1, 32K, RO, VP_two.xuv  # Audio Prompt partition #1,1
    2, 32K, RO, (erase)      # Audio Prompt partition #1,2
    3, 16K, RO, (erase)      # Audio Prompt partition #2,1
    4, 16K, RO, (erase)      # Audio Prompt partition #2,2
    5, 64K, RS, (erase)     # Test partition #1
    6, 64K, RS, (erase)     # Test partition #2
    7, 8K, RS, (erase)      # Single-banked test partition
*/
/* Only applicable to the ADK
static const UPGRADE_UPGRADABLE_PARTITION_T logicalPartitions[]
                    = {UPGRADE_PARTITION_SINGLE(0x1000,DFU),
                       UPGRADE_PARTITION_DOUBLE(0x1001,0x1002,MOUNTED),
                       UPGRADE_PARTITION_DOUBLE(0x1003,0x1004,MOUNTED),
                       UPGRADE_PARTITION_DOUBLE(0x1005,0x1006,UNMOUNTED),
                       UPGRADE_PARTITION_SINGLE(0x1007,KEEP_ERASED)
                      };
*/

/* The factory-set upgrade version and PS config version.

   After a successful upgrade the values from the upgrade header
   will be written to the upgrade PS key and used in future.
*/
static const upgrade_version init_version = { 1, 0 };
static const uint16 init_config_version = 1;

#define NOW 0

#define UPGRADE_RESTARTED_DELAY D_SEC(1)

#ifdef ENABLE_BISTO
#define BISTO_OTA_RESTARTED_SLC_DELAY D_SEC(3)
#endif /* ENABLE_BISTO */

/* Maximum size of the variant string is 6 chars */
#define VARIANT_BUFFER_SIZE (7)

/* The 'SQIF' flag in the PSKEY_FSTAB entry must be set to signal a partition
   is in external flash. See description of PSKEY_FSTAB for full details. */
#define EXTERNAL_FLASH_PARTITION_PSKEY_VALUE(partition) (0x1000 | (partition))

#define SINK_UPGRADE_CONTEXT_KEY (CONFIG_UPGRADE_CONTEXT)
#define SINK_UPGRADE_LIBRARY_CONTEXT_OFFSET (1)

/* Contents of the sink_upgrade context word:
   LSB stores the gaia_transport_type
   The top bit of the MSB stores the config reset enable flag. */
#define SINK_UPGRADE_TRANSPORT_MASK (0xFF)
#define SINK_UPGRADE_CONFIG_RESET_BIT (0x1U << 15)

#ifndef ARRAY_DIM
#define ARRAY_DIM(x) (sizeof ((x)) / sizeof ((x)[0]))
#endif


static void setUpgradeContext(uint16 context);
static uint16 getUpgradeContext(void);
static void sinkUpgradeSetUpgradeConfigResetEnable(bool enabled);
static bool sinkUpgradeGetUpgradeConfigResetEnable(void);
static bool sinkUpgradeIsResetConfigEnabled(void);


/*************************************************************************
NAME
    sinkUpgradeGetLogicalPartitions

DESCRIPTION
    Read the logical partition map from the config store.

    This function allocates a memory buffer to hold the partition table
    but does not free it. The upgrade library takes ownership of it when
    it is passed in to UpgradeInit.
*/
static void sinkUpgradeGetLogicalPartitions(const UPGRADE_UPGRADABLE_PARTITION_T **partitions, uint16 *count)
{
#ifdef ENABLE_UPGRADE_PARTITIONS_CONFIG
    uint16 i;
    uint16 config_size;
    const sink_upgrade_readonly_config_def_t *config;
    uint16 num_partitions = 0;
    UPGRADE_UPGRADABLE_PARTITION_T *logical_partitions = NULL;

    config_size = configManagerGetReadOnlyConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID, (const void **)&config);
    if (config_size)
    {
        config_size *= sizeof(uint16);

        num_partitions = ((config_size - sizeof(config->upgrade_config)) / sizeof(config->logical_partitions_array[0]));
        logical_partitions = mallocZDebugPanic(sizeof(UPGRADE_UPGRADABLE_PARTITION_T) * num_partitions);

        for (i = 0; i < num_partitions; i++)
        {
            logical_partitions[i].banking = config->logical_partitions_array[i].logical_type;
            logical_partitions[i].bank1 = EXTERNAL_FLASH_PARTITION_PSKEY_VALUE(config->logical_partitions_array[i].physical_partition_1);
            logical_partitions[i].bank2 = EXTERNAL_FLASH_PARTITION_PSKEY_VALUE(config->logical_partitions_array[i].physical_partition_2);
        }

        configManagerReleaseConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID);
    }

    *partitions = logical_partitions;
    *count = num_partitions;
#else
    /* Non-configurable partition layout, so send a dummy (empty)
       partition table to the upgrade library */
    static const UPGRADE_UPGRADABLE_PARTITION_T empty_table[1] = { { 0 } };

    *partitions = empty_table;
    *count = ARRAY_DIM(empty_table);
#endif /* ENABLE_UPGRADE_PARTITIONS_CONFIG */
}

/*************************************************************************
NAME
    sinkUpgradeGetVariantId

DESCRIPTION
    Get the variant Id from the firmware and convert it into a variant
    string that can be passed to UpgradeInit.

    This function allocates a buffer for the string which must be freed
    after the call to UpgradeInit.
*/
static void sinkUpgradeGetVariant(char *variant, size_t length)
{
    int i = 0;
    char chr;
    uint32 product_id;

    PanicFalse(length >= VARIANT_BUFFER_SIZE);

    product_id = VmalVmReadProductId();
    if (product_id == 0)
    {
        variant[0] = '\0';
        return;
    }

    /* The product Id is encoded as two ascii chars + 4 integers in BCD format. */

    /* The ascii chars may be undefined or invalid (e.g. '\0').
       If so, don't include them in the variant string. */
    chr = (char)((product_id >> 8) & 0xFF);
    if (isalnum(chr))
        variant[i++] = chr;

    chr = (char)(product_id & 0xFF);
    if (isalnum(chr))
        variant[i++] = chr;

    sprintf(&variant[i], "%04X", ((uint16)((product_id >> 16) & 0xFFFF)));
}

/*************************************************************************
NAME
    sinkUpgradeInit

DESCRIPTION
    Initialise the Upgrade library
*/
void sinkUpgradeInit(Task task)
{
    uint16 num_partitions;
    const UPGRADE_UPGRADABLE_PARTITION_T *logical_partitions;
    char variant[VARIANT_BUFFER_SIZE];

    sinkUpgradeGetVariant(variant, sizeof(variant));

    sinkUpgradeGetLogicalPartitions(&logical_partitions, &num_partitions);

    /* Allow storage of info at end of (SINK_UPGRADE_CONTEXT_KEY) */
    UpgradeInit(task, SINK_UPGRADE_CONTEXT_KEY, SINK_UPGRADE_LIBRARY_CONTEXT_OFFSET,
                    logical_partitions,
                    num_partitions,
                    UPGRADE_INIT_POWER_MANAGEMENT,
                    variant,
                    upgrade_perm_always_ask,
                    &init_version,
                    init_config_version);
}

/*******************************************************************************
NAME
    sinkUpgradeIsUpgradeMsg

DESCRIPTION
    Check if a message should be handled by sinkUpgradeMsgHandler

PARAMETERS
    id      The ID for the message

RETURNS
    bool TRUE if it is an upgrade message, FALSE otherwise.
*/
bool sinkUpgradeIsUpgradeMsg(uint16 id)
{
#ifdef MESSAGE_IMAGE_UPGRADE_ERASE_STATUS
    if ( id == MESSAGE_IMAGE_UPGRADE_ERASE_STATUS )
    {
        return TRUE;
    }
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_COPY_STATUS
    if ( id == MESSAGE_IMAGE_UPGRADE_COPY_STATUS )
    {
        return TRUE;
    }
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS
    if ( id == MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS )
    {
        return TRUE;
    }
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS
    if ( id == MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS )
    {
        return TRUE;
    }
#endif
    return ( (id >= UPGRADE_UPSTREAM_MESSAGE_BASE ) && (id < UPGRADE_UPSTREAM_MESSAGE_TOP) );
}

/*******************************************************************************
NAME
    sinkUpgradeMsgHandler

DESCRIPTION
    Handle messages specific to the Upgrade library.

PARAMETERS
    task    The task the message is delivered
    id      The ID for the Upgrade message
    payload The message payload

RETURNS
    void
*/
void sinkUpgradeMsgHandler(Task task, MessageId id, Message message)
{
    switch (id)
    {
    case MESSAGE_DFU_SQIF_STATUS:
        {
            MessageDFUFromSQifStatus *msg = (MessageDFUFromSQifStatus *)message;
            UPGRADE_INFO(("MESSAGE_DFU_SQIF_STATUS: %u\n", msg->status));
            UpgradeDfuStatus(msg);
        }
        break;

    case UPGRADE_APPLY_IND:
        {
            UPGRADE_INFO(("UPGRADE_APPLY_IND:\n"));

            UpgradeApplyResponse(NOW);

            /* Store the current BA mode into PS key so that we can start in correct BA mode after reboot */
            sinkBroadcastAudioStoreConfigItem();
            /* The upgrade library has asked permission to reboot into the new
            application, so cache the state of the config reset enabled flag
            so that it can be read in the early init after the reboot. */
            sinkUpgradeSetUpgradeConfigResetEnable(sinkUpgradeIsResetConfigEnabled());
        }
        break;

    case UPGRADE_SHUT_AUDIO:
        audioShutDownForUpgrade(task, UPRGADE_COPY_AUDIO_IMAGE_OR_SWAP);
        break;

    case UPRGADE_COPY_AUDIO_IMAGE_OR_SWAP:
        SetAudioBusy((TaskData*)task);
        gaiaResetUpgradeInProgress();

#if defined MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS && defined APPS_SQIF_HAS_AUDIO
        UPGRADE_INFO(("UpgradeCopyAudioImage\n"));
        UpgradeCopyAudioImage();
#else
        UPGRADE_INFO(("UpgradeImageSwap\n"));
        UpgradeImageSwap();
#endif

        break;

    case UPGRADE_AUDIO_COPY_FAILURE:
        SetAudioBusy(NULL);
        break;

    case UPGRADE_STATUS_IND:
        {
            UPGRADE_STATUS_IND_T *msg = (UPGRADE_STATUS_IND_T *)message;
            UPGRADE_INFO(("UPGRADE_STATUS_INFO: state 0x%x\n", msg->state));

            if (msg->state == upgrade_state_commiting)
            {
                if (sinkUpgradeGetUpgradeConfigResetEnable())
                {
                    /* Upgrade is being committed so reset the config_store values to
                       the defaults built into the new application. */
                    ConfigStoreResetToDefaults();
                    ConfigStoreDisableConstMode();
                }
            }
        }
        break;

    case UPGRADE_BLOCKING_IND:
        {
            UPGRADE_INFO(("UPGRADE_BLOCKING_IND:\n"));
            if(AudioIsAudioPromptPlaying())
            {
                UpgradeBlockingResponse(100);
            }
            else
            {
                VolumeUpdateMuteStatusAllOutputs(TRUE);
                UpgradeBlockingResponse(NOW);
            }
        }
        break;

    case UPGRADE_BLOCKING_IS_DONE_IND:
        {
            UPGRADE_INFO(("UPGRADE_BLOCKING_IS_DONE_IND:\n"));
            VolumeUpdateMuteStatusAllOutputs(FALSE);
        }
        break;

    case UPGRADE_INIT_CFM:
        {
            UPGRADE_INIT_CFM_T *msg = (UPGRADE_INIT_CFM_T *)message;
            UPGRADE_INFO(("UPGRADE_INIT_CFM: status %u\n", msg->status));
            if (msg->status != upgrade_status_success)
                Panic();
        }
        break;

    case UPGRADE_RESTARTED_IND:
        {
            UPGRADE_RESTARTED_IND_T *ind = (UPGRADE_RESTARTED_IND_T *)message;
            UPGRADE_INFO(("UPGRADE_RESTARTED_IND: reason %u\n", ind->reason));

            if (sinkDataGetSinkInitialisingStatus())
            {
                /* if not in a state to be able to power on yet,
                    re-send this message with a delay. */
                MESSAGE_MAKE(restarted, UPGRADE_RESTARTED_IND_T);
                restarted->reason = ind->reason;
                UPGRADE_INFO((" sink not initialised; delaying\n"));
                MessageSendLater(task, UPGRADE_RESTARTED_IND, restarted, UPGRADE_RESTARTED_DELAY);
            }
            else
            {
                /* The upgrade library is letting the application know that a restart
                   of some sort has occurred.

                   The reason indicates how important it is to make ourselves
                   connectable.

                   For our purposes we use any indication of an upgrade being in progress
                   to send ourselves a power on event. */
                if (ind->reason != upgrade_reconnect_not_required)
                {
                    gaia_transport_type transport_type;

                    transport_type = (gaia_transport_type)sinkUpgradeGetUpgradeTransportType();

                    UPGRADE_INFO(("transport type used some time ago was 0x%x\n", transport_type));

                    if ((transport_type == gaia_transport_spp) ||
                            (transport_type == gaia_transport_rfcomm))
                    {
                        UPGRADE_INFO(("sending EventUsrPowerOn\n"));
                        MessageSend(&theSink.task, EventUsrPowerOn, NULL);
                    }
                    else if (transport_type == gaia_transport_gatt)
                    {
                        UPGRADE_INFO(("Powering on BLE\n"));
                        MessageSend(&theSink.task, EventUsrPowerOn, NULL);
                        sinkBlePowerOnEvent();
                    }
#ifdef ENABLE_BISTO
                    else if (BistoRebootDueToOtaUpgrade())
                    {
                        UPGRADE_INFO(("Bisto: sending EventUsrPowerOn\n"));
                        MessageSend(&theSink.task, EventUsrPowerOn, NULL);
                        MessageSendLater(&theSink.task, EventUsrEstablishSLC, NULL, BISTO_OTA_RESTARTED_SLC_DELAY);
                        sinkBlePowerOnEvent();
                    }
#endif /* ENABLE_BISTO */
                    else
                    {
                        UPGRADE_INFO(("sending nothing\n"));
                    }
                }
            }
        }
        break;

#ifdef MESSAGE_IMAGE_UPGRADE_ERASE_STATUS
    case MESSAGE_IMAGE_UPGRADE_ERASE_STATUS:
        {
            UPGRADE_INFO(("MESSAGE_IMAGE_UPGRADE_ERASE_STATUS\n"));
            UpgradeEraseStatus(message);
        }
        break;
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_COPY_STATUS
    case MESSAGE_IMAGE_UPGRADE_COPY_STATUS:
        {
            UPGRADE_INFO(("MESSAGE_IMAGE_UPGRADE_COPY_STATUS\n"));
            UpgradeCopyStatus(message);
        }
        break;
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS
    case MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS:
        {
            UPGRADE_INFO(("MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS\n"));
            UpgradeCopyAudioStatus(message);
        }
        break;
#endif
#ifdef MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS
    case MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS:
        {
            UPGRADE_INFO(("MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS\n"));
            UpgradeHashAllSectionsUpdateStatus(message);
        }
        break;
#endif

/* Currently in ADK6 devkits, MESSAGE_EXE_FS_VALIDATION_STATUS is defined in
   system_messages.h but MessageExeFsValidationResult is not.
   They are both related to the zarkov-only ValidationInitiateExecutablePartition
   trap, so neither should be defined really. */
#ifndef HYDRACORE
#ifdef MESSAGE_EXE_FS_VALIDATION_STATUS
    case MESSAGE_EXE_FS_VALIDATION_STATUS:
        {
            MessageExeFsValidationResult *msg = (MessageExeFsValidationResult *)message;
            UPGRADE_INFO(("MESSAGE_EXE_FS_VALIDATION_STATUS received result is %s\n", (msg->result == VALIDATE_PARTITION_PASS) ? "PASS" : "FAIL"));
            UpgradeApplicationValidationStatus(msg->result == VALIDATE_PARTITION_PASS);
        }
        break;
#endif
#endif

    case UPGRADE_START_DATA_IND:
        {
             gaia_transport_type transport_type;
             transport_type = (gaia_transport_type) sinkUpgradeGetUpgradeTransportType();

             UPGRADE_INFO(("UPGRADE_START_DATA_IND received"));

             if(transport_type == gaia_transport_gatt)
             {
                 if (!sinkUpgradeIsProtectAudio())
                 {
                      gaiaSetGattUpgradeInProgress(TRUE);
                      sinkBleResetParamsForUpgrade();
                      sinkGaiaSendUpgradeOptimisationEvent(TRUE);
                 }
             }
        }
    break;

    case UPGRADE_END_DATA_IND:
        {
             gaia_transport_type transport_type;

             transport_type = (gaia_transport_type) sinkUpgradeGetUpgradeTransportType();

             UPGRADE_INFO(("UPGRADE_END_DATA_IND received"));

             if(transport_type == gaia_transport_gatt)
             {
                UPGRADE_END_DATA_IND_T *msg = (UPGRADE_END_DATA_IND_T *)message;
                UPGRADE_INFO(("UPGRADE_END_DATA_IND state %d", msg->state));

                if (msg->state == upgrade_end_state_abort)
                {
                    sinkGaiaSendUpgradeOptimisationEvent(FALSE);
                    gaiaResetUpgradeInProgress();
                }
                else if (msg->state == upgrade_end_state_complete)
                {
                    sinkGaiaSendUpgradeOptimisationEvent(FALSE);

                    /*Can consider disconnecting profile connections here*/

                    /*Reset upgrade in progress in UPRGADE_COPY_AUDIO_IMAGE_OR_SWAP*/
                }
            }
        }
    break;

    default:
        UPGRADE_INFO(("Unhandled 0x%04X\n", id));
        break;
    }
}

/*******************************************************************************
NAME
    sinkUpgradePowerEventHandler

DESCRIPTION
    Handle power events and pass relevant ones to the Upgrade library.
    The upgrade library is interested in two kinds of events:
    1. When the library is in the 'normal' state then low battery when
       a charger is not connected triggers transition to
       the 'low battery error' state.
    2. When the library is in the 'low battery error' state, then connecting
       a charger (or when battery level will magically change from low to ok
       without connecting a charger) enables transition back to
       the 'normal' state.

PARAMETERS
    void

RETURNS
    void
*/
void sinkUpgradePowerEventHandler(void)
{
    upgrade_power_state_t power_state = upgrade_battery_ok;

    if(powerManagerIsChargerConnected())
    {
        power_state = upgrade_charger_connected;
    }
    else if(powerManagerIsVbatLow() || powerManagerIsVbatCritical())
    {
        /* Only when charger is not connected and battery level is low notify
         * the upgrade library. This is because purpose the upgrade library's
         * low battery handling is to prevent draining battery to much when
         * charger is not connected.
         */
        power_state = upgrade_battery_low;
    }

    UpgradePowerManagementSetState(power_state);
}

void sinkUpgradeSetUpgradeTransportType(uint16 type)
{
    uint16 context = getUpgradeContext();

    context &= ~SINK_UPGRADE_TRANSPORT_MASK;
    context |= ((uint16)type & SINK_UPGRADE_TRANSPORT_MASK);

    setUpgradeContext(context);
}

uint16 sinkUpgradeGetUpgradeTransportType(void)
{
    return (uint16)(getUpgradeContext() & SINK_UPGRADE_TRANSPORT_MASK);
}

/*************************************************************************
NAME
    setUpgradeContext

DESCRIPTION
    Set the sink_upgrade context into the upgrade context PS key that is
    shared with the upgrade library.
*/
static void setUpgradeContext(uint16 context)
{
    uint16 *buffer;
    uint16 actual_length;

    buffer = mallocZDebugNoPanic(64 * sizeof(uint16));
    if (buffer)
    {
        actual_length = ConfigRetrieve(CONFIG_UPGRADE_CONTEXT, buffer, (64 * sizeof(uint16)));

        buffer[0] = context;

        if (actual_length < 1)
        {
            actual_length = 1;
        }

        ConfigStore(CONFIG_UPGRADE_CONTEXT, buffer, actual_length * sizeof(uint16));

        freeDebugNoPanic(buffer);
    }
}

/*************************************************************************
NAME
    getUpgradeContext

DESCRIPTION
    Get the sink_upgrade context from the upgrade context PS key that is
    shared with the upgrade library.
*/
static uint16 getUpgradeContext(void)
{
    uint16 *buffer;
    uint16 actual_length;
    uint16 context = 0;

    buffer = mallocZDebugNoPanic(64 * sizeof(uint16));
    if (buffer)
    {
        actual_length = ConfigRetrieve(SINK_UPGRADE_CONTEXT_KEY, buffer, (64 * sizeof(uint16)));

        if (actual_length >= 1)
        {
            context = buffer[0];
        }

        freeDebugNoPanic(buffer);
    }

    return context;
}

/******************************************************************************
NAME
    sinkUpgradeSetUpgradeConfigResetEnable

DESCRIPTION
    Store the value of the "reset app configuration after update" flag
    in the CONFIG_UPGRADE_CONTEXT PS key so that it can be read in the early
    init (before the config_store library has been initialised).

PARAMS
    enabled State of configuration reset enabled flag to store.

RETURNS
    void
*/
static void sinkUpgradeSetUpgradeConfigResetEnable(bool enabled)
{
    uint16 context = getUpgradeContext();

    if (enabled)
        context |= SINK_UPGRADE_CONFIG_RESET_BIT;
    else
        context &= ~SINK_UPGRADE_CONFIG_RESET_BIT;

    setUpgradeContext(context);
}

/******************************************************************************
NAME
    sinkUpgradeGetUpgradeConfigResetEnable

DESCRIPTION
    Read the stored value of the "reset app configuration after update" flag.

PARAMS
    void

RETURNS
    bool State of configuration reset enabled flag to store.
*/
static bool sinkUpgradeGetUpgradeConfigResetEnable(void)
{
    return ((getUpgradeContext() & SINK_UPGRADE_CONFIG_RESET_BIT) == SINK_UPGRADE_CONFIG_RESET_BIT);
}

/*************************************************************************
NAME
    sinkUpgradeIsResetConfigEnabled

DESCRIPTION
    Query the config_store to see if the application configuration should
    be reset when the VM application is upgraded.
*/
static bool sinkUpgradeIsResetConfigEnabled(void)
{
    bool enabled = FALSE;
    uint16 config_size;
    const sink_upgrade_readonly_config_def_t *config;

    config_size = configManagerGetReadOnlyConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID, (const void **)&config);
    if (config_size)
    {
        enabled = config->upgrade_config.enable_app_config_reset;

        configManagerReleaseConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID);
    }

    return enabled;
}

bool SinkUpgradeRunningNewImage(void)
{
    if (sinkUpgradeGetUpgradeConfigResetEnable())
        return UpgradeRunningNewApplication(SINK_UPGRADE_CONTEXT_KEY, SINK_UPGRADE_LIBRARY_CONTEXT_OFFSET);
    else
        return FALSE;

}

/*************************************************************************
NAME
    sinkUpgradeIsProtectAudio

DESCRIPTION
    Get the value of 'protect audio during audio'
    This will be Disabled by default, meaning, on an ongoing upgrade, audio will be suspended and
    Upgrade improvements will apply.
    If Enabled, ongoing audio will be streaming, while upgrade improvements may not be applicable.

RETURNS
    TRUE if audio needs to be retained, FALSE otherwise

**************************************************************************/
bool sinkUpgradeIsProtectAudio(void)
{
    bool protect_audio = FALSE;
    uint16 config_size;
    const sink_upgrade_readonly_config_def_t *config;

    config_size = configManagerGetReadOnlyConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID, (const void **)&config);
    if (config_size)
    {
        protect_audio = config->upgrade_config.protect_audio_during_upgrade;

        configManagerReleaseConfig(SINK_UPGRADE_READONLY_CONFIG_BLK_ID);
    }

    return protect_audio;
}

#endif /* ENABLE_UPGRADE */
