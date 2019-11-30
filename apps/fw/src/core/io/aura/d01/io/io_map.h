
/* *************************************************************************  *
   COMMERCIAL IN CONFIDENCE
   Copyright (C) 2017 Qualcomm Technologies International Ltd.

 


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


#if defined(IO_DEFS_MODULE_K32_TRACE) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_TRACE) */


#if defined(IO_DEFS_MODULE_KALIMBA_INTERPROC_INT) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_KALIMBA_INTERPROC_INT
#define __IO_MAP_H__IO_MAP_MODULE_KALIMBA_INTERPROC_INT

/* -- kalimba_interproc_int -- Inter-processor interrupt generation registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  P0_TO_P1_INTERPROC_EVENT_1;          /* W    1 bits */
extern volatile uint32  P0_TO_P1_INTERPROC_EVENT_2;          /* W    1 bits */
extern volatile uint32  P1_TO_P0_INTERPROC_EVENT_1;          /* W    1 bits */
extern volatile uint32  P1_TO_P0_INTERPROC_EVENT_2;          /* W    1 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_KALIMBA_INTERPROC_INT */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_KALIMBA_INTERPROC_INT) */



#if defined(IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SQIF_WINDOWS
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SQIF_WINDOWS

/* -- apps_sys_sqif_windows -- SQIF Datapath Register Bank. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL;   /* RW   4 bits */
extern volatile apps_sys_sqif_windows__mutex_lock_enum  APPS_SYS_SQIF_WINDOWS_MUTEX_LOCK;    /* RW   4 bits */
extern volatile uint32  APPS_SYS_SQIF_WINDOW_OFFSET;         /* RW  32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SQIF_WINDOWS */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS) */

#if defined(IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_BANKED_EXCEPTIONS_P1
#define __IO_MAP_H__IO_MAP_MODULE_APPS_BANKED_EXCEPTIONS_P1

/* -- apps_banked_exceptions_p1 -- Control of banked exceptions for P1 -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL;  /* RW   4 bits */
extern volatile apps_banked_exceptions_p1__exception_source  APPS_BANKED_EXCEPTIONS_P1_ENABLES;   /* RW  20 bits */
extern volatile apps_banked_exceptions_p1__mutex_lock_enum  APPS_BANKED_EXCEPTIONS_P1_MUTEX_LOCK;  /* RW   4 bits */
extern volatile apps_banked_exceptions_p1__exception_source  APPS_BANKED_EXCEPTIONS_P1_STATUS;    /* RW  20 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_BANKED_EXCEPTIONS_P1 */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1) */

#if defined(IO_DEFS_MODULE_KALIMBA_READ_CACHE) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_KALIMBA_READ_CACHE
#define __IO_MAP_H__IO_MAP_MODULE_KALIMBA_READ_CACHE

/* -- kalimba_read_cache -- Kalimba read cache block configuration registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  KALIMBA_READ_CACHE_ACCESS_CTRL;      /* RW   4 bits */
extern volatile uint32  KALIMBA_READ_CACHE_CONTROL;          /* RW   7 bits */
extern volatile uint32  KALIMBA_READ_CACHE_DEBUG_EN;         /* W    1 bits */
extern volatile uint32  KALIMBA_READ_CACHE_DM_CONFIG;        /* RW   3 bits */
extern volatile uint32  KALIMBA_READ_CACHE_DM_HIT_COUNTER;   /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_DM_MISS_COUNTER;  /* RW  32 bits */
extern volatile kalimba_read_cache_invalidate_control  KALIMBA_READ_CACHE_INVALIDATE_CONTROL;  /* RW   2 bits */
extern volatile uint32  KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES;  /* RW  16 bits */
extern volatile uint32  KALIMBA_READ_CACHE_INVALIDATE_START_LINE;  /* RW  16 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MISS_LOG_ENABLE;  /* RW   1 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MISS_LOG_INDEX;   /* RW  16 bits */
extern volatile uint32  KALIMBA_READ_CACHE_MISS_LOG_SIZE;    /* RW  16 bits */
extern volatile kalimba_read_cache__mutex_lock_enum  KALIMBA_READ_CACHE_MUTEX_LOCK;       /* RW   4 bits */
extern volatile uint32  KALIMBA_READ_CACHE_PM_CONFIG;        /* RW   3 bits */
extern volatile uint32  KALIMBA_READ_CACHE_PM_HIT_COUNTER;   /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_PM_MISS_COUNTER;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM;  /* RW  32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_SOFT_RESET;       /* W    1 bits */

/* Declarations of read only registers */
extern volatile uint32  KALIMBA_READ_CACHE_DEBUG_DATA;       /* R   32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_PARAMS;           /* R   32 bits */
extern volatile uint32  KALIMBA_READ_CACHE_TRANSITION_STATUS;  /* R    4 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_KALIMBA_READ_CACHE */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_KALIMBA_READ_CACHE) */







#if defined(IO_DEFS_MODULE_BITSERIAL0) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_BITSERIAL0
#define __IO_MAP_H__IO_MAP_MODULE_BITSERIAL0

