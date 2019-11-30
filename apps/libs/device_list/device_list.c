/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      A collection of device objects that can be retrieved by property type and value.
*/

#include <string.h>

#include <device_list.h>


#define DEVICE_LIST_MAX_NUM_DEVICES 3

static device_t device_list[DEVICE_LIST_MAX_NUM_DEVICES];


void DeviceList_Init(void)
{
    memset(&device_list, 0, sizeof(device_list));
}

unsigned DeviceList_GetNumOfDevices(void)
{
    int i;
    unsigned count = 0;

    for (i = 0; i < DEVICE_LIST_MAX_NUM_DEVICES; i++)
    {
        if (device_list[i])
            count++;
    }

    return count;
}

void DeviceList_RemoveAllDevices(void)
{
    int i;

    for (i = 0; i < DEVICE_LIST_MAX_NUM_DEVICES; i++)
    {
        if (device_list[i])
        {
            /* Should the device be destroyed by this function? */
            device_list[i] = 0;
        }
    }
}

bool DeviceList_AddDevice(device_t device)
{
    int i;
    bool added = FALSE;

    for (i = 0; i < DEVICE_LIST_MAX_NUM_DEVICES; i++)
    {
        if (device_list[i] == 0)
        {
            device_list[i] = device;
            added = TRUE;
            break;
        }
        else if (device_list[i] == device)
        {
            added = FALSE;
            break;
        }
    }

    return added;
}

void DeviceList_RemoveDevice(device_t device)
{
    int i;

    for (i = 0; i < DEVICE_LIST_MAX_NUM_DEVICES; i++)
    {
        if (device_list[i] == device)
        {
            device_list[i] = 0;
            /* Should the device be destroyed by this function? */
            break;
        }
    }
}

device_t DeviceList_GetDeviceForPropertyValue(device_property_t id, void *value, size_t size)
{
    int i;
    device_t found_device = 0;

    for (i = 0; i < DEVICE_LIST_MAX_NUM_DEVICES; i++)
    {
        if (device_list[i])
        {
            void *property;
            size_t property_size;

            if (Device_GetProperty(device_list[i], id, &property, &property_size))
            {
                if ((size == property_size)
                    && !memcmp(value, property, size))
                {
                    found_device = device_list[i];
                    break;
                }
            }
        }
    }

    return found_device;
}
