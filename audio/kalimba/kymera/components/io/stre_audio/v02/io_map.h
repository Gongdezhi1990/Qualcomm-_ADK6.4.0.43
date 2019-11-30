
/* *************************************************************************  *
   COMMERCIAL IN CONFIDENCE
   Copyright (C) 2017 Qualcomm Technologies International Ltd.

   Qualcomm Technologies International Ltd.
   Churchill House,
   Cambridge Business Park,
   Cowley Park,
   Cambridge, CB4 0WZ. UK
   http://www.csr.com

   $Id: //depot/dspsw/stre_rom_v02/kalimba/kymera/components/io/stre_audio/v02/io_map.h#2 $
   $Name$
   $Source$

   DESCRIPTION
      Hardware declarations header file (lower level).
      Lists memory mapped register addresses.

   INTERFACE
      Entry   :-
      Exit    :-

   MODIFICATIONS
      1.0    12/04/99    RWY    First created.
      1.x    xx/xx/xx    RWY    Automatically generated.

*  *************************************************************************  */

#define __IO_MAP_H__

/* We need this for the types of registers which are enums: */
#include "io_defs.h"




#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_TRACE
#define __IO_MAP_H__IO_MAP_MODULE_K32_TRACE

/* -- k32_trace -- Kalimba 32-bit Trace Control registers. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  TRACE_0_CFG;                         /* RW  10 bits */
extern volatile uint32  TRACE_0_DMEM_BASE_ADDR;              /* RW  32 bits */
extern volatile uint32  TRACE_0_DMEM_CFG;                    /* RW  13 bits */
extern volatile uint32  TRACE_0_END_TRIGGER;                 /* RW  32 bits */
extern volatile uint32  TRACE_0_START_TRIGGER;               /* RW  32 bits */
extern volatile uint32  TRACE_0_TBUS_BASE_ADDR;              /* RW  32 bits */
extern volatile uint32  TRACE_0_TBUS_CFG;                    /* RW  30 bits */
extern volatile uint32  TRACE_0_TRIGGER_CFG;                 /* RW  12 bits */
extern volatile uint32  TRACE_1_CFG;                         /* RW  10 bits */
extern volatile uint32  TRACE_1_DMEM_BASE_ADDR;              /* RW  32 bits */
extern volatile uint32  TRACE_1_DMEM_CFG;                    /* RW  13 bits */
extern volatile uint32  TRACE_1_END_TRIGGER;                 /* RW  32 bits */
extern volatile uint32  TRACE_1_START_TRIGGER;               /* RW  32 bits */
extern volatile uint32  TRACE_1_TBUS_BASE_ADDR;              /* RW  32 bits */
extern volatile uint32  TRACE_1_TBUS_CFG;                    /* RW  30 bits */
extern volatile uint32  TRACE_1_TRIGGER_CFG;                 /* RW  12 bits */
extern volatile uint32  TRACE_ACCESS_CTRL;                   /* RW   4 bits */
extern volatile uint32  TRACE_DEBUG_SEL;                     /* RW   4 bits */
extern volatile k32_trace__mutex_lock_enum  TRACE_MUTEX_LOCK;                    /* RW   4 bits */

/* Declarations of read only registers */
extern volatile uint32  TRACE_0_TRIGGER_STATUS;              /* R    6 bits */
extern volatile uint32  TRACE_1_TRIGGER_STATUS;              /* R    6 bits */
extern volatile uint32  TRACE_DMEM_STATUS;                   /* R    4 bits */
extern volatile uint32  TRACE_TBUS_STATUS;                   /* R    4 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_TRACE */


#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_DOLOOP_CACHE
#define __IO_MAP_H__IO_MAP_MODULE_K32_DOLOOP_CACHE

/* -- k32_doloop_cache -- Kalimba 32-bit DoLoop Cache Control registers. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  DOLOOP_CACHE_CONFIG;                 /* RW   2 bits */