/* -- bitserial0 -- Bitserializer regs -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  BITSERIAL0_CLK_CONTROL;              /* RW  10 bits */
extern volatile uint32  BITSERIAL0_CLK_RATE_HI;              /* RW  16 bits */
extern volatile uint32  BITSERIAL0_CLK_RATE_LO;              /* RW  16 bits */
extern volatile uint32  BITSERIAL0_CLK_SAMP_OFFSET;          /* RW  16 bits */
extern volatile uint32  BITSERIAL0_CONFIG;                   /* RW  16 bits */
extern volatile uint32  BITSERIAL0_CONFIG2;                  /* RW  15 bits */
extern volatile uint32  BITSERIAL0_CONFIG3;                  /* RW   5 bits */
extern volatile uint32  BITSERIAL0_CONFIG_SEL_TIME;          /* RW  16 bits */
extern volatile uint32  BITSERIAL0_CONFIG_SEL_TIME2;         /* RW   4 bits */
extern volatile uint32  BITSERIAL0_CURATOR_SSID;             /* RW   4 bits */
extern volatile uint32  BITSERIAL0_DEBUG_SEL;                /* RW   9 bits */
extern volatile uint32  BITSERIAL0_DEEP_SLEEP_ACTIVE;        /* RW   1 bits */
extern volatile uint32  BITSERIAL0_DEGLITCH_EN;              /* RW   2 bits */
extern volatile uint32  BITSERIAL0_I2C_ADDRESS;              /* RW  10 bits */
extern volatile uint32  BITSERIAL0_INTERBYTE_SPACING;        /* RW  16 bits */
extern volatile uint32  BITSERIAL0_INTERRUPT_CLEAR;          /* RW  14 bits */
extern volatile uint32  BITSERIAL0_INTERRUPT_EN;             /* RW  14 bits */
extern volatile uint32  BITSERIAL0_INTERRUPT_SOURCE_CLEAR;   /* RW  14 bits */
extern volatile uint32  BITSERIAL0_REMOTE_SUBSYSTEM_SSID;    /* RW   4 bits */
extern volatile uint32  BITSERIAL0_RWB;                      /* RW   2 bits */
extern volatile uint32  BITSERIAL0_RX_BUFFER;                /* RW  12 bits */
extern volatile uint32  BITSERIAL0_RX_BUFFER2;               /* RW  12 bits */
extern volatile uint32  BITSERIAL0_SET_SEL_INACTIVE;         /* RW   1 bits */
extern volatile uint32  BITSERIAL0_SLAVE_DATA_READY_BYTE;    /* RW   8 bits */
extern volatile uint32  BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS;  /* RW  16 bits */
extern volatile uint32  BITSERIAL0_SLAVE_READ_COMMAND_BYTE;  /* RW   8 bits */
extern volatile uint32  BITSERIAL0_SLAVE_UNDERFLOW_BYTE;     /* RW   8 bits */
extern volatile uint32  BITSERIAL0_STREAMING_THRESHOLD;      /* RW  16 bits */
extern volatile uint32  BITSERIAL0_TXRX_LENGTH;              /* RW  16 bits */
extern volatile uint32  BITSERIAL0_TXRX_LENGTH2;             /* RW  16 bits */
extern volatile uint32  BITSERIAL0_TX_BUFFER;                /* RW  12 bits */
extern volatile uint32  BITSERIAL0_TX_BUFFER2;               /* RW  12 bits */
extern volatile uint32  BITSERIAL0_WORD_CONFIG;              /* RW  12 bits */

/* Declarations of read only registers */
extern volatile uint32  BITSERIAL0_ERROR_STATUS;             /* R    9 bits */
extern volatile uint32  BITSERIAL0_I2C_ACKS;                 /* R   16 bits */
extern volatile uint32  BITSERIAL0_INTERRUPT_SOURCE;         /* R   14 bits */
extern volatile uint32  BITSERIAL0_INTERRUPT_STATUS;         /* R   14 bits */
extern volatile uint32  BITSERIAL0_SLAVE_READ_COUNT;         /* R   16 bits */
extern volatile uint32  BITSERIAL0_SLAVE_WRITE_COUNT;        /* R   16 bits */
extern volatile uint32  BITSERIAL0_STATUS;                   /* R   13 bits */
extern volatile uint32  BITSERIAL0_WORDS_SENT;               /* R   16 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_BITSERIAL0 */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_BITSERIAL0) */




#if defined(IO_DEFS_MODULE_APPS_SYS_PIO) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_PIO
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_PIO

/* -- apps_sys_pio -- Apps subsystem PIO controls (registers appear per processor) -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  APPS_SYS_PIO_DRIVE[3];               /* RW  72 bits */
extern volatile uint32  APPS_SYS_PIO_DRIVE_WORD0;            /* RW */
extern volatile uint32  APPS_SYS_PIO_DRIVE_WORD1;            /* RW */
extern volatile uint32  APPS_SYS_PIO_DRIVE_WORD2;            /* RW */
extern volatile uint32  APPS_SYS_PIO_DRIVE_ENABLE[3];        /* RW  72 bits */
extern volatile uint32  APPS_SYS_PIO_DRIVE_ENABLE_WORD0;     /* RW */
extern volatile uint32  APPS_SYS_PIO_DRIVE_ENABLE_WORD1;     /* RW */
extern volatile uint32  APPS_SYS_PIO_DRIVE_ENABLE_WORD2;     /* RW */

