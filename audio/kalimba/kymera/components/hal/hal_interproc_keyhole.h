/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup HAL Hardware Abstraction Layer
 * \file  gal_interproc_keyhole.h
 *
 * Public header file for HAL interproc functions
 *
 * \ingroup HAL
 *
 */
#ifndef _HAL_INTER_PROC_KEYHOLE_H_
#define _HAL_INTER_PROC_KEYHOLE_H_

#include "panic/panic.h"

/****************************************************************************
Public Constant and macros
*/

/**
 * Set the NVMEM_WIN0_CONFIG through inter-proc keyhole of target cpu
 */ 
#define hal_inter_proc_keyhole_set_nvmem_win0_config( cpu, val ) \
            hal_inter_proc_keyhole_write_dm_register( cpu, NVMEM_WIN0_CONFIG, val)

/* We don't touch CLOCK_DIVIDE_RATE elsewhere, so don't use it here
 * either, to avoid confusion. These two methods of putting the CPU to
 * sleep are equivalent. */
#define hal_inter_proc_keyhole_shallow_sleep_on(cpu)  \
    do { \
        /* Write ALLOW_GOTO_SHALLOW_SLEEP to deal with irrespective of the current value \
         * (FIXME: doesn't cope with being shut down in the middle of \
         * an interrupt, etc. Perhaps we should assume clean shutdown?) */ \
        hal_inter_proc_keyhole_write_dm_register(cpu, ALLOW_GOTO_SHALLOW_SLEEP, 1); \
        /* Actually put the core into sleep. */ \
        hal_inter_proc_keyhole_write_dm_register(cpu, GOTO_SHALLOW_SLEEP, 1); \
    } while(0)

/* Don't reset ALLOW_GOTO_SHALLOW_SLEEP since Pn might not be sleeping
 * and we are not blocking the Pn going to shallow sleep in future. 
 * Pull the core out of sleep first if it is sleeping.
 *
 * Pn clock must be enabled to make inter-proc keyhole write to work 
 * when it is already in shallow sleep. Whoever (ipc) calling this macro
 * must be doing it first.
 */
#define hal_inter_proc_keyhole_shallow_sleep_off(cpu)  \
        hal_inter_proc_keyhole_write_dm_register(cpu, GOTO_SHALLOW_SLEEP, 0)

/**
 * Set whether the given processor vetoes chip-wide deep sleep.
 */
#define hal_inter_proc_keyhole_set_deep_sleep_allowed(cpu, val) \
            hal_inter_proc_keyhole_write_dm_register(cpu, PROC_DEEP_SLEEP_EN, val)

/**
 *  It sets the debug register through keyhole. 
 */
#define hal_inter_proc_keyhole_set_debug( cpu, val) \
            hal_inter_proc_keyhole_write_dm_register( cpu, DEBUG, val)

/* Set PC through keyhole */
#define hal_inter_proc_keyhole_set_pc(cpu, val) \
            hal_inter_proc_keyhole_write_dm_register( cpu, REGFILE_PC, val)

/**
 *  Run or stop by writing DEBUG_RUN register through keyhole. 
 *  val = 0x1 sets  DEBUG_RUN and run the cpu
 *  val = 0x0 resets DEBUG_RUN and stop the cpu
 */
#define hal_inter_proc_keyhole_set_debug_run( cpu, val) \
            hal_inter_proc_keyhole_set_debug( cpu, \
                ((val << DEBUG_RUN_POSN) & DEBUG_RUN_MASK))

/**
 * Get the debug register value through inter-proc keyhole
 */
#define hal_inter_proc_keyhole_get_debug( cpu, val ) \
            hal_inter_proc_keyhole_read_dm_register( cpu, DEBUG, val) 

/**
 * Get the debug register value through inter-proc keyhole
 */
#define hal_inter_proc_keyhole_get_status( cpu, val ) \
            hal_inter_proc_keyhole_read_dm_register( cpu, STATUS, val) 

/**
 * Get the status of debug run through inter-proc keyhole ( true / false)
 */
#define hal_inter_proc_keyhole_get_debug_run_status( cpu, val ) \
            do { \
                hal_inter_proc_keyhole_read_dm_register( cpu, STATUS, val); \
                val = !!(val & STATUS_RUNNING_MASK); \
            } \
            while(0)

/**
 * \brief Hal marco to write a register through ipc keyhole
 */
#define hal_inter_proc_keyhole_write_dm_register( cpu, reg, val) \
        do { \
            hal_set_inter_proc_keyhole_write_dm_register( cpu, (volatile uint32*) &reg); \
            hal_set_reg_inter_proc_keyhole_write_data(val); \
        }while(0)
/**
 * \brief Hal marco to write a register through ipc keyhole
 */
#define hal_inter_proc_keyhole_read_dm_register( cpu, reg, val ) \
        do { \
            hal_set_inter_proc_keyhole_read_dm_register( cpu, (volatile uint32*) &reg); \
            val = hal_get_reg_inter_proc_keyhole_read_data(); \
        } while(0)

/****************************************************************************
Public Function Declarations
*/
/**
 * \brief Set the inter proc keyhole for writing
 *
 * \param target_cpu - The target cpu
 * \param mapped_reg - The mapped register
 *
 * \return void
 */
void hal_set_inter_proc_keyhole_write_dm_register( uint8    target_cpu,
                                          volatile uint32  *mapped_reg );

/**
 * \brief Set the interproc keyhole for reading
 *
 * \param target_cpu - The target cpu
 * \param mapped_reg - The mapped register
 *
 * \return void
 */

void hal_set_inter_proc_keyhole_read_dm_register( uint8    target_cpu,
                                         volatile uint32   *mapped_reg );

/****************************************************************************
Private inline Function Definitions
*/
/**
 * Force the given processor into or out of shallow sleep.
 */ 
inline static void hal_inter_proc_keyhole_set_shallow_sleep(uint8 cpu, bool sleep)
{
    if(!(sleep))
    {  
        hal_inter_proc_keyhole_shallow_sleep_off(cpu); 
    } 
    else 
    {  
        hal_inter_proc_keyhole_shallow_sleep_on(cpu); 
    }
}

#endif /* _HAL_INTERPROC_KEYHOLE_H_ */
