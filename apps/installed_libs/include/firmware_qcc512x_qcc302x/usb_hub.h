#ifndef __USB_HUB_H__
#define __USB_HUB_H__
#include <app/usb/usb_if.h>
#include <app/usb/usb_hub_if.h>

/*! file  @brief Control of internal USB hub. 
** 
**
The new USB device architecture includes an internal USB hub, these
traps allow some control over it, like attaching/detaching device
to/from the hub.
*/

#if TRAPSET_USB_HUB

/**
 *  \brief Attach USB device to the hub
 *     Device freezes all descriptors and starts attachment to the host by
 *     attaching to the hub. Changes to the descriptors are no longer allowed
 *     and correspondent traps will fail.
 *         
 *  \return FALSE if something has gone wrong.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb_hub
 */
bool UsbHubAttach(void );

/**
 *  \brief Detach USB device from the hub
 *     Detaches USB device from the hub and hence from the host so that
 *     USB descriptors can be modified.
 *         
 *  \return FALSE if something has gone wrong.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb_hub
 */
bool UsbHubDetach(void );

/**
 *  \brief Configure USB device
 *         
 *   Re-configure USB device using supplied parameters. All parameters must
 *   be supplied at the same time and only those that have corresponding bits
 *   set in device_params->ValidParams are effective. Those that do not have
 *   these bits set will be taken from the MIB keys.
 *   
 *   Any descriptors previously added will be destroyed and configuration
 *   completely cleared. Application can then use USB device traps to
 *   add all interfaces and endpoints again.
 *   
 *   The trap can be called with device_params=NULL just to clear current
 *   configuration and start configuring a new device.
 *   
 *   This trap can only be called while not attached.
 *   Usually it is used as follows:
 *   
 *   1. Device detached with UsbHubDetach().
 *   2. Configuration cleared and parameters set with UsbHubConfigure().
 *   3. New configuration set with UsbAddInterface(), UsbAddEndPoints(), 
 *   UsbAddDescriptor(), etc.
 *   4. Device attached with UsbHubAttach().   
 *         
 *  \param device_params Pointer to a structure with USB device parameters
 *             (can be de-allocated after the trap call). If "NULL" is supplied,
 *             then parameters are taken from MIB keys.
 *             
 *  \return TRUE if finished successfully,
 *           FALSE if something has gone wrong.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb_hub
 */
bool UsbHubConfigure(const usb_device_parameters * device_params);

/**
 *  \brief Configure USB device parameters
 *   
 *   Change USB device configuration parameters at runtime without affecting
 *   descriptors and endpoints, unlike UsbHubConfigure() existing configuration
 *   is not cleared.
 *   
 *   The trap can only be used while device is not attached.
 *   Device can be detached with UsbHubDetach(), parameters like
 *   VID, PID, MaxPower, etc changed with UsbHubConfigKey() and then 
 *   device attached again with UsbHubAttach().
 *   
 *   Special value \#USB_DEVICE_CFG_FROM_MIB can be used to unset
 *   a parameter in which case its value is taken from the MIB keys again.
 *   
 *   Where parameter is uint8 array, e.g. \#USB_DEVICE_CFG_DEVICE_CLASS_CODES,
 *   little-ending packing is expected, e.g.:
 *   
 *       uint8 data[3];
 *       uint32 value = data[0] | (data[1] << 8) | (data[2] << 16);
 *       result = UsbHubConfigKey(USB_DEVICE_CFG_DEVICE_CLASS_CODES, value);
 *         
 *  \param key Configuration key id.
 *             
 *  \param value Configuration key value. Special value
 *             \#USB_DEVICE_CFG_FROM_MIB can be used to unset a parameter
 *             in which case its value is taken from the MIB keys again.
 *             
 *  \return TRUE if finished successfully.
 *           Can return FALSE if called while device is attached or if non-existing
 *           key is supplied.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_usb_hub
 */
bool UsbHubConfigKey(usb_device_cfg_key key, uint32 value);
#endif /* TRAPSET_USB_HUB */
#endif
