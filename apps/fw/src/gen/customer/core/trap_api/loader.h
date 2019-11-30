#ifndef __LOADER_H__
#define __LOADER_H__
/** \file */
#if TRAPSET_CORE

/**
 *  \brief This API will reboot the firmware in "loader" mode. This trap can be used to
 *  trigger a DFU process based on an external event, for example: " A combination
 *  of keys being pressed at the device".
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void LoaderModeEnter(void );

/**
 *  \brief Reboots the firmware in "loader" mode to perform DFU from SQIF operation
 *   This trap can be used to trigger the DFU process from SQIF after the
 *   VM application writes the DFU file onto one of the serial flash
 *   partitions. Partition type should be a raw partition type.
 *   Once the DFU process is over and the device reboots back into "stack" mode,
 *   the VM application's system task then receives MESSAGE_DFU_SQIF_STATUS message
 *   containing the status of DFU operation.
 *   If there is a power loss during the DFU process, the VM application will
 *   not receive the MESSAGE_DFU_SQIF_STATUS message.
 *   \note
 *   This trap will only work if the loader supports the DFU process from SQIF.
 *   If the "loader" does not have the knowledge of the DFU process from SQIF, then
 *   this trap will reboot the device into "loader" mode and the loader will
 *   wait for wired DFU command from the host.
 *   \note
 *   This trap performs basic validation of the partition number and the DFU
 *   file and reboots the device into the "loader" mode. Hence, the control
 *   does not comes back to the VM application. If the device is not rebooted into
 *   the "loader" mode due to validation failure, then the controls returns back
 *   to the VM application and appropriate action should be taken by the
 *   VM application.
 *   \note
 *   A value of 0xFFFD, 0xFFFE and 0xFFFF MUST not be used for partition
 *   number since these values are reserved for internal use. If these
 *   values are passed, then trap will return to the VM application
 *   without any impact.
 *  \param partition A valid SQIF partition containing the DFU file
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void LoaderPerformDfuFromSqif(uint16 partition);
#endif /* TRAPSET_CORE */
#endif
