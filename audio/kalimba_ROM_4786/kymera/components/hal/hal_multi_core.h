/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hal_multi_core.h
 * \ingroup hal
 *
 * Header for functions shared between chips used to initialize the
 * secondary cores.
 */

#ifndef _HAL_MULTI_CORE_H_
#define _HAL_MULTI_CORE_H_

/****************************************************************************
Public Constant and macros
*/
#define HAL_MAX_NUM_CORES 2

/* Minimum number of cores to enable dual core feature */
#define HAL_MIN_NUM_CORES 2

#ifdef CHIP_BASE_NAPIER
#define hal_set_reg_enable_private_ram(n) hal_set_reg_enable_fast_private_ram(n)
#endif /* CHIP_BASE_NAPIER */

/****************************************************************************
Public types
*/

typedef enum
{
    DM1 = 0,
    DM2 = 1
} hal_dm_bank;

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Initialise the multi-core specific hardware registers
 *        hal_init() must be called before calling this API and it
 *        is required to be called if more than one core is supported.
 *        This must be called by P0 only.
 *
 * \params num_cores  - Number of cores to be enabled.
 *                      currently it must be always 2.
 */
extern void hal_multi_core_init(uint8 num_cores);

/**
 * \brief Resets multi-core specific hardware registers back to single-core
 *        defaults.
 */
extern void hal_multi_core_disable(void);

/**
 * \brief Return the number of active cores
 *
 *
 * \return num_cores : Number of active cores
 */
extern uint8 hal_get_active_num_cores(void);

/**
 * \brief  Sets the number of active core. Used when cores are disabled after
 *         boot. This can only be called by P0.
 *
 * \param  num_cores Number of active cores
 * \return           TRUE if successful and FALSE
 */
extern bool hal_set_active_num_cores(uint8 num_cores);

extern void hal_multi_core_configure_arbiter(unsigned bank, unsigned core, hal_dm_bank bus);

extern void hal_multi_core_set_dm_bank_write_access(unsigned bank, unsigned core, hal_dm_bank bus, bool allow);

#endif /* _HAL_MULTI_CORE_H_ */
