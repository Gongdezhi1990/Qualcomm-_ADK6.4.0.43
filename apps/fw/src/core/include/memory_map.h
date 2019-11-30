/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef MEMORY_MAP_H_
#define MEMORY_MAP_H_

#define IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP
#include "io/io_defs.h" /* Get symbolic constants for the bounds of local and
                           remote RAM*/

#ifdef SUBSYSTEM_APPS
 /*lint --e{613}  -emacro( (413), MEMORY_MAP_IS_P0_DM_RAM )  PxD_P0_DM_RAM_LOWER
  * happens to equal NULL */
 #define MEMORY_MAP_IS_P0_DM_RAM(ptr)\
     ((ptr) != NULL && (void *)PxD_P0_DM_RAM_LOWER <= (ptr) && \
                                     (ptr) < (void *)PxD_P0_DM_RAM_UPPER)
 #define MEMORY_MAP_IS_P1_DM_RAM(ptr)\
     ((void *)PxD_P1_DM_RAM_LOWER <= (ptr) && \
                                         (ptr) < (void *)PxD_P1_DM_RAM_UPPER)

 #define MEMORY_MAP_IS_P0_DM_CONST(ptr) \
     (((size_t)(ptr) >= P0D_SQIF0_CACHED_LOWER && \
               (size_t)(ptr) < P0D_SQIF0_CACHED_UPPER) || \
      ((size_t)(ptr) >= P0D_SQIF1_CACHED_LOWER && \
               (size_t)(ptr) < P0D_SQIF1_CACHED_UPPER))

 #define vm_const_is_encoded MEMORY_MAP_IS_P0_DM_CONST

 #define MEMORY_MAP_IS_P1_DIRECTSQIF(ptr) \
    ( ((size_t)(ptr) >= P1D_SQIF01_DIRECT_LOWER) && \
       ((size_t)(ptr) < P1D_SQIF01_DIRECT_UPPER) \
    )
    
#endif


#ifdef CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036
#define MEMORY_MAP_ADJUST_CONST_P1_P0(ptr, ptrtype) \
    (((size_t)(ptr) >= P0D_SQIF1_CACHED_LOWER && \
                            (size_t)(ptr) < P0D_SQIF1_CACHED_UPPER) ?\
        (ptrtype)(((size_t)(ptr) - P0D_SQIF1_CACHED_LOWER) + \
                          P0D_SQIF0_CACHED_LOWER + ipc_get_p1_flash_offset()) :\
        (ptr))

#endif /* CHIP_DEF_P1_SQIF_SHALLOW_SLEEP_WA_B_195036 */
#endif /* MEMORY_MAP_H_ */
