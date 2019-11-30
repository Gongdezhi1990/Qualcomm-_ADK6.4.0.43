/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup profiler Profiler
 * \ingroup platform
 *
 * \file  profiler_c.h
 * \ingroup profiler
 *
 * \brief
 * Profiler module gives different methods of measuring MIPS.
 *
 * 1. Static measurements: The profiler structure is declared as a global variable.
 *    This variable can be used with PROFILE_START(PROF), PROFILE_STOP(PROF) and
 *    PROFILER_MEASURE(PROF, Y) to define the segment where the measurement should be
 *    done. The name and ID of the profiler can be ignored because the name of the
 *    global variable can be read externally from the debug information (.elf file).
 *
 * 2. Dynamic measurements: The profiler structure is allocated at runtime.
 *    PROFILER_CREATE(name, ID, retval) macro creates a new profiler with the given
 *    name and ID. Note: The name is saved in an unmapped area and it can only be
 *    extracted from the debug information (.elf file). The created variable can be
 *    used together with PROFILE_START(PROF), PROFILE_STOP(PROF) and
 *    PROFILER_MEASURE(PROF, Y) to measure mips.
 *
 * 3. Simplified dynamic measurements: To give a fast measuring method to the users
 *    of this module, one can use DYN_PROFILER_MEASURE(NAME,ID,Y),
 *    DYN_PROFILER_START(NAME, ID) and DYN_PROFILER_STOP(NAME, ID) without taking care of
 *    memory allocation of the profiler. Note: this method search through the available
 *    profilers to find the right one which increases the overall mips consumption.
 *    The extensive use of this measuring method should be avoided.
 *
 * The profiler structure contains a constant name and ID to avoid the creation of an
 * internal id for each profiler. For example: the operator and endpoint ID can be used
 * directly without any conversation if the name for the two measurement is different.
 * The same applies if you want to measure a function inside an operator.
 *
 * Use PROFILER_DEREGISTER(PROF) or DYN_PROFILER_DEREGISTER(NAME, ID)  to remove any
 * unwanted profiler.
 *
 */
#ifndef PROFILER_HEADER_C_INCLUDED
#define PROFILER_HEADER_C_INCLUDED
/* Check if the profiler is enabled */
#if defined(PROFILER_ON) && !defined(__GNUC__)

/*****************************************************************************
Include Files
*/
#include "types.h"
#include "platform/pl_intrinsics.h"
#include "pl_timers/pl_timers.h"

/****************************************************************************
Public Constant Definitions
*/
/**
 * Until a profiler is not part of the profiler list the next field of the structure
 * should be equal with uninitialised constant ($profiler.UNINITIALISED).
 */
#define UNINITIALISED_PROFILER  ((profiler*)(-1))


/**
 * Definition used to signal the mips usage for non existent profiler.
 */
#define INVALID_MIPS_USAGE (-1)

/****************************************************************************
Public Function Declarations
*/

/**
 * Initialise the profiler library. Should be called before the timer library has been
 * initialised. Implemented in profiler.asm.
 *
 */
extern void profiler_initialise(void);

/**
 * Start profiling of a particular routine. Implemented in profiler.asm.
 *
 * @param address - address of structure to use for profiling
 */
extern void profiler_start(void* address);

/**
 * Stop profiling of a particular routine. Implemented in profiler.asm.
 *
 * @param address - address of structure to use for profiling
 */
extern void profiler_stop(void* address);

/**
 * Get profiler state. Implemented in profiler.asm.
 *
 */
extern bool get_profiler_state(void);

/**
 * Enable profiler. Implemented in profiler.asm.
 *
 */
extern void profiler_enable(void);

/**
 * Disable profiler. Implemented in profiler.asm.
 *
 */
extern void profiler_disable(void);

/**
 * Set nominal CPU speed, for use by off-chip tools.
 * 0 means "unknown".
 * Implemented in profiler.asm.
 *
 */
extern void profiler_set_cpu_speed(unsigned cpu_speed_mhz);

/**
 * Clears the profiler list completely when the profiler is disabled.
 *
 */
extern void profiler_deregister_all(void);

/****************************************************************************
Public Type Declarations
*/