/* Declarations of read only registers */
extern volatile uint32  APPS_SYS_PIO_STATUS[3];              /* R   72 bits */
extern volatile uint32  APPS_SYS_PIO_STATUS_WORD0;           /* R */
extern volatile uint32  APPS_SYS_PIO_STATUS_WORD1;           /* R */
extern volatile uint32  APPS_SYS_PIO_STATUS_WORD2;           /* R */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_PIO */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_PIO) */


#if defined(IO_DEFS_MODULE_SQIF) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_SQIF
#define __IO_MAP_H__IO_MAP_MODULE_SQIF

/* -- sqif -- Serial Quad I/O Flash registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  SQIF_ACCESS_CTRL;                    /* RW   4 bits */
extern volatile uint32  SQIF_CONF;                           /* RW  12 bits */
extern volatile uint32  SQIF_CONF2;                          /* RW  13 bits */
extern volatile uint32  SQIF_CORR_CFG;                       /* RW  14 bits */
extern volatile uint32  SQIF_CTRL;                           /* RW  12 bits */
extern volatile uint32  SQIF_DDR_DBG_SEL;                    /* RW   8 bits */
extern volatile uint32  SQIF_DEBUG_SEL;                      /* RW   4 bits */
extern volatile uint32  SQIF_FLASH_SIZE;                     /* RW   6 bits */
extern volatile uint32  SQIF_INST;                           /* RW  16 bits */
extern volatile sqif__mutex_lock_enum  SQIF_MUTEX_LOCK;                     /* RW   4 bits */
extern volatile uint32  SQIF_PAD_CAL;                        /* RW   5 bits */
extern volatile uint32  SQIF_PEEK_GO;                        /* RW   1 bits */
extern volatile uint32  SQIF_PHASE_SEL0;                     /* RW  15 bits */
extern volatile uint32  SQIF_PHASE_SEL1;                     /* RW  15 bits */
extern volatile uint32  SQIF_PHASE_SEL2;                     /* RW  11 bits */
extern volatile uint32  SQIF_POKE;                           /* RW   8 bits */
extern volatile uint32  SQIF_POKE_LAST;                      /* RW   8 bits */
extern volatile uint32  SQIF_RAM_CONF;                       /* RW   8 bits */

/* Declarations of read only registers */
extern volatile uint32  SQIF_DDR_CENTER;                     /* R   10 bits */
extern volatile uint32  SQIF_DDR_LOCKED;                     /* R    4 bits */
extern volatile uint32  SQIF_DDR_MARGIN;                     /* R   10 bits */
extern volatile uint32  SQIF_PAD_DBG;                        /* R    6 bits */
extern volatile uint32  SQIF_PEEK;                           /* R    8 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_SQIF */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_SQIF) */


#if defined(IO_DEFS_MODULE_APPS_SYS_CPU1_VM) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU1_VM
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU1_VM

/* -- apps_sys_cpu1_vm -- CPU1 Apps-specific virtual memory registers -- */

/* Declarations of read/write/mixed registers */
extern volatile apps_sys_cpu1_vm__vm_buffer_access_do_action_enum  VM_BUFFER_ACCESS_CPU1_DO_ACTION;     /* RW   1 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU1_HANDLE;        /* RW  12 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE;  /* RW  18 bits */
extern volatile uint32  VM_CPU1_ACCESS_CTRL;                 /* RW   4 bits */
extern volatile uint32  VM_CPU1_CLEAR_PORT;                  /* RW   2 bits */
extern volatile apps_sys_cpu1_vm__mutex_lock_enum  VM_CPU1_MUTEX_LOCK;                  /* RW   4 bits */

/* Declarations of read only registers */
extern volatile uint32  VM_BUFFER_ACCESS_CPU1_OFFSET_READ;   /* R   18 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU1_STATUS;        /* R    5 bits */
extern volatile uint32  VM_REQUEST_CPU1_READ_STATUS;         /* R   18 bits */
extern volatile uint32  VM_REQUEST_CPU1_WRITE_STATUS;        /* R   18 bits */

/* Declarations of register aliases*/
extern volatile uint32  MMU_BUFFER_ACCESS_CPU1_HANDLE;       /* A   12 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU1_OFFSET_WRITE;  /* A   18 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU1_OFFSET_READ;  /* A   18 bits */
extern volatile apps_sys_cpu1_vm__vm_buffer_access_do_action_enum  MMU_BUFFER_ACCESS_CPU1_DO_ACTION;    /* A    1 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU1_STATUS;       /* A    5 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU1_VM */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_CPU1_VM) */


#if defined(IO_DEFS_MODULE_HOST_SYS_CONFIG) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_HOST_SYS_CONFIG
#define __IO_MAP_H__IO_MAP_MODULE_HOST_SYS_CONFIG

