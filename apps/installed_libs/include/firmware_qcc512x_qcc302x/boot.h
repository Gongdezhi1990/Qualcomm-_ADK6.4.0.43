#ifndef __BOOT_H__
#define __BOOT_H__

/*! file  @brief Control booting BlueCore with different active settings 
** 
** 
These functions control the BlueCore bootmode on unified firmware. Each bootmode has a unique view of the
persistent store, and so a single device could boot up as (for example) an HCI dongle using USB in one bootmode
and a HID dongle using BCSP in another bootmode.
** 
Incorrect use of these functions could produce a non-functional module, and so caution should be exercised. 
** 
BootGetPreservedWord and BootSetPreservedWord can be used to preserve a single word of state between boots.
*/

#if TRAPSET_CORE

/**
 *  \brief Read the current boot mode.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
uint16 BootGetMode(void );

/**
 *  \brief Set the mode and force a reboot (warm reset) of the chip using the boot image
 *  bank. 
 *  \param newBootmode The new boot mode to use.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
void BootSetMode(uint16 newBootmode);

/**
 *  \brief Read the (single) word preserved between boots.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint16 BootGetPreservedWord(void );

/**
 *  \brief Write the (single) word preserved between boots. 
 *  \param state The word of state to preserve.
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void BootSetPreservedWord(uint16 state);
#endif /* TRAPSET_CORE */
#endif
