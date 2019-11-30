/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file fault.c
 * \ingroup fault
 *
 * Groan quietly
 *
 * \section fault DESCRIPTION
 * This file contains all of the functions dealing with fault reporting
 *
 * NOTES
 *
 * FAULT_LEAN_AND_MEAN and FAULT_VERY_LEAN_AND_MEAN get set by
 * preprocessor directives in fault_private.h.
 */

#include "fault/fault_private.h"

#if defined(DEBUG_LEAN_NO_FAULT) && defined(PRODUCTION_BUILD)
#error "Don't use DEBUG_LEAN_NO_FAULT in a production build."
#endif

#ifndef FAULT_LEAN_AND_MEAN
static FAULTINFO faultinfo[NFI];
#define faultdb    faultinfo
#define LOCKDB()   block_interrupts()
#define UNLOCKDB() unblock_interrupts()
#endif

#ifndef FAULT_VERY_LEAN_AND_MEAN

/**
 * Initialisation state of fault.  We need this because faults may
 * be queued before the mechanism is fully initialised.  If other
 * modules could be persuaded not to do that, we wouldn't need this.
 */
typedef enum FAULT_INIT_STATE {
    /* Not yet initialised. */
    FAULT_UNINIT,
    /*
     * Not yet initialised; there is a fault waiting and the timer
     * should be kicked.
     */
    FAULT_UNINIT_FAULTED,
    /* Initialised. */
    FAULT_INIT
} FAULT_INIT_STATE;
static FAULT_INIT_STATE fault_init_state = FAULT_UNINIT;
#endif

#ifndef FAULT_LEAN_AND_MEAN

/** Is schedule_publish_faults() scheduled to run? */
static bool schedule_publish_faults_scheduled = FALSE;

/** Is publish_faults() scheduled to run? */
static bool publish_faults_scheduled = FALSE;

/* Minimum period between calls to publish_faults() in microseconds. */
#define PUBLISHING_DELAY (2 * SECOND)

#endif

/** Is the panic_on_fault functionality enabled? */
static bool panic_on_fault_enabled;

/**
 * If panic_on_fault_enabled is TRUE and panic_on_fault_id is fault_none then
 * any call to fault_diatribe() will provoke a call to fault_diatribe().  If panic_on_fault_id
 * is not fault_none then fault_diatribe() will call panic() whenever the specified
 * faultid arises. */
static faultid panic_on_fault_id = FAULT_NONE;

#if FAULT_TIMESTAMP_WIDTH == 16
/** We used to store the timestamp of the first occurrence of a fault_diatribe()
 * in a time.  To save some ram the time value is now held in a uint16,
 * shifted right by 8 - this should give a range of 16s, and a mystery
 * resolution.  Macros to convert between time and "shifted time". */
#define time_to_shifted_time(t) /*lint -e(704) could not care less what happens to the shifted-in bits in the shift, since this is disregarded anyway in the cast! */ ((uint16)((t)>>8))
#define shifted_time_to_time(st) (((TIME)(st))<<8)
#endif

/* Forward references. */
static void fault_core(faultid confession, DIATRIBE_TYPE arg, bool freshen);
#ifndef FAULT_VERY_LEAN_AND_MEAN
static void record_fault(faultid f, DIATRIBE_TYPE arg, bool freshen);
static void provoke_publication(void);
static void queue_schedule_publish_faults(void);
#endif
#ifndef FAULT_LEAN_AND_MEAN
static void groan(const FAULTINFO *fi);
static void queue_publish_faults(void);
static bool publish_a_fault(FAULTINFO *fi);
static void publish_a_fresh_fault(FAULTINFO *fi);
static void publish_fresh_faults(void);
#endif

#ifdef INSTALL_FAULT_TEST
#define FAULT_APPCMD_TEST_CODE 0xFA
static APPCMD_RESPONSE fault_test(uint32 command,
                                  uint32 * params,
                                  uint32 * result);
static volatile TIME fault_test_last_groan;
#endif
/**
 * Get ready to groan
 */
