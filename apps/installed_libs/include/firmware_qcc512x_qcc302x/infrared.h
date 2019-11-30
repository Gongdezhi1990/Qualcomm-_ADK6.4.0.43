#ifndef __INFRARED_H__
#define __INFRARED_H__
#include <app/infrared/infrared_if.h>

/*! file  @brief Traps to control the Infrared receiver */

#if TRAPSET_INFRARED

/**
 *  \brief Control the infrared receiver 
 *     More detailed information on the keys and values can be found in
 *  irfrared_if.h
 *  \param key Which Infrared parameter to configure Parameters are defined in
 *  \#infrared_config_key 
 *  \param value The value the parameter specified by "key" should be set to 
 *  \return TRUE if the key is valid and the input value is in valid range, else FALSE.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_infrared
 * 
 * WARNING: This trap is UNIMPLEMENTED
 */
bool InfraredConfigure(infrared_config_key key, uint32 value);
#endif /* TRAPSET_INFRARED */
#endif