/**
 * Profiler structure used by the profiler library.
 */
typedef struct profiler
{
    struct profiler  *next;

    /**
     * Name of the profiler. This can be NULL if the profiler is static (declared as a
     * global variable).
     */
    const char *name;
    /**
     * Two profilers can have the same name if their ID is different. This helps
     * avoiding the creation of internal id for each profiler because a profiler
     * is specified by a name and an ID.
     */
    unsigned id;
    /**
     * Protection against getting called from both background and interrupt
     */
    unsigned nest_count;
    /**
     * The calculated cpu usage in fraction(thousands). This is refreshed every 1.024
     * seconds.
     */
    unsigned cpu_fraction;

    /**
     * The peak cpu usage (max of cpu_fraction).
     */
    unsigned peak_cpu_fraction;

    /**
     * Internal variable used to calculate the total time spent between profiler
     * start and stop.
     */
    unsigned start_time;

    /**
     * Internal variable used to extract any interrupt time from the total time.
     */
    unsigned int_start_time;

    /**
     * The total time spent on the profiled code between scheduled events (1.024
     * seconds).
     */
    unsigned total_time;
#ifdef DETAILED_PROFILER_ON
    #ifdef K32
        unsigned run_clks_start;
        unsigned run_clks_total;
        unsigned run_clks_average;
        unsigned run_clks_max;
        unsigned int_start_clks;
        unsigned instrs_start;
        unsigned instrs_total;
        unsigned instrs_average;
        unsigned instrs_max;
        unsigned int_start_instrs;
        unsigned core_stalls_start;
        unsigned core_stalls_total;
        unsigned core_stalls_average;
        unsigned core_stalls_max;
        unsigned int_start_core_stalls;
        unsigned prefetch_wait_start;
        unsigned prefetch_wait_total;
        unsigned prefetch_wait_average;
        unsigned prefetch_wait_max;
        unsigned int_start_prefetch_wait;
        #ifdef CHIP_BASE_CRESCENDO
            unsigned mem_access_stalls_start;
            unsigned mem_access_stalls_total;
            unsigned mem_access_stalls_average;
            unsigned mem_access_stalls_max;
            unsigned int_start_access_stalls;
            unsigned instr_expand_stalls_start;
            unsigned instr_expand_stalls_total;
            unsigned instr_expand_stalls_average;
            unsigned instr_expand_stalls_max;
            unsigned int_start_instr_expand_stalls;
        #endif /* CHIP_BASE_CRESCENDO */
    #else /* K32 */
        unsigned run_clks_ms_start;
        unsigned run_clks_ls_start;
        unsigned run_clks_ms_total;
        unsigned run_clks_ls_total;
        unsigned run_clks_average;
        unsigned run_clks_ms_max;
        unsigned run_clks_ls_max;
        unsigned int_start_clks_ms;
        unsigned int_start_clks_ls;
        unsigned instrs_ms_start;
        unsigned instrs_ls_start;
        unsigned instrs_ms_total;
        unsigned instrs_ls_total;
        unsigned instrs_average;
        unsigned instrs_ms_max;
        unsigned instrs_ls_max;
        unsigned int_start_instrs_ms;
        unsigned int_start_instrs_ls;
        unsigned stalls_ms_start;
        unsigned stalls_ls_start;
        unsigned stalls_ms_total;
        unsigned stalls_ls_total;
        unsigned stalls_average;
        unsigned stalls_ms_max;
        unsigned stalls_ls_max;
        unsigned int_start_stalls_ms;
        unsigned int_start_stalls_ls;
    #endif
#endif
        unsigned temp_count;
        unsigned count;
        
        unsigned kick_total;
        unsigned kick_inc;
} profiler;

/**
 * Mips measure list declared in profiler.asm.
 */
extern profiler* profiler_list;

/**
 * Profiler responsible for measuring the sleep time (declared in profiler.asm).
 *
 */
extern profiler sleep_time;

/**
 * Contains the timer_id of the last scheduled profiler event.
 *
 */
extern tTimerId profiler_timer_id;
/*****************************************************************************
Private Function Definitions
*/

/**
 * Creates a new profiler and initialise it.
 *
 * @param name - Constant name of the profiler
 * @param id - the id of the profiler
 * @return Returns a pointer to the newly created profiler
 */
