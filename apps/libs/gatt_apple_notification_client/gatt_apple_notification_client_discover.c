/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_discover.h"
#include "gatt_apple_notification_client_private.h"
#include "gatt_apple_notification_client_external_msg_send.h"
#include "gatt_apple_notification_client_ready_state.h"
#include "gatt_apple_notification_client_write.h"

static void nextAfterDiscoverCharacteristics(GANCS *ancs)
{
    switch (ancs->pending_cmd)
    {
        case ancs_pending_discover_all_characteristics:
            /* Check if the mandatory Notification Source is supported by the server, otherwise there is no point to initialise client */
            if (CHECK_VALID_HANDLE(ancs->notification_source))
            {
                gattAncsSendInitResponse(ancs, gatt_status_success);
                gattAncsReadyStateUpdate(ancs, TRUE);
            }
            else
                gattAncsSendInitResponse(ancs, gatt_status_failure);
        break;

        default:
            DEBUG_PANIC(("ANCS: No action after discover characteristics [0x%x]\n", ancs->pending_cmd));
        break;
    }
}

static void processDiscoveredDescriptor(GANCS *ancs, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    switch (ancs->pending_cmd)
    {
        case ancs_pending_discover_all_characteristic_descriptors:
            /* Expected discovered descriptor, wait for more */
        break;

        case ancs_pending_set_ns_notify_enable:
        case ancs_pending_set_ns_notify_disable:
            if (cfm->uuid_type == gatt_uuid16)
            {
                if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
                {
                    bool notify_pending = (ancs->pending_cmd == ancs_pending_set_ns_notify_enable);

                    PRINT(("ANCS: Found Notification Source CCD handle = [0x%04x]\n", cfm->handle));

                    writeClientConfigNotifyValue(ancs, notify_pending, cfm->handle);
                    ancs->ns_ccd = cfm->handle;
                    ancs->pending_cmd = ancs_pending_write_ns_cconfig;
                }
            }
        break;

        case ancs_pending_set_ds_notify_enable:
        case ancs_pending_set_ds_notify_disable:
            if (cfm->uuid_type == gatt_uuid16)
            {
                if (cfm->uuid[0] == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_UUID)
                {
                    bool notify_pending = (ancs->pending_cmd == ancs_pending_set_ds_notify_enable);

                    PRINT(("ANCS: Found Data Source CCD handle = [0x%04x]\n", cfm->handle));

                    writeClientConfigNotifyValue(ancs, notify_pending, cfm->handle);
                    ancs->ds_ccd = cfm->handle;
                    ancs->pending_cmd = ancs_pending_write_ds_cconfig;
                }
            }
        break;

        case ancs_pending_write_ns_cconfig:
        case ancs_pending_write_ds_cconfig:
            PRINT(("ANCS: Processing descriptor, state [0x%04x]\n", ancs->pending_cmd));
        break;

        default:
            DEBUG_PANIC(("ANCS: Wrong state for descriptor processing, state [0x%04x], handle [0x%04x]\n", ancs->pending_cmd, cfm->handle));
        break;
    }
}

static void nextAfterDiscoverDescriptors(GANCS *ancs)
{
    switch (ancs->pending_cmd)
    {
        case ancs_pending_discover_all_characteristic_descriptors:
            ancs->pending_cmd = ancs_pending_none;
            break;

        case ancs_pending_set_ns_notify_enable:
        case ancs_pending_set_ns_notify_disable:
            gattAncsSendSetNotificationSourceNotificationResponse(ancs, gatt_status_request_not_supported);
            break;

        case ancs_pending_set_ds_notify_enable:
        case ancs_pending_set_ds_notify_disable:
            gattAncsSendSetDataSourceNotificationResponse(ancs, gatt_status_request_not_supported);
            break;

        case ancs_pending_write_ns_cconfig:
        case ancs_pending_write_ds_cconfig:
            /* No action needed as write of client configuration descriptor will happen next */
            break;

        default:
            DEBUG_PANIC(("ANCS: No action after discover descriptors [0x%x]\n", ancs->pending_cmd));
            break;
    }
}


/*******************************************************************************
 * Helper function to get the endhandle for discovering characteristic descriptor of NS.
 */
