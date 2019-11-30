/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      A device instance that represents a collection of profiles/services/etc.

    A device will usually be a connected remote device connected to the local
    device, but it could also be used to store the properties of the local device.
*/

#include <string.h>

#include <panic.h>

#include <device.h>
#include <key_value_list.h>


struct device_tag
{
    key_value_list_t properties;
};


/*****************************************************************************/

device_t Device_Create(uint16 max_properties)
{
    struct device_tag * device = 0;

    device = PanicUnlessMalloc(sizeof(*device));
    memset(device, 0, sizeof(*device));

    device->properties = KeyValueList_Create(max_properties);

    return device;
}

void Device_Destroy(device_t device)
{
    PanicNull(device);
    KeyValueList_Destroy(device->properties);
    free(device);
}

bool Device_IsPropertySet(device_t device, device_property_t id)
{
    PanicNull(device);
    return KeyValueList_IsSet(device->properties, id);
}

void Device_RemoveProperty(device_t device, device_property_t id)
{
    PanicNull(device);
    KeyValueList_Remove(device->properties, id);
}

bool Device_SetProperty(device_t device, device_property_t id, void *value, size_t size)
{
    PanicNull(device);
    return KeyValueList_Add(device->properties, id, value, size);
}

bool Device_GetProperty(device_t device, device_property_t id, void **value, size_t *size)
{
    PanicNull(device);
    return KeyValueList_Get(device->properties, id, value, size);
}

bool Device_SetPropertyPtr(device_t device, device_property_t id, void *value)
{
    PanicNull(device);
    return KeyValueList_Add(device->properties, id, &value, sizeof(value));
}

void *Device_GetPropertyPtr(device_t device, device_property_t id)
{
    void *value = NULL;
    size_t size = sizeof(value);

    PanicNull(device);

    return KeyValueList_Get(device->properties, id, &value, &size) ? *(void **)value : 0;
}

bool Device_SetPropertyU32(device_t device, device_property_t id, uint32 value)
{
    return KeyValueList_Add(device->properties, id, &value, sizeof(value));
}

bool Device_GetPropertyU32(device_t device, device_property_t id, uint32 *value)
{
    bool found = FALSE;
    uint32 *u32 = NULL;
    size_t size = sizeof(*u32);

    PanicNull(device);

    if (KeyValueList_Get(device->properties, id, (void **)&u32, &size))
    {
        *value = *u32;
        found = TRUE;
    }

    return found;
}

bool Device_SetPropertyU16(device_t device, device_property_t id, uint16 value)
{
    PanicNull(device);
    return KeyValueList_Add(device->properties, id, &value, sizeof(value));
}

bool Device_GetPropertyU16(device_t device, device_property_t id, uint16 *value)
{
    bool found = FALSE;
    uint16 *u16 = NULL;
    size_t size = sizeof(*u16);

    PanicNull(device);

    if (KeyValueList_Get(device->properties, id, (void **)&u16, &size))
    {
        *value = *u16;
        found = TRUE;
    }

    return found;
}