extern profiler* create_dynamic_profiler(const char *name, unsigned id);


/**
 * Initialise a given profiler.
 *
 * @param name - Constant name of the profiler
 * @param id - the id of the profiler
 * @param prof - pointer to the profiler which will be initialise.
 */
extern void init_static_profiler(profiler* prof, const char *name, unsigned id);

/**
 *  Returns the profiler structure associated with the constant name and id.
 *
 * @param name - pointer to a constant string which represents the name of the profiler.
 * @param id - dynamic id
 * @return pointer to the profiler structure associated with the name and id.
 */
static inline profiler* get_log_structure(const char *name, unsigned id)
{
    profiler **cur_prof= &profiler_list;

    LOCK_INTERRUPTS;
    while ((*cur_prof)!=NULL)
    {
        if (((*cur_prof)->id == id) && ((*cur_prof)->name == name))
        {
            UNLOCK_INTERRUPTS;
            return (*cur_prof);
        }
        cur_prof = &((*cur_prof)->next);
    }
    UNLOCK_INTERRUPTS;

    return create_dynamic_profiler(name,id);
}

/**
 *  Looks up for the profiler by and deletes it from the profiler list.
 *
 * @param profiler_to_remove - pointer to the profiler marked for delete.
 */
extern void deregister(profiler *profiler_to_remove);

/**
 *  Looks up for the profiler by name and ID and deletes it from the profiler list.
 *
 * @param name - Constant name of the profiler
 * @param id - ID of the profiler
 */
extern void deregister_by_name_and_id(const char *name, unsigned id);


/*****************************************************************************
Private Macro Definitions
*/

/**
 * Declare a string for use in profiler.
 *
 * Depending on the compiler this text may need to be declared as *.
 * Also if -fwritable-strings option is enabled which disables string pooling
 * we are screwed.
 */
#define LOG_STRING(label, text) \
    LOG_STRING_ATTR static const char label[] = text

/**
 * Special storage section "attribute" for profiler string declarations.
 */
#if !defined (__GNUC__)
/* Alternative version that works in KCC */
#define LOG_STRING_ATTR _Pragma("datasection DEBUG_TRACE_STRINGS")
#else
#define LOG_STRING_ATTR __attribute__((section("DBG_STRING")))
#endif

/*****************************************************************************
Public Macro Definitions
*/

/*****************************************************************************
 * Macros for dynamically allocated user profilers.
*/

/**
 * Initialise the profiler library. Please see the documentation of profiler_initialise.
 */
#define PROFILER_INIT()           profiler_initialise()

/**
 * Initalise a static profiler with the given name and ID.
 */
#define PROFILER_STATIC_INIT(PROF, name, ID)\
    do                                                                  \
    {                                                                   \
        LOG_STRING(log_name, name);                                     \
        init_static_profiler(PROF, log_name, (unsigned)(ID));           \
    }                                                                   \
    while (0)


/**
 * Macro for starting a profiler.
 */
#define PROFILER_START(PROF) profiler_start(PROF)

/**
 * Macro for stopping a running profiler.
 */
#define PROFILER_STOP(PROF) profiler_stop(PROF)

/**
 * Measure the mips for a given call or instruction (Y).
 */
#define PROFILER_MEASURE(PROF, Y)\
    do                                                                  \
    {                                                                   \
        profiler_start(PROF);                                           \
        Y;                                                              \
        profiler_stop(PROF);                                            \
    }                                                                   \
    while (0)

/**
 * Macro for deregistering a profiler.
 * Defined to nothing (below) if profiler isn't enabled in the build
 */
#define PROFILER_DEREGISTER(PROF) deregister(PROF)

/**
 * Macro for deleting a profiler.
 * Defined to nothing (below) if profiler isn't enabled in the build
 */
#define PROFILER_DELETE(PROF) pdelete(PROF)


/*****************************************************************************
 * Macros for dynamically allocated user profilers.
*/

/**
 * Macro for starting a profiler.
 *
 * Whenever it's possible try to used DYN_PROFILER_MEASURE to avoid double search for
 * the profiler.
 *
 * If there is no profiler with the given name this macro will create one. Use
 * DYN_PROFILER_DEREGISTER to remove any unwanted profiling.
 *
 * Only use this type of profiling for testing!
 */
