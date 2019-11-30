/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup preserved Preserved static data
 * \file 
 * Preserved static data
 *
 * \section preserved_h_usage USAGE
 *  This is the public header file for the preserved module.  It should
 *  be included in all modules that need to save state over a warm reset.
 *
 */

 /**
  * \mainpage
  * \section preserved_intro INTRODUCTION
  *
  * DESCRIPTION
  *
  * When power is first applied to the UWB WiMedia chip the contents of its
  * RAM are in an indeterminate state. However, sometimes the software may be
  * restarted by other mechanisms:
  * - the reset line may be toggled by external hardware;
  * - one of the watchdog timers may reset the chip;
  * - the software may force a reset (using one of the watchdog timers),
  * e.g. because an error condition has occurred (such as running out of
  * pmalloc memory) or at the request of the host;
  * - the software may branch to address zero, e.g. by dereferencing a NULL
  * function pointer.
  * Under these conditions, the contents of the RAM when the software
  * restarts are the same as immediately prior to the reset.
  *
  * The normal process for starting an (embedded) C program is to copy data
  * from ROM for variables that need to be initialised to non-zero values,
  * and to clear the rest of the RAM to zero.
  *
  * To support the preservation of a small number of variables across a
  * reset, the C startup code (xap2crt0.xap) avoids initialising or clearing
  * an area of RAM. This is used by this subsystem to pass data across a
  * reset. It is not cleared automatically, even if determined to be invalid.
  *
  * Immediately prior to an intentional reset a call should be made to
  * preserved_sign(), which calculates and stores a checksum over the
  * contents of the preserved memory. This does not occur for unexpected
  * resets, such as the reset line being controlled externally or a normal
  * watchdog timeout.
  *
  * When the system (re)starts it calls init_preserved(), which uses the
  * checksum to determine whether the contents of the preserved memory are
  * intact. The result of this check is subsequently available via
  * preserved_valid(). This is likely to be required by the initialisation
  * code of other subsystems, so the init_preserved() call should be made
  * early during the system's initialisation.
  *
  * Unlike BlueCore, the preserved memory should be signed when panic() is
  * called or prior to a cold reset. This is necessary to distinguish between
  * a deliberate reset and an unexpected watchdog reset. In both of these
  * cases preserved_clear() should be called (after the determining the cause
  * of reset) in order to fake loss of preserved data.
  *
  * Each processor has its own private and independently signed preserved
  * memory block. Hence, it is possible for one processor's block to be
  * intact whilst another's is corrupt.
  *
  * Any subsystem xxx that requires preserved data should contain a header
  * file called "xxx_preserved.h" that defines a struct xxx_preserved
  * containing the necessary data and a macro XXX_PRESERVED_DATA(m) as
  * follows:
  * \verbatim
  *
  * #ifndef XXX_PRESERVED_H
  * #define XXX_PRESERVED_H
  *
  * typedef struct xxx_preserved
  * {
  *  List of preserved variables for this subsystem
  *  int my_int;
  * } xxx_preserved;
  * #define XXX_PRESERVED_DATA(m) PRESERVED_DATA(m, (xxx_preserved, xxx))
  * 
  * #endif
  *
  * \endverbatim
  * If no preserved data is required with the current build options then this
  * can be reduced to:
  * \verbatim
  *
  * #ifndef XXX_PRESERVED_H
  * #define XXX_PRESERVED_H
  *
  * #define XXX_PRESERVED_DATA(m)
  *
  * #endif
  *
  * \endverbatim
  * It is not necessary to specifically list this header file anywhere;
  * it will be automatically located and used if the subsystem forms part
  * of the current build.
  */

#ifndef PRESERVED_H
#define PRESERVED_H

#include "hydra/hydra_types.h"
#include "preserved/preserved_subsystems.h"
#include "hal/hal.h"

#ifdef INITIALISE_DM_ON_BOOT
/* FIXME: the relevant symbols aren't available on Crescendo.
 * Find another way to do this check (probably in the link script
 * rather than here). */
#ifndef TODO_CRESCENDO
/* Make sure the preserve block is really going to be preserved */
#if ((PRESERVE_BLOCK_WORDS + DM_INIT_LIMIT) > DM_SIZE_IN_WORDS)
#error Preserve block / DM init size conflict
#endif
#endif /* TODO_CRESCENDO */
#endif /* INITIALISE_DM_ON_BOOT */

/** Helper macro to forward an expansion of the preserved data list to the
 * appropriate formatting macro */
#define PRESERVED_DATA(m,n)                     \
    PRESERVED_DATA_EXPAND_##m n

/** Header information for managing the preserved data */
typedef struct preserved_header
{
   /** Magic value to mark that the preserved memory block is likely to be
    * valid. This is checked prior to verifying the checksum. */
    uint16 magic_value;

    /** Sum of all the words in the preserved memory block (as indicated by
     * the "length" field, excluding the magic value and the checksum field
     * itself */
    uint16 checksum;

    /** Length of the preserved memory block, including this header */
    size_t length;
} preserved_header;

/** Structure containing all of the preserved data */
typedef struct preserved_struct
{
    /** Header information for managing the preserved data */
    preserved_header header;

    /** Preserved data for each subsystem */
#define PRESERVED_DATA_EXPAND_STRUCT(s,f) s f;
    PRESERVED_DATA_LIST(STRUCT)
} preserved_struct;

/** The preserved data */
extern volatile preserved_struct preserved;
#ifdef AUDIO_SECOND_CORE
extern volatile preserved_struct preserved1;
#endif//AUDIO_SECOND_CORE

#ifdef AUDIO_SECOND_CORE
#define PRESERVED_ADDR ((hal_get_reg_processor_id() == 0)? &preserved: &preserved1)
#else
#define PRESERVED_ADDR  &preserved
#endif


/**
 * Initialise the preserved memory block
 *
 * Checks that the checksum generated by preserved_sign() is still valid.
 * The result of this check is subsequently available via preserved_valid().
 *
 * The contents of the preserved memory are not cleared, even if determined
 * to be invalid. However, if the length stored within the preserved memory
 * appears to be valid, then any fields after that length are cleared.
 *
 * This function also deliberately corrupts the checksum to guard against
 * the data being preserved across any subsequent unexpected reset. Hence,
 * it must only be called once following a call to preserved_sign(). Special
 * care is required if a separate loader program is used, e.g. to support
 * DFU (Device Firmware Upgrade) or firmware loaded from the host.
 */
extern void init_preserved(void);


/**
 * Is the preserved memory block intact
 *
 * Returns
 *
 * TRUE if the preceding call to init_preserved() determined that the
 * preserved memory block is intact, else FALSE.
 */
extern bool preserved_valid(void);


/**
 * Clear the contents of the preserved memory block
 *
 * Clear the contents of the preserved memory block to zero and mark it as
 * invalid.
 */
extern void preserved_clear(void);


/**
 * Sign the preserved memory block
 *
 * Calcluates and stores the checksum for the preserved memory block.
 * Obviously, if the memory within the block is written to after calling
 * preserved_sign() then the checksum will be rendered invalid.
 *
 * If "valid" is TRUE then the calculated checksum value is stored so that
 * it can be verified by init_preserved(). However, if "sign" is FALSE then a
 * different value is written so that the subsequent test by init_preserved()
 * will fail.
 */
extern void preserved_sign(bool valid);

#endif /* PRESERVED_H */
