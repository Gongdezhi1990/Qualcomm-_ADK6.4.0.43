/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#ifndef ID_H
#define ID_H

#include "hydra/hydra_types.h"

/* Generated at build time by id.pl then passed to id_string_filegen.py */
extern const uint32 build_id_number;
extern const uint8 build_id_string[];

/**
 * Get the fw version as a uint32
 */
extern uint32 patched_fw_version;

/**
 * Get the build identifier as a uint32
 *
 * Each firmware build has a unique build ID.  This function returns
 * that number.
 */
extern uint32 get_build_id_number(void);

/**
 * Get the lowest four decimal digits of the patched buildID as a binary-coded
 * decimal uint16.
 * This is used by the USB (which enumerates after patching).
 *
 */
extern uint16 id_get_build_bcd(void);

/**
 * Get the (possibly updated) fw version as a uint32
 *
 */
extern uint32 id_get_patched_fw_version(void);

extern void id_set_new_patched_fw_version(uint32 patched_fw_version);

/* Inlined version of simple functions */
#define get_build_id_number() (build_id_number)
#define id_get_patched_fw_version() (patched_fw_version)
#define id_set_new_patched_fw_version(new) (patched_fw_version = new)

#endif /* ID_H */