/* -- host_sys_config -- Host subsystem interface control registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_BTM_DEBUG_SELECT;           /* RW   6 bits */
extern volatile uint32  HOST_SYS_CLK_80M_DIV;                /* RW   8 bits */
extern volatile host_sys_block_ids  HOST_SYS_CONFIG_ENABLES;             /* RW   5 bits */
extern volatile uint32  HOST_SYS_CONFIG_FORCE_CLK_ENABLES;   /* RW   7 bits */
extern volatile host_sys_block_ids  HOST_SYS_CONFIG_MASK_HW_CLK_REQS;    /* RW   5 bits */
extern volatile uint32  HOST_SYS_DEBUG_SELECT;               /* RW   8 bits */
extern volatile uint32  HOST_SYS_MMU_RAM_SEQ_TIMING_REG0;    /* RW  16 bits */
extern volatile uint32  HOST_SYS_MMU_RAM_SEQ_TIMING_REG1;    /* RW  16 bits */
extern volatile uint32  HOST_SYS_MMU_RAM_SEQ_TIMING_REG2;    /* RW  12 bits */
extern volatile host_sys_block_ids  HOST_SYS_POWER_ENABLES;              /* RW   5 bits */
extern volatile uint32  HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN;  /* RW  16 bits */
extern volatile uint32  HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_UART_CTSB_INPUT_PIO_CTRL;   /* RW   7 bits */
extern volatile uint32  HOST_SYS_UART_DATA_INPUT_PIO_CTRL;   /* RW   7 bits */
extern volatile uint32  HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL;  /* RW   7 bits */
extern volatile uint32  USB_TX_PREFETCH_RAM_16EPS_RAM0_EMA;  /* RW   5 bits */
extern volatile uint32  USB_TX_PREFETCH_RAM_16EPS_RAM1_EMA;  /* RW   5 bits */
extern volatile uint32  USB_TX_PREFETCH_RAM_16EPS_RAM2_EMA;  /* RW   5 bits */
extern volatile uint32  USB_TX_PREFETCH_RAM_16EPS_RAM3_EMA;  /* RW   5 bits */

/* Declarations of read only registers */
extern volatile uint32  HOST_SYS_BUILD_OPTIONS;              /* R    6 bits */
extern volatile host_sys_block_ids  HOST_SYS_HIFS_BUILT;                 /* R    5 bits */
extern volatile host_sys_block_ids  HOST_SYS_POWER_STATUS;               /* R    5 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_HOST_SYS_CONFIG */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_HOST_SYS_CONFIG) */

#if defined(IO_DEFS_MODULE_K32_CORE) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_CORE) */




#if defined(IO_DEFS_MODULE_APPS_SYS_SYS) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SYS
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SYS

/* -- apps_sys_sys -- Auto-generated from subsystem manifest TMux Config. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  APPS_FORCE_PWM_MODE_REQ;             /* RW   2 bits */
extern volatile uint32  APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL;  /* RW   4 bits */
extern volatile uint32  APPS_SYS_ACCESS_CTRL;                /* RW   4 bits */
extern volatile uint32  APPS_SYS_CACHE_SEL;                  /* RW   1 bits */
extern volatile uint32  APPS_SYS_CPU0_TCM_REMAP_CONTROLS;    /* RW   4 bits */
extern volatile uint32  APPS_SYS_CPU1_TCM_REMAP_CONTROLS;    /* RW   4 bits */
extern volatile uint32  APPS_SYS_DM0_ACCESS_RESTRICT;        /* RW   2 bits */
extern volatile uint32  APPS_SYS_INTERCONNECT_DEBUG_CONTROL;  /* RW  15 bits */
extern volatile apps_sys_sys__mutex_lock_enum  APPS_SYS_MUTEX_LOCK;                 /* RW   4 bits */
extern volatile uint32  APPS_SYS_PIO_MUX[3];                 /* RW  72 bits */
extern volatile uint32  APPS_SYS_PIO_MUX_WORD0;              /* RW */
extern volatile uint32  APPS_SYS_PIO_MUX_WORD1;              /* RW */
extern volatile uint32  APPS_SYS_PIO_MUX_WORD2;              /* RW */
extern volatile uint32  APPS_SYS_SQIF0_PRESENT;              /* RW   2 bits */
extern volatile uint32  APPS_SYS_SQIF1_PRESENT;              /* RW   2 bits */
extern volatile uint32  APPS_SYS_SQIF_SEL;                   /* RW   1 bits */
extern volatile apps_sys_sys__apps_sys_sqif_window_offset_index  APPS_SYS_SQIF_WINDOW_CONTROL;        /* RW   5 bits */
extern volatile uint32  APPS_SYS_TBUS_WINDOW_EN;             /* RW   2 bits */
extern volatile uint32  APPS_SYS_TBUS_WINDOW_IDS;            /* RW   8 bits */
extern volatile uint32  APPS_SYS_TCM_LOCK;                   /* RW   2 bits */
extern volatile uint32  APPS_SYS_TEST_REG;                   /* RW  32 bits */
extern volatile apps_sys_debug_source  SUB_SYS_DEBUG_SELECT_HIGH;           /* RW   7 bits */
extern volatile apps_sys_debug_source  SUB_SYS_DEBUG_SELECT_LOW;            /* RW   7 bits */
extern volatile uint32  SUB_SYS_DEBUG_SELECT_SHIFT;          /* RW   2 bits */
extern volatile uint32  SUB_SYS_RST_STATUS;                  /* RW   3 bits */
extern volatile uint32  SUB_SYS_TRANSACTION_MUX_DBG_SEL;     /* RW   6 bits */

