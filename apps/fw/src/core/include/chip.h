/* Copyright (c) 2019 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * /file
 *
 * Provide macros to allow the chip model and variant to be read at runtime.
 */

#ifndef _CHIP_H_
#define _CHIP_H_

/*
 * Define some macros that allow run-time determination of the chip.
 */
#define CHIPVSN_MAJOR  (0x00ff)
#define CHIPVSN_VARIANT  (0x0f00)
#define CHIPVSN_MINOR  (0xf000)

/* Main chips */
#define ISCHIP_CRESCENDO    (0x0046)
#define ISCHIP_CRESCENDO_MASK   CHIPVSN_MAJOR
#define ISCHIP_AURA         (0x0049)
#define ISCHIP_AURA_MASK        CHIPVSN_MAJOR
#define ISCHIP_AURAPLUS     (0x004b)
#define ISCHIP_AURAPLUS_MASK    CHIPVSN_MAJOR

/* Variant chips */
#define ISCHIP_CRESCENDOA04 (0x3046)
#define ISCHIP_CRESCENDOA04_MASK    (CHIPVSN_MAJOR|CHIPVSN_VARIANT|CHIPVSN_MINOR)
#define ISCHIP_CRESCENDOA05 (0x3146)
#define ISCHIP_CRESCENDOA05_MASK    (CHIPVSN_MAJOR|CHIPVSN_VARIANT|CHIPVSN_MINOR)
#define ISCHIP_AURA2P0      (0x2049)
#define ISCHIP_AURA2P0_MASK     (CHIPVSN_MAJOR|CHIPVSN_VARIANT|CHIPVSN_MINOR)
#define ISCHIP_AURA2P1      (0x2149)
#define ISCHIP_AURA2P1_MASK     (CHIPVSN_MAJOR|CHIPVSN_VARIANT|CHIPVSN_MINOR)

/* Accessor functions */
#define IsChip(x) ((hal_get_reg_sub_sys_chip_version() & ISCHIP_##x##_MASK) == ISCHIP_##x)

#define GetChip(x) (hal_get_reg_sub_sys_chip_version())

#endif /* _CHIP_H_ */
