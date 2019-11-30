#ifndef __PSU_H__
#define __PSU_H__
#include <app/psu/psu_if.h>

/*! file  @brief Traps specifically for power supply control */

#if TRAPSET_PSU

/**
 *  \brief Returns TRUE if VREG_EN(_H) is raised.
 *   This is only supported on BlueCore variants with charger hardware. 
 *   On BlueCore variants prior to BC5 this must be polled if we wish to 
 *   detect if the level of VREG_EN(_H) has changed. On BlueCore variants 
 *   from BC5 onwards changes in the VREG_EN(_H) level can generate events;
 *   See MessageChargerTask documentation for details.
 *   (Typically an ON/OFF button is wired to VREG_EN.  When the button is
 *   pressed, VREG_EN goes high, the chip powers up, and the firmware
 *   latches the SMPSU regulator as part of the early boot sequence.
 *   VREG_EN can then be removed. To turn off the chip using the same
 *   button we need to have visibility of VREG_EN.)
 * 
 * \note This trap may be called from a high-priority task handler
 * 
 * \ingroup trapset_psu
 */
bool PsuGetVregEn(void );

/**
 *  \brief Configures PSU regulators 
 *     
 *     Various BlueCore models have different numbers and types of power supplies/
 *  regulators.
 *     This means the actual power supply controlled by any psu_id may change
 *  between different
 *     BlueCore models. Consult the datasheet for your BlueCore variant for more
 *  information.
 *  \param psu Which power supply to reconfigure 
 *  \param key Which aspect of the PSU to configure 
 *  \param value Which value to use
 *  \return TRUE if the psu_id, the key and the input value corresponding to the given key
 *  are all valid, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_psu
 */
bool PsuConfigure(psu_id psu, psu_config_key key, uint16 value);
#endif /* TRAPSET_PSU */
#endif
