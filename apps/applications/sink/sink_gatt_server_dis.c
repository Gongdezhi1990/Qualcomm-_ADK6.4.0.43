/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_dis.c

DESCRIPTION
    Routines to handle messages sent from the GATT Device Information Server Task.
*/

#include <string.h>
#include <stdlib.h>

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* Application headers */
#include "sink_gatt_db.h"
#include "sink_gatt_server_dis.h"

#include "sink_ble.h"
#include "sink_debug.h"
#include "sink_gatt_server.h"
#include "sink_config.h"
#include "sink_configmanager.h"

#include "config_definition.h"
#include <config_store.h>
#include <byte_utils.h>

#ifdef GATT_DIS_SERVER
#include "sink_gatt_server_dis_config_def.h"

#ifdef DEBUG_GATT
#define GATT_DEBUG(x) DEBUG(x)
#else
#define GATT_DEBUG(x) 
#endif

static gatt_dis_init_params_t dis_init_params;


/*******************************************************************************/
static bool sinkGattGetDeviceInfoParams(void)
{
    uint16 str_len = 0;
    sink_gatt_server_dis_config_def_t *dis_strings = NULL;

    dis_init_params.dis_strings = (gatt_dis_strings_t*) malloc(sizeof(gatt_dis_strings_t));

    if(dis_init_params.dis_strings != NULL)
    {
        memset(dis_init_params.dis_strings, 0, sizeof(gatt_dis_strings_t));

        dis_init_params.dis_strings->manufacturer_name_string = NULL;
        dis_init_params.dis_strings->model_num_string = NULL;
        dis_init_params.dis_strings->serial_num_string = NULL;
        dis_init_params.dis_strings->hw_revision_string = NULL;
        dis_init_params.dis_strings->fw_revision_string = NULL;
        dis_init_params.dis_strings->sw_revision_string = NULL;

        /* Check for minimum length of the buffer necessary to hold the contents. */
        str_len = configManagerGetReadOnlyConfig(SINK_GATT_SERVER_DIS_CONFIG_BLK_ID, (const void **)&dis_strings);
        /* Allow only first 31 characters of the Manufacturer name information. */
        if(str_len >= GATT_DIS_MAX_MANUF_NAME_LEN)
        {
            /* Extra word is required for storing NULL character at the end of the string. */
            str_len = GATT_DIS_MAX_MANUF_NAME_LEN;
        }

        if (str_len)
        {
            /* Find length packed ASCII string from Config Store. */
            uint16 len_bytes = ByteUtilsGetPackedStringLen((const uint16*)dis_strings->manufacturer_name_string, str_len);

            /* Allocate extra one word for storing NULL character at the end of the string. */
            dis_init_params.dis_strings->manufacturer_name_string = malloc(len_bytes + 1);

            if (dis_init_params.dis_strings->manufacturer_name_string != NULL)
            {
                /* Unpack and assign the Config Store manufacturer string*/
                memset((void*)dis_init_params.dis_strings->manufacturer_name_string, 0, (len_bytes + 1));
                ByteUtilsMemCpyUnpackString((uint8 *)dis_init_params.dis_strings->manufacturer_name_string, (const uint16*)dis_strings->manufacturer_name_string, len_bytes + 1);

                configManagerReleaseConfig(SINK_GATT_SERVER_DIS_CONFIG_BLK_ID);
            }
        }
        return TRUE;
    }

    /* Failed to allocate memory */
    GATT_DEBUG(("GATT Device Info Server failed to allocate memory\n"));
    return FALSE;
}

/*******************************************************************************/
static void sinkGattFreeDisPtrs(void)
{
    /* Free the allocated memories. */
    if(dis_init_params.dis_strings->manufacturer_name_string != NULL)
        free((void*)dis_init_params.dis_strings->manufacturer_name_string);

    if(dis_init_params.dis_strings != NULL)
        free(dis_init_params.dis_strings);
}

/*******************************************************************************/
bool sinkGattDeviceInfoServerInitialise(uint16 **ptr)
{
    if (ptr)
    {
        /* Read the device information service to be initialized */
        if(sinkGattGetDeviceInfoParams())
        {
            if (GattDeviceInfoServerInit(sinkGetBleTask(), (gdiss_t*)*ptr, &dis_init_params,
                                    HANDLE_DEVICE_INFORMATION_SERVICE,
                                    HANDLE_DEVICE_INFORMATION_SERVICE_END))
            {
                GATT_DEBUG(("GATT Device Info Server initialised\n"));
                /* The size of DISS is also calculated and memory is alocated.
                 * So advance the ptr so that the next Server while initializing.
                 * shall not over write the same memory */
               *ptr += ADJ_GATT_STRUCT_OFFSET(gdiss_t);
                return TRUE;
            }

            /* Free the allocated memory */
            sinkGattFreeDisPtrs();
        }
    }
    return FALSE;
}

#endif /* GATT_DIS_SERVER */

bool sinkGattDeviceInfoServiceEnabled(void)
{
#ifdef GATT_DIS_SERVER
    return TRUE;
#else
    return FALSE;
#endif
}