void init_fault(void)
{
#ifndef FAULT_VERY_LEAN_AND_MEAN
    bool do_stuff = (bool)(fault_init_state == FAULT_UNINIT_FAULTED);

#ifndef FAULT_LEAN_AND_MEAN
    publish_faults_scheduled = FALSE;
#endif
    fault_init_state = FAULT_INIT;

    if (do_stuff)
    {
        queue_schedule_publish_faults();
    }
    else
    {
        schedule_publish_faults_scheduled = FALSE;
    }
#endif
#ifdef INSTALL_FAULT_TEST
    (void)appcmd_add_test_handler(FAULT_APPCMD_TEST_CODE, fault_test);
#endif /* INSTALL_FAULT_TEST */
}

/**
 * Protest and continue
 */
#undef fault
void fault_diatribe(faultid confession, DIATRIBE_TYPE arg)
{
    L0_DBG_MSG2("Fault %d diatribe = %d", confession, arg);
    fault_core(confession, arg, FALSE);
}

/**
 * Protest more authoritatively and continue
 */
void fault_update(faultid confession, DIATRIBE_TYPE arg)
{
    fault_core(confession, arg, TRUE);
}

/**
 * Protest
 */
static void fault_core(faultid confession, DIATRIBE_TYPE arg, bool freshen)
{
#ifndef FAULT_VERY_LEAN_AND_MEAN
    /* Store the fault report in the database. */
    record_fault(confession, arg, freshen);

    /* Call panic() if the appropriate debugging options are set. */
    if (panic_on_fault_enabled)
    {
        if (panic_on_fault_id == FAULT_NONE || panic_on_fault_id == confession)
        {
            /* Report the fault ID */
            panic_diatribe(PANIC_HYDRA_ON_FAULT, (uint16f)confession);
        }
    }

    provoke_publication();
    /* publish_fresh_faults(); */
#endif
}

/**
 * Record a fault
 */
#ifndef FAULT_VERY_LEAN_AND_MEAN
static void record_fault(faultid f, DIATRIBE_TYPE arg, bool freshen)
{
    FAULTINFO *fi, newfi;
    uint16 i;
#if FAULT_TIMESTAMP_WIDTH == 32
    TIME now = get_time();
#endif
#if FAULT_TIMESTAMP_WIDTH == 16
    uint16 now = time_to_shifted_time(get_time());
#endif

#ifdef PRESERVED_MODULE_PRESENT
    /* Store in the "preserve" ram. */
    volatile preserved_struct *this_preserved = (volatile preserved_struct *)PRESERVED_ADDR;

    this_preserved->fault.last_id = f;
    this_preserved->fault.last_arg = (uint16)arg;
#endif

    /* Are we already collecting faults of this type? */
    for (i = 0, fi = faultdb; i < NFI; i++, fi++)
    {
        if (fi->f == f)
        {
            /*
             * There should be nothing to clear the database above us,
             * although new stuff may appear there.
             */

            LOCKDB();
            if (fi->f == f)
            {
                if (fi->n < MAX_FI_REPORTS)
                {
                    ++fi->n;
                }
                if (fi->tn < MAX_FI_TOTAL)
                {
                    ++fi->tn;
                }
                if (freshen)
                {
                    fi->arg = arg;
#if FAULT_TIMESTAMP_WIDTH != 0
                    fi->st = now;
#endif
                }
                UNLOCKDB();
                return;
            }
            UNLOCKDB();
        }
    }

    /* We are not yet collecting messages of this type, so try to open a
       new record in faultinfo[], taking care to minimise the time for which
       we block interrupts. */

    newfi.f = f;
    newfi.arg = arg;
    newfi.n = 1;
    newfi.tn = 1;
    newfi.h = FALSE;
#if FAULT_TIMESTAMP_WIDTH != 0
    newfi.st = now;
#endif

    for (i = 0, fi = faultdb; i < NFI; i++, fi++)
    {
        if (fi->f == FAULT_NONE)
        {
            LOCKDB();
            if (fi->f == FAULT_NONE)
            {
                *fi = newfi;
                UNLOCKDB();
                return;
            }
            UNLOCKDB();
        }
    }

    /* No room in the database for the fault report.  We used to record
       the overflow fault as an "unknown fault counter", but to save RAM we
       now just discard it. */
}

