/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra Platform Interface
*/

#ifndef HYDRA_H
#define HYDRA_H

/*
 * Interface dependencies.
 */
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_CHIP
/*lint -e750 -e962*/ #define IO_DEFS_MODULE_APPS_SYS_SYS
#include "io/io.h" /* for system_bus enum */

/*
 * hal_macros.h is included for both chip and host builds.
 * Do not redefine macros for host builds here, instead do it
 * within include_host/hal_macros_divert.h
 */
#include "hal/hal_macros.h"

#define CURATOR_SSID                    (SYSTEM_BUS_CURATOR)

/*****************************************************************************
 * Hydra Subsystem ID.
 ****************************************************************************/

/**
 * Hydra Subsystem ID.
 *
 * Used for addressing MMU buffers and inter-subsystem messages over the
 * transaction bus.
 *
 * \todo     The system_bus enum in io.h fits the bill but has an obscure name.
 * Persuade hw to change the system_bus name and remove this typedef.
 */
typedef system_bus hydra_ssid;

/**
 * Is this a valid subsystem id on this system?
 */
#define hydra_ssid_is_valid(ssid) (SYSTEM_BUS_NUM_SUBSYSTEMS > (ssid))

/**
 * The local Hydra Subsystem ID.
 *
 * Use hydra_ssid_is_local(some_ssid) to test.
 */
#define hydra_get_local_ssid() ((hydra_ssid)hal_get_reg_sub_sys_id())

/**
 * Is this the local Hydra Subsystem ID?
 */
#define hydra_ssid_is_local(ssid) ((ssid) == hydra_get_local_ssid())

/**
 * The given address is on the same chip as this subsystem
 */
#define hydra_ssid_is_on_chip(ssid) (SYSTEM_BUS_NUM_SUBSYSTEMS > (ssid))

/**
 * Convert a subsystem address that is used by a peer subsystem into an
 * address that a this subsystem can use. This takes account of the peer being
 * off-chip and hence having a different view of subsystem numbering. If the
 * \c peer_curator_ssid is zero (peer is on the same chip as us) then this
 * macro does nothing.
 * \param ssid The ssid that is to be translated
 * \param peer_curator_ssid The address that this subsystem uses to address
 * the curator in the chip where the peer subsystem is.
 * \returns The translated ssid
 */
#define HYDRA_TRANSLATE_PEER_SSID(ssid, peer_curator_ssid)              \
            (hydra_ssid)(((uint16)(ssid) + (uint16)(peer_curator_ssid)) & 0xf)

/**
 * Convert a subsystem address that is used by this subsystem into an
 * address that a peer subsystem can use. This takes account of the peer being
 * off-chip and hence having a different view of subsystem numbering. If the
 * \c peer_curator_ssid is zero (peer is on the same chip as us) then this
 * macro does nothing.
 * This macro is not expected to be widely used. One example is where an
 * address from a peer was translated on reception and stored locally
 * but then needs to be used in an indication to that peer. This happens in
 * the \c hostio_tbus transports.
 * \param ssid The ssid that is to be translated
 * \param peer_curator_ssid The address that this subsystem uses to address
 * the curator in the chip where the peer subsystem is.
 * \returns The translated ssid
 */
#define HYDRA_TRANSLATE_SSID_FOR_PEER(ssid, peer_curator_ssid)          \
        (hydra_ssid)(((uint16)(ssid) - (uint16)(peer_curator_ssid)) & 0xf)

/** Convert an mmu handle that a peer subsystem has supplied into one that
 * this subsystem can use to access it. This takes account of the peer being
 * off-chip and hence having a different view of subsystem numbering. If the
 * \c peer_curator_ssid is zero (peer is on the same chip as us) then this
 * macro does nothing.
 * \param h The MMU handle to be translated (modified)
 * \param peer_curator_ssid The address that this subsystem uses to address
 * the curator in the chip where the peer subsystem is.
 */
#define HYDRA_TRANSLATE_PEER_MMU_H(h, peer_curator_ssid)                \
        mmu_handle_init(&h,                                             \
                HYDRA_TRANSLATE_PEER_SSID(mmu_handle_ssid(h),           \
                                            (peer_curator_ssid)),       \
                mmu_handle_index(h))

/** Convert an mmu handle that we use to address an MMU in a peer subsystem
 * into one that it will understand. This takes account of the peer being
 * off-chip and hence having a different view of subsystem numbering. If the
 * \c peer_curator_ssid is zero (peer is on the same chip as us) then this
 * macro does nothing.
 * This macro is not expected to be widely used. One example is where a
 * handle of an MMU in a peer was translated on reception and stored locally
 * but then needs to be used in an indication to that peer. This happens in
 * the \c hostio_tbus transports.
 *
 * \param h The MMU handle to be translated (modified)
 * \param peer_curator_ssid The address that this subsystem uses to address
 * the curator in the chip where the peer subsystem is.
 */
#define HYDRA_TRANSLATE_MMU_H_FOR_PEER_SS(h, peer_curator_ssid)         \
        mmu_handle_init(&h,                                             \
                HYDRA_TRANSLATE_SSID_FOR_PEER(mmu_handle_ssid(h),       \
                                            (peer_curator_ssid)),       \
                mmu_handle_index(h))

/*****************************************************************************
 * Subsidiary Interfaces
 ****************************************************************************/

/*#include "hydra/hydra_txbus.h"*/

#endif /* HYDRA_H */
