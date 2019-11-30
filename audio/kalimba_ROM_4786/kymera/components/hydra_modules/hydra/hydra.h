/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file 
 * Hydra Platform Interface
 *
 * \defgroup hydra hydra
 * \ingroup hydra_includes
 * \section hydra_includes_overview Overview
 *
 * This is a bucket for Hydra features that do not have a natural
 * home in any particular subsystem.
 *
 * It defines the type \c hydra_ssid for subsystem addresses along with
 * macros for handling them - \c hydra_ssid_is_valid(),
 * \c hydra_get_local_ssid(), \c hydra_ssid_is_local(),
 * \c hydra_ssid_is_on_chip().
 *
 * The are also macros for dealing with address and mmu handle translation
 * needed to accommodate subsystems in a companion chip connected over
 * transaction bridge. \c HYDRA_TRANSLATE_PEER_SSID() and
 *  \c HYDRA_TRANSLATE_PEER_MMU_H() are used for translating addresses and
 *  mmu handles received from a subsystem that may be on or off-chip.
 *
 * More specialised macros \c HYDRA_TRANSLATE_MMU_H_FOR_PEER_SS() and
 * \c HYDRA_TRANSLATE_SSID_FOR_PEER() are provided for special circumstances
 * where addresses or handles in the local chip context need to be
 * translated for use by a peer.
 */

#ifndef HYDRA_H
#define HYDRA_H

/*
 * Interface dependencies.
 */

#include "hal.h"

#ifdef CHIP_AMBER
#ifndef hal_get_chip_version_major
/* These definitions have changed name in amber/d01 and beyond */
#define hal_get_chip_version_major() hal_get_chip_version_major_sys_chip_version()
#define hal_get_chip_version_minor() hal_get_chip_version_minor_sys_chip_version()
#define hal_get_chip_version_variant() hal_get_chip_version_variant_sys_chip_version()
#endif
#endif

#ifdef CHIP_NAPIER
/* These definitions do not exist in napier */
#define hal_get_chip_version_major() (0)
#define hal_get_chip_version_minor() (0)
#define hal_get_chip_version_variant() (0)
#endif

#if defined(CHIP_BASE_CRESCENDO) && defined(SUPPORT_PARTIAL_EMULATOR)
#define CRESCENDO_PARTIAL_EMU_VARIANT   0x0e
#define PARTIAL_EMUALTOR_ENV            (hal_get_chip_version_variant() ==  \
                                            CRESCENDO_PARTIAL_EMU_VARIANT)

#define SLAVE_AMBER_N_SS                5
#define REMOTE_CURATOR_SSID             (hydra_ssid)(16-SLAVE_AMBER_N_SS)
#define CURATOR_SSID                    (hydra_ssid)(PARTIAL_EMUALTOR_ENV ? \
                                        REMOTE_CURATOR_SSID:SYSTEM_BUS_CURATOR)
#else

#define CURATOR_SSID                    (SYSTEM_BUS_CURATOR)
#endif /* CHIP_BASE_CRESCENDO */

#ifndef CHIP_BASE_HYDRA
typedef uint8 system_bus;
#endif /* CHIP_BASE_HYDRA */

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
#ifdef CHIP_BASE_HYDRA
#define hydra_get_local_ssid() ((hydra_ssid)hal_get_reg_sub_sys_id())
#else
#define hydra_get_local_ssid() ((0))
#endif /* CHIP_BASE_HYDRA */

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

#include "hydra/hydra_txbus.h"

#endif /* HYDRA_H */
