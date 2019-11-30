#ifndef __DORMANT_H__
#define __DORMANT_H__
#include <app/dormant/dormant_if.h>



#if TRAPSET_CORE

/**
 *  \brief Configures the dormant mode. All the configurable settings are defined in
 *  \#dormant_config_key.
 *  \param key Which aspect of dormant to configure. 
 *  \param value Which value to use. 
 *  \return TRUE if the key is valid and the input value corresponding to the key is valid,
 *  else FALSE.
 *                        Please note that if the request to go into dormant mode
 *  is successful the trap will never return.
 * 
 * \note This trap may NOT be called from a high-priority task handler
 * 
 * \ingroup trapset_core
 */
bool DormantConfigure(dormant_config_key key, uint32 value);
#endif /* TRAPSET_CORE */
#endif