/**
 * provoke_publication
 */
static void provoke_publication(void)
{
    if (fault_init_state == FAULT_INIT)
    {
        if(!schedule_publish_faults_scheduled)
        {
            /*
             * A fault from a higher priority level could come in
             * here, resulting in two itimed events.  This is safe (E&OE) --
             * you just get two calls to schedule_publish_faults().
             */
            queue_schedule_publish_faults();
        }
    }
    else
    {
        fault_init_state = FAULT_UNINIT_FAULTED;
    }
}

/**
 * Ensure schedule_publish_faults is called
 */
static void queue_schedule_publish_faults(void)
{
    /* Provoke regular publishing of fault reports. */
    schedule_publish_faults_scheduled = TRUE;
    itimed_event_in((INTERVAL)10, itid_fault_schedule);
}
#endif

#ifndef FAULT_LEAN_AND_MEAN
/**
 * Force air through larynx
 *
 * Groan quietly using CCP.
 *
 * This must only be called from the high background (BG_HIGH).
 */
static void groan(const FAULTINFO *fi)
{
    TIME t;
    /* We have to divide a DIATRIBE_TYPE into an array of 16-bit integers.
     * In principle, uint16 has the semantics "uint_least16_t" so may actually
     * have more than 16 bits (e.g. on a 24-bit platform).  This means that
     * the number of 16-bit "pieces" in a DIATRIBE_TYPE is not simply
     * sizeof(DIATRIBE_TYPE)/sizeof(uint16).*/
#define DIATRIBE_TYPE_BIT ((sizeof(DIATRIBE_TYPE)/sizeof(char))*CHAR_BIT)
#define ARRAY_LENGTH ((DIATRIBE_TYPE_BIT+15)/16)
    uint16 args16[ARRAY_LENGTH];
    uint16f i;
    DIATRIBE_TYPE arg;
#if FAULT_TIMESTAMP_WIDTH == 32
    t = fi->st;
#endif
#if FAULT_TIMESTAMP_WIDTH == 16
    t = shifted_time_to_time(fi->st);
#endif
#if FAULT_TIMESTAMP_WIDTH == 0
    t = 0;
#endif

    arg = fi->arg;
    for (i = 0; i < ARRAY_LENGTH; ++i)
    {
        args16[i] = arg & 0xffff;
        /* We limit the shift for lint's benefit: for obvious reasons if arg is
         * too narrow to support a 16-bit right-shift then we don't use the
         * shifted value so we don't care that we've done an illogical shift */
        arg >>= MIN(DIATRIBE_TYPE_BIT-1, 16);
    }

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)
    if (KIP_SECONDARY_CONTEXT()) {
        /**
         * Talking to Curator is restricted to P0. Send a request to P0 to publish
         * the fault.
         */

        uint16 con_id = PACK_CON_ID(PACK_SEND_RECV_ID(IPC_PROCESSOR_0,0),
                                    PACK_SEND_RECV_ID(IPC_PROCESSOR_1,0));

        KIP_MSG_PUBLISH_FAULT_REQ* req_msg = xpmalloc(sizeof(KIP_MSG_PUBLISH_FAULT_REQ)
                                                      + (ARRAY_LENGTH * sizeof(uint16)));

        KIP_MSG_PUBLISH_FAULT_REQ_PACK(req_msg, con_id, (uint16)fi->f, IPC_PROCESSOR_1,
                                       (uint16)fi->n, t, ARRAY_LENGTH, 0);

        memcpy(req_msg->_data + KIP_MSG_PUBLISH_FAULT_REQ_ARGS_WORD_OFFSET,
               args16, ARRAY_LENGTH * sizeof(uint16));



        kip_adaptor_send_message(con_id,
                                 KIP_MSG_ID_PUBLISH_FAULT_REQ,        /* msg_id */
                                 KIP_MSG_PUBLISH_FAULT_REQ_WORD_SIZE, /* length */
                                 (unsigned*)req_msg,                  /* msg_data */
                                 NULL);                               /* context */
        pfree(req_msg);
    }
    else