#define DYN_PROFILER_START(name, ID) \
    do                                                                  \
    {                                                                   \
        LOG_STRING(log_name, name);                                     \
        profiler_start(get_log_structure(log_name, (unsigned)(ID)));    \
    }                                                                   \
    while (0)

/**
 * Macro for stopping a running profiler.
 *
 * Whenever it's possible try to used DYN_PROFILER_MEASURE to avoid double search for
 * the profiler.
 */
#define DYN_PROFILER_STOP(name, ID) \
    do                                                                  \
    {                                                                   \
        LOG_STRING(log_name, name);                                     \
        profiler_stop(get_log_structure(log_name, (unsigned)(ID)));     \
    }                                                                   \
    while (0)

/**
 * Macro to measure the mips for a given call or instruction (Y).
 * The profiler is specified by name and ID.
 *
 * If there is no profiler with the given name this macro will create one. Use
 * DYN_PROFILER_DEREGISTER to remove any unwanted profiling.
 *
 * Only use this type of profiling for testing!
 */
#define DYN_PROFILER_MEASURE(NAME,ID,Y)\
    do                                                                  \
    {                                                                   \
        LOG_STRING(log_name, NAME);                                     \
        profiler* profiler_entry    = get_log_structure(log_name, (unsigned)(ID));   \
        profiler_start(profiler_entry);                                 \
        Y;                                                              \
        profiler_stop(profiler_entry);                                  \
    }                                                                   \
    while (0)

/**
 *  Macro to deregister a profiler by name and ID.
 */
#define DYN_PROFILER_DEREGISTER(NAME, ID) \
        do                                                                  \
        {                                                                   \
            LOG_STRING(log_name, NAME);                                     \
            deregister_by_name_and_id(log_name, (unsigned)(ID));            \
        }                                                                   \
        while (0)

/**
 * Enable overall profiling if it was disabled. Implemented in profiler.asm.
 *
 */
#define ENABLE_PROFILER()\
        LOCK_INTERRUPTS;                                            \
        profiler_enable();                                          \
        UNLOCK_INTERRUPTS

/**
 * Disable overall profiling. Implemented in profiler.asm.
 *
 */
#define DISABLE_PROFILER()\
        LOCK_INTERRUPTS;                                            \
        timer_cancel_event(profiler_timer_id);                      \
        profiler_deregister_all();                                  \
        profiler_disable();                                         \
        UNLOCK_INTERRUPTS

/**
 * Set nominal CPU speed, for use by off-chip tools.
 * 0 means "unknown".
 *
 */
#define PROFILER_SET_CPU_SPEED(mhz) profiler_set_cpu_speed(mhz)

#else /* defined(PROFILER_ON) && !defined(__GNUC__) */

/**
 * Dummy macros if the profiler is disabled.
 */
#undef PROFILER_ON
#undef INTERRUPT_PROFILER_ON
#undef DETAILED_PROFILER_ON

#define PROFILER_INIT()           ((void)0)

#define PROFILER_STATIC_INIT(PROF, name, ID) ((void)0)

#define PROFILER_MEASURE(PROF,Y)        Y

#define PROFILER_START(PROF)            ((void)0)

#define PROFILER_STOP(PROF)             ((void)0)

#define PROFILER_DEREGISTER(PROF)       ((void)0)

#define PROFILER_DELETE(PROF)           ((void)0)

#define DYN_PROFILER_MEASURE(NAME,ID,Y)    Y

#define DYN_PROFILER_START(NAME, ID)    ((void)0)

#define DYN_PROFILER_STOP(NAME, ID)     ((void)0)

#define DYN_PROFILER_DEREGISTER(NAME, ID)  ((void)0)

#define ENABLE_PROFILER()       ((void)0)

#define DISABLE_PROFILER()       ((void)0)

#define PROFILER_SET_CPU_SPEED(mhz) (UNUSED(mhz))

#endif /* defined(PROFILER_ON) && !defined(__GNUC__) */

#endif /* PROFILER_HEADER_C_INCLUDED */
