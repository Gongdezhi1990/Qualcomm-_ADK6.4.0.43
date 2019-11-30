/*
 * Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd 
 * 
 */
/**
 *  \file
 * Initialises core modules for P1
 * 
 */
#include "init/init_private.h"

void initialise_os(void)
{
#if CHIP_HAS_CLEAN_SHALLOW_SLEEP_CLOCK_STOP_THAT_DEFAULTS_OFF
    /* Get shallow sleep to stop the clock cleanly */
    hal_set_reg_clock_stop_wind_down_sequence_en(1);
#endif

    L0_DBG_MSG1("Processor %d", THIS_PROCESSOR);
    L0_DBG_MSG1("Firmware ID %d", build_id_number);

    init_int();
    excep_enable(); /* Relies on int */
    memprot_enable(); /* Relies on int */
    init_itime();
    init_pmalloc();
    pioint_init(); /* Currently a no-op. */
    ipc_init();
    appcmd_init();
    init_sched();
    init_fault(); /* fault uses itime and sched */
#ifdef ENABLE_CACHE_TEST_LARGE_CODE
    cache_test();
#endif /* ENABLE_CACHE_TEST_LARGE_CODE */

    _init();
}