#endif
    {
        subreport_event(CCP_EVENT_LEVEL_FAULT, (uint16)fi->f, P0,
                        (uint16)fi->n, t, args16, ARRAY_LENGTH);
    }

#ifdef INSTALL_FAULT_TEST
    /* Let the test code know that the groan has been groaned. */

    /* Make sure we don't accidentally set the timestamp to the "not set"
     * value.  It's a one in 4 billion chance, but hey, I'm feeling lucky */
    fault_test_last_groan = MAX(hal_get_time(), 1);
#endif
}

/**
 * ensure publish_faults is called
 */
static void queue_publish_faults(void)
{
    /* Ensure only one call to publish_faults() is scheduled at a time. */
    block_interrupts();
    if (!publish_faults_scheduled)
    {
        publish_faults_scheduled = TRUE;

        /* Call publish_faults() after a respectful delay. */
        itimed_event_in(PUBLISHING_DELAY, itid_fault_publish);
    }
    unblock_interrupts();
}

/**
 * Service the fault database
 *
 * Calls to fault_diatribe() park error messages in a small database.  The
 * publish_fault() call is called periodically whenever the database
 * holds any faults.
 *
 * The function makes a set of reports to an interested party -
 * typically the host.  Reports are not made for faults which have
 * only occurred once, however, on the basis that these have already
 * been reported (see publish_fresh_faults).
 *
 * This must only be called from the high background (BG_HIGH).
 */
static bool publish_a_fault(FAULTINFO *fi)
{
    FAULTINFO tmpfi;
    bool more = FALSE;

    if (fi->f != FAULT_NONE)
    {
        LOCKDB();
        tmpfi = *fi;
        if (fi->n != 0)
        {
            fi->n = 0;
            fi->h = TRUE;
        }
        else
        {
            fi->f = FAULT_NONE;
        }
        UNLOCKDB();
        if (tmpfi.n != 0)
        {
            more = TRUE;
            groan(&tmpfi);
        }
    }

    return(more);
}

/**
 * publish_faults
 */
