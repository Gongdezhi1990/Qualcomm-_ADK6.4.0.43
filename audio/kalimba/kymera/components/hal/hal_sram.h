/****************************************************************************
 * Copyright 2018 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hal_sram.h
 * \ingroup hal
 *
 * Header for functions to enable SRAM support
 * secondary cores.
 */

#ifndef _HAL_SRAM_H_
#define _HAL_SRAM_H_

#include "types.h"

/****************************************************************************
Public Constant and macros
*/

#define hal_sram_is_configured() (hal_get_sqif_conf2_ram_en() != 0)  


/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Call this API to record the  SRAM status while enabling or disabling 
 * SRAM.
 *
 * \param enable: TRUE to enable
 *
 * \return TRUE if SRAM is enabled
 */
extern bool hal_set_sram_enabled(bool enable);


/**
 * \brief Get the sram state
 *
 * \return TRUE if sram is enabled. 
 */
extern bool hal_get_sram_enabled(void);

/**
 * \brief Get the sram state. Call this function to check
 * whether any processor is using SRAM before shutting down.
 *
 * \return TRUE if sram is in use by any processor
 */
extern bool hal_get_sram_in_use(void);


#endif /*_HAL_SRAM_H_*/
