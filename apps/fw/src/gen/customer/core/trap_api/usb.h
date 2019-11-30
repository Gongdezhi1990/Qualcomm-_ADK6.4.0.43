#ifndef __USB_H__
#define __USB_H__
#include <app/usb/usb_if.h>

/*! file  @brief Control of USB EndPoints, and other USB related traps. 
** 
** 
Bluelab applications have access to the on-chip USB interface.
USB devices communications occur through channels known as endpoints.
The USB specification defines a number of USB device classes through
Class Specifications. Each of the Class Specifications define the type
and direction of endpoints required to comply with that particular device
class. Examples include the Human Interface Device (HID) class, and the 
Communication Device Class (CDC).
** 
** 
In addition to the documentation provided here, the USB2.0 specification and
the "Bluetooth and USB Design Considerations" document available from
www.csrsupport.com are useful sources of information.
*/

#if TRAPSET_USB

/**
 *  \brief Determine if USB connection is attached or detached 
 *     Consult the USB battery charging specification available at www.usb.org
 *     for more details on the difference between a dedicated charger and a
 *  standard host.
 *     Note that BlueCore will only report a correct AttachedStatus if
 *  PSKEY_USB_PIO_VBUS
 *     is set correctly and either a PIO or the internal battery charger (if
 *  available) is
 *     connected to the USB VBUS pin.
 *  \return A value to indicate whether the USB connection is currently attached or
 *  detached. If USB charger detection is enabled then, if attached, the value
 *  returned will give information as to the type of device you are attached to
 *  (for example, a standard host/hub or a dedicated charger).
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
usb_attached_status UsbAttachedStatus(void );

/**
 *  \brief Adds a USB interface. See the USB2.0 spec section 9.6.5 
 *         This API is used to register both the USB interface descriptor details
 *         and any class specific descriptors to be returned to the host. 
 *         
 *  \param codes Defines the USB class, sub-class and protocol. 
 *  \param type Defines the class specific descriptor type. 
 *  \param if_descriptor Points to the class specific descriptor. 
 *  \param descriptor_length The length in bytes of if_descriptor.
 *  \return           An interface number on success, else returns usb_interface_error.
 *           Possible reasons for usb_interface_error include insufficient
 *           pmalloc space for new firmware data structures or device already
 *           attached to internal hub with UsbHubAttach().
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
UsbInterface UsbAddInterface(const UsbCodes * codes, uint16 type, const uint8 * if_descriptor, uint16 descriptor_length);

/**
 *  \brief Adds USB endpoints to the interface passed. See the USB2.0 spec section 9.6.6 
 *    
 *  \param interface The interface to add endpoints to (returned by a call to UsbAddInterface()). 
 *  \param num_end_points The number of endpoints to be added. 
 *  \param end_point_info Points to an array of endpoint definitions.
 *  \return TRUE if the endpoints were successfully added, else FALSE. Possible reasons for
 *  a FALSE return value include an invalid "interface" parameter, insufficient
 *  pmalloc space for new firmware data structures, BlueCore is already enumerated
 *  on the bus, PSKEY_HOST_INTERFACE is not set to USB or PSKEY_USB_VM_CONTROL is
 *  FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
bool UsbAddEndPoints(UsbInterface interface, uint16 num_end_points, const EndPointInfo * end_point_info);

/**
 *  \brief Adds a USB descriptor to an interface or endpoint. 
 *    
 *  \param interface The interface to add the descriptor to. 
 *  \param type Descriptor type (lower 8 bits). 
 *  \param descriptor Pointer to the descriptor to add. 
 *  \param descriptor_length The length of the descriptor in bytes.
 *  \return           TRUE if the descriptor was successfully added, else FALSE.
 *           Descriptor type for a class specific endpoint descriptor will contain
 *           the address of the endpoint in the upper 8 bits. The upper 8 bits
 *           should be 0 otherwise. This API is normally used to add HID report or
 *           class specific endpoint descriptors.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
bool UsbAddDescriptor(UsbInterface interface, uint16 type, const uint8 * descriptor, uint16 descriptor_length);

/**
 *  \brief Adds a USB String Descriptor. See the USB2.0 spec section 9.6.7 
 *   The string_descriptor passed is in UTF16 format and MUST be NUL terminated.
 *   The NUL terminator will not be passed to USB host as part of the string
 *   descriptor when a string descriptor request is received, it is simply
 *   to denote the end of the string.
 *   Example:
 *   In order to register string descriptor 11 as "Test".
 *   UsbAddStringDescriptor(11, myStringDescriptor);
 *  \param string_index The USB String Descriptor number. Only index in the range 5 to 255 can be used
 *  as 1 to 4 are reserved. 
 *  \param string_descriptor A pointer to the string descriptor.
 *  \return TRUE if the descriptor was added, else FALSE. A FALSE value will be returned if
 *  an invalid string_index parameter is given.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
bool UsbAddStringDescriptor(uint8 string_index, const uint16 * string_descriptor);

/**
 *  \brief Add an Interface Association Descriptor to a USB interface 
 *  \param if_num The interface number returned by a call to UsbAddInterface() 
 *  \param ia_descriptor A pointer to the Interface Association descriptor in VM address space 
 *  \param descriptor_length The length of the Interface Association descriptor pointed to by ia_descriptor. 
 *  \return           TRUE if the Interface Association descriptor was added correctly,
 *           else FALSE. A FALSE value will be returned if an invalid
 *           if_num is given.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
bool UsbAddInterfaceAssociationDescriptor(uint16 if_num, const uint8 * ia_descriptor, uint16 descriptor_length);

/**
 *  \brief Adds a DFU interface to the USB port.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void UsbAddDfuInterface(void );

/**
 *  \brief Creates an additional USB configuration. 
 *     All devices initially have one configuration (with iConfiguration,
 *     bmAttributes and bMaxPower in the configuration descriptor being provided
 *     by pskeys), and for most devices one configuration is all you want.
 *     However, if desired, this function can be used to create additional
 *     configurations. It will add a new configuration each time it is called,
 *     it can be called repeatedly, but must be called before any interfaces
 *     have been added.
 *     The support for multiple configurations is limited in that the
 *     interfaces/endpoints have to be the same for all configurations, the
 *     only values that can be different are the iConfiguration, bmAttributes and
 *     bMaxPower fields in the configuration descriptor. For the new
 *     configuration, these values are provided by the members of 'desc_info'.
 *     Each member of 'desc_info' can be set to the special value
 *     SAME_AS_FIRST_CONFIG in which case the corresponding entry in the
 *     configuration descriptor for the new configuration will just be copied
 *     over from the first configuration.
 *  \param desc_info Points to info for the new configuration descriptor.
 *  \return           On success, returns a value corresponding to the bConfigurationValue
 *  of
 *           the new configuration. This will start at 2 for the first extra
 *           configuration created. On error, returns zero. Possible reasons for
 *           an error include calling the function after interfaces have been
 *  added,
 *           insufficient pmalloc space for new firmware data structures,
 *           device is already attached with UsbHubAttach.
 *           
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
uint16 UsbAddConfiguration(const ConfigDescriptorInfo * desc_info);

/**
 *  \brief Determine the state of the USB device 
 *         Returns state of USB device implemented by the application.
 *         
 *         This is different from the state of internal USB hub and does not
 *         represent state of connection to USB host.
 *         
 *         For example application can receive MESSAGE_USB_ATTACHED when
 *         attached to USB host but USB device state returned by this trap
 *         can still report "detached" if USB device is not attached to
 *         internal hub with \#UsbHubAttach or is not configured at all.
 *         
 *  \return The current state of the USB device from \#usb_device_state
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
usb_device_state UsbDeviceState(void );

/**
 *  \brief Adds additional alternate USB interface. See the USB2.0 spec section 9.6.5 
 *         After adding interface with UsbAddInterface, this trap can be used to
 *         add additional alternate interfaces.
 *         
 *         The original UsbAddInterface call creates bAlternateSetting=0 without
 *         endpoints. If any isochronous endpoints have been then added
 *         with UsbAddEndPoints() this creates another alternate interface
 *         bAlternateSetting=1 which contains all the endpoints and class interface
 *         descriptors.
 *         
 *         Any additional alternate interfaces can be added with UsbAddAltInterface
 *         and get consecutive bAlternateSetting indices. UsbAddEndPoints and
 *         UsbAddDescriptor can be then used to fill it with endpoints and
 *         class endpoint descriptors.
 *         
 *         Endpoints and endpoint class descriptors are always associate with
 *         the last alternate interface, therefore alternate interfaces are
 *         configured one by one with UsbAddEndPoints and UsbAddDescriptor calls
 *         following corresponding UsbAddAltInterface calls.
 *         
 *         Additional alternate interfaces can have different class descriptors
 *         and endpoint parameters except the following limitation:
 *         all alternate interfaces with non-zero number of endpoints have
 *         the same number of endpoints and corresponding endpoints have
 *         the same bEndpointAddress and bmAttributes. This effectively forbids
 *         creating alternate interfaces with different sets of endpoints,
 *         while still allowing zero-bandwidth alternate interfaces without
 *  endpoints.
 *         
 *         GET_DESCRIPTOR request directed to USB interface to request class
 *         descriptor will be matched against all descriptors added with
 *         UsbAddDescriptor() and also against the "type" specified in the 
 *         UsbAddInterface() call. If the latter matches, then class descriptors of
 *         the currently active alternate interface will be returned.
 *         
 *  \param interface The interface to add alternate interface to (returned by a call to
 *  UsbAddInterface()). 
 *  \param if_descriptor Points to the class specific descriptor. 
 *  \param descriptor_length The length in bytes of if_descriptor.
 *  \return Assigned bAlternateSetting on success, else returns
 *           usb_alt_interface_error. Possible reasons for the error include
 *           insufficient pmalloc space for new firmware data structures or USB
 *           device is already attached to the hub.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
UsbAltInterface UsbAddAltInterface(UsbInterface interface, const uint8 * if_descriptor, uint16 descriptor_length);

/**
 *  \brief Allow or Disallow USB debugging 
 *           This trap can be used to allow or disallow USB debug (still needing
 *  challenge-response), but only under the following conditions:
 *             1. Efuse 'USB Debug Master Disable' is not set
 *             2. Efuse 'USB Debug Allow' is not set
 *         
 *  \param flag Set TRUE to allow USB debugging, FALSE to disallow.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb
 */
void UsbDebugAllow(bool flag);
#endif /* TRAPSET_USB */
#endif