void publish_faults_bg(void)
{
    FAULTINFO *fi;
    uint16 i;
    bool more = FALSE;

    /* Must do this first to avoid a race hazard - fault reports could
       come while we're flushing the database back to the dazed user. */
    publish_faults_scheduled = FALSE;

    /* Scan, flush and report the database of identified faults. */
    for (i = 0, fi = faultinfo; i < NFI; i++, fi++)
    {
        more = more | publish_a_fault(fi);
    }

    /* Need to reprovoke regular publishing of fault reports
       if any are known to remain in the database. */
    if (more)
    {
        queue_publish_faults();
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
 * emit DebugWord16_indications
 *
 * Emits a DebugWord16_indication (via the SDL) for each fresh
 * entry in the faults database.
 *
 * This must only be called from the background.
 */
static void publish_a_fresh_fault(FAULTINFO *fi)
{
    FAULTINFO tmpfi;

    if (fi->f != FAULT_NONE && !fi->h)
    {
        LOCKDB();
        tmpfi = *fi;
        fi->n = 0;
        fi->h = TRUE;
        UNLOCKDB();
        groan(&tmpfi);
    }
}

/**
 * publish_fresh_faults
 */
void publish_fresh_faults_bg(void)
{
    FAULTINFO *fi;
    uint16 i;

    for (i = 0, fi = faultinfo; i < NFI; i++, fi++)
    {
        publish_a_fresh_fault(fi);
    }
}

/** Dispatch to background to publish fresh faults */
static void publish_fresh_faults(void)
{
    GEN_BG_INT(fault_publish_fresh);
}

/**
 * Set up (delayed) publication of faults
 *
 * This can be called from itime as any I/O is dispatched to the
 * background.
 */
void schedule_publish_faults(void)
{
    /* Try to ensure only one call to schedule_publish_faults() is
     * scheduled at a time. */
    schedule_publish_faults_scheduled = FALSE;

    /* Emit DebugWord16_indication for each fresh entry in the db. */
    publish_fresh_faults();

    /* Ensure only one call to publish_faults() is scheduled at a time. */
    queue_publish_faults();
}

#endif /* FAULT_LEAN_AND_MEAN */

/**
 * Configure panic_on_fault
 */
void fault_set_panic_on_fault(bool enable)
{
    panic_on_fault_enabled = enable;
}

/**
 * Obtain panic_on_fault configuration
 */
bool fault_get_panic_on_fault(void)
{
    return(panic_on_fault_enabled);
}

/**
 * Refine panic_on_fault configuration
 */
void fault_set_panic_on_fault_code(uint16 id)
{
    panic_on_fault_id = (faultid)(id);
}

/**
 * Obtain panic_on_fault refined config
 */
uint16 fault_get_panic_on_fault_code(void)
{
    return((uint16)(panic_on_fault_id));
}

#ifdef INSTALL_FAULT_TEST
#define IMMEDIATE_PUBLISHING_DELAY ((INTERVAL)500)
#define DELAYED_PUBLISHING_DELAY \
                 time_add(PUBLISHING_DELAY, IMMEDIATE_PUBLISHING_DELAY)

bool fault_test_check_publishing_delay(TIME deadline)
{
    TIME extended_deadline = time_add(deadline, (TIME)100U*MILLISECOND);
    INTERVAL late_by;

    /* Busy wait for a generous version of the publishing delay */
    /*lint -e722 */
    while (time_eq(fault_test_last_groan, 0) &&
            time_lt(hal_get_time(), extended_deadline));

    /* The interrupt really ought to have fired by now, but we don't want to
     * get confused if it hasn't and it fires in the middle of this block */
    block_interrupts();
    if (time_eq(fault_test_last_groan, 0))
    {
        L0_DBG_MSG1("It is now 0x%x and no groan has been seen",
                    hal_get_time());
        L0_DBG_MSG1("(just to check, fault_test_last_groan = 0x%x)",
                fault_test_last_groan);
        unblock_interrupts();
        L0_DBG_MSG("Raised fault not published!");
        return FALSE;
    }

    /* Was the fault published on time? */
    late_by = time_sub(fault_test_last_groan, deadline);
    unblock_interrupts();

    if (late_by > 0)
    {
        L0_DBG_MSG1("Raised fault published late: over deadline by %ld us",
                    late_by);
        return FALSE;
    }
    return TRUE;
}

static bool fault_test_raise_a_fault(faultid id, bool first_time)
{
    TIME raised_at;
    uint16f i;

    raised_at = hal_get_time();
    fault_test_last_groan = (TIME)0;
    L0_DBG_MSG1("Raised fault at time 0x%x", raised_at);

    fault_diatribe(id, 0);

    for (i = 0; i < NFI; i++)
    {
        if (faultdb[i].f == id)
        {
            break;
        }
    }
    if (i == NFI)
    {
        /* Fault didn't appear in database */
        L0_DBG_MSG("Raised fault didn't appear!");
        return FALSE;
    }

    L0_DBG_MSG("Raised fault appeared");

    if (!fault_test_check_publishing_delay(time_add(raised_at,
                                   first_time ? IMMEDIATE_PUBLISHING_DELAY :
                                                DELAYED_PUBLISHING_DELAY)))
    {
        return FALSE;
    }
    L0_DBG_MSG1("Raised fault published after %ld us",
                time_sub(fault_test_last_groan, raised_at));
    return TRUE;
}


static APPCMD_RESPONSE fault_test(uint32 command,
                                  uint32 * params,
                                  uint32 * result)
{
    UNUSED(command);
    UNUSED(params);

    /* Raise some faults.  Check that they appear in the database 'n' stuff */


    if (!fault_test_raise_a_fault(FAULT_HYDRA_PANIC, TRUE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    if (!fault_test_raise_a_fault(FAULT_HYDRA_PANIC, FALSE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    if (!fault_test_raise_a_fault(FAULT_HYDRA_DIVIDE_BY_ZERO, TRUE))
    {
        *result = 0;
        return APPCMD_RESPONSE_SUCCESS;
    }

    *result = 1;
    return APPCMD_RESPONSE_SUCCESS;
}
#endif
