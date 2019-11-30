/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/

#include <csrtypes.h>
#include <string.h>
#include <panic.h>

#include "config_definition.h"

#include "sink_configmanager.h"
#include "sink_pio_debounce.h"
#include "sink_pio_config_def.h"

#include "pio_common.h"

#define PACK_32(pio_low, pio_high) ((((uint32)pio_high << 16) & 0xffff0000ul) | ((uint32)(pio_low) & 0x0000fffful))

typedef struct
{

    uint8              numChecks;
    uint8              timeBetweenChecks;

    pio_common_allbits pioDebounceAssignments;

} pioDebounceGroupConfiguration_t;

/****************************************************************************
NAME
    GetDebounceGroupConfiguration

DESCRIPTION
    Get debounce group configuration.

PARAMETERS
    debounceGroup - specifies the pio group
    readOnlyConfigData - the configuration data
    debounceGroupConfiguration - required configuration
*/
static void GetDebounceGroupConfiguration(uint16                          debounceGroup,
                                          pio_readonly_config_def_t       *readOnlyConfigData,
                                          pioDebounceGroupConfiguration_t *debounceGroupConfiguration)
{
    switch(debounceGroup)
    {
        case 0:

            debounceGroupConfiguration->
               numChecks = readOnlyConfigData->pio_debounce_group_0_num_checks;
            debounceGroupConfiguration->
               timeBetweenChecks = readOnlyConfigData->pio_debounce_group_0_time_between_checks;

            debounceGroupConfiguration->pioDebounceAssignments.mask[0] =
               PACK_32(readOnlyConfigData->pio_debounce_group_0_0_15, readOnlyConfigData->pio_debounce_group_0_16_31);
            debounceGroupConfiguration->pioDebounceAssignments.mask[1] =
               PACK_32(readOnlyConfigData->pio_debounce_group_0_32_47, readOnlyConfigData->pio_debounce_group_0_48_63);
            debounceGroupConfiguration->pioDebounceAssignments.mask[2] =
               PACK_32(readOnlyConfigData->pio_debounce_group_0_64_79, readOnlyConfigData->pio_debounce_group_0_80_95);

            break;

        case 1:

            debounceGroupConfiguration->
               numChecks = readOnlyConfigData->pio_debounce_group_1_num_checks;
            debounceGroupConfiguration->
               timeBetweenChecks = readOnlyConfigData->pio_debounce_group_1_time_between_checks;

            debounceGroupConfiguration->pioDebounceAssignments.mask[0] =
               PACK_32(readOnlyConfigData->pio_debounce_group_1_0_15, readOnlyConfigData->pio_debounce_group_1_16_31);
            debounceGroupConfiguration->pioDebounceAssignments.mask[1] =
               PACK_32(readOnlyConfigData->pio_debounce_group_1_32_47, readOnlyConfigData->pio_debounce_group_1_48_63);
            debounceGroupConfiguration->pioDebounceAssignments.mask[2] =
               PACK_32(readOnlyConfigData->pio_debounce_group_1_64_79, readOnlyConfigData->pio_debounce_group_1_80_95);

            break;

        default:

            Panic(); /* Illegal group number... */
            break;
    }
}

/****************************************************************************
NAME
    PioConfigureDebounceGroups

DESCRIPTION
    Configure PIO debounce groups.

RETURNS
    TRUE: Group 0 configured, FALSE: Group 0 not configured
*/
bool PioConfigureDebounceGroups(void)
{
    bool debounceGroup0Configured = FALSE;

    pioDebounceGroupConfiguration_t debounceGroupConfiguration;

    pio_readonly_config_def_t *readOnlyConfigData = NULL;

    if(configManagerGetReadOnlyConfig(PIO_READONLY_CONFIG_BLK_ID, (const void **)&readOnlyConfigData))
    {
        configManagerReleaseConfig(PIO_READONLY_CONFIG_BLK_ID);

        /* Only config group 0 if button debounce settings are disabled. */

        if(!readOnlyConfigData->pio_debounce_group_0_enable_button_mappings)
        {
            /* Configure debounce group 0. */

            GetDebounceGroupConfiguration(0u,
                                          readOnlyConfigData,
                                          &debounceGroupConfiguration);

            /* Only setup group if at least one PIO is specified. */

            if((debounceGroupConfiguration.pioDebounceAssignments.mask[0] != 0) ||
                  (debounceGroupConfiguration.pioDebounceAssignments.mask[1] != 0) ||
                     (debounceGroupConfiguration.pioDebounceAssignments.mask[2] != 0))
            {
                if(!PioCommonDebounceGroup(0u,
                                           &debounceGroupConfiguration.pioDebounceAssignments,
                                           (uint16)debounceGroupConfiguration.numChecks,
                                           (uint16)debounceGroupConfiguration.timeBetweenChecks))
                {
                    Panic();
                }
            }

            debounceGroup0Configured = TRUE;
        }

        /* Configure debounce group 1. */

        GetDebounceGroupConfiguration(1u,
                                      readOnlyConfigData,
                                      &debounceGroupConfiguration);

        /* Only setup group if at least one PIO is specified. */

        if((debounceGroupConfiguration.pioDebounceAssignments.mask[0] != 0) ||
              (debounceGroupConfiguration.pioDebounceAssignments.mask[1] != 0) ||
                 (debounceGroupConfiguration.pioDebounceAssignments.mask[2] != 0))
        {
            if(!PioCommonDebounceGroup(1u,
                                       &debounceGroupConfiguration.pioDebounceAssignments,
                                       (uint16)debounceGroupConfiguration.numChecks,
                                       (uint16)debounceGroupConfiguration.timeBetweenChecks))
            {
                Panic();
            }
        }
    }
    else
    {
        Panic();
    }

    return debounceGroup0Configured;
}
