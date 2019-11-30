#ifndef __NATIVE_H__
#define __NATIVE_H__

/*! file  @brief Traps specifically for native execution */

#if TRAPSET_NATIVE

/**
 *  \brief Pause the VM ready to start native debugging (if appropriate)
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_native
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
void NativePauseForDebug(void );
#endif /* TRAPSET_NATIVE */
#endif
