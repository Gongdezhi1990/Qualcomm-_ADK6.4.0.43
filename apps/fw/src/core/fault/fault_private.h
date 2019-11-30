/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Internal header for fault.
 */

#ifndef FAULT_PRIVATE_H
#define FAULT_PRIVATE_H

#include "fault/fault.h"
/** Undefine the fault macro. This causes conflicts with our build system. */
#undef fault
#include "hal/haltime.h"
#include "itime/itime.h"
#ifdef SUBSYSTEM_AUDIO
#include "platform/core_library_preserve_block.h"
#else
#ifdef PRESERVED_MODULE_PRESENT
#include "preserved/preserved.h"
#endif /* PRESERVED_MODULE_PRESENT */
#endif /* SUBSYSTEM_AUDIO */
#ifdef SUBREPORT_MODULE_PRESENT
#include "subreport/subreport.h"
#endif
#include "int/int.h"
#include "panic/panic.h"
#include "sched/sched.h"
#ifdef IPC_MODULE_PRESENT
#include "ipc/ipc.h"
#endif
#include "utils/utils.h"
#include "hydra_log/hydra_log.h"

/** Minimum period between calls to publish_faults() in microseconds. */
#define PUBLISHING_DELAY (100 * MILLISECOND) 

/**
 * Typedef for the fault database element.
 */
typedef struct {
    faultid f;                 /**< A fault identifier. */
    DIATRIBE_TYPE arg;         /**< A fault argument. */
    uint32 st;                 /**< Time of first report. */
    uint16 n;                  /**< Number of reports of f. */
} faultinfo;

/** Size of faultinfo[] database. */
#define NFI 4

/** Limit of number of faults to record. */
#define MAX_FI_REPORTS 32767

/** Fault database. */
extern faultinfo faultdb[NFI];


/**
 * Publishes a fault. This work either from P0 to Curator via CCP or from P1 to
 * P0 via IPC.
 * \param fi Fault database entry.
 * \param cpu CPU identifier.
 * \note This must only be called from the high background (BG_HIGH).
 */ 
void groan(const faultinfo *fi, uint16 cpu);

/**
 * Inserts one fault entry into the database. This also sets the timestamp used
 * later in the reports.
 * \param f Fault ID.
 * \param arg Fault argument.
 * \param freshen Indicates if overwriting the argument and timestamp is
 *                wanted.
 */
void fault_db_insert(faultid f, DIATRIBE_TYPE arg, bool freshen);

/**
 * Returns one database entry at a time. It sets the number of occurrences to 0
 * and removes the dispensed entries on the next pass.
 * \param fi Pointer to a buffer to write the database entry.
 * \return TRUE if the database has more entries or FALSE if exhausted.
 */
bool fault_db_dispense(faultinfo *fi);


#endif /* FAULT_PRIVATE_H */