/* Declarations of read only registers */
extern volatile uint32  APPS_SMPS_IN_PWM_MODE_STATUS;        /* R    2 bits */
extern volatile uint32  RAM_SEQUENCER_ACTIVE_STATUS;         /* R   11 bits */
extern volatile uint32  SUB_SYS_CHIP_VERSION;                /* R   16 bits */
extern volatile uint32  SUB_SYS_DEBUG_STATUS;                /* R   32 bits */
extern volatile uint32  SUB_SYS_ID;                          /* R    4 bits */
extern volatile uint32  SUB_SYS_REG_SRC_HASH;                /* R   16 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_SYS */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_SYS) */

#if defined(IO_DEFS_MODULE_K32_MONITOR) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_MONITOR) */

#if defined(IO_DEFS_MODULE_LED_CTRL) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_LED_CTRL
#define __IO_MAP_H__IO_MAP_MODULE_LED_CTRL

/* -- led_ctrl -- Control registers for LED controllers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  LED_CONFIGURE;                       /* RW   1 bits */
extern volatile uint32  LED_COUNTHOLD_VALUE;                 /* RW  16 bits */
extern volatile uint32  LED_CTRL_CLK_ENABLE;                 /* W    1 bits */
extern volatile uint32  LED_CTRL_SOFT_RESET;                 /* W    1 bits */
extern volatile uint32  LED_DEBUG_SELECT;                    /* RW   4 bits */
extern volatile uint32  LED_EN;                              /* RW   6 bits */
extern volatile uint32  LED_HOLD_HIGH_CONFIG;                /* RW  16 bits */
extern volatile uint32  LED_HOLD_LOW_CONFIG;                 /* RW  16 bits */
extern volatile uint32  LED_INDEX;                           /* RW   4 bits */
extern volatile uint32  LED_LOGARITHMIC_EN;                  /* RW   1 bits */
extern volatile uint32  LED_LOGARITHMIC_OFFSET_HIGH;         /* RW   4 bits */
extern volatile uint32  LED_LOGARITHMIC_OFFSET_LOW;          /* RW   4 bits */
extern volatile uint32  LED_MAX_HIGH_CONFIG;                 /* RW  16 bits */
extern volatile uint32  LED_MAX_LOW_CONFIG;                  /* RW  16 bits */
extern volatile uint32  LED_MIN_HIGH_CONFIG;                 /* RW  16 bits */
extern volatile uint32  LED_MIN_LOW_CONFIG;                  /* RW  16 bits */
extern volatile uint32  LED_PIN_CONFIG;                      /* RW   2 bits */
extern volatile uint32  LED_RAMP_CONFIG;                     /* RW  16 bits */
extern volatile uint32  LED_RAMP_CURRENT_HIGH_CONFIG;        /* RW  16 bits */
extern volatile uint32  LED_RAMP_CURRENT_LOW_CONFIG;         /* RW  16 bits */
extern volatile uint32  LED_RAMP_SHIFT;                      /* RW   3 bits */
extern volatile uint32  LED_SINGLE_SHOT_HIGH_CONFIG;         /* RW  16 bits */
extern volatile uint32  LED_SINGLE_SHOT_LOW_CONFIG;          /* RW  16 bits */
extern volatile uint32  LED_SINGLE_SHOT_MODE;                /* RW   6 bits */
extern volatile uint32  LED_START_UP_STATE;                  /* RW   3 bits */
extern volatile uint32  LED_UPDATE;                          /* RW   6 bits */

/* Declarations of read only registers */
extern volatile uint32  LED_HOLD_HIGH_CONFIG_STATUS;         /* R   16 bits */
extern volatile uint32  LED_HOLD_LOW_CONFIG_STATUS;          /* R   16 bits */
extern volatile uint32  LED_LOGARITHMIC_STATUS;              /* R   12 bits */
extern volatile uint32  LED_MAX_HIGH_CONFIG_STATUS;          /* R   16 bits */
extern volatile uint32  LED_MAX_LOW_CONFIG_STATUS;           /* R   16 bits */
extern volatile uint32  LED_MIN_HIGH_CONFIG_STATUS;          /* R   16 bits */
extern volatile uint32  LED_MIN_LOW_CONFIG_STATUS;           /* R   16 bits */
extern volatile uint32  LED_PIN_CONFIG_STATUS;               /* R    2 bits */
extern volatile uint32  LED_RAMP_CONFIG_STATUS;              /* R   16 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_LED_CTRL */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_LED_CTRL) */



#if defined(IO_DEFS_MODULE_BITSERIAL1) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_BITSERIAL1
#define __IO_MAP_H__IO_MAP_MODULE_BITSERIAL1

