#ifndef __SRAM_H__
#define __SRAM_H__

/*! file  @brief Traps to map SRAM into VM memory space */

#if TRAPSET_SRAM

/**
 *  \brief Maps external SRAM into the VM memory space
 *   
 *   At the first invocation this finds a sufficient window in the VM's
 *   data address space and makes the requested portion of external SRAM
 *   visible; the application can then access external SRAM through the
 *   returned pointer (see below for the limitations). Subsequent
 *   invocations can map in different portions of SRAM, at which point
 *   any existing pointers into the old portion become invalid. (In
 *   general, accesses through such pointers cannot be trapped by the VM,
 *   so may cause subtly wrong behaviour rather than panicking the
 *   application).
 *   There are restrictions on the use of external SRAM mapped in this
 *   way. The application may read and write to the SRAM directly, but
 *   pointers into it may not be passed into most traps; if an attempt is
 *   made to do so, the application will be stopped. Only a few traps
 *   fully support operations on pointers into SRAM -- currently memset,
 *   memmove, and Util* traps.
 *   If the requested size is greater than the maximum supported, or
 *   mapping fails for some other reason, a NULL pointer is returned.
 *   To use external SRAM, the BlueCore device and firmware must both
 *   support it, and the application circuit must connect up the SRAM
 *   appropriately, including a PIO to select it (which will be
 *   unavailable for any other use). PSKEY_SRAM_PIO must be set to
 *   indicate this PIO to the firmware; by default the PSKEY is not set,
 *   and if not set, SramMap() will always return NULL. Please refer to
 *   appropriate product data sheet for the PIO value.
 *   External SRAM is only supported in certain BlueCore firmware.
 *   Please refer the appropriate product data sheet for external SRAM
 *   support and refer CS-307510-AN and CS-324244-AN for usage and other
 *   details.
 *   Example Usage:
 *         ptr = PanicNull(SramMap(...));
 *  \param sram_addr External SRAM physical address to map 
 *  \param size Size (in uint16s) of SRAM to map
 *  \return Pointer to start of mapped SRAM, NULL if cannot map
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_sram
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
uint16 * SramMap(uint32 sram_addr, uint16 size);
#endif /* TRAPSET_SRAM */
#endif
