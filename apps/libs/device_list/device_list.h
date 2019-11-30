/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      A collection of device objects that can be retrieved by property type and value.
*/
#ifndef DEVICE_LIST_H_
#define DEVICE_LIST_H_

#include <device.h>


/*! \brief Initialise the device list module. */
void DeviceList_Init(void);

/*! \brief Get the number of devices in the list.

    \return The number of devices currently in the list.
*/
unsigned DeviceList_GetNumOfDevices(void);

/*! \brief Add a device to the list.

    If the device list is full or memory could not be allocated to add the
    #device this function will fail and return FALSE.

    \param device The device to add to the list.
i
    \return TRUE if device was added, FALSE if the add failed for any reason.
*/
bool DeviceList_AddDevice(device_t device);

/*! \brief Remove a device from the list.

    The device is only removed from the list - it is not destroyed. The owner
    of the device must destroy it when it is no longer needed.

    If the device is not in the list or is not valid this function will not
    alter the list and will not panic.

    \param device Device to remove from the list.
*/
void DeviceList_RemoveDevice(device_t device);

/*! \brief Remove all devices from the list.

    The devices are only removed from the list - they are not destroyed.
    The owner(s) of the device(s) must destroy them when no longer needed.
*/
void DeviceList_RemoveAllDevices(void);

/*! \brief Find a device with a matching property value.

    The device will only be found if it contains the given property and the
    value of the property matches the #size bytes pointed to by #value.

    The caller is expected to know the correct size and format of the property
    being searched for.

    Examples of types of property search:

    Searching for a uint32 property
    \code
    uint32 value = 0x42;

    device = DeviceList_GetDeviceForKeyValue(property_id, &value, sizeof(value));
    \endcode

    Searching for a bdaddr property
    \code
    bdaddr value = { 0xFF0F, 0xFF, 0xEF };

    device = DeviceList_GetDeviceForKeyValue(property_id, &value, sizeof(value));
    \endcode

    Searching for a pointer property
    \code
    a_struct *value = &my_struct;

    device = DeviceList_GetDeviceForKeyValue(property_id, &value, sizeof(value));
    \endcode

    \param id Property to match on a device.
    \param value Pointer to the value of the property to match.
    \param size Size of the property value.

    \return Handle to the first device that matches, or 0 if no match.
*/
device_t DeviceList_GetDeviceForPropertyValue(device_property_t id, void *value, size_t size);

#endif // DEVICE_LIST_H_