/* -- bitserial1 -- Bitserializer regs -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  BITSERIAL1_CLK_CONTROL;              /* RW  10 bits */
extern volatile uint32  BITSERIAL1_CLK_RATE_HI;              /* RW  16 bits */
extern volatile uint32  BITSERIAL1_CLK_RATE_LO;              /* RW  16 bits */
extern volatile uint32  BITSERIAL1_CLK_SAMP_OFFSET;          /* RW  16 bits */
extern volatile uint32  BITSERIAL1_CONFIG;                   /* RW  16 bits */
extern volatile uint32  BITSERIAL1_CONFIG2;                  /* RW  15 bits */
extern volatile uint32  BITSERIAL1_CONFIG3;                  /* RW   5 bits */
extern volatile uint32  BITSERIAL1_CONFIG_SEL_TIME;          /* RW  16 bits */
extern volatile uint32  BITSERIAL1_CONFIG_SEL_TIME2;         /* RW   4 bits */
extern volatile uint32  BITSERIAL1_CURATOR_SSID;             /* RW   4 bits */
extern volatile uint32  BITSERIAL1_DEBUG_SEL;                /* RW   9 bits */
extern volatile uint32  BITSERIAL1_DEEP_SLEEP_ACTIVE;        /* RW   1 bits */
extern volatile uint32  BITSERIAL1_DEGLITCH_EN;              /* RW   2 bits */
extern volatile uint32  BITSERIAL1_I2C_ADDRESS;              /* RW  10 bits */
extern volatile uint32  BITSERIAL1_INTERBYTE_SPACING;        /* RW  16 bits */
extern volatile uint32  BITSERIAL1_INTERRUPT_CLEAR;          /* RW  14 bits */
extern volatile uint32  BITSERIAL1_INTERRUPT_EN;             /* RW  14 bits */
extern volatile uint32  BITSERIAL1_INTERRUPT_SOURCE_CLEAR;   /* RW  14 bits */
extern volatile uint32  BITSERIAL1_REMOTE_SUBSYSTEM_SSID;    /* RW   4 bits */
extern volatile uint32  BITSERIAL1_RWB;                      /* RW   2 bits */
extern volatile uint32  BITSERIAL1_RX_BUFFER;                /* RW  12 bits */
extern volatile uint32  BITSERIAL1_RX_BUFFER2;               /* RW  12 bits */
extern volatile uint32  BITSERIAL1_SET_SEL_INACTIVE;         /* RW   1 bits */
extern volatile uint32  BITSERIAL1_SLAVE_DATA_READY_BYTE;    /* RW   8 bits */
extern volatile uint32  BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS;  /* RW  16 bits */
extern volatile uint32  BITSERIAL1_SLAVE_READ_COMMAND_BYTE;  /* RW   8 bits */
extern volatile uint32  BITSERIAL1_SLAVE_UNDERFLOW_BYTE;     /* RW   8 bits */
extern volatile uint32  BITSERIAL1_STREAMING_THRESHOLD;      /* RW  16 bits */
extern volatile uint32  BITSERIAL1_TXRX_LENGTH;              /* RW  16 bits */
extern volatile uint32  BITSERIAL1_TXRX_LENGTH2;             /* RW  16 bits */
extern volatile uint32  BITSERIAL1_TX_BUFFER;                /* RW  12 bits */
extern volatile uint32  BITSERIAL1_TX_BUFFER2;               /* RW  12 bits */
extern volatile uint32  BITSERIAL1_WORD_CONFIG;              /* RW  12 bits */

/* Declarations of read only registers */
extern volatile uint32  BITSERIAL1_ERROR_STATUS;             /* R    9 bits */
extern volatile uint32  BITSERIAL1_I2C_ACKS;                 /* R   16 bits */
extern volatile uint32  BITSERIAL1_INTERRUPT_SOURCE;         /* R   14 bits */
extern volatile uint32  BITSERIAL1_INTERRUPT_STATUS;         /* R   14 bits */
extern volatile uint32  BITSERIAL1_SLAVE_READ_COUNT;         /* R   16 bits */
extern volatile uint32  BITSERIAL1_SLAVE_WRITE_COUNT;        /* R   16 bits */
extern volatile uint32  BITSERIAL1_STATUS;                   /* R   13 bits */
extern volatile uint32  BITSERIAL1_WORDS_SENT;               /* R   16 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_BITSERIAL1 */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_BITSERIAL1) */




#if defined(IO_DEFS_MODULE_K32_MISC) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_MISC) */



#if defined(IO_DEFS_MODULE_K32_TIMERS) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_TIMERS) */

#if defined(IO_DEFS_MODULE_K32_PREFETCH) 

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
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_PREFETCH) */

#if defined(IO_DEFS_MODULE_APPS_SYS_CPU0_VM) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU0_VM
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU0_VM

/* -- apps_sys_cpu0_vm -- Apps-specific virtual memory registers -- */

/* Declarations of read/write/mixed registers */
extern volatile vm_buffer_access_do_action_enum  VM_BUFFER_ACCESS_CPU0_DO_ACTION;     /* RW   1 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU0_HANDLE;        /* RW  12 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE;  /* RW  18 bits */
extern volatile uint32  VM_CPU0_ACCESS_CTRL;                 /* RW   4 bits */
extern volatile uint32  VM_CPU0_CLEAR_PORT;                  /* RW   2 bits */
extern volatile apps_sys_cpu0_vm__mutex_lock_enum  VM_CPU0_MUTEX_LOCK;                  /* RW   4 bits */
extern volatile vm_debug_select  VM_DEBUG_SELECT;                     /* RW   5 bits */
extern volatile uint32  VM_FREE_LIST_ADDR;                   /* RW  32 bits */
extern volatile uint32  VM_FREE_LIST_ADD_PAGE;               /* RW  16 bits */
extern volatile uint32  VM_FREE_LIST_END_ADDR;               /* RW  32 bits */
extern volatile uint32  VM_INTERCONNECT_MUX_DEBUG_ENABLE;    /* RW   1 bits */
extern volatile uint32  VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES;  /* RW   1 bits */
extern volatile uint32  VM_LOOKUP_ERROR_FLAGS_CLEAR;         /* RW  16 bits */
extern volatile uint32  VM_LOOKUP_SELECT;                    /* RW   4 bits */
extern volatile uint32  VM_REQUEST_CPU0_READ_REMOTE_CONFIG;  /* RW  14 bits */
extern volatile uint32  VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG;  /* RW  14 bits */
extern volatile uint32  VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR;  /* W    1 bits */