/* Declarations of read only registers */
extern volatile uint32  DOLOOP_CACHE_FILL_COUNT;             /* R   32 bits */
extern volatile uint32  DOLOOP_CACHE_HIT_COUNT;              /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_DOLOOP_CACHE */





#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_TIMERS
#define __IO_MAP_H__IO_MAP_MODULE_K32_TIMERS

/* -- k32_timers -- Kalimba 32-bit Timers Control registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  TIMER1_EN;                           /* RW   1 bits */
extern volatile uint32  TIMER1_TRIGGER;                      /* RW  32 bits */
extern volatile uint32  TIMER2_EN;                           /* RW   1 bits */
extern volatile uint32  TIMER2_TRIGGER;                      /* RW  32 bits */

/* Declarations of read only registers */
extern volatile uint32  TIMER_TIME;                          /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_TIMERS */






#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_PREFETCH
#define __IO_MAP_H__IO_MAP_MODULE_K32_PREFETCH

/* -- k32_prefetch -- Kalimba 32-bit Prefetch Control registers. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  PREFETCH_CONFIG;                     /* RW   1 bits */
extern volatile uint32  PREFETCH_FLUSH;                      /* RW   1 bits */

/* Declarations of read only registers */
extern volatile uint32  PREFETCH_DEBUG;                      /* R   25 bits */
extern volatile uint32  PREFETCH_DEBUG_ADDR;                 /* R   32 bits */
extern volatile uint32  PREFETCH_PREFETCH_COUNT;             /* R   32 bits */
extern volatile uint32  PREFETCH_REQUEST_COUNT;              /* R   32 bits */
extern volatile uint32  PREFETCH_WAIT_OUT_COUNT;             /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_PREFETCH */













#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_MC_INTER_PROC_KEYHOLE
#define __IO_MAP_H__IO_MAP_MODULE_K32_MC_INTER_PROC_KEYHOLE

/* -- k32_mc_inter_proc_keyhole -- Kalimba 32-bit Multicore inter-processor communication keyhole register block -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  INTER_PROC_KEYHOLE_ACCESS_CTRL;      /* RW   4 bits */
extern volatile uint32  INTER_PROC_KEYHOLE_ADDR;             /* RW  32 bits */
extern volatile uint32  INTER_PROC_KEYHOLE_CTRL;             /* RW   8 bits */
extern volatile k32_mc_inter_proc_keyhole__mutex_lock_enum  INTER_PROC_KEYHOLE_MUTEX_LOCK;       /* RW   4 bits */
extern volatile uint32  INTER_PROC_KEYHOLE_WRITE_DATA;       /* RW  32 bits */

/* Declarations of read only registers */
extern volatile uint32  INTER_PROC_KEYHOLE_READ_DATA;        /* R   32 bits */
extern volatile inter_proc_keyhole_status  INTER_PROC_KEYHOLE_STATUS;           /* R    1 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_MC_INTER_PROC_KEYHOLE */
















#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_CORE
#define __IO_MAP_H__IO_MAP_MODULE_K32_CORE

/* -- k32_core -- Kalimba 32-bit Core Control registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  ARITHMETIC_MODE;                     /* RW   5 bits */
extern volatile uint32  BITREVERSE_VAL;                      /* RW  32 bits */
extern volatile uint32  DBG_COUNTERS_EN;                     /* RW   1 bits */
extern volatile uint32  FRAME_POINTER;                       /* RW  32 bits */
extern volatile uint32  MM_DOLOOP_END;                       /* RW  32 bits */
extern volatile uint32  MM_DOLOOP_START;                     /* RW  32 bits */
extern volatile uint32  MM_QUOTIENT;                         /* RW  32 bits */
extern volatile uint32  MM_REM;                              /* RW  32 bits */
extern volatile uint32  MM_RINTLINK;                         /* RW  32 bits */
extern volatile uint32  STACK_END_ADDR;                      /* RW  32 bits */
extern volatile uint32  STACK_POINTER;                       /* RW  32 bits */
extern volatile uint32  STACK_START_ADDR;                    /* RW  32 bits */
extern volatile uint32  TEST_REG_0;                          /* RW  32 bits */
extern volatile uint32  TEST_REG_1;                          /* RW  32 bits */
extern volatile uint32  TEST_REG_2;                          /* RW  32 bits */
extern volatile uint32  TEST_REG_3;                          /* RW  32 bits */

