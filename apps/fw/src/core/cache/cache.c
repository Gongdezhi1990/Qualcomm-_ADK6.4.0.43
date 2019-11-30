/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#include "cache/cache.h"
#define IO_DEFS_MODULE_APPS_SYS_SYS
#define IO_DEFS_MODULE_KALIMBA_READ_CACHE
#define IO_DEFS_MODULE_K32_CORE
#define IO_DEFS_MODULE_SQIF
#define IO_DEFS_MODULE_SQIF_DATAPATH
#define IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS
#define IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP
#include "io/io.h"
#include "hal/hal.h"
#include "hydra_log/hydra_log.h"
#include "hydra/hydra_macros.h"
#include "appcmd/appcmd.h"
#include "assert.h"


#ifdef ENABLE_CACHE_TEST_LARGE_CODE
static void run_cache_tests(uint32 seed, uint32 loops);
extern uint32 cache_test_1(uint32 seed, uint32 value);
extern uint32 cache_test_2(uint32 seed, uint32 value);
extern uint32 cache_test_3(uint32 seed, uint32 value);
extern uint32 cache_test_4(uint32 seed, uint32 value);
extern uint32 cache_test_5(uint32 seed, uint32 value);
extern uint32 cache_test_6(uint32 seed, uint32 value);
extern uint32 cache_test_7(uint32 seed, uint32 value);
extern uint32 cache_test_8(uint32 seed, uint32 value);
APPCMD_RESPONSE cache_appcmd_test_handler_fn(uint32 command,
                                            uint32 * params,
                                            uint32 * result);

const uint32 cache_const_mask[20] = { 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
                                    31, 31, 31, 31, 31, 31, 31, 31, 31, 31 };

#endif /* ENABLE_CACHE_TEST_LARGE_CODE */


#ifdef ENABLE_CACHE_TEST_LARGE_CODE

void cache_test(void)
{
    uint32 i;
    uint32 seed = 0x12345678;
    run_cache_tests(seed, 2);
    for(i=0; i<3; ++i)
    {
        run_cache_tests(seed, 2);
    }
}

static void run_cache_tests(uint32 seed, uint32 loops)
{
    uint32 i;
    uint32 results[8];
    uint32 instructions, stalls, num_clocks, hits, miss, dm_hits, dm_miss;

    instructions = hal_get_reg_num_instrs();
    stalls = hal_get_reg_num_core_stalls();
    num_clocks = hal_get_reg_num_run_clks();
    hal_set_reg_apps_sys_cache_sel(THIS_PROCESSOR);
    hits = hal_get_reg_kalimba_read_cache_pm_hit_counter();
    miss = hal_get_reg_kalimba_read_cache_pm_miss_counter();
    dm_hits = hal_get_reg_kalimba_read_cache_dm_hit_counter();
    dm_miss = hal_get_reg_kalimba_read_cache_dm_miss_counter();

    results[0] = cache_test_1(seed, loops);
    results[1] = cache_test_2(seed, loops);
    results[2] = cache_test_3(seed, loops);
    results[3] = cache_test_4(seed, loops);
    results[4] = cache_test_5(seed, loops);
    results[5] = cache_test_6(seed, loops);
    results[6] = cache_test_7(seed, loops);
    results[7] = cache_test_8(seed, loops);

    instructions = hal_get_reg_num_instrs() - instructions;
    stalls = hal_get_reg_num_core_stalls() - stalls;
    num_clocks = hal_get_reg_num_run_clks() - num_clocks;
    hal_set_reg_apps_sys_cache_sel(THIS_PROCESSOR);
    hits = hal_get_reg_kalimba_read_cache_pm_hit_counter() - hits;
    miss = hal_get_reg_kalimba_read_cache_pm_miss_counter() - miss;
    dm_hits = hal_get_reg_kalimba_read_cache_dm_hit_counter() - dm_hits;
    dm_miss = hal_get_reg_kalimba_read_cache_dm_miss_counter() - dm_miss;

    L0_DBG_MSG1("CACHE TEST result %8x", results[0]);
    for(i=1; i<8; ++i)
    {
        assert(results[i] == results[0]);
    }

    L2_DBG_MSG3("Cache test end %d instructions, %d stalls, %d clocks",
                                    instructions, stalls, num_clocks);
    L2_DBG_MSG4("CACHE PM hits %d misses %d = %d %% miss, %d clock/inst",
            hits, miss, miss * 100 / hits, num_clocks/instructions);
    L2_DBG_MSG3("CACHE DM hits %d misses %d = %d %% miss",
                            dm_hits, dm_miss, dm_miss * 100 / dm_hits);

}

APPCMD_RESPONSE cache_appcmd_test_handler_fn(uint32 command,
                                            uint32 * params,
                                            uint32 * result)
{
    uint32 seed = 0x12345678;
    UNUSED(command);
    UNUSED(params);
    UNUSED(result);
    run_cache_tests(seed, params[0]);
    return APPCMD_RESPONSE_SUCCESS;
}

#endif /* ENABLE_CACHE_TEST_LARGE_CODE */