/* Declarations of read only registers */
extern volatile uint32  VM_BUFFER_ACCESS_CPU0_OFFSET_READ;   /* R   18 bits */
extern volatile uint32  VM_BUFFER_ACCESS_CPU0_STATUS;        /* R    5 bits */
extern volatile uint32  VM_FREE_LIST_NEXT_ADDR;              /* R   32 bits */
extern volatile uint32  VM_LOOKUP_ERROR_FLAGS;               /* R   16 bits */
extern volatile uint32  VM_LOOKUP_INT_EVENTS_STICKY;         /* R   16 bits */
extern volatile uint32  VM_REQUEST_CPU0_READ_STATUS;         /* R   18 bits */
extern volatile uint32  VM_REQUEST_CPU0_WRITE_STATUS;        /* R   18 bits */

/* Declarations of register aliases*/
extern volatile uint32  MMU_FREE_LIST_ADDR;                  /* A   32 bits */
extern volatile uint32  MMU_FREE_LIST_NEXT_ADDR;             /* A   32 bits */
extern volatile uint32  MMU_FREE_LIST_END_ADDR;              /* A   32 bits */
extern volatile uint32  MMU_FREE_LIST_ADD_PAGE;              /* A   16 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU0_HANDLE;       /* A   12 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU0_OFFSET_WRITE;  /* A   18 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU0_OFFSET_READ;  /* A   18 bits */
extern volatile vm_buffer_access_do_action_enum  MMU_BUFFER_ACCESS_CPU0_DO_ACTION;    /* A    1 bits */
extern volatile uint32  MMU_BUFFER_ACCESS_CPU0_STATUS;       /* A    5 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CPU0_VM */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_CPU0_VM) */

#if defined(IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_KALIMBA_HW_SEMAPHORES
#define __IO_MAP_H__IO_MAP_MODULE_KALIMBA_HW_SEMAPHORES

/* -- kalimba_hw_semaphores -- Hardware semaphores interface registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  SEMAPHORE_ADDR;                      /* RW   6 bits */
extern volatile uint32  SEMAPHORE_DATA;                      /* RW   1 bits */

/* Declarations of read only registers */
extern volatile uint32  SEMAPHORE_STATUS;                    /* R    1 bits */
extern volatile uint32  SEMAPHORE_STATUS_RAW;                /* R    1 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_KALIMBA_HW_SEMAPHORES */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES) */


#if defined(IO_DEFS_MODULE_APPS_SYS_CLKGEN) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CLKGEN
#define __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CLKGEN

/* -- apps_sys_clkgen -- Applictions subsystem clock generation registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  CLKGEN_ACCESS_CTRL;                  /* RW   4 bits */
extern volatile uint32  CLKGEN_CORE_CLK_RATE;                /* RW   4 bits */
extern volatile uint32  CLKGEN_DEBUG;                        /* RW   4 bits */
extern volatile uint32  CLKGEN_ENABLES;                      /* RW  30 bits */
extern volatile apps_sys_clkgen__mutex_lock_enum  CLKGEN_MUTEX_LOCK;                   /* RW   4 bits */
extern volatile uint32  CLKGEN_SDIO_HOST_CLK_CONFIG;         /* RW   4 bits */
extern volatile uint32  CLKGEN_TIMER_ENABLES;                /* RW   2 bits */

/* Declarations of read only registers */
extern volatile uint32  CLKGEN_STATUS_SLOW_CLK;              /* R    1 bits */
extern volatile uint32  CLKGEN_TIMER_FAST_STATUS;            /* R   16 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_APPS_SYS_CLKGEN */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_APPS_SYS_CLKGEN) */



#if defined(IO_DEFS_MODULE_K32_INTERRUPT) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_K32_INTERRUPT
#define __IO_MAP_H__IO_MAP_MODULE_K32_INTERRUPT

/* -- k32_interrupt -- Kalimba 32-bit Interrupt Controller Control registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  INT_ACK;                             /* RW   1 bits */
extern volatile uint32  INT_ADDR;                            /* RW  32 bits */
extern volatile uint32  INT_BLOCK_PRIORITY;                  /* RW   2 bits */
extern volatile uint32  INT_CLK_SWITCH_EN;                   /* RW   1 bits */
extern volatile k32_interrupt__clock_divide_rate_enum  INT_CLOCK_DIVIDE_RATE;               /* RW   2 bits */
extern volatile uint32  INT_GBL_ENABLE;                      /* RW   1 bits */
extern volatile uint32  INT_LOAD_INFO;                       /* RW  20 bits */
extern volatile uint32  INT_PRIORITY;                        /* RW   2 bits */
extern volatile uint32  INT_SELECT;                          /* RW   5 bits */
extern volatile uint32  INT_SOURCES_EN;                      /* RW  32 bits */
extern volatile uint32  INT_SW0_EVENT;                       /* RW   1 bits */
extern volatile uint32  INT_SW1_EVENT;                       /* RW   1 bits */
extern volatile uint32  INT_UNBLOCK;                         /* RW   1 bits */