/* Declarations of read only registers */
extern volatile uint32  BITREVERSE_ADDR;                     /* R   32 bits */
extern volatile uint32  BITREVERSE_DATA;                     /* R   32 bits */
extern volatile uint32  BITREVERSE_DATA16;                   /* R   32 bits */
extern volatile uint32  NUM_CORE_STALLS;                     /* R   32 bits */
extern volatile uint32  NUM_INSTRS;                          /* R   32 bits */
extern volatile uint32  NUM_INSTR_EXPAND_STALLS;             /* R   32 bits */
extern volatile uint32  NUM_MEM_ACCESS_STALLS;               /* R   32 bits */
extern volatile uint32  NUM_RUN_CLKS;                        /* R   32 bits */
extern volatile uint32  PC_STATUS;                           /* R   32 bits */
extern volatile uint32  STACK_OVERFLOW_PC;                   /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_CORE */





#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_MONITOR
#define __IO_MAP_H__IO_MAP_MODULE_K32_MONITOR

/* -- k32_monitor -- Kalimba 32-bit Monitor Control registers. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  DM1_PROG_EXCEPTION_REGION_END_ADDR;  /* RW  32 bits */
extern volatile uint32  DM1_PROG_EXCEPTION_REGION_START_ADDR;  /* RW  32 bits */
extern volatile uint32  DM2_PROG_EXCEPTION_REGION_END_ADDR;  /* RW  32 bits */
extern volatile uint32  DM2_PROG_EXCEPTION_REGION_START_ADDR;  /* RW  32 bits */
extern volatile uint32  EXCEPTION_EN;                        /* RW   2 bits */
extern volatile uint32  PM_PROG_EXCEPTION_REGION_END_ADDR;   /* RW  32 bits */
extern volatile uint32  PM_PROG_EXCEPTION_REGION_START_ADDR;  /* RW  32 bits */
extern volatile uint32  PROG_EXCEPTION_REGION_ENABLE;        /* RW   4 bits */

/* Declarations of read only registers */
extern volatile uint32  EXCEPTION_PC;                        /* R   32 bits */
extern volatile exception_type  EXCEPTION_TYPE;                      /* R    4 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_MONITOR */












#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_MISC
#define __IO_MAP_H__IO_MAP_MODULE_K32_MISC

/* -- k32_misc -- Kalimba 32-bit Misc Control registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  ALLOW_GOTO_SHALLOW_SLEEP;            /* RW   1 bits */
extern volatile uint32  CLOCK_CONT_SHALLOW_SLEEP_RATE;       /* RW   8 bits */
extern volatile clock_divide_rate_enum  CLOCK_DIVIDE_RATE;                   /* RW   2 bits */
extern volatile uint32  CLOCK_STOP_WIND_DOWN_SEQUENCE_EN;    /* RW   1 bits */
extern volatile uint32  DISABLE_MUTEX_AND_ACCESS_IMMUNITY;   /* RW   1 bits */
extern volatile uint32  GOTO_SHALLOW_SLEEP;                  /* W    1 bits */
extern volatile uint32  PMWIN_ENABLE;                        /* RW   1 bits */
extern volatile uint32  PROC_DEEP_SLEEP_EN;                  /* RW   1 bits */

/* Declarations of read only registers */
extern volatile uint32  PROCESSOR_ID;                        /* R    1 bits */
extern volatile uint32  SHALLOW_SLEEP_STATUS;                /* R    1 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_MISC */


