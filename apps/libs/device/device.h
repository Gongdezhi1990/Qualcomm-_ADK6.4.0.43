/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\brief      A device instance that represents a collection of profiles/services/etc.

    A device will usually be a connected remote device connected to the local
    device, but it could also be used to store the properties of the local device.
*/
#ifndef DEVICE_H_
#define DEVICE_H_


/*! \brief Opaque handle to a device instance. */
typedef struct device_tag * device_t;

/*! \brief A device property is represented by a 16bit unsigned integer. */
typedef uint16 device_property_t;

/*! \brief All device properties must be less than this value. */
#define DEVICE_PROPERTY_INVALID 0xFFFF /* UINT16_MAX */

/*! \brief Create a new device_t object.

    If allocating memory for the new object fails this function will panic.

    \param max_properties Maximum number of properties the device can hold.

    \return Handle to a new device object.
*/
device_t Device_Create(uint16 max_properties);

/*! \brief Destroy a device_t object.

    Destroy and free any memory used by a device_t and its properties.

    \param device Device onbject to destroy.
*/
void Device_Destroy(device_t device);

/*! \brief Check if the given property is set for a device.

    \param device Device to search for the property.
    \param id Property to check.

    \return TRUE if the property is set; FALSE otherwise.
*/
bool Device_IsPropertySet(device_t device, device_property_t id);

/*! \brief Remove a property from a device.

    Removes the given property from the #device and frees any memory owned
    by the property.

    If the property does not exist in the device this function does nothing.

    \param device Device to remove the property from.
    \param id Property to be removed.
*/
void Device_RemoveProperty(device_t device, device_property_t id);

/*! \brief Set a property for a device by copying the data it contains.

    When the property is set, it is copied from #value to a buffer owned by
    the property.

    This function will fail if the property already exists or the device
    cannot hold another property.

    If size is too large or there is not enough memory to allocate a buffer to
    copy the data into this function will panic.

    \param device Device to set the property for.
    \param id Property to set.
    \param value Pointer to the location to copy the data from.
    \param size Size of the data in bytes pointed to by #value

    \return TRUE if the property was set ok; FALSE otherwise.
*/
bool Device_SetProperty(device_t device, device_property_t id, void *value, size_t size);

/*! \brief Retrieve a pointer to a property value.

    The caller of this function must know in advance the type of the property
    being retreived.

    If the property does not exist in the device #value and #size will not be
    modified.

    \param device Device to get the property for.
    \param id Property to get.
    \param[out] value The pointer to the property data is written into this.
    \param[out] size The size of the property data is written into this.

    \return TRUE if the property exists, FALSE otherwise.
*/
bool Device_GetProperty(device_t device, device_property_t id, void **value, size_t *size);

/*! \brief Set a pointer property for a device.

    When the property is set only the pointer value is copied. The device_t
    does not take ownership of the pointer.

    The calling code is responsible for managing the memory the pointer
    refers to.

    \param device Device to set the property for.
    \param id Property to set
    \param value Pointer to store in the property.

    \return TRUE if the property was set ok; FALSE otherwise.
*/
bool Device_SetPropertyPtr(device_t device, device_property_t id, void *value);

/*! \brief Get a pointer property for a device.

    \param device Device to get the property for.
    \param id Property to get.

    \return The stored pointer if the property exists, 0 otherwise.
*/
void *Device_GetPropertyPtr(device_t device, device_property_t id);

/*! \brief Set a uint32 property for a device.

    When the property is set #value is copied into the property.

    \param device Device to set the property for.
    \param id Property to set
    \param value uint32 to store in the property.

    \return TRUE if the property was set ok; FALSE otherwise.
*/
bool Device_SetPropertyU32(device_t device, device_property_t id, uint32 value);

/*! \brief Get a uint32 property for a device.

    If the property does not exist this function will return FALSE and the
    memory pointed to by #value will not be modified.

    \param device Device to get the property for.
    \param id Property to get.
    \param value [out] Pointer to the uint32 where the value will be written to.

    \return TRUE if the property exists, FALSE otherwise.
*/
bool Device_GetPropertyU32(device_t device, device_property_t id, uint32 *value);

/*! \brief Set a uint16 property for a device.

    When the property is set #value is copied into the property.

    \param device Device to set the property for.
    \param id Property to set
    \param value uint16 to store in the property.

    \return TRUE if the property was set ok; FALSE otherwise.
*/
bool Device_SetPropertyU16(device_t device, device_property_t id, uint16 value);

/*! \brief Get a uint16 property for a device.

    If the property does not exist this function will return FALSE and the
    memory pointed to by #value will not be modified.

    \param device Device to get the property for.
    \param id Property to get.
    \param value [out] Pointer to the uint16 where the value will be written to.

    \return TRUE if the property exists, FALSE otherwise.
*/
bool Device_GetPropertyU16(device_t device, device_property_t id, uint16 *value);


#endif // DEVICE_H_
