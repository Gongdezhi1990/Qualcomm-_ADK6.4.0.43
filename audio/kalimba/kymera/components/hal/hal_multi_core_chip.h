/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file hal_multi_core_chip.h
 * \ingroup hal
 *
 * Header for chip specific function to initialize the secondary cores.
 * This file should be included only in "hal_multi_core.c". The actual
 * implementations should be stored into "$CHIP_NAME/hal_$CHIP_NAME.c".
 */

#ifndef HAL_MULTI_CORE_CHIP_H
#define HAL_MULTI_CORE_CHIP_H

extern void hal_multi_core_init_memory(void);

/**
 * \brief Reset Crescendo multi-core registers to P0.
 */
extern void hal_multi_core_reconfigure_memory(void);

extern void hal_multi_core_configure_arbiter_chip(unsigned bank, unsigned core, hal_dm_bank bus);
extern void hal_multi_core_set_dm_bank_write_access_chip(unsigned bank, unsigned core, hal_dm_bank bus, bool allow);
#endif /* HAL_MULTI_CORE_CHIP_H */