/* Declarations of read only registers */
extern volatile uint32  INT_SAVE_INFO;                       /* R   22 bits */
extern volatile uint32  INT_SOURCE;                          /* R    5 bits */
extern volatile uint32  INT_STATUS;                          /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_K32_INTERRUPT */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_K32_INTERRUPT) */




#if defined(IO_DEFS_MODULE_SQIF_DATAPATH) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_SQIF_DATAPATH
#define __IO_MAP_H__IO_MAP_MODULE_SQIF_DATAPATH

/* -- sqif_datapath -- SQIF Datapath Registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  SQIF_DATAPATH_ACCESS_CTRL;           /* RW   4 bits */
extern volatile uint32  SQIF_DATAPATH_BANK;                  /* RW  15 bits */
extern volatile sqif_datapath__mutex_lock_enum  SQIF_DATAPATH_MUTEX_LOCK;            /* RW   4 bits */
extern volatile uint32  SQIF_DATAPATH_STATUS;                /* RW   2 bits */

/* Declarations of read only registers */
extern volatile uint32  SQIF_DATAPATH_PERF_COUNTER;          /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_SQIF_DATAPATH */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_SQIF_DATAPATH) */




#if defined(IO_DEFS_MODULE_KALIMBA_PIO_INT) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_KALIMBA_PIO_INT
#define __IO_MAP_H__IO_MAP_MODULE_KALIMBA_PIO_INT

/* -- kalimba_pio_int -- Generic Kalimba Timer registers -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  KALIMBA_PIO_INT_ACCESS_CTRL;         /* RW   4 bits */
extern volatile kalimba_pio_int__mutex_lock_enum  KALIMBA_PIO_INT_MUTEX_LOCK;          /* RW   4 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_CLEAR_DATA[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_CLEAR_DATA_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_CLEAR_DATA_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_CLEAR_DATA_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_FALLING_ENABLE[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_FALLING_ENABLE_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_FALLING_ENABLE_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_FALLING_ENABLE_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_RISING_ENABLE[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_RISING_ENABLE_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_RISING_ENABLE_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_RISING_ENABLE_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_CLEAR_DATA[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_CLEAR_DATA_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_CLEAR_DATA_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_CLEAR_DATA_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_FALLING_ENABLE[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_FALLING_ENABLE_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_FALLING_ENABLE_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_FALLING_ENABLE_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_RISING_ENABLE[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_RISING_ENABLE_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_RISING_ENABLE_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_RISING_ENABLE_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_ENABLES;       /* RW  14 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD;  /* RW  16 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME;  /* RW  32 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD;  /* RW  16 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME;  /* RW  32 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE;  /* RW   7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT;  /* RW  15 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_INVERT[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_INVERT_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_INVERT_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_INVERT_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_MASK[3];  /* RW  72 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_MASK_WORD0;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_MASK_WORD1;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_MASK_WORD2;  /* RW */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME;  /* RW  32 bits */

/* Declarations of read only registers */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE[3];  /* R   72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_WORD0;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_WORD1;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_PIO1_EVENT_CAUSE_WORD2;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE[3];  /* R   72 bits */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_WORD0;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_WORD1;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_PIO2_EVENT_CAUSE_WORD2;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME;  /* R   32 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE;  /* R    7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME;  /* R   32 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE;  /* R    7 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_STATUS[3];  /* R   72 bits */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_STATUS_WORD0;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_STATUS_WORD1;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_SW3_EVENT_PIO_STATUS_WORD2;  /* R */
extern volatile uint32  KALIMBA_PIO_INT_TIMER_TIME2;         /* R   32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_KALIMBA_PIO_INT */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_KALIMBA_PIO_INT) */

#if defined(IO_DEFS_MODULE_BUS_PROC_STREAM) 

#ifndef __IO_MAP_H__IO_MAP_MODULE_BUS_PROC_STREAM
#define __IO_MAP_H__IO_MAP_MODULE_BUS_PROC_STREAM

/* -- bus_proc_stream -- Processor-driven transaction stream generation control registers. -- */

/* Declarations of read/write/mixed registers */
extern volatile uint32  BUS_PROC_STREAM_ACCESS_CTRL;         /* RW   4 bits */
extern volatile uint32  BUS_PROC_STREAM_CONFIG;              /* RW  12 bits */
extern volatile uint32  BUS_PROC_STREAM_FINAL;               /* RW  16 bits */
extern volatile bus_proc_stream__mutex_lock_enum  BUS_PROC_STREAM_MUTEX_LOCK;          /* RW   4 bits */
extern volatile uint32  BUS_PROC_STREAM_PAYLOAD0;            /* RW  32 bits */
extern volatile uint32  BUS_PROC_STREAM_PAYLOAD1;            /* RW  32 bits */

#endif /* __IO_MAP_H__IO_MAP_MODULE_BUS_PROC_STREAM */
#endif /* !defined(IO_DEFS_LIMIT_MODULES) || defined(IO_DEFS_MODULE_BUS_PROC_STREAM) */



