/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Fault implementation.
 *
 * \section fault DESCRIPTION
 * This file contains all of the core functions dealing with fault processing.
 */

#include "fault/fault_private.h"
#include "fault/fault_appcmd.h"

/**
 * Initialisation state enum.
 */
typedef enum FAULT_INIT_STATE {
    /* Not yet initialised. */
    FAULT_UNINIT,
    /* Not yet initialised; there is a fault waiting. */
    FAULT_UNINIT_FAULTED,
    /* Initialised. */
    FAULT_INIT
} FAULT_INIT_STATE;

/**
 * Fault init state container. We need this because faults may be queued before
 * the mechanism is fully initialised.
 */ 
static FAULT_INIT_STATE fault_init_state = FAULT_UNINIT;

/** Is publish_faults() already scheduled to run? */
static bool publish_faults_scheduled = FALSE;

/** Is the panic_on_fault functionality enabled? */
static bool panic_on_fault_enabled;

/**
 * If panic_on_fault_enabled is TRUE and panic_on_fault_id is FAULT_NONE then
 * any call to fault_diatribe() will provoke a call to panic(). If
 * panic_on_fault_id is not FAULT_NONE then fault_diatribe() will call panic()
 * whenever the specified faultid arises.
 */
static faultid panic_on_fault_id = FAULT_NONE;

/* Forward references. */

/**
 * Triggers a publication if the module is initialized.
 */
static void provoke_publication(void);

/**
 * Set up (delayed) publication of faults.
 * This can be called from itime as any I/O is dispatched to the
 * background.
 */ 
static void schedule_publish_faults(void);

/**
 * Central entry point for all faults.
 * \param confession Fault ID.
 * \param arg Fault argument (32 bit).
 * \param freshen If TRUE and fault already exists in the database this will
 *                trigger an update of the argument and timestamp. Otherwise
 *                only the number of occurences will be incremented.
 */
static void fault_core(faultid confession, DIATRIBE_TYPE arg, bool freshen);


void init_fault(void)
{
    bool do_stuff = (bool)(fault_init_state == FAULT_UNINIT_FAULTED);

    publish_faults_scheduled = FALSE;
    fault_init_state = FAULT_INIT;

    if (do_stuff)
    {
        provoke_publication();
    }

    fault_install_appcmd_handler();
}

void fault_diatribe(faultid confession, DIATRIBE_TYPE arg)
{
    fault_core(confession, arg, FALSE);
}

void fault_update(faultid confession, DIATRIBE_TYPE arg)
{
    fault_core(confession, arg, TRUE);
}

static void fault_core(faultid confession, DIATRIBE_TYPE arg, bool freshen)
{
    L0_DBG_MSG2("FAULT 0x%x 0x%x", confession, arg);

    /* Store the fault report in the database. */
    fault_db_insert(confession, arg, freshen);

    /* Call panic() if the appropriate debugging options are set. */
    if (panic_on_fault_enabled)
    {
        if (panic_on_fault_id == FAULT_NONE || panic_on_fault_id == confession)
        {
            panic(PANIC_HYDRA_ON_FAULT);
        }
    }

    provoke_publication();
}

static void provoke_publication(void)
{
    if (fault_init_state == FAULT_INIT)
    {
        schedule_publish_faults();
    }
    else
    {
        fault_init_state = FAULT_UNINIT_FAULTED;
    }
}

static void schedule_publish_faults(void)
{
    bool to_publish = FALSE;

    /* Ensure only one call to publish_faults() is scheduled at a time. */
    block_interrupts();
    if (!publish_faults_scheduled)
    {
        publish_faults_scheduled = TRUE;
        to_publish = TRUE;
    }
    unblock_interrupts();

    if(to_publish)
    {
        /* Call publish_faults() after a respectful delay. */
        itimed_event_in(PUBLISHING_DELAY, itid_fault_publish);
    }
}  

/**
 * Dispatch to background to publish faults over host interface.
 */
void publish_faults(void)
{
    GEN_BG_INT(fault_publish);
} 

/**
 * publish_faults
 *
 * Service the fault database
 *
 * Calls to fault_diatribe() or fault_update() park error messages in a small
 * database.  The publish_fault() call is called periodically whenever the
 * database holds any faults.
 *
 * The function makes a set of reports to an interested party - typically the
 * host.
 *
 * This must only be called from the high background (BG_HIGH).
 */
void publish_faults_bg(void)
{
    faultinfo tmpfi;
    bool more = FALSE;

    /* Allow other faults to be scheduled. */
    publish_faults_scheduled = FALSE;

    while (fault_db_dispense(&tmpfi))
    {
        if(tmpfi.n)
        {
            more = TRUE;
            groan(&tmpfi, THIS_PROCESSOR);
        }
    }

    /* Need to reprovoke regular publishing of fault reports
       if any are known to remain in the database. */
    if (more)
    {
        schedule_publish_faults();
    }
}

void fault_set_panic_on_fault(bool enable)
{
    panic_on_fault_enabled = enable;
}

bool fault_get_panic_on_fault(void)
{
    return(panic_on_fault_enabled);
}

void fault_set_panic_on_fault_code(uint16 id)
{
    panic_on_fault_id = (faultid)(id);
}

uint16 fault_get_panic_on_fault_code(void)
{
    return((uint16)(panic_on_fault_id));
}