uint16 findEndHandleForCharDesc(GANCS *ancs, uint16 startHandle, uint16 endHandle, uint8 characteristic_wanted)
{
    uint8 charIndex = 0;
    uint8 charVal = 0;
    uint8 mask;
    uint8 char_report_mask = ancs->char_report_mask;

    unsigned retHandle = 0;

    /* The characteristics are 2 bit fields overlaid in the same byte
       Our task data has a mask for what to report, and the required
       characteristic value
     */

    /* if and only if there is proper characteristic request for the descriptor */
    while( charIndex < GATT_APPLE_NOTIFICATION_MAX_CHAR )
    {
        if(char_report_mask)
        {
            mask = GATT_APPLE_NOTIFICATION_FIELD_MASK(charIndex);
            /* Mask the value and shift */
            charVal = char_report_mask & mask;
            charVal = charVal >> GATT_APPLE_NOTIFICATION_FIELD_START(charIndex);

            /* Remove the value we have just checked from the report mask */
            char_report_mask = (char_report_mask & ~mask);

            /* Did the value match the one we wanted */
            if( charVal == characteristic_wanted)
            {
                /* Check the next field */
                mask = GATT_APPLE_NOTIFICATION_FIELD_MASK(charIndex+1);;
                charVal = (char_report_mask & mask);
                charVal = charVal >> GATT_APPLE_NOTIFICATION_FIELD_START(charIndex+1);

                switch( charVal )
                {
                    case GATT_APPLE_NOTIFICATION_NS:
                        retHandle = ancs->notification_source- 1;
                    break;

                    case GATT_APPLE_NOTIFICATION_DS:
                        retHandle = ancs->data_source - 1;
                    break;

                    case GATT_APPLE_NOTIFICATION_CP:
                        retHandle = ancs->control_point - 1;
                    break;

                    default:
                    {
                        /* TODO : This is probably wrong! */
                        if(startHandle < endHandle)
                        {
                            retHandle = endHandle;
                        }
                        else
                        {
                            retHandle = startHandle;
                        }
                    }
                    break;
                }
                /* Exit loop */
                break;
            }
       }
       charIndex ++;
    }

    return (uint16)retHandle;
}


/****************************************************************************/
void handleAncsDiscoverAllCharacteristicsResp(GANCS *ancs, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *cfm)
{
    uint8  charIndex = 0;
    uint8  charVal = 0;
        /* char_report_mask is 8 bit value which is divided as 2 bits each for the 3 characteristic.
        The least significant 2 bits indicates the first characteristic discovered,
        the second significant 2 bits indicates the second characteristic discovered and so on & so forth.
        This mask is later used for getting the start & end handle for discovernig characteristic descriptor
        for each characteristic
    */
    charIndex = ancs->char_report_mask_index;

    if (cfm->status == gatt_status_success)
    {
        if (cfm->uuid_type == gatt_uuid128)
        {
            if (CHECK_ANCS_NS_UUID(cfm))
            {
                PRINT(("ANCS: Found Notification Source handle [0x%04x], status [0x%04x]\n", cfm->handle , cfm->status));
                ancs->notification_source = cfm->handle;
                charVal = GATT_APPLE_NOTIFICATION_NS;
            }
            else if(CHECK_ANCS_DS_UUID(cfm))
            {
                PRINT(("ANCS: Found Data Source handle [0x%04x], status [0x%04x]\n", cfm->handle , cfm->status));
                ancs->data_source = cfm->handle;
                charVal = GATT_APPLE_NOTIFICATION_DS;
            }
            else if(CHECK_ANCS_CP_UUID(cfm))
            {
                PRINT(("ANCS: Found Control Point handle [0x%04x], status [0x%04x]\n", cfm->handle , cfm->status));
                ancs->control_point = cfm->handle;
                charVal = GATT_APPLE_NOTIFICATION_CP;
            }

            if (charVal)
            {
                charVal = (uint8)(charVal << GATT_APPLE_NOTIFICATION_FIELD_START(charIndex));
                ancs->char_report_mask |= charVal;
                charIndex++;
            }
            ancs->char_report_mask_index = charIndex;
        }
        /* Ignore unwanted characteristics */
    }

    /* No more to come, so process the characteristics */
    if (!cfm->more_to_come)
    {
        /* Reset the index as this is going to be used in getting the descriptor */
        ancs->char_report_mask_index = 0;
        nextAfterDiscoverCharacteristics(ancs);
    }
}


/****************************************************************************/
bool ancsDiscoverAllCharacteristicDescriptors(GANCS *ancs, uint16 start_handle, uint16 end_handle)
{
    GattManagerDiscoverAllCharacteristicDescriptors(&ancs->lib_task,
                                                start_handle,
                                                end_handle);
    return TRUE;
}


/****************************************************************************/
void handleAncsDiscoverAllCharacteristicDescriptorsResp(GANCS *ancs, const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *cfm)
{
    if (cfm->status == gatt_status_success)
    {
        processDiscoveredDescriptor(ancs, cfm);
    }

    if (!cfm->more_to_come)
    {
        nextAfterDiscoverDescriptors(ancs);
    }
}
