
/* *************************************************************************  *
   COMMERCIAL IN CONFIDENCE
   Copyright (C) 2017 Qualcomm Technologies International Ltd.

 


   DESCRIPTION
      Hardware declarations header file (higher level).
      Lists masks and values for use when setting or interpreting
      the contents of memory mapped hardware registers.

   INTERFACE
      Entry   :-
      Exit    :-

   MODIFICATIONS
      1.0    24/06/99    RWY    First created.
      1.x    xx/xx/xx    RWY    Automatically generated.

*  *************************************************************************  */

#define __IO_DEFS_H__

#define CHIP_REGISTER_HASH 0x1F1C


#if defined(IO_DEFS_MODULE_K32_TRACE) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_TRACE
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_TRACE

/* -- k32_trace -- Kalimba 32-bit Trace Control registers. -- */

enum trace_cfg_enum_posn_enum
{
   TRACE_TRIGGER_CFG_END_TRIG_EN_POSN                 = (int)0,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_END_TRIG_EN_LSB_POSN = (int)0,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_END_TRIG_EN_MSB_POSN = (int)0,
   TRACE_TRIGGER_CFG_START_TRIG_EN_POSN               = (int)1,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_START_TRIG_EN_LSB_POSN = (int)1,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_START_TRIG_EN_MSB_POSN = (int)1,
   TRACE_TRIGGER_CFG_TRIGGER_LENGTH_LSB_POSN          = (int)2,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_LSB_POSN = (int)2,
   TRACE_TRIGGER_CFG_TRIGGER_LENGTH_MSB_POSN          = (int)11,
   TRACE_CFG_ENUM_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_MSB_POSN = (int)11
};
typedef enum trace_cfg_enum_posn_enum trace_cfg_enum_posn;

#define TRACE_TRIGGER_CFG_END_TRIG_EN_MASK       (0x00000001u)
#define TRACE_TRIGGER_CFG_START_TRIG_EN_MASK     (0x00000002u)
#define TRACE_TRIGGER_CFG_TRIGGER_LENGTH_LSB_MASK (0x00000004u)
#define TRACE_TRIGGER_CFG_TRIGGER_LENGTH_MSB_MASK (0x00000800u)

enum trace_trig_status_enum_posn_enum
{
   TRACE_TRIGGER_STATUS_START_FOUND_POSN              = (int)0,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_START_FOUND_LSB_POSN = (int)0,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_START_FOUND_MSB_POSN = (int)0,
   TRACE_TRIGGER_STATUS_START_COMPL_POSN              = (int)1,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_START_COMPL_LSB_POSN = (int)1,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_START_COMPL_MSB_POSN = (int)1,
   TRACE_TRIGGER_STATUS_END_FOUND_POSN                = (int)2,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_END_FOUND_LSB_POSN = (int)2,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_END_FOUND_MSB_POSN = (int)2,
   TRACE_TRIGGER_STATUS_END_COMPL_POSN                = (int)3,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_END_COMPL_LSB_POSN = (int)3,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_END_COMPL_MSB_POSN = (int)3,
   TRACE_TRIGGER_STATUS_SMDBG_LSB_POSN                = (int)4,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_SMDBG_LSB_POSN = (int)4,
   TRACE_TRIGGER_STATUS_SMDBG_MSB_POSN                = (int)5,
   TRACE_TRIG_STATUS_ENUM_TRACE_TRIGGER_STATUS_SMDBG_MSB_POSN = (int)5
};
typedef enum trace_trig_status_enum_posn_enum trace_trig_status_enum_posn;

#define TRACE_TRIGGER_STATUS_START_FOUND_MASK    (0x00000001u)
#define TRACE_TRIGGER_STATUS_START_COMPL_MASK    (0x00000002u)
#define TRACE_TRIGGER_STATUS_END_FOUND_MASK      (0x00000004u)
#define TRACE_TRIGGER_STATUS_END_COMPL_MASK      (0x00000008u)
#define TRACE_TRIGGER_STATUS_SMDBG_LSB_MASK      (0x00000010u)
#define TRACE_TRIGGER_STATUS_SMDBG_MSB_MASK      (0x00000020u)

enum trace_0_cfg_posn_enum
{
   TRACE_CFG_0_ENABLE_POSN                            = (int)0,
   TRACE_0_CFG_TRACE_CFG_0_ENABLE_LSB_POSN            = (int)0,
   TRACE_0_CFG_TRACE_CFG_0_ENABLE_MSB_POSN            = (int)0,
   TRACE_CFG_0_SYNC_INTERVAL_LSB_POSN                 = (int)1,
   TRACE_0_CFG_TRACE_CFG_0_SYNC_INTERVAL_LSB_POSN     = (int)1,
   TRACE_CFG_0_SYNC_INTERVAL_MSB_POSN                 = (int)3,
   TRACE_0_CFG_TRACE_CFG_0_SYNC_INTERVAL_MSB_POSN     = (int)3,
   TRACE_CFG_0_CPU_SELECT_LSB_POSN                    = (int)4,
   TRACE_0_CFG_TRACE_CFG_0_CPU_SELECT_LSB_POSN        = (int)4,
   TRACE_CFG_0_CPU_SELECT_MSB_POSN                    = (int)5,
   TRACE_0_CFG_TRACE_CFG_0_CPU_SELECT_MSB_POSN        = (int)5,
   TRACE_CFG_0_FLUSH_FIFO_POSN                        = (int)6,
   TRACE_0_CFG_TRACE_CFG_0_FLUSH_FIFO_LSB_POSN        = (int)6,
   TRACE_0_CFG_TRACE_CFG_0_FLUSH_FIFO_MSB_POSN        = (int)6,
   TRACE_CFG_0_STALL_CORE_ON_TRACE_FULL_POSN          = (int)7,
   TRACE_0_CFG_TRACE_CFG_0_STALL_CORE_ON_TRACE_FULL_LSB_POSN = (int)7,
   TRACE_0_CFG_TRACE_CFG_0_STALL_CORE_ON_TRACE_FULL_MSB_POSN = (int)7,
   TRACE_CFG_0_CLR_STORED_ON_SYNC_POSN                = (int)8,
   TRACE_0_CFG_TRACE_CFG_0_CLR_STORED_ON_SYNC_LSB_POSN = (int)8,
   TRACE_0_CFG_TRACE_CFG_0_CLR_STORED_ON_SYNC_MSB_POSN = (int)8,
   TRACE_CFG_0_FLUSH_BITGEN_POSN                      = (int)9,
   TRACE_0_CFG_TRACE_CFG_0_FLUSH_BITGEN_LSB_POSN      = (int)9,
   TRACE_0_CFG_TRACE_CFG_0_FLUSH_BITGEN_MSB_POSN      = (int)9
};
typedef enum trace_0_cfg_posn_enum trace_0_cfg_posn;

#define TRACE_CFG_0_ENABLE_MASK                  (0x00000001u)
#define TRACE_CFG_0_SYNC_INTERVAL_LSB_MASK       (0x00000002u)
#define TRACE_CFG_0_SYNC_INTERVAL_MSB_MASK       (0x00000008u)
#define TRACE_CFG_0_CPU_SELECT_LSB_MASK          (0x00000010u)
#define TRACE_CFG_0_CPU_SELECT_MSB_MASK          (0x00000020u)
#define TRACE_CFG_0_FLUSH_FIFO_MASK              (0x00000040u)
#define TRACE_CFG_0_STALL_CORE_ON_TRACE_FULL_MASK (0x00000080u)
#define TRACE_CFG_0_CLR_STORED_ON_SYNC_MASK      (0x00000100u)
#define TRACE_CFG_0_FLUSH_BITGEN_MASK            (0x00000200u)

enum trace_0_dmem_cfg_posn_enum
{
   TRACE_0_DMEM_EN_POSN                               = (int)0,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_EN_LSB_POSN          = (int)0,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_EN_MSB_POSN          = (int)0,
   TRACE_0_DMEM_CFG_WRAP_POSN                         = (int)1,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_CFG_WRAP_LSB_POSN    = (int)1,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_CFG_WRAP_MSB_POSN    = (int)1,
   TRACE_0_DMEM_CFG_LENGTH_LSB_POSN                   = (int)2,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_CFG_LENGTH_LSB_POSN  = (int)2,
   TRACE_0_DMEM_CFG_LENGTH_MSB_POSN                   = (int)12,
   TRACE_0_DMEM_CFG_TRACE_0_DMEM_CFG_LENGTH_MSB_POSN  = (int)12
};
typedef enum trace_0_dmem_cfg_posn_enum trace_0_dmem_cfg_posn;

#define TRACE_0_DMEM_EN_MASK                     (0x00000001u)
#define TRACE_0_DMEM_CFG_WRAP_MASK               (0x00000002u)
#define TRACE_0_DMEM_CFG_LENGTH_LSB_MASK         (0x00000004u)
#define TRACE_0_DMEM_CFG_LENGTH_MSB_MASK         (0x00001000u)

enum trace_0_tbus_cfg_posn_enum
{
   TRACE_0_TBUS_EN_POSN                               = (int)0,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_EN_LSB_POSN          = (int)0,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_EN_MSB_POSN          = (int)0,
   TRACE_0_TBUS_CFG_TRAN_TYPE_POSN                    = (int)1,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_TRAN_TYPE_LSB_POSN = (int)1,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_TRAN_TYPE_MSB_POSN = (int)1,
   TRACE_0_TBUS_CFG_WRAP_POSN                         = (int)2,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_WRAP_LSB_POSN    = (int)2,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_WRAP_MSB_POSN    = (int)2,
   TRACE_0_TBUS_CFG_DEST_SYS_LSB_POSN                 = (int)3,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_DEST_SYS_LSB_POSN = (int)3,
   TRACE_0_TBUS_CFG_DEST_SYS_MSB_POSN                 = (int)6,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_DEST_SYS_MSB_POSN = (int)6,
   TRACE_0_TBUS_CFG_DEST_BLK_LSB_POSN                 = (int)7,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_DEST_BLK_LSB_POSN = (int)7,
   TRACE_0_TBUS_CFG_DEST_BLK_MSB_POSN                 = (int)10,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_DEST_BLK_MSB_POSN = (int)10,
   TRACE_0_TBUS_CFG_SRC_BLK_LSB_POSN                  = (int)11,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_SRC_BLK_LSB_POSN = (int)11,
   TRACE_0_TBUS_CFG_SRC_BLK_MSB_POSN                  = (int)14,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_SRC_BLK_MSB_POSN = (int)14,
   TRACE_0_TBUS_CFG_TAG_LSB_POSN                      = (int)15,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_TAG_LSB_POSN     = (int)15,
   TRACE_0_TBUS_CFG_TAG_MSB_POSN                      = (int)18,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_TAG_MSB_POSN     = (int)18,
   TRACE_0_TBUS_CFG_LENGTH_LSB_POSN                   = (int)19,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_LENGTH_LSB_POSN  = (int)19,
   TRACE_0_TBUS_CFG_LENGTH_MSB_POSN                   = (int)29,
   TRACE_0_TBUS_CFG_TRACE_0_TBUS_CFG_LENGTH_MSB_POSN  = (int)29
};
typedef enum trace_0_tbus_cfg_posn_enum trace_0_tbus_cfg_posn;

#define TRACE_0_TBUS_EN_MASK                     (0x00000001u)
#define TRACE_0_TBUS_CFG_TRAN_TYPE_MASK          (0x00000002u)
#define TRACE_0_TBUS_CFG_WRAP_MASK               (0x00000004u)
#define TRACE_0_TBUS_CFG_DEST_SYS_LSB_MASK       (0x00000008u)
#define TRACE_0_TBUS_CFG_DEST_SYS_MSB_MASK       (0x00000040u)
#define TRACE_0_TBUS_CFG_DEST_BLK_LSB_MASK       (0x00000080u)
#define TRACE_0_TBUS_CFG_DEST_BLK_MSB_MASK       (0x00000400u)
#define TRACE_0_TBUS_CFG_SRC_BLK_LSB_MASK        (0x00000800u)
#define TRACE_0_TBUS_CFG_SRC_BLK_MSB_MASK        (0x00004000u)
#define TRACE_0_TBUS_CFG_TAG_LSB_MASK            (0x00008000u)
#define TRACE_0_TBUS_CFG_TAG_MSB_MASK            (0x00040000u)
#define TRACE_0_TBUS_CFG_LENGTH_LSB_MASK         (0x00080000u)
#define TRACE_0_TBUS_CFG_LENGTH_MSB_MASK         (0x20000000u)

enum trace_1_cfg_posn_enum
{
   TRACE_CFG_1_ENABLE_POSN                            = (int)0,
   TRACE_1_CFG_TRACE_CFG_1_ENABLE_LSB_POSN            = (int)0,
   TRACE_1_CFG_TRACE_CFG_1_ENABLE_MSB_POSN            = (int)0,
   TRACE_CFG_1_SYNC_INTERVAL_LSB_POSN                 = (int)1,
   TRACE_1_CFG_TRACE_CFG_1_SYNC_INTERVAL_LSB_POSN     = (int)1,
   TRACE_CFG_1_SYNC_INTERVAL_MSB_POSN                 = (int)3,
   TRACE_1_CFG_TRACE_CFG_1_SYNC_INTERVAL_MSB_POSN     = (int)3,
   TRACE_CFG_1_CPU_SELECT_LSB_POSN                    = (int)4,
   TRACE_1_CFG_TRACE_CFG_1_CPU_SELECT_LSB_POSN        = (int)4,
   TRACE_CFG_1_CPU_SELECT_MSB_POSN                    = (int)5,
   TRACE_1_CFG_TRACE_CFG_1_CPU_SELECT_MSB_POSN        = (int)5,
   TRACE_CFG_1_FLUSH_FIFO_POSN                        = (int)6,
   TRACE_1_CFG_TRACE_CFG_1_FLUSH_FIFO_LSB_POSN        = (int)6,
   TRACE_1_CFG_TRACE_CFG_1_FLUSH_FIFO_MSB_POSN        = (int)6,
   TRACE_CFG_1_STALL_CORE_ON_TRACE_FULL_POSN          = (int)7,
   TRACE_1_CFG_TRACE_CFG_1_STALL_CORE_ON_TRACE_FULL_LSB_POSN = (int)7,
   TRACE_1_CFG_TRACE_CFG_1_STALL_CORE_ON_TRACE_FULL_MSB_POSN = (int)7,
   TRACE_CFG_1_CLR_STORED_ON_SYNC_POSN                = (int)8,
   TRACE_1_CFG_TRACE_CFG_1_CLR_STORED_ON_SYNC_LSB_POSN = (int)8,
   TRACE_1_CFG_TRACE_CFG_1_CLR_STORED_ON_SYNC_MSB_POSN = (int)8,
   TRACE_CFG_1_FLUSH_BITGEN_POSN                      = (int)9,
   TRACE_1_CFG_TRACE_CFG_1_FLUSH_BITGEN_LSB_POSN      = (int)9,
   TRACE_1_CFG_TRACE_CFG_1_FLUSH_BITGEN_MSB_POSN      = (int)9
};
typedef enum trace_1_cfg_posn_enum trace_1_cfg_posn;

#define TRACE_CFG_1_ENABLE_MASK                  (0x00000001u)
#define TRACE_CFG_1_SYNC_INTERVAL_LSB_MASK       (0x00000002u)
#define TRACE_CFG_1_SYNC_INTERVAL_MSB_MASK       (0x00000008u)
#define TRACE_CFG_1_CPU_SELECT_LSB_MASK          (0x00000010u)
#define TRACE_CFG_1_CPU_SELECT_MSB_MASK          (0x00000020u)
#define TRACE_CFG_1_FLUSH_FIFO_MASK              (0x00000040u)
#define TRACE_CFG_1_STALL_CORE_ON_TRACE_FULL_MASK (0x00000080u)
#define TRACE_CFG_1_CLR_STORED_ON_SYNC_MASK      (0x00000100u)
#define TRACE_CFG_1_FLUSH_BITGEN_MASK            (0x00000200u)

enum trace_1_dmem_cfg_posn_enum
{
   TRACE_1_DMEM_EN_POSN                               = (int)0,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_EN_LSB_POSN          = (int)0,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_EN_MSB_POSN          = (int)0,
   TRACE_1_DMEM_CFG_WRAP_POSN                         = (int)1,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_CFG_WRAP_LSB_POSN    = (int)1,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_CFG_WRAP_MSB_POSN    = (int)1,
   TRACE_1_DMEM_CFG_LENGTH_LSB_POSN                   = (int)2,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_CFG_LENGTH_LSB_POSN  = (int)2,
   TRACE_1_DMEM_CFG_LENGTH_MSB_POSN                   = (int)12,
   TRACE_1_DMEM_CFG_TRACE_1_DMEM_CFG_LENGTH_MSB_POSN  = (int)12
};
typedef enum trace_1_dmem_cfg_posn_enum trace_1_dmem_cfg_posn;

#define TRACE_1_DMEM_EN_MASK                     (0x00000001u)
#define TRACE_1_DMEM_CFG_WRAP_MASK               (0x00000002u)
#define TRACE_1_DMEM_CFG_LENGTH_LSB_MASK         (0x00000004u)
#define TRACE_1_DMEM_CFG_LENGTH_MSB_MASK         (0x00001000u)

enum trace_1_tbus_cfg_posn_enum
{
   TRACE_1_TBUS_EN_POSN                               = (int)0,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_EN_LSB_POSN          = (int)0,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_EN_MSB_POSN          = (int)0,
   TRACE_1_TBUS_CFG_TRAN_TYPE_POSN                    = (int)1,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_TRAN_TYPE_LSB_POSN = (int)1,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_TRAN_TYPE_MSB_POSN = (int)1,
   TRACE_1_TBUS_CFG_WRAP_POSN                         = (int)2,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_WRAP_LSB_POSN    = (int)2,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_WRAP_MSB_POSN    = (int)2,
   TRACE_1_TBUS_CFG_DEST_SYS_LSB_POSN                 = (int)3,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_DEST_SYS_LSB_POSN = (int)3,
   TRACE_1_TBUS_CFG_DEST_SYS_MSB_POSN                 = (int)6,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_DEST_SYS_MSB_POSN = (int)6,
   TRACE_1_TBUS_CFG_DEST_BLK_LSB_POSN                 = (int)7,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_DEST_BLK_LSB_POSN = (int)7,
   TRACE_1_TBUS_CFG_DEST_BLK_MSB_POSN                 = (int)10,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_DEST_BLK_MSB_POSN = (int)10,
   TRACE_1_TBUS_CFG_SRC_BLK_LSB_POSN                  = (int)11,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_SRC_BLK_LSB_POSN = (int)11,
   TRACE_1_TBUS_CFG_SRC_BLK_MSB_POSN                  = (int)14,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_SRC_BLK_MSB_POSN = (int)14,
   TRACE_1_TBUS_CFG_TAG_LSB_POSN                      = (int)15,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_TAG_LSB_POSN     = (int)15,
   TRACE_1_TBUS_CFG_TAG_MSB_POSN                      = (int)18,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_TAG_MSB_POSN     = (int)18,
   TRACE_1_TBUS_CFG_LENGTH_LSB_POSN                   = (int)19,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_LENGTH_LSB_POSN  = (int)19,
   TRACE_1_TBUS_CFG_LENGTH_MSB_POSN                   = (int)29,
   TRACE_1_TBUS_CFG_TRACE_1_TBUS_CFG_LENGTH_MSB_POSN  = (int)29
};
typedef enum trace_1_tbus_cfg_posn_enum trace_1_tbus_cfg_posn;

#define TRACE_1_TBUS_EN_MASK                     (0x00000001u)
#define TRACE_1_TBUS_CFG_TRAN_TYPE_MASK          (0x00000002u)
#define TRACE_1_TBUS_CFG_WRAP_MASK               (0x00000004u)
#define TRACE_1_TBUS_CFG_DEST_SYS_LSB_MASK       (0x00000008u)
#define TRACE_1_TBUS_CFG_DEST_SYS_MSB_MASK       (0x00000040u)
#define TRACE_1_TBUS_CFG_DEST_BLK_LSB_MASK       (0x00000080u)
#define TRACE_1_TBUS_CFG_DEST_BLK_MSB_MASK       (0x00000400u)
#define TRACE_1_TBUS_CFG_SRC_BLK_LSB_MASK        (0x00000800u)
#define TRACE_1_TBUS_CFG_SRC_BLK_MSB_MASK        (0x00004000u)
#define TRACE_1_TBUS_CFG_TAG_LSB_MASK            (0x00008000u)
#define TRACE_1_TBUS_CFG_TAG_MSB_MASK            (0x00040000u)
#define TRACE_1_TBUS_CFG_LENGTH_LSB_MASK         (0x00080000u)
#define TRACE_1_TBUS_CFG_LENGTH_MSB_MASK         (0x20000000u)

enum trace_dmem_status_posn_enum
{
   TRACE_DMEM_STATUS_CNTL_0_DUMP_DONE_POSN            = (int)0,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_0_DUMP_DONE_LSB_POSN = (int)0,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_0_DUMP_DONE_MSB_POSN = (int)0,
   TRACE_DMEM_STATUS_CNTL_1_DUMP_DONE_POSN            = (int)1,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_1_DUMP_DONE_LSB_POSN = (int)1,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_1_DUMP_DONE_MSB_POSN = (int)1,
   TRACE_DMEM_STATUS_CNTL_2_DUMP_DONE_POSN            = (int)2,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_2_DUMP_DONE_LSB_POSN = (int)2,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_2_DUMP_DONE_MSB_POSN = (int)2,
   TRACE_DMEM_STATUS_CNTL_3_DUMP_DONE_POSN            = (int)3,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_3_DUMP_DONE_LSB_POSN = (int)3,
   TRACE_DMEM_STATUS_TRACE_DMEM_STATUS_CNTL_3_DUMP_DONE_MSB_POSN = (int)3
};
typedef enum trace_dmem_status_posn_enum trace_dmem_status_posn;

#define TRACE_DMEM_STATUS_CNTL_0_DUMP_DONE_MASK  (0x00000001u)
#define TRACE_DMEM_STATUS_CNTL_1_DUMP_DONE_MASK  (0x00000002u)
#define TRACE_DMEM_STATUS_CNTL_2_DUMP_DONE_MASK  (0x00000004u)
#define TRACE_DMEM_STATUS_CNTL_3_DUMP_DONE_MASK  (0x00000008u)

enum trace_tbus_status_posn_enum
{
   TRACE_TBUS_STATUS_CNTL_0_DUMP_DONE_POSN            = (int)0,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_0_DUMP_DONE_LSB_POSN = (int)0,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_0_DUMP_DONE_MSB_POSN = (int)0,
   TRACE_TBUS_STATUS_CNTL_1_DUMP_DONE_POSN            = (int)1,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_1_DUMP_DONE_LSB_POSN = (int)1,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_1_DUMP_DONE_MSB_POSN = (int)1,
   TRACE_TBUS_STATUS_CNTL_2_DUMP_DONE_POSN            = (int)2,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_2_DUMP_DONE_LSB_POSN = (int)2,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_2_DUMP_DONE_MSB_POSN = (int)2,
   TRACE_TBUS_STATUS_CNTL_3_DUMP_DONE_POSN            = (int)3,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_3_DUMP_DONE_LSB_POSN = (int)3,
   TRACE_TBUS_STATUS_TRACE_TBUS_STATUS_CNTL_3_DUMP_DONE_MSB_POSN = (int)3
};
typedef enum trace_tbus_status_posn_enum trace_tbus_status_posn;

#define TRACE_TBUS_STATUS_CNTL_0_DUMP_DONE_MASK  (0x00000001u)
#define TRACE_TBUS_STATUS_CNTL_1_DUMP_DONE_MASK  (0x00000002u)
#define TRACE_TBUS_STATUS_CNTL_2_DUMP_DONE_MASK  (0x00000004u)
#define TRACE_TBUS_STATUS_CNTL_3_DUMP_DONE_MASK  (0x00000008u)

enum trace_0_dmem_base_addr_posn_enum
{
   TRACE_0_DMEM_BASE_ADDR_LSB_POSN                    = (int)0,
   TRACE_0_DMEM_BASE_ADDR_TRACE_0_DMEM_BASE_ADDR_LSB_POSN = (int)0,
   TRACE_0_DMEM_BASE_ADDR_MSB_POSN                    = (int)31,
   TRACE_0_DMEM_BASE_ADDR_TRACE_0_DMEM_BASE_ADDR_MSB_POSN = (int)31
};
typedef enum trace_0_dmem_base_addr_posn_enum trace_0_dmem_base_addr_posn;

#define TRACE_0_DMEM_BASE_ADDR_LSB_MASK          (0x00000001u)
#define TRACE_0_DMEM_BASE_ADDR_MSB_MASK          (0x80000000u)

enum trace_0_end_trigger_posn_enum
{
   TRACE_0_END_TRIGGER_LSB_POSN                       = (int)0,
   TRACE_0_END_TRIGGER_TRACE_0_END_TRIGGER_LSB_POSN   = (int)0,
   TRACE_0_END_TRIGGER_MSB_POSN                       = (int)31,
   TRACE_0_END_TRIGGER_TRACE_0_END_TRIGGER_MSB_POSN   = (int)31
};
typedef enum trace_0_end_trigger_posn_enum trace_0_end_trigger_posn;

#define TRACE_0_END_TRIGGER_LSB_MASK             (0x00000001u)
#define TRACE_0_END_TRIGGER_MSB_MASK             (0x80000000u)

enum trace_0_start_trigger_posn_enum
{
   TRACE_0_START_TRIGGER_LSB_POSN                     = (int)0,
   TRACE_0_START_TRIGGER_TRACE_0_START_TRIGGER_LSB_POSN = (int)0,
   TRACE_0_START_TRIGGER_MSB_POSN                     = (int)31,
   TRACE_0_START_TRIGGER_TRACE_0_START_TRIGGER_MSB_POSN = (int)31
};
typedef enum trace_0_start_trigger_posn_enum trace_0_start_trigger_posn;

#define TRACE_0_START_TRIGGER_LSB_MASK           (0x00000001u)
#define TRACE_0_START_TRIGGER_MSB_MASK           (0x80000000u)

enum trace_0_tbus_base_addr_posn_enum
{
   TRACE_0_TBUS_BASE_ADDR_LSB_POSN                    = (int)0,
   TRACE_0_TBUS_BASE_ADDR_TRACE_0_TBUS_BASE_ADDR_LSB_POSN = (int)0,
   TRACE_0_TBUS_BASE_ADDR_MSB_POSN                    = (int)31,
   TRACE_0_TBUS_BASE_ADDR_TRACE_0_TBUS_BASE_ADDR_MSB_POSN = (int)31
};
typedef enum trace_0_tbus_base_addr_posn_enum trace_0_tbus_base_addr_posn;

#define TRACE_0_TBUS_BASE_ADDR_LSB_MASK          (0x00000001u)
#define TRACE_0_TBUS_BASE_ADDR_MSB_MASK          (0x80000000u)

enum trace_1_dmem_base_addr_posn_enum
{
   TRACE_1_DMEM_BASE_ADDR_LSB_POSN                    = (int)0,
   TRACE_1_DMEM_BASE_ADDR_TRACE_1_DMEM_BASE_ADDR_LSB_POSN = (int)0,
   TRACE_1_DMEM_BASE_ADDR_MSB_POSN                    = (int)31,
   TRACE_1_DMEM_BASE_ADDR_TRACE_1_DMEM_BASE_ADDR_MSB_POSN = (int)31
};
typedef enum trace_1_dmem_base_addr_posn_enum trace_1_dmem_base_addr_posn;

#define TRACE_1_DMEM_BASE_ADDR_LSB_MASK          (0x00000001u)
#define TRACE_1_DMEM_BASE_ADDR_MSB_MASK          (0x80000000u)

enum trace_1_end_trigger_posn_enum
{
   TRACE_1_END_TRIGGER_LSB_POSN                       = (int)0,
   TRACE_1_END_TRIGGER_TRACE_1_END_TRIGGER_LSB_POSN   = (int)0,
   TRACE_1_END_TRIGGER_MSB_POSN                       = (int)31,
   TRACE_1_END_TRIGGER_TRACE_1_END_TRIGGER_MSB_POSN   = (int)31
};
typedef enum trace_1_end_trigger_posn_enum trace_1_end_trigger_posn;

#define TRACE_1_END_TRIGGER_LSB_MASK             (0x00000001u)
#define TRACE_1_END_TRIGGER_MSB_MASK             (0x80000000u)

enum trace_1_start_trigger_posn_enum
{
   TRACE_1_START_TRIGGER_LSB_POSN                     = (int)0,
   TRACE_1_START_TRIGGER_TRACE_1_START_TRIGGER_LSB_POSN = (int)0,
   TRACE_1_START_TRIGGER_MSB_POSN                     = (int)31,
   TRACE_1_START_TRIGGER_TRACE_1_START_TRIGGER_MSB_POSN = (int)31
};
typedef enum trace_1_start_trigger_posn_enum trace_1_start_trigger_posn;

#define TRACE_1_START_TRIGGER_LSB_MASK           (0x00000001u)
#define TRACE_1_START_TRIGGER_MSB_MASK           (0x80000000u)

enum trace_1_tbus_base_addr_posn_enum
{
   TRACE_1_TBUS_BASE_ADDR_LSB_POSN                    = (int)0,
   TRACE_1_TBUS_BASE_ADDR_TRACE_1_TBUS_BASE_ADDR_LSB_POSN = (int)0,
   TRACE_1_TBUS_BASE_ADDR_MSB_POSN                    = (int)31,
   TRACE_1_TBUS_BASE_ADDR_TRACE_1_TBUS_BASE_ADDR_MSB_POSN = (int)31
};
typedef enum trace_1_tbus_base_addr_posn_enum trace_1_tbus_base_addr_posn;

#define TRACE_1_TBUS_BASE_ADDR_LSB_MASK          (0x00000001u)
#define TRACE_1_TBUS_BASE_ADDR_MSB_MASK          (0x80000000u)

enum trace_debug_sel_posn_enum
{
   TRACE_DEBUG_SEL_LSB_POSN                           = (int)0,
   TRACE_DEBUG_SEL_TRACE_DEBUG_SEL_LSB_POSN           = (int)0,
   TRACE_DEBUG_SEL_MSB_POSN                           = (int)3,
   TRACE_DEBUG_SEL_TRACE_DEBUG_SEL_MSB_POSN           = (int)3
};
typedef enum trace_debug_sel_posn_enum trace_debug_sel_posn;

#define TRACE_DEBUG_SEL_LSB_MASK                 (0x00000001u)
#define TRACE_DEBUG_SEL_MSB_MASK                 (0x00000008u)

enum k32_trace__access_ctrl_enum_posn_enum
{
   K32_TRACE__P0_ACCESS_PERMISSION_POSN               = (int)0,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   K32_TRACE__P1_ACCESS_PERMISSION_POSN               = (int)1,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   K32_TRACE__P2_ACCESS_PERMISSION_POSN               = (int)2,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   K32_TRACE__P3_ACCESS_PERMISSION_POSN               = (int)3,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   K32_TRACE__ACCESS_CTRL_ENUM_K32_TRACE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum k32_trace__access_ctrl_enum_posn_enum k32_trace__access_ctrl_enum_posn;

#define K32_TRACE__P0_ACCESS_PERMISSION_MASK     (0x00000001u)
#define K32_TRACE__P1_ACCESS_PERMISSION_MASK     (0x00000002u)
#define K32_TRACE__P2_ACCESS_PERMISSION_MASK     (0x00000004u)
#define K32_TRACE__P3_ACCESS_PERMISSION_MASK     (0x00000008u)

enum k32_trace__p0_access_permission_enum
{
   K32_TRACE__P0_ACCESS_BLOCKED             = (int)0x0,
   K32_TRACE__P0_ACCESS_UNBLOCKED           = (int)0x1,
   MAX_K32_TRACE__P0_ACCESS_PERMISSION      = (int)0x1
};
#define NUM_K32_TRACE__P0_ACCESS_PERMISSION (0x2)
typedef enum k32_trace__p0_access_permission_enum k32_trace__p0_access_permission;


enum k32_trace__p1_access_permission_enum
{
   K32_TRACE__P1_ACCESS_BLOCKED             = (int)0x0,
   K32_TRACE__P1_ACCESS_UNBLOCKED           = (int)0x1,
   MAX_K32_TRACE__P1_ACCESS_PERMISSION      = (int)0x1
};
#define NUM_K32_TRACE__P1_ACCESS_PERMISSION (0x2)
typedef enum k32_trace__p1_access_permission_enum k32_trace__p1_access_permission;


enum k32_trace__p2_access_permission_enum
{
   K32_TRACE__P2_ACCESS_BLOCKED             = (int)0x0,
   K32_TRACE__P2_ACCESS_UNBLOCKED           = (int)0x1,
   MAX_K32_TRACE__P2_ACCESS_PERMISSION      = (int)0x1
};
#define NUM_K32_TRACE__P2_ACCESS_PERMISSION (0x2)
typedef enum k32_trace__p2_access_permission_enum k32_trace__p2_access_permission;


enum k32_trace__p3_access_permission_enum
{
   K32_TRACE__P3_ACCESS_BLOCKED             = (int)0x0,
   K32_TRACE__P3_ACCESS_UNBLOCKED           = (int)0x1,
   MAX_K32_TRACE__P3_ACCESS_PERMISSION      = (int)0x1
};
#define NUM_K32_TRACE__P3_ACCESS_PERMISSION (0x2)
typedef enum k32_trace__p3_access_permission_enum k32_trace__p3_access_permission;


enum k32_trace__mutex_lock_enum_enum
{
   K32_TRACE__MUTEX_AVAILABLE               = (int)0x0,
   K32_TRACE__MUTEX_CLAIMED_BY_P0           = (int)0x1,
   K32_TRACE__MUTEX_CLAIMED_BY_P1           = (int)0x2,
   K32_TRACE__MUTEX_CLAIMED_BY_P2           = (int)0x4,
   K32_TRACE__MUTEX_CLAIMED_BY_P3           = (int)0x8,
   K32_TRACE__MUTEX_DISABLED                = (int)0xF,
   MAX_K32_TRACE__MUTEX_LOCK_ENUM           = (int)0xF
};
typedef enum k32_trace__mutex_lock_enum_enum k32_trace__mutex_lock_enum;


enum trace_0_trigger_cfg_posn_enum
{
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_END_TRIG_EN_LSB_POSN = (int)0,
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_END_TRIG_EN_MSB_POSN = (int)0,
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_START_TRIG_EN_LSB_POSN = (int)1,
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_START_TRIG_EN_MSB_POSN = (int)1,
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_LSB_POSN = (int)2,
   TRACE_0_TRIGGER_CFG_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_MSB_POSN = (int)11
};
typedef enum trace_0_trigger_cfg_posn_enum trace_0_trigger_cfg_posn;


enum trace_0_trigger_status_posn_enum
{
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_FOUND_LSB_POSN = (int)0,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_FOUND_MSB_POSN = (int)0,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_COMPL_LSB_POSN = (int)1,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_COMPL_MSB_POSN = (int)1,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_FOUND_LSB_POSN = (int)2,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_FOUND_MSB_POSN = (int)2,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_COMPL_LSB_POSN = (int)3,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_COMPL_MSB_POSN = (int)3,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_SMDBG_LSB_POSN = (int)4,
   TRACE_0_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_SMDBG_MSB_POSN = (int)5
};
typedef enum trace_0_trigger_status_posn_enum trace_0_trigger_status_posn;


enum trace_1_trigger_cfg_posn_enum
{
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_END_TRIG_EN_LSB_POSN = (int)0,
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_END_TRIG_EN_MSB_POSN = (int)0,
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_START_TRIG_EN_LSB_POSN = (int)1,
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_START_TRIG_EN_MSB_POSN = (int)1,
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_LSB_POSN = (int)2,
   TRACE_1_TRIGGER_CFG_TRACE_TRIGGER_CFG_TRIGGER_LENGTH_MSB_POSN = (int)11
};
typedef enum trace_1_trigger_cfg_posn_enum trace_1_trigger_cfg_posn;


enum trace_1_trigger_status_posn_enum
{
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_FOUND_LSB_POSN = (int)0,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_FOUND_MSB_POSN = (int)0,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_COMPL_LSB_POSN = (int)1,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_START_COMPL_MSB_POSN = (int)1,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_FOUND_LSB_POSN = (int)2,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_FOUND_MSB_POSN = (int)2,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_COMPL_LSB_POSN = (int)3,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_END_COMPL_MSB_POSN = (int)3,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_SMDBG_LSB_POSN = (int)4,
   TRACE_1_TRIGGER_STATUS_TRACE_TRIGGER_STATUS_SMDBG_MSB_POSN = (int)5
};
typedef enum trace_1_trigger_status_posn_enum trace_1_trigger_status_posn;


enum trace_access_ctrl_posn_enum
{
   TRACE_ACCESS_CTRL_K32_TRACE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   TRACE_ACCESS_CTRL_K32_TRACE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   TRACE_ACCESS_CTRL_K32_TRACE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   TRACE_ACCESS_CTRL_K32_TRACE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   TRACE_ACCESS_CTRL_K32_TRACE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   TRACE_ACCESS_CTRL_K32_TRACE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   TRACE_ACCESS_CTRL_K32_TRACE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   TRACE_ACCESS_CTRL_K32_TRACE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum trace_access_ctrl_posn_enum trace_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_K32_TRACE */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_TRACE */


#if defined(IO_DEFS_MODULE_KALIMBA_INTERPROC_INT) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_INTERPROC_INT
#define __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_INTERPROC_INT

/* -- kalimba_interproc_int -- Inter-processor interrupt generation registers -- */

enum p0_to_p1_interproc_event_1_posn_enum
{
   P0_TO_P1_INTERPROC_EVENT_1_POSN                    = (int)0,
   P0_TO_P1_INTERPROC_EVENT_1_P0_TO_P1_INTERPROC_EVENT_1_LSB_POSN = (int)0,
   P0_TO_P1_INTERPROC_EVENT_1_P0_TO_P1_INTERPROC_EVENT_1_MSB_POSN = (int)0
};
typedef enum p0_to_p1_interproc_event_1_posn_enum p0_to_p1_interproc_event_1_posn;

#define P0_TO_P1_INTERPROC_EVENT_1_MASK          (0x00000001u)

enum p0_to_p1_interproc_event_2_posn_enum
{
   P0_TO_P1_INTERPROC_EVENT_2_POSN                    = (int)0,
   P0_TO_P1_INTERPROC_EVENT_2_P0_TO_P1_INTERPROC_EVENT_2_LSB_POSN = (int)0,
   P0_TO_P1_INTERPROC_EVENT_2_P0_TO_P1_INTERPROC_EVENT_2_MSB_POSN = (int)0
};
typedef enum p0_to_p1_interproc_event_2_posn_enum p0_to_p1_interproc_event_2_posn;

#define P0_TO_P1_INTERPROC_EVENT_2_MASK          (0x00000001u)

enum p1_to_p0_interproc_event_1_posn_enum
{
   P1_TO_P0_INTERPROC_EVENT_1_POSN                    = (int)0,
   P1_TO_P0_INTERPROC_EVENT_1_P1_TO_P0_INTERPROC_EVENT_1_LSB_POSN = (int)0,
   P1_TO_P0_INTERPROC_EVENT_1_P1_TO_P0_INTERPROC_EVENT_1_MSB_POSN = (int)0
};
typedef enum p1_to_p0_interproc_event_1_posn_enum p1_to_p0_interproc_event_1_posn;

#define P1_TO_P0_INTERPROC_EVENT_1_MASK          (0x00000001u)

enum p1_to_p0_interproc_event_2_posn_enum
{
   P1_TO_P0_INTERPROC_EVENT_2_POSN                    = (int)0,
   P1_TO_P0_INTERPROC_EVENT_2_P1_TO_P0_INTERPROC_EVENT_2_LSB_POSN = (int)0,
   P1_TO_P0_INTERPROC_EVENT_2_P1_TO_P0_INTERPROC_EVENT_2_MSB_POSN = (int)0
};
typedef enum p1_to_p0_interproc_event_2_posn_enum p1_to_p0_interproc_event_2_posn;

#define P1_TO_P0_INTERPROC_EVENT_2_MASK          (0x00000001u)

#endif /* IO_DEFS_MODULE_KALIMBA_INTERPROC_INT */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_INTERPROC_INT */



#if defined(IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS

/* -- apps_sys_sqif_windows -- SQIF Datapath Register Bank. -- */

enum apps_sqif_datapath_ports_enum
{
   CPU0_CACHED_FLASH                        = (int)0x0,
   CPU1_CACHED_FLASH                        = (int)0x1,
   DMAC_FLASH                               = (int)0x2,
   SQIF0_RAM                                = (int)0x3,
   CPU0_UNCACHED_FLASH                      = (int)0x4,
   CPU1_UNCACHED_FLASH                      = (int)0x5,
   TBUS_FLASH                               = (int)0x6,
   MAX_APPS_SQIF_DATAPATH_PORTS             = (int)0x6
};
#define NUM_APPS_SQIF_DATAPATH_PORTS (0x7)
typedef enum apps_sqif_datapath_ports_enum apps_sqif_datapath_ports;


enum apps_sys_sqif_window_offset_index_enum
{
   SQIF0_CACHE0_WINDOW_OFFSET               = (int)0x0,
   SQIF0_CACHE1_WINDOW_OFFSET               = (int)0x1,
   SQIF0_DMAC_WINDOW_OFFSET                 = (int)0x2,
   SQIF0_SPI_RAM_WINDOW_OFFSET              = (int)0x3,
   SQIF0_UNCACHED_CPU0_WINDOW_OFFSET        = (int)0x4,
   SQIF0_UNCACHED_CPU1_WINDOW_OFFSET        = (int)0x5,
   SQIF0_TBUS0_WINDOW_OFFSET                = (int)0x6,
   SQIF0_TBUS1_WINDOW_OFFSET                = (int)0x7,
   SQIF0_DEBUG_WINDOW_OFFSET                = (int)0x8,
   SQIF_NUM_WINDOW_OFFSETS                  = (int)0x9,
   SQIF1_CACHE0_WINDOW_OFFSET               = (int)0x9,
   SQIF1_CACHE1_WINDOW_OFFSET               = (int)0xA,
   SQIF1_DMAC_WINDOW_OFFSET                 = (int)0xB,
   SQIF1_SPI_RAM_WINDOW_OFFSET              = (int)0xC,
   SQIF1_UNCACHED_CPU0_WINDOW_OFFSET        = (int)0xD,
   SQIF1_UNCACHED_CPU1_WINDOW_OFFSET        = (int)0xE,
   SQIF1_TBUS0_WINDOW_OFFSET                = (int)0xF,
   SQIF1_TBUS1_WINDOW_OFFSET                = (int)0x10,
   SQIF1_DEBUG_WINDOW_OFFSET                = (int)0x11,
   MAX_APPS_SYS_SQIF_WINDOW_OFFSET_INDEX    = (int)0x11
};
typedef enum apps_sys_sqif_window_offset_index_enum apps_sys_sqif_window_offset_index;


enum apps_sys_sqif_window_offset_posn_enum
{
   APPS_SYS_SQIF_WINDOW_OFFSET_LSB_POSN               = (int)0,
   APPS_SYS_SQIF_WINDOW_OFFSET_APPS_SYS_SQIF_WINDOW_OFFSET_LSB_POSN = (int)0,
   APPS_SYS_SQIF_WINDOW_OFFSET_MSB_POSN               = (int)31,
   APPS_SYS_SQIF_WINDOW_OFFSET_APPS_SYS_SQIF_WINDOW_OFFSET_MSB_POSN = (int)31
};
typedef enum apps_sys_sqif_window_offset_posn_enum apps_sys_sqif_window_offset_posn;

#define APPS_SYS_SQIF_WINDOW_OFFSET_LSB_MASK     (0x00000001u)
#define APPS_SYS_SQIF_WINDOW_OFFSET_MSB_MASK     (0x80000000u)

enum apps_sys_sqif_windows__access_ctrl_enum_posn_enum
{
   APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_POSN   = (int)0,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_POSN   = (int)1,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_POSN   = (int)2,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_POSN   = (int)3,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_SQIF_WINDOWS__ACCESS_CTRL_ENUM_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_sqif_windows__access_ctrl_enum_posn_enum apps_sys_sqif_windows__access_ctrl_enum_posn;

#define APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum apps_sys_sqif_windows__p0_access_permission_enum
{
   APPS_SYS_SQIF_WINDOWS__P0_ACCESS_BLOCKED = (int)0x0,
   APPS_SYS_SQIF_WINDOWS__P0_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sqif_windows__p0_access_permission_enum apps_sys_sqif_windows__p0_access_permission;


enum apps_sys_sqif_windows__p1_access_permission_enum
{
   APPS_SYS_SQIF_WINDOWS__P1_ACCESS_BLOCKED = (int)0x0,
   APPS_SYS_SQIF_WINDOWS__P1_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sqif_windows__p1_access_permission_enum apps_sys_sqif_windows__p1_access_permission;


enum apps_sys_sqif_windows__p2_access_permission_enum
{
   APPS_SYS_SQIF_WINDOWS__P2_ACCESS_BLOCKED = (int)0x0,
   APPS_SYS_SQIF_WINDOWS__P2_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sqif_windows__p2_access_permission_enum apps_sys_sqif_windows__p2_access_permission;


enum apps_sys_sqif_windows__p3_access_permission_enum
{
   APPS_SYS_SQIF_WINDOWS__P3_ACCESS_BLOCKED = (int)0x0,
   APPS_SYS_SQIF_WINDOWS__P3_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sqif_windows__p3_access_permission_enum apps_sys_sqif_windows__p3_access_permission;


enum apps_sys_sqif_windows__mutex_lock_enum_enum
{
   APPS_SYS_SQIF_WINDOWS__MUTEX_AVAILABLE   = (int)0x0,
   APPS_SYS_SQIF_WINDOWS__MUTEX_CLAIMED_BY_P0 = (int)0x1,
   APPS_SYS_SQIF_WINDOWS__MUTEX_CLAIMED_BY_P1 = (int)0x2,
   APPS_SYS_SQIF_WINDOWS__MUTEX_CLAIMED_BY_P2 = (int)0x4,
   APPS_SYS_SQIF_WINDOWS__MUTEX_CLAIMED_BY_P3 = (int)0x8,
   APPS_SYS_SQIF_WINDOWS__MUTEX_DISABLED    = (int)0xF,
   MAX_APPS_SYS_SQIF_WINDOWS__MUTEX_LOCK_ENUM = (int)0xF
};
typedef enum apps_sys_sqif_windows__mutex_lock_enum_enum apps_sys_sqif_windows__mutex_lock_enum;


enum apps_sys_sqif_windows_access_ctrl_posn_enum
{
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_SQIF_WINDOWS_ACCESS_CTRL_APPS_SYS_SQIF_WINDOWS__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_sqif_windows_access_ctrl_posn_enum apps_sys_sqif_windows_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SQIF_WINDOWS */

#if defined(IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1

/* -- apps_banked_exceptions_p1 -- Control of banked exceptions for P1 -- */

enum apps_banked_exceptions_p1__access_ctrl_enum_posn_enum
{
   APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_POSN = (int)0,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_POSN = (int)1,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_POSN = (int)2,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_POSN = (int)3,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_BANKED_EXCEPTIONS_P1__ACCESS_CTRL_ENUM_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_banked_exceptions_p1__access_ctrl_enum_posn_enum apps_banked_exceptions_p1__access_ctrl_enum_posn;

#define APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum apps_banked_exceptions_p1__p0_access_permission_enum
{
   APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_BLOCKED = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_banked_exceptions_p1__p0_access_permission_enum apps_banked_exceptions_p1__p0_access_permission;


enum apps_banked_exceptions_p1__p1_access_permission_enum
{
   APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_BLOCKED = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_banked_exceptions_p1__p1_access_permission_enum apps_banked_exceptions_p1__p1_access_permission;


enum apps_banked_exceptions_p1__p2_access_permission_enum
{
   APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_BLOCKED = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_banked_exceptions_p1__p2_access_permission_enum apps_banked_exceptions_p1__p2_access_permission;


enum apps_banked_exceptions_p1__p3_access_permission_enum
{
   APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_BLOCKED = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_UNBLOCKED = (int)0x1,
   MAX_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_banked_exceptions_p1__p3_access_permission_enum apps_banked_exceptions_p1__p3_access_permission;


enum apps_banked_exceptions_p1__exception_source_enum
{
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_REGISTERS = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_VML_CPU0 = (int)0x1,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_VML_CPU1 = (int)0x2,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_VML_TBUS_REMOTE = (int)0x3,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_VML_DMAC = (int)0x4,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_VM_CPU1_REMOTE_REQ = (int)0x5,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CPU1_ACCESS_FAULT = (int)0x6,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE0_RAM0_ACCESS = (int)0x9,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE0_RAM1_ACCESS = (int)0xA,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE0_TAG_ACCESS = (int)0xB,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE1_RAM0_ACCESS = (int)0xC,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE1_RAM1_ACCESS = (int)0xD,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CACHE1_TAG_ACCESS = (int)0xE,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_SQIF0_TBUS_ACCESS = (int)0xF,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_SQIF1_TBUS_ACCESS = (int)0x10,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CPU0_VM_BUFFER_MAP = (int)0x11,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_CPU1_VM_BUFFER_MAP = (int)0x12,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_REMOTE_REGISTERS = (int)0x13,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_MAX = (int)0x14,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_WIDTH_SPOILER = (int)0x7FFFFFFF,
   APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE_SDIO_HOST_DM0_ACCESS = (int)0x8,
   MAX_APPS_BANKED_EXCEPTIONS_P1__EXCEPTION_SOURCE = (int)0x7FFFFFFF
};
typedef enum apps_banked_exceptions_p1__exception_source_enum apps_banked_exceptions_p1__exception_source;


enum apps_banked_exceptions_p1__mutex_lock_enum_enum
{
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_AVAILABLE = (int)0x0,
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_CLAIMED_BY_P0 = (int)0x1,
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_CLAIMED_BY_P1 = (int)0x2,
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_CLAIMED_BY_P2 = (int)0x4,
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_CLAIMED_BY_P3 = (int)0x8,
   APPS_BANKED_EXCEPTIONS_P1__MUTEX_DISABLED = (int)0xF,
   MAX_APPS_BANKED_EXCEPTIONS_P1__MUTEX_LOCK_ENUM = (int)0xF
};
typedef enum apps_banked_exceptions_p1__mutex_lock_enum_enum apps_banked_exceptions_p1__mutex_lock_enum;


enum apps_banked_exceptions_p1_access_ctrl_posn_enum
{
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_BANKED_EXCEPTIONS_P1_ACCESS_CTRL_APPS_BANKED_EXCEPTIONS_P1__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_banked_exceptions_p1_access_ctrl_posn_enum apps_banked_exceptions_p1_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1 */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_BANKED_EXCEPTIONS_P1 */

#if defined(IO_DEFS_MODULE_KALIMBA_READ_CACHE) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_READ_CACHE
#define __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_READ_CACHE

/* -- kalimba_read_cache -- Kalimba read cache block configuration registers -- */

enum kalimba_read_cache_port_id_enum_enum
{
   KALIMBA_READ_CACHE_PORT_ID_PM            = (int)0x0,
   KALIMBA_READ_CACHE_PORT_ID_DM            = (int)0x1,
   MAX_KALIMBA_READ_CACHE_PORT_ID_ENUM      = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE_PORT_ID_ENUM (0x2)
typedef enum kalimba_read_cache_port_id_enum_enum kalimba_read_cache_port_id_enum;


enum kalimba_read_cache_request_state_enum
{
   KALIMBA_READ_CACHE_REQ_STATE_IDLE        = (int)0x0,
   KALIMBA_READ_CACHE_REQ_STATE_STALL       = (int)0x1,
   KALIMBA_READ_CACHE_REQ_STATE_ON_PROGRESS = (int)0x2,
   MAX_KALIMBA_READ_CACHE_REQUEST_STATE     = (int)0x2
};
#define NUM_KALIMBA_READ_CACHE_REQUEST_STATE (0x3)
typedef enum kalimba_read_cache_request_state_enum kalimba_read_cache_request_state;


enum kalimba_read_cache_tag_2way_enum_posn_enum
{
   KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_LSB_POSN     = (int)0,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_MSB_POSN     = (int)3,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_MSB_POSN = (int)3,
   KALIMBA_READ_CACHE_TAG_2WAY_LRU_POSN               = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LRU_LSB_POSN = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LRU_MSB_POSN = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_LSB_POSN      = (int)5,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_LSB_POSN = (int)5,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_MSB_POSN      = (int)15,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_MSB_POSN = (int)15,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_LSB_POSN     = (int)16,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_MSB_POSN     = (int)19,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_MSB_POSN = (int)19,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY1_RESERVED_POSN     = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_RESERVED_LSB_POSN = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_RESERVED_MSB_POSN = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_LSB_POSN      = (int)21,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_LSB_POSN = (int)21,
   KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_MSB_POSN      = (int)31,
   KALIMBA_READ_CACHE_TAG_2WAY_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_tag_2way_enum_posn_enum kalimba_read_cache_tag_2way_enum_posn;

#define KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY0_SLVB_MSB_MASK (0x00000008u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LRU_MASK     (0x00000010u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_LSB_MASK (0x00000020u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY0_TAG_MSB_MASK (0x00008000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_LSB_MASK (0x00010000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY1_SLVB_MSB_MASK (0x00080000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY1_RESERVED_MASK (0x00100000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_LSB_MASK (0x00200000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_WAY1_TAG_MSB_MASK (0x80000000u)

enum kalimba_read_cache_tag_2way_lockable_enum_posn_enum
{
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_MSB_POSN = (int)3,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_MSB_POSN = (int)3,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_LOCKED_POSN = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_LOCKED_LSB_POSN = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_LOCKED_MSB_POSN = (int)4,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_LSB_POSN = (int)5,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_LSB_POSN = (int)5,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_MSB_POSN = (int)14,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_MSB_POSN = (int)14,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_RESERVED_POSN = (int)15,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_RESERVED_LSB_POSN = (int)15,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_RESERVED_MSB_POSN = (int)15,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_MSB_POSN = (int)19,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_MSB_POSN = (int)19,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_LOCKED_POSN = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_LOCKED_LSB_POSN = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_LOCKED_MSB_POSN = (int)20,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_LSB_POSN = (int)21,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_LSB_POSN = (int)21,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_MSB_POSN = (int)30,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_MSB_POSN = (int)30,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_LRU_POSN      = (int)31,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_LRU_LSB_POSN = (int)31,
   KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_ENUM_KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_LRU_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_tag_2way_lockable_enum_posn_enum kalimba_read_cache_tag_2way_lockable_enum_posn;

#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_SLVB_MSB_MASK (0x00000008u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_LOCKED_MASK (0x00000010u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_LSB_MASK (0x00000020u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_TAG_MSB_MASK (0x00004000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY0_RESERVED_MASK (0x00008000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_LSB_MASK (0x00010000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_SLVB_MSB_MASK (0x00080000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_LOCKED_MASK (0x00100000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_LSB_MASK (0x00200000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_WAY1_TAG_MSB_MASK (0x40000000u)
#define KALIMBA_READ_CACHE_TAG_2WAY_LOCKABLE_LRU_MASK (0x80000000u)

enum kalimba_read_cache_tag_direct_enum_posn_enum
{
   KALIMBA_READ_CACHE_TAG_DIRECT_SLVB_LSB_POSN        = (int)0,
   KALIMBA_READ_CACHE_TAG_DIRECT_ENUM_WORD1_KALIMBA_READ_CACHE_TAG_DIRECT_SLVB_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_TAG_DIRECT_SLVB_MSB_POSN        = (int)31,
   KALIMBA_READ_CACHE_TAG_DIRECT_ENUM_WORD1_KALIMBA_READ_CACHE_TAG_DIRECT_SLVB_MSB_POSN = (int)31,
   KALIMBA_READ_CACHE_TAG_DIRECT_TAG_LSB_POSN         = (int)32,
   KALIMBA_READ_CACHE_TAG_DIRECT_ENUM_WORD0_KALIMBA_READ_CACHE_TAG_DIRECT_TAG_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_TAG_DIRECT_TAG_MSB_POSN         = (int)45,
   KALIMBA_READ_CACHE_TAG_DIRECT_ENUM_WORD0_KALIMBA_READ_CACHE_TAG_DIRECT_TAG_MSB_POSN = (int)13
};
typedef enum kalimba_read_cache_tag_direct_enum_posn_enum kalimba_read_cache_tag_direct_enum_posn;


enum kalimba_read_cache_control_posn_enum
{
   KALIMBA_READ_CACHE_CONTROL_MODE_LSB_POSN           = (int)0,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_MODE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_CONTROL_MODE_MSB_POSN           = (int)1,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_MODE_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_CONTROL_ENABLE_PM_COUNTERS_POSN = (int)2,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_PM_COUNTERS_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_PM_COUNTERS_MSB_POSN = (int)2,
   KALIMBA_READ_CACHE_CONTROL_ENABLE_DM_COUNTERS_POSN = (int)3,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_DM_COUNTERS_LSB_POSN = (int)3,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_DM_COUNTERS_MSB_POSN = (int)3,
   KALIMBA_READ_CACHE_CONTROL_ENABLE_SLAVE_COUNTERS_POSN = (int)4,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_SLAVE_COUNTERS_LSB_POSN = (int)4,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_SLAVE_COUNTERS_MSB_POSN = (int)4,
   KALIMBA_READ_CACHE_CONTROL_ENABLE_BUFFER_IN_BYPASS_POSN = (int)5,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_BUFFER_IN_BYPASS_LSB_POSN = (int)5,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_BUFFER_IN_BYPASS_MSB_POSN = (int)5,
   KALIMBA_READ_CACHE_CONTROL_ENABLE_MASTER_COUNTERS_POSN = (int)6,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_MASTER_COUNTERS_LSB_POSN = (int)6,
   KALIMBA_READ_CACHE_CONTROL_KALIMBA_READ_CACHE_CONTROL_ENABLE_MASTER_COUNTERS_MSB_POSN = (int)6
};
typedef enum kalimba_read_cache_control_posn_enum kalimba_read_cache_control_posn;

#define KALIMBA_READ_CACHE_CONTROL_MODE_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_CONTROL_MODE_MSB_MASK (0x00000002u)
#define KALIMBA_READ_CACHE_CONTROL_ENABLE_PM_COUNTERS_MASK (0x00000004u)
#define KALIMBA_READ_CACHE_CONTROL_ENABLE_DM_COUNTERS_MASK (0x00000008u)
#define KALIMBA_READ_CACHE_CONTROL_ENABLE_SLAVE_COUNTERS_MASK (0x00000010u)
#define KALIMBA_READ_CACHE_CONTROL_ENABLE_BUFFER_IN_BYPASS_MASK (0x00000020u)
#define KALIMBA_READ_CACHE_CONTROL_ENABLE_MASTER_COUNTERS_MASK (0x00000040u)

enum kalimba_read_cache_control_mode_enum
{
   KALIMBA_READ_CACHE_MODE_DISABLED         = (int)0x0,
   KALIMBA_READ_CACHE_MODE_2WAY             = (int)0x1,
   KALIMBA_READ_CACHE_MODE_2WAYHALF         = (int)0x2,
   KALIMBA_READ_CACHE_MODE_DIRECT           = (int)0x3,
   MAX_KALIMBA_READ_CACHE_CONTROL_MODE      = (int)0x3
};
#define NUM_KALIMBA_READ_CACHE_CONTROL_MODE (0x4)
typedef enum kalimba_read_cache_control_mode_enum kalimba_read_cache_control_mode;


enum kalimba_read_cache_debug_data_posn_enum
{
   KALIMBA_READ_CACHE_REQUEST_MODE_LSB_POSN           = (int)0,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_REQUEST_MODE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_REQUEST_MODE_MSB_POSN           = (int)1,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_REQUEST_MODE_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_DUMMY0_LSB_POSN                 = (int)2,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY0_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_DUMMY0_MSB_POSN                 = (int)3,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY0_MSB_POSN = (int)3,
   KALIMBA_READ_CACHE_IS_STUCK_POSN                   = (int)4,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_IS_STUCK_LSB_POSN = (int)4,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_IS_STUCK_MSB_POSN = (int)4,
   KALIMBA_READ_CACHE_FSM_NOT_BUSY_POSN               = (int)5,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FSM_NOT_BUSY_LSB_POSN = (int)5,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FSM_NOT_BUSY_MSB_POSN = (int)5,
   KALIMBA_READ_CACHE_REQ_CANCELED_POSN               = (int)6,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_REQ_CANCELED_LSB_POSN = (int)6,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_REQ_CANCELED_MSB_POSN = (int)6,
   KALIMBA_READ_CACHE_DUMMY1_POSN                     = (int)7,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY1_LSB_POSN = (int)7,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY1_MSB_POSN = (int)7,
   KALIMBA_READ_CACHE_PM_REQ_STATE_LSB_POSN           = (int)8,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_PM_REQ_STATE_LSB_POSN = (int)8,
   KALIMBA_READ_CACHE_PM_REQ_STATE_MSB_POSN           = (int)9,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_PM_REQ_STATE_MSB_POSN = (int)9,
   KALIMBA_READ_CACHE_DM_REQ_STATE_LSB_POSN           = (int)10,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DM_REQ_STATE_LSB_POSN = (int)10,
   KALIMBA_READ_CACHE_DM_REQ_STATE_MSB_POSN           = (int)11,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DM_REQ_STATE_MSB_POSN = (int)11,
   KALIMBA_READ_CACHE_DUMMY2_LSB_POSN                 = (int)12,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY2_LSB_POSN = (int)12,
   KALIMBA_READ_CACHE_DUMMY2_MSB_POSN                 = (int)15,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY2_MSB_POSN = (int)15,
   KALIMBA_READ_CACHE_FETCH_STATE_POSN                = (int)16,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FETCH_STATE_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FETCH_STATE_MSB_POSN = (int)16,
   KALIMBA_READ_CACHE_FILL_STATE_POSN                 = (int)17,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FILL_STATE_LSB_POSN = (int)17,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FILL_STATE_MSB_POSN = (int)17,
   KALIMBA_READ_CACHE_FETCH_ID_POSN                   = (int)18,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FETCH_ID_LSB_POSN = (int)18,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FETCH_ID_MSB_POSN = (int)18,
   KALIMBA_READ_CACHE_FILL_ID_POSN                    = (int)19,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FILL_ID_LSB_POSN = (int)19,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_FILL_ID_MSB_POSN = (int)19,
   KALIMBA_READ_CACHE_BUFF_REQ_CNTR_LSB_POSN          = (int)20,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_BUFF_REQ_CNTR_LSB_POSN = (int)20,
   KALIMBA_READ_CACHE_BUFF_REQ_CNTR_MSB_POSN          = (int)21,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_BUFF_REQ_CNTR_MSB_POSN = (int)21,
   KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_LSB_POSN       = (int)22,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_LSB_POSN = (int)22,
   KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_MSB_POSN       = (int)23,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_MSB_POSN = (int)23,
   REQ_ID_OUT_POSN                                    = (int)24,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_ID_OUT_LSB_POSN  = (int)24,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_ID_OUT_MSB_POSN  = (int)24,
   REQ_RD_EN_OUT_POSN                                 = (int)25,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_RD_EN_OUT_LSB_POSN = (int)25,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_RD_EN_OUT_MSB_POSN = (int)25,
   REQ_HIT_POSN                                       = (int)26,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_HIT_LSB_POSN     = (int)26,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_HIT_MSB_POSN     = (int)26,
   REQ_MISS_POSN                                      = (int)27,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_MISS_LSB_POSN    = (int)27,
   KALIMBA_READ_CACHE_DEBUG_DATA_REQ_MISS_MSB_POSN    = (int)27,
   KALIMBA_READ_CACHE_DUMMY3_LSB_POSN                 = (int)28,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY3_LSB_POSN = (int)28,
   KALIMBA_READ_CACHE_DUMMY3_MSB_POSN                 = (int)31,
   KALIMBA_READ_CACHE_DEBUG_DATA_KALIMBA_READ_CACHE_DUMMY3_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_debug_data_posn_enum kalimba_read_cache_debug_data_posn;

#define KALIMBA_READ_CACHE_REQUEST_MODE_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_REQUEST_MODE_MSB_MASK (0x00000002u)
#define KALIMBA_READ_CACHE_DUMMY0_LSB_MASK       (0x00000004u)
#define KALIMBA_READ_CACHE_DUMMY0_MSB_MASK       (0x00000008u)
#define KALIMBA_READ_CACHE_IS_STUCK_MASK         (0x00000010u)
#define KALIMBA_READ_CACHE_FSM_NOT_BUSY_MASK     (0x00000020u)
#define KALIMBA_READ_CACHE_REQ_CANCELED_MASK     (0x00000040u)
#define KALIMBA_READ_CACHE_DUMMY1_MASK           (0x00000080u)
#define KALIMBA_READ_CACHE_PM_REQ_STATE_LSB_MASK (0x00000100u)
#define KALIMBA_READ_CACHE_PM_REQ_STATE_MSB_MASK (0x00000200u)
#define KALIMBA_READ_CACHE_DM_REQ_STATE_LSB_MASK (0x00000400u)
#define KALIMBA_READ_CACHE_DM_REQ_STATE_MSB_MASK (0x00000800u)
#define KALIMBA_READ_CACHE_DUMMY2_LSB_MASK       (0x00001000u)
#define KALIMBA_READ_CACHE_DUMMY2_MSB_MASK       (0x00008000u)
#define KALIMBA_READ_CACHE_FETCH_STATE_MASK      (0x00010000u)
#define KALIMBA_READ_CACHE_FILL_STATE_MASK       (0x00020000u)
#define KALIMBA_READ_CACHE_FETCH_ID_MASK         (0x00040000u)
#define KALIMBA_READ_CACHE_FILL_ID_MASK          (0x00080000u)
#define KALIMBA_READ_CACHE_BUFF_REQ_CNTR_LSB_MASK (0x00100000u)
#define KALIMBA_READ_CACHE_BUFF_REQ_CNTR_MSB_MASK (0x00200000u)
#define KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_LSB_MASK (0x00400000u)
#define KALIMBA_READ_CACHE_BUFF_SAMPLE_CNTR_MSB_MASK (0x00800000u)
#define REQ_ID_OUT_MASK                          (0x01000000u)
#define REQ_RD_EN_OUT_MASK                       (0x02000000u)
#define REQ_HIT_MASK                             (0x04000000u)
#define REQ_MISS_MASK                            (0x08000000u)
#define KALIMBA_READ_CACHE_DUMMY3_LSB_MASK       (0x10000000u)
#define KALIMBA_READ_CACHE_DUMMY3_MSB_MASK       (0x80000000u)

enum kalimba_read_cache_request_mode_enum
{
   KALIMBA_READ_CACHE_REQ_FREE              = (int)0x0,
   KALIMBA_READ_CACHE_REQ_ILEGAL            = (int)0x1,
   KALIMBA_READ_CACHE_REQ_DM_LOCK           = (int)0x2,
   KALIMBA_READ_CACHE_REQ_PM_LOCK           = (int)0x3,
   MAX_KALIMBA_READ_CACHE_REQUEST_MODE      = (int)0x3
};
#define NUM_KALIMBA_READ_CACHE_REQUEST_MODE (0x4)
typedef enum kalimba_read_cache_request_mode_enum kalimba_read_cache_request_mode;


enum kalimba_read_cache_fetch_state_enum
{
   KALIMBA_READ_CACHE_FETCH_IDLE            = (int)0x0,
   KALIMBA_READ_CACHE_FETCH_FETCH           = (int)0x1,
   MAX_KALIMBA_READ_CACHE_FETCH_STATE       = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE_FETCH_STATE (0x2)
typedef enum kalimba_read_cache_fetch_state_enum kalimba_read_cache_fetch_state;


enum kalimba_read_cache_fill_state_enum
{
   KALIMBA_READ_CACHE_FILL_IDLE             = (int)0x0,
   KALIMBA_READ_CACHE_FILL_FILL             = (int)0x1,
   MAX_KALIMBA_READ_CACHE_FILL_STATE        = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE_FILL_STATE (0x2)
typedef enum kalimba_read_cache_fill_state_enum kalimba_read_cache_fill_state;


enum kalimba_read_cache_dm_config_posn_enum
{
   KALIMBA_READ_CACHE_DM_CONFIG_PM_ABORT_POSN         = (int)0,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_PM_ABORT_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_PM_ABORT_MSB_POSN = (int)0,
   KALIMBA_READ_CACHE_DM_CONFIG_DM_ABORT_POSN         = (int)1,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_DM_ABORT_LSB_POSN = (int)1,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_DM_ABORT_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_DM_CONFIG_IDLE_ABORT_POSN       = (int)2,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_IDLE_ABORT_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_DM_CONFIG_KALIMBA_READ_CACHE_DM_CONFIG_IDLE_ABORT_MSB_POSN = (int)2
};
typedef enum kalimba_read_cache_dm_config_posn_enum kalimba_read_cache_dm_config_posn;

#define KALIMBA_READ_CACHE_DM_CONFIG_PM_ABORT_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_DM_CONFIG_DM_ABORT_MASK (0x00000002u)
#define KALIMBA_READ_CACHE_DM_CONFIG_IDLE_ABORT_MASK (0x00000004u)

enum kalimba_read_cache_invalidate_control_enum
{
   KALIMBA_READ_CACHE_IVALIDATE_ILEGAL_VALUE = (int)0x0,
   KALIMBA_READ_CACHE_INVALIDATE_2WAY_TAG_RAMS = (int)0x1,
   KALIMBA_READ_CACHE_INVALIDATE_2WAY_HALF_TAG_RAMS = (int)0x2,
   KALIMBA_READ_CACHE_INVALIDATE_DIRECT_TAG_FLOPS = (int)0x3,
   MAX_KALIMBA_READ_CACHE_INVALIDATE_CONTROL = (int)0x3
};
#define NUM_KALIMBA_READ_CACHE_INVALIDATE_CONTROL (0x4)
typedef enum kalimba_read_cache_invalidate_control_enum kalimba_read_cache_invalidate_control;


enum kalimba_read_cache_params_posn_enum
{
   KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_LSB_POSN      = (int)0,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_MSB_POSN      = (int)7,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_MSB_POSN = (int)7,
   KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_LSB_POSN = (int)8,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_LSB_POSN = (int)8,
   KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_MSB_POSN = (int)15,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_MSB_POSN = (int)15,
   KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_LSB_POSN = (int)16,
   KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_MSB_POSN = (int)23,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_MSB_POSN = (int)23,
   KALIMBA_READ_CACHE_PARAMS_EXTENT_LSB_POSN          = (int)24,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_EXTENT_LSB_POSN = (int)24,
   KALIMBA_READ_CACHE_PARAMS_EXTENT_MSB_POSN          = (int)31,
   KALIMBA_READ_CACHE_PARAMS_KALIMBA_READ_CACHE_PARAMS_EXTENT_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_params_posn_enum kalimba_read_cache_params_posn;

#define KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_PARAMS_WORD_WIDTH_MSB_MASK (0x00000080u)
#define KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_LSB_MASK (0x00000100u)
#define KALIMBA_READ_CACHE_PARAMS_DIRECT_LINE_LENGTH_MSB_MASK (0x00008000u)
#define KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_LSB_MASK (0x00010000u)
#define KALIMBA_READ_CACHE_PARAMS_2WAY_LINE_LENGTH_MSB_MASK (0x00800000u)
#define KALIMBA_READ_CACHE_PARAMS_EXTENT_LSB_MASK (0x01000000u)
#define KALIMBA_READ_CACHE_PARAMS_EXTENT_MSB_MASK (0x80000000u)

enum kalimba_read_cache_pm_config_posn_enum
{
   KALIMBA_READ_CACHE_PM_CONFIG_PM_ABORT_POSN         = (int)0,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_PM_ABORT_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_PM_ABORT_MSB_POSN = (int)0,
   KALIMBA_READ_CACHE_PM_CONFIG_DM_ABORT_POSN         = (int)1,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_DM_ABORT_LSB_POSN = (int)1,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_DM_ABORT_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_PM_CONFIG_IDLE_ABORT_POSN       = (int)2,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_IDLE_ABORT_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_PM_CONFIG_KALIMBA_READ_CACHE_PM_CONFIG_IDLE_ABORT_MSB_POSN = (int)2
};
typedef enum kalimba_read_cache_pm_config_posn_enum kalimba_read_cache_pm_config_posn;

#define KALIMBA_READ_CACHE_PM_CONFIG_PM_ABORT_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_PM_CONFIG_DM_ABORT_MASK (0x00000002u)
#define KALIMBA_READ_CACHE_PM_CONFIG_IDLE_ABORT_MASK (0x00000004u)

enum kalimba_read_cache_transition_status_posn_enum
{
   KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_LSB_POSN    = (int)0,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_MSB_POSN    = (int)1,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_PENDING_MODE_CHANE_POSN         = (int)2,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_PENDING_MODE_CHANE_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_PENDING_MODE_CHANE_MSB_POSN = (int)2,
   KALIMBA_READ_CACHE_PENDING_INVALDIATE_POSN         = (int)3,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_PENDING_INVALDIATE_LSB_POSN = (int)3,
   KALIMBA_READ_CACHE_TRANSITION_STATUS_KALIMBA_READ_CACHE_PENDING_INVALDIATE_MSB_POSN = (int)3
};
typedef enum kalimba_read_cache_transition_status_posn_enum kalimba_read_cache_transition_status_posn;

#define KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_ACTUAL_CONTROL_MODE_MSB_MASK (0x00000002u)
#define KALIMBA_READ_CACHE_PENDING_MODE_CHANE_MASK (0x00000004u)
#define KALIMBA_READ_CACHE_PENDING_INVALDIATE_MASK (0x00000008u)

enum kalimba_read_cache_debug_en_posn_enum
{
   KALIMBA_READ_CACHE_DEBUG_EN_POSN                   = (int)0,
   KALIMBA_READ_CACHE_DEBUG_EN_KALIMBA_READ_CACHE_DEBUG_EN_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_DEBUG_EN_KALIMBA_READ_CACHE_DEBUG_EN_MSB_POSN = (int)0
};
typedef enum kalimba_read_cache_debug_en_posn_enum kalimba_read_cache_debug_en_posn;

#define KALIMBA_READ_CACHE_DEBUG_EN_MASK         (0x00000001u)

enum kalimba_read_cache_dm_hit_counter_posn_enum
{
   KALIMBA_READ_CACHE_DM_HIT_COUNTER_LSB_POSN         = (int)0,
   KALIMBA_READ_CACHE_DM_HIT_COUNTER_KALIMBA_READ_CACHE_DM_HIT_COUNTER_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_DM_HIT_COUNTER_MSB_POSN         = (int)31,
   KALIMBA_READ_CACHE_DM_HIT_COUNTER_KALIMBA_READ_CACHE_DM_HIT_COUNTER_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_dm_hit_counter_posn_enum kalimba_read_cache_dm_hit_counter_posn;

#define KALIMBA_READ_CACHE_DM_HIT_COUNTER_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_DM_HIT_COUNTER_MSB_MASK (0x80000000u)

enum kalimba_read_cache_dm_miss_counter_posn_enum
{
   KALIMBA_READ_CACHE_DM_MISS_COUNTER_LSB_POSN        = (int)0,
   KALIMBA_READ_CACHE_DM_MISS_COUNTER_KALIMBA_READ_CACHE_DM_MISS_COUNTER_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_DM_MISS_COUNTER_MSB_POSN        = (int)31,
   KALIMBA_READ_CACHE_DM_MISS_COUNTER_KALIMBA_READ_CACHE_DM_MISS_COUNTER_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_dm_miss_counter_posn_enum kalimba_read_cache_dm_miss_counter_posn;

#define KALIMBA_READ_CACHE_DM_MISS_COUNTER_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_DM_MISS_COUNTER_MSB_MASK (0x80000000u)

enum kalimba_read_cache_invalidate_num_lines_posn_enum
{
   KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_LSB_POSN   = (int)0,
   KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_MSB_POSN   = (int)15,
   KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_MSB_POSN = (int)15
};
typedef enum kalimba_read_cache_invalidate_num_lines_posn_enum kalimba_read_cache_invalidate_num_lines_posn;

#define KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_INVALIDATE_NUM_LINES_MSB_MASK (0x00008000u)

enum kalimba_read_cache_invalidate_start_line_posn_enum
{
   KALIMBA_READ_CACHE_INVALIDATE_START_LINE_LSB_POSN  = (int)0,
   KALIMBA_READ_CACHE_INVALIDATE_START_LINE_KALIMBA_READ_CACHE_INVALIDATE_START_LINE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_INVALIDATE_START_LINE_MSB_POSN  = (int)15,
   KALIMBA_READ_CACHE_INVALIDATE_START_LINE_KALIMBA_READ_CACHE_INVALIDATE_START_LINE_MSB_POSN = (int)15
};
typedef enum kalimba_read_cache_invalidate_start_line_posn_enum kalimba_read_cache_invalidate_start_line_posn;

#define KALIMBA_READ_CACHE_INVALIDATE_START_LINE_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_INVALIDATE_START_LINE_MSB_MASK (0x00008000u)

enum kalimba_read_cache_master_wait_counter_dm_posn_enum
{
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_MSB_POSN = (int)31,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_master_wait_counter_dm_posn_enum kalimba_read_cache_master_wait_counter_dm_posn;

#define KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_DM_MSB_MASK (0x80000000u)

enum kalimba_read_cache_master_wait_counter_pm_posn_enum
{
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_MSB_POSN = (int)31,
   KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_master_wait_counter_pm_posn_enum kalimba_read_cache_master_wait_counter_pm_posn;

#define KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_MASTER_WAIT_COUNTER_PM_MSB_MASK (0x80000000u)

enum kalimba_read_cache_miss_log_base_addr_posn_enum
{
   KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_LSB_POSN     = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_MSB_POSN     = (int)31,
   KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_miss_log_base_addr_posn_enum kalimba_read_cache_miss_log_base_addr_posn;

#define KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_MISS_LOG_BASE_ADDR_MSB_MASK (0x80000000u)

enum kalimba_read_cache_miss_log_enable_posn_enum
{
   KALIMBA_READ_CACHE_MISS_LOG_ENABLE_POSN            = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_ENABLE_KALIMBA_READ_CACHE_MISS_LOG_ENABLE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_ENABLE_KALIMBA_READ_CACHE_MISS_LOG_ENABLE_MSB_POSN = (int)0
};
typedef enum kalimba_read_cache_miss_log_enable_posn_enum kalimba_read_cache_miss_log_enable_posn;

#define KALIMBA_READ_CACHE_MISS_LOG_ENABLE_MASK  (0x00000001u)

enum kalimba_read_cache_miss_log_index_posn_enum
{
   KALIMBA_READ_CACHE_MISS_LOG_INDEX_LSB_POSN         = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_INDEX_KALIMBA_READ_CACHE_MISS_LOG_INDEX_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_INDEX_MSB_POSN         = (int)15,
   KALIMBA_READ_CACHE_MISS_LOG_INDEX_KALIMBA_READ_CACHE_MISS_LOG_INDEX_MSB_POSN = (int)15
};
typedef enum kalimba_read_cache_miss_log_index_posn_enum kalimba_read_cache_miss_log_index_posn;

#define KALIMBA_READ_CACHE_MISS_LOG_INDEX_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_MISS_LOG_INDEX_MSB_MASK (0x00008000u)

enum kalimba_read_cache_miss_log_size_posn_enum
{
   KALIMBA_READ_CACHE_MISS_LOG_SIZE_LSB_POSN          = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_SIZE_KALIMBA_READ_CACHE_MISS_LOG_SIZE_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_MISS_LOG_SIZE_MSB_POSN          = (int)15,
   KALIMBA_READ_CACHE_MISS_LOG_SIZE_KALIMBA_READ_CACHE_MISS_LOG_SIZE_MSB_POSN = (int)15
};
typedef enum kalimba_read_cache_miss_log_size_posn_enum kalimba_read_cache_miss_log_size_posn;

#define KALIMBA_READ_CACHE_MISS_LOG_SIZE_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_MISS_LOG_SIZE_MSB_MASK (0x00008000u)

enum kalimba_read_cache_pm_hit_counter_posn_enum
{
   KALIMBA_READ_CACHE_PM_HIT_COUNTER_LSB_POSN         = (int)0,
   KALIMBA_READ_CACHE_PM_HIT_COUNTER_KALIMBA_READ_CACHE_PM_HIT_COUNTER_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_PM_HIT_COUNTER_MSB_POSN         = (int)31,
   KALIMBA_READ_CACHE_PM_HIT_COUNTER_KALIMBA_READ_CACHE_PM_HIT_COUNTER_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_pm_hit_counter_posn_enum kalimba_read_cache_pm_hit_counter_posn;

#define KALIMBA_READ_CACHE_PM_HIT_COUNTER_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_PM_HIT_COUNTER_MSB_MASK (0x80000000u)

enum kalimba_read_cache_pm_miss_counter_posn_enum
{
   KALIMBA_READ_CACHE_PM_MISS_COUNTER_LSB_POSN        = (int)0,
   KALIMBA_READ_CACHE_PM_MISS_COUNTER_KALIMBA_READ_CACHE_PM_MISS_COUNTER_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_PM_MISS_COUNTER_MSB_POSN        = (int)31,
   KALIMBA_READ_CACHE_PM_MISS_COUNTER_KALIMBA_READ_CACHE_PM_MISS_COUNTER_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_pm_miss_counter_posn_enum kalimba_read_cache_pm_miss_counter_posn;

#define KALIMBA_READ_CACHE_PM_MISS_COUNTER_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_PM_MISS_COUNTER_MSB_MASK (0x80000000u)

enum kalimba_read_cache_slave_wait_counter_posn_enum
{
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_LSB_POSN     = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_MSB_POSN     = (int)31,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_slave_wait_counter_posn_enum kalimba_read_cache_slave_wait_counter_posn;

#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_MSB_MASK (0x80000000u)

enum kalimba_read_cache_slave_wait_counter_dm_posn_enum
{
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_LSB_POSN  = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_MSB_POSN  = (int)31,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_slave_wait_counter_dm_posn_enum kalimba_read_cache_slave_wait_counter_dm_posn;

#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_DM_MSB_MASK (0x80000000u)

enum kalimba_read_cache_slave_wait_counter_pm_posn_enum
{
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_LSB_POSN  = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_MSB_POSN  = (int)31,
   KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_MSB_POSN = (int)31
};
typedef enum kalimba_read_cache_slave_wait_counter_pm_posn_enum kalimba_read_cache_slave_wait_counter_pm_posn;

#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_LSB_MASK (0x00000001u)
#define KALIMBA_READ_CACHE_SLAVE_WAIT_COUNTER_PM_MSB_MASK (0x80000000u)

enum kalimba_read_cache_soft_reset_posn_enum
{
   KALIMBA_READ_CACHE_SOFT_RESET_POSN                 = (int)0,
   KALIMBA_READ_CACHE_SOFT_RESET_KALIMBA_READ_CACHE_SOFT_RESET_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_SOFT_RESET_KALIMBA_READ_CACHE_SOFT_RESET_MSB_POSN = (int)0
};
typedef enum kalimba_read_cache_soft_reset_posn_enum kalimba_read_cache_soft_reset_posn;

#define KALIMBA_READ_CACHE_SOFT_RESET_MASK       (0x00000001u)

enum kalimba_read_cache__access_ctrl_enum_posn_enum
{
   KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_POSN      = (int)0,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_POSN      = (int)1,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_POSN      = (int)2,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_POSN      = (int)3,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   KALIMBA_READ_CACHE__ACCESS_CTRL_ENUM_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum kalimba_read_cache__access_ctrl_enum_posn_enum kalimba_read_cache__access_ctrl_enum_posn;

#define KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum kalimba_read_cache__p0_access_permission_enum
{
   KALIMBA_READ_CACHE__P0_ACCESS_BLOCKED    = (int)0x0,
   KALIMBA_READ_CACHE__P0_ACCESS_UNBLOCKED  = (int)0x1,
   MAX_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION (0x2)
typedef enum kalimba_read_cache__p0_access_permission_enum kalimba_read_cache__p0_access_permission;


enum kalimba_read_cache__p1_access_permission_enum
{
   KALIMBA_READ_CACHE__P1_ACCESS_BLOCKED    = (int)0x0,
   KALIMBA_READ_CACHE__P1_ACCESS_UNBLOCKED  = (int)0x1,
   MAX_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION (0x2)
typedef enum kalimba_read_cache__p1_access_permission_enum kalimba_read_cache__p1_access_permission;


enum kalimba_read_cache__p2_access_permission_enum
{
   KALIMBA_READ_CACHE__P2_ACCESS_BLOCKED    = (int)0x0,
   KALIMBA_READ_CACHE__P2_ACCESS_UNBLOCKED  = (int)0x1,
   MAX_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION (0x2)
typedef enum kalimba_read_cache__p2_access_permission_enum kalimba_read_cache__p2_access_permission;


enum kalimba_read_cache__p3_access_permission_enum
{
   KALIMBA_READ_CACHE__P3_ACCESS_BLOCKED    = (int)0x0,
   KALIMBA_READ_CACHE__P3_ACCESS_UNBLOCKED  = (int)0x1,
   MAX_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION (0x2)
typedef enum kalimba_read_cache__p3_access_permission_enum kalimba_read_cache__p3_access_permission;


enum kalimba_read_cache__mutex_lock_enum_enum
{
   KALIMBA_READ_CACHE__MUTEX_AVAILABLE      = (int)0x0,
   KALIMBA_READ_CACHE__MUTEX_CLAIMED_BY_P0  = (int)0x1,
   KALIMBA_READ_CACHE__MUTEX_CLAIMED_BY_P1  = (int)0x2,
   KALIMBA_READ_CACHE__MUTEX_CLAIMED_BY_P2  = (int)0x4,
   KALIMBA_READ_CACHE__MUTEX_CLAIMED_BY_P3  = (int)0x8,
   KALIMBA_READ_CACHE__MUTEX_DISABLED       = (int)0xF,
   MAX_KALIMBA_READ_CACHE__MUTEX_LOCK_ENUM  = (int)0xF
};
typedef enum kalimba_read_cache__mutex_lock_enum_enum kalimba_read_cache__mutex_lock_enum;


enum kalimba_read_cache_access_ctrl_posn_enum
{
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   KALIMBA_READ_CACHE_ACCESS_CTRL_KALIMBA_READ_CACHE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum kalimba_read_cache_access_ctrl_posn_enum kalimba_read_cache_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_KALIMBA_READ_CACHE */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_READ_CACHE */








#if defined(IO_DEFS_MODULE_BITSERIAL0) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL0
#define __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL0

/* -- bitserial0 -- Bitserializer regs -- */

enum bitserial0_buffer_enum_posn_enum
{
   BITSERIAL0_BUFF_HANDLE_LSB_POSN                    = (int)0,
   BITSERIAL0_BUFFER_ENUM_BITSERIAL0_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL0_BUFF_HANDLE_MSB_POSN                    = (int)7,
   BITSERIAL0_BUFFER_ENUM_BITSERIAL0_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL0_REMOTE_SSID_LSB_POSN                    = (int)8,
   BITSERIAL0_BUFFER_ENUM_BITSERIAL0_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL0_REMOTE_SSID_MSB_POSN                    = (int)11,
   BITSERIAL0_BUFFER_ENUM_BITSERIAL0_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial0_buffer_enum_posn_enum bitserial0_buffer_enum_posn;

#define BITSERIAL0_BUFF_HANDLE_LSB_MASK          (0x00000001u)
#define BITSERIAL0_BUFF_HANDLE_MSB_MASK          (0x00000080u)
#define BITSERIAL0_REMOTE_SSID_LSB_MASK          (0x00000100u)
#define BITSERIAL0_REMOTE_SSID_MSB_MASK          (0x00000800u)

enum bitserial0_event_types_enum
{
   BITSERIAL0_EVENT_TX_SUCCESS              = (int)0x0,
   BITSERIAL0_EVENT_RX_SUCCESS              = (int)0x1,
   BITSERIAL0_EVENT_ERROR_IN_BUFFER         = (int)0x2,
   BITSERIAL0_EVENT_ERROR_IN_READ           = (int)0x3,
   BITSERIAL0_EVENT_SLAVE_START_STOP_BITS_ERROR = (int)0x4,
   BITSERIAL0_EVENT_FIFO_UNDERFLOW          = (int)0x5,
   BITSERIAL0_EVENT_FIFO_OVERFLOW           = (int)0x6,
   BITSERIAL0_EVENT_I2C_ACK_ERROR           = (int)0x7,
   BITSERIAL0_EVENT_I2C_LOST_ARB            = (int)0x8,
   BITSERIAL0_EVENT_I2C_NAK_STOP            = (int)0x9,
   BITSERIAL0_EVENT_I2C_NAK_RESTART         = (int)0xA,
   BITSERIAL0_EVENT_TX_NOT_CONFIGURED       = (int)0xB,
   BITSERIAL0_EVENT_RX_CMD_DETECTED         = (int)0xC,
   BITSERIAL0_EVENT_TX_STREAMING_SUCCESS    = (int)0xD,
   BITSERIAL0_EVENT_WIDTH                   = (int)0xE,
   MAX_BITSERIAL0_EVENT_TYPES               = (int)0xE
};
#define NUM_BITSERIAL0_EVENT_TYPES (0xF)
typedef enum bitserial0_event_types_enum bitserial0_event_types;


enum bitserial0_clk_control_posn_enum
{
   BITSERIAL0_MASTER_SLAVE_CLK_REQ_EN_POSN            = (int)0,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_MASTER_SLAVE_CLK_REQ_EN_LSB_POSN = (int)0,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_MASTER_SLAVE_CLK_REQ_EN_MSB_POSN = (int)0,
   BITSERIAL0_MAIN_CLK_REQ_EN_POSN                    = (int)1,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_MAIN_CLK_REQ_EN_LSB_POSN = (int)1,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_MAIN_CLK_REQ_EN_MSB_POSN = (int)1,
   BITSERIAL0_CLK_DIVIDE_LSB_POSN                     = (int)2,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_CLK_DIVIDE_LSB_POSN = (int)2,
   BITSERIAL0_CLK_DIVIDE_MSB_POSN                     = (int)9,
   BITSERIAL0_CLK_CONTROL_BITSERIAL0_CLK_DIVIDE_MSB_POSN = (int)9
};
typedef enum bitserial0_clk_control_posn_enum bitserial0_clk_control_posn;

#define BITSERIAL0_MASTER_SLAVE_CLK_REQ_EN_MASK  (0x00000001u)
#define BITSERIAL0_MAIN_CLK_REQ_EN_MASK          (0x00000002u)
#define BITSERIAL0_CLK_DIVIDE_LSB_MASK           (0x00000004u)
#define BITSERIAL0_CLK_DIVIDE_MSB_MASK           (0x00000200u)

enum bitserial0_config_posn_enum
{
   BITSERIAL0_CONFIG_CLK_INVERT_POSN                  = (int)0,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLK_INVERT_LSB_POSN = (int)0,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLK_INVERT_MSB_POSN = (int)0,
   BITSERIAL0_CONFIG_DIN_INVERT_POSN                  = (int)1,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_DIN_INVERT_LSB_POSN = (int)1,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_DIN_INVERT_MSB_POSN = (int)1,
   BITSERIAL0_CONFIG_DOUT_INVERT_POSN                 = (int)2,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_DOUT_INVERT_LSB_POSN = (int)2,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_DOUT_INVERT_MSB_POSN = (int)2,
   BITSERIAL0_CONFIG_BYTESWAP_EN_POSN                 = (int)3,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_BYTESWAP_EN_LSB_POSN = (int)3,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_BYTESWAP_EN_MSB_POSN = (int)3,
   BITSERIAL0_CONFIG_INT_EVENT_SUBSYSTEM_EN_POSN      = (int)4,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_INT_EVENT_SUBSYSTEM_EN_LSB_POSN = (int)4,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_INT_EVENT_SUBSYSTEM_EN_MSB_POSN = (int)4,
   BITSERIAL0_CONFIG_INT_EVENT_CURATOR_EN_POSN        = (int)5,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_INT_EVENT_CURATOR_EN_LSB_POSN = (int)5,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_INT_EVENT_CURATOR_EN_MSB_POSN = (int)5,
   BITSERIAL0_CONFIG_BITREVERSE_EN_POSN               = (int)6,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_BITREVERSE_EN_LSB_POSN = (int)6,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_BITREVERSE_EN_MSB_POSN = (int)6,
   BITSERIAL0_CONFIG_CLEAR_STICKY_ACK_POSN            = (int)7,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLEAR_STICKY_ACK_LSB_POSN = (int)7,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLEAR_STICKY_ACK_MSB_POSN = (int)7,
   BITSERIAL0_CONFIG_CLK_OFFSET_EN_POSN               = (int)8,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLK_OFFSET_EN_LSB_POSN = (int)8,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_CLK_OFFSET_EN_MSB_POSN = (int)8,
   BITSERIAL0_CONFIG_NEG_EDGE_SAMP_EN_POSN            = (int)9,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_NEG_EDGE_SAMP_EN_LSB_POSN = (int)9,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_NEG_EDGE_SAMP_EN_MSB_POSN = (int)9,
   BITSERIAL0_CONFIG_I2C_MODE_EN_POSN                 = (int)10,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_I2C_MODE_EN_LSB_POSN = (int)10,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_I2C_MODE_EN_MSB_POSN = (int)10,
   BITSERIAL0_CONFIG_POSEDGE_LAUNCH_MODE_EN_POSN      = (int)11,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_POSEDGE_LAUNCH_MODE_EN_LSB_POSN = (int)11,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_POSEDGE_LAUNCH_MODE_EN_MSB_POSN = (int)11,
   BITSERIAL0_CONFIG_STOP_TOKEN_DISABLE_POSN          = (int)12,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_STOP_TOKEN_DISABLE_LSB_POSN = (int)12,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_STOP_TOKEN_DISABLE_MSB_POSN = (int)12,
   BITSERIAL0_CONFIG_SEL_INVERT_POSN                  = (int)13,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_INVERT_LSB_POSN = (int)13,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_INVERT_MSB_POSN = (int)13,
   BITSERIAL0_CONFIG_SEL_EN_POSN                      = (int)14,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_EN_LSB_POSN = (int)14,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_EN_MSB_POSN = (int)14,
   BITSERIAL0_CONFIG_SEL_TIME_EN_POSN                 = (int)15,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_TIME_EN_LSB_POSN = (int)15,
   BITSERIAL0_CONFIG_BITSERIAL0_CONFIG_SEL_TIME_EN_MSB_POSN = (int)15
};
typedef enum bitserial0_config_posn_enum bitserial0_config_posn;

#define BITSERIAL0_CONFIG_CLK_INVERT_MASK        (0x00000001u)
#define BITSERIAL0_CONFIG_DIN_INVERT_MASK        (0x00000002u)
#define BITSERIAL0_CONFIG_DOUT_INVERT_MASK       (0x00000004u)
#define BITSERIAL0_CONFIG_BYTESWAP_EN_MASK       (0x00000008u)
#define BITSERIAL0_CONFIG_INT_EVENT_SUBSYSTEM_EN_MASK (0x00000010u)
#define BITSERIAL0_CONFIG_INT_EVENT_CURATOR_EN_MASK (0x00000020u)
#define BITSERIAL0_CONFIG_BITREVERSE_EN_MASK     (0x00000040u)
#define BITSERIAL0_CONFIG_CLEAR_STICKY_ACK_MASK  (0x00000080u)
#define BITSERIAL0_CONFIG_CLK_OFFSET_EN_MASK     (0x00000100u)
#define BITSERIAL0_CONFIG_NEG_EDGE_SAMP_EN_MASK  (0x00000200u)
#define BITSERIAL0_CONFIG_I2C_MODE_EN_MASK       (0x00000400u)
#define BITSERIAL0_CONFIG_POSEDGE_LAUNCH_MODE_EN_MASK (0x00000800u)
#define BITSERIAL0_CONFIG_STOP_TOKEN_DISABLE_MASK (0x00001000u)
#define BITSERIAL0_CONFIG_SEL_INVERT_MASK        (0x00002000u)
#define BITSERIAL0_CONFIG_SEL_EN_MASK            (0x00004000u)
#define BITSERIAL0_CONFIG_SEL_TIME_EN_MASK       (0x00008000u)

enum bitserial0_config2_posn_enum
{
   BITSERIAL0_CONFIG2_COMBO_MODE_POSN                 = (int)0,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_COMBO_MODE_LSB_POSN = (int)0,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_COMBO_MODE_MSB_POSN = (int)0,
   BITSERIAL0_CONFIG2_STOP_TOKEN_DISABLE2_POSN        = (int)1,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_STOP_TOKEN_DISABLE2_LSB_POSN = (int)1,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_STOP_TOKEN_DISABLE2_MSB_POSN = (int)1,
   BITSERIAL0_CONFIG2_SEL_EN2_POSN                    = (int)2,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SEL_EN2_LSB_POSN = (int)2,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SEL_EN2_MSB_POSN = (int)2,
   BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LSB_POSN          = (int)3,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LSB_POSN = (int)3,
   BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_MSB_POSN          = (int)4,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_MSB_POSN = (int)4,
   BITSERIAL0_CONFIG2_SLAVE_MODE_POSN                 = (int)5,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_MODE_LSB_POSN = (int)5,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_MODE_MSB_POSN = (int)5,
   BITSERIAL0_CONFIG2_SLAVE_10BIT_ADDR_MODE_POSN      = (int)6,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_10BIT_ADDR_MODE_LSB_POSN = (int)6,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_10BIT_ADDR_MODE_MSB_POSN = (int)6,
   BITSERIAL0_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_POSN = (int)7,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_LSB_POSN = (int)7,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_MSB_POSN = (int)7,
   BITSERIAL0_CONFIG2_DATA_READY_WORD_DISABLE_POSN    = (int)8,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DATA_READY_WORD_DISABLE_LSB_POSN = (int)8,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DATA_READY_WORD_DISABLE_MSB_POSN = (int)8,
   BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_POSN     = (int)9,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_LSB_POSN = (int)9,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_MSB_POSN = (int)9,
   BITSERIAL0_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_POSN      = (int)10,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_LSB_POSN = (int)10,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MSB_POSN = (int)10,
   BITSERIAL0_CONFIG2_DISABLE_UNDERFLOW_INT_POSN      = (int)11,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DISABLE_UNDERFLOW_INT_LSB_POSN = (int)11,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DISABLE_UNDERFLOW_INT_MSB_POSN = (int)11,
   BITSERIAL0_CONFIG2_DEBUG_SEL_LSB_POSN              = (int)12,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DEBUG_SEL_LSB_POSN = (int)12,
   BITSERIAL0_CONFIG2_DEBUG_SEL_MSB_POSN              = (int)13,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_DEBUG_SEL_MSB_POSN = (int)13,
   BITSERIAL0_CONFIG2_FORCE_SEL_ACTIVE_POSN           = (int)14,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_FORCE_SEL_ACTIVE_LSB_POSN = (int)14,
   BITSERIAL0_CONFIG2_BITSERIAL0_CONFIG2_FORCE_SEL_ACTIVE_MSB_POSN = (int)14
};
typedef enum bitserial0_config2_posn_enum bitserial0_config2_posn;

#define BITSERIAL0_CONFIG2_COMBO_MODE_MASK       (0x00000001u)
#define BITSERIAL0_CONFIG2_STOP_TOKEN_DISABLE2_MASK (0x00000002u)
#define BITSERIAL0_CONFIG2_SEL_EN2_MASK          (0x00000004u)
#define BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LSB_MASK (0x00000008u)
#define BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_MSB_MASK (0x00000010u)
#define BITSERIAL0_CONFIG2_SLAVE_MODE_MASK       (0x00000020u)
#define BITSERIAL0_CONFIG2_SLAVE_10BIT_ADDR_MODE_MASK (0x00000040u)
#define BITSERIAL0_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_MASK (0x00000080u)
#define BITSERIAL0_CONFIG2_DATA_READY_WORD_DISABLE_MASK (0x00000100u)
#define BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_MASK (0x00000200u)
#define BITSERIAL0_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MASK (0x00000400u)
#define BITSERIAL0_CONFIG2_DISABLE_UNDERFLOW_INT_MASK (0x00000800u)
#define BITSERIAL0_CONFIG2_DEBUG_SEL_LSB_MASK    (0x00001000u)
#define BITSERIAL0_CONFIG2_DEBUG_SEL_MSB_MASK    (0x00002000u)
#define BITSERIAL0_CONFIG2_FORCE_SEL_ACTIVE_MASK (0x00004000u)

enum bitserial0_config2_dout_idle_sel_enum
{
   BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LOW     = (int)0x0,
   BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_HIGH    = (int)0x1,
   BITSERIAL0_CONFIG2_DOUT_IDLE_SEL_LAST    = (int)0x2,
   MAX_BITSERIAL0_CONFIG2_DOUT_IDLE_SEL     = (int)0x2
};
#define NUM_BITSERIAL0_CONFIG2_DOUT_IDLE_SEL (0x3)
typedef enum bitserial0_config2_dout_idle_sel_enum bitserial0_config2_dout_idle_sel;


enum bitserial0_config2_slave_read_mode_switch_enum
{
   BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_AUTO = (int)0x0,
   BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH_MANUAL = (int)0x1,
   MAX_BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH = (int)0x1
};
#define NUM_BITSERIAL0_CONFIG2_SLAVE_READ_MODE_SWITCH (0x2)
typedef enum bitserial0_config2_slave_read_mode_switch_enum bitserial0_config2_slave_read_mode_switch;


enum bitserial0_config2_enum
{
   BITSERIAL0_CONFIG2_DEBUG_SEL_MASTER      = (int)0x0,
   BITSERIAL0_CONFIG2_DEBUG_SEL_SLAVE       = (int)0x1,
   BITSERIAL0_CONFIG2_DEBUG_SEL_VML_REQUEST = (int)0x2,
   MAX_BITSERIAL0_CONFIG2                   = (int)0x2
};
#define NUM_BITSERIAL0_CONFIG2 (0x3)
typedef enum bitserial0_config2_enum bitserial0_config2;


enum bitserial0_config3_posn_enum
{
   BITSERIAL0_CONFIG3_ACT_ON_NAK_LSB_POSN             = (int)0,
   BITSERIAL0_CONFIG3_BITSERIAL0_CONFIG3_ACT_ON_NAK_LSB_POSN = (int)0,
   BITSERIAL0_CONFIG3_ACT_ON_NAK_MSB_POSN             = (int)1,
   BITSERIAL0_CONFIG3_BITSERIAL0_CONFIG3_ACT_ON_NAK_MSB_POSN = (int)1,
   BITSERIAL0_CLR_SLAVE_READ_FIFO_POSN                = (int)2,
   BITSERIAL0_CONFIG3_BITSERIAL0_CLR_SLAVE_READ_FIFO_LSB_POSN = (int)2,
   BITSERIAL0_CONFIG3_BITSERIAL0_CLR_SLAVE_READ_FIFO_MSB_POSN = (int)2,
   BITSERIAL0_CLR_SLAVE_WRITE_FIFO_POSN               = (int)3,
   BITSERIAL0_CONFIG3_BITSERIAL0_CLR_SLAVE_WRITE_FIFO_LSB_POSN = (int)3,
   BITSERIAL0_CONFIG3_BITSERIAL0_CLR_SLAVE_WRITE_FIFO_MSB_POSN = (int)3,
   BITSERIAL0_STREAMING_EN_POSN                       = (int)4,
   BITSERIAL0_CONFIG3_BITSERIAL0_STREAMING_EN_LSB_POSN = (int)4,
   BITSERIAL0_CONFIG3_BITSERIAL0_STREAMING_EN_MSB_POSN = (int)4
};
typedef enum bitserial0_config3_posn_enum bitserial0_config3_posn;

#define BITSERIAL0_CONFIG3_ACT_ON_NAK_LSB_MASK   (0x00000001u)
#define BITSERIAL0_CONFIG3_ACT_ON_NAK_MSB_MASK   (0x00000002u)
#define BITSERIAL0_CLR_SLAVE_READ_FIFO_MASK      (0x00000004u)
#define BITSERIAL0_CLR_SLAVE_WRITE_FIFO_MASK     (0x00000008u)
#define BITSERIAL0_STREAMING_EN_MASK             (0x00000010u)

enum bitserial0_config3_enum
{
   BITSERIAL0_CONFIG3_ACT_ON_NAK_LEGACY     = (int)0x0,
   BITSERIAL0_CONFIG3_ACT_ON_NAK_STOP       = (int)0x1,
   BITSERIAL0_CONFIG3_ACT_ON_NAK_RESTART    = (int)0x2,
   MAX_BITSERIAL0_CONFIG3                   = (int)0x2
};
#define NUM_BITSERIAL0_CONFIG3 (0x3)
typedef enum bitserial0_config3_enum bitserial0_config3;


enum bitserial0_debug_sel_posn_enum
{
   BITSERIAL0_DEBUG_SEL_SLAVE_MEM_CORE_POSN           = (int)0,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_MEM_CORE_LSB_POSN = (int)0,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_MEM_CORE_MSB_POSN = (int)0,
   BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_LSB_POSN       = (int)1,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_LSB_POSN = (int)1,
   BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_MSB_POSN       = (int)4,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_MSB_POSN = (int)4,
   BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_LSB_POSN        = (int)5,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_LSB_POSN = (int)5,
   BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_MSB_POSN        = (int)8,
   BITSERIAL0_DEBUG_SEL_BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_MSB_POSN = (int)8
};
typedef enum bitserial0_debug_sel_posn_enum bitserial0_debug_sel_posn;

#define BITSERIAL0_DEBUG_SEL_SLAVE_MEM_CORE_MASK (0x00000001u)
#define BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_LSB_MASK (0x00000002u)
#define BITSERIAL0_DEBUG_SEL_SLAVE_CORE_SEL_MSB_MASK (0x00000010u)
#define BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_LSB_MASK (0x00000020u)
#define BITSERIAL0_DEBUG_SEL_SLAVE_MEM_SEL_MSB_MASK (0x00000100u)

enum bitserial0_deglitch_en_posn_enum
{
   BITSERIAL0_DEGLITCH_EN_CLOCK_POSN                  = (int)0,
   BITSERIAL0_DEGLITCH_EN_BITSERIAL0_DEGLITCH_EN_CLOCK_LSB_POSN = (int)0,
   BITSERIAL0_DEGLITCH_EN_BITSERIAL0_DEGLITCH_EN_CLOCK_MSB_POSN = (int)0,
   BITSERIAL0_DEGLITCH_EN_DATA_POSN                   = (int)1,
   BITSERIAL0_DEGLITCH_EN_BITSERIAL0_DEGLITCH_EN_DATA_LSB_POSN = (int)1,
   BITSERIAL0_DEGLITCH_EN_BITSERIAL0_DEGLITCH_EN_DATA_MSB_POSN = (int)1
};
typedef enum bitserial0_deglitch_en_posn_enum bitserial0_deglitch_en_posn;

#define BITSERIAL0_DEGLITCH_EN_CLOCK_MASK        (0x00000001u)
#define BITSERIAL0_DEGLITCH_EN_DATA_MASK         (0x00000002u)

enum bitserial0_error_status_posn_enum
{
   BITSERIAL0_VML_RESP_STATUS_LSB_POSN                = (int)0,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_VML_RESP_STATUS_LSB_POSN = (int)0,
   BITSERIAL0_VML_RESP_STATUS_MSB_POSN                = (int)3,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_VML_RESP_STATUS_MSB_POSN = (int)3,
   BITSERIAL0_READ_RESP_STATUS_LSB_POSN               = (int)4,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_READ_RESP_STATUS_LSB_POSN = (int)4,
   BITSERIAL0_READ_RESP_STATUS_MSB_POSN               = (int)7,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_READ_RESP_STATUS_MSB_POSN = (int)7,
   BITSERIAL0_TRANSACTION_PART2_POSN                  = (int)8,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_TRANSACTION_PART2_LSB_POSN = (int)8,
   BITSERIAL0_ERROR_STATUS_BITSERIAL0_TRANSACTION_PART2_MSB_POSN = (int)8
};
typedef enum bitserial0_error_status_posn_enum bitserial0_error_status_posn;

#define BITSERIAL0_VML_RESP_STATUS_LSB_MASK      (0x00000001u)
#define BITSERIAL0_VML_RESP_STATUS_MSB_MASK      (0x00000008u)
#define BITSERIAL0_READ_RESP_STATUS_LSB_MASK     (0x00000010u)
#define BITSERIAL0_READ_RESP_STATUS_MSB_MASK     (0x00000080u)
#define BITSERIAL0_TRANSACTION_PART2_MASK        (0x00000100u)

enum bitserial0_i2c_acks_posn_enum
{
   BITSERIAL0_I2C_ACKS_HISTORY_LSB_POSN               = (int)0,
   BITSERIAL0_I2C_ACKS_BITSERIAL0_I2C_ACKS_HISTORY_LSB_POSN = (int)0,
   BITSERIAL0_I2C_ACKS_HISTORY_MSB_POSN               = (int)14,
   BITSERIAL0_I2C_ACKS_BITSERIAL0_I2C_ACKS_HISTORY_MSB_POSN = (int)14,
   BITSERIAL0_I2C_ACKS_STICKY_POSN                    = (int)15,
   BITSERIAL0_I2C_ACKS_BITSERIAL0_I2C_ACKS_STICKY_LSB_POSN = (int)15,
   BITSERIAL0_I2C_ACKS_BITSERIAL0_I2C_ACKS_STICKY_MSB_POSN = (int)15
};
typedef enum bitserial0_i2c_acks_posn_enum bitserial0_i2c_acks_posn;

#define BITSERIAL0_I2C_ACKS_HISTORY_LSB_MASK     (0x00000001u)
#define BITSERIAL0_I2C_ACKS_HISTORY_MSB_MASK     (0x00004000u)
#define BITSERIAL0_I2C_ACKS_STICKY_MASK          (0x00008000u)

enum bitserial0_interbyte_spacing_posn_enum
{
   BITSERIAL0_INTERBYTE_SPACING_CYCLES_LSB_POSN       = (int)0,
   BITSERIAL0_INTERBYTE_SPACING_BITSERIAL0_INTERBYTE_SPACING_CYCLES_LSB_POSN = (int)0,
   BITSERIAL0_INTERBYTE_SPACING_CYCLES_MSB_POSN       = (int)14,
   BITSERIAL0_INTERBYTE_SPACING_BITSERIAL0_INTERBYTE_SPACING_CYCLES_MSB_POSN = (int)14,
   BITSERIAL0_INTERBYTE_SPACING_EN_POSN               = (int)15,
   BITSERIAL0_INTERBYTE_SPACING_BITSERIAL0_INTERBYTE_SPACING_EN_LSB_POSN = (int)15,
   BITSERIAL0_INTERBYTE_SPACING_BITSERIAL0_INTERBYTE_SPACING_EN_MSB_POSN = (int)15
};
typedef enum bitserial0_interbyte_spacing_posn_enum bitserial0_interbyte_spacing_posn;

#define BITSERIAL0_INTERBYTE_SPACING_CYCLES_LSB_MASK (0x00000001u)
#define BITSERIAL0_INTERBYTE_SPACING_CYCLES_MSB_MASK (0x00004000u)
#define BITSERIAL0_INTERBYTE_SPACING_EN_MASK     (0x00008000u)

enum bitserial0_status_posn_enum
{
   BITSERIAL0_BUSY_POSN                               = (int)0,
   BITSERIAL0_STATUS_BITSERIAL0_BUSY_LSB_POSN         = (int)0,
   BITSERIAL0_STATUS_BITSERIAL0_BUSY_MSB_POSN         = (int)0,
   BITSERIAL0_RWB_POSN                                = (int)1,
   BITSERIAL0_STATUS_BITSERIAL0_RWB_LSB_POSN          = (int)1,
   BITSERIAL0_STATUS_BITSERIAL0_RWB_MSB_POSN          = (int)1,
   BITSERIAL0_EVENT_POSN                              = (int)2,
   BITSERIAL0_STATUS_BITSERIAL0_EVENT_LSB_POSN        = (int)2,
   BITSERIAL0_STATUS_BITSERIAL0_EVENT_MSB_POSN        = (int)2,
   BITSERIAL0_FAILED_POSN                             = (int)3,
   BITSERIAL0_STATUS_BITSERIAL0_FAILED_LSB_POSN       = (int)3,
   BITSERIAL0_STATUS_BITSERIAL0_FAILED_MSB_POSN       = (int)3,
   BITSERIAL0_I2C_LOST_ARBITRATION_POSN               = (int)4,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_LOST_ARBITRATION_LSB_POSN = (int)4,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_LOST_ARBITRATION_MSB_POSN = (int)4,
   BITSERIAL0_I2C_BUS_BUSY_POSN                       = (int)5,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_BUS_BUSY_LSB_POSN = (int)5,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_BUS_BUSY_MSB_POSN = (int)5,
   BITSERIAL0_PART_2_POSN                             = (int)6,
   BITSERIAL0_STATUS_BITSERIAL0_PART_2_LSB_POSN       = (int)6,
   BITSERIAL0_STATUS_BITSERIAL0_PART_2_MSB_POSN       = (int)6,
   BITSERIAL0_FIFO_OVERFLOW_POSN                      = (int)7,
   BITSERIAL0_STATUS_BITSERIAL0_FIFO_OVERFLOW_LSB_POSN = (int)7,
   BITSERIAL0_STATUS_BITSERIAL0_FIFO_OVERFLOW_MSB_POSN = (int)7,
   BITSERIAL0_FIFO_UNDERFLOW_POSN                     = (int)8,
   BITSERIAL0_STATUS_BITSERIAL0_FIFO_UNDERFLOW_LSB_POSN = (int)8,
   BITSERIAL0_STATUS_BITSERIAL0_FIFO_UNDERFLOW_MSB_POSN = (int)8,
   BITSERIAL0_SLAVE_START_STOP_BITS_ERROR_POSN        = (int)9,
   BITSERIAL0_STATUS_BITSERIAL0_SLAVE_START_STOP_BITS_ERROR_LSB_POSN = (int)9,
   BITSERIAL0_STATUS_BITSERIAL0_SLAVE_START_STOP_BITS_ERROR_MSB_POSN = (int)9,
   BITSERIAL0_I2C_NACK_POSN                           = (int)10,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_NACK_LSB_POSN     = (int)10,
   BITSERIAL0_STATUS_BITSERIAL0_I2C_NACK_MSB_POSN     = (int)10,
   BITSERIAL0_DEEP_SLEEP_CDC_BUSY_POSN                = (int)11,
   BITSERIAL0_STATUS_BITSERIAL0_DEEP_SLEEP_CDC_BUSY_LSB_POSN = (int)11,
   BITSERIAL0_STATUS_BITSERIAL0_DEEP_SLEEP_CDC_BUSY_MSB_POSN = (int)11,
   BITSERIAL0_TX_NOT_CONFIGURED_POSN                  = (int)12,
   BITSERIAL0_STATUS_BITSERIAL0_TX_NOT_CONFIGURED_LSB_POSN = (int)12,
   BITSERIAL0_STATUS_BITSERIAL0_TX_NOT_CONFIGURED_MSB_POSN = (int)12
};
typedef enum bitserial0_status_posn_enum bitserial0_status_posn;

#define BITSERIAL0_BUSY_MASK                     (0x00000001u)
#define BITSERIAL0_RWB_MASK                      (0x00000002u)
#define BITSERIAL0_EVENT_MASK                    (0x00000004u)
#define BITSERIAL0_FAILED_MASK                   (0x00000008u)
#define BITSERIAL0_I2C_LOST_ARBITRATION_MASK     (0x00000010u)
#define BITSERIAL0_I2C_BUS_BUSY_MASK             (0x00000020u)
#define BITSERIAL0_PART_2_MASK                   (0x00000040u)
#define BITSERIAL0_FIFO_OVERFLOW_MASK            (0x00000080u)
#define BITSERIAL0_FIFO_UNDERFLOW_MASK           (0x00000100u)
#define BITSERIAL0_SLAVE_START_STOP_BITS_ERROR_MASK (0x00000200u)
#define BITSERIAL0_I2C_NACK_MASK                 (0x00000400u)
#define BITSERIAL0_DEEP_SLEEP_CDC_BUSY_MASK      (0x00000800u)
#define BITSERIAL0_TX_NOT_CONFIGURED_MASK        (0x00001000u)

enum bitserial0_word_config_posn_enum
{
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_LSB_POSN          = (int)0,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_BYTES_LSB_POSN = (int)0,
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_MSB_POSN          = (int)1,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_BYTES_MSB_POSN = (int)1,
   BITSERIAL0_WORD_CONFIG_NUM_STOP_LSB_POSN           = (int)2,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_STOP_LSB_POSN = (int)2,
   BITSERIAL0_WORD_CONFIG_NUM_STOP_MSB_POSN           = (int)3,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_STOP_MSB_POSN = (int)3,
   BITSERIAL0_WORD_CONFIG_NUM_START_LSB_POSN          = (int)4,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_START_LSB_POSN = (int)4,
   BITSERIAL0_WORD_CONFIG_NUM_START_MSB_POSN          = (int)5,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_NUM_START_MSB_POSN = (int)5,
   BITSERIAL0_WORD_CONFIG_START_BITS_LSB_POSN         = (int)6,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_START_BITS_LSB_POSN = (int)6,
   BITSERIAL0_WORD_CONFIG_START_BITS_MSB_POSN         = (int)8,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_START_BITS_MSB_POSN = (int)8,
   BITSERIAL0_WORD_CONFIG_STOP_BITS_LSB_POSN          = (int)9,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_STOP_BITS_LSB_POSN = (int)9,
   BITSERIAL0_WORD_CONFIG_STOP_BITS_MSB_POSN          = (int)11,
   BITSERIAL0_WORD_CONFIG_BITSERIAL0_WORD_CONFIG_STOP_BITS_MSB_POSN = (int)11
};
typedef enum bitserial0_word_config_posn_enum bitserial0_word_config_posn;

#define BITSERIAL0_WORD_CONFIG_NUM_BYTES_LSB_MASK (0x00000001u)
#define BITSERIAL0_WORD_CONFIG_NUM_BYTES_MSB_MASK (0x00000002u)
#define BITSERIAL0_WORD_CONFIG_NUM_STOP_LSB_MASK (0x00000004u)
#define BITSERIAL0_WORD_CONFIG_NUM_STOP_MSB_MASK (0x00000008u)
#define BITSERIAL0_WORD_CONFIG_NUM_START_LSB_MASK (0x00000010u)
#define BITSERIAL0_WORD_CONFIG_NUM_START_MSB_MASK (0x00000020u)
#define BITSERIAL0_WORD_CONFIG_START_BITS_LSB_MASK (0x00000040u)
#define BITSERIAL0_WORD_CONFIG_START_BITS_MSB_MASK (0x00000100u)
#define BITSERIAL0_WORD_CONFIG_STOP_BITS_LSB_MASK (0x00000200u)
#define BITSERIAL0_WORD_CONFIG_STOP_BITS_MSB_MASK (0x00000800u)

enum bitserial0_word_config_enum
{
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_ONE     = (int)0x0,
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_TWO     = (int)0x1,
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_THREE   = (int)0x2,
   BITSERIAL0_WORD_CONFIG_NUM_BYTES_FOUR    = (int)0x3,
   MAX_BITSERIAL0_WORD_CONFIG               = (int)0x3
};
#define NUM_BITSERIAL0_WORD_CONFIG (0x4)
typedef enum bitserial0_word_config_enum bitserial0_word_config;


enum bitserial0_clk_rate_hi_posn_enum
{
   BITSERIAL0_CLK_RATE_HI_LSB_POSN                    = (int)0,
   BITSERIAL0_CLK_RATE_HI_BITSERIAL0_CLK_RATE_HI_LSB_POSN = (int)0,
   BITSERIAL0_CLK_RATE_HI_MSB_POSN                    = (int)15,
   BITSERIAL0_CLK_RATE_HI_BITSERIAL0_CLK_RATE_HI_MSB_POSN = (int)15
};
typedef enum bitserial0_clk_rate_hi_posn_enum bitserial0_clk_rate_hi_posn;

#define BITSERIAL0_CLK_RATE_HI_LSB_MASK          (0x00000001u)
#define BITSERIAL0_CLK_RATE_HI_MSB_MASK          (0x00008000u)

enum bitserial0_clk_rate_lo_posn_enum
{
   BITSERIAL0_CLK_RATE_LO_LSB_POSN                    = (int)0,
   BITSERIAL0_CLK_RATE_LO_BITSERIAL0_CLK_RATE_LO_LSB_POSN = (int)0,
   BITSERIAL0_CLK_RATE_LO_MSB_POSN                    = (int)15,
   BITSERIAL0_CLK_RATE_LO_BITSERIAL0_CLK_RATE_LO_MSB_POSN = (int)15
};
typedef enum bitserial0_clk_rate_lo_posn_enum bitserial0_clk_rate_lo_posn;

#define BITSERIAL0_CLK_RATE_LO_LSB_MASK          (0x00000001u)
#define BITSERIAL0_CLK_RATE_LO_MSB_MASK          (0x00008000u)

enum bitserial0_clk_samp_offset_posn_enum
{
   BITSERIAL0_CLK_SAMP_OFFSET_LSB_POSN                = (int)0,
   BITSERIAL0_CLK_SAMP_OFFSET_BITSERIAL0_CLK_SAMP_OFFSET_LSB_POSN = (int)0,
   BITSERIAL0_CLK_SAMP_OFFSET_MSB_POSN                = (int)15,
   BITSERIAL0_CLK_SAMP_OFFSET_BITSERIAL0_CLK_SAMP_OFFSET_MSB_POSN = (int)15
};
typedef enum bitserial0_clk_samp_offset_posn_enum bitserial0_clk_samp_offset_posn;

#define BITSERIAL0_CLK_SAMP_OFFSET_LSB_MASK      (0x00000001u)
#define BITSERIAL0_CLK_SAMP_OFFSET_MSB_MASK      (0x00008000u)

enum bitserial0_config_sel_time_posn_enum
{
   BITSERIAL0_CONFIG_SEL_TIME_LSB_POSN                = (int)0,
   BITSERIAL0_CONFIG_SEL_TIME_BITSERIAL0_CONFIG_SEL_TIME_LSB_POSN = (int)0,
   BITSERIAL0_CONFIG_SEL_TIME_MSB_POSN                = (int)15,
   BITSERIAL0_CONFIG_SEL_TIME_BITSERIAL0_CONFIG_SEL_TIME_MSB_POSN = (int)15
};
typedef enum bitserial0_config_sel_time_posn_enum bitserial0_config_sel_time_posn;

#define BITSERIAL0_CONFIG_SEL_TIME_LSB_MASK      (0x00000001u)
#define BITSERIAL0_CONFIG_SEL_TIME_MSB_MASK      (0x00008000u)

enum bitserial0_config_sel_time2_posn_enum
{
   BITSERIAL0_CONFIG_SEL_TIME2_LSB_POSN               = (int)0,
   BITSERIAL0_CONFIG_SEL_TIME2_BITSERIAL0_CONFIG_SEL_TIME2_LSB_POSN = (int)0,
   BITSERIAL0_CONFIG_SEL_TIME2_MSB_POSN               = (int)3,
   BITSERIAL0_CONFIG_SEL_TIME2_BITSERIAL0_CONFIG_SEL_TIME2_MSB_POSN = (int)3
};
typedef enum bitserial0_config_sel_time2_posn_enum bitserial0_config_sel_time2_posn;

#define BITSERIAL0_CONFIG_SEL_TIME2_LSB_MASK     (0x00000001u)
#define BITSERIAL0_CONFIG_SEL_TIME2_MSB_MASK     (0x00000008u)

enum bitserial0_curator_ssid_posn_enum
{
   BITSERIAL0_CURATOR_SSID_LSB_POSN                   = (int)0,
   BITSERIAL0_CURATOR_SSID_BITSERIAL0_CURATOR_SSID_LSB_POSN = (int)0,
   BITSERIAL0_CURATOR_SSID_MSB_POSN                   = (int)3,
   BITSERIAL0_CURATOR_SSID_BITSERIAL0_CURATOR_SSID_MSB_POSN = (int)3
};
typedef enum bitserial0_curator_ssid_posn_enum bitserial0_curator_ssid_posn;

#define BITSERIAL0_CURATOR_SSID_LSB_MASK         (0x00000001u)
#define BITSERIAL0_CURATOR_SSID_MSB_MASK         (0x00000008u)

enum bitserial0_deep_sleep_active_posn_enum
{
   BITSERIAL0_DEEP_SLEEP_ACTIVE_POSN                  = (int)0,
   BITSERIAL0_DEEP_SLEEP_ACTIVE_BITSERIAL0_DEEP_SLEEP_ACTIVE_LSB_POSN = (int)0,
   BITSERIAL0_DEEP_SLEEP_ACTIVE_BITSERIAL0_DEEP_SLEEP_ACTIVE_MSB_POSN = (int)0
};
typedef enum bitserial0_deep_sleep_active_posn_enum bitserial0_deep_sleep_active_posn;

#define BITSERIAL0_DEEP_SLEEP_ACTIVE_MASK        (0x00000001u)

enum bitserial0_i2c_address_posn_enum
{
   BITSERIAL0_I2C_ADDRESS_LSB_POSN                    = (int)0,
   BITSERIAL0_I2C_ADDRESS_BITSERIAL0_I2C_ADDRESS_LSB_POSN = (int)0,
   BITSERIAL0_I2C_ADDRESS_MSB_POSN                    = (int)9,
   BITSERIAL0_I2C_ADDRESS_BITSERIAL0_I2C_ADDRESS_MSB_POSN = (int)9
};
typedef enum bitserial0_i2c_address_posn_enum bitserial0_i2c_address_posn;

#define BITSERIAL0_I2C_ADDRESS_LSB_MASK          (0x00000001u)
#define BITSERIAL0_I2C_ADDRESS_MSB_MASK          (0x00000200u)

enum bitserial0_interrupt_clear_posn_enum
{
   BITSERIAL0_INTERRUPT_CLEAR_LSB_POSN                = (int)0,
   BITSERIAL0_INTERRUPT_CLEAR_BITSERIAL0_INTERRUPT_CLEAR_LSB_POSN = (int)0,
   BITSERIAL0_INTERRUPT_CLEAR_MSB_POSN                = (int)13,
   BITSERIAL0_INTERRUPT_CLEAR_BITSERIAL0_INTERRUPT_CLEAR_MSB_POSN = (int)13
};
typedef enum bitserial0_interrupt_clear_posn_enum bitserial0_interrupt_clear_posn;

#define BITSERIAL0_INTERRUPT_CLEAR_LSB_MASK      (0x00000001u)
#define BITSERIAL0_INTERRUPT_CLEAR_MSB_MASK      (0x00002000u)

enum bitserial0_interrupt_en_posn_enum
{
   BITSERIAL0_INTERRUPT_EN_LSB_POSN                   = (int)0,
   BITSERIAL0_INTERRUPT_EN_BITSERIAL0_INTERRUPT_EN_LSB_POSN = (int)0,
   BITSERIAL0_INTERRUPT_EN_MSB_POSN                   = (int)13,
   BITSERIAL0_INTERRUPT_EN_BITSERIAL0_INTERRUPT_EN_MSB_POSN = (int)13
};
typedef enum bitserial0_interrupt_en_posn_enum bitserial0_interrupt_en_posn;

#define BITSERIAL0_INTERRUPT_EN_LSB_MASK         (0x00000001u)
#define BITSERIAL0_INTERRUPT_EN_MSB_MASK         (0x00002000u)

enum bitserial0_interrupt_source_posn_enum
{
   BITSERIAL0_INTERRUPT_SOURCE_LSB_POSN               = (int)0,
   BITSERIAL0_INTERRUPT_SOURCE_BITSERIAL0_INTERRUPT_SOURCE_LSB_POSN = (int)0,
   BITSERIAL0_INTERRUPT_SOURCE_MSB_POSN               = (int)13,
   BITSERIAL0_INTERRUPT_SOURCE_BITSERIAL0_INTERRUPT_SOURCE_MSB_POSN = (int)13
};
typedef enum bitserial0_interrupt_source_posn_enum bitserial0_interrupt_source_posn;

#define BITSERIAL0_INTERRUPT_SOURCE_LSB_MASK     (0x00000001u)
#define BITSERIAL0_INTERRUPT_SOURCE_MSB_MASK     (0x00002000u)

enum bitserial0_interrupt_source_clear_posn_enum
{
   BITSERIAL0_INTERRUPT_SOURCE_CLEAR_LSB_POSN         = (int)0,
   BITSERIAL0_INTERRUPT_SOURCE_CLEAR_BITSERIAL0_INTERRUPT_SOURCE_CLEAR_LSB_POSN = (int)0,
   BITSERIAL0_INTERRUPT_SOURCE_CLEAR_MSB_POSN         = (int)13,
   BITSERIAL0_INTERRUPT_SOURCE_CLEAR_BITSERIAL0_INTERRUPT_SOURCE_CLEAR_MSB_POSN = (int)13
};
typedef enum bitserial0_interrupt_source_clear_posn_enum bitserial0_interrupt_source_clear_posn;

#define BITSERIAL0_INTERRUPT_SOURCE_CLEAR_LSB_MASK (0x00000001u)
#define BITSERIAL0_INTERRUPT_SOURCE_CLEAR_MSB_MASK (0x00002000u)

enum bitserial0_interrupt_status_posn_enum
{
   BITSERIAL0_INTERRUPT_STATUS_LSB_POSN               = (int)0,
   BITSERIAL0_INTERRUPT_STATUS_BITSERIAL0_INTERRUPT_STATUS_LSB_POSN = (int)0,
   BITSERIAL0_INTERRUPT_STATUS_MSB_POSN               = (int)13,
   BITSERIAL0_INTERRUPT_STATUS_BITSERIAL0_INTERRUPT_STATUS_MSB_POSN = (int)13
};
typedef enum bitserial0_interrupt_status_posn_enum bitserial0_interrupt_status_posn;

#define BITSERIAL0_INTERRUPT_STATUS_LSB_MASK     (0x00000001u)
#define BITSERIAL0_INTERRUPT_STATUS_MSB_MASK     (0x00002000u)

enum bitserial0_remote_subsystem_ssid_posn_enum
{
   BITSERIAL0_REMOTE_SUBSYSTEM_SSID_LSB_POSN          = (int)0,
   BITSERIAL0_REMOTE_SUBSYSTEM_SSID_BITSERIAL0_REMOTE_SUBSYSTEM_SSID_LSB_POSN = (int)0,
   BITSERIAL0_REMOTE_SUBSYSTEM_SSID_MSB_POSN          = (int)3,
   BITSERIAL0_REMOTE_SUBSYSTEM_SSID_BITSERIAL0_REMOTE_SUBSYSTEM_SSID_MSB_POSN = (int)3
};
typedef enum bitserial0_remote_subsystem_ssid_posn_enum bitserial0_remote_subsystem_ssid_posn;

#define BITSERIAL0_REMOTE_SUBSYSTEM_SSID_LSB_MASK (0x00000001u)
#define BITSERIAL0_REMOTE_SUBSYSTEM_SSID_MSB_MASK (0x00000008u)

enum bitserial0_set_sel_inactive_posn_enum
{
   BITSERIAL0_SET_SEL_INACTIVE_POSN                   = (int)0,
   BITSERIAL0_SET_SEL_INACTIVE_BITSERIAL0_SET_SEL_INACTIVE_LSB_POSN = (int)0,
   BITSERIAL0_SET_SEL_INACTIVE_BITSERIAL0_SET_SEL_INACTIVE_MSB_POSN = (int)0
};
typedef enum bitserial0_set_sel_inactive_posn_enum bitserial0_set_sel_inactive_posn;

#define BITSERIAL0_SET_SEL_INACTIVE_MASK         (0x00000001u)

enum bitserial0_slave_data_ready_byte_posn_enum
{
   BITSERIAL0_SLAVE_DATA_READY_BYTE_LSB_POSN          = (int)0,
   BITSERIAL0_SLAVE_DATA_READY_BYTE_BITSERIAL0_SLAVE_DATA_READY_BYTE_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_DATA_READY_BYTE_MSB_POSN          = (int)7,
   BITSERIAL0_SLAVE_DATA_READY_BYTE_BITSERIAL0_SLAVE_DATA_READY_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial0_slave_data_ready_byte_posn_enum bitserial0_slave_data_ready_byte_posn;

#define BITSERIAL0_SLAVE_DATA_READY_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL0_SLAVE_DATA_READY_BYTE_MSB_MASK (0x00000080u)

enum bitserial0_slave_num_protocol_words_posn_enum
{
   BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_LSB_POSN       = (int)0,
   BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_MSB_POSN       = (int)15,
   BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_MSB_POSN = (int)15
};
typedef enum bitserial0_slave_num_protocol_words_posn_enum bitserial0_slave_num_protocol_words_posn;

#define BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_LSB_MASK (0x00000001u)
#define BITSERIAL0_SLAVE_NUM_PROTOCOL_WORDS_MSB_MASK (0x00008000u)

enum bitserial0_slave_read_command_byte_posn_enum
{
   BITSERIAL0_SLAVE_READ_COMMAND_BYTE_LSB_POSN        = (int)0,
   BITSERIAL0_SLAVE_READ_COMMAND_BYTE_BITSERIAL0_SLAVE_READ_COMMAND_BYTE_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_READ_COMMAND_BYTE_MSB_POSN        = (int)7,
   BITSERIAL0_SLAVE_READ_COMMAND_BYTE_BITSERIAL0_SLAVE_READ_COMMAND_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial0_slave_read_command_byte_posn_enum bitserial0_slave_read_command_byte_posn;

#define BITSERIAL0_SLAVE_READ_COMMAND_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL0_SLAVE_READ_COMMAND_BYTE_MSB_MASK (0x00000080u)

enum bitserial0_slave_read_count_posn_enum
{
   BITSERIAL0_SLAVE_READ_COUNT_LSB_POSN               = (int)0,
   BITSERIAL0_SLAVE_READ_COUNT_BITSERIAL0_SLAVE_READ_COUNT_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_READ_COUNT_MSB_POSN               = (int)15,
   BITSERIAL0_SLAVE_READ_COUNT_BITSERIAL0_SLAVE_READ_COUNT_MSB_POSN = (int)15
};
typedef enum bitserial0_slave_read_count_posn_enum bitserial0_slave_read_count_posn;

#define BITSERIAL0_SLAVE_READ_COUNT_LSB_MASK     (0x00000001u)
#define BITSERIAL0_SLAVE_READ_COUNT_MSB_MASK     (0x00008000u)

enum bitserial0_slave_underflow_byte_posn_enum
{
   BITSERIAL0_SLAVE_UNDERFLOW_BYTE_LSB_POSN           = (int)0,
   BITSERIAL0_SLAVE_UNDERFLOW_BYTE_BITSERIAL0_SLAVE_UNDERFLOW_BYTE_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_UNDERFLOW_BYTE_MSB_POSN           = (int)7,
   BITSERIAL0_SLAVE_UNDERFLOW_BYTE_BITSERIAL0_SLAVE_UNDERFLOW_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial0_slave_underflow_byte_posn_enum bitserial0_slave_underflow_byte_posn;

#define BITSERIAL0_SLAVE_UNDERFLOW_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL0_SLAVE_UNDERFLOW_BYTE_MSB_MASK (0x00000080u)

enum bitserial0_slave_write_count_posn_enum
{
   BITSERIAL0_SLAVE_WRITE_COUNT_LSB_POSN              = (int)0,
   BITSERIAL0_SLAVE_WRITE_COUNT_BITSERIAL0_SLAVE_WRITE_COUNT_LSB_POSN = (int)0,
   BITSERIAL0_SLAVE_WRITE_COUNT_MSB_POSN              = (int)15,
   BITSERIAL0_SLAVE_WRITE_COUNT_BITSERIAL0_SLAVE_WRITE_COUNT_MSB_POSN = (int)15
};
typedef enum bitserial0_slave_write_count_posn_enum bitserial0_slave_write_count_posn;

#define BITSERIAL0_SLAVE_WRITE_COUNT_LSB_MASK    (0x00000001u)
#define BITSERIAL0_SLAVE_WRITE_COUNT_MSB_MASK    (0x00008000u)

enum bitserial0_streaming_threshold_posn_enum
{
   BITSERIAL0_STREAMING_THRESHOLD_LSB_POSN            = (int)0,
   BITSERIAL0_STREAMING_THRESHOLD_BITSERIAL0_STREAMING_THRESHOLD_LSB_POSN = (int)0,
   BITSERIAL0_STREAMING_THRESHOLD_MSB_POSN            = (int)15,
   BITSERIAL0_STREAMING_THRESHOLD_BITSERIAL0_STREAMING_THRESHOLD_MSB_POSN = (int)15
};
typedef enum bitserial0_streaming_threshold_posn_enum bitserial0_streaming_threshold_posn;

#define BITSERIAL0_STREAMING_THRESHOLD_LSB_MASK  (0x00000001u)
#define BITSERIAL0_STREAMING_THRESHOLD_MSB_MASK  (0x00008000u)

enum bitserial0_txrx_length_posn_enum
{
   BITSERIAL0_TXRX_LENGTH_LSB_POSN                    = (int)0,
   BITSERIAL0_TXRX_LENGTH_BITSERIAL0_TXRX_LENGTH_LSB_POSN = (int)0,
   BITSERIAL0_TXRX_LENGTH_MSB_POSN                    = (int)15,
   BITSERIAL0_TXRX_LENGTH_BITSERIAL0_TXRX_LENGTH_MSB_POSN = (int)15
};
typedef enum bitserial0_txrx_length_posn_enum bitserial0_txrx_length_posn;

#define BITSERIAL0_TXRX_LENGTH_LSB_MASK          (0x00000001u)
#define BITSERIAL0_TXRX_LENGTH_MSB_MASK          (0x00008000u)

enum bitserial0_txrx_length2_posn_enum
{
   BITSERIAL0_TXRX_LENGTH2_LSB_POSN                   = (int)0,
   BITSERIAL0_TXRX_LENGTH2_BITSERIAL0_TXRX_LENGTH2_LSB_POSN = (int)0,
   BITSERIAL0_TXRX_LENGTH2_MSB_POSN                   = (int)15,
   BITSERIAL0_TXRX_LENGTH2_BITSERIAL0_TXRX_LENGTH2_MSB_POSN = (int)15
};
typedef enum bitserial0_txrx_length2_posn_enum bitserial0_txrx_length2_posn;

#define BITSERIAL0_TXRX_LENGTH2_LSB_MASK         (0x00000001u)
#define BITSERIAL0_TXRX_LENGTH2_MSB_MASK         (0x00008000u)

enum bitserial0_words_sent_posn_enum
{
   BITSERIAL0_WORDS_SENT_LSB_POSN                     = (int)0,
   BITSERIAL0_WORDS_SENT_BITSERIAL0_WORDS_SENT_LSB_POSN = (int)0,
   BITSERIAL0_WORDS_SENT_MSB_POSN                     = (int)15,
   BITSERIAL0_WORDS_SENT_BITSERIAL0_WORDS_SENT_MSB_POSN = (int)15
};
typedef enum bitserial0_words_sent_posn_enum bitserial0_words_sent_posn;

#define BITSERIAL0_WORDS_SENT_LSB_MASK           (0x00000001u)
#define BITSERIAL0_WORDS_SENT_MSB_MASK           (0x00008000u)

enum bitserial0_rx_buffer_posn_enum
{
   BITSERIAL0_RX_BUFFER_BITSERIAL0_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL0_RX_BUFFER_BITSERIAL0_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL0_RX_BUFFER_BITSERIAL0_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL0_RX_BUFFER_BITSERIAL0_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial0_rx_buffer_posn_enum bitserial0_rx_buffer_posn;


enum bitserial0_rx_buffer2_posn_enum
{
   BITSERIAL0_RX_BUFFER2_BITSERIAL0_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL0_RX_BUFFER2_BITSERIAL0_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL0_RX_BUFFER2_BITSERIAL0_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL0_RX_BUFFER2_BITSERIAL0_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial0_rx_buffer2_posn_enum bitserial0_rx_buffer2_posn;


enum bitserial0_tx_buffer_posn_enum
{
   BITSERIAL0_TX_BUFFER_BITSERIAL0_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL0_TX_BUFFER_BITSERIAL0_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL0_TX_BUFFER_BITSERIAL0_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL0_TX_BUFFER_BITSERIAL0_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial0_tx_buffer_posn_enum bitserial0_tx_buffer_posn;


enum bitserial0_tx_buffer2_posn_enum
{
   BITSERIAL0_TX_BUFFER2_BITSERIAL0_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL0_TX_BUFFER2_BITSERIAL0_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL0_TX_BUFFER2_BITSERIAL0_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL0_TX_BUFFER2_BITSERIAL0_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial0_tx_buffer2_posn_enum bitserial0_tx_buffer2_posn;


#endif /* IO_DEFS_MODULE_BITSERIAL0 */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL0 */





#if defined(IO_DEFS_MODULE_SQIF) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_SQIF
#define __IO_DEFS_H__IO_DEFS_MODULE_SQIF

/* -- sqif -- Serial Quad I/O Flash registers -- */

enum sqif_commands_enum
{
   SQIF_WINBOND_FAST_READ                   = (int)0xB,
   SQIF_WINBOND_FAST_READ_DUAL_OUTPUT       = (int)0x3B,
   SQIF_WINBOND_FAST_READ_QUAD_OUTPUT       = (int)0x6B,
   SQIF_WINBOND_FAST_READ_DUAL_IO           = (int)0xBB,
   SQIF_WINBOND_FAST_READ_QUAD_IO           = (int)0xEB,
   SQIF_WINBOND_WORD_READ_QUAD_IO           = (int)0xE7,
   SQIF_WINBOND_HIGH_PERFORMANCE_MODE       = (int)0xA3,
   SQIF_WINBOND_WRITE_STATUS_REGISTER       = (int)0x1,
   SQIF_WINBOND_WRITE_ENABLE_VOLATILE       = (int)0x50,
   SQIF_WINBOND_READ_JEDEC_ID               = (int)0x9F,
   SQIF_WINBOND_CONTINUOUS_READ_MODE_RESET  = (int)0xFF,
   MAX_SQIF_COMMANDS                        = (int)0xFF
};
typedef enum sqif_commands_enum sqif_commands;


enum sqif_conf_posn_enum
{
   SQIF_CONF_WID_LSB_POSN                             = (int)0,
   SQIF_CONF_SQIF_CONF_WID_LSB_POSN                   = (int)0,
   SQIF_CONF_WID_MSB_POSN                             = (int)2,
   SQIF_CONF_SQIF_CONF_WID_MSB_POSN                   = (int)2,
   SQIF_CONF_CMD_POSN                                 = (int)3,
   SQIF_CONF_SQIF_CONF_CMD_LSB_POSN                   = (int)3,
   SQIF_CONF_SQIF_CONF_CMD_MSB_POSN                   = (int)3,
   SQIF_CONF_ADD_LSB_POSN                             = (int)4,
   SQIF_CONF_SQIF_CONF_ADD_LSB_POSN                   = (int)4,
   SQIF_CONF_ADD_MSB_POSN                             = (int)5,
   SQIF_CONF_SQIF_CONF_ADD_MSB_POSN                   = (int)5,
   SQIF_CONF_MOD_LSB_POSN                             = (int)6,
   SQIF_CONF_SQIF_CONF_MOD_LSB_POSN                   = (int)6,
   SQIF_CONF_MOD_MSB_POSN                             = (int)7,
   SQIF_CONF_SQIF_CONF_MOD_MSB_POSN                   = (int)7,
   SQIF_CONF_DUM_LSB_POSN                             = (int)8,
   SQIF_CONF_SQIF_CONF_DUM_LSB_POSN                   = (int)8,
   SQIF_CONF_DUM_MSB_POSN                             = (int)9,
   SQIF_CONF_SQIF_CONF_DUM_MSB_POSN                   = (int)9,
   SQIF_CONF_CS_HOLD_POSN                             = (int)10,
   SQIF_CONF_SQIF_CONF_CS_HOLD_LSB_POSN               = (int)10,
   SQIF_CONF_SQIF_CONF_CS_HOLD_MSB_POSN               = (int)10,
   SQIF_CONF_ABT_POSN                                 = (int)11,
   SQIF_CONF_SQIF_CONF_ABT_LSB_POSN                   = (int)11,
   SQIF_CONF_SQIF_CONF_ABT_MSB_POSN                   = (int)11
};
typedef enum sqif_conf_posn_enum sqif_conf_posn;

#define SQIF_CONF_WID_LSB_MASK                   (0x00000001u)
#define SQIF_CONF_WID_MSB_MASK                   (0x00000004u)
#define SQIF_CONF_CMD_MASK                       (0x00000008u)
#define SQIF_CONF_ADD_LSB_MASK                   (0x00000010u)
#define SQIF_CONF_ADD_MSB_MASK                   (0x00000020u)
#define SQIF_CONF_MOD_LSB_MASK                   (0x00000040u)
#define SQIF_CONF_MOD_MSB_MASK                   (0x00000080u)
#define SQIF_CONF_DUM_LSB_MASK                   (0x00000100u)
#define SQIF_CONF_DUM_MSB_MASK                   (0x00000200u)
#define SQIF_CONF_CS_HOLD_MASK                   (0x00000400u)
#define SQIF_CONF_ABT_MASK                       (0x00000800u)

enum sqif_conf_wid_enum
{
   SQIF_CONF_WID_C1A1D1                     = (int)0x0,
   SQIF_CONF_WID_C1A1D2                     = (int)0x1,
   SQIF_CONF_WID_C1A1D4                     = (int)0x2,
   SQIF_CONF_WID_C1A2D2                     = (int)0x3,
   SQIF_CONF_WID_C1A4D4                     = (int)0x4,
   SQIF_CONF_WID_C4A4D4                     = (int)0x5,
   SQIF_CONF_WID_C2A2D2                     = (int)0x6,
   SQIF_CONF_WID_RSVD2                      = (int)0x7,
   MAX_SQIF_CONF_WID                        = (int)0x7
};
#define NUM_SQIF_CONF_WID (0x8)
typedef enum sqif_conf_wid_enum sqif_conf_wid;


enum sqif_conf_cmd_enum
{
   SQIF_CONF_CMD_ZERO                       = (int)0x0,
   SQIF_CONF_CMD_ONE                        = (int)0x1,
   MAX_SQIF_CONF_CMD                        = (int)0x1
};
#define NUM_SQIF_CONF_CMD (0x2)
typedef enum sqif_conf_cmd_enum sqif_conf_cmd;


enum sqif_conf_add_enum
{
   SQIF_CONF_ADD_TWO                        = (int)0x0,
   SQIF_CONF_ADD_THREE                      = (int)0x1,
   SQIF_CONF_ADD_FOUR                       = (int)0x2,
   SQIF_CONF_ADD_RSV                        = (int)0x3,
   MAX_SQIF_CONF_ADD                        = (int)0x3
};
#define NUM_SQIF_CONF_ADD (0x4)
typedef enum sqif_conf_add_enum sqif_conf_add;


enum sqif_conf_mod_enum
{
   SQIF_CONF_MOD_ZERO                       = (int)0x0,
   SQIF_CONF_MOD_HALF                       = (int)0x1,
   SQIF_CONF_MOD_ONE                        = (int)0x2,
   SQIF_CONF_MOD_RSVD                       = (int)0x3,
   MAX_SQIF_CONF_MOD                        = (int)0x3
};
#define NUM_SQIF_CONF_MOD (0x4)
typedef enum sqif_conf_mod_enum sqif_conf_mod;


enum sqif_conf_dum_enum
{
   SQIF_CONF_DUM_ZERO                       = (int)0x0,
   SQIF_CONF_DUM_HALF                       = (int)0x1,
   SQIF_CONF_DUM_ONE                        = (int)0x2,
   SQIF_CONF_DUM_TWO                        = (int)0x3,
   MAX_SQIF_CONF_DUM                        = (int)0x3
};
#define NUM_SQIF_CONF_DUM (0x4)
typedef enum sqif_conf_dum_enum sqif_conf_dum;


enum sqif_conf_abt_enum
{
   SQIF_CONF_ABT_BRUTAL                     = (int)0x0,
   SQIF_CONF_ABT_GENTLE                     = (int)0x1,
   MAX_SQIF_CONF_ABT                        = (int)0x1
};
#define NUM_SQIF_CONF_ABT (0x2)
typedef enum sqif_conf_abt_enum sqif_conf_abt;


enum sqif_conf2_posn_enum
{
   SQIF_CONF2_RAM_EN_POSN                             = (int)0,
   SQIF_CONF2_SQIF_CONF2_RAM_EN_LSB_POSN              = (int)0,
   SQIF_CONF2_SQIF_CONF2_RAM_EN_MSB_POSN              = (int)0,
   SQIF_CONF2_GRAB_PADS_POSN                          = (int)1,
   SQIF_CONF2_SQIF_CONF2_GRAB_PADS_LSB_POSN           = (int)1,
   SQIF_CONF2_SQIF_CONF2_GRAB_PADS_MSB_POSN           = (int)1,
   SQIF_CONF2_PULL_WEAK_POSN                          = (int)2,
   SQIF_CONF2_SQIF_CONF2_PULL_WEAK_LSB_POSN           = (int)2,
   SQIF_CONF2_SQIF_CONF2_PULL_WEAK_MSB_POSN           = (int)2,
   SQIF_CONF2_SQIF_EN_POSN                            = (int)3,
   SQIF_CONF2_SQIF_CONF2_SQIF_EN_LSB_POSN             = (int)3,
   SQIF_CONF2_SQIF_CONF2_SQIF_EN_MSB_POSN             = (int)3,
   SQIF_CONF2_ENDIANNESS_POSN                         = (int)4,
   SQIF_CONF2_SQIF_CONF2_ENDIANNESS_LSB_POSN          = (int)4,
   SQIF_CONF2_SQIF_CONF2_ENDIANNESS_MSB_POSN          = (int)4,
   SQIF_CONF2_DDR_MODE_EN_POSN                        = (int)5,
   SQIF_CONF2_SQIF_CONF2_DDR_MODE_EN_LSB_POSN         = (int)5,
   SQIF_CONF2_SQIF_CONF2_DDR_MODE_EN_MSB_POSN         = (int)5,
   SQIF_CONF2_DUMMY_OVR_LSB_POSN                      = (int)6,
   SQIF_CONF2_SQIF_CONF2_DUMMY_OVR_LSB_POSN           = (int)6,
   SQIF_CONF2_DUMMY_OVR_MSB_POSN                      = (int)9,
   SQIF_CONF2_SQIF_CONF2_DUMMY_OVR_MSB_POSN           = (int)9,
   SQIF_CONF2_FORCE_DLP_CAPTURE_POSN                  = (int)10,
   SQIF_CONF2_SQIF_CONF2_FORCE_DLP_CAPTURE_LSB_POSN   = (int)10,
   SQIF_CONF2_SQIF_CONF2_FORCE_DLP_CAPTURE_MSB_POSN   = (int)10,
   SQIF_CONF2_SDR_DELAY_EN_POSN                       = (int)11,
   SQIF_CONF2_SQIF_CONF2_SDR_DELAY_EN_LSB_POSN        = (int)11,
   SQIF_CONF2_SQIF_CONF2_SDR_DELAY_EN_MSB_POSN        = (int)11,
   SQIF_CONF2_CLOCK_EN_POSN                           = (int)12,
   SQIF_CONF2_SQIF_CONF2_CLOCK_EN_LSB_POSN            = (int)12,
   SQIF_CONF2_SQIF_CONF2_CLOCK_EN_MSB_POSN            = (int)12
};
typedef enum sqif_conf2_posn_enum sqif_conf2_posn;

#define SQIF_CONF2_RAM_EN_MASK                   (0x00000001u)
#define SQIF_CONF2_GRAB_PADS_MASK                (0x00000002u)
#define SQIF_CONF2_PULL_WEAK_MASK                (0x00000004u)
#define SQIF_CONF2_SQIF_EN_MASK                  (0x00000008u)
#define SQIF_CONF2_ENDIANNESS_MASK               (0x00000010u)
#define SQIF_CONF2_DDR_MODE_EN_MASK              (0x00000020u)
#define SQIF_CONF2_DUMMY_OVR_LSB_MASK            (0x00000040u)
#define SQIF_CONF2_DUMMY_OVR_MSB_MASK            (0x00000200u)
#define SQIF_CONF2_FORCE_DLP_CAPTURE_MASK        (0x00000400u)
#define SQIF_CONF2_SDR_DELAY_EN_MASK             (0x00000800u)
#define SQIF_CONF2_CLOCK_EN_MASK                 (0x00001000u)

enum sqif_conf2_endianness_enum
{
   SQIF_CONF2_ENDIANNESS_BIG                = (int)0x0,
   SQIF_CONF2_ENDIANNESS_LITTLE             = (int)0x1,
   MAX_SQIF_CONF2_ENDIANNESS                = (int)0x1
};
#define NUM_SQIF_CONF2_ENDIANNESS (0x2)
typedef enum sqif_conf2_endianness_enum sqif_conf2_endianness;


enum sqif_corr_cfg_posn_enum
{
   SQIF_CORR_DLP_LSB_POSN                             = (int)0,
   SQIF_CORR_CFG_SQIF_CORR_DLP_LSB_POSN               = (int)0,
   SQIF_CORR_DLP_MSB_POSN                             = (int)7,
   SQIF_CORR_CFG_SQIF_CORR_DLP_MSB_POSN               = (int)7,
   SQIF_CORR_DLP_LENGTH_LSB_POSN                      = (int)8,
   SQIF_CORR_CFG_SQIF_CORR_DLP_LENGTH_LSB_POSN        = (int)8,
   SQIF_CORR_DLP_LENGTH_MSB_POSN                      = (int)10,
   SQIF_CORR_CFG_SQIF_CORR_DLP_LENGTH_MSB_POSN        = (int)10,
   SQIF_CORR_CONSEC_LSB_POSN                          = (int)11,
   SQIF_CORR_CFG_SQIF_CORR_CONSEC_LSB_POSN            = (int)11,
   SQIF_CORR_CONSEC_MSB_POSN                          = (int)12,
   SQIF_CORR_CFG_SQIF_CORR_CONSEC_MSB_POSN            = (int)12,
   SQIF_CORR_ENABLE_POSN                              = (int)13,
   SQIF_CORR_CFG_SQIF_CORR_ENABLE_LSB_POSN            = (int)13,
   SQIF_CORR_CFG_SQIF_CORR_ENABLE_MSB_POSN            = (int)13
};
typedef enum sqif_corr_cfg_posn_enum sqif_corr_cfg_posn;

#define SQIF_CORR_DLP_LSB_MASK                   (0x00000001u)
#define SQIF_CORR_DLP_MSB_MASK                   (0x00000080u)
#define SQIF_CORR_DLP_LENGTH_LSB_MASK            (0x00000100u)
#define SQIF_CORR_DLP_LENGTH_MSB_MASK            (0x00000400u)
#define SQIF_CORR_CONSEC_LSB_MASK                (0x00000800u)
#define SQIF_CORR_CONSEC_MSB_MASK                (0x00001000u)
#define SQIF_CORR_ENABLE_MASK                    (0x00002000u)

enum sqif_ctrl_posn_enum
{
   SQIF_CTRL_FLASH_WR_MORE_POSN                       = (int)0,
   SQIF_CTRL_SQIF_CTRL_FLASH_WR_MORE_LSB_POSN         = (int)0,
   SQIF_CTRL_SQIF_CTRL_FLASH_WR_MORE_MSB_POSN         = (int)0,
   SQIF_CTRL_PAP_RAM_POSN                             = (int)1,
   SQIF_CTRL_SQIF_CTRL_PAP_RAM_LSB_POSN               = (int)1,
   SQIF_CTRL_SQIF_CTRL_PAP_RAM_MSB_POSN               = (int)1,
   SQIF_CTRL_PAP_WID_LSB_POSN                         = (int)2,
   SQIF_CTRL_SQIF_CTRL_PAP_WID_LSB_POSN               = (int)2,
   SQIF_CTRL_PAP_WID_MSB_POSN                         = (int)3,
   SQIF_CTRL_SQIF_CTRL_PAP_WID_MSB_POSN               = (int)3,
   SQIF_CTRL_CLK_RATE_FLASH_LSB_POSN                  = (int)4,
   SQIF_CTRL_SQIF_CTRL_CLK_RATE_FLASH_LSB_POSN        = (int)4,
   SQIF_CTRL_CLK_RATE_FLASH_MSB_POSN                  = (int)7,
   SQIF_CTRL_SQIF_CTRL_CLK_RATE_FLASH_MSB_POSN        = (int)7,
   SQIF_CTRL_CLK_RATE_RAM_LSB_POSN                    = (int)8,
   SQIF_CTRL_SQIF_CTRL_CLK_RATE_RAM_LSB_POSN          = (int)8,
   SQIF_CTRL_CLK_RATE_RAM_MSB_POSN                    = (int)11,
   SQIF_CTRL_SQIF_CTRL_CLK_RATE_RAM_MSB_POSN          = (int)11
};
typedef enum sqif_ctrl_posn_enum sqif_ctrl_posn;

#define SQIF_CTRL_FLASH_WR_MORE_MASK             (0x00000001u)
#define SQIF_CTRL_PAP_RAM_MASK                   (0x00000002u)
#define SQIF_CTRL_PAP_WID_LSB_MASK               (0x00000004u)
#define SQIF_CTRL_PAP_WID_MSB_MASK               (0x00000008u)
#define SQIF_CTRL_CLK_RATE_FLASH_LSB_MASK        (0x00000010u)
#define SQIF_CTRL_CLK_RATE_FLASH_MSB_MASK        (0x00000080u)
#define SQIF_CTRL_CLK_RATE_RAM_LSB_MASK          (0x00000100u)
#define SQIF_CTRL_CLK_RATE_RAM_MSB_MASK          (0x00000800u)

enum sqif_ctrl_pap_wid_enum
{
   SQIF_CTRL_PAP_WID_ONE                    = (int)0x0,
   SQIF_CTRL_PAP_WID_TWO                    = (int)0x1,
   SQIF_CTRL_PAP_WID_FOUR                   = (int)0x2,
   SQIF_CTRL_PAP_WID_RSVD                   = (int)0x3,
   MAX_SQIF_CTRL_PAP_WID                    = (int)0x3
};
#define NUM_SQIF_CTRL_PAP_WID (0x4)
typedef enum sqif_ctrl_pap_wid_enum sqif_ctrl_pap_wid;


enum sqif_ddr_dbg_sel_posn_enum
{
   SQIF_DDR_MARGIN_SEL_LSB_POSN                       = (int)0,
   SQIF_DDR_DBG_SEL_SQIF_DDR_MARGIN_SEL_LSB_POSN      = (int)0,
   SQIF_DDR_MARGIN_SEL_MSB_POSN                       = (int)2,
   SQIF_DDR_DBG_SEL_SQIF_DDR_MARGIN_SEL_MSB_POSN      = (int)2,
   SQIF_DDR_CENTER_SEL_LSB_POSN                       = (int)3,
   SQIF_DDR_DBG_SEL_SQIF_DDR_CENTER_SEL_LSB_POSN      = (int)3,
   SQIF_DDR_CENTER_SEL_MSB_POSN                       = (int)4,
   SQIF_DDR_DBG_SEL_SQIF_DDR_CENTER_SEL_MSB_POSN      = (int)4,
   SQIF_DDR_PAD_DBG_SEL_LSB_POSN                      = (int)5,
   SQIF_DDR_DBG_SEL_SQIF_DDR_PAD_DBG_SEL_LSB_POSN     = (int)5,
   SQIF_DDR_PAD_DBG_SEL_MSB_POSN                      = (int)7,
   SQIF_DDR_DBG_SEL_SQIF_DDR_PAD_DBG_SEL_MSB_POSN     = (int)7
};
typedef enum sqif_ddr_dbg_sel_posn_enum sqif_ddr_dbg_sel_posn;

#define SQIF_DDR_MARGIN_SEL_LSB_MASK             (0x00000001u)
#define SQIF_DDR_MARGIN_SEL_MSB_MASK             (0x00000004u)
#define SQIF_DDR_CENTER_SEL_LSB_MASK             (0x00000008u)
#define SQIF_DDR_CENTER_SEL_MSB_MASK             (0x00000010u)
#define SQIF_DDR_PAD_DBG_SEL_LSB_MASK            (0x00000020u)
#define SQIF_DDR_PAD_DBG_SEL_MSB_MASK            (0x00000080u)

enum sqif_inst_posn_enum
{
   SQIF_INST_CMD_LSB_POSN                             = (int)0,
   SQIF_INST_SQIF_INST_CMD_LSB_POSN                   = (int)0,
   SQIF_INST_CMD_MSB_POSN                             = (int)7,
   SQIF_INST_SQIF_INST_CMD_MSB_POSN                   = (int)7,
   SQIF_INST_MOD_LSB_POSN                             = (int)8,
   SQIF_INST_SQIF_INST_MOD_LSB_POSN                   = (int)8,
   SQIF_INST_MOD_MSB_POSN                             = (int)15,
   SQIF_INST_SQIF_INST_MOD_MSB_POSN                   = (int)15
};
typedef enum sqif_inst_posn_enum sqif_inst_posn;

#define SQIF_INST_CMD_LSB_MASK                   (0x00000001u)
#define SQIF_INST_CMD_MSB_MASK                   (0x00000080u)
#define SQIF_INST_MOD_LSB_MASK                   (0x00000100u)
#define SQIF_INST_MOD_MSB_MASK                   (0x00008000u)

enum sqif_pad_cal_posn_enum
{
   SQIF_PAD_CAL_RX_EN_POSN                            = (int)0,
   SQIF_PAD_CAL_SQIF_PAD_CAL_RX_EN_LSB_POSN           = (int)0,
   SQIF_PAD_CAL_SQIF_PAD_CAL_RX_EN_MSB_POSN           = (int)0,
   SQIF_PAD_CAL_RX_RST_POSN                           = (int)1,
   SQIF_PAD_CAL_SQIF_PAD_CAL_RX_RST_LSB_POSN          = (int)1,
   SQIF_PAD_CAL_SQIF_PAD_CAL_RX_RST_MSB_POSN          = (int)1,
   SQIF_PAD_CAL_CLK_EN_POSN                           = (int)2,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_EN_LSB_POSN          = (int)2,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_EN_MSB_POSN          = (int)2,
   SQIF_PAD_CAL_CLK_RST_POSN                          = (int)3,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_RST_LSB_POSN         = (int)3,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_RST_MSB_POSN         = (int)3,
   SQIF_PAD_CAL_CLK_DEL_SEL_POSN                      = (int)4,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_DEL_SEL_LSB_POSN     = (int)4,
   SQIF_PAD_CAL_SQIF_PAD_CAL_CLK_DEL_SEL_MSB_POSN     = (int)4
};
typedef enum sqif_pad_cal_posn_enum sqif_pad_cal_posn;

#define SQIF_PAD_CAL_RX_EN_MASK                  (0x00000001u)
#define SQIF_PAD_CAL_RX_RST_MASK                 (0x00000002u)
#define SQIF_PAD_CAL_CLK_EN_MASK                 (0x00000004u)
#define SQIF_PAD_CAL_CLK_RST_MASK                (0x00000008u)
#define SQIF_PAD_CAL_CLK_DEL_SEL_MASK            (0x00000010u)

enum sqif_peek_posn_enum
{
   SQIF_PEEK_DATA_LSB_POSN                            = (int)0,
   SQIF_PEEK_SQIF_PEEK_DATA_LSB_POSN                  = (int)0,
   SQIF_PEEK_DATA_MSB_POSN                            = (int)7,
   SQIF_PEEK_SQIF_PEEK_DATA_MSB_POSN                  = (int)7
};
typedef enum sqif_peek_posn_enum sqif_peek_posn;

#define SQIF_PEEK_DATA_LSB_MASK                  (0x00000001u)
#define SQIF_PEEK_DATA_MSB_MASK                  (0x00000080u)

enum sqif_peek_go_posn_enum
{
   SQIF_PEEK_GO_LAST_POSN                             = (int)0,
   SQIF_PEEK_GO_SQIF_PEEK_GO_LAST_LSB_POSN            = (int)0,
   SQIF_PEEK_GO_SQIF_PEEK_GO_LAST_MSB_POSN            = (int)0
};
typedef enum sqif_peek_go_posn_enum sqif_peek_go_posn;

#define SQIF_PEEK_GO_LAST_MASK                   (0x00000001u)

enum sqif_peek_go_last_enum
{
   SQIF_PEEK_GO_LAST_NO                     = (int)0x0,
   SQIF_PEEK_GO_LAST_YES                    = (int)0x1,
   MAX_SQIF_PEEK_GO_LAST                    = (int)0x1
};
#define NUM_SQIF_PEEK_GO_LAST (0x2)
typedef enum sqif_peek_go_last_enum sqif_peek_go_last;


enum sqif_phase_sel0_posn_enum
{
   SQIF_PHASE_SEL0_LOW_PAD0_LSB_POSN                  = (int)0,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD0_LSB_POSN  = (int)0,
   SQIF_PHASE_SEL0_LOW_PAD0_MSB_POSN                  = (int)4,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD0_MSB_POSN  = (int)4,
   SQIF_PHASE_SEL0_LOW_PAD1_LSB_POSN                  = (int)5,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD1_LSB_POSN  = (int)5,
   SQIF_PHASE_SEL0_LOW_PAD1_MSB_POSN                  = (int)9,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD1_MSB_POSN  = (int)9,
   SQIF_PHASE_SEL0_LOW_PAD2_LSB_POSN                  = (int)10,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD2_LSB_POSN  = (int)10,
   SQIF_PHASE_SEL0_LOW_PAD2_MSB_POSN                  = (int)14,
   SQIF_PHASE_SEL0_SQIF_PHASE_SEL0_LOW_PAD2_MSB_POSN  = (int)14
};
typedef enum sqif_phase_sel0_posn_enum sqif_phase_sel0_posn;

#define SQIF_PHASE_SEL0_LOW_PAD0_LSB_MASK        (0x00000001u)
#define SQIF_PHASE_SEL0_LOW_PAD0_MSB_MASK        (0x00000010u)
#define SQIF_PHASE_SEL0_LOW_PAD1_LSB_MASK        (0x00000020u)
#define SQIF_PHASE_SEL0_LOW_PAD1_MSB_MASK        (0x00000200u)
#define SQIF_PHASE_SEL0_LOW_PAD2_LSB_MASK        (0x00000400u)
#define SQIF_PHASE_SEL0_LOW_PAD2_MSB_MASK        (0x00004000u)

enum sqif_phase_sel1_posn_enum
{
   SQIF_PHASE_SEL1_LOW_PAD3_LSB_POSN                  = (int)0,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_LOW_PAD3_LSB_POSN  = (int)0,
   SQIF_PHASE_SEL1_LOW_PAD3_MSB_POSN                  = (int)4,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_LOW_PAD3_MSB_POSN  = (int)4,
   SQIF_PHASE_SEL1_HIGH_PAD0_LSB_POSN                 = (int)5,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_HIGH_PAD0_LSB_POSN = (int)5,
   SQIF_PHASE_SEL1_HIGH_PAD0_MSB_POSN                 = (int)9,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_HIGH_PAD0_MSB_POSN = (int)9,
   SQIF_PHASE_SEL1_HIGH_PAD1_LSB_POSN                 = (int)10,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_HIGH_PAD1_LSB_POSN = (int)10,
   SQIF_PHASE_SEL1_HIGH_PAD1_MSB_POSN                 = (int)14,
   SQIF_PHASE_SEL1_SQIF_PHASE_SEL1_HIGH_PAD1_MSB_POSN = (int)14
};
typedef enum sqif_phase_sel1_posn_enum sqif_phase_sel1_posn;

#define SQIF_PHASE_SEL1_LOW_PAD3_LSB_MASK        (0x00000001u)
#define SQIF_PHASE_SEL1_LOW_PAD3_MSB_MASK        (0x00000010u)
#define SQIF_PHASE_SEL1_HIGH_PAD0_LSB_MASK       (0x00000020u)
#define SQIF_PHASE_SEL1_HIGH_PAD0_MSB_MASK       (0x00000200u)
#define SQIF_PHASE_SEL1_HIGH_PAD1_LSB_MASK       (0x00000400u)
#define SQIF_PHASE_SEL1_HIGH_PAD1_MSB_MASK       (0x00004000u)

enum sqif_phase_sel2_posn_enum
{
   SQIF_PHASE_SEL2_HIGH_PAD2_LSB_POSN                 = (int)0,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_HIGH_PAD2_LSB_POSN = (int)0,
   SQIF_PHASE_SEL2_HIGH_PAD2_MSB_POSN                 = (int)4,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_HIGH_PAD2_MSB_POSN = (int)4,
   SQIF_PHASE_SEL2_HIGH_PAD3_LSB_POSN                 = (int)5,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_HIGH_PAD3_LSB_POSN = (int)5,
   SQIF_PHASE_SEL2_HIGH_PAD3_MSB_POSN                 = (int)9,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_HIGH_PAD3_MSB_POSN = (int)9,
   SQIF_PHASE_SEL2_DDR_OVERRIDE_POSN                  = (int)10,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_DDR_OVERRIDE_LSB_POSN = (int)10,
   SQIF_PHASE_SEL2_SQIF_PHASE_SEL2_DDR_OVERRIDE_MSB_POSN = (int)10
};
typedef enum sqif_phase_sel2_posn_enum sqif_phase_sel2_posn;

#define SQIF_PHASE_SEL2_HIGH_PAD2_LSB_MASK       (0x00000001u)
#define SQIF_PHASE_SEL2_HIGH_PAD2_MSB_MASK       (0x00000010u)
#define SQIF_PHASE_SEL2_HIGH_PAD3_LSB_MASK       (0x00000020u)
#define SQIF_PHASE_SEL2_HIGH_PAD3_MSB_MASK       (0x00000200u)
#define SQIF_PHASE_SEL2_DDR_OVERRIDE_MASK        (0x00000400u)

enum sqif_poke_posn_enum
{
   SQIF_POKE_DATA_LSB_POSN                            = (int)0,
   SQIF_POKE_SQIF_POKE_DATA_LSB_POSN                  = (int)0,
   SQIF_POKE_DATA_MSB_POSN                            = (int)7,
   SQIF_POKE_SQIF_POKE_DATA_MSB_POSN                  = (int)7
};
typedef enum sqif_poke_posn_enum sqif_poke_posn;

#define SQIF_POKE_DATA_LSB_MASK                  (0x00000001u)
#define SQIF_POKE_DATA_MSB_MASK                  (0x00000080u)

enum sqif_poke_last_posn_enum
{
   SQIF_POKE_LAST_DATA_LSB_POSN                       = (int)0,
   SQIF_POKE_LAST_SQIF_POKE_LAST_DATA_LSB_POSN        = (int)0,
   SQIF_POKE_LAST_DATA_MSB_POSN                       = (int)7,
   SQIF_POKE_LAST_SQIF_POKE_LAST_DATA_MSB_POSN        = (int)7
};
typedef enum sqif_poke_last_posn_enum sqif_poke_last_posn;

#define SQIF_POKE_LAST_DATA_LSB_MASK             (0x00000001u)
#define SQIF_POKE_LAST_DATA_MSB_MASK             (0x00000080u)

enum sqif_ram_conf_posn_enum
{
   SQIF_RAM_CONF_WID_LSB_POSN                         = (int)0,
   SQIF_RAM_CONF_SQIF_RAM_CONF_WID_LSB_POSN           = (int)0,
   SQIF_RAM_CONF_WID_MSB_POSN                         = (int)2,
   SQIF_RAM_CONF_SQIF_RAM_CONF_WID_MSB_POSN           = (int)2,
   SQIF_RAM_CONF_ADD_LSB_POSN                         = (int)3,
   SQIF_RAM_CONF_SQIF_RAM_CONF_ADD_LSB_POSN           = (int)3,
   SQIF_RAM_CONF_ADD_MSB_POSN                         = (int)4,
   SQIF_RAM_CONF_SQIF_RAM_CONF_ADD_MSB_POSN           = (int)4,
   SQIF_RAM_CONF_DUM_LSB_POSN                         = (int)5,
   SQIF_RAM_CONF_SQIF_RAM_CONF_DUM_LSB_POSN           = (int)5,
   SQIF_RAM_CONF_DUM_MSB_POSN                         = (int)6,
   SQIF_RAM_CONF_SQIF_RAM_CONF_DUM_MSB_POSN           = (int)6,
   SQIF_RAM_CONF_DUM_WR_EN_POSN                       = (int)7,
   SQIF_RAM_CONF_SQIF_RAM_CONF_DUM_WR_EN_LSB_POSN     = (int)7,
   SQIF_RAM_CONF_SQIF_RAM_CONF_DUM_WR_EN_MSB_POSN     = (int)7
};
typedef enum sqif_ram_conf_posn_enum sqif_ram_conf_posn;

#define SQIF_RAM_CONF_WID_LSB_MASK               (0x00000001u)
#define SQIF_RAM_CONF_WID_MSB_MASK               (0x00000004u)
#define SQIF_RAM_CONF_ADD_LSB_MASK               (0x00000008u)
#define SQIF_RAM_CONF_ADD_MSB_MASK               (0x00000010u)
#define SQIF_RAM_CONF_DUM_LSB_MASK               (0x00000020u)
#define SQIF_RAM_CONF_DUM_MSB_MASK               (0x00000040u)
#define SQIF_RAM_CONF_DUM_WR_EN_MASK             (0x00000080u)

enum sqif_ddr_center_posn_enum
{
   SQIF_DDR_CENTER_LSB_POSN                           = (int)0,
   SQIF_DDR_CENTER_SQIF_DDR_CENTER_LSB_POSN           = (int)0,
   SQIF_DDR_CENTER_MSB_POSN                           = (int)9,
   SQIF_DDR_CENTER_SQIF_DDR_CENTER_MSB_POSN           = (int)9
};
typedef enum sqif_ddr_center_posn_enum sqif_ddr_center_posn;

#define SQIF_DDR_CENTER_LSB_MASK                 (0x00000001u)
#define SQIF_DDR_CENTER_MSB_MASK                 (0x00000200u)

enum sqif_ddr_locked_posn_enum
{
   SQIF_DDR_LOCKED_LSB_POSN                           = (int)0,
   SQIF_DDR_LOCKED_SQIF_DDR_LOCKED_LSB_POSN           = (int)0,
   SQIF_DDR_LOCKED_MSB_POSN                           = (int)3,
   SQIF_DDR_LOCKED_SQIF_DDR_LOCKED_MSB_POSN           = (int)3
};
typedef enum sqif_ddr_locked_posn_enum sqif_ddr_locked_posn;

#define SQIF_DDR_LOCKED_LSB_MASK                 (0x00000001u)
#define SQIF_DDR_LOCKED_MSB_MASK                 (0x00000008u)

enum sqif_ddr_margin_posn_enum
{
   SQIF_DDR_MARGIN_LSB_POSN                           = (int)0,
   SQIF_DDR_MARGIN_SQIF_DDR_MARGIN_LSB_POSN           = (int)0,
   SQIF_DDR_MARGIN_MSB_POSN                           = (int)9,
   SQIF_DDR_MARGIN_SQIF_DDR_MARGIN_MSB_POSN           = (int)9
};
typedef enum sqif_ddr_margin_posn_enum sqif_ddr_margin_posn;

#define SQIF_DDR_MARGIN_LSB_MASK                 (0x00000001u)
#define SQIF_DDR_MARGIN_MSB_MASK                 (0x00000200u)

enum sqif_debug_sel_posn_enum
{
   SQIF_DEBUG_SEL_LSB_POSN                            = (int)0,
   SQIF_DEBUG_SEL_SQIF_DEBUG_SEL_LSB_POSN             = (int)0,
   SQIF_DEBUG_SEL_MSB_POSN                            = (int)3,
   SQIF_DEBUG_SEL_SQIF_DEBUG_SEL_MSB_POSN             = (int)3
};
typedef enum sqif_debug_sel_posn_enum sqif_debug_sel_posn;

#define SQIF_DEBUG_SEL_LSB_MASK                  (0x00000001u)
#define SQIF_DEBUG_SEL_MSB_MASK                  (0x00000008u)

enum sqif_flash_size_posn_enum
{
   SQIF_FLASH_SIZE_LSB_POSN                           = (int)0,
   SQIF_FLASH_SIZE_SQIF_FLASH_SIZE_LSB_POSN           = (int)0,
   SQIF_FLASH_SIZE_MSB_POSN                           = (int)5,
   SQIF_FLASH_SIZE_SQIF_FLASH_SIZE_MSB_POSN           = (int)5
};
typedef enum sqif_flash_size_posn_enum sqif_flash_size_posn;

#define SQIF_FLASH_SIZE_LSB_MASK                 (0x00000001u)
#define SQIF_FLASH_SIZE_MSB_MASK                 (0x00000020u)

enum sqif_pad_dbg_posn_enum
{
   SQIF_PAD_DBG_LSB_POSN                              = (int)0,
   SQIF_PAD_DBG_SQIF_PAD_DBG_LSB_POSN                 = (int)0,
   SQIF_PAD_DBG_MSB_POSN                              = (int)5,
   SQIF_PAD_DBG_SQIF_PAD_DBG_MSB_POSN                 = (int)5
};
typedef enum sqif_pad_dbg_posn_enum sqif_pad_dbg_posn;

#define SQIF_PAD_DBG_LSB_MASK                    (0x00000001u)
#define SQIF_PAD_DBG_MSB_MASK                    (0x00000020u)

enum sqif__access_ctrl_enum_posn_enum
{
   SQIF__P0_ACCESS_PERMISSION_POSN                    = (int)0,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   SQIF__P1_ACCESS_PERMISSION_POSN                    = (int)1,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   SQIF__P2_ACCESS_PERMISSION_POSN                    = (int)2,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   SQIF__P3_ACCESS_PERMISSION_POSN                    = (int)3,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   SQIF__ACCESS_CTRL_ENUM_SQIF__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum sqif__access_ctrl_enum_posn_enum sqif__access_ctrl_enum_posn;

#define SQIF__P0_ACCESS_PERMISSION_MASK          (0x00000001u)
#define SQIF__P1_ACCESS_PERMISSION_MASK          (0x00000002u)
#define SQIF__P2_ACCESS_PERMISSION_MASK          (0x00000004u)
#define SQIF__P3_ACCESS_PERMISSION_MASK          (0x00000008u)

enum sqif__p0_access_permission_enum
{
   SQIF__P0_ACCESS_BLOCKED                  = (int)0x0,
   SQIF__P0_ACCESS_UNBLOCKED                = (int)0x1,
   MAX_SQIF__P0_ACCESS_PERMISSION           = (int)0x1
};
#define NUM_SQIF__P0_ACCESS_PERMISSION (0x2)
typedef enum sqif__p0_access_permission_enum sqif__p0_access_permission;


enum sqif__p1_access_permission_enum
{
   SQIF__P1_ACCESS_BLOCKED                  = (int)0x0,
   SQIF__P1_ACCESS_UNBLOCKED                = (int)0x1,
   MAX_SQIF__P1_ACCESS_PERMISSION           = (int)0x1
};
#define NUM_SQIF__P1_ACCESS_PERMISSION (0x2)
typedef enum sqif__p1_access_permission_enum sqif__p1_access_permission;


enum sqif__p2_access_permission_enum
{
   SQIF__P2_ACCESS_BLOCKED                  = (int)0x0,
   SQIF__P2_ACCESS_UNBLOCKED                = (int)0x1,
   MAX_SQIF__P2_ACCESS_PERMISSION           = (int)0x1
};
#define NUM_SQIF__P2_ACCESS_PERMISSION (0x2)
typedef enum sqif__p2_access_permission_enum sqif__p2_access_permission;


enum sqif__p3_access_permission_enum
{
   SQIF__P3_ACCESS_BLOCKED                  = (int)0x0,
   SQIF__P3_ACCESS_UNBLOCKED                = (int)0x1,
   MAX_SQIF__P3_ACCESS_PERMISSION           = (int)0x1
};
#define NUM_SQIF__P3_ACCESS_PERMISSION (0x2)
typedef enum sqif__p3_access_permission_enum sqif__p3_access_permission;


enum sqif__mutex_lock_enum_enum
{
   SQIF__MUTEX_AVAILABLE                    = (int)0x0,
   SQIF__MUTEX_CLAIMED_BY_P0                = (int)0x1,
   SQIF__MUTEX_CLAIMED_BY_P1                = (int)0x2,
   SQIF__MUTEX_CLAIMED_BY_P2                = (int)0x4,
   SQIF__MUTEX_CLAIMED_BY_P3                = (int)0x8,
   SQIF__MUTEX_DISABLED                     = (int)0xF,
   MAX_SQIF__MUTEX_LOCK_ENUM                = (int)0xF
};
typedef enum sqif__mutex_lock_enum_enum sqif__mutex_lock_enum;


enum sqif_access_ctrl_posn_enum
{
   SQIF_ACCESS_CTRL_SQIF__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   SQIF_ACCESS_CTRL_SQIF__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   SQIF_ACCESS_CTRL_SQIF__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   SQIF_ACCESS_CTRL_SQIF__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   SQIF_ACCESS_CTRL_SQIF__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   SQIF_ACCESS_CTRL_SQIF__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   SQIF_ACCESS_CTRL_SQIF__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   SQIF_ACCESS_CTRL_SQIF__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum sqif_access_ctrl_posn_enum sqif_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_SQIF */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_SQIF */


#if defined(IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS

/* -- apps_sys_exceptions -- CPU exception sources in Apps Sys -- */

enum exception_source_enum
{
   EXCEPTION_SOURCE_REGISTERS               = (int)0x0,
   EXCEPTION_SOURCE_VML_CPU0                = (int)0x1,
   EXCEPTION_SOURCE_VML_CPU1                = (int)0x2,
   EXCEPTION_SOURCE_VML_TBUS_REMOTE         = (int)0x3,
   EXCEPTION_SOURCE_VML_DMAC                = (int)0x4,
   EXCEPTION_SOURCE_VM_CPU1_REMOTE_REQ      = (int)0x5,
   EXCEPTION_SOURCE_CPU1_ACCESS_FAULT       = (int)0x6,
   EXCEPTION_SOURCE_CACHE0_RAM0_ACCESS      = (int)0x9,
   EXCEPTION_SOURCE_CACHE0_RAM1_ACCESS      = (int)0xA,
   EXCEPTION_SOURCE_CACHE0_TAG_ACCESS       = (int)0xB,
   EXCEPTION_SOURCE_CACHE1_RAM0_ACCESS      = (int)0xC,
   EXCEPTION_SOURCE_CACHE1_RAM1_ACCESS      = (int)0xD,
   EXCEPTION_SOURCE_CACHE1_TAG_ACCESS       = (int)0xE,
   EXCEPTION_SOURCE_SQIF0_TBUS_ACCESS       = (int)0xF,
   EXCEPTION_SOURCE_SQIF1_TBUS_ACCESS       = (int)0x10,
   EXCEPTION_SOURCE_CPU0_VM_BUFFER_MAP      = (int)0x11,
   EXCEPTION_SOURCE_CPU1_VM_BUFFER_MAP      = (int)0x12,
   EXCEPTION_SOURCE_REMOTE_REGISTERS        = (int)0x13,
   EXCEPTION_SOURCE_MAX                     = (int)0x14,
   EXCEPTION_SOURCE_WIDTH_SPOILER           = (int)0x7FFFFFFF,
   EXCEPTION_SOURCE_SDIO_HOST_DM0_ACCESS    = (int)0x8,
   MAX_EXCEPTION_SOURCE                     = (int)0x7FFFFFFF
};
typedef enum exception_source_enum exception_source;


#endif /* IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_EXCEPTIONS */

#if defined(IO_DEFS_MODULE_APPS_SYS_CPU1_VM) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU1_VM
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU1_VM

/* -- apps_sys_cpu1_vm -- CPU1 Apps-specific virtual memory registers -- */

enum vm_cpu1_requests_enum_posn_enum
{
   VM_CPU1_REQUESTS_WRITE_POSN                        = (int)0,
   VM_CPU1_REQUESTS_ENUM_VM_CPU1_REQUESTS_WRITE_LSB_POSN = (int)0,
   VM_CPU1_REQUESTS_ENUM_VM_CPU1_REQUESTS_WRITE_MSB_POSN = (int)0,
   VM_CPU1_REQUESTS_READ_POSN                         = (int)1,
   VM_CPU1_REQUESTS_ENUM_VM_CPU1_REQUESTS_READ_LSB_POSN = (int)1,
   VM_CPU1_REQUESTS_ENUM_VM_CPU1_REQUESTS_READ_MSB_POSN = (int)1
};
typedef enum vm_cpu1_requests_enum_posn_enum vm_cpu1_requests_enum_posn;

#define VM_CPU1_REQUESTS_WRITE_MASK              (0x00000001u)
#define VM_CPU1_REQUESTS_READ_MASK               (0x00000002u)

enum vm_buffer_access_cpu1_offset_read_posn_enum
{
   VM_BUFFER_ACCESS_CPU1_OFFSET_READ_LSB_POSN         = (int)0,
   VM_BUFFER_ACCESS_CPU1_OFFSET_READ_VM_BUFFER_ACCESS_CPU1_OFFSET_READ_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU1_OFFSET_READ_MSB_POSN         = (int)17,
   VM_BUFFER_ACCESS_CPU1_OFFSET_READ_VM_BUFFER_ACCESS_CPU1_OFFSET_READ_MSB_POSN = (int)17
};
typedef enum vm_buffer_access_cpu1_offset_read_posn_enum vm_buffer_access_cpu1_offset_read_posn;

#define VM_BUFFER_ACCESS_CPU1_OFFSET_READ_LSB_MASK (0x00000001u)
#define VM_BUFFER_ACCESS_CPU1_OFFSET_READ_MSB_MASK (0x00020000u)

enum vm_buffer_access_cpu1_offset_write_posn_enum
{
   VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_LSB_POSN        = (int)0,
   VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_MSB_POSN        = (int)17,
   VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_MSB_POSN = (int)17
};
typedef enum vm_buffer_access_cpu1_offset_write_posn_enum vm_buffer_access_cpu1_offset_write_posn;

#define VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_LSB_MASK (0x00000001u)
#define VM_BUFFER_ACCESS_CPU1_OFFSET_WRITE_MSB_MASK (0x00020000u)

enum apps_sys_cpu1_vm__vm_buffer_access_do_action_enum_enum
{
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_DO_ACTION_READ = (int)0x0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_DO_ACTION_WRITE = (int)0x1,
   MAX_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_DO_ACTION_ENUM = (int)0x1
};
#define NUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_DO_ACTION_ENUM (0x2)
typedef enum apps_sys_cpu1_vm__vm_buffer_access_do_action_enum_enum apps_sys_cpu1_vm__vm_buffer_access_do_action_enum;


enum apps_sys_cpu1_vm__vm_buffer_access_handle_enum_posn_enum
{
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN = (int)7,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN = (int)7,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN = (int)8,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN = (int)8,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN = (int)11,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN = (int)11
};
typedef enum apps_sys_cpu1_vm__vm_buffer_access_handle_enum_posn_enum apps_sys_cpu1_vm__vm_buffer_access_handle_enum_posn;

#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_MASK (0x00000001u)
#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_MASK (0x00000080u)
#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_MASK (0x00000100u)
#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_MASK (0x00000800u)

enum apps_sys_cpu1_vm__vm_buffer_access_status_enum_posn_enum
{
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_MSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN = (int)1,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN = (int)1,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN = (int)4,
   APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN = (int)4
};
typedef enum apps_sys_cpu1_vm__vm_buffer_access_status_enum_posn_enum apps_sys_cpu1_vm__vm_buffer_access_status_enum_posn;

#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_MASK (0x00000001u)
#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_MASK (0x00000002u)
#define APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_MASK (0x00000010u)

enum apps_sys_cpu1_vm__access_ctrl_enum_posn_enum
{
   APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_POSN        = (int)0,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_POSN        = (int)1,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_POSN        = (int)2,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_POSN        = (int)3,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_CPU1_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_cpu1_vm__access_ctrl_enum_posn_enum apps_sys_cpu1_vm__access_ctrl_enum_posn;

#define APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum apps_sys_cpu1_vm__p0_access_permission_enum
{
   APPS_SYS_CPU1_VM__P0_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU1_VM__P0_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu1_vm__p0_access_permission_enum apps_sys_cpu1_vm__p0_access_permission;


enum apps_sys_cpu1_vm__p1_access_permission_enum
{
   APPS_SYS_CPU1_VM__P1_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU1_VM__P1_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu1_vm__p1_access_permission_enum apps_sys_cpu1_vm__p1_access_permission;


enum apps_sys_cpu1_vm__p2_access_permission_enum
{
   APPS_SYS_CPU1_VM__P2_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU1_VM__P2_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu1_vm__p2_access_permission_enum apps_sys_cpu1_vm__p2_access_permission;


enum apps_sys_cpu1_vm__p3_access_permission_enum
{
   APPS_SYS_CPU1_VM__P3_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU1_VM__P3_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu1_vm__p3_access_permission_enum apps_sys_cpu1_vm__p3_access_permission;


enum apps_sys_cpu1_vm__mutex_lock_enum_enum
{
   APPS_SYS_CPU1_VM__MUTEX_AVAILABLE        = (int)0x0,
   APPS_SYS_CPU1_VM__MUTEX_CLAIMED_BY_P0    = (int)0x1,
   APPS_SYS_CPU1_VM__MUTEX_CLAIMED_BY_P1    = (int)0x2,
   APPS_SYS_CPU1_VM__MUTEX_CLAIMED_BY_P2    = (int)0x4,
   APPS_SYS_CPU1_VM__MUTEX_CLAIMED_BY_P3    = (int)0x8,
   APPS_SYS_CPU1_VM__MUTEX_DISABLED         = (int)0xF,
   MAX_APPS_SYS_CPU1_VM__MUTEX_LOCK_ENUM    = (int)0xF
};
typedef enum apps_sys_cpu1_vm__mutex_lock_enum_enum apps_sys_cpu1_vm__mutex_lock_enum;


enum apps_sys_cpu1_vm__vm_requests_status_enum_posn_enum
{
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_POSN    = (int)12,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_POSN = (int)13,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17,
   APPS_SYS_CPU1_VM__VM_REQUESTS_STATUS_ENUM_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum apps_sys_cpu1_vm__vm_requests_status_enum_posn_enum apps_sys_cpu1_vm__vm_requests_status_enum_posn;

#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_MASK (0x00000001u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_MASK (0x00000080u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_MASK (0x00000100u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_MASK (0x00000800u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_MASK (0x00001000u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_MASK (0x00002000u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_LSB_MASK (0x00004000u)
#define APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_MSB_MASK (0x00020000u)

enum vm_cpu1_clear_port_posn_enum
{
   VM_CPU1_CLEAR_PORT_VM_CPU1_REQUESTS_WRITE_LSB_POSN = (int)0,
   VM_CPU1_CLEAR_PORT_VM_CPU1_REQUESTS_WRITE_MSB_POSN = (int)0,
   VM_CPU1_CLEAR_PORT_VM_CPU1_REQUESTS_READ_LSB_POSN  = (int)1,
   VM_CPU1_CLEAR_PORT_VM_CPU1_REQUESTS_READ_MSB_POSN  = (int)1
};
typedef enum vm_cpu1_clear_port_posn_enum vm_cpu1_clear_port_posn;


enum vm_buffer_access_cpu1_handle_posn_enum
{
   VM_BUFFER_ACCESS_CPU1_HANDLE_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU1_HANDLE_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN = (int)7,
   VM_BUFFER_ACCESS_CPU1_HANDLE_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN = (int)8,
   VM_BUFFER_ACCESS_CPU1_HANDLE_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN = (int)11
};
typedef enum vm_buffer_access_cpu1_handle_posn_enum vm_buffer_access_cpu1_handle_posn;


enum vm_buffer_access_cpu1_status_posn_enum
{
   VM_BUFFER_ACCESS_CPU1_STATUS_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU1_STATUS_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACTIVE_MSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU1_STATUS_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN = (int)1,
   VM_BUFFER_ACCESS_CPU1_STATUS_APPS_SYS_CPU1_VM__VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN = (int)4
};
typedef enum vm_buffer_access_cpu1_status_posn_enum vm_buffer_access_cpu1_status_posn;


enum vm_cpu1_access_ctrl_posn_enum
{
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   VM_CPU1_ACCESS_CTRL_APPS_SYS_CPU1_VM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum vm_cpu1_access_ctrl_posn_enum vm_cpu1_access_ctrl_posn;


enum vm_request_cpu1_read_status_posn_enum
{
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   VM_REQUEST_CPU1_READ_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum vm_request_cpu1_read_status_posn_enum vm_request_cpu1_read_status_posn;


enum vm_request_cpu1_write_status_posn_enum
{
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   VM_REQUEST_CPU1_WRITE_STATUS_APPS_SYS_CPU1_VM__VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum vm_request_cpu1_write_status_posn_enum vm_request_cpu1_write_status_posn;


#endif /* IO_DEFS_MODULE_APPS_SYS_CPU1_VM */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU1_VM */


#if defined(IO_DEFS_MODULE_HOST_SYS_CONFIG) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_HOST_SYS_CONFIG
#define __IO_DEFS_H__IO_DEFS_MODULE_HOST_SYS_CONFIG

/* -- host_sys_config -- Host subsystem interface control registers -- */

enum host_sys_block_ids_enum
{
   HOST_SYS_UART_BLOCK_ID                   = (int)0x0,
   HOST_SYS_USB_BLOCK_ID                    = (int)0x1,
   HOST_SYS_SDIO_BLOCK_ID                   = (int)0x2,
   HOST_SYS_BITSERIAL_0_BLOCK_ID            = (int)0x3,
   HOST_SYS_BITSERIAL_1_BLOCK_ID            = (int)0x4,
   HOST_SYS_HOST_CONFIG_REGS_BLOCK_ID       = (int)0xD,
   HOST_SYS_REGISTERS_BLOCK_ID              = (int)0xE,
   HOST_SYS_DEBUG_TBUS_BLOCK_ID             = (int)0xF,
   MAX_HOST_SYS_BLOCK_IDS                   = (int)0xF
};
typedef enum host_sys_block_ids_enum host_sys_block_ids;


enum host_sys_ram_ema_fields_posn_enum
{
   HOST_SYS_EMAW_LSB_POSN                             = (int)0,
   HOST_SYS_RAM_EMA_FIELDS_HOST_SYS_EMAW_LSB_POSN     = (int)0,
   HOST_SYS_EMAW_MSB_POSN                             = (int)1,
   HOST_SYS_RAM_EMA_FIELDS_HOST_SYS_EMAW_MSB_POSN     = (int)1,
   HOST_SYS_EMA_LSB_POSN                              = (int)2,
   HOST_SYS_RAM_EMA_FIELDS_HOST_SYS_EMA_LSB_POSN      = (int)2,
   HOST_SYS_EMA_MSB_POSN                              = (int)4,
   HOST_SYS_RAM_EMA_FIELDS_HOST_SYS_EMA_MSB_POSN      = (int)4
};
typedef enum host_sys_ram_ema_fields_posn_enum host_sys_ram_ema_fields_posn;

#define HOST_SYS_EMAW_LSB_MASK                   (0x00000001u)
#define HOST_SYS_EMAW_MSB_MASK                   (0x00000002u)
#define HOST_SYS_EMA_LSB_MASK                    (0x00000004u)
#define HOST_SYS_EMA_MSB_MASK                    (0x00000010u)

enum host_sys_remote_access_address_enum_enum
{
   HOST_SYS_REMOTE_ACCESS_ADDRESS_UNUSED    = (int)0x0,
   MAX_HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM  = (int)0x0
};
#define NUM_HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM (0x1)
typedef enum host_sys_remote_access_address_enum_enum host_sys_remote_access_address_enum;


enum host_sys_remote_access_address_enum_posn_enum
{
   HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_LSB_POSN   = (int)1,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM_HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_LSB_POSN = (int)1,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_MSB_POSN   = (int)8,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM_HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_MSB_POSN = (int)8,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_LSB_POSN   = (int)9,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM_HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_LSB_POSN = (int)9,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_MSB_POSN   = (int)12,
   HOST_SYS_REMOTE_ACCESS_ADDRESS_ENUM_HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_MSB_POSN = (int)12
};
typedef enum host_sys_remote_access_address_enum_posn_enum host_sys_remote_access_address_enum_posn;

#define HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_LSB_MASK (0x00000002u)
#define HOST_SYS_REMOTE_ACCESS_ADDRESS_REG_ADDR_MSB_MASK (0x00000100u)
#define HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_LSB_MASK (0x00000200u)
#define HOST_SYS_REMOTE_ACCESS_ADDRESS_BLOCK_ID_MSB_MASK (0x00001000u)

enum host_sys_build_options_posn_enum
{
   HOST_SYS_BUILD_OPTIONS_POWER_AWARE_POSN            = (int)0,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_POWER_AWARE_LSB_POSN = (int)0,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_POWER_AWARE_MSB_POSN = (int)0,
   HOST_SYS_BUILD_OPTIONS_BTM_PHASE_LOCKED_CLOCKS_POSN = (int)1,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_BTM_PHASE_LOCKED_CLOCKS_LSB_POSN = (int)1,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_BTM_PHASE_LOCKED_CLOCKS_MSB_POSN = (int)1,
   HOST_SYS_BUILD_OPTIONS_BTM_LOCKED_CLOCKS_POSN      = (int)2,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_BTM_LOCKED_CLOCKS_LSB_POSN = (int)2,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_BTM_LOCKED_CLOCKS_MSB_POSN = (int)2,
   HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_LSB_POSN    = (int)3,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_LSB_POSN = (int)3,
   HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_MSB_POSN    = (int)5,
   HOST_SYS_BUILD_OPTIONS_HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_MSB_POSN = (int)5
};
typedef enum host_sys_build_options_posn_enum host_sys_build_options_posn;

#define HOST_SYS_BUILD_OPTIONS_POWER_AWARE_MASK  (0x00000001u)
#define HOST_SYS_BUILD_OPTIONS_BTM_PHASE_LOCKED_CLOCKS_MASK (0x00000002u)
#define HOST_SYS_BUILD_OPTIONS_BTM_LOCKED_CLOCKS_MASK (0x00000004u)
#define HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_LSB_MASK (0x00000008u)
#define HOST_SYS_BUILD_OPTIONS_CLK_80M_DIV_MIN_MSB_MASK (0x00000020u)

enum host_sys_config_force_clk_enables_posn_enum
{
   HOST_SYS_UART_CLK_FORCE_ENABLE_POSN                = (int)0,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_UART_CLK_FORCE_ENABLE_LSB_POSN = (int)0,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_UART_CLK_FORCE_ENABLE_MSB_POSN = (int)0,
   HOST_SYS_USB_CLK_FORCE_ENABLE_POSN                 = (int)1,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_USB_CLK_FORCE_ENABLE_LSB_POSN = (int)1,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_USB_CLK_FORCE_ENABLE_MSB_POSN = (int)1,
   HOST_SYS_BITSERIAL_0_CLK_FORCE_ENABLE_POSN         = (int)2,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_BITSERIAL_0_CLK_FORCE_ENABLE_LSB_POSN = (int)2,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_BITSERIAL_0_CLK_FORCE_ENABLE_MSB_POSN = (int)2,
   HOST_SYS_BITSERIAL_1_CLK_FORCE_ENABLE_POSN         = (int)3,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_BITSERIAL_1_CLK_FORCE_ENABLE_LSB_POSN = (int)3,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_BITSERIAL_1_CLK_FORCE_ENABLE_MSB_POSN = (int)3,
   HOST_SYS_SDIO_CLK_FORCE_ENABLE_POSN                = (int)4,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_SDIO_CLK_FORCE_ENABLE_LSB_POSN = (int)4,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_SDIO_CLK_FORCE_ENABLE_MSB_POSN = (int)4,
   HOST_SYS_REG_IF_CLK_FORCE_ENABLE_POSN              = (int)5,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_REG_IF_CLK_FORCE_ENABLE_LSB_POSN = (int)5,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_REG_IF_CLK_FORCE_ENABLE_MSB_POSN = (int)5,
   HOST_SYS_CONFIG_CLK_FORCE_ENABLE_POSN              = (int)6,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_CONFIG_CLK_FORCE_ENABLE_LSB_POSN = (int)6,
   HOST_SYS_CONFIG_FORCE_CLK_ENABLES_HOST_SYS_CONFIG_CLK_FORCE_ENABLE_MSB_POSN = (int)6
};
typedef enum host_sys_config_force_clk_enables_posn_enum host_sys_config_force_clk_enables_posn;

#define HOST_SYS_UART_CLK_FORCE_ENABLE_MASK      (0x00000001u)
#define HOST_SYS_USB_CLK_FORCE_ENABLE_MASK       (0x00000002u)
#define HOST_SYS_BITSERIAL_0_CLK_FORCE_ENABLE_MASK (0x00000004u)
#define HOST_SYS_BITSERIAL_1_CLK_FORCE_ENABLE_MASK (0x00000008u)
#define HOST_SYS_SDIO_CLK_FORCE_ENABLE_MASK      (0x00000010u)
#define HOST_SYS_REG_IF_CLK_FORCE_ENABLE_MASK    (0x00000020u)
#define HOST_SYS_CONFIG_CLK_FORCE_ENABLE_MASK    (0x00000040u)

enum host_sys_debug_select_posn_enum
{
   HOST_SYS_DEBUG_SELECT1500_LSB_POSN                 = (int)0,
   HOST_SYS_DEBUG_SELECT_HOST_SYS_DEBUG_SELECT1500_LSB_POSN = (int)0,
   HOST_SYS_DEBUG_SELECT1500_MSB_POSN                 = (int)3,
   HOST_SYS_DEBUG_SELECT_HOST_SYS_DEBUG_SELECT1500_MSB_POSN = (int)3,
   HOST_SYS_DEBUG_SELECT3116_LSB_POSN                 = (int)4,
   HOST_SYS_DEBUG_SELECT_HOST_SYS_DEBUG_SELECT3116_LSB_POSN = (int)4,
   HOST_SYS_DEBUG_SELECT3116_MSB_POSN                 = (int)7,
   HOST_SYS_DEBUG_SELECT_HOST_SYS_DEBUG_SELECT3116_MSB_POSN = (int)7
};
typedef enum host_sys_debug_select_posn_enum host_sys_debug_select_posn;

#define HOST_SYS_DEBUG_SELECT1500_LSB_MASK       (0x00000001u)
#define HOST_SYS_DEBUG_SELECT1500_MSB_MASK       (0x00000008u)
#define HOST_SYS_DEBUG_SELECT3116_LSB_MASK       (0x00000010u)
#define HOST_SYS_DEBUG_SELECT3116_MSB_MASK       (0x00000080u)

enum host_sys_mmu_ram_seq_timing_reg0_posn_enum
{
   HOST_SYS_TValF3_LSB_POSN                           = (int)0,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValF3_LSB_POSN = (int)0,
   HOST_SYS_TValF3_MSB_POSN                           = (int)3,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValF3_MSB_POSN = (int)3,
   HOST_SYS_TValF1_LSB_POSN                           = (int)4,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValF1_LSB_POSN = (int)4,
   HOST_SYS_TValF1_MSB_POSN                           = (int)7,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValF1_MSB_POSN = (int)7,
   HOST_SYS_TValE1_LSB_POSN                           = (int)8,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValE1_LSB_POSN = (int)8,
   HOST_SYS_TValE1_MSB_POSN                           = (int)11,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValE1_MSB_POSN = (int)11,
   HOST_SYS_TValD1_LSB_POSN                           = (int)12,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValD1_LSB_POSN = (int)12,
   HOST_SYS_TValD1_MSB_POSN                           = (int)15,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG0_HOST_SYS_TValD1_MSB_POSN = (int)15
};
typedef enum host_sys_mmu_ram_seq_timing_reg0_posn_enum host_sys_mmu_ram_seq_timing_reg0_posn;

#define HOST_SYS_TValF3_LSB_MASK                 (0x00000001u)
#define HOST_SYS_TValF3_MSB_MASK                 (0x00000008u)
#define HOST_SYS_TValF1_LSB_MASK                 (0x00000010u)
#define HOST_SYS_TValF1_MSB_MASK                 (0x00000080u)
#define HOST_SYS_TValE1_LSB_MASK                 (0x00000100u)
#define HOST_SYS_TValE1_MSB_MASK                 (0x00000800u)
#define HOST_SYS_TValD1_LSB_MASK                 (0x00001000u)
#define HOST_SYS_TValD1_MSB_MASK                 (0x00008000u)

enum host_sys_mmu_ram_seq_timing_reg1_posn_enum
{
   HOST_SYS_TValC1_LSB_POSN                           = (int)0,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValC1_LSB_POSN = (int)0,
   HOST_SYS_TValC1_MSB_POSN                           = (int)3,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValC1_MSB_POSN = (int)3,
   HOST_SYS_TValB1_LSB_POSN                           = (int)4,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValB1_LSB_POSN = (int)4,
   HOST_SYS_TValB1_MSB_POSN                           = (int)7,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValB1_MSB_POSN = (int)7,
   HOST_SYS_TValA1_LSB_POSN                           = (int)8,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValA1_LSB_POSN = (int)8,
   HOST_SYS_TValA1_MSB_POSN                           = (int)11,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValA1_MSB_POSN = (int)11,
   HOST_SYS_TValD_LSB_POSN                            = (int)12,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValD_LSB_POSN = (int)12,
   HOST_SYS_TValD_MSB_POSN                            = (int)15,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG1_HOST_SYS_TValD_MSB_POSN = (int)15
};
typedef enum host_sys_mmu_ram_seq_timing_reg1_posn_enum host_sys_mmu_ram_seq_timing_reg1_posn;

#define HOST_SYS_TValC1_LSB_MASK                 (0x00000001u)
#define HOST_SYS_TValC1_MSB_MASK                 (0x00000008u)
#define HOST_SYS_TValB1_LSB_MASK                 (0x00000010u)
#define HOST_SYS_TValB1_MSB_MASK                 (0x00000080u)
#define HOST_SYS_TValA1_LSB_MASK                 (0x00000100u)
#define HOST_SYS_TValA1_MSB_MASK                 (0x00000800u)
#define HOST_SYS_TValD_LSB_MASK                  (0x00001000u)
#define HOST_SYS_TValD_MSB_MASK                  (0x00008000u)

enum host_sys_mmu_ram_seq_timing_reg2_posn_enum
{
   HOST_SYS_TValC_LSB_POSN                            = (int)0,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValC_LSB_POSN = (int)0,
   HOST_SYS_TValC_MSB_POSN                            = (int)3,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValC_MSB_POSN = (int)3,
   HOST_SYS_TValB_LSB_POSN                            = (int)4,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValB_LSB_POSN = (int)4,
   HOST_SYS_TValB_MSB_POSN                            = (int)7,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValB_MSB_POSN = (int)7,
   HOST_SYS_TValA_LSB_POSN                            = (int)8,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValA_LSB_POSN = (int)8,
   HOST_SYS_TValA_MSB_POSN                            = (int)11,
   HOST_SYS_MMU_RAM_SEQ_TIMING_REG2_HOST_SYS_TValA_MSB_POSN = (int)11
};
typedef enum host_sys_mmu_ram_seq_timing_reg2_posn_enum host_sys_mmu_ram_seq_timing_reg2_posn;

#define HOST_SYS_TValC_LSB_MASK                  (0x00000001u)
#define HOST_SYS_TValC_MSB_MASK                  (0x00000008u)
#define HOST_SYS_TValB_LSB_MASK                  (0x00000010u)
#define HOST_SYS_TValB_MSB_MASK                  (0x00000080u)
#define HOST_SYS_TValA_LSB_MASK                  (0x00000100u)
#define HOST_SYS_TValA_MSB_MASK                  (0x00000800u)

enum host_sys_bitserial_0_clk_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_LSB_POSN   = (int)0,
   HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_MSB_POSN   = (int)6,
   HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_clk_input_pio_ctrl_posn_enum host_sys_bitserial_0_clk_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_CLK_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_0_clk_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_clk_output_pio_ctrl_posn_enum host_sys_bitserial_0_clk_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_CLK_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_0_data_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_data_input_pio_ctrl_posn_enum host_sys_bitserial_0_data_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_DATA_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_0_data_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_MSB_POSN = (int)6,
   HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_data_output_pio_ctrl_posn_enum host_sys_bitserial_0_data_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_DATA_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_0_sel_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_LSB_POSN   = (int)0,
   HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_MSB_POSN   = (int)6,
   HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_sel_input_pio_ctrl_posn_enum host_sys_bitserial_0_sel_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_SEL_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_0_sel_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_0_sel_output_pio_ctrl_posn_enum host_sys_bitserial_0_sel_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_0_SEL_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_clk_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_LSB_POSN   = (int)0,
   HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_MSB_POSN   = (int)6,
   HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_clk_input_pio_ctrl_posn_enum host_sys_bitserial_1_clk_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_CLK_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_clk_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_clk_output_pio_ctrl_posn_enum host_sys_bitserial_1_clk_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_CLK_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_data_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_data_input_pio_ctrl_posn_enum host_sys_bitserial_1_data_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_DATA_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_data_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_MSB_POSN = (int)6,
   HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_data_output_pio_ctrl_posn_enum host_sys_bitserial_1_data_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_DATA_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_sel_input_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_LSB_POSN   = (int)0,
   HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_MSB_POSN   = (int)6,
   HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_sel_input_pio_ctrl_posn_enum host_sys_bitserial_1_sel_input_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_SEL_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_bitserial_1_sel_output_pio_ctrl_posn_enum
{
   HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_LSB_POSN  = (int)0,
   HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_MSB_POSN  = (int)6,
   HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_bitserial_1_sel_output_pio_ctrl_posn_enum host_sys_bitserial_1_sel_output_pio_ctrl_posn;

#define HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_BITSERIAL_1_SEL_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_btm_debug_select_posn_enum
{
   HOST_SYS_BTM_DEBUG_SELECT_LSB_POSN                 = (int)0,
   HOST_SYS_BTM_DEBUG_SELECT_HOST_SYS_BTM_DEBUG_SELECT_LSB_POSN = (int)0,
   HOST_SYS_BTM_DEBUG_SELECT_MSB_POSN                 = (int)5,
   HOST_SYS_BTM_DEBUG_SELECT_HOST_SYS_BTM_DEBUG_SELECT_MSB_POSN = (int)5
};
typedef enum host_sys_btm_debug_select_posn_enum host_sys_btm_debug_select_posn;

#define HOST_SYS_BTM_DEBUG_SELECT_LSB_MASK       (0x00000001u)
#define HOST_SYS_BTM_DEBUG_SELECT_MSB_MASK       (0x00000020u)

enum host_sys_clk_80m_div_posn_enum
{
   HOST_SYS_CLK_80M_DIV_LSB_POSN                      = (int)0,
   HOST_SYS_CLK_80M_DIV_HOST_SYS_CLK_80M_DIV_LSB_POSN = (int)0,
   HOST_SYS_CLK_80M_DIV_MSB_POSN                      = (int)7,
   HOST_SYS_CLK_80M_DIV_HOST_SYS_CLK_80M_DIV_MSB_POSN = (int)7
};
typedef enum host_sys_clk_80m_div_posn_enum host_sys_clk_80m_div_posn;

#define HOST_SYS_CLK_80M_DIV_LSB_MASK            (0x00000001u)
#define HOST_SYS_CLK_80M_DIV_MSB_MASK            (0x00000080u)

enum host_sys_reg_access_subsys_in_addr_en_posn_enum
{
   HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_LSB_POSN     = (int)0,
   HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_LSB_POSN = (int)0,
   HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_MSB_POSN     = (int)15,
   HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_MSB_POSN = (int)15
};
typedef enum host_sys_reg_access_subsys_in_addr_en_posn_enum host_sys_reg_access_subsys_in_addr_en_posn;

#define HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_LSB_MASK (0x00000001u)
#define HOST_SYS_REG_ACCESS_SUBSYS_IN_ADDR_EN_MSB_MASK (0x00008000u)

enum host_sys_sdio_sideband_interrupt_output_pio_ctrl_posn_enum
{
   HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_MSB_POSN = (int)6,
   HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_sdio_sideband_interrupt_output_pio_ctrl_posn_enum host_sys_sdio_sideband_interrupt_output_pio_ctrl_posn;

#define HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_SDIO_SIDEBAND_INTERRUPT_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_uart_ctsb_input_pio_ctrl_posn_enum
{
   HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_LSB_POSN         = (int)0,
   HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_MSB_POSN         = (int)6,
   HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_uart_ctsb_input_pio_ctrl_posn_enum host_sys_uart_ctsb_input_pio_ctrl_posn;

#define HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_UART_CTSB_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_uart_data_input_pio_ctrl_posn_enum
{
   HOST_SYS_UART_DATA_INPUT_PIO_CTRL_LSB_POSN         = (int)0,
   HOST_SYS_UART_DATA_INPUT_PIO_CTRL_HOST_SYS_UART_DATA_INPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_UART_DATA_INPUT_PIO_CTRL_MSB_POSN         = (int)6,
   HOST_SYS_UART_DATA_INPUT_PIO_CTRL_HOST_SYS_UART_DATA_INPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_uart_data_input_pio_ctrl_posn_enum host_sys_uart_data_input_pio_ctrl_posn;

#define HOST_SYS_UART_DATA_INPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_UART_DATA_INPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_uart_data_output_pio_ctrl_posn_enum
{
   HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_LSB_POSN        = (int)0,
   HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_MSB_POSN        = (int)6,
   HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_uart_data_output_pio_ctrl_posn_enum host_sys_uart_data_output_pio_ctrl_posn;

#define HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_UART_DATA_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum host_sys_uart_rtsb_output_pio_ctrl_posn_enum
{
   HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_LSB_POSN        = (int)0,
   HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_LSB_POSN = (int)0,
   HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_MSB_POSN        = (int)6,
   HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_MSB_POSN = (int)6
};
typedef enum host_sys_uart_rtsb_output_pio_ctrl_posn_enum host_sys_uart_rtsb_output_pio_ctrl_posn;

#define HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_LSB_MASK (0x00000001u)
#define HOST_SYS_UART_RTSB_OUTPUT_PIO_CTRL_MSB_MASK (0x00000040u)

enum usb_tx_prefetch_ram_16eps_ram0_ema_posn_enum
{
   USB_TX_PREFETCH_RAM_16EPS_RAM0_EMA_HOST_SYS_EMAW_LSB_POSN = (int)0,
   USB_TX_PREFETCH_RAM_16EPS_RAM0_EMA_HOST_SYS_EMAW_MSB_POSN = (int)1,
   USB_TX_PREFETCH_RAM_16EPS_RAM0_EMA_HOST_SYS_EMA_LSB_POSN = (int)2,
   USB_TX_PREFETCH_RAM_16EPS_RAM0_EMA_HOST_SYS_EMA_MSB_POSN = (int)4
};
typedef enum usb_tx_prefetch_ram_16eps_ram0_ema_posn_enum usb_tx_prefetch_ram_16eps_ram0_ema_posn;


enum usb_tx_prefetch_ram_16eps_ram1_ema_posn_enum
{
   USB_TX_PREFETCH_RAM_16EPS_RAM1_EMA_HOST_SYS_EMAW_LSB_POSN = (int)0,
   USB_TX_PREFETCH_RAM_16EPS_RAM1_EMA_HOST_SYS_EMAW_MSB_POSN = (int)1,
   USB_TX_PREFETCH_RAM_16EPS_RAM1_EMA_HOST_SYS_EMA_LSB_POSN = (int)2,
   USB_TX_PREFETCH_RAM_16EPS_RAM1_EMA_HOST_SYS_EMA_MSB_POSN = (int)4
};
typedef enum usb_tx_prefetch_ram_16eps_ram1_ema_posn_enum usb_tx_prefetch_ram_16eps_ram1_ema_posn;


enum usb_tx_prefetch_ram_16eps_ram2_ema_posn_enum
{
   USB_TX_PREFETCH_RAM_16EPS_RAM2_EMA_HOST_SYS_EMAW_LSB_POSN = (int)0,
   USB_TX_PREFETCH_RAM_16EPS_RAM2_EMA_HOST_SYS_EMAW_MSB_POSN = (int)1,
   USB_TX_PREFETCH_RAM_16EPS_RAM2_EMA_HOST_SYS_EMA_LSB_POSN = (int)2,
   USB_TX_PREFETCH_RAM_16EPS_RAM2_EMA_HOST_SYS_EMA_MSB_POSN = (int)4
};
typedef enum usb_tx_prefetch_ram_16eps_ram2_ema_posn_enum usb_tx_prefetch_ram_16eps_ram2_ema_posn;


enum usb_tx_prefetch_ram_16eps_ram3_ema_posn_enum
{
   USB_TX_PREFETCH_RAM_16EPS_RAM3_EMA_HOST_SYS_EMAW_LSB_POSN = (int)0,
   USB_TX_PREFETCH_RAM_16EPS_RAM3_EMA_HOST_SYS_EMAW_MSB_POSN = (int)1,
   USB_TX_PREFETCH_RAM_16EPS_RAM3_EMA_HOST_SYS_EMA_LSB_POSN = (int)2,
   USB_TX_PREFETCH_RAM_16EPS_RAM3_EMA_HOST_SYS_EMA_MSB_POSN = (int)4
};
typedef enum usb_tx_prefetch_ram_16eps_ram3_ema_posn_enum usb_tx_prefetch_ram_16eps_ram3_ema_posn;


#endif /* IO_DEFS_MODULE_HOST_SYS_CONFIG */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_HOST_SYS_CONFIG */

#if defined(IO_DEFS_MODULE_K32_CORE) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_CORE
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_CORE

/* -- k32_core -- Kalimba 32-bit Core Control registers -- */

enum arithmetic_mode_posn_enum
{
   ADDSUB_SATURATE_ON_OVERFLOW_POSN                   = (int)0,
   ARITHMETIC_MODE_ADDSUB_SATURATE_ON_OVERFLOW_LSB_POSN = (int)0,
   ARITHMETIC_MODE_ADDSUB_SATURATE_ON_OVERFLOW_MSB_POSN = (int)0,
   ARITHMETIC_16BIT_MODE_POSN                         = (int)1,
   ARITHMETIC_MODE_ARITHMETIC_16BIT_MODE_LSB_POSN     = (int)1,
   ARITHMETIC_MODE_ARITHMETIC_16BIT_MODE_MSB_POSN     = (int)1,
   DISABLE_UNBIASED_ROUNDING_POSN                     = (int)2,
   ARITHMETIC_MODE_DISABLE_UNBIASED_ROUNDING_LSB_POSN = (int)2,
   ARITHMETIC_MODE_DISABLE_UNBIASED_ROUNDING_MSB_POSN = (int)2,
   DISABLE_FRAC_MULT_ROUNDING_POSN                    = (int)3,
   ARITHMETIC_MODE_DISABLE_FRAC_MULT_ROUNDING_LSB_POSN = (int)3,
   ARITHMETIC_MODE_DISABLE_FRAC_MULT_ROUNDING_MSB_POSN = (int)3,
   DISABLE_RMAC_STORE_ROUNDING_POSN                   = (int)4,
   ARITHMETIC_MODE_DISABLE_RMAC_STORE_ROUNDING_LSB_POSN = (int)4,
   ARITHMETIC_MODE_DISABLE_RMAC_STORE_ROUNDING_MSB_POSN = (int)4
};
typedef enum arithmetic_mode_posn_enum arithmetic_mode_posn;

#define ADDSUB_SATURATE_ON_OVERFLOW_MASK         (0x00000001u)
#define ARITHMETIC_16BIT_MODE_MASK               (0x00000002u)
#define DISABLE_UNBIASED_ROUNDING_MASK           (0x00000004u)
#define DISABLE_FRAC_MULT_ROUNDING_MASK          (0x00000008u)
#define DISABLE_RMAC_STORE_ROUNDING_MASK         (0x00000010u)

enum bitreverse_addr_posn_enum
{
   BITREVERSE_ADDR_LSB_POSN                           = (int)0,
   BITREVERSE_ADDR_BITREVERSE_ADDR_LSB_POSN           = (int)0,
   BITREVERSE_ADDR_MSB_POSN                           = (int)31,
   BITREVERSE_ADDR_BITREVERSE_ADDR_MSB_POSN           = (int)31
};
typedef enum bitreverse_addr_posn_enum bitreverse_addr_posn;

#define BITREVERSE_ADDR_LSB_MASK                 (0x00000001u)
#define BITREVERSE_ADDR_MSB_MASK                 (0x80000000u)

enum bitreverse_data_posn_enum
{
   BITREVERSE_DATA_LSB_POSN                           = (int)0,
   BITREVERSE_DATA_BITREVERSE_DATA_LSB_POSN           = (int)0,
   BITREVERSE_DATA_MSB_POSN                           = (int)31,
   BITREVERSE_DATA_BITREVERSE_DATA_MSB_POSN           = (int)31
};
typedef enum bitreverse_data_posn_enum bitreverse_data_posn;

#define BITREVERSE_DATA_LSB_MASK                 (0x00000001u)
#define BITREVERSE_DATA_MSB_MASK                 (0x80000000u)

enum bitreverse_data16_posn_enum
{
   BITREVERSE_DATA16_LSB_POSN                         = (int)0,
   BITREVERSE_DATA16_BITREVERSE_DATA16_LSB_POSN       = (int)0,
   BITREVERSE_DATA16_MSB_POSN                         = (int)31,
   BITREVERSE_DATA16_BITREVERSE_DATA16_MSB_POSN       = (int)31
};
typedef enum bitreverse_data16_posn_enum bitreverse_data16_posn;

#define BITREVERSE_DATA16_LSB_MASK               (0x00000001u)
#define BITREVERSE_DATA16_MSB_MASK               (0x80000000u)

enum bitreverse_val_posn_enum
{
   BITREVERSE_VAL_LSB_POSN                            = (int)0,
   BITREVERSE_VAL_BITREVERSE_VAL_LSB_POSN             = (int)0,
   BITREVERSE_VAL_MSB_POSN                            = (int)31,
   BITREVERSE_VAL_BITREVERSE_VAL_MSB_POSN             = (int)31
};
typedef enum bitreverse_val_posn_enum bitreverse_val_posn;

#define BITREVERSE_VAL_LSB_MASK                  (0x00000001u)
#define BITREVERSE_VAL_MSB_MASK                  (0x80000000u)

enum dbg_counters_en_posn_enum
{
   DBG_COUNTERS_EN_POSN                               = (int)0,
   DBG_COUNTERS_EN_DBG_COUNTERS_EN_LSB_POSN           = (int)0,
   DBG_COUNTERS_EN_DBG_COUNTERS_EN_MSB_POSN           = (int)0
};
typedef enum dbg_counters_en_posn_enum dbg_counters_en_posn;

#define DBG_COUNTERS_EN_MASK                     (0x00000001u)

enum frame_pointer_posn_enum
{
   FRAME_POINTER_LSB_POSN                             = (int)0,
   FRAME_POINTER_FRAME_POINTER_LSB_POSN               = (int)0,
   FRAME_POINTER_MSB_POSN                             = (int)31,
   FRAME_POINTER_FRAME_POINTER_MSB_POSN               = (int)31
};
typedef enum frame_pointer_posn_enum frame_pointer_posn;

#define FRAME_POINTER_LSB_MASK                   (0x00000001u)
#define FRAME_POINTER_MSB_MASK                   (0x80000000u)

enum mm_doloop_end_posn_enum
{
   MM_DOLOOP_END_LSB_POSN                             = (int)0,
   MM_DOLOOP_END_MM_DOLOOP_END_LSB_POSN               = (int)0,
   MM_DOLOOP_END_MSB_POSN                             = (int)31,
   MM_DOLOOP_END_MM_DOLOOP_END_MSB_POSN               = (int)31
};
typedef enum mm_doloop_end_posn_enum mm_doloop_end_posn;

#define MM_DOLOOP_END_LSB_MASK                   (0x00000001u)
#define MM_DOLOOP_END_MSB_MASK                   (0x80000000u)

enum mm_doloop_start_posn_enum
{
   MM_DOLOOP_START_LSB_POSN                           = (int)0,
   MM_DOLOOP_START_MM_DOLOOP_START_LSB_POSN           = (int)0,
   MM_DOLOOP_START_MSB_POSN                           = (int)31,
   MM_DOLOOP_START_MM_DOLOOP_START_MSB_POSN           = (int)31
};
typedef enum mm_doloop_start_posn_enum mm_doloop_start_posn;

#define MM_DOLOOP_START_LSB_MASK                 (0x00000001u)
#define MM_DOLOOP_START_MSB_MASK                 (0x80000000u)

enum mm_quotient_posn_enum
{
   MM_QUOTIENT_LSB_POSN                               = (int)0,
   MM_QUOTIENT_MM_QUOTIENT_LSB_POSN                   = (int)0,
   MM_QUOTIENT_MSB_POSN                               = (int)31,
   MM_QUOTIENT_MM_QUOTIENT_MSB_POSN                   = (int)31
};
typedef enum mm_quotient_posn_enum mm_quotient_posn;

#define MM_QUOTIENT_LSB_MASK                     (0x00000001u)
#define MM_QUOTIENT_MSB_MASK                     (0x80000000u)

enum mm_rem_posn_enum
{
   MM_REM_LSB_POSN                                    = (int)0,
   MM_REM_MM_REM_LSB_POSN                             = (int)0,
   MM_REM_MSB_POSN                                    = (int)31,
   MM_REM_MM_REM_MSB_POSN                             = (int)31
};
typedef enum mm_rem_posn_enum mm_rem_posn;

#define MM_REM_LSB_MASK                          (0x00000001u)
#define MM_REM_MSB_MASK                          (0x80000000u)

enum mm_rintlink_posn_enum
{
   MM_RINTLINK_LSB_POSN                               = (int)0,
   MM_RINTLINK_MM_RINTLINK_LSB_POSN                   = (int)0,
   MM_RINTLINK_MSB_POSN                               = (int)31,
   MM_RINTLINK_MM_RINTLINK_MSB_POSN                   = (int)31
};
typedef enum mm_rintlink_posn_enum mm_rintlink_posn;

#define MM_RINTLINK_LSB_MASK                     (0x00000001u)
#define MM_RINTLINK_MSB_MASK                     (0x80000000u)

enum num_core_stalls_posn_enum
{
   NUM_CORE_STALLS_LSB_POSN                           = (int)0,
   NUM_CORE_STALLS_NUM_CORE_STALLS_LSB_POSN           = (int)0,
   NUM_CORE_STALLS_MSB_POSN                           = (int)31,
   NUM_CORE_STALLS_NUM_CORE_STALLS_MSB_POSN           = (int)31
};
typedef enum num_core_stalls_posn_enum num_core_stalls_posn;

#define NUM_CORE_STALLS_LSB_MASK                 (0x00000001u)
#define NUM_CORE_STALLS_MSB_MASK                 (0x80000000u)

enum num_instrs_posn_enum
{
   NUM_INSTRS_LSB_POSN                                = (int)0,
   NUM_INSTRS_NUM_INSTRS_LSB_POSN                     = (int)0,
   NUM_INSTRS_MSB_POSN                                = (int)31,
   NUM_INSTRS_NUM_INSTRS_MSB_POSN                     = (int)31
};
typedef enum num_instrs_posn_enum num_instrs_posn;

#define NUM_INSTRS_LSB_MASK                      (0x00000001u)
#define NUM_INSTRS_MSB_MASK                      (0x80000000u)

enum num_instr_expand_stalls_posn_enum
{
   NUM_INSTR_EXPAND_STALLS_LSB_POSN                   = (int)0,
   NUM_INSTR_EXPAND_STALLS_NUM_INSTR_EXPAND_STALLS_LSB_POSN = (int)0,
   NUM_INSTR_EXPAND_STALLS_MSB_POSN                   = (int)31,
   NUM_INSTR_EXPAND_STALLS_NUM_INSTR_EXPAND_STALLS_MSB_POSN = (int)31
};
typedef enum num_instr_expand_stalls_posn_enum num_instr_expand_stalls_posn;

#define NUM_INSTR_EXPAND_STALLS_LSB_MASK         (0x00000001u)
#define NUM_INSTR_EXPAND_STALLS_MSB_MASK         (0x80000000u)

enum num_mem_access_stalls_posn_enum
{
   NUM_MEM_ACCESS_STALLS_LSB_POSN                     = (int)0,
   NUM_MEM_ACCESS_STALLS_NUM_MEM_ACCESS_STALLS_LSB_POSN = (int)0,
   NUM_MEM_ACCESS_STALLS_MSB_POSN                     = (int)31,
   NUM_MEM_ACCESS_STALLS_NUM_MEM_ACCESS_STALLS_MSB_POSN = (int)31
};
typedef enum num_mem_access_stalls_posn_enum num_mem_access_stalls_posn;

#define NUM_MEM_ACCESS_STALLS_LSB_MASK           (0x00000001u)
#define NUM_MEM_ACCESS_STALLS_MSB_MASK           (0x80000000u)

enum num_run_clks_posn_enum
{
   NUM_RUN_CLKS_LSB_POSN                              = (int)0,
   NUM_RUN_CLKS_NUM_RUN_CLKS_LSB_POSN                 = (int)0,
   NUM_RUN_CLKS_MSB_POSN                              = (int)31,
   NUM_RUN_CLKS_NUM_RUN_CLKS_MSB_POSN                 = (int)31
};
typedef enum num_run_clks_posn_enum num_run_clks_posn;

#define NUM_RUN_CLKS_LSB_MASK                    (0x00000001u)
#define NUM_RUN_CLKS_MSB_MASK                    (0x80000000u)

enum pc_status_posn_enum
{
   PC_STATUS_LSB_POSN                                 = (int)0,
   PC_STATUS_PC_STATUS_LSB_POSN                       = (int)0,
   PC_STATUS_MSB_POSN                                 = (int)31,
   PC_STATUS_PC_STATUS_MSB_POSN                       = (int)31
};
typedef enum pc_status_posn_enum pc_status_posn;

#define PC_STATUS_LSB_MASK                       (0x00000001u)
#define PC_STATUS_MSB_MASK                       (0x80000000u)

enum stack_end_addr_posn_enum
{
   STACK_END_ADDR_LSB_POSN                            = (int)0,
   STACK_END_ADDR_STACK_END_ADDR_LSB_POSN             = (int)0,
   STACK_END_ADDR_MSB_POSN                            = (int)31,
   STACK_END_ADDR_STACK_END_ADDR_MSB_POSN             = (int)31
};
typedef enum stack_end_addr_posn_enum stack_end_addr_posn;

#define STACK_END_ADDR_LSB_MASK                  (0x00000001u)
#define STACK_END_ADDR_MSB_MASK                  (0x80000000u)

enum stack_overflow_pc_posn_enum
{
   STACK_OVERFLOW_PC_LSB_POSN                         = (int)0,
   STACK_OVERFLOW_PC_STACK_OVERFLOW_PC_LSB_POSN       = (int)0,
   STACK_OVERFLOW_PC_MSB_POSN                         = (int)31,
   STACK_OVERFLOW_PC_STACK_OVERFLOW_PC_MSB_POSN       = (int)31
};
typedef enum stack_overflow_pc_posn_enum stack_overflow_pc_posn;

#define STACK_OVERFLOW_PC_LSB_MASK               (0x00000001u)
#define STACK_OVERFLOW_PC_MSB_MASK               (0x80000000u)

enum stack_pointer_posn_enum
{
   STACK_POINTER_LSB_POSN                             = (int)0,
   STACK_POINTER_STACK_POINTER_LSB_POSN               = (int)0,
   STACK_POINTER_MSB_POSN                             = (int)31,
   STACK_POINTER_STACK_POINTER_MSB_POSN               = (int)31
};
typedef enum stack_pointer_posn_enum stack_pointer_posn;

#define STACK_POINTER_LSB_MASK                   (0x00000001u)
#define STACK_POINTER_MSB_MASK                   (0x80000000u)

enum stack_start_addr_posn_enum
{
   STACK_START_ADDR_LSB_POSN                          = (int)0,
   STACK_START_ADDR_STACK_START_ADDR_LSB_POSN         = (int)0,
   STACK_START_ADDR_MSB_POSN                          = (int)31,
   STACK_START_ADDR_STACK_START_ADDR_MSB_POSN         = (int)31
};
typedef enum stack_start_addr_posn_enum stack_start_addr_posn;

#define STACK_START_ADDR_LSB_MASK                (0x00000001u)
#define STACK_START_ADDR_MSB_MASK                (0x80000000u)

enum test_reg_0_posn_enum
{
   TEST_REG_0_LSB_POSN                                = (int)0,
   TEST_REG_0_TEST_REG_0_LSB_POSN                     = (int)0,
   TEST_REG_0_MSB_POSN                                = (int)31,
   TEST_REG_0_TEST_REG_0_MSB_POSN                     = (int)31
};
typedef enum test_reg_0_posn_enum test_reg_0_posn;

#define TEST_REG_0_LSB_MASK                      (0x00000001u)
#define TEST_REG_0_MSB_MASK                      (0x80000000u)

enum test_reg_1_posn_enum
{
   TEST_REG_1_LSB_POSN                                = (int)0,
   TEST_REG_1_TEST_REG_1_LSB_POSN                     = (int)0,
   TEST_REG_1_MSB_POSN                                = (int)31,
   TEST_REG_1_TEST_REG_1_MSB_POSN                     = (int)31
};
typedef enum test_reg_1_posn_enum test_reg_1_posn;

#define TEST_REG_1_LSB_MASK                      (0x00000001u)
#define TEST_REG_1_MSB_MASK                      (0x80000000u)

enum test_reg_2_posn_enum
{
   TEST_REG_2_LSB_POSN                                = (int)0,
   TEST_REG_2_TEST_REG_2_LSB_POSN                     = (int)0,
   TEST_REG_2_MSB_POSN                                = (int)31,
   TEST_REG_2_TEST_REG_2_MSB_POSN                     = (int)31
};
typedef enum test_reg_2_posn_enum test_reg_2_posn;

#define TEST_REG_2_LSB_MASK                      (0x00000001u)
#define TEST_REG_2_MSB_MASK                      (0x80000000u)

enum test_reg_3_posn_enum
{
   TEST_REG_3_LSB_POSN                                = (int)0,
   TEST_REG_3_TEST_REG_3_LSB_POSN                     = (int)0,
   TEST_REG_3_MSB_POSN                                = (int)31,
   TEST_REG_3_TEST_REG_3_MSB_POSN                     = (int)31
};
typedef enum test_reg_3_posn_enum test_reg_3_posn;

#define TEST_REG_3_LSB_MASK                      (0x00000001u)
#define TEST_REG_3_MSB_MASK                      (0x80000000u)

#endif /* IO_DEFS_MODULE_K32_CORE */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_CORE */




#if defined(IO_DEFS_MODULE_APPS_SYS_SYS) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SYS
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SYS

/* -- apps_sys_sys -- Auto-generated from subsystem manifest TMux Config. -- */

enum apps_sys_debug_source_enum
{
   APPS_SYS_DEBUG_SOURCE_BUS_TRANSACTION_MUX = (int)0x1,
   APPS_SYS_DEBUG_SOURCE_BUS_DEBUG          = (int)0x2,
   APPS_SYS_DEBUG_SOURCE_BUS_INTERRUPT      = (int)0x3,
   APPS_SYS_DEBUG_SOURCE_BUS_MESSAGE        = (int)0x4,
   APPS_SYS_DEBUG_SOURCE_BUS_MISC           = (int)0x5,
   APPS_SYS_DEBUG_SOURCE_BUS_REMOTE         = (int)0x6,
   APPS_SYS_DEBUG_SOURCE_CLKGEN             = (int)0x7,
   APPS_SYS_DEBUG_SOURCE_IRQ0               = (int)0x8,
   APPS_SYS_DEBUG_SOURCE_IRQ1               = (int)0x9,
   APPS_SYS_DEBUG_SOURCE_VM                 = (int)0xA,
   APPS_SYS_DEBUG_SOURCE_CACHE0             = (int)0xB,
   APPS_SYS_DEBUG_SOURCE_CACHE1             = (int)0xC,
   APPS_SYS_DEBUG_SOURCE_INTERCONN          = (int)0x10,
   APPS_SYS_DEBUG_SOURCE_SQIF_DATAPATH0     = (int)0x11,
   APPS_SYS_DEBUG_SOURCE_SQIF_DATAPATH1     = (int)0x12,
   APPS_SYS_DEBUG_SOURCE_SHARED_DM          = (int)0x13,
   APPS_SYS_DEBUG_SOURCE_DMAC               = (int)0x14,
   APPS_SYS_DEBUG_SOURCE_BUS_MEM_ADPTR      = (int)0x18,
   APPS_SYS_DEBUG_SOURCE_KAL0_ACTIVITY      = (int)0x19,
   APPS_SYS_DEBUG_SOURCE_KAL1_ACTIVITY      = (int)0x1A,
   APPS_SYS_DEBUG_SOURCE_KAL0_EXECUTION     = (int)0x1B,
   APPS_SYS_DEBUG_SOURCE_KAL1_EXECUTION     = (int)0x1C,
   APPS_SYS_DEBUG_SOURCE_KAL0_PM_MUX        = (int)0x1D,
   APPS_SYS_DEBUG_SOURCE_KAL1_PM_MUX        = (int)0x1E,
   APPS_SYS_DEBUG_SOURCE_KAL0_READ_CACHE_DM = (int)0x1F,
   APPS_SYS_DEBUG_SOURCE_KAL1_READ_CACHE_DM = (int)0x20,
   APPS_SYS_DEBUG_SOURCE_CACHE0_RAM0_MUX    = (int)0x21,
   APPS_SYS_DEBUG_SOURCE_CACHE0_RAM1_MUX    = (int)0x22,
   APPS_SYS_DEBUG_SOURCE_CACHE0_TAG_RAM_MUX = (int)0x23,
   APPS_SYS_DEBUG_SOURCE_CACHE1_RAM0_MUX    = (int)0x24,
   APPS_SYS_DEBUG_SOURCE_CACHE1_RAM1_MUX    = (int)0x25,
   APPS_SYS_DEBUG_SOURCE_CACHE1_TAG_RAM_MUX = (int)0x26,
   APPS_SYS_DEBUG_SOURCE_KAL0_DM_RAM_MUX    = (int)0x27,
   APPS_SYS_DEBUG_SOURCE_KAL1_DM_RAM_MUX    = (int)0x28,
   APPS_SYS_DEBUG_SOURCE_REG_BUS_WAIT0      = (int)0x29,
   APPS_SYS_DEBUG_SOURCE_REG_BUS_WAIT1      = (int)0x2A,
   APPS_SYS_DEBUG_SOURCE_REG_BUS_READ_DATA  = (int)0x2B,
   APPS_SYS_DEBUG_SOURCE_REG_BUS_WRITE_DATA = (int)0x2C,
   APPS_SYS_DEBUG_SOURCE_CORE_VITALS        = (int)0x2D,
   APPS_SYS_DEBUG_SOURCE_SQIF0_TBUS_BRIDGE  = (int)0x2E,
   APPS_SYS_DEBUG_SOURCE_SQIF1_TBUS_BRIDGE  = (int)0x2F,
   APPS_SYS_DEBUG_SOURCE_KAL0_PC            = (int)0x30,
   APPS_SYS_DEBUG_SOURCE_KAL1_PC            = (int)0x31,
   APPS_SYS_DEBUG_SOURCE_KAL_PM_BKPOINTS    = (int)0x32,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_CPU_DM  = (int)0x33,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_SHARED1 = (int)0x34,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_SHARED2 = (int)0x35,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_TCM     = (int)0x36,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_CACHE0  = (int)0x37,
   APPS_SYS_DEBUG_SOURCE_AUTO_SLEEP_CACHE1  = (int)0x38,
   APPS_SYS_DEBUG_SOURCE_TRACE              = (int)0x39,
   APPS_SYS_DEBUG_SOURCE_VM_INTERCONNECT    = (int)0x3A,
   APPS_SYS_DEBUG_SOURCE_KALIMBA_DEBUG_MUX  = (int)0x3B,
   APPS_SYS_DEBUG_SOURCE_APU                = (int)0x3C,
   APPS_SYS_DEBUG_SOURCE_DEBUG_MUX          = (int)0x3D,
   APPS_SYS_DEBUG_SOURCE_MISC_TBUS_ARB      = (int)0x3E,
   APPS_SYS_DEBUG_SOURCE_EXT_REGS           = (int)0x3F,
   APPS_SYS_DEBUG_SOURCE_TCM0_LOWER_MUX     = (int)0x40,
   APPS_SYS_DEBUG_SOURCE_TCM0_UPPER_MUX     = (int)0x41,
   APPS_SYS_DEBUG_SOURCE_TCM1_MUX           = (int)0x42,
   APPS_SYS_DEBUG_SOURCE_LAST               = (int)0x42,
   APPS_SYS_DEBUG_SOURCE_SDIO_HOST_DEBUG0   = (int)0x15,
   APPS_SYS_DEBUG_SOURCE_SDIO_HOST_DEBUG1   = (int)0x16,
   APPS_SYS_DEBUG_SOURCE_SDIO_HOST_DEBUG2   = (int)0x17,
   MAX_APPS_SYS_DEBUG_SOURCE                = (int)0x42
};
typedef enum apps_sys_debug_source_enum apps_sys_debug_source;


enum chip_version_posn_enum
{
   CHIP_VERSION_MAJOR_LSB_POSN                        = (int)0,
   CHIP_VERSION_CHIP_VERSION_MAJOR_LSB_POSN           = (int)0,
   CHIP_VERSION_MAJOR_MSB_POSN                        = (int)7,
   CHIP_VERSION_CHIP_VERSION_MAJOR_MSB_POSN           = (int)7,
   CHIP_VERSION_VARIANT_LSB_POSN                      = (int)8,
   CHIP_VERSION_CHIP_VERSION_VARIANT_LSB_POSN         = (int)8,
   CHIP_VERSION_VARIANT_MSB_POSN                      = (int)11,
   CHIP_VERSION_CHIP_VERSION_VARIANT_MSB_POSN         = (int)11,
   CHIP_VERSION_MINOR_LSB_POSN                        = (int)12,
   CHIP_VERSION_CHIP_VERSION_MINOR_LSB_POSN           = (int)12,
   CHIP_VERSION_MINOR_MSB_POSN                        = (int)15,
   CHIP_VERSION_CHIP_VERSION_MINOR_MSB_POSN           = (int)15
};
typedef enum chip_version_posn_enum chip_version_posn;

#define CHIP_VERSION_MAJOR_LSB_MASK              (0x00000001u)
#define CHIP_VERSION_MAJOR_MSB_MASK              (0x00000080u)
#define CHIP_VERSION_VARIANT_LSB_MASK            (0x00000100u)
#define CHIP_VERSION_VARIANT_MSB_MASK            (0x00000800u)
#define CHIP_VERSION_MINOR_LSB_MASK              (0x00001000u)
#define CHIP_VERSION_MINOR_MSB_MASK              (0x00008000u)

enum dmac_key_sel_enum
{
   EFUSE_KEY_SEL                            = (int)0x0,
   RTL1_KEY_SEL                             = (int)0x1,
   RTL2_KEY_SEL                             = (int)0x2,
   MAX_DMAC_KEY_SEL                         = (int)0x2
};
#define NUM_DMAC_KEY_SEL (0x3)
typedef enum dmac_key_sel_enum dmac_key_sel;


enum remote_registers_host_sys_regs_block_id_enum_posn_enum
{
   REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_LSB_POSN       = (int)0,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_ENUM_REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_LSB_POSN = (int)0,
   REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_MSB_POSN       = (int)11,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_ENUM_REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_MSB_POSN = (int)11,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_LSB_POSN   = (int)12,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_ENUM_REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_LSB_POSN = (int)12,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_MSB_POSN   = (int)15,
   REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_ENUM_REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_MSB_POSN = (int)15
};
typedef enum remote_registers_host_sys_regs_block_id_enum_posn_enum remote_registers_host_sys_regs_block_id_enum_posn;

#define REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_LSB_MASK (0x00000001u)
#define REMOTE_REGISTERS_HOST_SYS_REGS_ADDR_MSB_MASK (0x00000800u)
#define REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_LSB_MASK (0x00001000u)
#define REMOTE_REGISTERS_HOST_SYS_REGS_BLOCK_ID_MSB_MASK (0x00008000u)

enum shared_dm_masters_enum
{
   SHARED_DM_MASTER_VM_FREE_PAGE_ADD        = (int)0x0,
   SHARED_DM_MASTER_VML0                    = (int)0x1,
   SHARED_DM_MASTER_VML1                    = (int)0x2,
   SHARED_DM_MASTER_VML2                    = (int)0x3,
   SHARED_DM_MASTER_VML3                    = (int)0x4,
   SHARED_DM_MASTER_VM_REQ_CPU0             = (int)0x5,
   SHARED_DM_MASTER_VM_REQ_CPU1             = (int)0x6,
   SHARED_DM_MASTER_CPU0_DM                 = (int)0x7,
   SHARED_DM_MASTER_CPU1_DM                 = (int)0x8,
   SHARED_DM_MASTER_REMOTE                  = (int)0x9,
   SHARED_DM_MASTER_DMAC                    = (int)0xC,
   SHARED_DM_MASTER_CACHE0_LOG              = (int)0xD,
   SHARED_DM_MASTER_CACHE1_LOG              = (int)0xE,
   SHARED_DM_MASTER_TRACE_LOG               = (int)0xF,
   NUM_SHARED_DM_MASTERS                    = (int)0x10,
   SHARED_DM_MASTER_SDIO_HOST               = (int)0xB,
   MAX_SHARED_DM_MASTERS                    = (int)0x10
};
typedef enum shared_dm_masters_enum shared_dm_masters;


enum tcm_remap_controls_posn_enum
{
   TCM_REMAP_LOW_SELECT_POSN                          = (int)0,
   TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_LSB_POSN   = (int)0,
   TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_MSB_POSN   = (int)0,
   TCM_REMAP_LOW_ENABLE_POSN                          = (int)1,
   TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_LSB_POSN   = (int)1,
   TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_MSB_POSN   = (int)1,
   TCM_REMAP_HIGH_SELECT_POSN                         = (int)2,
   TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_LSB_POSN  = (int)2,
   TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_MSB_POSN  = (int)2,
   TCM_REMAP_HIGH_ENABLE_POSN                         = (int)3,
   TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_LSB_POSN  = (int)3,
   TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_MSB_POSN  = (int)3
};
typedef enum tcm_remap_controls_posn_enum tcm_remap_controls_posn;

#define TCM_REMAP_LOW_SELECT_MASK                (0x00000001u)
#define TCM_REMAP_LOW_ENABLE_MASK                (0x00000002u)
#define TCM_REMAP_HIGH_SELECT_MASK               (0x00000004u)
#define TCM_REMAP_HIGH_ENABLE_MASK               (0x00000008u)

enum apps_force_pwm_mode_req_posn_enum
{
   APPS_FORCE_PWM_MODE_ANA_SMPS_REQ_POSN              = (int)0,
   APPS_FORCE_PWM_MODE_REQ_APPS_FORCE_PWM_MODE_ANA_SMPS_REQ_LSB_POSN = (int)0,
   APPS_FORCE_PWM_MODE_REQ_APPS_FORCE_PWM_MODE_ANA_SMPS_REQ_MSB_POSN = (int)0,
   APPS_FORCE_PWM_MODE_CORE_SMPS_REQ_POSN             = (int)1,
   APPS_FORCE_PWM_MODE_REQ_APPS_FORCE_PWM_MODE_CORE_SMPS_REQ_LSB_POSN = (int)1,
   APPS_FORCE_PWM_MODE_REQ_APPS_FORCE_PWM_MODE_CORE_SMPS_REQ_MSB_POSN = (int)1
};
typedef enum apps_force_pwm_mode_req_posn_enum apps_force_pwm_mode_req_posn;

#define APPS_FORCE_PWM_MODE_ANA_SMPS_REQ_MASK    (0x00000001u)
#define APPS_FORCE_PWM_MODE_CORE_SMPS_REQ_MASK   (0x00000002u)

enum apps_remote_subsystem_accessor_access_control_posn_enum
{
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_DMAC_POSN = (int)0,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_DMAC_LSB_POSN = (int)0,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_DMAC_MSB_POSN = (int)0,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_REMOTE_POSN = (int)1,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_REMOTE_LSB_POSN = (int)1,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_REMOTE_MSB_POSN = (int)1,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_DMAC_POSN = (int)2,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_DMAC_LSB_POSN = (int)2,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_DMAC_MSB_POSN = (int)2,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_REMOTE_POSN = (int)3,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_REMOTE_LSB_POSN = (int)3,
   APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_REMOTE_MSB_POSN = (int)3
};
typedef enum apps_remote_subsystem_accessor_access_control_posn_enum apps_remote_subsystem_accessor_access_control_posn;

#define APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_DMAC_MASK (0x00000001u)
#define APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW0_REMOTE_MASK (0x00000002u)
#define APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_DMAC_MASK (0x00000004u)
#define APPS_REMOTE_SUBSYSTEM_ACCESSOR_ACCESS_CONTROL_WINDOW1_REMOTE_MASK (0x00000008u)

enum apps_smps_in_pwm_mode_status_posn_enum
{
   APPS_SMPS_ANA_IN_PWM_MODE_POSN                     = (int)0,
   APPS_SMPS_IN_PWM_MODE_STATUS_APPS_SMPS_ANA_IN_PWM_MODE_LSB_POSN = (int)0,
   APPS_SMPS_IN_PWM_MODE_STATUS_APPS_SMPS_ANA_IN_PWM_MODE_MSB_POSN = (int)0,
   APPS_SMPS_CORE_IN_PWM_MODE_POSN                    = (int)1,
   APPS_SMPS_IN_PWM_MODE_STATUS_APPS_SMPS_CORE_IN_PWM_MODE_LSB_POSN = (int)1,
   APPS_SMPS_IN_PWM_MODE_STATUS_APPS_SMPS_CORE_IN_PWM_MODE_MSB_POSN = (int)1
};
typedef enum apps_smps_in_pwm_mode_status_posn_enum apps_smps_in_pwm_mode_status_posn;

#define APPS_SMPS_ANA_IN_PWM_MODE_MASK           (0x00000001u)
#define APPS_SMPS_CORE_IN_PWM_MODE_MASK          (0x00000002u)

enum apps_sys_dm0_access_restrict_posn_enum
{
   APPS_SYS_DM0_ACCESS_RESTRICT_SDIO_HOST_POSN        = (int)1,
   APPS_SYS_DM0_ACCESS_RESTRICT_APPS_SYS_DM0_ACCESS_RESTRICT_SDIO_HOST_LSB_POSN = (int)1,
   APPS_SYS_DM0_ACCESS_RESTRICT_APPS_SYS_DM0_ACCESS_RESTRICT_SDIO_HOST_MSB_POSN = (int)1
};
typedef enum apps_sys_dm0_access_restrict_posn_enum apps_sys_dm0_access_restrict_posn;

#define APPS_SYS_DM0_ACCESS_RESTRICT_SDIO_HOST_MASK (0x00000002u)

enum apps_sys_interconnect_debug_control_posn_enum
{
   INTERCONNECT_DEBUG_MUX_SEL_LSB_POSN                = (int)0,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_INTERCONNECT_DEBUG_MUX_SEL_LSB_POSN = (int)0,
   INTERCONNECT_DEBUG_MUX_SEL_MSB_POSN                = (int)4,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_INTERCONNECT_DEBUG_MUX_SEL_MSB_POSN = (int)4,
   INTERCONNECT_DEBUG_ENABLE_POSN                     = (int)5,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_INTERCONNECT_DEBUG_ENABLE_LSB_POSN = (int)5,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_INTERCONNECT_DEBUG_ENABLE_MSB_POSN = (int)5,
   SHARED_RAM_DEBUG_MUX_SEL_LSB_POSN                  = (int)6,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_SHARED_RAM_DEBUG_MUX_SEL_LSB_POSN = (int)6,
   SHARED_RAM_DEBUG_MUX_SEL_MSB_POSN                  = (int)9,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_SHARED_RAM_DEBUG_MUX_SEL_MSB_POSN = (int)9,
   SHARED_RAM_DEBUG_ENABLE_POSN                       = (int)10,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_SHARED_RAM_DEBUG_ENABLE_LSB_POSN = (int)10,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_SHARED_RAM_DEBUG_ENABLE_MSB_POSN = (int)10,
   REMOTE_MASTER_DEBUG_MUX_SEL_LSB_POSN               = (int)11,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_REMOTE_MASTER_DEBUG_MUX_SEL_LSB_POSN = (int)11,
   REMOTE_MASTER_DEBUG_MUX_SEL_MSB_POSN               = (int)13,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_REMOTE_MASTER_DEBUG_MUX_SEL_MSB_POSN = (int)13,
   RAM_MASTER_MUX_DEBUG_ENABLE_POSN                   = (int)14,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_RAM_MASTER_MUX_DEBUG_ENABLE_LSB_POSN = (int)14,
   APPS_SYS_INTERCONNECT_DEBUG_CONTROL_RAM_MASTER_MUX_DEBUG_ENABLE_MSB_POSN = (int)14
};
typedef enum apps_sys_interconnect_debug_control_posn_enum apps_sys_interconnect_debug_control_posn;

#define INTERCONNECT_DEBUG_MUX_SEL_LSB_MASK      (0x00000001u)
#define INTERCONNECT_DEBUG_MUX_SEL_MSB_MASK      (0x00000010u)
#define INTERCONNECT_DEBUG_ENABLE_MASK           (0x00000020u)
#define SHARED_RAM_DEBUG_MUX_SEL_LSB_MASK        (0x00000040u)
#define SHARED_RAM_DEBUG_MUX_SEL_MSB_MASK        (0x00000200u)
#define SHARED_RAM_DEBUG_ENABLE_MASK             (0x00000400u)
#define REMOTE_MASTER_DEBUG_MUX_SEL_LSB_MASK     (0x00000800u)
#define REMOTE_MASTER_DEBUG_MUX_SEL_MSB_MASK     (0x00002000u)
#define RAM_MASTER_MUX_DEBUG_ENABLE_MASK         (0x00004000u)

enum apps_sys_sqif0_present_posn_enum
{
   APPS_SYS_SQIF0_PRESENT_FLASH_POSN                  = (int)0,
   APPS_SYS_SQIF0_PRESENT_APPS_SYS_SQIF0_PRESENT_FLASH_LSB_POSN = (int)0,
   APPS_SYS_SQIF0_PRESENT_APPS_SYS_SQIF0_PRESENT_FLASH_MSB_POSN = (int)0,
   APPS_SYS_SQIF0_PRESENT_SRAM_POSN                   = (int)1,
   APPS_SYS_SQIF0_PRESENT_APPS_SYS_SQIF0_PRESENT_SRAM_LSB_POSN = (int)1,
   APPS_SYS_SQIF0_PRESENT_APPS_SYS_SQIF0_PRESENT_SRAM_MSB_POSN = (int)1
};
typedef enum apps_sys_sqif0_present_posn_enum apps_sys_sqif0_present_posn;

#define APPS_SYS_SQIF0_PRESENT_FLASH_MASK        (0x00000001u)
#define APPS_SYS_SQIF0_PRESENT_SRAM_MASK         (0x00000002u)

enum apps_sys_sqif1_present_posn_enum
{
   APPS_SYS_SQIF1_PRESENT_FLASH_POSN                  = (int)0,
   APPS_SYS_SQIF1_PRESENT_APPS_SYS_SQIF1_PRESENT_FLASH_LSB_POSN = (int)0,
   APPS_SYS_SQIF1_PRESENT_APPS_SYS_SQIF1_PRESENT_FLASH_MSB_POSN = (int)0,
   APPS_SYS_SQIF1_PRESENT_SRAM_POSN                   = (int)1,
   APPS_SYS_SQIF1_PRESENT_APPS_SYS_SQIF1_PRESENT_SRAM_LSB_POSN = (int)1,
   APPS_SYS_SQIF1_PRESENT_APPS_SYS_SQIF1_PRESENT_SRAM_MSB_POSN = (int)1
};
typedef enum apps_sys_sqif1_present_posn_enum apps_sys_sqif1_present_posn;

#define APPS_SYS_SQIF1_PRESENT_FLASH_MASK        (0x00000001u)
#define APPS_SYS_SQIF1_PRESENT_SRAM_MASK         (0x00000002u)

enum apps_sys_tbus_window_en_posn_enum
{
   EN_0_POSN                                          = (int)0,
   APPS_SYS_TBUS_WINDOW_EN_EN_0_LSB_POSN              = (int)0,
   APPS_SYS_TBUS_WINDOW_EN_EN_0_MSB_POSN              = (int)0,
   EN_1_POSN                                          = (int)1,
   APPS_SYS_TBUS_WINDOW_EN_EN_1_LSB_POSN              = (int)1,
   APPS_SYS_TBUS_WINDOW_EN_EN_1_MSB_POSN              = (int)1
};
typedef enum apps_sys_tbus_window_en_posn_enum apps_sys_tbus_window_en_posn;

#define EN_0_MASK                                (0x00000001u)
#define EN_1_MASK                                (0x00000002u)

enum apps_sys_tbus_window_ids_posn_enum
{
   MASTER_0_LSB_POSN                                  = (int)0,
   APPS_SYS_TBUS_WINDOW_IDS_MASTER_0_LSB_POSN         = (int)0,
   MASTER_0_MSB_POSN                                  = (int)3,
   APPS_SYS_TBUS_WINDOW_IDS_MASTER_0_MSB_POSN         = (int)3,
   MASTER_1_LSB_POSN                                  = (int)4,
   APPS_SYS_TBUS_WINDOW_IDS_MASTER_1_LSB_POSN         = (int)4,
   MASTER_1_MSB_POSN                                  = (int)7,
   APPS_SYS_TBUS_WINDOW_IDS_MASTER_1_MSB_POSN         = (int)7
};
typedef enum apps_sys_tbus_window_ids_posn_enum apps_sys_tbus_window_ids_posn;

#define MASTER_0_LSB_MASK                        (0x00000001u)
#define MASTER_0_MSB_MASK                        (0x00000008u)
#define MASTER_1_LSB_MASK                        (0x00000010u)
#define MASTER_1_MSB_MASK                        (0x00000080u)

enum apps_sys_tcm_lock_posn_enum
{
   TCM0_POSN                                          = (int)0,
   APPS_SYS_TCM_LOCK_TCM0_LSB_POSN                    = (int)0,
   APPS_SYS_TCM_LOCK_TCM0_MSB_POSN                    = (int)0,
   TCM1_POSN                                          = (int)1,
   APPS_SYS_TCM_LOCK_TCM1_LSB_POSN                    = (int)1,
   APPS_SYS_TCM_LOCK_TCM1_MSB_POSN                    = (int)1
};
typedef enum apps_sys_tcm_lock_posn_enum apps_sys_tcm_lock_posn;

#define TCM0_MASK                                (0x00000001u)
#define TCM1_MASK                                (0x00000002u)

enum sub_sys_debug_select_shift_posn_enum
{
   SUB_SYS_DEBUG_SELECT_SHIFT_LOW_POSN                = (int)0,
   SUB_SYS_DEBUG_SELECT_SHIFT_SUB_SYS_DEBUG_SELECT_SHIFT_LOW_LSB_POSN = (int)0,
   SUB_SYS_DEBUG_SELECT_SHIFT_SUB_SYS_DEBUG_SELECT_SHIFT_LOW_MSB_POSN = (int)0,
   SUB_SYS_DEBUG_SELECT_SHIFT_HIGH_POSN               = (int)1,
   SUB_SYS_DEBUG_SELECT_SHIFT_SUB_SYS_DEBUG_SELECT_SHIFT_HIGH_LSB_POSN = (int)1,
   SUB_SYS_DEBUG_SELECT_SHIFT_SUB_SYS_DEBUG_SELECT_SHIFT_HIGH_MSB_POSN = (int)1
};
typedef enum sub_sys_debug_select_shift_posn_enum sub_sys_debug_select_shift_posn;

#define SUB_SYS_DEBUG_SELECT_SHIFT_LOW_MASK      (0x00000001u)
#define SUB_SYS_DEBUG_SELECT_SHIFT_HIGH_MASK     (0x00000002u)

enum apps_sys_mux_enum
{
   APPS_SYS_MUX_REMOTE_VM_LOOKUP_IN         = (int)0x0,
   APPS_SYS_MUX_REMOTE_DATA_IN              = (int)0x1,
   APPS_SYS_MUX_INTERRUPTS                  = (int)0x2,
   APPS_SYS_MUX_MESSAGES                    = (int)0x3,
   APPS_SYS_MUX_MISC                        = (int)0x4,
   APPS_SYS_MUX_REMOTE_ACCESS_OUT           = (int)0x5,
   APPS_SYS_MUX_REMOTE_VM_ACCESS_CPU0_OUT   = (int)0x6,
   APPS_SYS_MUX_DMAC                        = (int)0x7,
   APPS_SYS_MUX_MISC_PROC                   = (int)0x8,
   APPS_SYS_MUX_DEBUG                       = (int)0xF,
   APPS_SYS_MUX_NUM_QUEUES                  = (int)0x9,
   MAX_APPS_SYS_MUX                         = (int)0xF
};
typedef enum apps_sys_mux_enum apps_sys_mux;


enum apps_sys_mux_connected_enum
{
   APPS_SYS_MUX_CONNECTED_BLOCKS            = (int)0x1FF,
   MAX_APPS_SYS_MUX_CONNECTED               = (int)0x1FF
};
typedef enum apps_sys_mux_connected_enum apps_sys_mux_connected;


enum apps_sys_cache_sel_posn_enum
{
   APPS_SYS_CACHE_SEL_POSN                            = (int)0,
   APPS_SYS_CACHE_SEL_APPS_SYS_CACHE_SEL_LSB_POSN     = (int)0,
   APPS_SYS_CACHE_SEL_APPS_SYS_CACHE_SEL_MSB_POSN     = (int)0
};
typedef enum apps_sys_cache_sel_posn_enum apps_sys_cache_sel_posn;

#define APPS_SYS_CACHE_SEL_MASK                  (0x00000001u)

enum apps_sys_sqif_sel_posn_enum
{
   APPS_SYS_SQIF_SEL_POSN                             = (int)0,
   APPS_SYS_SQIF_SEL_APPS_SYS_SQIF_SEL_LSB_POSN       = (int)0,
   APPS_SYS_SQIF_SEL_APPS_SYS_SQIF_SEL_MSB_POSN       = (int)0
};
typedef enum apps_sys_sqif_sel_posn_enum apps_sys_sqif_sel_posn;

#define APPS_SYS_SQIF_SEL_MASK                   (0x00000001u)

enum apps_sys_test_reg_posn_enum
{
   APPS_SYS_TEST_REG_LSB_POSN                         = (int)0,
   APPS_SYS_TEST_REG_APPS_SYS_TEST_REG_LSB_POSN       = (int)0,
   APPS_SYS_TEST_REG_MSB_POSN                         = (int)31,
   APPS_SYS_TEST_REG_APPS_SYS_TEST_REG_MSB_POSN       = (int)31
};
typedef enum apps_sys_test_reg_posn_enum apps_sys_test_reg_posn;

#define APPS_SYS_TEST_REG_LSB_MASK               (0x00000001u)
#define APPS_SYS_TEST_REG_MSB_MASK               (0x80000000u)

enum ram_sequencer_active_status_posn_enum
{
   RAM_SEQUENCER_ACTIVE_STATUS_LSB_POSN               = (int)0,
   RAM_SEQUENCER_ACTIVE_STATUS_RAM_SEQUENCER_ACTIVE_STATUS_LSB_POSN = (int)0,
   RAM_SEQUENCER_ACTIVE_STATUS_MSB_POSN               = (int)10,
   RAM_SEQUENCER_ACTIVE_STATUS_RAM_SEQUENCER_ACTIVE_STATUS_MSB_POSN = (int)10
};
typedef enum ram_sequencer_active_status_posn_enum ram_sequencer_active_status_posn;

#define RAM_SEQUENCER_ACTIVE_STATUS_LSB_MASK     (0x00000001u)
#define RAM_SEQUENCER_ACTIVE_STATUS_MSB_MASK     (0x00000400u)

enum sub_sys_debug_status_posn_enum
{
   SUB_SYS_DEBUG_STATUS_LSB_POSN                      = (int)0,
   SUB_SYS_DEBUG_STATUS_SUB_SYS_DEBUG_STATUS_LSB_POSN = (int)0,
   SUB_SYS_DEBUG_STATUS_MSB_POSN                      = (int)31,
   SUB_SYS_DEBUG_STATUS_SUB_SYS_DEBUG_STATUS_MSB_POSN = (int)31
};
typedef enum sub_sys_debug_status_posn_enum sub_sys_debug_status_posn;

#define SUB_SYS_DEBUG_STATUS_LSB_MASK            (0x00000001u)
#define SUB_SYS_DEBUG_STATUS_MSB_MASK            (0x80000000u)

enum sub_sys_id_posn_enum
{
   SUB_SYS_ID_LSB_POSN                                = (int)0,
   SUB_SYS_ID_SUB_SYS_ID_LSB_POSN                     = (int)0,
   SUB_SYS_ID_MSB_POSN                                = (int)3,
   SUB_SYS_ID_SUB_SYS_ID_MSB_POSN                     = (int)3
};
typedef enum sub_sys_id_posn_enum sub_sys_id_posn;

#define SUB_SYS_ID_LSB_MASK                      (0x00000001u)
#define SUB_SYS_ID_MSB_MASK                      (0x00000008u)

enum sub_sys_reg_src_hash_posn_enum
{
   SUB_SYS_REG_SRC_HASH_LSB_POSN                      = (int)0,
   SUB_SYS_REG_SRC_HASH_SUB_SYS_REG_SRC_HASH_LSB_POSN = (int)0,
   SUB_SYS_REG_SRC_HASH_MSB_POSN                      = (int)15,
   SUB_SYS_REG_SRC_HASH_SUB_SYS_REG_SRC_HASH_MSB_POSN = (int)15
};
typedef enum sub_sys_reg_src_hash_posn_enum sub_sys_reg_src_hash_posn;

#define SUB_SYS_REG_SRC_HASH_LSB_MASK            (0x00000001u)
#define SUB_SYS_REG_SRC_HASH_MSB_MASK            (0x00008000u)

enum sub_sys_rst_status_posn_enum
{
   SUB_SYS_RST_STATUS_LSB_POSN                        = (int)0,
   SUB_SYS_RST_STATUS_SUB_SYS_RST_STATUS_LSB_POSN     = (int)0,
   SUB_SYS_RST_STATUS_MSB_POSN                        = (int)2,
   SUB_SYS_RST_STATUS_SUB_SYS_RST_STATUS_MSB_POSN     = (int)2
};
typedef enum sub_sys_rst_status_posn_enum sub_sys_rst_status_posn;

#define SUB_SYS_RST_STATUS_LSB_MASK              (0x00000001u)
#define SUB_SYS_RST_STATUS_MSB_MASK              (0x00000004u)

enum sub_sys_transaction_mux_dbg_sel_posn_enum
{
   SUB_SYS_TRANSACTION_MUX_DBG_SEL_LSB_POSN           = (int)0,
   SUB_SYS_TRANSACTION_MUX_DBG_SEL_SUB_SYS_TRANSACTION_MUX_DBG_SEL_LSB_POSN = (int)0,
   SUB_SYS_TRANSACTION_MUX_DBG_SEL_MSB_POSN           = (int)5,
   SUB_SYS_TRANSACTION_MUX_DBG_SEL_SUB_SYS_TRANSACTION_MUX_DBG_SEL_MSB_POSN = (int)5
};
typedef enum sub_sys_transaction_mux_dbg_sel_posn_enum sub_sys_transaction_mux_dbg_sel_posn;

#define SUB_SYS_TRANSACTION_MUX_DBG_SEL_LSB_MASK (0x00000001u)
#define SUB_SYS_TRANSACTION_MUX_DBG_SEL_MSB_MASK (0x00000020u)

enum apps_sys_sys__access_ctrl_enum_posn_enum
{
   APPS_SYS_SYS__P0_ACCESS_PERMISSION_POSN            = (int)0,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_SYS__P1_ACCESS_PERMISSION_POSN            = (int)1,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_SYS__P2_ACCESS_PERMISSION_POSN            = (int)2,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_SYS__P3_ACCESS_PERMISSION_POSN            = (int)3,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_SYS__ACCESS_CTRL_ENUM_APPS_SYS_SYS__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_sys__access_ctrl_enum_posn_enum apps_sys_sys__access_ctrl_enum_posn;

#define APPS_SYS_SYS__P0_ACCESS_PERMISSION_MASK  (0x00000001u)
#define APPS_SYS_SYS__P1_ACCESS_PERMISSION_MASK  (0x00000002u)
#define APPS_SYS_SYS__P2_ACCESS_PERMISSION_MASK  (0x00000004u)
#define APPS_SYS_SYS__P3_ACCESS_PERMISSION_MASK  (0x00000008u)

enum apps_sys_sys__p0_access_permission_enum
{
   APPS_SYS_SYS__P0_ACCESS_BLOCKED          = (int)0x0,
   APPS_SYS_SYS__P0_ACCESS_UNBLOCKED        = (int)0x1,
   MAX_APPS_SYS_SYS__P0_ACCESS_PERMISSION   = (int)0x1
};
#define NUM_APPS_SYS_SYS__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sys__p0_access_permission_enum apps_sys_sys__p0_access_permission;


enum apps_sys_sys__p1_access_permission_enum
{
   APPS_SYS_SYS__P1_ACCESS_BLOCKED          = (int)0x0,
   APPS_SYS_SYS__P1_ACCESS_UNBLOCKED        = (int)0x1,
   MAX_APPS_SYS_SYS__P1_ACCESS_PERMISSION   = (int)0x1
};
#define NUM_APPS_SYS_SYS__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sys__p1_access_permission_enum apps_sys_sys__p1_access_permission;


enum apps_sys_sys__p2_access_permission_enum
{
   APPS_SYS_SYS__P2_ACCESS_BLOCKED          = (int)0x0,
   APPS_SYS_SYS__P2_ACCESS_UNBLOCKED        = (int)0x1,
   MAX_APPS_SYS_SYS__P2_ACCESS_PERMISSION   = (int)0x1
};
#define NUM_APPS_SYS_SYS__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sys__p2_access_permission_enum apps_sys_sys__p2_access_permission;


enum apps_sys_sys__p3_access_permission_enum
{
   APPS_SYS_SYS__P3_ACCESS_BLOCKED          = (int)0x0,
   APPS_SYS_SYS__P3_ACCESS_UNBLOCKED        = (int)0x1,
   MAX_APPS_SYS_SYS__P3_ACCESS_PERMISSION   = (int)0x1
};
#define NUM_APPS_SYS_SYS__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_sys__p3_access_permission_enum apps_sys_sys__p3_access_permission;


enum apps_sys_sys__mutex_lock_enum_enum
{
   APPS_SYS_SYS__MUTEX_AVAILABLE            = (int)0x0,
   APPS_SYS_SYS__MUTEX_CLAIMED_BY_P0        = (int)0x1,
   APPS_SYS_SYS__MUTEX_CLAIMED_BY_P1        = (int)0x2,
   APPS_SYS_SYS__MUTEX_CLAIMED_BY_P2        = (int)0x4,
   APPS_SYS_SYS__MUTEX_CLAIMED_BY_P3        = (int)0x8,
   APPS_SYS_SYS__MUTEX_DISABLED             = (int)0xF,
   MAX_APPS_SYS_SYS__MUTEX_LOCK_ENUM        = (int)0xF
};
typedef enum apps_sys_sys__mutex_lock_enum_enum apps_sys_sys__mutex_lock_enum;


enum apps_sys_sys__apps_sys_sqif_window_offset_index_enum
{
   APPS_SYS_SYS__SQIF0_CACHE0_WINDOW_OFFSET = (int)0x0,
   APPS_SYS_SYS__SQIF0_CACHE1_WINDOW_OFFSET = (int)0x1,
   APPS_SYS_SYS__SQIF0_DMAC_WINDOW_OFFSET   = (int)0x2,
   APPS_SYS_SYS__SQIF0_SPI_RAM_WINDOW_OFFSET = (int)0x3,
   APPS_SYS_SYS__SQIF0_UNCACHED_CPU0_WINDOW_OFFSET = (int)0x4,
   APPS_SYS_SYS__SQIF0_UNCACHED_CPU1_WINDOW_OFFSET = (int)0x5,
   APPS_SYS_SYS__SQIF0_TBUS0_WINDOW_OFFSET  = (int)0x6,
   APPS_SYS_SYS__SQIF0_TBUS1_WINDOW_OFFSET  = (int)0x7,
   APPS_SYS_SYS__SQIF0_DEBUG_WINDOW_OFFSET  = (int)0x8,
   APPS_SYS_SYS__SQIF_NUM_WINDOW_OFFSETS    = (int)0x9,
   APPS_SYS_SYS__SQIF1_CACHE0_WINDOW_OFFSET = (int)0x9,
   APPS_SYS_SYS__SQIF1_CACHE1_WINDOW_OFFSET = (int)0xA,
   APPS_SYS_SYS__SQIF1_DMAC_WINDOW_OFFSET   = (int)0xB,
   APPS_SYS_SYS__SQIF1_SPI_RAM_WINDOW_OFFSET = (int)0xC,
   APPS_SYS_SYS__SQIF1_UNCACHED_CPU0_WINDOW_OFFSET = (int)0xD,
   APPS_SYS_SYS__SQIF1_UNCACHED_CPU1_WINDOW_OFFSET = (int)0xE,
   APPS_SYS_SYS__SQIF1_TBUS0_WINDOW_OFFSET  = (int)0xF,
   APPS_SYS_SYS__SQIF1_TBUS1_WINDOW_OFFSET  = (int)0x10,
   APPS_SYS_SYS__SQIF1_DEBUG_WINDOW_OFFSET  = (int)0x11,
   MAX_APPS_SYS_SYS__APPS_SYS_SQIF_WINDOW_OFFSET_INDEX = (int)0x11
};
typedef enum apps_sys_sys__apps_sys_sqif_window_offset_index_enum apps_sys_sys__apps_sys_sqif_window_offset_index;


enum apps_sys_cpu0_tcm_remap_controls_posn_enum
{
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_LSB_POSN = (int)0,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_MSB_POSN = (int)0,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_LSB_POSN = (int)1,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_MSB_POSN = (int)1,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_LSB_POSN = (int)2,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_MSB_POSN = (int)2,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_LSB_POSN = (int)3,
   APPS_SYS_CPU0_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_MSB_POSN = (int)3
};
typedef enum apps_sys_cpu0_tcm_remap_controls_posn_enum apps_sys_cpu0_tcm_remap_controls_posn;


enum apps_sys_cpu1_tcm_remap_controls_posn_enum
{
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_LSB_POSN = (int)0,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_SELECT_MSB_POSN = (int)0,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_LSB_POSN = (int)1,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_LOW_ENABLE_MSB_POSN = (int)1,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_LSB_POSN = (int)2,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_SELECT_MSB_POSN = (int)2,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_LSB_POSN = (int)3,
   APPS_SYS_CPU1_TCM_REMAP_CONTROLS_TCM_REMAP_HIGH_ENABLE_MSB_POSN = (int)3
};
typedef enum apps_sys_cpu1_tcm_remap_controls_posn_enum apps_sys_cpu1_tcm_remap_controls_posn;


enum sub_sys_chip_version_posn_enum
{
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_MAJOR_LSB_POSN   = (int)0,
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_MAJOR_MSB_POSN   = (int)7,
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_VARIANT_LSB_POSN = (int)8,
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_VARIANT_MSB_POSN = (int)11,
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_MINOR_LSB_POSN   = (int)12,
   SUB_SYS_CHIP_VERSION_CHIP_VERSION_MINOR_MSB_POSN   = (int)15
};
typedef enum sub_sys_chip_version_posn_enum sub_sys_chip_version_posn;


enum apps_sys_access_ctrl_posn_enum
{
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_ACCESS_CTRL_APPS_SYS_SYS__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_access_ctrl_posn_enum apps_sys_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_APPS_SYS_SYS */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_SYS */

#if defined(IO_DEFS_MODULE_K32_MONITOR) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_MONITOR
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_MONITOR

/* -- k32_monitor -- Kalimba 32-bit Monitor Control registers. -- */

enum exception_en_posn_enum
{
   EXCEPTION_EN_BREAK_POSN                            = (int)0,
   EXCEPTION_EN_EXCEPTION_EN_BREAK_LSB_POSN           = (int)0,
   EXCEPTION_EN_EXCEPTION_EN_BREAK_MSB_POSN           = (int)0,
   EXCEPTION_EN_IRQ_POSN                              = (int)1,
   EXCEPTION_EN_EXCEPTION_EN_IRQ_LSB_POSN             = (int)1,
   EXCEPTION_EN_EXCEPTION_EN_IRQ_MSB_POSN             = (int)1
};
typedef enum exception_en_posn_enum exception_en_posn;

#define EXCEPTION_EN_BREAK_MASK                  (0x00000001u)
#define EXCEPTION_EN_IRQ_MASK                    (0x00000002u)

enum exception_type_enum
{
   EXCEPTION_TYPE_NONE                      = (int)0x0,
   EXCEPTION_TYPE_DM1_UNALIGNED_32BIT       = (int)0x1,
   EXCEPTION_TYPE_DM1_UNALIGNED_16BIT       = (int)0x2,
   EXCEPTION_TYPE_DM1_UNMAPPED              = (int)0x3,
   EXCEPTION_TYPE_DM2_UNALIGNED_32BIT       = (int)0x4,
   EXCEPTION_TYPE_DM2_UNALIGNED_16BIT       = (int)0x5,
   EXCEPTION_TYPE_DM2_UNMAPPED              = (int)0x6,
   EXCEPTION_TYPE_PM_UNMAPPED               = (int)0x7,
   EXCEPTION_TYPE_PM_PROG_REGION            = (int)0x8,
   EXCEPTION_TYPE_DM1_PROG_REGION           = (int)0x9,
   EXCEPTION_TYPE_DM2_PROG_REGION           = (int)0xA,
   EXCEPTION_TYPE_STACK_OVERFLOW            = (int)0xB,
   EXCEPTION_TYPE_OTHER                     = (int)0xC,
   EXCEPTION_TYPE_PM_OUT_OF_BOUNDS          = (int)0xD,
   MAX_EXCEPTION_TYPE                       = (int)0xD
};
#define NUM_EXCEPTION_TYPE (0xE)
typedef enum exception_type_enum exception_type;


enum prog_exception_region_enable_posn_enum
{
   PM_PROG_EXCEPTION_REGION_ENABLE_POSN               = (int)0,
   PROG_EXCEPTION_REGION_ENABLE_PM_PROG_EXCEPTION_REGION_ENABLE_LSB_POSN = (int)0,
   PROG_EXCEPTION_REGION_ENABLE_PM_PROG_EXCEPTION_REGION_ENABLE_MSB_POSN = (int)0,
   DM1_PROG_EXCEPTION_REGION_ENABLE_POSN              = (int)1,
   PROG_EXCEPTION_REGION_ENABLE_DM1_PROG_EXCEPTION_REGION_ENABLE_LSB_POSN = (int)1,
   PROG_EXCEPTION_REGION_ENABLE_DM1_PROG_EXCEPTION_REGION_ENABLE_MSB_POSN = (int)1,
   DM2_PROG_EXCEPTION_REGION_ENABLE_POSN              = (int)2,
   PROG_EXCEPTION_REGION_ENABLE_DM2_PROG_EXCEPTION_REGION_ENABLE_LSB_POSN = (int)2,
   PROG_EXCEPTION_REGION_ENABLE_DM2_PROG_EXCEPTION_REGION_ENABLE_MSB_POSN = (int)2,
   PM_PROG_EXCEPTION_OOB_ENABLE_POSN                  = (int)3,
   PROG_EXCEPTION_REGION_ENABLE_PM_PROG_EXCEPTION_OOB_ENABLE_LSB_POSN = (int)3,
   PROG_EXCEPTION_REGION_ENABLE_PM_PROG_EXCEPTION_OOB_ENABLE_MSB_POSN = (int)3
};
typedef enum prog_exception_region_enable_posn_enum prog_exception_region_enable_posn;

#define PM_PROG_EXCEPTION_REGION_ENABLE_MASK     (0x00000001u)
#define DM1_PROG_EXCEPTION_REGION_ENABLE_MASK    (0x00000002u)
#define DM2_PROG_EXCEPTION_REGION_ENABLE_MASK    (0x00000004u)
#define PM_PROG_EXCEPTION_OOB_ENABLE_MASK        (0x00000008u)

enum dm1_prog_exception_region_end_addr_posn_enum
{
   DM1_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN        = (int)0,
   DM1_PROG_EXCEPTION_REGION_END_ADDR_DM1_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN = (int)0,
   DM1_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN        = (int)31,
   DM1_PROG_EXCEPTION_REGION_END_ADDR_DM1_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN = (int)31
};
typedef enum dm1_prog_exception_region_end_addr_posn_enum dm1_prog_exception_region_end_addr_posn;

#define DM1_PROG_EXCEPTION_REGION_END_ADDR_LSB_MASK (0x00000001u)
#define DM1_PROG_EXCEPTION_REGION_END_ADDR_MSB_MASK (0x80000000u)

enum dm1_prog_exception_region_start_addr_posn_enum
{
   DM1_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN      = (int)0,
   DM1_PROG_EXCEPTION_REGION_START_ADDR_DM1_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN = (int)0,
   DM1_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN      = (int)31,
   DM1_PROG_EXCEPTION_REGION_START_ADDR_DM1_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN = (int)31
};
typedef enum dm1_prog_exception_region_start_addr_posn_enum dm1_prog_exception_region_start_addr_posn;

#define DM1_PROG_EXCEPTION_REGION_START_ADDR_LSB_MASK (0x00000001u)
#define DM1_PROG_EXCEPTION_REGION_START_ADDR_MSB_MASK (0x80000000u)

enum dm2_prog_exception_region_end_addr_posn_enum
{
   DM2_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN        = (int)0,
   DM2_PROG_EXCEPTION_REGION_END_ADDR_DM2_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN = (int)0,
   DM2_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN        = (int)31,
   DM2_PROG_EXCEPTION_REGION_END_ADDR_DM2_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN = (int)31
};
typedef enum dm2_prog_exception_region_end_addr_posn_enum dm2_prog_exception_region_end_addr_posn;

#define DM2_PROG_EXCEPTION_REGION_END_ADDR_LSB_MASK (0x00000001u)
#define DM2_PROG_EXCEPTION_REGION_END_ADDR_MSB_MASK (0x80000000u)

enum dm2_prog_exception_region_start_addr_posn_enum
{
   DM2_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN      = (int)0,
   DM2_PROG_EXCEPTION_REGION_START_ADDR_DM2_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN = (int)0,
   DM2_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN      = (int)31,
   DM2_PROG_EXCEPTION_REGION_START_ADDR_DM2_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN = (int)31
};
typedef enum dm2_prog_exception_region_start_addr_posn_enum dm2_prog_exception_region_start_addr_posn;

#define DM2_PROG_EXCEPTION_REGION_START_ADDR_LSB_MASK (0x00000001u)
#define DM2_PROG_EXCEPTION_REGION_START_ADDR_MSB_MASK (0x80000000u)

enum exception_pc_posn_enum
{
   EXCEPTION_PC_LSB_POSN                              = (int)0,
   EXCEPTION_PC_EXCEPTION_PC_LSB_POSN                 = (int)0,
   EXCEPTION_PC_MSB_POSN                              = (int)31,
   EXCEPTION_PC_EXCEPTION_PC_MSB_POSN                 = (int)31
};
typedef enum exception_pc_posn_enum exception_pc_posn;

#define EXCEPTION_PC_LSB_MASK                    (0x00000001u)
#define EXCEPTION_PC_MSB_MASK                    (0x80000000u)

enum pm_prog_exception_region_end_addr_posn_enum
{
   PM_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN         = (int)0,
   PM_PROG_EXCEPTION_REGION_END_ADDR_PM_PROG_EXCEPTION_REGION_END_ADDR_LSB_POSN = (int)0,
   PM_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN         = (int)31,
   PM_PROG_EXCEPTION_REGION_END_ADDR_PM_PROG_EXCEPTION_REGION_END_ADDR_MSB_POSN = (int)31
};
typedef enum pm_prog_exception_region_end_addr_posn_enum pm_prog_exception_region_end_addr_posn;

#define PM_PROG_EXCEPTION_REGION_END_ADDR_LSB_MASK (0x00000001u)
#define PM_PROG_EXCEPTION_REGION_END_ADDR_MSB_MASK (0x80000000u)

enum pm_prog_exception_region_start_addr_posn_enum
{
   PM_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN       = (int)0,
   PM_PROG_EXCEPTION_REGION_START_ADDR_PM_PROG_EXCEPTION_REGION_START_ADDR_LSB_POSN = (int)0,
   PM_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN       = (int)31,
   PM_PROG_EXCEPTION_REGION_START_ADDR_PM_PROG_EXCEPTION_REGION_START_ADDR_MSB_POSN = (int)31
};
typedef enum pm_prog_exception_region_start_addr_posn_enum pm_prog_exception_region_start_addr_posn;

#define PM_PROG_EXCEPTION_REGION_START_ADDR_LSB_MASK (0x00000001u)
#define PM_PROG_EXCEPTION_REGION_START_ADDR_MSB_MASK (0x80000000u)

#endif /* IO_DEFS_MODULE_K32_MONITOR */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_MONITOR */

#if defined(IO_DEFS_MODULE_LED_CTRL) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_LED_CTRL
#define __IO_DEFS_H__IO_DEFS_MODULE_LED_CTRL

/* -- led_ctrl -- Control registers for LED controllers -- */

enum led_instance_posn_enum
{
   LED0_POSN                                          = (int)0,
   LED_INSTANCE_LED0_LSB_POSN                         = (int)0,
   LED_INSTANCE_LED0_MSB_POSN                         = (int)0,
   LED1_POSN                                          = (int)1,
   LED_INSTANCE_LED1_LSB_POSN                         = (int)1,
   LED_INSTANCE_LED1_MSB_POSN                         = (int)1,
   LED2_POSN                                          = (int)2,
   LED_INSTANCE_LED2_LSB_POSN                         = (int)2,
   LED_INSTANCE_LED2_MSB_POSN                         = (int)2,
   LED3_POSN                                          = (int)3,
   LED_INSTANCE_LED3_LSB_POSN                         = (int)3,
   LED_INSTANCE_LED3_MSB_POSN                         = (int)3,
   LED4_POSN                                          = (int)4,
   LED_INSTANCE_LED4_LSB_POSN                         = (int)4,
   LED_INSTANCE_LED4_MSB_POSN                         = (int)4,
   LED5_POSN                                          = (int)5,
   LED_INSTANCE_LED5_LSB_POSN                         = (int)5,
   LED_INSTANCE_LED5_MSB_POSN                         = (int)5
};
typedef enum led_instance_posn_enum led_instance_posn;

#define LED0_MASK                                (0x00000001u)
#define LED1_MASK                                (0x00000002u)
#define LED2_MASK                                (0x00000004u)
#define LED3_MASK                                (0x00000008u)
#define LED4_MASK                                (0x00000010u)
#define LED5_MASK                                (0x00000020u)

enum led_pin_config_enum_enum
{
   LED_PIN_CONFIG_PUSH_PULL                 = (int)0x0,
   LED_PIN_CONFIG_OPEN_DRAIN                = (int)0x1,
   LED_PIN_CONFIG_OPEN_SOURCE               = (int)0x2,
   LED_PIN_CONFIG_PUSH_PULL_INVERTED        = (int)0x3,
   MAX_LED_PIN_CONFIG_ENUM                  = (int)0x3
};
#define NUM_LED_PIN_CONFIG_ENUM (0x4)
typedef enum led_pin_config_enum_enum led_pin_config_enum;


enum led_start_up_state_enum_enum
{
   LED_START_UP_STATE_COUNT_LOW_HOLD_MIN    = (int)0x0,
   LED_START_UP_STATE_COUNT_HIGH_HOLD_MIN   = (int)0x1,
   LED_START_UP_STATE_COUNT_LOW_RAMP_UP     = (int)0x2,
   LED_START_UP_STATE_COUNT_HIGH_RAMP_UP    = (int)0x3,
   LED_START_UP_STATE_COUNT_LOW_HOLD_MAX    = (int)0x4,
   LED_START_UP_STATE_COUNT_HIGH_HOLD_MAX   = (int)0x5,
   LED_START_UP_STATE_COUNT_LOW_RAMP_DOWN   = (int)0x6,
   LED_START_UP_STATE_COUNT_HIGH_RAMP_DOWN  = (int)0x7,
   MAX_LED_START_UP_STATE_ENUM              = (int)0x7
};
#define NUM_LED_START_UP_STATE_ENUM (0x8)
typedef enum led_start_up_state_enum_enum led_start_up_state_enum;


enum led_configure_posn_enum
{
   LED_CONFIGURE_POSN                                 = (int)0,
   LED_CONFIGURE_LED_CONFIGURE_LSB_POSN               = (int)0,
   LED_CONFIGURE_LED_CONFIGURE_MSB_POSN               = (int)0
};
typedef enum led_configure_posn_enum led_configure_posn;

#define LED_CONFIGURE_MASK                       (0x00000001u)

enum led_counthold_value_posn_enum
{
   LED_COUNTHOLD_VALUE_LSB_POSN                       = (int)0,
   LED_COUNTHOLD_VALUE_LED_COUNTHOLD_VALUE_LSB_POSN   = (int)0,
   LED_COUNTHOLD_VALUE_MSB_POSN                       = (int)15,
   LED_COUNTHOLD_VALUE_LED_COUNTHOLD_VALUE_MSB_POSN   = (int)15
};
typedef enum led_counthold_value_posn_enum led_counthold_value_posn;

#define LED_COUNTHOLD_VALUE_LSB_MASK             (0x00000001u)
#define LED_COUNTHOLD_VALUE_MSB_MASK             (0x00008000u)

enum led_ctrl_clk_enable_posn_enum
{
   LED_CTRL_CLK_ENABLE_POSN                           = (int)0,
   LED_CTRL_CLK_ENABLE_LED_CTRL_CLK_ENABLE_LSB_POSN   = (int)0,
   LED_CTRL_CLK_ENABLE_LED_CTRL_CLK_ENABLE_MSB_POSN   = (int)0
};
typedef enum led_ctrl_clk_enable_posn_enum led_ctrl_clk_enable_posn;

#define LED_CTRL_CLK_ENABLE_MASK                 (0x00000001u)

enum led_ctrl_soft_reset_posn_enum
{
   LED_CTRL_SOFT_RESET_POSN                           = (int)0,
   LED_CTRL_SOFT_RESET_LED_CTRL_SOFT_RESET_LSB_POSN   = (int)0,
   LED_CTRL_SOFT_RESET_LED_CTRL_SOFT_RESET_MSB_POSN   = (int)0
};
typedef enum led_ctrl_soft_reset_posn_enum led_ctrl_soft_reset_posn;

#define LED_CTRL_SOFT_RESET_MASK                 (0x00000001u)

enum led_debug_select_posn_enum
{
   LED_DEBUG_SELECT_LSB_POSN                          = (int)0,
   LED_DEBUG_SELECT_LED_DEBUG_SELECT_LSB_POSN         = (int)0,
   LED_DEBUG_SELECT_MSB_POSN                          = (int)3,
   LED_DEBUG_SELECT_LED_DEBUG_SELECT_MSB_POSN         = (int)3
};
typedef enum led_debug_select_posn_enum led_debug_select_posn;

#define LED_DEBUG_SELECT_LSB_MASK                (0x00000001u)
#define LED_DEBUG_SELECT_MSB_MASK                (0x00000008u)

enum led_hold_high_config_posn_enum
{
   LED_HOLD_HIGH_CONFIG_LSB_POSN                      = (int)0,
   LED_HOLD_HIGH_CONFIG_LED_HOLD_HIGH_CONFIG_LSB_POSN = (int)0,
   LED_HOLD_HIGH_CONFIG_MSB_POSN                      = (int)15,
   LED_HOLD_HIGH_CONFIG_LED_HOLD_HIGH_CONFIG_MSB_POSN = (int)15
};
typedef enum led_hold_high_config_posn_enum led_hold_high_config_posn;

#define LED_HOLD_HIGH_CONFIG_LSB_MASK            (0x00000001u)
#define LED_HOLD_HIGH_CONFIG_MSB_MASK            (0x00008000u)

enum led_hold_high_config_status_posn_enum
{
   LED_HOLD_HIGH_CONFIG_STATUS_LSB_POSN               = (int)0,
   LED_HOLD_HIGH_CONFIG_STATUS_LED_HOLD_HIGH_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_HOLD_HIGH_CONFIG_STATUS_MSB_POSN               = (int)15,
   LED_HOLD_HIGH_CONFIG_STATUS_LED_HOLD_HIGH_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_hold_high_config_status_posn_enum led_hold_high_config_status_posn;

#define LED_HOLD_HIGH_CONFIG_STATUS_LSB_MASK     (0x00000001u)
#define LED_HOLD_HIGH_CONFIG_STATUS_MSB_MASK     (0x00008000u)

enum led_hold_low_config_posn_enum
{
   LED_HOLD_LOW_CONFIG_LSB_POSN                       = (int)0,
   LED_HOLD_LOW_CONFIG_LED_HOLD_LOW_CONFIG_LSB_POSN   = (int)0,
   LED_HOLD_LOW_CONFIG_MSB_POSN                       = (int)15,
   LED_HOLD_LOW_CONFIG_LED_HOLD_LOW_CONFIG_MSB_POSN   = (int)15
};
typedef enum led_hold_low_config_posn_enum led_hold_low_config_posn;

#define LED_HOLD_LOW_CONFIG_LSB_MASK             (0x00000001u)
#define LED_HOLD_LOW_CONFIG_MSB_MASK             (0x00008000u)

enum led_hold_low_config_status_posn_enum
{
   LED_HOLD_LOW_CONFIG_STATUS_LSB_POSN                = (int)0,
   LED_HOLD_LOW_CONFIG_STATUS_LED_HOLD_LOW_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_HOLD_LOW_CONFIG_STATUS_MSB_POSN                = (int)15,
   LED_HOLD_LOW_CONFIG_STATUS_LED_HOLD_LOW_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_hold_low_config_status_posn_enum led_hold_low_config_status_posn;

#define LED_HOLD_LOW_CONFIG_STATUS_LSB_MASK      (0x00000001u)
#define LED_HOLD_LOW_CONFIG_STATUS_MSB_MASK      (0x00008000u)

enum led_index_posn_enum
{
   LED_INDEX_LSB_POSN                                 = (int)0,
   LED_INDEX_LED_INDEX_LSB_POSN                       = (int)0,
   LED_INDEX_MSB_POSN                                 = (int)3,
   LED_INDEX_LED_INDEX_MSB_POSN                       = (int)3
};
typedef enum led_index_posn_enum led_index_posn;

#define LED_INDEX_LSB_MASK                       (0x00000001u)
#define LED_INDEX_MSB_MASK                       (0x00000008u)

enum led_logarithmic_en_posn_enum
{
   LED_LOGARITHMIC_EN_POSN                            = (int)0,
   LED_LOGARITHMIC_EN_LED_LOGARITHMIC_EN_LSB_POSN     = (int)0,
   LED_LOGARITHMIC_EN_LED_LOGARITHMIC_EN_MSB_POSN     = (int)0
};
typedef enum led_logarithmic_en_posn_enum led_logarithmic_en_posn;

#define LED_LOGARITHMIC_EN_MASK                  (0x00000001u)

enum led_logarithmic_offset_high_posn_enum
{
   LED_LOGARITHMIC_OFFSET_HIGH_LSB_POSN               = (int)0,
   LED_LOGARITHMIC_OFFSET_HIGH_LED_LOGARITHMIC_OFFSET_HIGH_LSB_POSN = (int)0,
   LED_LOGARITHMIC_OFFSET_HIGH_MSB_POSN               = (int)3,
   LED_LOGARITHMIC_OFFSET_HIGH_LED_LOGARITHMIC_OFFSET_HIGH_MSB_POSN = (int)3
};
typedef enum led_logarithmic_offset_high_posn_enum led_logarithmic_offset_high_posn;

#define LED_LOGARITHMIC_OFFSET_HIGH_LSB_MASK     (0x00000001u)
#define LED_LOGARITHMIC_OFFSET_HIGH_MSB_MASK     (0x00000008u)

enum led_logarithmic_offset_low_posn_enum
{
   LED_LOGARITHMIC_OFFSET_LOW_LSB_POSN                = (int)0,
   LED_LOGARITHMIC_OFFSET_LOW_LED_LOGARITHMIC_OFFSET_LOW_LSB_POSN = (int)0,
   LED_LOGARITHMIC_OFFSET_LOW_MSB_POSN                = (int)3,
   LED_LOGARITHMIC_OFFSET_LOW_LED_LOGARITHMIC_OFFSET_LOW_MSB_POSN = (int)3
};
typedef enum led_logarithmic_offset_low_posn_enum led_logarithmic_offset_low_posn;

#define LED_LOGARITHMIC_OFFSET_LOW_LSB_MASK      (0x00000001u)
#define LED_LOGARITHMIC_OFFSET_LOW_MSB_MASK      (0x00000008u)

enum led_logarithmic_status_posn_enum
{
   LED_LOGARITHMIC_STATUS_LSB_POSN                    = (int)0,
   LED_LOGARITHMIC_STATUS_LED_LOGARITHMIC_STATUS_LSB_POSN = (int)0,
   LED_LOGARITHMIC_STATUS_MSB_POSN                    = (int)11,
   LED_LOGARITHMIC_STATUS_LED_LOGARITHMIC_STATUS_MSB_POSN = (int)11
};
typedef enum led_logarithmic_status_posn_enum led_logarithmic_status_posn;

#define LED_LOGARITHMIC_STATUS_LSB_MASK          (0x00000001u)
#define LED_LOGARITHMIC_STATUS_MSB_MASK          (0x00000800u)

enum led_max_high_config_posn_enum
{
   LED_MAX_HIGH_CONFIG_LSB_POSN                       = (int)0,
   LED_MAX_HIGH_CONFIG_LED_MAX_HIGH_CONFIG_LSB_POSN   = (int)0,
   LED_MAX_HIGH_CONFIG_MSB_POSN                       = (int)15,
   LED_MAX_HIGH_CONFIG_LED_MAX_HIGH_CONFIG_MSB_POSN   = (int)15
};
typedef enum led_max_high_config_posn_enum led_max_high_config_posn;

#define LED_MAX_HIGH_CONFIG_LSB_MASK             (0x00000001u)
#define LED_MAX_HIGH_CONFIG_MSB_MASK             (0x00008000u)

enum led_max_high_config_status_posn_enum
{
   LED_MAX_HIGH_CONFIG_STATUS_LSB_POSN                = (int)0,
   LED_MAX_HIGH_CONFIG_STATUS_LED_MAX_HIGH_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_MAX_HIGH_CONFIG_STATUS_MSB_POSN                = (int)15,
   LED_MAX_HIGH_CONFIG_STATUS_LED_MAX_HIGH_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_max_high_config_status_posn_enum led_max_high_config_status_posn;

#define LED_MAX_HIGH_CONFIG_STATUS_LSB_MASK      (0x00000001u)
#define LED_MAX_HIGH_CONFIG_STATUS_MSB_MASK      (0x00008000u)

enum led_max_low_config_posn_enum
{
   LED_MAX_LOW_CONFIG_LSB_POSN                        = (int)0,
   LED_MAX_LOW_CONFIG_LED_MAX_LOW_CONFIG_LSB_POSN     = (int)0,
   LED_MAX_LOW_CONFIG_MSB_POSN                        = (int)15,
   LED_MAX_LOW_CONFIG_LED_MAX_LOW_CONFIG_MSB_POSN     = (int)15
};
typedef enum led_max_low_config_posn_enum led_max_low_config_posn;

#define LED_MAX_LOW_CONFIG_LSB_MASK              (0x00000001u)
#define LED_MAX_LOW_CONFIG_MSB_MASK              (0x00008000u)

enum led_max_low_config_status_posn_enum
{
   LED_MAX_LOW_CONFIG_STATUS_LSB_POSN                 = (int)0,
   LED_MAX_LOW_CONFIG_STATUS_LED_MAX_LOW_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_MAX_LOW_CONFIG_STATUS_MSB_POSN                 = (int)15,
   LED_MAX_LOW_CONFIG_STATUS_LED_MAX_LOW_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_max_low_config_status_posn_enum led_max_low_config_status_posn;

#define LED_MAX_LOW_CONFIG_STATUS_LSB_MASK       (0x00000001u)
#define LED_MAX_LOW_CONFIG_STATUS_MSB_MASK       (0x00008000u)

enum led_min_high_config_posn_enum
{
   LED_MIN_HIGH_CONFIG_LSB_POSN                       = (int)0,
   LED_MIN_HIGH_CONFIG_LED_MIN_HIGH_CONFIG_LSB_POSN   = (int)0,
   LED_MIN_HIGH_CONFIG_MSB_POSN                       = (int)15,
   LED_MIN_HIGH_CONFIG_LED_MIN_HIGH_CONFIG_MSB_POSN   = (int)15
};
typedef enum led_min_high_config_posn_enum led_min_high_config_posn;

#define LED_MIN_HIGH_CONFIG_LSB_MASK             (0x00000001u)
#define LED_MIN_HIGH_CONFIG_MSB_MASK             (0x00008000u)

enum led_min_high_config_status_posn_enum
{
   LED_MIN_HIGH_CONFIG_STATUS_LSB_POSN                = (int)0,
   LED_MIN_HIGH_CONFIG_STATUS_LED_MIN_HIGH_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_MIN_HIGH_CONFIG_STATUS_MSB_POSN                = (int)15,
   LED_MIN_HIGH_CONFIG_STATUS_LED_MIN_HIGH_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_min_high_config_status_posn_enum led_min_high_config_status_posn;

#define LED_MIN_HIGH_CONFIG_STATUS_LSB_MASK      (0x00000001u)
#define LED_MIN_HIGH_CONFIG_STATUS_MSB_MASK      (0x00008000u)

enum led_min_low_config_posn_enum
{
   LED_MIN_LOW_CONFIG_LSB_POSN                        = (int)0,
   LED_MIN_LOW_CONFIG_LED_MIN_LOW_CONFIG_LSB_POSN     = (int)0,
   LED_MIN_LOW_CONFIG_MSB_POSN                        = (int)15,
   LED_MIN_LOW_CONFIG_LED_MIN_LOW_CONFIG_MSB_POSN     = (int)15
};
typedef enum led_min_low_config_posn_enum led_min_low_config_posn;

#define LED_MIN_LOW_CONFIG_LSB_MASK              (0x00000001u)
#define LED_MIN_LOW_CONFIG_MSB_MASK              (0x00008000u)

enum led_min_low_config_status_posn_enum
{
   LED_MIN_LOW_CONFIG_STATUS_LSB_POSN                 = (int)0,
   LED_MIN_LOW_CONFIG_STATUS_LED_MIN_LOW_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_MIN_LOW_CONFIG_STATUS_MSB_POSN                 = (int)15,
   LED_MIN_LOW_CONFIG_STATUS_LED_MIN_LOW_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_min_low_config_status_posn_enum led_min_low_config_status_posn;

#define LED_MIN_LOW_CONFIG_STATUS_LSB_MASK       (0x00000001u)
#define LED_MIN_LOW_CONFIG_STATUS_MSB_MASK       (0x00008000u)

enum led_pin_config_posn_enum
{
   LED_PIN_CONFIG_LSB_POSN                            = (int)0,
   LED_PIN_CONFIG_LED_PIN_CONFIG_LSB_POSN             = (int)0,
   LED_PIN_CONFIG_MSB_POSN                            = (int)1,
   LED_PIN_CONFIG_LED_PIN_CONFIG_MSB_POSN             = (int)1
};
typedef enum led_pin_config_posn_enum led_pin_config_posn;

#define LED_PIN_CONFIG_LSB_MASK                  (0x00000001u)
#define LED_PIN_CONFIG_MSB_MASK                  (0x00000002u)

enum led_pin_config_status_posn_enum
{
   LED_PIN_CONFIG_STATUS_LSB_POSN                     = (int)0,
   LED_PIN_CONFIG_STATUS_LED_PIN_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_PIN_CONFIG_STATUS_MSB_POSN                     = (int)1,
   LED_PIN_CONFIG_STATUS_LED_PIN_CONFIG_STATUS_MSB_POSN = (int)1
};
typedef enum led_pin_config_status_posn_enum led_pin_config_status_posn;

#define LED_PIN_CONFIG_STATUS_LSB_MASK           (0x00000001u)
#define LED_PIN_CONFIG_STATUS_MSB_MASK           (0x00000002u)

enum led_ramp_config_posn_enum
{
   LED_RAMP_CONFIG_LSB_POSN                           = (int)0,
   LED_RAMP_CONFIG_LED_RAMP_CONFIG_LSB_POSN           = (int)0,
   LED_RAMP_CONFIG_MSB_POSN                           = (int)15,
   LED_RAMP_CONFIG_LED_RAMP_CONFIG_MSB_POSN           = (int)15
};
typedef enum led_ramp_config_posn_enum led_ramp_config_posn;

#define LED_RAMP_CONFIG_LSB_MASK                 (0x00000001u)
#define LED_RAMP_CONFIG_MSB_MASK                 (0x00008000u)

enum led_ramp_config_status_posn_enum
{
   LED_RAMP_CONFIG_STATUS_LSB_POSN                    = (int)0,
   LED_RAMP_CONFIG_STATUS_LED_RAMP_CONFIG_STATUS_LSB_POSN = (int)0,
   LED_RAMP_CONFIG_STATUS_MSB_POSN                    = (int)15,
   LED_RAMP_CONFIG_STATUS_LED_RAMP_CONFIG_STATUS_MSB_POSN = (int)15
};
typedef enum led_ramp_config_status_posn_enum led_ramp_config_status_posn;

#define LED_RAMP_CONFIG_STATUS_LSB_MASK          (0x00000001u)
#define LED_RAMP_CONFIG_STATUS_MSB_MASK          (0x00008000u)

enum led_ramp_current_high_config_posn_enum
{
   LED_RAMP_CURRENT_HIGH_CONFIG_LSB_POSN              = (int)0,
   LED_RAMP_CURRENT_HIGH_CONFIG_LED_RAMP_CURRENT_HIGH_CONFIG_LSB_POSN = (int)0,
   LED_RAMP_CURRENT_HIGH_CONFIG_MSB_POSN              = (int)15,
   LED_RAMP_CURRENT_HIGH_CONFIG_LED_RAMP_CURRENT_HIGH_CONFIG_MSB_POSN = (int)15
};
typedef enum led_ramp_current_high_config_posn_enum led_ramp_current_high_config_posn;

#define LED_RAMP_CURRENT_HIGH_CONFIG_LSB_MASK    (0x00000001u)
#define LED_RAMP_CURRENT_HIGH_CONFIG_MSB_MASK    (0x00008000u)

enum led_ramp_current_low_config_posn_enum
{
   LED_RAMP_CURRENT_LOW_CONFIG_LSB_POSN               = (int)0,
   LED_RAMP_CURRENT_LOW_CONFIG_LED_RAMP_CURRENT_LOW_CONFIG_LSB_POSN = (int)0,
   LED_RAMP_CURRENT_LOW_CONFIG_MSB_POSN               = (int)15,
   LED_RAMP_CURRENT_LOW_CONFIG_LED_RAMP_CURRENT_LOW_CONFIG_MSB_POSN = (int)15
};
typedef enum led_ramp_current_low_config_posn_enum led_ramp_current_low_config_posn;

#define LED_RAMP_CURRENT_LOW_CONFIG_LSB_MASK     (0x00000001u)
#define LED_RAMP_CURRENT_LOW_CONFIG_MSB_MASK     (0x00008000u)

enum led_ramp_shift_posn_enum
{
   LED_RAMP_SHIFT_LSB_POSN                            = (int)0,
   LED_RAMP_SHIFT_LED_RAMP_SHIFT_LSB_POSN             = (int)0,
   LED_RAMP_SHIFT_MSB_POSN                            = (int)2,
   LED_RAMP_SHIFT_LED_RAMP_SHIFT_MSB_POSN             = (int)2
};
typedef enum led_ramp_shift_posn_enum led_ramp_shift_posn;

#define LED_RAMP_SHIFT_LSB_MASK                  (0x00000001u)
#define LED_RAMP_SHIFT_MSB_MASK                  (0x00000004u)

enum led_single_shot_high_config_posn_enum
{
   LED_SINGLE_SHOT_HIGH_CONFIG_LSB_POSN               = (int)0,
   LED_SINGLE_SHOT_HIGH_CONFIG_LED_SINGLE_SHOT_HIGH_CONFIG_LSB_POSN = (int)0,
   LED_SINGLE_SHOT_HIGH_CONFIG_MSB_POSN               = (int)15,
   LED_SINGLE_SHOT_HIGH_CONFIG_LED_SINGLE_SHOT_HIGH_CONFIG_MSB_POSN = (int)15
};
typedef enum led_single_shot_high_config_posn_enum led_single_shot_high_config_posn;

#define LED_SINGLE_SHOT_HIGH_CONFIG_LSB_MASK     (0x00000001u)
#define LED_SINGLE_SHOT_HIGH_CONFIG_MSB_MASK     (0x00008000u)

enum led_single_shot_low_config_posn_enum
{
   LED_SINGLE_SHOT_LOW_CONFIG_LSB_POSN                = (int)0,
   LED_SINGLE_SHOT_LOW_CONFIG_LED_SINGLE_SHOT_LOW_CONFIG_LSB_POSN = (int)0,
   LED_SINGLE_SHOT_LOW_CONFIG_MSB_POSN                = (int)15,
   LED_SINGLE_SHOT_LOW_CONFIG_LED_SINGLE_SHOT_LOW_CONFIG_MSB_POSN = (int)15
};
typedef enum led_single_shot_low_config_posn_enum led_single_shot_low_config_posn;

#define LED_SINGLE_SHOT_LOW_CONFIG_LSB_MASK      (0x00000001u)
#define LED_SINGLE_SHOT_LOW_CONFIG_MSB_MASK      (0x00008000u)

enum led_start_up_state_posn_enum
{
   LED_START_UP_STATE_LSB_POSN                        = (int)0,
   LED_START_UP_STATE_LED_START_UP_STATE_LSB_POSN     = (int)0,
   LED_START_UP_STATE_MSB_POSN                        = (int)2,
   LED_START_UP_STATE_LED_START_UP_STATE_MSB_POSN     = (int)2
};
typedef enum led_start_up_state_posn_enum led_start_up_state_posn;

#define LED_START_UP_STATE_LSB_MASK              (0x00000001u)
#define LED_START_UP_STATE_MSB_MASK              (0x00000004u)

enum led_en_posn_enum
{
   LED_EN_LED0_LSB_POSN                               = (int)0,
   LED_EN_LED0_MSB_POSN                               = (int)0,
   LED_EN_LED1_LSB_POSN                               = (int)1,
   LED_EN_LED1_MSB_POSN                               = (int)1,
   LED_EN_LED2_LSB_POSN                               = (int)2,
   LED_EN_LED2_MSB_POSN                               = (int)2,
   LED_EN_LED3_LSB_POSN                               = (int)3,
   LED_EN_LED3_MSB_POSN                               = (int)3,
   LED_EN_LED4_LSB_POSN                               = (int)4,
   LED_EN_LED4_MSB_POSN                               = (int)4,
   LED_EN_LED5_LSB_POSN                               = (int)5,
   LED_EN_LED5_MSB_POSN                               = (int)5
};
typedef enum led_en_posn_enum led_en_posn;


enum led_single_shot_mode_posn_enum
{
   LED_SINGLE_SHOT_MODE_LED0_LSB_POSN                 = (int)0,
   LED_SINGLE_SHOT_MODE_LED0_MSB_POSN                 = (int)0,
   LED_SINGLE_SHOT_MODE_LED1_LSB_POSN                 = (int)1,
   LED_SINGLE_SHOT_MODE_LED1_MSB_POSN                 = (int)1,
   LED_SINGLE_SHOT_MODE_LED2_LSB_POSN                 = (int)2,
   LED_SINGLE_SHOT_MODE_LED2_MSB_POSN                 = (int)2,
   LED_SINGLE_SHOT_MODE_LED3_LSB_POSN                 = (int)3,
   LED_SINGLE_SHOT_MODE_LED3_MSB_POSN                 = (int)3,
   LED_SINGLE_SHOT_MODE_LED4_LSB_POSN                 = (int)4,
   LED_SINGLE_SHOT_MODE_LED4_MSB_POSN                 = (int)4,
   LED_SINGLE_SHOT_MODE_LED5_LSB_POSN                 = (int)5,
   LED_SINGLE_SHOT_MODE_LED5_MSB_POSN                 = (int)5
};
typedef enum led_single_shot_mode_posn_enum led_single_shot_mode_posn;


enum led_update_posn_enum
{
   LED_UPDATE_LED0_LSB_POSN                           = (int)0,
   LED_UPDATE_LED0_MSB_POSN                           = (int)0,
   LED_UPDATE_LED1_LSB_POSN                           = (int)1,
   LED_UPDATE_LED1_MSB_POSN                           = (int)1,
   LED_UPDATE_LED2_LSB_POSN                           = (int)2,
   LED_UPDATE_LED2_MSB_POSN                           = (int)2,
   LED_UPDATE_LED3_LSB_POSN                           = (int)3,
   LED_UPDATE_LED3_MSB_POSN                           = (int)3,
   LED_UPDATE_LED4_LSB_POSN                           = (int)4,
   LED_UPDATE_LED4_MSB_POSN                           = (int)4,
   LED_UPDATE_LED5_LSB_POSN                           = (int)5,
   LED_UPDATE_LED5_MSB_POSN                           = (int)5
};
typedef enum led_update_posn_enum led_update_posn;


#endif /* IO_DEFS_MODULE_LED_CTRL */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_LED_CTRL */



#if defined(IO_DEFS_MODULE_BITSERIAL1) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL1
#define __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL1

/* -- bitserial1 -- Bitserializer regs -- */

enum bitserial1_buffer_enum_posn_enum
{
   BITSERIAL1_BUFF_HANDLE_LSB_POSN                    = (int)0,
   BITSERIAL1_BUFFER_ENUM_BITSERIAL1_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL1_BUFF_HANDLE_MSB_POSN                    = (int)7,
   BITSERIAL1_BUFFER_ENUM_BITSERIAL1_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL1_REMOTE_SSID_LSB_POSN                    = (int)8,
   BITSERIAL1_BUFFER_ENUM_BITSERIAL1_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL1_REMOTE_SSID_MSB_POSN                    = (int)11,
   BITSERIAL1_BUFFER_ENUM_BITSERIAL1_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial1_buffer_enum_posn_enum bitserial1_buffer_enum_posn;

#define BITSERIAL1_BUFF_HANDLE_LSB_MASK          (0x00000001u)
#define BITSERIAL1_BUFF_HANDLE_MSB_MASK          (0x00000080u)
#define BITSERIAL1_REMOTE_SSID_LSB_MASK          (0x00000100u)
#define BITSERIAL1_REMOTE_SSID_MSB_MASK          (0x00000800u)

enum bitserial1_event_types_enum
{
   BITSERIAL1_EVENT_TX_SUCCESS              = (int)0x0,
   BITSERIAL1_EVENT_RX_SUCCESS              = (int)0x1,
   BITSERIAL1_EVENT_ERROR_IN_BUFFER         = (int)0x2,
   BITSERIAL1_EVENT_ERROR_IN_READ           = (int)0x3,
   BITSERIAL1_EVENT_SLAVE_START_STOP_BITS_ERROR = (int)0x4,
   BITSERIAL1_EVENT_FIFO_UNDERFLOW          = (int)0x5,
   BITSERIAL1_EVENT_FIFO_OVERFLOW           = (int)0x6,
   BITSERIAL1_EVENT_I2C_ACK_ERROR           = (int)0x7,
   BITSERIAL1_EVENT_I2C_LOST_ARB            = (int)0x8,
   BITSERIAL1_EVENT_I2C_NAK_STOP            = (int)0x9,
   BITSERIAL1_EVENT_I2C_NAK_RESTART         = (int)0xA,
   BITSERIAL1_EVENT_TX_NOT_CONFIGURED       = (int)0xB,
   BITSERIAL1_EVENT_RX_CMD_DETECTED         = (int)0xC,
   BITSERIAL1_EVENT_TX_STREAMING_SUCCESS    = (int)0xD,
   BITSERIAL1_EVENT_WIDTH                   = (int)0xE,
   MAX_BITSERIAL1_EVENT_TYPES               = (int)0xE
};
#define NUM_BITSERIAL1_EVENT_TYPES (0xF)
typedef enum bitserial1_event_types_enum bitserial1_event_types;


enum bitserial1_clk_control_posn_enum
{
   BITSERIAL1_MASTER_SLAVE_CLK_REQ_EN_POSN            = (int)0,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_MASTER_SLAVE_CLK_REQ_EN_LSB_POSN = (int)0,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_MASTER_SLAVE_CLK_REQ_EN_MSB_POSN = (int)0,
   BITSERIAL1_MAIN_CLK_REQ_EN_POSN                    = (int)1,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_MAIN_CLK_REQ_EN_LSB_POSN = (int)1,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_MAIN_CLK_REQ_EN_MSB_POSN = (int)1,
   BITSERIAL1_CLK_DIVIDE_LSB_POSN                     = (int)2,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_CLK_DIVIDE_LSB_POSN = (int)2,
   BITSERIAL1_CLK_DIVIDE_MSB_POSN                     = (int)9,
   BITSERIAL1_CLK_CONTROL_BITSERIAL1_CLK_DIVIDE_MSB_POSN = (int)9
};
typedef enum bitserial1_clk_control_posn_enum bitserial1_clk_control_posn;

#define BITSERIAL1_MASTER_SLAVE_CLK_REQ_EN_MASK  (0x00000001u)
#define BITSERIAL1_MAIN_CLK_REQ_EN_MASK          (0x00000002u)
#define BITSERIAL1_CLK_DIVIDE_LSB_MASK           (0x00000004u)
#define BITSERIAL1_CLK_DIVIDE_MSB_MASK           (0x00000200u)

enum bitserial1_config_posn_enum
{
   BITSERIAL1_CONFIG_CLK_INVERT_POSN                  = (int)0,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLK_INVERT_LSB_POSN = (int)0,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLK_INVERT_MSB_POSN = (int)0,
   BITSERIAL1_CONFIG_DIN_INVERT_POSN                  = (int)1,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_DIN_INVERT_LSB_POSN = (int)1,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_DIN_INVERT_MSB_POSN = (int)1,
   BITSERIAL1_CONFIG_DOUT_INVERT_POSN                 = (int)2,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_DOUT_INVERT_LSB_POSN = (int)2,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_DOUT_INVERT_MSB_POSN = (int)2,
   BITSERIAL1_CONFIG_BYTESWAP_EN_POSN                 = (int)3,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_BYTESWAP_EN_LSB_POSN = (int)3,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_BYTESWAP_EN_MSB_POSN = (int)3,
   BITSERIAL1_CONFIG_INT_EVENT_SUBSYSTEM_EN_POSN      = (int)4,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_INT_EVENT_SUBSYSTEM_EN_LSB_POSN = (int)4,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_INT_EVENT_SUBSYSTEM_EN_MSB_POSN = (int)4,
   BITSERIAL1_CONFIG_INT_EVENT_CURATOR_EN_POSN        = (int)5,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_INT_EVENT_CURATOR_EN_LSB_POSN = (int)5,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_INT_EVENT_CURATOR_EN_MSB_POSN = (int)5,
   BITSERIAL1_CONFIG_BITREVERSE_EN_POSN               = (int)6,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_BITREVERSE_EN_LSB_POSN = (int)6,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_BITREVERSE_EN_MSB_POSN = (int)6,
   BITSERIAL1_CONFIG_CLEAR_STICKY_ACK_POSN            = (int)7,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLEAR_STICKY_ACK_LSB_POSN = (int)7,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLEAR_STICKY_ACK_MSB_POSN = (int)7,
   BITSERIAL1_CONFIG_CLK_OFFSET_EN_POSN               = (int)8,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLK_OFFSET_EN_LSB_POSN = (int)8,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_CLK_OFFSET_EN_MSB_POSN = (int)8,
   BITSERIAL1_CONFIG_NEG_EDGE_SAMP_EN_POSN            = (int)9,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_NEG_EDGE_SAMP_EN_LSB_POSN = (int)9,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_NEG_EDGE_SAMP_EN_MSB_POSN = (int)9,
   BITSERIAL1_CONFIG_I2C_MODE_EN_POSN                 = (int)10,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_I2C_MODE_EN_LSB_POSN = (int)10,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_I2C_MODE_EN_MSB_POSN = (int)10,
   BITSERIAL1_CONFIG_POSEDGE_LAUNCH_MODE_EN_POSN      = (int)11,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_POSEDGE_LAUNCH_MODE_EN_LSB_POSN = (int)11,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_POSEDGE_LAUNCH_MODE_EN_MSB_POSN = (int)11,
   BITSERIAL1_CONFIG_STOP_TOKEN_DISABLE_POSN          = (int)12,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_STOP_TOKEN_DISABLE_LSB_POSN = (int)12,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_STOP_TOKEN_DISABLE_MSB_POSN = (int)12,
   BITSERIAL1_CONFIG_SEL_INVERT_POSN                  = (int)13,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_INVERT_LSB_POSN = (int)13,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_INVERT_MSB_POSN = (int)13,
   BITSERIAL1_CONFIG_SEL_EN_POSN                      = (int)14,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_EN_LSB_POSN = (int)14,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_EN_MSB_POSN = (int)14,
   BITSERIAL1_CONFIG_SEL_TIME_EN_POSN                 = (int)15,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_TIME_EN_LSB_POSN = (int)15,
   BITSERIAL1_CONFIG_BITSERIAL1_CONFIG_SEL_TIME_EN_MSB_POSN = (int)15
};
typedef enum bitserial1_config_posn_enum bitserial1_config_posn;

#define BITSERIAL1_CONFIG_CLK_INVERT_MASK        (0x00000001u)
#define BITSERIAL1_CONFIG_DIN_INVERT_MASK        (0x00000002u)
#define BITSERIAL1_CONFIG_DOUT_INVERT_MASK       (0x00000004u)
#define BITSERIAL1_CONFIG_BYTESWAP_EN_MASK       (0x00000008u)
#define BITSERIAL1_CONFIG_INT_EVENT_SUBSYSTEM_EN_MASK (0x00000010u)
#define BITSERIAL1_CONFIG_INT_EVENT_CURATOR_EN_MASK (0x00000020u)
#define BITSERIAL1_CONFIG_BITREVERSE_EN_MASK     (0x00000040u)
#define BITSERIAL1_CONFIG_CLEAR_STICKY_ACK_MASK  (0x00000080u)
#define BITSERIAL1_CONFIG_CLK_OFFSET_EN_MASK     (0x00000100u)
#define BITSERIAL1_CONFIG_NEG_EDGE_SAMP_EN_MASK  (0x00000200u)
#define BITSERIAL1_CONFIG_I2C_MODE_EN_MASK       (0x00000400u)
#define BITSERIAL1_CONFIG_POSEDGE_LAUNCH_MODE_EN_MASK (0x00000800u)
#define BITSERIAL1_CONFIG_STOP_TOKEN_DISABLE_MASK (0x00001000u)
#define BITSERIAL1_CONFIG_SEL_INVERT_MASK        (0x00002000u)
#define BITSERIAL1_CONFIG_SEL_EN_MASK            (0x00004000u)
#define BITSERIAL1_CONFIG_SEL_TIME_EN_MASK       (0x00008000u)

enum bitserial1_config2_posn_enum
{
   BITSERIAL1_CONFIG2_COMBO_MODE_POSN                 = (int)0,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_COMBO_MODE_LSB_POSN = (int)0,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_COMBO_MODE_MSB_POSN = (int)0,
   BITSERIAL1_CONFIG2_STOP_TOKEN_DISABLE2_POSN        = (int)1,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_STOP_TOKEN_DISABLE2_LSB_POSN = (int)1,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_STOP_TOKEN_DISABLE2_MSB_POSN = (int)1,
   BITSERIAL1_CONFIG2_SEL_EN2_POSN                    = (int)2,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SEL_EN2_LSB_POSN = (int)2,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SEL_EN2_MSB_POSN = (int)2,
   BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_LSB_POSN          = (int)3,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_LSB_POSN = (int)3,
   BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_MSB_POSN          = (int)4,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_MSB_POSN = (int)4,
   BITSERIAL1_CONFIG2_SLAVE_MODE_POSN                 = (int)5,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_MODE_LSB_POSN = (int)5,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_MODE_MSB_POSN = (int)5,
   BITSERIAL1_CONFIG2_SLAVE_10BIT_ADDR_MODE_POSN      = (int)6,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_10BIT_ADDR_MODE_LSB_POSN = (int)6,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_10BIT_ADDR_MODE_MSB_POSN = (int)6,
   BITSERIAL1_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_POSN = (int)7,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_LSB_POSN = (int)7,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_MSB_POSN = (int)7,
   BITSERIAL1_CONFIG2_DATA_READY_WORD_DISABLE_POSN    = (int)8,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DATA_READY_WORD_DISABLE_LSB_POSN = (int)8,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DATA_READY_WORD_DISABLE_MSB_POSN = (int)8,
   BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_POSN     = (int)9,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_LSB_POSN = (int)9,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_MSB_POSN = (int)9,
   BITSERIAL1_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_POSN      = (int)10,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_LSB_POSN = (int)10,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MSB_POSN = (int)10,
   BITSERIAL1_CONFIG2_DISABLE_UNDERFLOW_INT_POSN      = (int)11,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DISABLE_UNDERFLOW_INT_LSB_POSN = (int)11,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DISABLE_UNDERFLOW_INT_MSB_POSN = (int)11,
   BITSERIAL1_CONFIG2_DEBUG_SEL_LSB_POSN              = (int)12,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DEBUG_SEL_LSB_POSN = (int)12,
   BITSERIAL1_CONFIG2_DEBUG_SEL_MSB_POSN              = (int)13,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_DEBUG_SEL_MSB_POSN = (int)13,
   BITSERIAL1_CONFIG2_FORCE_SEL_ACTIVE_POSN           = (int)14,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_FORCE_SEL_ACTIVE_LSB_POSN = (int)14,
   BITSERIAL1_CONFIG2_BITSERIAL1_CONFIG2_FORCE_SEL_ACTIVE_MSB_POSN = (int)14
};
typedef enum bitserial1_config2_posn_enum bitserial1_config2_posn;

#define BITSERIAL1_CONFIG2_COMBO_MODE_MASK       (0x00000001u)
#define BITSERIAL1_CONFIG2_STOP_TOKEN_DISABLE2_MASK (0x00000002u)
#define BITSERIAL1_CONFIG2_SEL_EN2_MASK          (0x00000004u)
#define BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_LSB_MASK (0x00000008u)
#define BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_MSB_MASK (0x00000010u)
#define BITSERIAL1_CONFIG2_SLAVE_MODE_MASK       (0x00000020u)
#define BITSERIAL1_CONFIG2_SLAVE_10BIT_ADDR_MODE_MASK (0x00000040u)
#define BITSERIAL1_CONFIG2_I2C_SL_CLK_STRETCH_DISABLE_MASK (0x00000080u)
#define BITSERIAL1_CONFIG2_DATA_READY_WORD_DISABLE_MASK (0x00000100u)
#define BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_MASK (0x00000200u)
#define BITSERIAL1_CONFIG2_SLAVE_ANY_CMD_BYTE_EN_MASK (0x00000400u)
#define BITSERIAL1_CONFIG2_DISABLE_UNDERFLOW_INT_MASK (0x00000800u)
#define BITSERIAL1_CONFIG2_DEBUG_SEL_LSB_MASK    (0x00001000u)
#define BITSERIAL1_CONFIG2_DEBUG_SEL_MSB_MASK    (0x00002000u)
#define BITSERIAL1_CONFIG2_FORCE_SEL_ACTIVE_MASK (0x00004000u)

enum bitserial1_config2_dout_idle_sel_enum
{
   BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_LOW     = (int)0x0,
   BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_HIGH    = (int)0x1,
   BITSERIAL1_CONFIG2_DOUT_IDLE_SEL_LAST    = (int)0x2,
   MAX_BITSERIAL1_CONFIG2_DOUT_IDLE_SEL     = (int)0x2
};
#define NUM_BITSERIAL1_CONFIG2_DOUT_IDLE_SEL (0x3)
typedef enum bitserial1_config2_dout_idle_sel_enum bitserial1_config2_dout_idle_sel;


enum bitserial1_config2_slave_read_mode_switch_enum
{
   BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_AUTO = (int)0x0,
   BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH_MANUAL = (int)0x1,
   MAX_BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH = (int)0x1
};
#define NUM_BITSERIAL1_CONFIG2_SLAVE_READ_MODE_SWITCH (0x2)
typedef enum bitserial1_config2_slave_read_mode_switch_enum bitserial1_config2_slave_read_mode_switch;


enum bitserial1_config2_enum
{
   BITSERIAL1_CONFIG2_DEBUG_SEL_MASTER      = (int)0x0,
   BITSERIAL1_CONFIG2_DEBUG_SEL_SLAVE       = (int)0x1,
   BITSERIAL1_CONFIG2_DEBUG_SEL_VML_REQUEST = (int)0x2,
   MAX_BITSERIAL1_CONFIG2                   = (int)0x2
};
#define NUM_BITSERIAL1_CONFIG2 (0x3)
typedef enum bitserial1_config2_enum bitserial1_config2;


enum bitserial1_config3_posn_enum
{
   BITSERIAL1_CONFIG3_ACT_ON_NAK_LSB_POSN             = (int)0,
   BITSERIAL1_CONFIG3_BITSERIAL1_CONFIG3_ACT_ON_NAK_LSB_POSN = (int)0,
   BITSERIAL1_CONFIG3_ACT_ON_NAK_MSB_POSN             = (int)1,
   BITSERIAL1_CONFIG3_BITSERIAL1_CONFIG3_ACT_ON_NAK_MSB_POSN = (int)1,
   BITSERIAL1_CLR_SLAVE_READ_FIFO_POSN                = (int)2,
   BITSERIAL1_CONFIG3_BITSERIAL1_CLR_SLAVE_READ_FIFO_LSB_POSN = (int)2,
   BITSERIAL1_CONFIG3_BITSERIAL1_CLR_SLAVE_READ_FIFO_MSB_POSN = (int)2,
   BITSERIAL1_CLR_SLAVE_WRITE_FIFO_POSN               = (int)3,
   BITSERIAL1_CONFIG3_BITSERIAL1_CLR_SLAVE_WRITE_FIFO_LSB_POSN = (int)3,
   BITSERIAL1_CONFIG3_BITSERIAL1_CLR_SLAVE_WRITE_FIFO_MSB_POSN = (int)3,
   BITSERIAL1_STREAMING_EN_POSN                       = (int)4,
   BITSERIAL1_CONFIG3_BITSERIAL1_STREAMING_EN_LSB_POSN = (int)4,
   BITSERIAL1_CONFIG3_BITSERIAL1_STREAMING_EN_MSB_POSN = (int)4
};
typedef enum bitserial1_config3_posn_enum bitserial1_config3_posn;

#define BITSERIAL1_CONFIG3_ACT_ON_NAK_LSB_MASK   (0x00000001u)
#define BITSERIAL1_CONFIG3_ACT_ON_NAK_MSB_MASK   (0x00000002u)
#define BITSERIAL1_CLR_SLAVE_READ_FIFO_MASK      (0x00000004u)
#define BITSERIAL1_CLR_SLAVE_WRITE_FIFO_MASK     (0x00000008u)
#define BITSERIAL1_STREAMING_EN_MASK             (0x00000010u)

enum bitserial1_config3_enum
{
   BITSERIAL1_CONFIG3_ACT_ON_NAK_LEGACY     = (int)0x0,
   BITSERIAL1_CONFIG3_ACT_ON_NAK_STOP       = (int)0x1,
   BITSERIAL1_CONFIG3_ACT_ON_NAK_RESTART    = (int)0x2,
   MAX_BITSERIAL1_CONFIG3                   = (int)0x2
};
#define NUM_BITSERIAL1_CONFIG3 (0x3)
typedef enum bitserial1_config3_enum bitserial1_config3;


enum bitserial1_debug_sel_posn_enum
{
   BITSERIAL1_DEBUG_SEL_SLAVE_MEM_CORE_POSN           = (int)0,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_MEM_CORE_LSB_POSN = (int)0,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_MEM_CORE_MSB_POSN = (int)0,
   BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_LSB_POSN       = (int)1,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_LSB_POSN = (int)1,
   BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_MSB_POSN       = (int)4,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_MSB_POSN = (int)4,
   BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_LSB_POSN        = (int)5,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_LSB_POSN = (int)5,
   BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_MSB_POSN        = (int)8,
   BITSERIAL1_DEBUG_SEL_BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_MSB_POSN = (int)8
};
typedef enum bitserial1_debug_sel_posn_enum bitserial1_debug_sel_posn;

#define BITSERIAL1_DEBUG_SEL_SLAVE_MEM_CORE_MASK (0x00000001u)
#define BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_LSB_MASK (0x00000002u)
#define BITSERIAL1_DEBUG_SEL_SLAVE_CORE_SEL_MSB_MASK (0x00000010u)
#define BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_LSB_MASK (0x00000020u)
#define BITSERIAL1_DEBUG_SEL_SLAVE_MEM_SEL_MSB_MASK (0x00000100u)

enum bitserial1_deglitch_en_posn_enum
{
   BITSERIAL1_DEGLITCH_EN_CLOCK_POSN                  = (int)0,
   BITSERIAL1_DEGLITCH_EN_BITSERIAL1_DEGLITCH_EN_CLOCK_LSB_POSN = (int)0,
   BITSERIAL1_DEGLITCH_EN_BITSERIAL1_DEGLITCH_EN_CLOCK_MSB_POSN = (int)0,
   BITSERIAL1_DEGLITCH_EN_DATA_POSN                   = (int)1,
   BITSERIAL1_DEGLITCH_EN_BITSERIAL1_DEGLITCH_EN_DATA_LSB_POSN = (int)1,
   BITSERIAL1_DEGLITCH_EN_BITSERIAL1_DEGLITCH_EN_DATA_MSB_POSN = (int)1
};
typedef enum bitserial1_deglitch_en_posn_enum bitserial1_deglitch_en_posn;

#define BITSERIAL1_DEGLITCH_EN_CLOCK_MASK        (0x00000001u)
#define BITSERIAL1_DEGLITCH_EN_DATA_MASK         (0x00000002u)

enum bitserial1_error_status_posn_enum
{
   BITSERIAL1_VML_RESP_STATUS_LSB_POSN                = (int)0,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_VML_RESP_STATUS_LSB_POSN = (int)0,
   BITSERIAL1_VML_RESP_STATUS_MSB_POSN                = (int)3,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_VML_RESP_STATUS_MSB_POSN = (int)3,
   BITSERIAL1_READ_RESP_STATUS_LSB_POSN               = (int)4,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_READ_RESP_STATUS_LSB_POSN = (int)4,
   BITSERIAL1_READ_RESP_STATUS_MSB_POSN               = (int)7,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_READ_RESP_STATUS_MSB_POSN = (int)7,
   BITSERIAL1_TRANSACTION_PART2_POSN                  = (int)8,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_TRANSACTION_PART2_LSB_POSN = (int)8,
   BITSERIAL1_ERROR_STATUS_BITSERIAL1_TRANSACTION_PART2_MSB_POSN = (int)8
};
typedef enum bitserial1_error_status_posn_enum bitserial1_error_status_posn;

#define BITSERIAL1_VML_RESP_STATUS_LSB_MASK      (0x00000001u)
#define BITSERIAL1_VML_RESP_STATUS_MSB_MASK      (0x00000008u)
#define BITSERIAL1_READ_RESP_STATUS_LSB_MASK     (0x00000010u)
#define BITSERIAL1_READ_RESP_STATUS_MSB_MASK     (0x00000080u)
#define BITSERIAL1_TRANSACTION_PART2_MASK        (0x00000100u)

enum bitserial1_i2c_acks_posn_enum
{
   BITSERIAL1_I2C_ACKS_HISTORY_LSB_POSN               = (int)0,
   BITSERIAL1_I2C_ACKS_BITSERIAL1_I2C_ACKS_HISTORY_LSB_POSN = (int)0,
   BITSERIAL1_I2C_ACKS_HISTORY_MSB_POSN               = (int)14,
   BITSERIAL1_I2C_ACKS_BITSERIAL1_I2C_ACKS_HISTORY_MSB_POSN = (int)14,
   BITSERIAL1_I2C_ACKS_STICKY_POSN                    = (int)15,
   BITSERIAL1_I2C_ACKS_BITSERIAL1_I2C_ACKS_STICKY_LSB_POSN = (int)15,
   BITSERIAL1_I2C_ACKS_BITSERIAL1_I2C_ACKS_STICKY_MSB_POSN = (int)15
};
typedef enum bitserial1_i2c_acks_posn_enum bitserial1_i2c_acks_posn;

#define BITSERIAL1_I2C_ACKS_HISTORY_LSB_MASK     (0x00000001u)
#define BITSERIAL1_I2C_ACKS_HISTORY_MSB_MASK     (0x00004000u)
#define BITSERIAL1_I2C_ACKS_STICKY_MASK          (0x00008000u)

enum bitserial1_interbyte_spacing_posn_enum
{
   BITSERIAL1_INTERBYTE_SPACING_CYCLES_LSB_POSN       = (int)0,
   BITSERIAL1_INTERBYTE_SPACING_BITSERIAL1_INTERBYTE_SPACING_CYCLES_LSB_POSN = (int)0,
   BITSERIAL1_INTERBYTE_SPACING_CYCLES_MSB_POSN       = (int)14,
   BITSERIAL1_INTERBYTE_SPACING_BITSERIAL1_INTERBYTE_SPACING_CYCLES_MSB_POSN = (int)14,
   BITSERIAL1_INTERBYTE_SPACING_EN_POSN               = (int)15,
   BITSERIAL1_INTERBYTE_SPACING_BITSERIAL1_INTERBYTE_SPACING_EN_LSB_POSN = (int)15,
   BITSERIAL1_INTERBYTE_SPACING_BITSERIAL1_INTERBYTE_SPACING_EN_MSB_POSN = (int)15
};
typedef enum bitserial1_interbyte_spacing_posn_enum bitserial1_interbyte_spacing_posn;

#define BITSERIAL1_INTERBYTE_SPACING_CYCLES_LSB_MASK (0x00000001u)
#define BITSERIAL1_INTERBYTE_SPACING_CYCLES_MSB_MASK (0x00004000u)
#define BITSERIAL1_INTERBYTE_SPACING_EN_MASK     (0x00008000u)

enum bitserial1_status_posn_enum
{
   BITSERIAL1_BUSY_POSN                               = (int)0,
   BITSERIAL1_STATUS_BITSERIAL1_BUSY_LSB_POSN         = (int)0,
   BITSERIAL1_STATUS_BITSERIAL1_BUSY_MSB_POSN         = (int)0,
   BITSERIAL1_RWB_POSN                                = (int)1,
   BITSERIAL1_STATUS_BITSERIAL1_RWB_LSB_POSN          = (int)1,
   BITSERIAL1_STATUS_BITSERIAL1_RWB_MSB_POSN          = (int)1,
   BITSERIAL1_EVENT_POSN                              = (int)2,
   BITSERIAL1_STATUS_BITSERIAL1_EVENT_LSB_POSN        = (int)2,
   BITSERIAL1_STATUS_BITSERIAL1_EVENT_MSB_POSN        = (int)2,
   BITSERIAL1_FAILED_POSN                             = (int)3,
   BITSERIAL1_STATUS_BITSERIAL1_FAILED_LSB_POSN       = (int)3,
   BITSERIAL1_STATUS_BITSERIAL1_FAILED_MSB_POSN       = (int)3,
   BITSERIAL1_I2C_LOST_ARBITRATION_POSN               = (int)4,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_LOST_ARBITRATION_LSB_POSN = (int)4,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_LOST_ARBITRATION_MSB_POSN = (int)4,
   BITSERIAL1_I2C_BUS_BUSY_POSN                       = (int)5,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_BUS_BUSY_LSB_POSN = (int)5,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_BUS_BUSY_MSB_POSN = (int)5,
   BITSERIAL1_PART_2_POSN                             = (int)6,
   BITSERIAL1_STATUS_BITSERIAL1_PART_2_LSB_POSN       = (int)6,
   BITSERIAL1_STATUS_BITSERIAL1_PART_2_MSB_POSN       = (int)6,
   BITSERIAL1_FIFO_OVERFLOW_POSN                      = (int)7,
   BITSERIAL1_STATUS_BITSERIAL1_FIFO_OVERFLOW_LSB_POSN = (int)7,
   BITSERIAL1_STATUS_BITSERIAL1_FIFO_OVERFLOW_MSB_POSN = (int)7,
   BITSERIAL1_FIFO_UNDERFLOW_POSN                     = (int)8,
   BITSERIAL1_STATUS_BITSERIAL1_FIFO_UNDERFLOW_LSB_POSN = (int)8,
   BITSERIAL1_STATUS_BITSERIAL1_FIFO_UNDERFLOW_MSB_POSN = (int)8,
   BITSERIAL1_SLAVE_START_STOP_BITS_ERROR_POSN        = (int)9,
   BITSERIAL1_STATUS_BITSERIAL1_SLAVE_START_STOP_BITS_ERROR_LSB_POSN = (int)9,
   BITSERIAL1_STATUS_BITSERIAL1_SLAVE_START_STOP_BITS_ERROR_MSB_POSN = (int)9,
   BITSERIAL1_I2C_NACK_POSN                           = (int)10,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_NACK_LSB_POSN     = (int)10,
   BITSERIAL1_STATUS_BITSERIAL1_I2C_NACK_MSB_POSN     = (int)10,
   BITSERIAL1_DEEP_SLEEP_CDC_BUSY_POSN                = (int)11,
   BITSERIAL1_STATUS_BITSERIAL1_DEEP_SLEEP_CDC_BUSY_LSB_POSN = (int)11,
   BITSERIAL1_STATUS_BITSERIAL1_DEEP_SLEEP_CDC_BUSY_MSB_POSN = (int)11,
   BITSERIAL1_TX_NOT_CONFIGURED_POSN                  = (int)12,
   BITSERIAL1_STATUS_BITSERIAL1_TX_NOT_CONFIGURED_LSB_POSN = (int)12,
   BITSERIAL1_STATUS_BITSERIAL1_TX_NOT_CONFIGURED_MSB_POSN = (int)12
};
typedef enum bitserial1_status_posn_enum bitserial1_status_posn;

#define BITSERIAL1_BUSY_MASK                     (0x00000001u)
#define BITSERIAL1_RWB_MASK                      (0x00000002u)
#define BITSERIAL1_EVENT_MASK                    (0x00000004u)
#define BITSERIAL1_FAILED_MASK                   (0x00000008u)
#define BITSERIAL1_I2C_LOST_ARBITRATION_MASK     (0x00000010u)
#define BITSERIAL1_I2C_BUS_BUSY_MASK             (0x00000020u)
#define BITSERIAL1_PART_2_MASK                   (0x00000040u)
#define BITSERIAL1_FIFO_OVERFLOW_MASK            (0x00000080u)
#define BITSERIAL1_FIFO_UNDERFLOW_MASK           (0x00000100u)
#define BITSERIAL1_SLAVE_START_STOP_BITS_ERROR_MASK (0x00000200u)
#define BITSERIAL1_I2C_NACK_MASK                 (0x00000400u)
#define BITSERIAL1_DEEP_SLEEP_CDC_BUSY_MASK      (0x00000800u)
#define BITSERIAL1_TX_NOT_CONFIGURED_MASK        (0x00001000u)

enum bitserial1_word_config_posn_enum
{
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_LSB_POSN          = (int)0,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_BYTES_LSB_POSN = (int)0,
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_MSB_POSN          = (int)1,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_BYTES_MSB_POSN = (int)1,
   BITSERIAL1_WORD_CONFIG_NUM_STOP_LSB_POSN           = (int)2,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_STOP_LSB_POSN = (int)2,
   BITSERIAL1_WORD_CONFIG_NUM_STOP_MSB_POSN           = (int)3,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_STOP_MSB_POSN = (int)3,
   BITSERIAL1_WORD_CONFIG_NUM_START_LSB_POSN          = (int)4,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_START_LSB_POSN = (int)4,
   BITSERIAL1_WORD_CONFIG_NUM_START_MSB_POSN          = (int)5,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_NUM_START_MSB_POSN = (int)5,
   BITSERIAL1_WORD_CONFIG_START_BITS_LSB_POSN         = (int)6,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_START_BITS_LSB_POSN = (int)6,
   BITSERIAL1_WORD_CONFIG_START_BITS_MSB_POSN         = (int)8,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_START_BITS_MSB_POSN = (int)8,
   BITSERIAL1_WORD_CONFIG_STOP_BITS_LSB_POSN          = (int)9,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_STOP_BITS_LSB_POSN = (int)9,
   BITSERIAL1_WORD_CONFIG_STOP_BITS_MSB_POSN          = (int)11,
   BITSERIAL1_WORD_CONFIG_BITSERIAL1_WORD_CONFIG_STOP_BITS_MSB_POSN = (int)11
};
typedef enum bitserial1_word_config_posn_enum bitserial1_word_config_posn;

#define BITSERIAL1_WORD_CONFIG_NUM_BYTES_LSB_MASK (0x00000001u)
#define BITSERIAL1_WORD_CONFIG_NUM_BYTES_MSB_MASK (0x00000002u)
#define BITSERIAL1_WORD_CONFIG_NUM_STOP_LSB_MASK (0x00000004u)
#define BITSERIAL1_WORD_CONFIG_NUM_STOP_MSB_MASK (0x00000008u)
#define BITSERIAL1_WORD_CONFIG_NUM_START_LSB_MASK (0x00000010u)
#define BITSERIAL1_WORD_CONFIG_NUM_START_MSB_MASK (0x00000020u)
#define BITSERIAL1_WORD_CONFIG_START_BITS_LSB_MASK (0x00000040u)
#define BITSERIAL1_WORD_CONFIG_START_BITS_MSB_MASK (0x00000100u)
#define BITSERIAL1_WORD_CONFIG_STOP_BITS_LSB_MASK (0x00000200u)
#define BITSERIAL1_WORD_CONFIG_STOP_BITS_MSB_MASK (0x00000800u)

enum bitserial1_word_config_enum
{
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_ONE     = (int)0x0,
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_TWO     = (int)0x1,
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_THREE   = (int)0x2,
   BITSERIAL1_WORD_CONFIG_NUM_BYTES_FOUR    = (int)0x3,
   MAX_BITSERIAL1_WORD_CONFIG               = (int)0x3
};
#define NUM_BITSERIAL1_WORD_CONFIG (0x4)
typedef enum bitserial1_word_config_enum bitserial1_word_config;


enum bitserial1_clk_rate_hi_posn_enum
{
   BITSERIAL1_CLK_RATE_HI_LSB_POSN                    = (int)0,
   BITSERIAL1_CLK_RATE_HI_BITSERIAL1_CLK_RATE_HI_LSB_POSN = (int)0,
   BITSERIAL1_CLK_RATE_HI_MSB_POSN                    = (int)15,
   BITSERIAL1_CLK_RATE_HI_BITSERIAL1_CLK_RATE_HI_MSB_POSN = (int)15
};
typedef enum bitserial1_clk_rate_hi_posn_enum bitserial1_clk_rate_hi_posn;

#define BITSERIAL1_CLK_RATE_HI_LSB_MASK          (0x00000001u)
#define BITSERIAL1_CLK_RATE_HI_MSB_MASK          (0x00008000u)

enum bitserial1_clk_rate_lo_posn_enum
{
   BITSERIAL1_CLK_RATE_LO_LSB_POSN                    = (int)0,
   BITSERIAL1_CLK_RATE_LO_BITSERIAL1_CLK_RATE_LO_LSB_POSN = (int)0,
   BITSERIAL1_CLK_RATE_LO_MSB_POSN                    = (int)15,
   BITSERIAL1_CLK_RATE_LO_BITSERIAL1_CLK_RATE_LO_MSB_POSN = (int)15
};
typedef enum bitserial1_clk_rate_lo_posn_enum bitserial1_clk_rate_lo_posn;

#define BITSERIAL1_CLK_RATE_LO_LSB_MASK          (0x00000001u)
#define BITSERIAL1_CLK_RATE_LO_MSB_MASK          (0x00008000u)

enum bitserial1_clk_samp_offset_posn_enum
{
   BITSERIAL1_CLK_SAMP_OFFSET_LSB_POSN                = (int)0,
   BITSERIAL1_CLK_SAMP_OFFSET_BITSERIAL1_CLK_SAMP_OFFSET_LSB_POSN = (int)0,
   BITSERIAL1_CLK_SAMP_OFFSET_MSB_POSN                = (int)15,
   BITSERIAL1_CLK_SAMP_OFFSET_BITSERIAL1_CLK_SAMP_OFFSET_MSB_POSN = (int)15
};
typedef enum bitserial1_clk_samp_offset_posn_enum bitserial1_clk_samp_offset_posn;

#define BITSERIAL1_CLK_SAMP_OFFSET_LSB_MASK      (0x00000001u)
#define BITSERIAL1_CLK_SAMP_OFFSET_MSB_MASK      (0x00008000u)

enum bitserial1_config_sel_time_posn_enum
{
   BITSERIAL1_CONFIG_SEL_TIME_LSB_POSN                = (int)0,
   BITSERIAL1_CONFIG_SEL_TIME_BITSERIAL1_CONFIG_SEL_TIME_LSB_POSN = (int)0,
   BITSERIAL1_CONFIG_SEL_TIME_MSB_POSN                = (int)15,
   BITSERIAL1_CONFIG_SEL_TIME_BITSERIAL1_CONFIG_SEL_TIME_MSB_POSN = (int)15
};
typedef enum bitserial1_config_sel_time_posn_enum bitserial1_config_sel_time_posn;

#define BITSERIAL1_CONFIG_SEL_TIME_LSB_MASK      (0x00000001u)
#define BITSERIAL1_CONFIG_SEL_TIME_MSB_MASK      (0x00008000u)

enum bitserial1_config_sel_time2_posn_enum
{
   BITSERIAL1_CONFIG_SEL_TIME2_LSB_POSN               = (int)0,
   BITSERIAL1_CONFIG_SEL_TIME2_BITSERIAL1_CONFIG_SEL_TIME2_LSB_POSN = (int)0,
   BITSERIAL1_CONFIG_SEL_TIME2_MSB_POSN               = (int)3,
   BITSERIAL1_CONFIG_SEL_TIME2_BITSERIAL1_CONFIG_SEL_TIME2_MSB_POSN = (int)3
};
typedef enum bitserial1_config_sel_time2_posn_enum bitserial1_config_sel_time2_posn;

#define BITSERIAL1_CONFIG_SEL_TIME2_LSB_MASK     (0x00000001u)
#define BITSERIAL1_CONFIG_SEL_TIME2_MSB_MASK     (0x00000008u)

enum bitserial1_curator_ssid_posn_enum
{
   BITSERIAL1_CURATOR_SSID_LSB_POSN                   = (int)0,
   BITSERIAL1_CURATOR_SSID_BITSERIAL1_CURATOR_SSID_LSB_POSN = (int)0,
   BITSERIAL1_CURATOR_SSID_MSB_POSN                   = (int)3,
   BITSERIAL1_CURATOR_SSID_BITSERIAL1_CURATOR_SSID_MSB_POSN = (int)3
};
typedef enum bitserial1_curator_ssid_posn_enum bitserial1_curator_ssid_posn;

#define BITSERIAL1_CURATOR_SSID_LSB_MASK         (0x00000001u)
#define BITSERIAL1_CURATOR_SSID_MSB_MASK         (0x00000008u)

enum bitserial1_deep_sleep_active_posn_enum
{
   BITSERIAL1_DEEP_SLEEP_ACTIVE_POSN                  = (int)0,
   BITSERIAL1_DEEP_SLEEP_ACTIVE_BITSERIAL1_DEEP_SLEEP_ACTIVE_LSB_POSN = (int)0,
   BITSERIAL1_DEEP_SLEEP_ACTIVE_BITSERIAL1_DEEP_SLEEP_ACTIVE_MSB_POSN = (int)0
};
typedef enum bitserial1_deep_sleep_active_posn_enum bitserial1_deep_sleep_active_posn;

#define BITSERIAL1_DEEP_SLEEP_ACTIVE_MASK        (0x00000001u)

enum bitserial1_i2c_address_posn_enum
{
   BITSERIAL1_I2C_ADDRESS_LSB_POSN                    = (int)0,
   BITSERIAL1_I2C_ADDRESS_BITSERIAL1_I2C_ADDRESS_LSB_POSN = (int)0,
   BITSERIAL1_I2C_ADDRESS_MSB_POSN                    = (int)9,
   BITSERIAL1_I2C_ADDRESS_BITSERIAL1_I2C_ADDRESS_MSB_POSN = (int)9
};
typedef enum bitserial1_i2c_address_posn_enum bitserial1_i2c_address_posn;

#define BITSERIAL1_I2C_ADDRESS_LSB_MASK          (0x00000001u)
#define BITSERIAL1_I2C_ADDRESS_MSB_MASK          (0x00000200u)

enum bitserial1_interrupt_clear_posn_enum
{
   BITSERIAL1_INTERRUPT_CLEAR_LSB_POSN                = (int)0,
   BITSERIAL1_INTERRUPT_CLEAR_BITSERIAL1_INTERRUPT_CLEAR_LSB_POSN = (int)0,
   BITSERIAL1_INTERRUPT_CLEAR_MSB_POSN                = (int)13,
   BITSERIAL1_INTERRUPT_CLEAR_BITSERIAL1_INTERRUPT_CLEAR_MSB_POSN = (int)13
};
typedef enum bitserial1_interrupt_clear_posn_enum bitserial1_interrupt_clear_posn;

#define BITSERIAL1_INTERRUPT_CLEAR_LSB_MASK      (0x00000001u)
#define BITSERIAL1_INTERRUPT_CLEAR_MSB_MASK      (0x00002000u)

enum bitserial1_interrupt_en_posn_enum
{
   BITSERIAL1_INTERRUPT_EN_LSB_POSN                   = (int)0,
   BITSERIAL1_INTERRUPT_EN_BITSERIAL1_INTERRUPT_EN_LSB_POSN = (int)0,
   BITSERIAL1_INTERRUPT_EN_MSB_POSN                   = (int)13,
   BITSERIAL1_INTERRUPT_EN_BITSERIAL1_INTERRUPT_EN_MSB_POSN = (int)13
};
typedef enum bitserial1_interrupt_en_posn_enum bitserial1_interrupt_en_posn;

#define BITSERIAL1_INTERRUPT_EN_LSB_MASK         (0x00000001u)
#define BITSERIAL1_INTERRUPT_EN_MSB_MASK         (0x00002000u)

enum bitserial1_interrupt_source_posn_enum
{
   BITSERIAL1_INTERRUPT_SOURCE_LSB_POSN               = (int)0,
   BITSERIAL1_INTERRUPT_SOURCE_BITSERIAL1_INTERRUPT_SOURCE_LSB_POSN = (int)0,
   BITSERIAL1_INTERRUPT_SOURCE_MSB_POSN               = (int)13,
   BITSERIAL1_INTERRUPT_SOURCE_BITSERIAL1_INTERRUPT_SOURCE_MSB_POSN = (int)13
};
typedef enum bitserial1_interrupt_source_posn_enum bitserial1_interrupt_source_posn;

#define BITSERIAL1_INTERRUPT_SOURCE_LSB_MASK     (0x00000001u)
#define BITSERIAL1_INTERRUPT_SOURCE_MSB_MASK     (0x00002000u)

enum bitserial1_interrupt_source_clear_posn_enum
{
   BITSERIAL1_INTERRUPT_SOURCE_CLEAR_LSB_POSN         = (int)0,
   BITSERIAL1_INTERRUPT_SOURCE_CLEAR_BITSERIAL1_INTERRUPT_SOURCE_CLEAR_LSB_POSN = (int)0,
   BITSERIAL1_INTERRUPT_SOURCE_CLEAR_MSB_POSN         = (int)13,
   BITSERIAL1_INTERRUPT_SOURCE_CLEAR_BITSERIAL1_INTERRUPT_SOURCE_CLEAR_MSB_POSN = (int)13
};
typedef enum bitserial1_interrupt_source_clear_posn_enum bitserial1_interrupt_source_clear_posn;

#define BITSERIAL1_INTERRUPT_SOURCE_CLEAR_LSB_MASK (0x00000001u)
#define BITSERIAL1_INTERRUPT_SOURCE_CLEAR_MSB_MASK (0x00002000u)

enum bitserial1_interrupt_status_posn_enum
{
   BITSERIAL1_INTERRUPT_STATUS_LSB_POSN               = (int)0,
   BITSERIAL1_INTERRUPT_STATUS_BITSERIAL1_INTERRUPT_STATUS_LSB_POSN = (int)0,
   BITSERIAL1_INTERRUPT_STATUS_MSB_POSN               = (int)13,
   BITSERIAL1_INTERRUPT_STATUS_BITSERIAL1_INTERRUPT_STATUS_MSB_POSN = (int)13
};
typedef enum bitserial1_interrupt_status_posn_enum bitserial1_interrupt_status_posn;

#define BITSERIAL1_INTERRUPT_STATUS_LSB_MASK     (0x00000001u)
#define BITSERIAL1_INTERRUPT_STATUS_MSB_MASK     (0x00002000u)

enum bitserial1_remote_subsystem_ssid_posn_enum
{
   BITSERIAL1_REMOTE_SUBSYSTEM_SSID_LSB_POSN          = (int)0,
   BITSERIAL1_REMOTE_SUBSYSTEM_SSID_BITSERIAL1_REMOTE_SUBSYSTEM_SSID_LSB_POSN = (int)0,
   BITSERIAL1_REMOTE_SUBSYSTEM_SSID_MSB_POSN          = (int)3,
   BITSERIAL1_REMOTE_SUBSYSTEM_SSID_BITSERIAL1_REMOTE_SUBSYSTEM_SSID_MSB_POSN = (int)3
};
typedef enum bitserial1_remote_subsystem_ssid_posn_enum bitserial1_remote_subsystem_ssid_posn;

#define BITSERIAL1_REMOTE_SUBSYSTEM_SSID_LSB_MASK (0x00000001u)
#define BITSERIAL1_REMOTE_SUBSYSTEM_SSID_MSB_MASK (0x00000008u)

enum bitserial1_set_sel_inactive_posn_enum
{
   BITSERIAL1_SET_SEL_INACTIVE_POSN                   = (int)0,
   BITSERIAL1_SET_SEL_INACTIVE_BITSERIAL1_SET_SEL_INACTIVE_LSB_POSN = (int)0,
   BITSERIAL1_SET_SEL_INACTIVE_BITSERIAL1_SET_SEL_INACTIVE_MSB_POSN = (int)0
};
typedef enum bitserial1_set_sel_inactive_posn_enum bitserial1_set_sel_inactive_posn;

#define BITSERIAL1_SET_SEL_INACTIVE_MASK         (0x00000001u)

enum bitserial1_slave_data_ready_byte_posn_enum
{
   BITSERIAL1_SLAVE_DATA_READY_BYTE_LSB_POSN          = (int)0,
   BITSERIAL1_SLAVE_DATA_READY_BYTE_BITSERIAL1_SLAVE_DATA_READY_BYTE_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_DATA_READY_BYTE_MSB_POSN          = (int)7,
   BITSERIAL1_SLAVE_DATA_READY_BYTE_BITSERIAL1_SLAVE_DATA_READY_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial1_slave_data_ready_byte_posn_enum bitserial1_slave_data_ready_byte_posn;

#define BITSERIAL1_SLAVE_DATA_READY_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL1_SLAVE_DATA_READY_BYTE_MSB_MASK (0x00000080u)

enum bitserial1_slave_num_protocol_words_posn_enum
{
   BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_LSB_POSN       = (int)0,
   BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_MSB_POSN       = (int)15,
   BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_MSB_POSN = (int)15
};
typedef enum bitserial1_slave_num_protocol_words_posn_enum bitserial1_slave_num_protocol_words_posn;

#define BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_LSB_MASK (0x00000001u)
#define BITSERIAL1_SLAVE_NUM_PROTOCOL_WORDS_MSB_MASK (0x00008000u)

enum bitserial1_slave_read_command_byte_posn_enum
{
   BITSERIAL1_SLAVE_READ_COMMAND_BYTE_LSB_POSN        = (int)0,
   BITSERIAL1_SLAVE_READ_COMMAND_BYTE_BITSERIAL1_SLAVE_READ_COMMAND_BYTE_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_READ_COMMAND_BYTE_MSB_POSN        = (int)7,
   BITSERIAL1_SLAVE_READ_COMMAND_BYTE_BITSERIAL1_SLAVE_READ_COMMAND_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial1_slave_read_command_byte_posn_enum bitserial1_slave_read_command_byte_posn;

#define BITSERIAL1_SLAVE_READ_COMMAND_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL1_SLAVE_READ_COMMAND_BYTE_MSB_MASK (0x00000080u)

enum bitserial1_slave_read_count_posn_enum
{
   BITSERIAL1_SLAVE_READ_COUNT_LSB_POSN               = (int)0,
   BITSERIAL1_SLAVE_READ_COUNT_BITSERIAL1_SLAVE_READ_COUNT_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_READ_COUNT_MSB_POSN               = (int)15,
   BITSERIAL1_SLAVE_READ_COUNT_BITSERIAL1_SLAVE_READ_COUNT_MSB_POSN = (int)15
};
typedef enum bitserial1_slave_read_count_posn_enum bitserial1_slave_read_count_posn;

#define BITSERIAL1_SLAVE_READ_COUNT_LSB_MASK     (0x00000001u)
#define BITSERIAL1_SLAVE_READ_COUNT_MSB_MASK     (0x00008000u)

enum bitserial1_slave_underflow_byte_posn_enum
{
   BITSERIAL1_SLAVE_UNDERFLOW_BYTE_LSB_POSN           = (int)0,
   BITSERIAL1_SLAVE_UNDERFLOW_BYTE_BITSERIAL1_SLAVE_UNDERFLOW_BYTE_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_UNDERFLOW_BYTE_MSB_POSN           = (int)7,
   BITSERIAL1_SLAVE_UNDERFLOW_BYTE_BITSERIAL1_SLAVE_UNDERFLOW_BYTE_MSB_POSN = (int)7
};
typedef enum bitserial1_slave_underflow_byte_posn_enum bitserial1_slave_underflow_byte_posn;

#define BITSERIAL1_SLAVE_UNDERFLOW_BYTE_LSB_MASK (0x00000001u)
#define BITSERIAL1_SLAVE_UNDERFLOW_BYTE_MSB_MASK (0x00000080u)

enum bitserial1_slave_write_count_posn_enum
{
   BITSERIAL1_SLAVE_WRITE_COUNT_LSB_POSN              = (int)0,
   BITSERIAL1_SLAVE_WRITE_COUNT_BITSERIAL1_SLAVE_WRITE_COUNT_LSB_POSN = (int)0,
   BITSERIAL1_SLAVE_WRITE_COUNT_MSB_POSN              = (int)15,
   BITSERIAL1_SLAVE_WRITE_COUNT_BITSERIAL1_SLAVE_WRITE_COUNT_MSB_POSN = (int)15
};
typedef enum bitserial1_slave_write_count_posn_enum bitserial1_slave_write_count_posn;

#define BITSERIAL1_SLAVE_WRITE_COUNT_LSB_MASK    (0x00000001u)
#define BITSERIAL1_SLAVE_WRITE_COUNT_MSB_MASK    (0x00008000u)

enum bitserial1_streaming_threshold_posn_enum
{
   BITSERIAL1_STREAMING_THRESHOLD_LSB_POSN            = (int)0,
   BITSERIAL1_STREAMING_THRESHOLD_BITSERIAL1_STREAMING_THRESHOLD_LSB_POSN = (int)0,
   BITSERIAL1_STREAMING_THRESHOLD_MSB_POSN            = (int)15,
   BITSERIAL1_STREAMING_THRESHOLD_BITSERIAL1_STREAMING_THRESHOLD_MSB_POSN = (int)15
};
typedef enum bitserial1_streaming_threshold_posn_enum bitserial1_streaming_threshold_posn;

#define BITSERIAL1_STREAMING_THRESHOLD_LSB_MASK  (0x00000001u)
#define BITSERIAL1_STREAMING_THRESHOLD_MSB_MASK  (0x00008000u)

enum bitserial1_txrx_length_posn_enum
{
   BITSERIAL1_TXRX_LENGTH_LSB_POSN                    = (int)0,
   BITSERIAL1_TXRX_LENGTH_BITSERIAL1_TXRX_LENGTH_LSB_POSN = (int)0,
   BITSERIAL1_TXRX_LENGTH_MSB_POSN                    = (int)15,
   BITSERIAL1_TXRX_LENGTH_BITSERIAL1_TXRX_LENGTH_MSB_POSN = (int)15
};
typedef enum bitserial1_txrx_length_posn_enum bitserial1_txrx_length_posn;

#define BITSERIAL1_TXRX_LENGTH_LSB_MASK          (0x00000001u)
#define BITSERIAL1_TXRX_LENGTH_MSB_MASK          (0x00008000u)

enum bitserial1_txrx_length2_posn_enum
{
   BITSERIAL1_TXRX_LENGTH2_LSB_POSN                   = (int)0,
   BITSERIAL1_TXRX_LENGTH2_BITSERIAL1_TXRX_LENGTH2_LSB_POSN = (int)0,
   BITSERIAL1_TXRX_LENGTH2_MSB_POSN                   = (int)15,
   BITSERIAL1_TXRX_LENGTH2_BITSERIAL1_TXRX_LENGTH2_MSB_POSN = (int)15
};
typedef enum bitserial1_txrx_length2_posn_enum bitserial1_txrx_length2_posn;

#define BITSERIAL1_TXRX_LENGTH2_LSB_MASK         (0x00000001u)
#define BITSERIAL1_TXRX_LENGTH2_MSB_MASK         (0x00008000u)

enum bitserial1_words_sent_posn_enum
{
   BITSERIAL1_WORDS_SENT_LSB_POSN                     = (int)0,
   BITSERIAL1_WORDS_SENT_BITSERIAL1_WORDS_SENT_LSB_POSN = (int)0,
   BITSERIAL1_WORDS_SENT_MSB_POSN                     = (int)15,
   BITSERIAL1_WORDS_SENT_BITSERIAL1_WORDS_SENT_MSB_POSN = (int)15
};
typedef enum bitserial1_words_sent_posn_enum bitserial1_words_sent_posn;

#define BITSERIAL1_WORDS_SENT_LSB_MASK           (0x00000001u)
#define BITSERIAL1_WORDS_SENT_MSB_MASK           (0x00008000u)

enum bitserial1_rx_buffer_posn_enum
{
   BITSERIAL1_RX_BUFFER_BITSERIAL1_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL1_RX_BUFFER_BITSERIAL1_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL1_RX_BUFFER_BITSERIAL1_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL1_RX_BUFFER_BITSERIAL1_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial1_rx_buffer_posn_enum bitserial1_rx_buffer_posn;


enum bitserial1_rx_buffer2_posn_enum
{
   BITSERIAL1_RX_BUFFER2_BITSERIAL1_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL1_RX_BUFFER2_BITSERIAL1_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL1_RX_BUFFER2_BITSERIAL1_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL1_RX_BUFFER2_BITSERIAL1_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial1_rx_buffer2_posn_enum bitserial1_rx_buffer2_posn;


enum bitserial1_tx_buffer_posn_enum
{
   BITSERIAL1_TX_BUFFER_BITSERIAL1_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL1_TX_BUFFER_BITSERIAL1_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL1_TX_BUFFER_BITSERIAL1_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL1_TX_BUFFER_BITSERIAL1_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial1_tx_buffer_posn_enum bitserial1_tx_buffer_posn;


enum bitserial1_tx_buffer2_posn_enum
{
   BITSERIAL1_TX_BUFFER2_BITSERIAL1_BUFF_HANDLE_LSB_POSN = (int)0,
   BITSERIAL1_TX_BUFFER2_BITSERIAL1_BUFF_HANDLE_MSB_POSN = (int)7,
   BITSERIAL1_TX_BUFFER2_BITSERIAL1_REMOTE_SSID_LSB_POSN = (int)8,
   BITSERIAL1_TX_BUFFER2_BITSERIAL1_REMOTE_SSID_MSB_POSN = (int)11
};
typedef enum bitserial1_tx_buffer2_posn_enum bitserial1_tx_buffer2_posn;


#endif /* IO_DEFS_MODULE_BITSERIAL1 */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_BITSERIAL1 */




#if defined(IO_DEFS_MODULE_K32_MISC) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_MISC
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_MISC

/* -- k32_misc -- Kalimba 32-bit Misc Control registers -- */

enum clock_divide_rate_enum_enum
{
   CLOCK_STOPPED                            = (int)0x0,
   CLOCK_RATE_MAX                           = (int)0x1,
   CLOCK_RATE_HALF                          = (int)0x2,
   CLOCK_RATE_RESERVED                      = (int)0x3,
   MAX_CLOCK_DIVIDE_RATE_ENUM               = (int)0x3
};
#define NUM_CLOCK_DIVIDE_RATE_ENUM (0x4)
typedef enum clock_divide_rate_enum_enum clock_divide_rate_enum;


enum allow_goto_shallow_sleep_posn_enum
{
   ALLOW_GOTO_SHALLOW_SLEEP_POSN                      = (int)0,
   ALLOW_GOTO_SHALLOW_SLEEP_ALLOW_GOTO_SHALLOW_SLEEP_LSB_POSN = (int)0,
   ALLOW_GOTO_SHALLOW_SLEEP_ALLOW_GOTO_SHALLOW_SLEEP_MSB_POSN = (int)0
};
typedef enum allow_goto_shallow_sleep_posn_enum allow_goto_shallow_sleep_posn;

#define ALLOW_GOTO_SHALLOW_SLEEP_MASK            (0x00000001u)

enum clock_cont_shallow_sleep_rate_posn_enum
{
   CLOCK_CONT_SHALLOW_SLEEP_RATE_LSB_POSN             = (int)0,
   CLOCK_CONT_SHALLOW_SLEEP_RATE_CLOCK_CONT_SHALLOW_SLEEP_RATE_LSB_POSN = (int)0,
   CLOCK_CONT_SHALLOW_SLEEP_RATE_MSB_POSN             = (int)7,
   CLOCK_CONT_SHALLOW_SLEEP_RATE_CLOCK_CONT_SHALLOW_SLEEP_RATE_MSB_POSN = (int)7
};
typedef enum clock_cont_shallow_sleep_rate_posn_enum clock_cont_shallow_sleep_rate_posn;

#define CLOCK_CONT_SHALLOW_SLEEP_RATE_LSB_MASK   (0x00000001u)
#define CLOCK_CONT_SHALLOW_SLEEP_RATE_MSB_MASK   (0x00000080u)

enum clock_stop_wind_down_sequence_en_posn_enum
{
   CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_POSN              = (int)0,
   CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_LSB_POSN = (int)0,
   CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_MSB_POSN = (int)0
};
typedef enum clock_stop_wind_down_sequence_en_posn_enum clock_stop_wind_down_sequence_en_posn;

#define CLOCK_STOP_WIND_DOWN_SEQUENCE_EN_MASK    (0x00000001u)

enum disable_mutex_and_access_immunity_posn_enum
{
   DISABLE_MUTEX_AND_ACCESS_IMMUNITY_POSN             = (int)0,
   DISABLE_MUTEX_AND_ACCESS_IMMUNITY_DISABLE_MUTEX_AND_ACCESS_IMMUNITY_LSB_POSN = (int)0,
   DISABLE_MUTEX_AND_ACCESS_IMMUNITY_DISABLE_MUTEX_AND_ACCESS_IMMUNITY_MSB_POSN = (int)0
};
typedef enum disable_mutex_and_access_immunity_posn_enum disable_mutex_and_access_immunity_posn;

#define DISABLE_MUTEX_AND_ACCESS_IMMUNITY_MASK   (0x00000001u)

enum goto_shallow_sleep_posn_enum
{
   GOTO_SHALLOW_SLEEP_POSN                            = (int)0,
   GOTO_SHALLOW_SLEEP_GOTO_SHALLOW_SLEEP_LSB_POSN     = (int)0,
   GOTO_SHALLOW_SLEEP_GOTO_SHALLOW_SLEEP_MSB_POSN     = (int)0
};
typedef enum goto_shallow_sleep_posn_enum goto_shallow_sleep_posn;

#define GOTO_SHALLOW_SLEEP_MASK                  (0x00000001u)

enum pmwin_enable_posn_enum
{
   PMWIN_ENABLE_POSN                                  = (int)0,
   PMWIN_ENABLE_PMWIN_ENABLE_LSB_POSN                 = (int)0,
   PMWIN_ENABLE_PMWIN_ENABLE_MSB_POSN                 = (int)0
};
typedef enum pmwin_enable_posn_enum pmwin_enable_posn;

#define PMWIN_ENABLE_MASK                        (0x00000001u)

enum processor_id_posn_enum
{
   PROCESSOR_ID_POSN                                  = (int)0,
   PROCESSOR_ID_PROCESSOR_ID_LSB_POSN                 = (int)0,
   PROCESSOR_ID_PROCESSOR_ID_MSB_POSN                 = (int)0
};
typedef enum processor_id_posn_enum processor_id_posn;

#define PROCESSOR_ID_MASK                        (0x00000001u)

enum proc_deep_sleep_en_posn_enum
{
   PROC_DEEP_SLEEP_EN_POSN                            = (int)0,
   PROC_DEEP_SLEEP_EN_PROC_DEEP_SLEEP_EN_LSB_POSN     = (int)0,
   PROC_DEEP_SLEEP_EN_PROC_DEEP_SLEEP_EN_MSB_POSN     = (int)0
};
typedef enum proc_deep_sleep_en_posn_enum proc_deep_sleep_en_posn;

#define PROC_DEEP_SLEEP_EN_MASK                  (0x00000001u)

enum shallow_sleep_status_posn_enum
{
   SHALLOW_SLEEP_STATUS_POSN                          = (int)0,
   SHALLOW_SLEEP_STATUS_SHALLOW_SLEEP_STATUS_LSB_POSN = (int)0,
   SHALLOW_SLEEP_STATUS_SHALLOW_SLEEP_STATUS_MSB_POSN = (int)0
};
typedef enum shallow_sleep_status_posn_enum shallow_sleep_status_posn;

#define SHALLOW_SLEEP_STATUS_MASK                (0x00000001u)

#endif /* IO_DEFS_MODULE_K32_MISC */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_MISC */



#if defined(IO_DEFS_MODULE_K32_TIMERS) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_TIMERS
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_TIMERS

/* -- k32_timers -- Kalimba 32-bit Timers Control registers -- */

enum timer1_en_posn_enum
{
   TIMER1_EN_POSN                                     = (int)0,
   TIMER1_EN_TIMER1_EN_LSB_POSN                       = (int)0,
   TIMER1_EN_TIMER1_EN_MSB_POSN                       = (int)0
};
typedef enum timer1_en_posn_enum timer1_en_posn;

#define TIMER1_EN_MASK                           (0x00000001u)

enum timer1_trigger_posn_enum
{
   TIMER1_TRIGGER_LSB_POSN                            = (int)0,
   TIMER1_TRIGGER_TIMER1_TRIGGER_LSB_POSN             = (int)0,
   TIMER1_TRIGGER_MSB_POSN                            = (int)31,
   TIMER1_TRIGGER_TIMER1_TRIGGER_MSB_POSN             = (int)31
};
typedef enum timer1_trigger_posn_enum timer1_trigger_posn;

#define TIMER1_TRIGGER_LSB_MASK                  (0x00000001u)
#define TIMER1_TRIGGER_MSB_MASK                  (0x80000000u)

enum timer2_en_posn_enum
{
   TIMER2_EN_POSN                                     = (int)0,
   TIMER2_EN_TIMER2_EN_LSB_POSN                       = (int)0,
   TIMER2_EN_TIMER2_EN_MSB_POSN                       = (int)0
};
typedef enum timer2_en_posn_enum timer2_en_posn;

#define TIMER2_EN_MASK                           (0x00000001u)

enum timer2_trigger_posn_enum
{
   TIMER2_TRIGGER_LSB_POSN                            = (int)0,
   TIMER2_TRIGGER_TIMER2_TRIGGER_LSB_POSN             = (int)0,
   TIMER2_TRIGGER_MSB_POSN                            = (int)31,
   TIMER2_TRIGGER_TIMER2_TRIGGER_MSB_POSN             = (int)31
};
typedef enum timer2_trigger_posn_enum timer2_trigger_posn;

#define TIMER2_TRIGGER_LSB_MASK                  (0x00000001u)
#define TIMER2_TRIGGER_MSB_MASK                  (0x80000000u)

enum timer_time_posn_enum
{
   TIMER_TIME_LSB_POSN                                = (int)0,
   TIMER_TIME_TIMER_TIME_LSB_POSN                     = (int)0,
   TIMER_TIME_MSB_POSN                                = (int)31,
   TIMER_TIME_TIMER_TIME_MSB_POSN                     = (int)31
};
typedef enum timer_time_posn_enum timer_time_posn;

#define TIMER_TIME_LSB_MASK                      (0x00000001u)
#define TIMER_TIME_MSB_MASK                      (0x80000000u)

#endif /* IO_DEFS_MODULE_K32_TIMERS */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_TIMERS */

#if defined(IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP

/* -- apps_sys_cpu_memory_map -- Memory maps for the CPUs in the APPS SYS -- */

/* Constants for APPS_SYS_CPU0_DM */
#define P0D_SQIF0_CACHED_LOWER                             (0x70000000u)
#define P0D_SQIF0_CACHED_UPPER                             (0x70800000u)
#define P0D_SQIF1_CACHED_LOWER                             (0x78000000u)
#define P0D_SQIF1_CACHED_UPPER                             (0x78800000u)
#define P0D_SQIF_RAM_LOWER                                 (0x80000000u)
#define P0D_SQIF_RAM_UPPER                                 (0x80800000u)
#define P0D_SQIF0_DIRECT_LOWER                             (0xb0000000u)
#define P0D_SQIF0_DIRECT_UPPER                             (0xd0000000u)
#define P0D_SQIF01_DIRECT_LOWER                            (0xd0000000u)
#define P0D_SQIF01_DIRECT_UPPER                            (0xf0000000u)
#define P0D_P1_DM_RAM_LOWER                                (0x00010000u)
#define P0D_P1_DM_RAM_UPPER                                (0x00018000u)
#define P0D_P0_DM_RAM_LOWER                                (0x00000000u)
#define P0D_P0_DM_RAM_UPPER                                (0x00008000u)


/* Constants for APPS_SYS_CPU0_PM */
#define P0P_SQIF0_CACHED_LOWER                             (0x00000000u)
#define P0P_SQIF0_CACHED_UPPER                             (0x00800000u)


/* Constants for APPS_SYS_CPU1_DM */
#define P1D_SQIF0_CACHED_LOWER                             (0x70000000u)
#define P1D_SQIF0_CACHED_UPPER                             (0x70800000u)
#define P1D_SQIF1_CACHED_LOWER                             (0x78000000u)
#define P1D_SQIF1_CACHED_UPPER                             (0x78800000u)
#define P1D_SQIF_RAM_LOWER                                 (0x88000000u)
#define P1D_SQIF_RAM_UPPER                                 (0x88800000u)
#define P1D_SQIF01_DIRECT_LOWER                            (0xd0000000u)
#define P1D_SQIF01_DIRECT_UPPER                            (0xf0000000u)
#define P1D_P0_DM_RAM_LOWER                                (0x00000000u)
#define P1D_P0_DM_RAM_UPPER                                (0x00008000u)
#define P1D_P1_DM_RAM_LOWER                                (0x00010000u)
#define P1D_P1_DM_RAM_UPPER                                (0x00018000u)


/* Constants for APPS_SYS_CPU1_PM */
#define P1P_SQIF1_CACHED_LOWER                             (0x00000000u)
#define P1P_SQIF1_CACHED_UPPER                             (0x00800000u)


/* Constants for APPS_SYS_CPUx_DM */
#define PxD_CROSS_CPU_REGS_LOWER                           (0x50000000u)
#define PxD_CROSS_CPU_REGS_UPPER                           (0x58000000u)
#define PxD_CROSS_CPU_REGS_ADDR_MASK                       (0x07ffffffu)
#define PxD_REMOTE_REGISTER_SPACE_LOWER                    (0x60000000u)
#define PxD_REMOTE_REGISTER_SPACE_UPPER                    (0x60050000u)
#define PxD_SQIF0_RAM_LOWER                                (0x80000000u)
#define PxD_SQIF0_RAM_UPPER                                (0x80800000u)
#define PxD_SQIF1_RAM_LOWER                                (0x88000000u)
#define PxD_SQIF1_RAM_UPPER                                (0x88800000u)
#define PxD_REMOTE_SS_ACCESS0_LOWER                        (0x90000000u)
#define PxD_REMOTE_SS_ACCESS0_UPPER                        (0x98000000u)
#define PxD_REMOTE_SS_ACCESS1_LOWER                        (0x98000000u)
#define PxD_REMOTE_SS_ACCESS1_UPPER                        (0xa0000000u)
#define PxD_VM_BUFFER_LOWER                                (0xa0000000u)
#define PxD_VM_BUFFER_UPPER                                (0xb0000000u)
#define PxD_P0_DM_RAM_LOWER                                (0x00000000u)
#define PxD_P0_DM_RAM_UPPER                                (0x00008000u)
#define PxD_P1_DM_RAM_LOWER                                (0x00010000u)
#define PxD_P1_DM_RAM_UPPER                                (0x00018000u)
#define PxD_SHARED_DM_RAM_LOWER                            (0x10000000u)
#define PxD_SHARED_DM_RAM_UPPER                            (0x10008000u)
#define PxD_TCM0_LOWER                                     (0x20000000u)
#define PxD_TCM0_UPPER                                     (0x20002000u)
#define PxD_TCM1_LOWER                                     (0x20002000u)
#define PxD_TCM1_UPPER                                     (0x20004000u)
#define PxD_P0_CACHE_DATA_RAM_LOWER                        (0x30000000u)
#define PxD_P0_CACHE_DATA_RAM_UPPER                        (0x30008000u)
#define PxD_P0_CACHE_DATA_RAM0_LOWER                       (0x30000000u)
#define PxD_P0_CACHE_DATA_RAM0_UPPER                       (0x30004000u)
#define PxD_P0_CACHE_DATA_RAM1_LOWER                       (0x30004000u)
#define PxD_P0_CACHE_DATA_RAM1_UPPER                       (0x30008000u)
#define PxD_P0_CACHE_TAG_RAM_LOWER                         (0x30008000u)
#define PxD_P0_CACHE_TAG_RAM_UPPER                         (0x30008800u)
#define PxD_P1_CACHE_DATA_RAM_LOWER                        (0x30030000u)
#define PxD_P1_CACHE_DATA_RAM_UPPER                        (0x30034000u)
#define PxD_P1_CACHE_DATA_RAM0_LOWER                       (0x30030000u)
#define PxD_P1_CACHE_DATA_RAM0_UPPER                       (0x30032000u)
#define PxD_P1_CACHE_DATA_RAM1_LOWER                       (0x30032000u)
#define PxD_P1_CACHE_DATA_RAM1_UPPER                       (0x30034000u)
#define PxD_P1_CACHE_TAG_RAM_LOWER                         (0x30034000u)
#define PxD_P1_CACHE_TAG_RAM_UPPER                         (0x30034400u)


/* Constants for APPS_SYS_CPUx_PM */
#define PxP_SQIFx_CACHED_LOWER                             (0x00000000u)
#define PxP_SQIFx_CACHED_UPPER                             (0x00800000u)
#define PxP_TCM0_LOWER                                     (0x00800000u)
#define PxP_TCM0_UPPER                                     (0x00802000u)
#define PxP_TCM1_LOWER                                     (0x00802000u)
#define PxP_TCM1_UPPER                                     (0x00804000u)


/* Constants for ADDR_SEL */
#define ADDR_SEL_MSB                                       (0x0000001fu)
#define ADDR_SEL_LSB                                       (0x0000001bu)
#define ADDR_SEL_PM_MSB                                    (0x00000018u)
#define ADDR_SEL_PM_LSB                                    (0x0000000du)
#define ADDR_SEL_CACHE_RAM_MSB                             (0x00000011u)
#define ADDR_SEL_CACHE_RAM_LSB                             (0x0000000au)


/* Constants for APPS_SYS_MEM */
#define MAP_REMOTE_REGISTER_SPACE_LOWER                    (0x60000000u)
#define MAP_REMOTE_REGISTER_SPACE_UPPER                    (0x60050000u)
#define MAP_SQIF0_CACHED_LOWER                             (0x70000000u)
#define MAP_SQIF0_CACHED_UPPER                             (0x70800000u)
#define MAP_SQIF1_CACHED_LOWER                             (0x78000000u)
#define MAP_SQIF1_CACHED_UPPER                             (0x78800000u)
#define MAP_SQIF0_RAM_LOWER                                (0x80000000u)
#define MAP_SQIF0_RAM_UPPER                                (0x80800000u)
#define MAP_SQIF1_RAM_LOWER                                (0x88000000u)
#define MAP_SQIF1_RAM_UPPER                                (0x88800000u)
#define MAP_REMOTE_SS_ACCESS0_LOWER                        (0x90000000u)
#define MAP_REMOTE_SS_ACCESS0_UPPER                        (0x98000000u)
#define MAP_REMOTE_SS_ACCESS1_LOWER                        (0x98000000u)
#define MAP_REMOTE_SS_ACCESS1_UPPER                        (0xa0000000u)
#define MAP_VM_BUFFER_LOWER                                (0xa0000000u)
#define MAP_VM_BUFFER_UPPER                                (0xb0000000u)
#define MAP_SQIF0_DIRECT_LOWER                             (0xb0000000u)
#define MAP_SQIF0_DIRECT_UPPER                             (0xd0000000u)
#define MAP_SQIF01_DIRECT_LOWER                            (0xd0000000u)
#define MAP_SQIF01_DIRECT_UPPER                            (0xf0000000u)
#define MAP_REGISTER_SPACE_LOWER                           (0xffffa000u)
#define MAP_REGISTER_SPACE_UPPER                           (0xfffffffcu)
#define MAP_P0_DM_RAM_LOWER                                (0x00000000u)
#define MAP_P0_DM_RAM_UPPER                                (0x00008000u)
#define MAP_P1_DM_RAM_LOWER                                (0x00010000u)
#define MAP_P1_DM_RAM_UPPER                                (0x00018000u)
#define MAP_TCM0_LOWER                                     (0x20000000u)
#define MAP_TCM0_UPPER                                     (0x20002000u)
#define MAP_TCM1_LOWER                                     (0x20002000u)
#define MAP_TCM1_UPPER                                     (0x20004000u)
#define MAP_P0_CACHE_DATA_RAM_LOWER                        (0x30000000u)
#define MAP_P0_CACHE_DATA_RAM_UPPER                        (0x30008000u)
#define MAP_P0_CACHE_DATA_RAM0_LOWER                       (0x30000000u)
#define MAP_P0_CACHE_DATA_RAM0_UPPER                       (0x30004000u)
#define MAP_P0_CACHE_DATA_RAM1_LOWER                       (0x30004000u)
#define MAP_P0_CACHE_DATA_RAM1_UPPER                       (0x30008000u)
#define MAP_P0_CACHE_TAG_RAM_LOWER                         (0x30008000u)
#define MAP_P0_CACHE_TAG_RAM_UPPER                         (0x30008800u)
#define MAP_P1_CACHE_DATA_RAM_LOWER                        (0x30030000u)
#define MAP_P1_CACHE_DATA_RAM_UPPER                        (0x30034000u)
#define MAP_P1_CACHE_DATA_RAM0_LOWER                       (0x30030000u)
#define MAP_P1_CACHE_DATA_RAM0_UPPER                       (0x30032000u)
#define MAP_P1_CACHE_DATA_RAM1_LOWER                       (0x30032000u)
#define MAP_P1_CACHE_DATA_RAM1_UPPER                       (0x30034000u)
#define MAP_P1_CACHE_TAG_RAM_LOWER                         (0x30034000u)
#define MAP_P1_CACHE_TAG_RAM_UPPER                         (0x30034400u)
#define MAP_SHARED_DM_RAM_LOWER                            (0x10000000u)
#define MAP_SHARED_DM_RAM_UPPER                            (0x10008000u)


#endif /* IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU_MEMORY_MAP */

#if defined(IO_DEFS_MODULE_K32_PREFETCH) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_PREFETCH
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_PREFETCH

/* -- k32_prefetch -- Kalimba 32-bit Prefetch Control registers. -- */

enum prefetch_config_posn_enum
{
   PREFETCH_CONFIG_COUNTERS_EN_POSN                   = (int)0,
   PREFETCH_CONFIG_PREFETCH_CONFIG_COUNTERS_EN_LSB_POSN = (int)0,
   PREFETCH_CONFIG_PREFETCH_CONFIG_COUNTERS_EN_MSB_POSN = (int)0
};
typedef enum prefetch_config_posn_enum prefetch_config_posn;

#define PREFETCH_CONFIG_COUNTERS_EN_MASK         (0x00000001u)

enum prefetch_debug_posn_enum
{
   PREFETCH_DEBUG_READ_EN_IN_POSN                     = (int)0,
   PREFETCH_DEBUG_PREFETCH_DEBUG_READ_EN_IN_LSB_POSN  = (int)0,
   PREFETCH_DEBUG_PREFETCH_DEBUG_READ_EN_IN_MSB_POSN  = (int)0,
   PREFETCH_DEBUG_WAIT_OUT_POSN                       = (int)1,
   PREFETCH_DEBUG_PREFETCH_DEBUG_WAIT_OUT_LSB_POSN    = (int)1,
   PREFETCH_DEBUG_PREFETCH_DEBUG_WAIT_OUT_MSB_POSN    = (int)1,
   PREFETCH_DEBUG_READ_EN_OUT_POSN                    = (int)2,
   PREFETCH_DEBUG_PREFETCH_DEBUG_READ_EN_OUT_LSB_POSN = (int)2,
   PREFETCH_DEBUG_PREFETCH_DEBUG_READ_EN_OUT_MSB_POSN = (int)2,
   PREFETCH_DEBUG_WAIT_IN_POSN                        = (int)3,
   PREFETCH_DEBUG_PREFETCH_DEBUG_WAIT_IN_LSB_POSN     = (int)3,
   PREFETCH_DEBUG_PREFETCH_DEBUG_WAIT_IN_MSB_POSN     = (int)3,
   PREFETCH_DEBUG_MEM_REQUEST_POSN                    = (int)4,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MEM_REQUEST_LSB_POSN = (int)4,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MEM_REQUEST_MSB_POSN = (int)4,
   PREFETCH_DEBUG_NEXT_MEMREQUEST_POSN                = (int)5,
   PREFETCH_DEBUG_PREFETCH_DEBUG_NEXT_MEMREQUEST_LSB_POSN = (int)5,
   PREFETCH_DEBUG_PREFETCH_DEBUG_NEXT_MEMREQUEST_MSB_POSN = (int)5,
   PREFETCH_DEBUG_PMEM_REQUEST_POSN                   = (int)6,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PMEM_REQUEST_LSB_POSN = (int)6,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PMEM_REQUEST_MSB_POSN = (int)6,
   PREFETCH_DEBUG_PM_WAIT_IN_PREV_POSN                = (int)7,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PM_WAIT_IN_PREV_LSB_POSN = (int)7,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PM_WAIT_IN_PREV_MSB_POSN = (int)7,
   PREFETCH_DEBUG_MEM_REQUEST_REG_POSN                = (int)8,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MEM_REQUEST_REG_LSB_POSN = (int)8,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MEM_REQUEST_REG_MSB_POSN = (int)8,
   PREFETCH_DEBUG_ALOW_PREFETCHING_LSB_POSN           = (int)9,
   PREFETCH_DEBUG_PREFETCH_DEBUG_ALOW_PREFETCHING_LSB_POSN = (int)9,
   PREFETCH_DEBUG_ALOW_PREFETCHING_MSB_POSN           = (int)10,
   PREFETCH_DEBUG_PREFETCH_DEBUG_ALOW_PREFETCHING_MSB_POSN = (int)10,
   PREFETCH_DEBUG_MISS_SEQ_T1_POSN                    = (int)11,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MISS_SEQ_T1_LSB_POSN = (int)11,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MISS_SEQ_T1_MSB_POSN = (int)11,
   PREFETCH_DEBUG_MISS_SEQ_T2_POSN                    = (int)12,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MISS_SEQ_T2_LSB_POSN = (int)12,
   PREFETCH_DEBUG_PREFETCH_DEBUG_MISS_SEQ_T2_MSB_POSN = (int)12,
   PREFETCH_DEBUG_VALID_PREFETCH_DATA_POSN            = (int)13,
   PREFETCH_DEBUG_PREFETCH_DEBUG_VALID_PREFETCH_DATA_LSB_POSN = (int)13,
   PREFETCH_DEBUG_PREFETCH_DEBUG_VALID_PREFETCH_DATA_MSB_POSN = (int)13,
   PREFETCH_DEBUG_PREFETCH_VALID_POSN                 = (int)14,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_VALID_LSB_POSN = (int)14,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_VALID_MSB_POSN = (int)14,
   PREFETCH_DEBUG_PREFETCH_HIT_LSB_POSN               = (int)15,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_HIT_LSB_POSN = (int)15,
   PREFETCH_DEBUG_PREFETCH_HIT_MSB_POSN               = (int)19,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_HIT_MSB_POSN = (int)19,
   PREFETCH_DEBUG_PREFETCH_VALIDS_LSB_POSN            = (int)20,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_VALIDS_LSB_POSN = (int)20,
   PREFETCH_DEBUG_PREFETCH_VALIDS_MSB_POSN            = (int)24,
   PREFETCH_DEBUG_PREFETCH_DEBUG_PREFETCH_VALIDS_MSB_POSN = (int)24
};
typedef enum prefetch_debug_posn_enum prefetch_debug_posn;

#define PREFETCH_DEBUG_READ_EN_IN_MASK           (0x00000001u)
#define PREFETCH_DEBUG_WAIT_OUT_MASK             (0x00000002u)
#define PREFETCH_DEBUG_READ_EN_OUT_MASK          (0x00000004u)
#define PREFETCH_DEBUG_WAIT_IN_MASK              (0x00000008u)
#define PREFETCH_DEBUG_MEM_REQUEST_MASK          (0x00000010u)
#define PREFETCH_DEBUG_NEXT_MEMREQUEST_MASK      (0x00000020u)
#define PREFETCH_DEBUG_PMEM_REQUEST_MASK         (0x00000040u)
#define PREFETCH_DEBUG_PM_WAIT_IN_PREV_MASK      (0x00000080u)
#define PREFETCH_DEBUG_MEM_REQUEST_REG_MASK      (0x00000100u)
#define PREFETCH_DEBUG_ALOW_PREFETCHING_LSB_MASK (0x00000200u)
#define PREFETCH_DEBUG_ALOW_PREFETCHING_MSB_MASK (0x00000400u)
#define PREFETCH_DEBUG_MISS_SEQ_T1_MASK          (0x00000800u)
#define PREFETCH_DEBUG_MISS_SEQ_T2_MASK          (0x00001000u)
#define PREFETCH_DEBUG_VALID_PREFETCH_DATA_MASK  (0x00002000u)
#define PREFETCH_DEBUG_PREFETCH_VALID_MASK       (0x00004000u)
#define PREFETCH_DEBUG_PREFETCH_HIT_LSB_MASK     (0x00008000u)
#define PREFETCH_DEBUG_PREFETCH_HIT_MSB_MASK     (0x00080000u)
#define PREFETCH_DEBUG_PREFETCH_VALIDS_LSB_MASK  (0x00100000u)
#define PREFETCH_DEBUG_PREFETCH_VALIDS_MSB_MASK  (0x01000000u)

enum prefetch_debug_addr_posn_enum
{
   PREFETCH_DEBUG_PMADDRIN_LSB_POSN                   = (int)0,
   PREFETCH_DEBUG_ADDR_PREFETCH_DEBUG_PMADDRIN_LSB_POSN = (int)0,
   PREFETCH_DEBUG_PMADDRIN_MSB_POSN                   = (int)15,
   PREFETCH_DEBUG_ADDR_PREFETCH_DEBUG_PMADDRIN_MSB_POSN = (int)15,
   PREFETCH_DEBUG_PMADDROUT_LSB_POSN                  = (int)16,
   PREFETCH_DEBUG_ADDR_PREFETCH_DEBUG_PMADDROUT_LSB_POSN = (int)16,
   PREFETCH_DEBUG_PMADDROUT_MSB_POSN                  = (int)31,
   PREFETCH_DEBUG_ADDR_PREFETCH_DEBUG_PMADDROUT_MSB_POSN = (int)31
};
typedef enum prefetch_debug_addr_posn_enum prefetch_debug_addr_posn;

#define PREFETCH_DEBUG_PMADDRIN_LSB_MASK         (0x00000001u)
#define PREFETCH_DEBUG_PMADDRIN_MSB_MASK         (0x00008000u)
#define PREFETCH_DEBUG_PMADDROUT_LSB_MASK        (0x00010000u)
#define PREFETCH_DEBUG_PMADDROUT_MSB_MASK        (0x80000000u)

enum prefetch_flush_posn_enum
{
   PREFETCH_FLUSH_POSN                                = (int)0,
   PREFETCH_FLUSH_PREFETCH_FLUSH_LSB_POSN             = (int)0,
   PREFETCH_FLUSH_PREFETCH_FLUSH_MSB_POSN             = (int)0
};
typedef enum prefetch_flush_posn_enum prefetch_flush_posn;

#define PREFETCH_FLUSH_MASK                      (0x00000001u)

enum prefetch_prefetch_count_posn_enum
{
   PREFETCH_PREFETCH_COUNT_LSB_POSN                   = (int)0,
   PREFETCH_PREFETCH_COUNT_PREFETCH_PREFETCH_COUNT_LSB_POSN = (int)0,
   PREFETCH_PREFETCH_COUNT_MSB_POSN                   = (int)31,
   PREFETCH_PREFETCH_COUNT_PREFETCH_PREFETCH_COUNT_MSB_POSN = (int)31
};
typedef enum prefetch_prefetch_count_posn_enum prefetch_prefetch_count_posn;

#define PREFETCH_PREFETCH_COUNT_LSB_MASK         (0x00000001u)
#define PREFETCH_PREFETCH_COUNT_MSB_MASK         (0x80000000u)

enum prefetch_request_count_posn_enum
{
   PREFETCH_REQUEST_COUNT_LSB_POSN                    = (int)0,
   PREFETCH_REQUEST_COUNT_PREFETCH_REQUEST_COUNT_LSB_POSN = (int)0,
   PREFETCH_REQUEST_COUNT_MSB_POSN                    = (int)31,
   PREFETCH_REQUEST_COUNT_PREFETCH_REQUEST_COUNT_MSB_POSN = (int)31
};
typedef enum prefetch_request_count_posn_enum prefetch_request_count_posn;

#define PREFETCH_REQUEST_COUNT_LSB_MASK          (0x00000001u)
#define PREFETCH_REQUEST_COUNT_MSB_MASK          (0x80000000u)

enum prefetch_wait_out_count_posn_enum
{
   PREFETCH_WAIT_OUT_COUNT_LSB_POSN                   = (int)0,
   PREFETCH_WAIT_OUT_COUNT_PREFETCH_WAIT_OUT_COUNT_LSB_POSN = (int)0,
   PREFETCH_WAIT_OUT_COUNT_MSB_POSN                   = (int)31,
   PREFETCH_WAIT_OUT_COUNT_PREFETCH_WAIT_OUT_COUNT_MSB_POSN = (int)31
};
typedef enum prefetch_wait_out_count_posn_enum prefetch_wait_out_count_posn;

#define PREFETCH_WAIT_OUT_COUNT_LSB_MASK         (0x00000001u)
#define PREFETCH_WAIT_OUT_COUNT_MSB_MASK         (0x80000000u)

#endif /* IO_DEFS_MODULE_K32_PREFETCH */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_PREFETCH */


#if defined(IO_DEFS_MODULE_APPS_SYS_CPU0_VM) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU0_VM
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU0_VM

/* -- apps_sys_cpu0_vm -- Apps-specific virtual memory registers -- */

enum mmu_buffer_size_enum
{
   MMU_BUFFER_SIZE_128                      = (int)0x0,
   MMU_BUFFER_SIZE_256                      = (int)0x1,
   MMU_BUFFER_SIZE_512                      = (int)0x2,
   MMU_BUFFER_SIZE_1024                     = (int)0x3,
   MMU_BUFFER_SIZE_2048                     = (int)0x4,
   MMU_BUFFER_SIZE_4096                     = (int)0x5,
   MMU_BUFFER_SIZE_8192                     = (int)0x6,
   MMU_BUFFER_SIZE_16384                    = (int)0x7,
   MMU_BUFFER_SIZE_32K                      = (int)0x8,
   MMU_BUFFER_SIZE_64K                      = (int)0x9,
   MMU_BUFFER_SIZE_128K                     = (int)0xA,
   MMU_BUFFER_SIZE_256K                     = (int)0xB,
   MMU_BUFFER_SIZE_512K                     = (int)0xC,
   MAX_MMU_BUFFER_SIZE                      = (int)0xC
};
#define NUM_MMU_BUFFER_SIZE (0xD)
typedef enum mmu_buffer_size_enum mmu_buffer_size;


enum mmu_vm_lookup_error_flags_posn_enum
{
   MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_POSN       = (int)0,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_LSB_POSN = (int)0,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_MSB_POSN = (int)0,
   MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_POSN  = (int)1,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_LSB_POSN = (int)1,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_MSB_POSN = (int)1,
   MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_POSN      = (int)2,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_LSB_POSN = (int)2,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_MSB_POSN = (int)2,
   MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_POSN         = (int)3,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_LSB_POSN = (int)3,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_MSB_POSN = (int)3,
   MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_LSB_POSN          = (int)4,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_LSB_POSN = (int)4,
   MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_MSB_POSN          = (int)11,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_MSB_POSN = (int)11,
   MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_LSB_POSN       = (int)12,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_LSB_POSN = (int)12,
   MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_MSB_POSN       = (int)15,
   MMU_VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_MSB_POSN = (int)15
};
typedef enum mmu_vm_lookup_error_flags_posn_enum mmu_vm_lookup_error_flags_posn;

#define MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_MASK (0x00000001u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_MASK (0x00000002u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_MASK (0x00000004u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_MASK (0x00000008u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_LSB_MASK (0x00000010u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_MSB_MASK (0x00000800u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_LSB_MASK (0x00001000u)
#define MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_MSB_MASK (0x00008000u)

enum vm_address_enum_posn_enum
{
   VM_ADDRESS_OFFSET_LSB_POSN                         = (int)0,
   VM_ADDRESS_ENUM_VM_ADDRESS_OFFSET_LSB_POSN         = (int)0,
   VM_ADDRESS_OFFSET_MSB_POSN                         = (int)18,
   VM_ADDRESS_ENUM_VM_ADDRESS_OFFSET_MSB_POSN         = (int)18,
   VM_ADDRESS_UNUSED_POSN                             = (int)19,
   VM_ADDRESS_ENUM_VM_ADDRESS_UNUSED_LSB_POSN         = (int)19,
   VM_ADDRESS_ENUM_VM_ADDRESS_UNUSED_MSB_POSN         = (int)19,
   VM_ADDRESS_HANDLE_LSB_POSN                         = (int)20,
   VM_ADDRESS_ENUM_VM_ADDRESS_HANDLE_LSB_POSN         = (int)20,
   VM_ADDRESS_HANDLE_MSB_POSN                         = (int)27,
   VM_ADDRESS_ENUM_VM_ADDRESS_HANDLE_MSB_POSN         = (int)27,
   VM_ADDRESS_RESERVED_LSB_POSN                       = (int)28,
   VM_ADDRESS_ENUM_VM_ADDRESS_RESERVED_LSB_POSN       = (int)28,
   VM_ADDRESS_RESERVED_MSB_POSN                       = (int)31,
   VM_ADDRESS_ENUM_VM_ADDRESS_RESERVED_MSB_POSN       = (int)31
};
typedef enum vm_address_enum_posn_enum vm_address_enum_posn;

#define VM_ADDRESS_OFFSET_LSB_MASK               (0x00000001u)
#define VM_ADDRESS_OFFSET_MSB_MASK               (0x00040000u)
#define VM_ADDRESS_UNUSED_MASK                   (0x00080000u)
#define VM_ADDRESS_HANDLE_LSB_MASK               (0x00100000u)
#define VM_ADDRESS_HANDLE_MSB_MASK               (0x08000000u)
#define VM_ADDRESS_RESERVED_LSB_MASK             (0x10000000u)
#define VM_ADDRESS_RESERVED_MSB_MASK             (0x80000000u)

enum vm_buffer_access_do_action_enum_enum
{
   VM_BUFFER_ACCESS_DO_ACTION_READ          = (int)0x0,
   VM_BUFFER_ACCESS_DO_ACTION_WRITE         = (int)0x1,
   MAX_VM_BUFFER_ACCESS_DO_ACTION_ENUM      = (int)0x1
};
#define NUM_VM_BUFFER_ACCESS_DO_ACTION_ENUM (0x2)
typedef enum vm_buffer_access_do_action_enum_enum vm_buffer_access_do_action_enum;


enum vm_buffer_access_handle_enum_posn_enum
{
   VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN            = (int)0,
   VM_BUFFER_ACCESS_HANDLE_ENUM_VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN            = (int)7,
   VM_BUFFER_ACCESS_HANDLE_ENUM_VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN = (int)7,
   VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN         = (int)8,
   VM_BUFFER_ACCESS_HANDLE_ENUM_VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN = (int)8,
   VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN         = (int)11,
   VM_BUFFER_ACCESS_HANDLE_ENUM_VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN = (int)11
};
typedef enum vm_buffer_access_handle_enum_posn_enum vm_buffer_access_handle_enum_posn;

#define VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_MASK  (0x00000001u)
#define VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_MASK  (0x00000080u)
#define VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_MASK (0x00000100u)
#define VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_MASK (0x00000800u)

enum vm_buffer_access_status_enum_posn_enum
{
   VM_BUFFER_ACCESS_STATUS_ACTIVE_POSN                = (int)0,
   VM_BUFFER_ACCESS_STATUS_ENUM_VM_BUFFER_ACCESS_STATUS_ACTIVE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_STATUS_ENUM_VM_BUFFER_ACCESS_STATUS_ACTIVE_MSB_POSN = (int)0,
   VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN            = (int)1,
   VM_BUFFER_ACCESS_STATUS_ENUM_VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN = (int)1,
   VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN            = (int)4,
   VM_BUFFER_ACCESS_STATUS_ENUM_VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN = (int)4
};
typedef enum vm_buffer_access_status_enum_posn_enum vm_buffer_access_status_enum_posn;

#define VM_BUFFER_ACCESS_STATUS_ACTIVE_MASK      (0x00000001u)
#define VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_MASK  (0x00000002u)
#define VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_MASK  (0x00000010u)

enum vm_cpu0_requests_enum_posn_enum
{
   VM_CPU0_REQUESTS_WRITE_POSN                        = (int)0,
   VM_CPU0_REQUESTS_ENUM_VM_CPU0_REQUESTS_WRITE_LSB_POSN = (int)0,
   VM_CPU0_REQUESTS_ENUM_VM_CPU0_REQUESTS_WRITE_MSB_POSN = (int)0,
   VM_CPU0_REQUESTS_READ_POSN                         = (int)1,
   VM_CPU0_REQUESTS_ENUM_VM_CPU0_REQUESTS_READ_LSB_POSN = (int)1,
   VM_CPU0_REQUESTS_ENUM_VM_CPU0_REQUESTS_READ_MSB_POSN = (int)1
};
typedef enum vm_cpu0_requests_enum_posn_enum vm_cpu0_requests_enum_posn;

#define VM_CPU0_REQUESTS_WRITE_MASK              (0x00000001u)
#define VM_CPU0_REQUESTS_READ_MASK               (0x00000002u)

enum vm_requests_remote_config_enum_posn_enum
{
   VM_REQUESTS_REMOTE_CONFIG_HANDLE_LSB_POSN          = (int)0,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_HANDLE_LSB_POSN = (int)0,
   VM_REQUESTS_REMOTE_CONFIG_HANDLE_MSB_POSN          = (int)7,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_HANDLE_MSB_POSN = (int)7,
   VM_REQUESTS_REMOTE_CONFIG_SUBSYS_LSB_POSN          = (int)8,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_LSB_POSN = (int)8,
   VM_REQUESTS_REMOTE_CONFIG_SUBSYS_MSB_POSN          = (int)11,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_MSB_POSN = (int)11,
   VM_REQUESTS_REMOTE_CONFIG_EN_POSN                  = (int)12,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_EN_LSB_POSN = (int)12,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_EN_MSB_POSN = (int)12,
   VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_POSN = (int)13,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_LSB_POSN = (int)13,
   VM_REQUESTS_REMOTE_CONFIG_ENUM_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MSB_POSN = (int)13
};
typedef enum vm_requests_remote_config_enum_posn_enum vm_requests_remote_config_enum_posn;

#define VM_REQUESTS_REMOTE_CONFIG_HANDLE_LSB_MASK (0x00000001u)
#define VM_REQUESTS_REMOTE_CONFIG_HANDLE_MSB_MASK (0x00000080u)
#define VM_REQUESTS_REMOTE_CONFIG_SUBSYS_LSB_MASK (0x00000100u)
#define VM_REQUESTS_REMOTE_CONFIG_SUBSYS_MSB_MASK (0x00000800u)
#define VM_REQUESTS_REMOTE_CONFIG_EN_MASK        (0x00001000u)
#define VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MASK (0x00002000u)

enum vm_requests_status_enum_posn_enum
{
   VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN           = (int)0,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN           = (int)7,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN           = (int)8,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN           = (int)11,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_STATUS_ACTIVE_POSN                      = (int)12,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   VM_REQUEST_STATUS_ACCESS_DENIED_POSN               = (int)13,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN           = (int)14,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN           = (int)17,
   VM_REQUESTS_STATUS_ENUM_VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum vm_requests_status_enum_posn_enum vm_requests_status_enum_posn;

#define VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_MASK (0x00000001u)
#define VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_MASK (0x00000080u)
#define VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_MASK (0x00000100u)
#define VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_MASK (0x00000800u)
#define VM_REQUEST_STATUS_ACTIVE_MASK            (0x00001000u)
#define VM_REQUEST_STATUS_ACCESS_DENIED_MASK     (0x00002000u)
#define VM_REQUEST_STATUS_BUFFER_STATUS_LSB_MASK (0x00004000u)
#define VM_REQUEST_STATUS_BUFFER_STATUS_MSB_MASK (0x00020000u)

enum vm_debug_select_enum
{
   VM_DEBUG_SELECT_REQUESTS_CPU0_WRITE      = (int)0x1,
   VM_DEBUG_SELECT_REQUESTS_CPU0_READ       = (int)0x2,
   VM_DEBUG_SELECT_REQUESTS_CPU1_WRITE      = (int)0x3,
   VM_DEBUG_SELECT_REQUESTS_CPU1_READ       = (int)0x4,
   VM_DEBUG_SELECT_LOOKUP0_DEBUG1           = (int)0x5,
   VM_DEBUG_SELECT_LOOKUP0_DEBUG2           = (int)0x6,
   VM_DEBUG_SELECT_LOOKUP0_DEBUG3           = (int)0x7,
   VM_DEBUG_SELECT_LOOKUP1_DEBUG1           = (int)0x8,
   VM_DEBUG_SELECT_LOOKUP1_DEBUG2           = (int)0x9,
   VM_DEBUG_SELECT_LOOKUP1_DEBUG3           = (int)0xA,
   VM_DEBUG_SELECT_LOOKUP2_DEBUG1           = (int)0xB,
   VM_DEBUG_SELECT_LOOKUP2_DEBUG2           = (int)0xC,
   VM_DEBUG_SELECT_LOOKUP2_DEBUG3           = (int)0xD,
   VM_DEBUG_SELECT_LOOKUP3_DEBUG1           = (int)0xE,
   VM_DEBUG_SELECT_LOOKUP3_DEBUG2           = (int)0xF,
   VM_DEBUG_SELECT_LOOKUP3_DEBUG3           = (int)0x10,
   VM_DEBUG_SELECT_ARBITER                  = (int)0x11,
   VM_DEBUG_SELECT_RAM_ACCESS               = (int)0x12,
   VM_DEBUG_SELECT_VM_ACCESSORS             = (int)0x13,
   VM_DEBUG_SELECT_ERROR_INDICATIONS        = (int)0x14,
   VM_DEBUG_BUFFER_OFFSET_ACCESS_CPU0       = (int)0x15,
   VM_DEBUG_BUFFER_OFFSET_ACCESS_CPU1       = (int)0x16,
   MAX_VM_DEBUG_SELECT                      = (int)0x16
};
typedef enum vm_debug_select_enum vm_debug_select;


enum vm_buffer_access_cpu0_offset_read_posn_enum
{
   VM_BUFFER_ACCESS_CPU0_OFFSET_READ_LSB_POSN         = (int)0,
   VM_BUFFER_ACCESS_CPU0_OFFSET_READ_VM_BUFFER_ACCESS_CPU0_OFFSET_READ_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU0_OFFSET_READ_MSB_POSN         = (int)17,
   VM_BUFFER_ACCESS_CPU0_OFFSET_READ_VM_BUFFER_ACCESS_CPU0_OFFSET_READ_MSB_POSN = (int)17
};
typedef enum vm_buffer_access_cpu0_offset_read_posn_enum vm_buffer_access_cpu0_offset_read_posn;

#define VM_BUFFER_ACCESS_CPU0_OFFSET_READ_LSB_MASK (0x00000001u)
#define VM_BUFFER_ACCESS_CPU0_OFFSET_READ_MSB_MASK (0x00020000u)

enum vm_buffer_access_cpu0_offset_write_posn_enum
{
   VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_LSB_POSN        = (int)0,
   VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_MSB_POSN        = (int)17,
   VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_MSB_POSN = (int)17
};
typedef enum vm_buffer_access_cpu0_offset_write_posn_enum vm_buffer_access_cpu0_offset_write_posn;

#define VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_LSB_MASK (0x00000001u)
#define VM_BUFFER_ACCESS_CPU0_OFFSET_WRITE_MSB_MASK (0x00020000u)

enum vm_free_list_addr_posn_enum
{
   VM_FREE_LIST_ADDR_LSB_POSN                         = (int)0,
   VM_FREE_LIST_ADDR_VM_FREE_LIST_ADDR_LSB_POSN       = (int)0,
   VM_FREE_LIST_ADDR_MSB_POSN                         = (int)31,
   VM_FREE_LIST_ADDR_VM_FREE_LIST_ADDR_MSB_POSN       = (int)31
};
typedef enum vm_free_list_addr_posn_enum vm_free_list_addr_posn;

#define VM_FREE_LIST_ADDR_LSB_MASK               (0x00000001u)
#define VM_FREE_LIST_ADDR_MSB_MASK               (0x80000000u)

enum vm_free_list_add_page_posn_enum
{
   VM_FREE_LIST_ADD_PAGE_LSB_POSN                     = (int)0,
   VM_FREE_LIST_ADD_PAGE_VM_FREE_LIST_ADD_PAGE_LSB_POSN = (int)0,
   VM_FREE_LIST_ADD_PAGE_MSB_POSN                     = (int)15,
   VM_FREE_LIST_ADD_PAGE_VM_FREE_LIST_ADD_PAGE_MSB_POSN = (int)15
};
typedef enum vm_free_list_add_page_posn_enum vm_free_list_add_page_posn;

#define VM_FREE_LIST_ADD_PAGE_LSB_MASK           (0x00000001u)
#define VM_FREE_LIST_ADD_PAGE_MSB_MASK           (0x00008000u)

enum vm_free_list_end_addr_posn_enum
{
   VM_FREE_LIST_END_ADDR_LSB_POSN                     = (int)0,
   VM_FREE_LIST_END_ADDR_VM_FREE_LIST_END_ADDR_LSB_POSN = (int)0,
   VM_FREE_LIST_END_ADDR_MSB_POSN                     = (int)31,
   VM_FREE_LIST_END_ADDR_VM_FREE_LIST_END_ADDR_MSB_POSN = (int)31
};
typedef enum vm_free_list_end_addr_posn_enum vm_free_list_end_addr_posn;

#define VM_FREE_LIST_END_ADDR_LSB_MASK           (0x00000001u)
#define VM_FREE_LIST_END_ADDR_MSB_MASK           (0x80000000u)

enum vm_free_list_next_addr_posn_enum
{
   VM_FREE_LIST_NEXT_ADDR_LSB_POSN                    = (int)0,
   VM_FREE_LIST_NEXT_ADDR_VM_FREE_LIST_NEXT_ADDR_LSB_POSN = (int)0,
   VM_FREE_LIST_NEXT_ADDR_MSB_POSN                    = (int)31,
   VM_FREE_LIST_NEXT_ADDR_VM_FREE_LIST_NEXT_ADDR_MSB_POSN = (int)31
};
typedef enum vm_free_list_next_addr_posn_enum vm_free_list_next_addr_posn;

#define VM_FREE_LIST_NEXT_ADDR_LSB_MASK          (0x00000001u)
#define VM_FREE_LIST_NEXT_ADDR_MSB_MASK          (0x80000000u)

enum vm_interconnect_mux_debug_enable_posn_enum
{
   VM_INTERCONNECT_MUX_DEBUG_ENABLE_POSN              = (int)0,
   VM_INTERCONNECT_MUX_DEBUG_ENABLE_VM_INTERCONNECT_MUX_DEBUG_ENABLE_LSB_POSN = (int)0,
   VM_INTERCONNECT_MUX_DEBUG_ENABLE_VM_INTERCONNECT_MUX_DEBUG_ENABLE_MSB_POSN = (int)0
};
typedef enum vm_interconnect_mux_debug_enable_posn_enum vm_interconnect_mux_debug_enable_posn;

#define VM_INTERCONNECT_MUX_DEBUG_ENABLE_MASK    (0x00000001u)

enum vm_lookup_allow_reading_from_unmapped_pages_posn_enum
{
   VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_POSN   = (int)0,
   VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_LSB_POSN = (int)0,
   VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_MSB_POSN = (int)0
};
typedef enum vm_lookup_allow_reading_from_unmapped_pages_posn_enum vm_lookup_allow_reading_from_unmapped_pages_posn;

#define VM_LOOKUP_ALLOW_READING_FROM_UNMAPPED_PAGES_MASK (0x00000001u)

enum vm_lookup_int_events_sticky_posn_enum
{
   VM_LOOKUP_INT_EVENTS_STICKY_LSB_POSN               = (int)0,
   VM_LOOKUP_INT_EVENTS_STICKY_VM_LOOKUP_INT_EVENTS_STICKY_LSB_POSN = (int)0,
   VM_LOOKUP_INT_EVENTS_STICKY_MSB_POSN               = (int)15,
   VM_LOOKUP_INT_EVENTS_STICKY_VM_LOOKUP_INT_EVENTS_STICKY_MSB_POSN = (int)15
};
typedef enum vm_lookup_int_events_sticky_posn_enum vm_lookup_int_events_sticky_posn;

#define VM_LOOKUP_INT_EVENTS_STICKY_LSB_MASK     (0x00000001u)
#define VM_LOOKUP_INT_EVENTS_STICKY_MSB_MASK     (0x00008000u)

enum vm_lookup_select_posn_enum
{
   VM_LOOKUP_SELECT_LSB_POSN                          = (int)0,
   VM_LOOKUP_SELECT_VM_LOOKUP_SELECT_LSB_POSN         = (int)0,
   VM_LOOKUP_SELECT_MSB_POSN                          = (int)3,
   VM_LOOKUP_SELECT_VM_LOOKUP_SELECT_MSB_POSN         = (int)3
};
typedef enum vm_lookup_select_posn_enum vm_lookup_select_posn;

#define VM_LOOKUP_SELECT_LSB_MASK                (0x00000001u)
#define VM_LOOKUP_SELECT_MSB_MASK                (0x00000008u)

enum vm_request_cpu1_requests_remote_config_denied_status_clear_posn_enum
{
   VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_POSN = (int)0,
   VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_LSB_POSN = (int)0,
   VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MSB_POSN = (int)0
};
typedef enum vm_request_cpu1_requests_remote_config_denied_status_clear_posn_enum vm_request_cpu1_requests_remote_config_denied_status_clear_posn;

#define VM_REQUEST_CPU1_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MASK (0x00000001u)

enum apps_sys_cpu0_vm__access_ctrl_enum_posn_enum
{
   APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_POSN        = (int)0,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_POSN        = (int)1,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_POSN        = (int)2,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_POSN        = (int)3,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_CPU0_VM__ACCESS_CTRL_ENUM_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_cpu0_vm__access_ctrl_enum_posn_enum apps_sys_cpu0_vm__access_ctrl_enum_posn;

#define APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum apps_sys_cpu0_vm__p0_access_permission_enum
{
   APPS_SYS_CPU0_VM__P0_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU0_VM__P0_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu0_vm__p0_access_permission_enum apps_sys_cpu0_vm__p0_access_permission;


enum apps_sys_cpu0_vm__p1_access_permission_enum
{
   APPS_SYS_CPU0_VM__P1_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU0_VM__P1_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu0_vm__p1_access_permission_enum apps_sys_cpu0_vm__p1_access_permission;


enum apps_sys_cpu0_vm__p2_access_permission_enum
{
   APPS_SYS_CPU0_VM__P2_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU0_VM__P2_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu0_vm__p2_access_permission_enum apps_sys_cpu0_vm__p2_access_permission;


enum apps_sys_cpu0_vm__p3_access_permission_enum
{
   APPS_SYS_CPU0_VM__P3_ACCESS_BLOCKED      = (int)0x0,
   APPS_SYS_CPU0_VM__P3_ACCESS_UNBLOCKED    = (int)0x1,
   MAX_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_cpu0_vm__p3_access_permission_enum apps_sys_cpu0_vm__p3_access_permission;


enum apps_sys_cpu0_vm__mutex_lock_enum_enum
{
   APPS_SYS_CPU0_VM__MUTEX_AVAILABLE        = (int)0x0,
   APPS_SYS_CPU0_VM__MUTEX_CLAIMED_BY_P0    = (int)0x1,
   APPS_SYS_CPU0_VM__MUTEX_CLAIMED_BY_P1    = (int)0x2,
   APPS_SYS_CPU0_VM__MUTEX_CLAIMED_BY_P2    = (int)0x4,
   APPS_SYS_CPU0_VM__MUTEX_CLAIMED_BY_P3    = (int)0x8,
   APPS_SYS_CPU0_VM__MUTEX_DISABLED         = (int)0xF,
   MAX_APPS_SYS_CPU0_VM__MUTEX_LOCK_ENUM    = (int)0xF
};
typedef enum apps_sys_cpu0_vm__mutex_lock_enum_enum apps_sys_cpu0_vm__mutex_lock_enum;


enum vm_buffer_access_cpu0_handle_posn_enum
{
   VM_BUFFER_ACCESS_CPU0_HANDLE_VM_BUFFER_ACCESS_HANDLE_HANDLE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU0_HANDLE_VM_BUFFER_ACCESS_HANDLE_HANDLE_MSB_POSN = (int)7,
   VM_BUFFER_ACCESS_CPU0_HANDLE_VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_LSB_POSN = (int)8,
   VM_BUFFER_ACCESS_CPU0_HANDLE_VM_BUFFER_ACCESS_HANDLE_SUBSYSTEM_MSB_POSN = (int)11
};
typedef enum vm_buffer_access_cpu0_handle_posn_enum vm_buffer_access_cpu0_handle_posn;


enum vm_buffer_access_cpu0_status_posn_enum
{
   VM_BUFFER_ACCESS_CPU0_STATUS_VM_BUFFER_ACCESS_STATUS_ACTIVE_LSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU0_STATUS_VM_BUFFER_ACCESS_STATUS_ACTIVE_MSB_POSN = (int)0,
   VM_BUFFER_ACCESS_CPU0_STATUS_VM_BUFFER_ACCESS_STATUS_ACCESS_LSB_POSN = (int)1,
   VM_BUFFER_ACCESS_CPU0_STATUS_VM_BUFFER_ACCESS_STATUS_ACCESS_MSB_POSN = (int)4
};
typedef enum vm_buffer_access_cpu0_status_posn_enum vm_buffer_access_cpu0_status_posn;


enum vm_cpu0_clear_port_posn_enum
{
   VM_CPU0_CLEAR_PORT_VM_CPU0_REQUESTS_WRITE_LSB_POSN = (int)0,
   VM_CPU0_CLEAR_PORT_VM_CPU0_REQUESTS_WRITE_MSB_POSN = (int)0,
   VM_CPU0_CLEAR_PORT_VM_CPU0_REQUESTS_READ_LSB_POSN  = (int)1,
   VM_CPU0_CLEAR_PORT_VM_CPU0_REQUESTS_READ_MSB_POSN  = (int)1
};
typedef enum vm_cpu0_clear_port_posn_enum vm_cpu0_clear_port_posn;


enum vm_lookup_error_flags_posn_enum
{
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_LSB_POSN = (int)0,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_MSB_POSN = (int)0,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_LSB_POSN = (int)1,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_MSB_POSN = (int)1,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_LSB_POSN = (int)2,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_MSB_POSN = (int)2,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_LSB_POSN = (int)3,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_MSB_POSN = (int)3,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_LSB_POSN = (int)4,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_MSB_POSN = (int)11,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_LSB_POSN = (int)12,
   VM_LOOKUP_ERROR_FLAGS_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_MSB_POSN = (int)15
};
typedef enum vm_lookup_error_flags_posn_enum vm_lookup_error_flags_posn;


enum vm_lookup_error_flags_clear_posn_enum
{
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_LSB_POSN = (int)0,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_ACCESS_DENIED_MSB_POSN = (int)0,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_LSB_POSN = (int)1,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_HOST_ACCESS_DENIED_MSB_POSN = (int)1,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_LSB_POSN = (int)2,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_INVALID_ACCESS_MSB_POSN = (int)2,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_LSB_POSN = (int)3,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_UNSUPPORTED_MSB_POSN = (int)3,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_LSB_POSN = (int)4,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_HANDLE_MSB_POSN = (int)11,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_LSB_POSN = (int)12,
   VM_LOOKUP_ERROR_FLAGS_CLEAR_MMU_VM_LOOKUP_ERROR_FLAGS_SUBSYSTEM_MSB_POSN = (int)15
};
typedef enum vm_lookup_error_flags_clear_posn_enum vm_lookup_error_flags_clear_posn;


enum vm_request_cpu0_read_remote_config_posn_enum
{
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_EN_LSB_POSN = (int)12,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_EN_MSB_POSN = (int)12,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_LSB_POSN = (int)13,
   VM_REQUEST_CPU0_READ_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MSB_POSN = (int)13
};
typedef enum vm_request_cpu0_read_remote_config_posn_enum vm_request_cpu0_read_remote_config_posn;


enum vm_request_cpu0_read_status_posn_enum
{
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   VM_REQUEST_CPU0_READ_STATUS_VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum vm_request_cpu0_read_status_posn_enum vm_request_cpu0_read_status_posn;


enum vm_request_cpu0_write_remote_config_posn_enum
{
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_EN_LSB_POSN = (int)12,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_EN_MSB_POSN = (int)12,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_LSB_POSN = (int)13,
   VM_REQUEST_CPU0_WRITE_REMOTE_CONFIG_VM_REQUESTS_REMOTE_CONFIG_DENIED_STATUS_CLEAR_MSB_POSN = (int)13
};
typedef enum vm_request_cpu0_write_remote_config_posn_enum vm_request_cpu0_write_remote_config_posn;


enum vm_request_cpu0_write_status_posn_enum
{
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_HANDLE_LSB_POSN = (int)0,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_HANDLE_MSB_POSN = (int)7,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_SUBSYS_LSB_POSN = (int)8,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_SUBSYS_MSB_POSN = (int)11,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_ACTIVE_LSB_POSN = (int)12,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_ACTIVE_MSB_POSN = (int)12,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_ACCESS_DENIED_LSB_POSN = (int)13,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_ACCESS_DENIED_MSB_POSN = (int)13,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_STATUS_LSB_POSN = (int)14,
   VM_REQUEST_CPU0_WRITE_STATUS_VM_REQUEST_STATUS_BUFFER_STATUS_MSB_POSN = (int)17
};
typedef enum vm_request_cpu0_write_status_posn_enum vm_request_cpu0_write_status_posn;


enum vm_cpu0_access_ctrl_posn_enum
{
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   VM_CPU0_ACCESS_CTRL_APPS_SYS_CPU0_VM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum vm_cpu0_access_ctrl_posn_enum vm_cpu0_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_APPS_SYS_CPU0_VM */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CPU0_VM */

#if defined(IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES
#define __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES

/* -- kalimba_hw_semaphores -- Hardware semaphores interface registers -- */

enum semaphore_addr_posn_enum
{
   SEMAPHORE_ADDR_LSB_POSN                            = (int)0,
   SEMAPHORE_ADDR_SEMAPHORE_ADDR_LSB_POSN             = (int)0,
   SEMAPHORE_ADDR_MSB_POSN                            = (int)5,
   SEMAPHORE_ADDR_SEMAPHORE_ADDR_MSB_POSN             = (int)5
};
typedef enum semaphore_addr_posn_enum semaphore_addr_posn;

#define SEMAPHORE_ADDR_LSB_MASK                  (0x00000001u)
#define SEMAPHORE_ADDR_MSB_MASK                  (0x00000020u)

enum semaphore_data_posn_enum
{
   SEMAPHORE_DATA_POSN                                = (int)0,
   SEMAPHORE_DATA_SEMAPHORE_DATA_LSB_POSN             = (int)0,
   SEMAPHORE_DATA_SEMAPHORE_DATA_MSB_POSN             = (int)0
};
typedef enum semaphore_data_posn_enum semaphore_data_posn;

#define SEMAPHORE_DATA_MASK                      (0x00000001u)

enum semaphore_status_posn_enum
{
   SEMAPHORE_STATUS_POSN                              = (int)0,
   SEMAPHORE_STATUS_SEMAPHORE_STATUS_LSB_POSN         = (int)0,
   SEMAPHORE_STATUS_SEMAPHORE_STATUS_MSB_POSN         = (int)0
};
typedef enum semaphore_status_posn_enum semaphore_status_posn;

#define SEMAPHORE_STATUS_MASK                    (0x00000001u)

enum semaphore_status_raw_posn_enum
{
   SEMAPHORE_STATUS_RAW_POSN                          = (int)0,
   SEMAPHORE_STATUS_RAW_SEMAPHORE_STATUS_RAW_LSB_POSN = (int)0,
   SEMAPHORE_STATUS_RAW_SEMAPHORE_STATUS_RAW_MSB_POSN = (int)0
};
typedef enum semaphore_status_raw_posn_enum semaphore_status_raw_posn;

#define SEMAPHORE_STATUS_RAW_MASK                (0x00000001u)

#endif /* IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_HW_SEMAPHORES */


#if defined(IO_DEFS_MODULE_APPS_SYS_CLKGEN) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CLKGEN
#define __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CLKGEN

/* -- apps_sys_clkgen -- Applictions subsystem clock generation registers -- */

enum clkgen_debug_activity_enum
{
   CLKGEN_DEBUG_ACTIVITY_FORCE_CLOCKS_ACTIVE = (int)0x0,
   CLKGEN_DEBUG_ACTIVITY_KAL_CLK_RATE_REQ0  = (int)0x2,
   CLKGEN_DEBUG_ACTIVITY_KAL_CLK_RATE_REQ1  = (int)0x3,
   CLKGEN_DEBUG_ACTIVITY_TMUX_ACTIVITY      = (int)0x4,
   CLKGEN_DEBUG_ACTIVITY_TBUS_DBG_ACTIVITY  = (int)0x5,
   CLKGEN_DEBUG_ACTIVITY_TBUS_ACTIVITY      = (int)0x6,
   CLKGEN_DEBUG_ACTIVITY_TBUS_MSG_ACTIVITY  = (int)0x7,
   CLKGEN_DEBUG_ACTIVITY_TBUS_MISC_ACTIVITY = (int)0x8,
   CLKGEN_DEBUG_ACTIVITY_TBUS_VML_ACTIVITY  = (int)0x9,
   CLKGEN_DEBUG_ACTIVITY_TBUS_MEM_ACTIVITY  = (int)0xA,
   CLKGEN_DEBUG_ACTIVITY_VM_ACTIVITY        = (int)0xB,
   CLKGEN_DEBUG_ACTIVITY_DMAC_ACTIVITY      = (int)0xC,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_REGS       = (int)0xD,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_SQIF_REGS0 = (int)0xE,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_SQIF_REGS1 = (int)0xF,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_DSP_REGS0  = (int)0x10,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_DSP_REGS1  = (int)0x11,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_TBUS_REGS  = (int)0x12,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_TBUS_MSG_REGS = (int)0x13,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_LED_CTRL_REGS = (int)0x15,
   CLKGEN_DEBUG_ACTIVITY_DSP_EXTERNAL_ACCESS_ACTIVITY0 = (int)0x16,
   CLKGEN_DEBUG_ACTIVITY_DSP_EXTERNAL_ACCESS_ACTIVITY1 = (int)0x17,
   CLKGEN_DEBUG_ACTIVITY_TBUS_REM_ACTIVITY0 = (int)0x18,
   CLKGEN_DEBUG_ACTIVITY_TBUS_REM_ACTIVITY1 = (int)0x19,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_DSP_CACHE0 = (int)0x1A,
   CLKGEN_DEBUG_ACTIVITY_REQ_CLK_DSP_CACHE1 = (int)0x1B,
   CLKGEN_DEBUG_ACTIVITY_SDIO_HOST_FORCE_CLOCKS_ACTIVE = (int)0x1,
   MAX_CLKGEN_DEBUG_ACTIVITY                = (int)0x1B
};
typedef enum clkgen_debug_activity_enum clkgen_debug_activity;


enum clkgen_debug_clocks_enum
{
   CLKGEN_DEBUG_CLOCKS_CLK_CORE_CONT        = (int)0x0,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP0             = (int)0x1,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP1             = (int)0x2,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP_REGS0        = (int)0x3,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP_REGS1        = (int)0x4,
   CLKGEN_DEBUG_CLOCKS_CLK_TRAN_BUS         = (int)0x5,
   CLKGEN_DEBUG_CLOCKS_CLK_MEM_ADPTR        = (int)0x6,
   CLKGEN_DEBUG_CLOCKS_CLK_DBG_ADPTR        = (int)0x7,
   CLKGEN_DEBUG_CLOCKS_CLK_INT_ADPTR        = (int)0x8,
   CLKGEN_DEBUG_CLOCKS_CLK_INT_ADPTR_REGS   = (int)0x9,
   CLKGEN_DEBUG_CLOCKS_CLK_MSG_ADPTR        = (int)0xA,
   CLKGEN_DEBUG_CLOCKS_CLK_MSG_ADPTR_REGS   = (int)0xB,
   CLKGEN_DEBUG_CLOCKS_CLK_MISC_ADPTR       = (int)0xC,
   CLKGEN_DEBUG_CLOCKS_CLK_VM               = (int)0xD,
   CLKGEN_DEBUG_CLOCKS_CLK_REGS             = (int)0xE,
   CLKGEN_DEBUG_CLOCKS_CLK_REGS_MASTER      = (int)0xF,
   CLKGEN_DEBUG_CLOCKS_CLK_DMAC             = (int)0x10,
   CLKGEN_DEBUG_CLOCKS_CLK_DECRYPT0         = (int)0x11,
   CLKGEN_DEBUG_CLOCKS_CLK_DECRYPT1         = (int)0x12,
   CLKGEN_DEBUG_CLOCKS_CLK_LED_CTRL_REGS    = (int)0x13,
   CLKGEN_DEBUG_CLOCKS_CLK_SQIF0            = (int)0x14,
   CLKGEN_DEBUG_CLOCKS_CLK_SQIF1            = (int)0x15,
   CLKGEN_DEBUG_CLOCKS_CLK_SQIF_REGS0       = (int)0x16,
   CLKGEN_DEBUG_CLOCKS_CLK_SQIF_REGS1       = (int)0x17,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP_CACHE0       = (int)0x18,
   CLKGEN_DEBUG_CLOCKS_CLK_DSP_CACHE1       = (int)0x19,
   CLKGEN_DEBUG_CLOCKS_CLK_SDIO_HOST        = (int)0x1D,
   CLKGEN_DEBUG_CLOCKS_CLK_SDIO_HOST_BUS    = (int)0x1E,
   MAX_CLKGEN_DEBUG_CLOCKS                  = (int)0x1E
};
typedef enum clkgen_debug_clocks_enum clkgen_debug_clocks;


enum clkgen_debug_gate_enables_enum
{
   CLKGEN_DEBUG_GATE_ENABLES_DSP0           = (int)0x0,
   CLKGEN_DEBUG_GATE_ENABLES_DSP_REGS0      = (int)0x1,
   CLKGEN_DEBUG_GATE_ENABLES_DSP1           = (int)0x2,
   CLKGEN_DEBUG_GATE_ENABLES_DSP_REGS1      = (int)0x3,
   CLKGEN_DEBUG_GATE_ENABLES_VM             = (int)0x4,
   CLKGEN_DEBUG_GATE_ENABLES_DMAC           = (int)0x5,
   CLKGEN_DEBUG_GATE_ENABLES_DBG_ADPTR      = (int)0x6,
   CLKGEN_DEBUG_GATE_ENABLES_MEM_ADPTR      = (int)0x7,
   CLKGEN_DEBUG_GATE_ENABLES_INT_ADPTR      = (int)0x8,
   CLKGEN_DEBUG_GATE_ENABLES_INT_ADPTR_REGS = (int)0x9,
   CLKGEN_DEBUG_GATE_ENABLES_MSG_ADPTR      = (int)0xA,
   CLKGEN_DEBUG_GATE_ENABLES_MSG_ADPTR_REGS = (int)0xB,
   CLKGEN_DEBUG_GATE_ENABLES_MISC_ADPTR     = (int)0xC,
   CLKGEN_DEBUG_GATE_ENABLES_REGS           = (int)0xD,
   CLKGEN_DEBUG_GATE_ENABLES_REGS_MASTER    = (int)0xE,
   CLKGEN_DEBUG_GATE_ENABLES_DECRYPT0       = (int)0x12,
   CLKGEN_DEBUG_GATE_ENABLES_DECRYPT1       = (int)0x13,
   CLKGEN_DEBUG_GATE_ENABLES_AUXDATACONV_REGS = (int)0x14,
   CLKGEN_DEBUG_GATE_ENABLES_LEDCTRL_REGS   = (int)0x15,
   CLKGEN_DEBUG_GATE_ENABLES_TBUS_REM_ACTIVITY = (int)0x16,
   CLKGEN_DEBUG_GATE_ENABLES_PIO_INT        = (int)0x17,
   CLKGEN_DEBUG_GATE_ENABLES_DATAPATH0      = (int)0x18,
   CLKGEN_DEBUG_GATE_ENABLES_DATAPATH1      = (int)0x19,
   CLKGEN_DEBUG_GATE_ENABLES_DSP_CACHE0     = (int)0x1A,
   CLKGEN_DEBUG_GATE_ENABLES_DSP_CACHE1     = (int)0x1B,
   CLKGEN_DEBUG_GATE_ENABLES_MISC_PROC_TBUS = (int)0x1C,
   CLKGEN_DEBUG_GATE_ENABLES_RAM_AUTO_SLEEP = (int)0x1D,
   CLKGEN_DEBUG_GATE_ENABLES_INTERCONNECT   = (int)0x1E,
   CLKGEN_DEBUG_GATE_ENABLES_SHARED_RAM     = (int)0x1F,
   CLKGEN_DEBUG_GATE_ENABLES_SDIO_HOST      = (int)0x11,
   MAX_CLKGEN_DEBUG_GATE_ENABLES            = (int)0x1F
};
typedef enum clkgen_debug_gate_enables_enum clkgen_debug_gate_enables;


enum clkgen_debug_misc_enum
{
   CLKGEN_DEBUG_MISC_FORCE_CLOCKS_ACTIVE_IN = (int)0x0,
   CLKGEN_DEBUG_MISC_TIME_SYNC_PULSE_IN     = (int)0x2,
   CLKGEN_DEBUG_MISC_CORE_CLOCK_DIVIDER_VALUE = (int)0x3,
   CLKGEN_DEBUG_MISC_CLK_CORE_DIV_PULSE     = (int)0x8,
   CLKGEN_DEBUG_MISC_CLK_40M_DIV_PULSE      = (int)0x9,
   CLKGEN_DEBUG_MISC_CLK_48M_DIV_PULSE      = (int)0xA,
   MAX_CLKGEN_DEBUG_MISC                    = (int)0xA
};
typedef enum clkgen_debug_misc_enum clkgen_debug_misc;


enum clkgen_debug_ram_gate_enables_enum
{
   CLKGEN_DEBUG_GATE_ENABLES_CPU0_CACHE_RAM0 = (int)0x0,
   CLKGEN_DEBUG_GATE_ENABLES_CPU0_CACHE_RAM1 = (int)0x1,
   CLKGEN_DEBUG_GATE_ENABLES_CPU0_CACHE_TAG = (int)0x2,
   CLKGEN_DEBUG_GATE_ENABLES_CPU1_CACHE_RAM0 = (int)0x3,
   CLKGEN_DEBUG_GATE_ENABLES_CPU1_CACHE_RAM1 = (int)0x4,
   CLKGEN_DEBUG_GATE_ENABLES_CPU1_CACHE_TAG = (int)0x5,
   CLKGEN_DEBUG_GATE_ENABLES_CPU0_DM_RAM    = (int)0x6,
   CLKGEN_DEBUG_GATE_ENABLES_CPU1_DM_RAM    = (int)0x7,
   CLKGEN_DEBUG_GATE_ENABLES_TCM1_RAM       = (int)0x8,
   MAX_CLKGEN_DEBUG_RAM_GATE_ENABLES        = (int)0x8
};
#define NUM_CLKGEN_DEBUG_RAM_GATE_ENABLES (0x9)
typedef enum clkgen_debug_ram_gate_enables_enum clkgen_debug_ram_gate_enables;


enum clkgen_debug_select_enum
{
   CLKGEN_DEBUG_SELECT_GATE_ENABLES         = (int)0x1,
   CLKGEN_DEBUG_SELECT_CLOCKS_BY_2          = (int)0x2,
   CLKGEN_DEBUG_SELECT_ACTIVITY             = (int)0x3,
   CLKGEN_DEBUG_SELECT_MISC                 = (int)0x5,
   CLKGEN_DEBUG_SELECT_RAM_GATE_ENABLES     = (int)0x6,
   MAX_CLKGEN_DEBUG_SELECT                  = (int)0x6
};
typedef enum clkgen_debug_select_enum clkgen_debug_select;


enum clkgen_debug_posn_enum
{
   CLKGEN_DEBUG_CLOCKS_ENABLE_POSN                    = (int)0,
   CLKGEN_DEBUG_CLKGEN_DEBUG_CLOCKS_ENABLE_LSB_POSN   = (int)0,
   CLKGEN_DEBUG_CLKGEN_DEBUG_CLOCKS_ENABLE_MSB_POSN   = (int)0,
   CLKGEN_DEBUG_SELECT_LSB_POSN                       = (int)1,
   CLKGEN_DEBUG_CLKGEN_DEBUG_SELECT_LSB_POSN          = (int)1,
   CLKGEN_DEBUG_SELECT_MSB_POSN                       = (int)3,
   CLKGEN_DEBUG_CLKGEN_DEBUG_SELECT_MSB_POSN          = (int)3
};
typedef enum clkgen_debug_posn_enum clkgen_debug_posn;

#define CLKGEN_DEBUG_CLOCKS_ENABLE_MASK          (0x00000001u)
#define CLKGEN_DEBUG_SELECT_LSB_MASK             (0x00000002u)
#define CLKGEN_DEBUG_SELECT_MSB_MASK             (0x00000008u)

enum clkgen_enables_posn_enum
{
   CLKGEN_120M_REGS_EN_POSN                           = (int)0,
   CLKGEN_ENABLES_CLKGEN_120M_REGS_EN_LSB_POSN        = (int)0,
   CLKGEN_ENABLES_CLKGEN_120M_REGS_EN_MSB_POSN        = (int)0,
   CLKGEN_DSP_REGS_EN_POSN                            = (int)1,
   CLKGEN_ENABLES_CLKGEN_DSP_REGS_EN_LSB_POSN         = (int)1,
   CLKGEN_ENABLES_CLKGEN_DSP_REGS_EN_MSB_POSN         = (int)1,
   CLKGEN_ADPTRS_EN_POSN                              = (int)2,
   CLKGEN_ENABLES_CLKGEN_ADPTRS_EN_LSB_POSN           = (int)2,
   CLKGEN_ENABLES_CLKGEN_ADPTRS_EN_MSB_POSN           = (int)2,
   CLKGEN_SQIF0_REGS_EN_POSN                          = (int)3,
   CLKGEN_ENABLES_CLKGEN_SQIF0_REGS_EN_LSB_POSN       = (int)3,
   CLKGEN_ENABLES_CLKGEN_SQIF0_REGS_EN_MSB_POSN       = (int)3,
   CLKGEN_SQIF1_REGS_EN_POSN                          = (int)4,
   CLKGEN_ENABLES_CLKGEN_SQIF1_REGS_EN_LSB_POSN       = (int)4,
   CLKGEN_ENABLES_CLKGEN_SQIF1_REGS_EN_MSB_POSN       = (int)4,
   CLKGEN_VM_EN_POSN                                  = (int)5,
   CLKGEN_ENABLES_CLKGEN_VM_EN_LSB_POSN               = (int)5,
   CLKGEN_ENABLES_CLKGEN_VM_EN_MSB_POSN               = (int)5,
   CLKGEN_DMAC_EN_POSN                                = (int)6,
   CLKGEN_ENABLES_CLKGEN_DMAC_EN_LSB_POSN             = (int)6,
   CLKGEN_ENABLES_CLKGEN_DMAC_EN_MSB_POSN             = (int)6,
   CLKGEN_DECRYPT0_EN_POSN                            = (int)7,
   CLKGEN_ENABLES_CLKGEN_DECRYPT0_EN_LSB_POSN         = (int)7,
   CLKGEN_ENABLES_CLKGEN_DECRYPT0_EN_MSB_POSN         = (int)7,
   CLKGEN_DECRYPT1_EN_POSN                            = (int)8,
   CLKGEN_ENABLES_CLKGEN_DECRYPT1_EN_LSB_POSN         = (int)8,
   CLKGEN_ENABLES_CLKGEN_DECRYPT1_EN_MSB_POSN         = (int)8,
   CLKGEN_LED_CTRL_REGS_EN_POSN                       = (int)9,
   CLKGEN_ENABLES_CLKGEN_LED_CTRL_REGS_EN_LSB_POSN    = (int)9,
   CLKGEN_ENABLES_CLKGEN_LED_CTRL_REGS_EN_MSB_POSN    = (int)9,
   CLKGEN_PIO_INT_EN_POSN                             = (int)10,
   CLKGEN_ENABLES_CLKGEN_PIO_INT_EN_LSB_POSN          = (int)10,
   CLKGEN_ENABLES_CLKGEN_PIO_INT_EN_MSB_POSN          = (int)10,
   CLKGEN_DATAPATH0_EN_POSN                           = (int)11,
   CLKGEN_ENABLES_CLKGEN_DATAPATH0_EN_LSB_POSN        = (int)11,
   CLKGEN_ENABLES_CLKGEN_DATAPATH0_EN_MSB_POSN        = (int)11,
   CLKGEN_DATAPATH1_EN_POSN                           = (int)12,
   CLKGEN_ENABLES_CLKGEN_DATAPATH1_EN_LSB_POSN        = (int)12,
   CLKGEN_ENABLES_CLKGEN_DATAPATH1_EN_MSB_POSN        = (int)12,
   CLKGEN_DSP0_CACHE_EN_POSN                          = (int)13,
   CLKGEN_ENABLES_CLKGEN_DSP0_CACHE_EN_LSB_POSN       = (int)13,
   CLKGEN_ENABLES_CLKGEN_DSP0_CACHE_EN_MSB_POSN       = (int)13,
   CLKGEN_DSP1_CACHE_EN_POSN                          = (int)14,
   CLKGEN_ENABLES_CLKGEN_DSP1_CACHE_EN_LSB_POSN       = (int)14,
   CLKGEN_ENABLES_CLKGEN_DSP1_CACHE_EN_MSB_POSN       = (int)14,
   CLKGEN_DSP_TRACE_EN_POSN                           = (int)15,
   CLKGEN_ENABLES_CLKGEN_DSP_TRACE_EN_LSB_POSN        = (int)15,
   CLKGEN_ENABLES_CLKGEN_DSP_TRACE_EN_MSB_POSN        = (int)15,
   CLKGEN_RAM_AUTO_SLEEP_EN_POSN                      = (int)21,
   CLKGEN_ENABLES_CLKGEN_RAM_AUTO_SLEEP_EN_LSB_POSN   = (int)21,
   CLKGEN_ENABLES_CLKGEN_RAM_AUTO_SLEEP_EN_MSB_POSN   = (int)21,
   CLKGEN_INTERCONNECT_EN_POSN                        = (int)22,
   CLKGEN_ENABLES_CLKGEN_INTERCONNECT_EN_LSB_POSN     = (int)22,
   CLKGEN_ENABLES_CLKGEN_INTERCONNECT_EN_MSB_POSN     = (int)22,
   CLKGEN_SHARED_DM_EN_POSN                           = (int)23,
   CLKGEN_ENABLES_CLKGEN_SHARED_DM_EN_LSB_POSN        = (int)23,
   CLKGEN_ENABLES_CLKGEN_SHARED_DM_EN_MSB_POSN        = (int)23,
   CLKGEN_RAM_GATES_EN_POSN                           = (int)24,
   CLKGEN_ENABLES_CLKGEN_RAM_GATES_EN_LSB_POSN        = (int)24,
   CLKGEN_ENABLES_CLKGEN_RAM_GATES_EN_MSB_POSN        = (int)24,
   CLKGEN_CPU0_CACHE_RAM_EN_POSN                      = (int)25,
   CLKGEN_ENABLES_CLKGEN_CPU0_CACHE_RAM_EN_LSB_POSN   = (int)25,
   CLKGEN_ENABLES_CLKGEN_CPU0_CACHE_RAM_EN_MSB_POSN   = (int)25,
   CLKGEN_CPU1_CACHE_RAM_EN_POSN                      = (int)26,
   CLKGEN_ENABLES_CLKGEN_CPU1_CACHE_RAM_EN_LSB_POSN   = (int)26,
   CLKGEN_ENABLES_CLKGEN_CPU1_CACHE_RAM_EN_MSB_POSN   = (int)26,
   CLKGEN_CPU0_DM_RAM_EN_POSN                         = (int)27,
   CLKGEN_ENABLES_CLKGEN_CPU0_DM_RAM_EN_LSB_POSN      = (int)27,
   CLKGEN_ENABLES_CLKGEN_CPU0_DM_RAM_EN_MSB_POSN      = (int)27,
   CLKGEN_CPU1_DM_RAM_EN_POSN                         = (int)28,
   CLKGEN_ENABLES_CLKGEN_CPU1_DM_RAM_EN_LSB_POSN      = (int)28,
   CLKGEN_ENABLES_CLKGEN_CPU1_DM_RAM_EN_MSB_POSN      = (int)28,
   CLKGEN_TCM1_RAM_EN_POSN                            = (int)29,
   CLKGEN_ENABLES_CLKGEN_TCM1_RAM_EN_LSB_POSN         = (int)29,
   CLKGEN_ENABLES_CLKGEN_TCM1_RAM_EN_MSB_POSN         = (int)29,
   CLKGEN_SDIO_HOST_EN_POSN                           = (int)18,
   CLKGEN_ENABLES_CLKGEN_SDIO_HOST_EN_LSB_POSN        = (int)18,
   CLKGEN_ENABLES_CLKGEN_SDIO_HOST_EN_MSB_POSN        = (int)18
};
typedef enum clkgen_enables_posn_enum clkgen_enables_posn;

#define CLKGEN_120M_REGS_EN_MASK                 (0x00000001u)
#define CLKGEN_DSP_REGS_EN_MASK                  (0x00000002u)
#define CLKGEN_ADPTRS_EN_MASK                    (0x00000004u)
#define CLKGEN_SQIF0_REGS_EN_MASK                (0x00000008u)
#define CLKGEN_SQIF1_REGS_EN_MASK                (0x00000010u)
#define CLKGEN_VM_EN_MASK                        (0x00000020u)
#define CLKGEN_DMAC_EN_MASK                      (0x00000040u)
#define CLKGEN_DECRYPT0_EN_MASK                  (0x00000080u)
#define CLKGEN_DECRYPT1_EN_MASK                  (0x00000100u)
#define CLKGEN_LED_CTRL_REGS_EN_MASK             (0x00000200u)
#define CLKGEN_PIO_INT_EN_MASK                   (0x00000400u)
#define CLKGEN_DATAPATH0_EN_MASK                 (0x00000800u)
#define CLKGEN_DATAPATH1_EN_MASK                 (0x00001000u)
#define CLKGEN_DSP0_CACHE_EN_MASK                (0x00002000u)
#define CLKGEN_DSP1_CACHE_EN_MASK                (0x00004000u)
#define CLKGEN_DSP_TRACE_EN_MASK                 (0x00008000u)
#define CLKGEN_RAM_AUTO_SLEEP_EN_MASK            (0x00200000u)
#define CLKGEN_INTERCONNECT_EN_MASK              (0x00400000u)
#define CLKGEN_SHARED_DM_EN_MASK                 (0x00800000u)
#define CLKGEN_RAM_GATES_EN_MASK                 (0x01000000u)
#define CLKGEN_CPU0_CACHE_RAM_EN_MASK            (0x02000000u)
#define CLKGEN_CPU1_CACHE_RAM_EN_MASK            (0x04000000u)
#define CLKGEN_CPU0_DM_RAM_EN_MASK               (0x08000000u)
#define CLKGEN_CPU1_DM_RAM_EN_MASK               (0x10000000u)
#define CLKGEN_TCM1_RAM_EN_MASK                  (0x20000000u)
#define CLKGEN_SDIO_HOST_EN_MASK                 (0x00040000u)

enum clkgen_sdio_host_clk_config_posn_enum
{
   CLKGEN_SDIO_HOST_CLK_DIV_RATIO_LSB_POSN            = (int)0,
   CLKGEN_SDIO_HOST_CLK_CONFIG_CLKGEN_SDIO_HOST_CLK_DIV_RATIO_LSB_POSN = (int)0,
   CLKGEN_SDIO_HOST_CLK_DIV_RATIO_MSB_POSN            = (int)3,
   CLKGEN_SDIO_HOST_CLK_CONFIG_CLKGEN_SDIO_HOST_CLK_DIV_RATIO_MSB_POSN = (int)3
};
typedef enum clkgen_sdio_host_clk_config_posn_enum clkgen_sdio_host_clk_config_posn;

#define CLKGEN_SDIO_HOST_CLK_DIV_RATIO_LSB_MASK  (0x00000001u)
#define CLKGEN_SDIO_HOST_CLK_DIV_RATIO_MSB_MASK  (0x00000008u)

enum clkgen_timer_enables_posn_enum
{
   CLKGEN_TIMER_EN_POSN                               = (int)0,
   CLKGEN_TIMER_ENABLES_CLKGEN_TIMER_EN_LSB_POSN      = (int)0,
   CLKGEN_TIMER_ENABLES_CLKGEN_TIMER_EN_MSB_POSN      = (int)0,
   CLKGEN_TIMER_FAST_EN_POSN                          = (int)1,
   CLKGEN_TIMER_ENABLES_CLKGEN_TIMER_FAST_EN_LSB_POSN = (int)1,
   CLKGEN_TIMER_ENABLES_CLKGEN_TIMER_FAST_EN_MSB_POSN = (int)1
};
typedef enum clkgen_timer_enables_posn_enum clkgen_timer_enables_posn;

#define CLKGEN_TIMER_EN_MASK                     (0x00000001u)
#define CLKGEN_TIMER_FAST_EN_MASK                (0x00000002u)

enum clkgen_core_clk_rate_posn_enum
{
   CLKGEN_CORE_CLK_RATE_LSB_POSN                      = (int)0,
   CLKGEN_CORE_CLK_RATE_CLKGEN_CORE_CLK_RATE_LSB_POSN = (int)0,
   CLKGEN_CORE_CLK_RATE_MSB_POSN                      = (int)3,
   CLKGEN_CORE_CLK_RATE_CLKGEN_CORE_CLK_RATE_MSB_POSN = (int)3
};
typedef enum clkgen_core_clk_rate_posn_enum clkgen_core_clk_rate_posn;

#define CLKGEN_CORE_CLK_RATE_LSB_MASK            (0x00000001u)
#define CLKGEN_CORE_CLK_RATE_MSB_MASK            (0x00000008u)

enum clkgen_status_slow_clk_posn_enum
{
   CLKGEN_STATUS_SLOW_CLK_POSN                        = (int)0,
   CLKGEN_STATUS_SLOW_CLK_CLKGEN_STATUS_SLOW_CLK_LSB_POSN = (int)0,
   CLKGEN_STATUS_SLOW_CLK_CLKGEN_STATUS_SLOW_CLK_MSB_POSN = (int)0
};
typedef enum clkgen_status_slow_clk_posn_enum clkgen_status_slow_clk_posn;

#define CLKGEN_STATUS_SLOW_CLK_MASK              (0x00000001u)

enum clkgen_timer_fast_status_posn_enum
{
   CLKGEN_TIMER_FAST_STATUS_LSB_POSN                  = (int)0,
   CLKGEN_TIMER_FAST_STATUS_CLKGEN_TIMER_FAST_STATUS_LSB_POSN = (int)0,
   CLKGEN_TIMER_FAST_STATUS_MSB_POSN                  = (int)15,
   CLKGEN_TIMER_FAST_STATUS_CLKGEN_TIMER_FAST_STATUS_MSB_POSN = (int)15
};
typedef enum clkgen_timer_fast_status_posn_enum clkgen_timer_fast_status_posn;

#define CLKGEN_TIMER_FAST_STATUS_LSB_MASK        (0x00000001u)
#define CLKGEN_TIMER_FAST_STATUS_MSB_MASK        (0x00008000u)

enum apps_sys_clkgen__access_ctrl_enum_posn_enum
{
   APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_POSN         = (int)0,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_POSN         = (int)1,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_POSN         = (int)2,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_POSN         = (int)3,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   APPS_SYS_CLKGEN__ACCESS_CTRL_ENUM_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum apps_sys_clkgen__access_ctrl_enum_posn_enum apps_sys_clkgen__access_ctrl_enum_posn;

#define APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum apps_sys_clkgen__p0_access_permission_enum
{
   APPS_SYS_CLKGEN__P0_ACCESS_BLOCKED       = (int)0x0,
   APPS_SYS_CLKGEN__P0_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_clkgen__p0_access_permission_enum apps_sys_clkgen__p0_access_permission;


enum apps_sys_clkgen__p1_access_permission_enum
{
   APPS_SYS_CLKGEN__P1_ACCESS_BLOCKED       = (int)0x0,
   APPS_SYS_CLKGEN__P1_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_clkgen__p1_access_permission_enum apps_sys_clkgen__p1_access_permission;


enum apps_sys_clkgen__p2_access_permission_enum
{
   APPS_SYS_CLKGEN__P2_ACCESS_BLOCKED       = (int)0x0,
   APPS_SYS_CLKGEN__P2_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_clkgen__p2_access_permission_enum apps_sys_clkgen__p2_access_permission;


enum apps_sys_clkgen__p3_access_permission_enum
{
   APPS_SYS_CLKGEN__P3_ACCESS_BLOCKED       = (int)0x0,
   APPS_SYS_CLKGEN__P3_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION (0x2)
typedef enum apps_sys_clkgen__p3_access_permission_enum apps_sys_clkgen__p3_access_permission;


enum apps_sys_clkgen__mutex_lock_enum_enum
{
   APPS_SYS_CLKGEN__MUTEX_AVAILABLE         = (int)0x0,
   APPS_SYS_CLKGEN__MUTEX_CLAIMED_BY_P0     = (int)0x1,
   APPS_SYS_CLKGEN__MUTEX_CLAIMED_BY_P1     = (int)0x2,
   APPS_SYS_CLKGEN__MUTEX_CLAIMED_BY_P2     = (int)0x4,
   APPS_SYS_CLKGEN__MUTEX_CLAIMED_BY_P3     = (int)0x8,
   APPS_SYS_CLKGEN__MUTEX_DISABLED          = (int)0xF,
   MAX_APPS_SYS_CLKGEN__MUTEX_LOCK_ENUM     = (int)0xF
};
typedef enum apps_sys_clkgen__mutex_lock_enum_enum apps_sys_clkgen__mutex_lock_enum;


enum clkgen_access_ctrl_posn_enum
{
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   CLKGEN_ACCESS_CTRL_APPS_SYS_CLKGEN__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum clkgen_access_ctrl_posn_enum clkgen_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_APPS_SYS_CLKGEN */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_APPS_SYS_CLKGEN */



#if defined(IO_DEFS_MODULE_K32_INTERRUPT) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_K32_INTERRUPT
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_INTERRUPT

/* -- k32_interrupt -- Kalimba 32-bit Interrupt Controller Control registers -- */

enum int_source_enum_posn_enum
{
   INT_SOURCE_TBUS_INT_ADPTR_EVENT_POSN               = (int)5,
   INT_SOURCE_ENUM_INT_SOURCE_TBUS_INT_ADPTR_EVENT_LSB_POSN = (int)5,
   INT_SOURCE_ENUM_INT_SOURCE_TBUS_INT_ADPTR_EVENT_MSB_POSN = (int)5,
   INT_SOURCE_TBUS_MSG_ADPTR_EVENT_POSN               = (int)6,
   INT_SOURCE_ENUM_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_LSB_POSN = (int)6,
   INT_SOURCE_ENUM_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_MSB_POSN = (int)6,
   INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_POSN        = (int)7,
   INT_SOURCE_ENUM_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_LSB_POSN = (int)7,
   INT_SOURCE_ENUM_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_MSB_POSN = (int)7,
   INT_SOURCE_TIME_UPDATE_EVENT_POSN                  = (int)8,
   INT_SOURCE_ENUM_INT_SOURCE_TIME_UPDATE_EVENT_LSB_POSN = (int)8,
   INT_SOURCE_ENUM_INT_SOURCE_TIME_UPDATE_EVENT_MSB_POSN = (int)8,
   INT_SOURCE_VML_EVENT_POSN                          = (int)11,
   INT_SOURCE_ENUM_INT_SOURCE_VML_EVENT_LSB_POSN      = (int)11,
   INT_SOURCE_ENUM_INT_SOURCE_VML_EVENT_MSB_POSN      = (int)11,
   INT_SOURCE_DMAC_QUEUE0_EVENT_POSN                  = (int)12,
   INT_SOURCE_ENUM_INT_SOURCE_DMAC_QUEUE0_EVENT_LSB_POSN = (int)12,
   INT_SOURCE_ENUM_INT_SOURCE_DMAC_QUEUE0_EVENT_MSB_POSN = (int)12,
   INT_SOURCE_DMAC_QUEUE1_EVENT_POSN                  = (int)13,
   INT_SOURCE_ENUM_INT_SOURCE_DMAC_QUEUE1_EVENT_LSB_POSN = (int)13,
   INT_SOURCE_ENUM_INT_SOURCE_DMAC_QUEUE1_EVENT_MSB_POSN = (int)13,
   INT_SOURCE_SQIF_ARBITER_EVENT_POSN                 = (int)15,
   INT_SOURCE_ENUM_INT_SOURCE_SQIF_ARBITER_EVENT_LSB_POSN = (int)15,
   INT_SOURCE_ENUM_INT_SOURCE_SQIF_ARBITER_EVENT_MSB_POSN = (int)15,
   INT_SOURCE_SQIF_ARBITER1_EVENT_POSN                = (int)16,
   INT_SOURCE_ENUM_INT_SOURCE_SQIF_ARBITER1_EVENT_LSB_POSN = (int)16,
   INT_SOURCE_ENUM_INT_SOURCE_SQIF_ARBITER1_EVENT_MSB_POSN = (int)16,
   INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_POSN            = (int)18,
   INT_SOURCE_ENUM_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_LSB_POSN = (int)18,
   INT_SOURCE_ENUM_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_MSB_POSN = (int)18,
   INT_SOURCE_INTERPROC_EVENT_1_POSN                  = (int)21,
   INT_SOURCE_ENUM_INT_SOURCE_INTERPROC_EVENT_1_LSB_POSN = (int)21,
   INT_SOURCE_ENUM_INT_SOURCE_INTERPROC_EVENT_1_MSB_POSN = (int)21,
   INT_SOURCE_INTERPROC_EVENT_2_POSN                  = (int)22,
   INT_SOURCE_ENUM_INT_SOURCE_INTERPROC_EVENT_2_LSB_POSN = (int)22,
   INT_SOURCE_ENUM_INT_SOURCE_INTERPROC_EVENT_2_MSB_POSN = (int)22,
   INT_SOURCE_PIO_INT_EVENT_1_POSN                    = (int)23,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_INT_EVENT_1_LSB_POSN = (int)23,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_INT_EVENT_1_MSB_POSN = (int)23,
   INT_SOURCE_PIO_INT_EVENT_2_POSN                    = (int)24,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_INT_EVENT_2_LSB_POSN = (int)24,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_INT_EVENT_2_MSB_POSN = (int)24,
   INT_SOURCE_PIO_TIMER_EVENT_3_POSN                  = (int)25,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_TIMER_EVENT_3_LSB_POSN = (int)25,
   INT_SOURCE_ENUM_INT_SOURCE_PIO_TIMER_EVENT_3_MSB_POSN = (int)25,
   INT_SOURCE_CPU1_EXCEPTION_POSN                     = (int)26,
   INT_SOURCE_ENUM_INT_SOURCE_CPU1_EXCEPTION_LSB_POSN = (int)26,
   INT_SOURCE_ENUM_INT_SOURCE_CPU1_EXCEPTION_MSB_POSN = (int)26,
   INT_SOURCE_LAST_POSN                               = (int)26,
   INT_SOURCE_ENUM_INT_SOURCE_LAST_LSB_POSN           = (int)26,
   INT_SOURCE_ENUM_INT_SOURCE_LAST_MSB_POSN           = (int)26,
   INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_POSN          = (int)19,
   INT_SOURCE_ENUM_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_LSB_POSN = (int)19,
   INT_SOURCE_ENUM_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_MSB_POSN = (int)19,
   INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_POSN             = (int)20,
   INT_SOURCE_ENUM_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_LSB_POSN = (int)20,
   INT_SOURCE_ENUM_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_MSB_POSN = (int)20,
   INT_SOURCE_TIMER1_POSN                             = (int)0,
   INT_SOURCE_ENUM_INT_SOURCE_TIMER1_LSB_POSN         = (int)0,
   INT_SOURCE_ENUM_INT_SOURCE_TIMER1_MSB_POSN         = (int)0,
   INT_SOURCE_TIMER2_POSN                             = (int)1,
   INT_SOURCE_ENUM_INT_SOURCE_TIMER2_LSB_POSN         = (int)1,
   INT_SOURCE_ENUM_INT_SOURCE_TIMER2_MSB_POSN         = (int)1,
   INT_SOURCE_SW_ERROR_POSN                           = (int)2,
   INT_SOURCE_ENUM_INT_SOURCE_SW_ERROR_LSB_POSN       = (int)2,
   INT_SOURCE_ENUM_INT_SOURCE_SW_ERROR_MSB_POSN       = (int)2,
   INT_SOURCE_SW0_POSN                                = (int)3,
   INT_SOURCE_ENUM_INT_SOURCE_SW0_LSB_POSN            = (int)3,
   INT_SOURCE_ENUM_INT_SOURCE_SW0_MSB_POSN            = (int)3,
   INT_SOURCE_SW1_POSN                                = (int)4,
   INT_SOURCE_ENUM_INT_SOURCE_SW1_LSB_POSN            = (int)4,
   INT_SOURCE_ENUM_INT_SOURCE_SW1_MSB_POSN            = (int)4
};
typedef enum int_source_enum_posn_enum int_source_enum_posn;

#define INT_SOURCE_TBUS_INT_ADPTR_EVENT_MASK     (0x00000020u)
#define INT_SOURCE_TBUS_MSG_ADPTR_EVENT_MASK     (0x00000040u)
#define INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_MASK (0x00000080u)
#define INT_SOURCE_TIME_UPDATE_EVENT_MASK        (0x00000100u)
#define INT_SOURCE_VML_EVENT_MASK                (0x00000800u)
#define INT_SOURCE_DMAC_QUEUE0_EVENT_MASK        (0x00001000u)
#define INT_SOURCE_DMAC_QUEUE1_EVENT_MASK        (0x00002000u)
#define INT_SOURCE_SQIF_ARBITER_EVENT_MASK       (0x00008000u)
#define INT_SOURCE_SQIF_ARBITER1_EVENT_MASK      (0x00010000u)
#define INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_MASK  (0x00040000u)
#define INT_SOURCE_INTERPROC_EVENT_1_MASK        (0x00200000u)
#define INT_SOURCE_INTERPROC_EVENT_2_MASK        (0x00400000u)
#define INT_SOURCE_PIO_INT_EVENT_1_MASK          (0x00800000u)
#define INT_SOURCE_PIO_INT_EVENT_2_MASK          (0x01000000u)
#define INT_SOURCE_PIO_TIMER_EVENT_3_MASK        (0x02000000u)
#define INT_SOURCE_CPU1_EXCEPTION_MASK           (0x04000000u)
#define INT_SOURCE_LAST_MASK                     (0x04000000u)
#define INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_MASK (0x00080000u)
#define INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_MASK   (0x00100000u)
#define INT_SOURCE_TIMER1_MASK                   (0x00000001u)
#define INT_SOURCE_TIMER2_MASK                   (0x00000002u)
#define INT_SOURCE_SW_ERROR_MASK                 (0x00000004u)
#define INT_SOURCE_SW0_MASK                      (0x00000008u)
#define INT_SOURCE_SW1_MASK                      (0x00000010u)

enum int_load_info_posn_enum
{
   INT_LOAD_INFO_PRIORITY_LSB_POSN                    = (int)0,
   INT_LOAD_INFO_INT_LOAD_INFO_PRIORITY_LSB_POSN      = (int)0,
   INT_LOAD_INFO_PRIORITY_MSB_POSN                    = (int)1,
   INT_LOAD_INFO_INT_LOAD_INFO_PRIORITY_MSB_POSN      = (int)1,
   INT_LOAD_INFO_SOURCE_LSB_POSN                      = (int)2,
   INT_LOAD_INFO_INT_LOAD_INFO_SOURCE_LSB_POSN        = (int)2,
   INT_LOAD_INFO_SOURCE_MSB_POSN                      = (int)9,
   INT_LOAD_INFO_INT_LOAD_INFO_SOURCE_MSB_POSN        = (int)9,
   INT_LOAD_INFO_INT_ACTIVE_POSN                      = (int)10,
   INT_LOAD_INFO_INT_LOAD_INFO_INT_ACTIVE_LSB_POSN    = (int)10,
   INT_LOAD_INFO_INT_LOAD_INFO_INT_ACTIVE_MSB_POSN    = (int)10,
   INT_LOAD_INFO_REQUEST_LSB_POSN                     = (int)11,
   INT_LOAD_INFO_INT_LOAD_INFO_REQUEST_LSB_POSN       = (int)11,
   INT_LOAD_INFO_REQUEST_MSB_POSN                     = (int)18,
   INT_LOAD_INFO_INT_LOAD_INFO_REQUEST_MSB_POSN       = (int)18,
   INT_LOAD_INFO_DONT_CLEAR_POSN                      = (int)19,
   INT_LOAD_INFO_INT_LOAD_INFO_DONT_CLEAR_LSB_POSN    = (int)19,
   INT_LOAD_INFO_INT_LOAD_INFO_DONT_CLEAR_MSB_POSN    = (int)19
};
typedef enum int_load_info_posn_enum int_load_info_posn;

#define INT_LOAD_INFO_PRIORITY_LSB_MASK          (0x00000001u)
#define INT_LOAD_INFO_PRIORITY_MSB_MASK          (0x00000002u)
#define INT_LOAD_INFO_SOURCE_LSB_MASK            (0x00000004u)
#define INT_LOAD_INFO_SOURCE_MSB_MASK            (0x00000200u)
#define INT_LOAD_INFO_INT_ACTIVE_MASK            (0x00000400u)
#define INT_LOAD_INFO_REQUEST_LSB_MASK           (0x00000800u)
#define INT_LOAD_INFO_REQUEST_MSB_MASK           (0x00040000u)
#define INT_LOAD_INFO_DONT_CLEAR_MASK            (0x00080000u)

enum int_save_info_posn_enum
{
   INT_SAVE_INFO_PRIORITY_LSB_POSN                    = (int)0,
   INT_SAVE_INFO_INT_SAVE_INFO_PRIORITY_LSB_POSN      = (int)0,
   INT_SAVE_INFO_PRIORITY_MSB_POSN                    = (int)1,
   INT_SAVE_INFO_INT_SAVE_INFO_PRIORITY_MSB_POSN      = (int)1,
   INT_SAVE_INFO_SOURCE_LSB_POSN                      = (int)2,
   INT_SAVE_INFO_INT_SAVE_INFO_SOURCE_LSB_POSN        = (int)2,
   INT_SAVE_INFO_SOURCE_MSB_POSN                      = (int)9,
   INT_SAVE_INFO_INT_SAVE_INFO_SOURCE_MSB_POSN        = (int)9,
   INT_SAVE_INFO_ACTIVE_POSN                          = (int)10,
   INT_SAVE_INFO_INT_SAVE_INFO_ACTIVE_LSB_POSN        = (int)10,
   INT_SAVE_INFO_INT_SAVE_INFO_ACTIVE_MSB_POSN        = (int)10,
   INT_SAVE_INFO_SOURCE_NEW_LSB_POSN                  = (int)11,
   INT_SAVE_INFO_INT_SAVE_INFO_SOURCE_NEW_LSB_POSN    = (int)11,
   INT_SAVE_INFO_SOURCE_NEW_MSB_POSN                  = (int)18,
   INT_SAVE_INFO_INT_SAVE_INFO_SOURCE_NEW_MSB_POSN    = (int)18,
   INT_SAVE_INFO_UNUSED19_POSN                        = (int)19,
   INT_SAVE_INFO_INT_SAVE_INFO_UNUSED19_LSB_POSN      = (int)19,
   INT_SAVE_INFO_INT_SAVE_INFO_UNUSED19_MSB_POSN      = (int)19,
   INT_SAVE_INFO_PRIORITY_NEW_LSB_POSN                = (int)20,
   INT_SAVE_INFO_INT_SAVE_INFO_PRIORITY_NEW_LSB_POSN  = (int)20,
   INT_SAVE_INFO_PRIORITY_NEW_MSB_POSN                = (int)21,
   INT_SAVE_INFO_INT_SAVE_INFO_PRIORITY_NEW_MSB_POSN  = (int)21
};
typedef enum int_save_info_posn_enum int_save_info_posn;

#define INT_SAVE_INFO_PRIORITY_LSB_MASK          (0x00000001u)
#define INT_SAVE_INFO_PRIORITY_MSB_MASK          (0x00000002u)
#define INT_SAVE_INFO_SOURCE_LSB_MASK            (0x00000004u)
#define INT_SAVE_INFO_SOURCE_MSB_MASK            (0x00000200u)
#define INT_SAVE_INFO_ACTIVE_MASK                (0x00000400u)
#define INT_SAVE_INFO_SOURCE_NEW_LSB_MASK        (0x00000800u)
#define INT_SAVE_INFO_SOURCE_NEW_MSB_MASK        (0x00040000u)
#define INT_SAVE_INFO_UNUSED19_MASK              (0x00080000u)
#define INT_SAVE_INFO_PRIORITY_NEW_LSB_MASK      (0x00100000u)
#define INT_SAVE_INFO_PRIORITY_NEW_MSB_MASK      (0x00200000u)

enum int_ack_posn_enum
{
   INT_ACK_POSN                                       = (int)0,
   INT_ACK_INT_ACK_LSB_POSN                           = (int)0,
   INT_ACK_INT_ACK_MSB_POSN                           = (int)0
};
typedef enum int_ack_posn_enum int_ack_posn;

#define INT_ACK_MASK                             (0x00000001u)

enum int_addr_posn_enum
{
   INT_ADDR_LSB_POSN                                  = (int)0,
   INT_ADDR_INT_ADDR_LSB_POSN                         = (int)0,
   INT_ADDR_MSB_POSN                                  = (int)31,
   INT_ADDR_INT_ADDR_MSB_POSN                         = (int)31
};
typedef enum int_addr_posn_enum int_addr_posn;

#define INT_ADDR_LSB_MASK                        (0x00000001u)
#define INT_ADDR_MSB_MASK                        (0x80000000u)

enum int_block_priority_posn_enum
{
   INT_BLOCK_PRIORITY_LSB_POSN                        = (int)0,
   INT_BLOCK_PRIORITY_INT_BLOCK_PRIORITY_LSB_POSN     = (int)0,
   INT_BLOCK_PRIORITY_MSB_POSN                        = (int)1,
   INT_BLOCK_PRIORITY_INT_BLOCK_PRIORITY_MSB_POSN     = (int)1
};
typedef enum int_block_priority_posn_enum int_block_priority_posn;

#define INT_BLOCK_PRIORITY_LSB_MASK              (0x00000001u)
#define INT_BLOCK_PRIORITY_MSB_MASK              (0x00000002u)

enum int_clk_switch_en_posn_enum
{
   INT_CLK_SWITCH_EN_POSN                             = (int)0,
   INT_CLK_SWITCH_EN_INT_CLK_SWITCH_EN_LSB_POSN       = (int)0,
   INT_CLK_SWITCH_EN_INT_CLK_SWITCH_EN_MSB_POSN       = (int)0
};
typedef enum int_clk_switch_en_posn_enum int_clk_switch_en_posn;

#define INT_CLK_SWITCH_EN_MASK                   (0x00000001u)

enum int_gbl_enable_posn_enum
{
   INT_GBL_ENABLE_POSN                                = (int)0,
   INT_GBL_ENABLE_INT_GBL_ENABLE_LSB_POSN             = (int)0,
   INT_GBL_ENABLE_INT_GBL_ENABLE_MSB_POSN             = (int)0
};
typedef enum int_gbl_enable_posn_enum int_gbl_enable_posn;

#define INT_GBL_ENABLE_MASK                      (0x00000001u)

enum int_priority_posn_enum
{
   INT_PRIORITY_LSB_POSN                              = (int)0,
   INT_PRIORITY_INT_PRIORITY_LSB_POSN                 = (int)0,
   INT_PRIORITY_MSB_POSN                              = (int)1,
   INT_PRIORITY_INT_PRIORITY_MSB_POSN                 = (int)1
};
typedef enum int_priority_posn_enum int_priority_posn;

#define INT_PRIORITY_LSB_MASK                    (0x00000001u)
#define INT_PRIORITY_MSB_MASK                    (0x00000002u)

enum int_select_posn_enum
{
   INT_SELECT_LSB_POSN                                = (int)0,
   INT_SELECT_INT_SELECT_LSB_POSN                     = (int)0,
   INT_SELECT_MSB_POSN                                = (int)4,
   INT_SELECT_INT_SELECT_MSB_POSN                     = (int)4
};
typedef enum int_select_posn_enum int_select_posn;

#define INT_SELECT_LSB_MASK                      (0x00000001u)
#define INT_SELECT_MSB_MASK                      (0x00000010u)

enum int_source_posn_enum
{
   INT_SOURCE_LSB_POSN                                = (int)0,
   INT_SOURCE_INT_SOURCE_LSB_POSN                     = (int)0,
   INT_SOURCE_MSB_POSN                                = (int)4,
   INT_SOURCE_INT_SOURCE_MSB_POSN                     = (int)4
};
typedef enum int_source_posn_enum int_source_posn;

#define INT_SOURCE_LSB_MASK                      (0x00000001u)
#define INT_SOURCE_MSB_MASK                      (0x00000010u)

enum int_sw0_event_posn_enum
{
   INT_SW0_EVENT_POSN                                 = (int)0,
   INT_SW0_EVENT_INT_SW0_EVENT_LSB_POSN               = (int)0,
   INT_SW0_EVENT_INT_SW0_EVENT_MSB_POSN               = (int)0
};
typedef enum int_sw0_event_posn_enum int_sw0_event_posn;

#define INT_SW0_EVENT_MASK                       (0x00000001u)

enum int_sw1_event_posn_enum
{
   INT_SW1_EVENT_POSN                                 = (int)0,
   INT_SW1_EVENT_INT_SW1_EVENT_LSB_POSN               = (int)0,
   INT_SW1_EVENT_INT_SW1_EVENT_MSB_POSN               = (int)0
};
typedef enum int_sw1_event_posn_enum int_sw1_event_posn;

#define INT_SW1_EVENT_MASK                       (0x00000001u)

enum int_unblock_posn_enum
{
   INT_UNBLOCK_POSN                                   = (int)0,
   INT_UNBLOCK_INT_UNBLOCK_LSB_POSN                   = (int)0,
   INT_UNBLOCK_INT_UNBLOCK_MSB_POSN                   = (int)0
};
typedef enum int_unblock_posn_enum int_unblock_posn;

#define INT_UNBLOCK_MASK                         (0x00000001u)

enum k32_interrupt__clock_divide_rate_enum_enum
{
   K32_INTERRUPT__CLOCK_STOPPED             = (int)0x0,
   K32_INTERRUPT__CLOCK_RATE_MAX            = (int)0x1,
   K32_INTERRUPT__CLOCK_RATE_HALF           = (int)0x2,
   K32_INTERRUPT__CLOCK_RATE_RESERVED       = (int)0x3,
   MAX_K32_INTERRUPT__CLOCK_DIVIDE_RATE_ENUM = (int)0x3
};
#define NUM_K32_INTERRUPT__CLOCK_DIVIDE_RATE_ENUM (0x4)
typedef enum k32_interrupt__clock_divide_rate_enum_enum k32_interrupt__clock_divide_rate_enum;


enum int_sources_en_posn_enum
{
   INT_SOURCES_EN_INT_SOURCE_TBUS_INT_ADPTR_EVENT_LSB_POSN = (int)5,
   INT_SOURCES_EN_INT_SOURCE_TBUS_INT_ADPTR_EVENT_MSB_POSN = (int)5,
   INT_SOURCES_EN_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_LSB_POSN = (int)6,
   INT_SOURCES_EN_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_MSB_POSN = (int)6,
   INT_SOURCES_EN_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_LSB_POSN = (int)7,
   INT_SOURCES_EN_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_MSB_POSN = (int)7,
   INT_SOURCES_EN_INT_SOURCE_TIME_UPDATE_EVENT_LSB_POSN = (int)8,
   INT_SOURCES_EN_INT_SOURCE_TIME_UPDATE_EVENT_MSB_POSN = (int)8,
   INT_SOURCES_EN_INT_SOURCE_VML_EVENT_LSB_POSN       = (int)11,
   INT_SOURCES_EN_INT_SOURCE_VML_EVENT_MSB_POSN       = (int)11,
   INT_SOURCES_EN_INT_SOURCE_DMAC_QUEUE0_EVENT_LSB_POSN = (int)12,
   INT_SOURCES_EN_INT_SOURCE_DMAC_QUEUE0_EVENT_MSB_POSN = (int)12,
   INT_SOURCES_EN_INT_SOURCE_DMAC_QUEUE1_EVENT_LSB_POSN = (int)13,
   INT_SOURCES_EN_INT_SOURCE_DMAC_QUEUE1_EVENT_MSB_POSN = (int)13,
   INT_SOURCES_EN_INT_SOURCE_SQIF_ARBITER_EVENT_LSB_POSN = (int)15,
   INT_SOURCES_EN_INT_SOURCE_SQIF_ARBITER_EVENT_MSB_POSN = (int)15,
   INT_SOURCES_EN_INT_SOURCE_SQIF_ARBITER1_EVENT_LSB_POSN = (int)16,
   INT_SOURCES_EN_INT_SOURCE_SQIF_ARBITER1_EVENT_MSB_POSN = (int)16,
   INT_SOURCES_EN_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_LSB_POSN = (int)18,
   INT_SOURCES_EN_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_MSB_POSN = (int)18,
   INT_SOURCES_EN_INT_SOURCE_INTERPROC_EVENT_1_LSB_POSN = (int)21,
   INT_SOURCES_EN_INT_SOURCE_INTERPROC_EVENT_1_MSB_POSN = (int)21,
   INT_SOURCES_EN_INT_SOURCE_INTERPROC_EVENT_2_LSB_POSN = (int)22,
   INT_SOURCES_EN_INT_SOURCE_INTERPROC_EVENT_2_MSB_POSN = (int)22,
   INT_SOURCES_EN_INT_SOURCE_PIO_INT_EVENT_1_LSB_POSN = (int)23,
   INT_SOURCES_EN_INT_SOURCE_PIO_INT_EVENT_1_MSB_POSN = (int)23,
   INT_SOURCES_EN_INT_SOURCE_PIO_INT_EVENT_2_LSB_POSN = (int)24,
   INT_SOURCES_EN_INT_SOURCE_PIO_INT_EVENT_2_MSB_POSN = (int)24,
   INT_SOURCES_EN_INT_SOURCE_PIO_TIMER_EVENT_3_LSB_POSN = (int)25,
   INT_SOURCES_EN_INT_SOURCE_PIO_TIMER_EVENT_3_MSB_POSN = (int)25,
   INT_SOURCES_EN_INT_SOURCE_CPU1_EXCEPTION_LSB_POSN  = (int)26,
   INT_SOURCES_EN_INT_SOURCE_CPU1_EXCEPTION_MSB_POSN  = (int)26,
   INT_SOURCES_EN_INT_SOURCE_LAST_LSB_POSN            = (int)26,
   INT_SOURCES_EN_INT_SOURCE_LAST_MSB_POSN            = (int)26,
   INT_SOURCES_EN_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_LSB_POSN = (int)19,
   INT_SOURCES_EN_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_MSB_POSN = (int)19,
   INT_SOURCES_EN_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_LSB_POSN = (int)20,
   INT_SOURCES_EN_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_MSB_POSN = (int)20,
   INT_SOURCES_EN_INT_SOURCE_TIMER1_LSB_POSN          = (int)0,
   INT_SOURCES_EN_INT_SOURCE_TIMER1_MSB_POSN          = (int)0,
   INT_SOURCES_EN_INT_SOURCE_TIMER2_LSB_POSN          = (int)1,
   INT_SOURCES_EN_INT_SOURCE_TIMER2_MSB_POSN          = (int)1,
   INT_SOURCES_EN_INT_SOURCE_SW_ERROR_LSB_POSN        = (int)2,
   INT_SOURCES_EN_INT_SOURCE_SW_ERROR_MSB_POSN        = (int)2,
   INT_SOURCES_EN_INT_SOURCE_SW0_LSB_POSN             = (int)3,
   INT_SOURCES_EN_INT_SOURCE_SW0_MSB_POSN             = (int)3,
   INT_SOURCES_EN_INT_SOURCE_SW1_LSB_POSN             = (int)4,
   INT_SOURCES_EN_INT_SOURCE_SW1_MSB_POSN             = (int)4
};
typedef enum int_sources_en_posn_enum int_sources_en_posn;


enum int_status_posn_enum
{
   INT_STATUS_INT_SOURCE_TBUS_INT_ADPTR_EVENT_LSB_POSN = (int)5,
   INT_STATUS_INT_SOURCE_TBUS_INT_ADPTR_EVENT_MSB_POSN = (int)5,
   INT_STATUS_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_LSB_POSN = (int)6,
   INT_STATUS_INT_SOURCE_TBUS_MSG_ADPTR_EVENT_MSB_POSN = (int)6,
   INT_STATUS_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_LSB_POSN = (int)7,
   INT_STATUS_INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT_MSB_POSN = (int)7,
   INT_STATUS_INT_SOURCE_TIME_UPDATE_EVENT_LSB_POSN   = (int)8,
   INT_STATUS_INT_SOURCE_TIME_UPDATE_EVENT_MSB_POSN   = (int)8,
   INT_STATUS_INT_SOURCE_VML_EVENT_LSB_POSN           = (int)11,
   INT_STATUS_INT_SOURCE_VML_EVENT_MSB_POSN           = (int)11,
   INT_STATUS_INT_SOURCE_DMAC_QUEUE0_EVENT_LSB_POSN   = (int)12,
   INT_STATUS_INT_SOURCE_DMAC_QUEUE0_EVENT_MSB_POSN   = (int)12,
   INT_STATUS_INT_SOURCE_DMAC_QUEUE1_EVENT_LSB_POSN   = (int)13,
   INT_STATUS_INT_SOURCE_DMAC_QUEUE1_EVENT_MSB_POSN   = (int)13,
   INT_STATUS_INT_SOURCE_SQIF_ARBITER_EVENT_LSB_POSN  = (int)15,
   INT_STATUS_INT_SOURCE_SQIF_ARBITER_EVENT_MSB_POSN  = (int)15,
   INT_STATUS_INT_SOURCE_SQIF_ARBITER1_EVENT_LSB_POSN = (int)16,
   INT_STATUS_INT_SOURCE_SQIF_ARBITER1_EVENT_MSB_POSN = (int)16,
   INT_STATUS_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_LSB_POSN = (int)18,
   INT_STATUS_INT_SOURCE_CPU1_ACCESS_FAULT_EVENT_MSB_POSN = (int)18,
   INT_STATUS_INT_SOURCE_INTERPROC_EVENT_1_LSB_POSN   = (int)21,
   INT_STATUS_INT_SOURCE_INTERPROC_EVENT_1_MSB_POSN   = (int)21,
   INT_STATUS_INT_SOURCE_INTERPROC_EVENT_2_LSB_POSN   = (int)22,
   INT_STATUS_INT_SOURCE_INTERPROC_EVENT_2_MSB_POSN   = (int)22,
   INT_STATUS_INT_SOURCE_PIO_INT_EVENT_1_LSB_POSN     = (int)23,
   INT_STATUS_INT_SOURCE_PIO_INT_EVENT_1_MSB_POSN     = (int)23,
   INT_STATUS_INT_SOURCE_PIO_INT_EVENT_2_LSB_POSN     = (int)24,
   INT_STATUS_INT_SOURCE_PIO_INT_EVENT_2_MSB_POSN     = (int)24,
   INT_STATUS_INT_SOURCE_PIO_TIMER_EVENT_3_LSB_POSN   = (int)25,
   INT_STATUS_INT_SOURCE_PIO_TIMER_EVENT_3_MSB_POSN   = (int)25,
   INT_STATUS_INT_SOURCE_CPU1_EXCEPTION_LSB_POSN      = (int)26,
   INT_STATUS_INT_SOURCE_CPU1_EXCEPTION_MSB_POSN      = (int)26,
   INT_STATUS_INT_SOURCE_LAST_LSB_POSN                = (int)26,
   INT_STATUS_INT_SOURCE_LAST_MSB_POSN                = (int)26,
   INT_STATUS_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_LSB_POSN = (int)19,
   INT_STATUS_INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT_MSB_POSN = (int)19,
   INT_STATUS_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_LSB_POSN = (int)20,
   INT_STATUS_INT_SOURCE_SDIO_HOST_WAKEUP_EVENT_MSB_POSN = (int)20,
   INT_STATUS_INT_SOURCE_TIMER1_LSB_POSN              = (int)0,
   INT_STATUS_INT_SOURCE_TIMER1_MSB_POSN              = (int)0,
   INT_STATUS_INT_SOURCE_TIMER2_LSB_POSN              = (int)1,
   INT_STATUS_INT_SOURCE_TIMER2_MSB_POSN              = (int)1,
   INT_STATUS_INT_SOURCE_SW_ERROR_LSB_POSN            = (int)2,
   INT_STATUS_INT_SOURCE_SW_ERROR_MSB_POSN            = (int)2,
   INT_STATUS_INT_SOURCE_SW0_LSB_POSN                 = (int)3,
   INT_STATUS_INT_SOURCE_SW0_MSB_POSN                 = (int)3,
   INT_STATUS_INT_SOURCE_SW1_LSB_POSN                 = (int)4,
   INT_STATUS_INT_SOURCE_SW1_MSB_POSN                 = (int)4
};
typedef enum int_status_posn_enum int_status_posn;


enum int_source_enum
{
   INT_SOURCE_TBUS_INT_ADPTR_EVENT          = (int)0x5,
   INT_SOURCE_TBUS_MSG_ADPTR_EVENT          = (int)0x6,
   INT_SOURCE_OUTBOUND_ACCESS_ERROR_EVENT   = (int)0x7,
   INT_SOURCE_TIME_UPDATE_EVENT             = (int)0x8,
   INT_SOURCE_VML_EVENT                     = (int)0xB,
   INT_SOURCE_DMAC_QUEUE0_EVENT             = (int)0xC,
   INT_SOURCE_DMAC_QUEUE1_EVENT             = (int)0xD,
   INT_SOURCE_SQIF_ARBITER_EVENT            = (int)0xF,
   INT_SOURCE_SQIF_ARBITER1_EVENT           = (int)0x10,
   INT_SOURCE_CPU1_ACCESS_FAULT_EVENT       = (int)0x12,
   INT_SOURCE_INTERPROC_EVENT_1             = (int)0x15,
   INT_SOURCE_INTERPROC_EVENT_2             = (int)0x16,
   INT_SOURCE_PIO_INT_EVENT_1               = (int)0x17,
   INT_SOURCE_PIO_INT_EVENT_2               = (int)0x18,
   INT_SOURCE_PIO_TIMER_EVENT_3             = (int)0x19,
   INT_SOURCE_CPU1_EXCEPTION                = (int)0x1A,
   INT_SOURCE_LAST                          = (int)0x1A,
   INT_SOURCE_SDIO_HOST_INTERRUPT_EVENT     = (int)0x13,
   INT_SOURCE_SDIO_HOST_WAKEUP_EVENT        = (int)0x14,
   INT_SOURCE_TIMER1                        = (int)0x0,
   INT_SOURCE_TIMER2                        = (int)0x1,
   INT_SOURCE_SW_ERROR                      = (int)0x2,
   INT_SOURCE_SW0                           = (int)0x3,
   INT_SOURCE_SW1                           = (int)0x4
};
typedef enum int_source_enum int_source;


#endif /* IO_DEFS_MODULE_K32_INTERRUPT */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_K32_INTERRUPT */

#if defined(IO_DEFS_MODULE_CHIP) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_CHIP
#define __IO_DEFS_H__IO_DEFS_MODULE_CHIP

/* -- chip -- Crescendo chip-level enumerations -- */

enum system_bus_posn_enum
{
   SYSTEM_BUS_HOST_SYS_POSN                           = (int)1,
   SYSTEM_BUS_SYSTEM_BUS_HOST_SYS_LSB_POSN            = (int)1,
   SYSTEM_BUS_SYSTEM_BUS_HOST_SYS_MSB_POSN            = (int)1,
   SYSTEM_BUS_BT_SYS_POSN                             = (int)2,
   SYSTEM_BUS_SYSTEM_BUS_BT_SYS_LSB_POSN              = (int)2,
   SYSTEM_BUS_SYSTEM_BUS_BT_SYS_MSB_POSN              = (int)2,
   SYSTEM_BUS_APPS_SYS_POSN                           = (int)4,
   SYSTEM_BUS_SYSTEM_BUS_APPS_SYS_LSB_POSN            = (int)4,
   SYSTEM_BUS_SYSTEM_BUS_APPS_SYS_MSB_POSN            = (int)4,
   SYSTEM_BUS_CURATOR_POSN                            = (int)0,
   SYSTEM_BUS_SYSTEM_BUS_CURATOR_LSB_POSN             = (int)0,
   SYSTEM_BUS_SYSTEM_BUS_CURATOR_MSB_POSN             = (int)0,
   SYSTEM_BUS_PIO_CTRL_SYS_POSN                       = (int)5,
   SYSTEM_BUS_SYSTEM_BUS_PIO_CTRL_SYS_LSB_POSN        = (int)5,
   SYSTEM_BUS_SYSTEM_BUS_PIO_CTRL_SYS_MSB_POSN        = (int)5,
   SYSTEM_BUS_AUDIO_SYS_POSN                          = (int)3,
   SYSTEM_BUS_SYSTEM_BUS_AUDIO_SYS_LSB_POSN           = (int)3,
   SYSTEM_BUS_SYSTEM_BUS_AUDIO_SYS_MSB_POSN           = (int)3
};
typedef enum system_bus_posn_enum system_bus_posn;

#define SYSTEM_BUS_HOST_SYS_MASK                 (0x00000002u)
#define SYSTEM_BUS_BT_SYS_MASK                   (0x00000004u)
#define SYSTEM_BUS_APPS_SYS_MASK                 (0x00000010u)
#define SYSTEM_BUS_CURATOR_MASK                  (0x00000001u)
#define SYSTEM_BUS_PIO_CTRL_SYS_MASK             (0x00000020u)
#define SYSTEM_BUS_AUDIO_SYS_MASK                (0x00000008u)

enum system_bus_enum
{
   SYSTEM_BUS_HOST_SYS                      = (int)0x1,
   SYSTEM_BUS_BT_SYS                        = (int)0x2,
   SYSTEM_BUS_APPS_SYS                      = (int)0x4,
   SYSTEM_BUS_CURATOR                       = (int)0x0,
   SYSTEM_BUS_PIO_CTRL_SYS                  = (int)0x5,
   SYSTEM_BUS_AUDIO_SYS                     = (int)0x3,
   SYSTEM_BUS_NUM_SUBSYSTEMS                = (int)0x6,
   SYSTEM_BUS_RANGE_LSB                     = (int)0x0,
   SYSTEM_BUS_RANGE_MSB                     = (int)0x5,
   MAX_SYSTEM_BUS                           = (int)0x6
};
typedef enum system_bus_enum system_bus;


enum deep_sleep_wakeup_posn_enum
{
   DEEP_SLEEP_WAKEUP_TIMER_POSN                       = (int)0,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_TIMER_LSB_POSN = (int)0,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_TIMER_MSB_POSN = (int)0,
   DEEP_SLEEP_WAKEUP_XTAL_POSN                        = (int)1,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_XTAL_LSB_POSN  = (int)1,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_XTAL_MSB_POSN  = (int)1,
   DEEP_SLEEP_WAKEUP_SPIB_POSN                        = (int)2,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_SPIB_LSB_POSN  = (int)2,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_SPIB_MSB_POSN  = (int)2,
   DEEP_SLEEP_WAKEUP_PIO_POSN                         = (int)3,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_PIO_LSB_POSN   = (int)3,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_PIO_MSB_POSN   = (int)3,
   DEEP_SLEEP_WAKEUP_TBRIDGE2_POSN                    = (int)11,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_TBRIDGE2_LSB_POSN = (int)11,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_TBRIDGE2_MSB_POSN = (int)11,
   DEEP_SLEEP_WAKEUP_PMU_TS_POSN                      = (int)12,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_PMU_TS_LSB_POSN = (int)12,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_PMU_TS_MSB_POSN = (int)12,
   DEEP_SLEEP_WAKEUP_USB_POSN                         = (int)8,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_USB_LSB_POSN   = (int)8,
   DEEP_SLEEP_WAKEUP_DEEP_SLEEP_WAKEUP_USB_MSB_POSN   = (int)8
};
typedef enum deep_sleep_wakeup_posn_enum deep_sleep_wakeup_posn;

#define DEEP_SLEEP_WAKEUP_TIMER_MASK             (0x00000001u)
#define DEEP_SLEEP_WAKEUP_XTAL_MASK              (0x00000002u)
#define DEEP_SLEEP_WAKEUP_SPIB_MASK              (0x00000004u)
#define DEEP_SLEEP_WAKEUP_PIO_MASK               (0x00000008u)
#define DEEP_SLEEP_WAKEUP_TBRIDGE2_MASK          (0x00000800u)
#define DEEP_SLEEP_WAKEUP_PMU_TS_MASK            (0x00001000u)
#define DEEP_SLEEP_WAKEUP_USB_MASK               (0x00000100u)

enum deep_sleep_wakeup_enum
{
   DEEP_SLEEP_WAKEUP_TOTAL_NUM_SOURCES      = (int)0xD,
   MAX_DEEP_SLEEP_WAKEUP                    = (int)0xD
};
typedef enum deep_sleep_wakeup_enum deep_sleep_wakeup;


enum scan_config_enum
{
   ATPG_NUM_SHIFT_CLK                       = (int)0x4,
   ATPG_NUM_COMPRESSOR_CHANNEL              = (int)0x4,
   ATPG_NUM_DECOMPRESSOR_CHANNEL            = (int)0x258,
   ATPG_NUM_STUCKATONLY_CHANNEL             = (int)0x0,
   MAX_SCAN_CONFIG                          = (int)0x258
};
typedef enum scan_config_enum scan_config;


#endif /* IO_DEFS_MODULE_CHIP */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_CHIP */




#if defined(IO_DEFS_MODULE_SQIF_DATAPATH) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_SQIF_DATAPATH
#define __IO_DEFS_H__IO_DEFS_MODULE_SQIF_DATAPATH

/* -- sqif_datapath -- SQIF Datapath Registers -- */

enum sqif_datapath_debug_mux_sel_enum
{
   DPATH_DBG_SQIF                           = (int)0x0,
   DPATH_DBG_DECRYPT                        = (int)0x1,
   DPATH_DBG_ARBITER_0                      = (int)0x2,
   DPATH_DBG_ARBITER_1                      = (int)0x3,
   MAX_SQIF_DATAPATH_DEBUG_MUX_SEL          = (int)0x3
};
#define NUM_SQIF_DATAPATH_DEBUG_MUX_SEL (0x4)
typedef enum sqif_datapath_debug_mux_sel_enum sqif_datapath_debug_mux_sel;


enum sqif_datapath_bank_posn_enum
{
   REG_INDEX_LSB_POSN                                 = (int)0,
   SQIF_DATAPATH_BANK_REG_INDEX_LSB_POSN              = (int)0,
   REG_INDEX_MSB_POSN                                 = (int)3,
   SQIF_DATAPATH_BANK_REG_INDEX_MSB_POSN              = (int)3,
   DEBUG_SEL_L_LSB_POSN                               = (int)4,
   SQIF_DATAPATH_BANK_DEBUG_SEL_L_LSB_POSN            = (int)4,
   DEBUG_SEL_L_MSB_POSN                               = (int)5,
   SQIF_DATAPATH_BANK_DEBUG_SEL_L_MSB_POSN            = (int)5,
   DEBUG_SEL_H_LSB_POSN                               = (int)6,
   SQIF_DATAPATH_BANK_DEBUG_SEL_H_LSB_POSN            = (int)6,
   DEBUG_SEL_H_MSB_POSN                               = (int)7,
   SQIF_DATAPATH_BANK_DEBUG_SEL_H_MSB_POSN            = (int)7,
   PERF_PORT_SEL_LSB_POSN                             = (int)8,
   SQIF_DATAPATH_BANK_PERF_PORT_SEL_LSB_POSN          = (int)8,
   PERF_PORT_SEL_MSB_POSN                             = (int)10,
   SQIF_DATAPATH_BANK_PERF_PORT_SEL_MSB_POSN          = (int)10,
   PERF_PORT_SEL_ALL_POSN                             = (int)11,
   SQIF_DATAPATH_BANK_PERF_PORT_SEL_ALL_LSB_POSN      = (int)11,
   SQIF_DATAPATH_BANK_PERF_PORT_SEL_ALL_MSB_POSN      = (int)11,
   PERF_CTR_CLEAR_POSN                                = (int)12,
   SQIF_DATAPATH_BANK_PERF_CTR_CLEAR_LSB_POSN         = (int)12,
   SQIF_DATAPATH_BANK_PERF_CTR_CLEAR_MSB_POSN         = (int)12,
   PERF_CTR_SEL_LSB_POSN                              = (int)13,
   SQIF_DATAPATH_BANK_PERF_CTR_SEL_LSB_POSN           = (int)13,
   PERF_CTR_SEL_MSB_POSN                              = (int)14,
   SQIF_DATAPATH_BANK_PERF_CTR_SEL_MSB_POSN           = (int)14
};
typedef enum sqif_datapath_bank_posn_enum sqif_datapath_bank_posn;

#define REG_INDEX_LSB_MASK                       (0x00000001u)
#define REG_INDEX_MSB_MASK                       (0x00000008u)
#define DEBUG_SEL_L_LSB_MASK                     (0x00000010u)
#define DEBUG_SEL_L_MSB_MASK                     (0x00000020u)
#define DEBUG_SEL_H_LSB_MASK                     (0x00000040u)
#define DEBUG_SEL_H_MSB_MASK                     (0x00000080u)
#define PERF_PORT_SEL_LSB_MASK                   (0x00000100u)
#define PERF_PORT_SEL_MSB_MASK                   (0x00000400u)
#define PERF_PORT_SEL_ALL_MASK                   (0x00000800u)
#define PERF_CTR_CLEAR_MASK                      (0x00001000u)
#define PERF_CTR_SEL_LSB_MASK                    (0x00002000u)
#define PERF_CTR_SEL_MSB_MASK                    (0x00004000u)

enum sqif_datapath_status_posn_enum
{
   IRQ_PENDING_POSN                                   = (int)0,
   SQIF_DATAPATH_STATUS_IRQ_PENDING_LSB_POSN          = (int)0,
   SQIF_DATAPATH_STATUS_IRQ_PENDING_MSB_POSN          = (int)0,
   SOFT_RESET_POSN                                    = (int)1,
   SQIF_DATAPATH_STATUS_SOFT_RESET_LSB_POSN           = (int)1,
   SQIF_DATAPATH_STATUS_SOFT_RESET_MSB_POSN           = (int)1
};
typedef enum sqif_datapath_status_posn_enum sqif_datapath_status_posn;

#define IRQ_PENDING_MASK                         (0x00000001u)
#define SOFT_RESET_MASK                          (0x00000002u)

enum sqif_datapath_perf_counter_posn_enum
{
   SQIF_DATAPATH_PERF_COUNTER_LSB_POSN                = (int)0,
   SQIF_DATAPATH_PERF_COUNTER_SQIF_DATAPATH_PERF_COUNTER_LSB_POSN = (int)0,
   SQIF_DATAPATH_PERF_COUNTER_MSB_POSN                = (int)31,
   SQIF_DATAPATH_PERF_COUNTER_SQIF_DATAPATH_PERF_COUNTER_MSB_POSN = (int)31
};
typedef enum sqif_datapath_perf_counter_posn_enum sqif_datapath_perf_counter_posn;

#define SQIF_DATAPATH_PERF_COUNTER_LSB_MASK      (0x00000001u)
#define SQIF_DATAPATH_PERF_COUNTER_MSB_MASK      (0x80000000u)

enum sqif_datapath__access_ctrl_enum_posn_enum
{
   SQIF_DATAPATH__P0_ACCESS_PERMISSION_POSN           = (int)0,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   SQIF_DATAPATH__P1_ACCESS_PERMISSION_POSN           = (int)1,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   SQIF_DATAPATH__P2_ACCESS_PERMISSION_POSN           = (int)2,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   SQIF_DATAPATH__P3_ACCESS_PERMISSION_POSN           = (int)3,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   SQIF_DATAPATH__ACCESS_CTRL_ENUM_SQIF_DATAPATH__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum sqif_datapath__access_ctrl_enum_posn_enum sqif_datapath__access_ctrl_enum_posn;

#define SQIF_DATAPATH__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define SQIF_DATAPATH__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define SQIF_DATAPATH__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define SQIF_DATAPATH__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum sqif_datapath__p0_access_permission_enum
{
   SQIF_DATAPATH__P0_ACCESS_BLOCKED         = (int)0x0,
   SQIF_DATAPATH__P0_ACCESS_UNBLOCKED       = (int)0x1,
   MAX_SQIF_DATAPATH__P0_ACCESS_PERMISSION  = (int)0x1
};
#define NUM_SQIF_DATAPATH__P0_ACCESS_PERMISSION (0x2)
typedef enum sqif_datapath__p0_access_permission_enum sqif_datapath__p0_access_permission;


enum sqif_datapath__p1_access_permission_enum
{
   SQIF_DATAPATH__P1_ACCESS_BLOCKED         = (int)0x0,
   SQIF_DATAPATH__P1_ACCESS_UNBLOCKED       = (int)0x1,
   MAX_SQIF_DATAPATH__P1_ACCESS_PERMISSION  = (int)0x1
};
#define NUM_SQIF_DATAPATH__P1_ACCESS_PERMISSION (0x2)
typedef enum sqif_datapath__p1_access_permission_enum sqif_datapath__p1_access_permission;


enum sqif_datapath__p2_access_permission_enum
{
   SQIF_DATAPATH__P2_ACCESS_BLOCKED         = (int)0x0,
   SQIF_DATAPATH__P2_ACCESS_UNBLOCKED       = (int)0x1,
   MAX_SQIF_DATAPATH__P2_ACCESS_PERMISSION  = (int)0x1
};
#define NUM_SQIF_DATAPATH__P2_ACCESS_PERMISSION (0x2)
typedef enum sqif_datapath__p2_access_permission_enum sqif_datapath__p2_access_permission;


enum sqif_datapath__p3_access_permission_enum
{
   SQIF_DATAPATH__P3_ACCESS_BLOCKED         = (int)0x0,
   SQIF_DATAPATH__P3_ACCESS_UNBLOCKED       = (int)0x1,
   MAX_SQIF_DATAPATH__P3_ACCESS_PERMISSION  = (int)0x1
};
#define NUM_SQIF_DATAPATH__P3_ACCESS_PERMISSION (0x2)
typedef enum sqif_datapath__p3_access_permission_enum sqif_datapath__p3_access_permission;


enum sqif_datapath__mutex_lock_enum_enum
{
   SQIF_DATAPATH__MUTEX_AVAILABLE           = (int)0x0,
   SQIF_DATAPATH__MUTEX_CLAIMED_BY_P0       = (int)0x1,
   SQIF_DATAPATH__MUTEX_CLAIMED_BY_P1       = (int)0x2,
   SQIF_DATAPATH__MUTEX_CLAIMED_BY_P2       = (int)0x4,
   SQIF_DATAPATH__MUTEX_CLAIMED_BY_P3       = (int)0x8,
   SQIF_DATAPATH__MUTEX_DISABLED            = (int)0xF,
   MAX_SQIF_DATAPATH__MUTEX_LOCK_ENUM       = (int)0xF
};
typedef enum sqif_datapath__mutex_lock_enum_enum sqif_datapath__mutex_lock_enum;


enum sqif_datapath_access_ctrl_posn_enum
{
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   SQIF_DATAPATH_ACCESS_CTRL_SQIF_DATAPATH__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum sqif_datapath_access_ctrl_posn_enum sqif_datapath_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_SQIF_DATAPATH */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_SQIF_DATAPATH */







#if defined(IO_DEFS_MODULE_KALIMBA_PIO_INT) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_PIO_INT
#define __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_PIO_INT

/* -- kalimba_pio_int -- Generic Kalimba Timer registers -- */

enum kalimba_pio_int_timer_enables_posn_enum
{
   KALIMBA_PIO_INT_TIMER_EN_POSN                      = (int)0,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_EN_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_EN_MSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_SW1_EVENT_EN_POSN            = (int)1,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW1_EVENT_EN_LSB_POSN = (int)1,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW1_EVENT_EN_MSB_POSN = (int)1,
   KALIMBA_PIO_INT_TIMER_SW2_EVENT_EN_POSN            = (int)2,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW2_EVENT_EN_LSB_POSN = (int)2,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW2_EVENT_EN_MSB_POSN = (int)2,
   KALIMBA_PIO_INT_TIMER_SW3_EVENT_EN_POSN            = (int)3,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW3_EVENT_EN_LSB_POSN = (int)3,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_SW3_EVENT_EN_MSB_POSN = (int)3,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_EN_POSN           = (int)4,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_EN_LSB_POSN = (int)4,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_EN_MSB_POSN = (int)4,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_INVERT_EN_POSN    = (int)5,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_INVERT_EN_LSB_POSN = (int)5,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_INVERT_EN_MSB_POSN = (int)5,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_DEFAULT_POSN      = (int)6,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_DEFAULT_LSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_DEFAULT_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_EN_POSN           = (int)7,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_EN_LSB_POSN = (int)7,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_EN_MSB_POSN = (int)7,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_INVERT_EN_POSN    = (int)8,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_INVERT_EN_LSB_POSN = (int)8,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_INVERT_EN_MSB_POSN = (int)8,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_DEFAULT_POSN      = (int)9,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_DEFAULT_LSB_POSN = (int)9,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_DEFAULT_MSB_POSN = (int)9,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_RISING_EN_POSN   = (int)10,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_RISING_EN_LSB_POSN = (int)10,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_RISING_EN_MSB_POSN = (int)10,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_FALLING_EN_POSN  = (int)11,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_FALLING_EN_LSB_POSN = (int)11,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_FALLING_EN_MSB_POSN = (int)11,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_RISING_EN_POSN   = (int)12,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_RISING_EN_LSB_POSN = (int)12,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_RISING_EN_MSB_POSN = (int)12,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_FALLING_EN_POSN  = (int)13,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_FALLING_EN_LSB_POSN = (int)13,
   KALIMBA_PIO_INT_TIMER_ENABLES_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_FALLING_EN_MSB_POSN = (int)13
};
typedef enum kalimba_pio_int_timer_enables_posn_enum kalimba_pio_int_timer_enables_posn;

#define KALIMBA_PIO_INT_TIMER_EN_MASK            (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_SW1_EVENT_EN_MASK  (0x00000002u)
#define KALIMBA_PIO_INT_TIMER_SW2_EVENT_EN_MASK  (0x00000004u)
#define KALIMBA_PIO_INT_TIMER_SW3_EVENT_EN_MASK  (0x00000008u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_EN_MASK (0x00000010u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_INVERT_EN_MASK (0x00000020u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_DEFAULT_MASK (0x00000040u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_EN_MASK (0x00000080u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_INVERT_EN_MASK (0x00000100u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_DEFAULT_MASK (0x00000200u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_RISING_EN_MASK (0x00000400u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_FALLING_EN_MASK (0x00000800u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_RISING_EN_MASK (0x00001000u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_FALLING_EN_MASK (0x00002000u)

enum kalimba_pio_int_timer_pio1_event_sel_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_EN_POSN       = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_EN_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_EN_MSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_LSB_POSN  = (int)1,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_LSB_POSN = (int)1,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_MSB_POSN  = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio1_event_sel_posn_enum kalimba_pio_int_timer_pio1_event_sel_posn;

#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_EN_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_LSB_MASK (0x00000002u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_SEL_POS_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio2_event_sel_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_EN_POSN       = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_EN_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_EN_MSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_LSB_POSN  = (int)1,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_LSB_POSN = (int)1,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_MSB_POSN  = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio2_event_sel_posn_enum kalimba_pio_int_timer_pio2_event_sel_posn;

#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_EN_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_LSB_MASK (0x00000002u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_SEL_POS_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio_strobe_select_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_LSB_POSN         = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_MSB_POSN         = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_UNUSED_POSN       = (int)7,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE_UNUSED_LSB_POSN = (int)7,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE_UNUSED_MSB_POSN = (int)7,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_LSB_POSN         = (int)8,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_LSB_POSN = (int)8,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_MSB_POSN         = (int)14,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE_SELECT_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_MSB_POSN = (int)14
};
typedef enum kalimba_pio_int_timer_pio_strobe_select_posn_enum kalimba_pio_int_timer_pio_strobe_select_posn;

#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_MSB_MASK (0x00000040u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE_UNUSED_MASK (0x00000080u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_LSB_MASK (0x00000100u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_MSB_MASK (0x00004000u)

enum kalimba_pio_int_timer_pio1_event_period_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_LSB_POSN   = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_MSB_POSN   = (int)15,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_MSB_POSN = (int)15
};
typedef enum kalimba_pio_int_timer_pio1_event_period_posn_enum kalimba_pio_int_timer_pio1_event_period_posn;

#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_MSB_MASK (0x00008000u)

enum kalimba_pio_int_timer_pio1_event_period_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio1_event_period_fine_posn_enum kalimba_pio_int_timer_pio1_event_period_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_PERIOD_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio1_event_time_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_LSB_POSN     = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_MSB_POSN     = (int)31,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_pio1_event_time_posn_enum kalimba_pio_int_timer_pio1_event_time_posn;

#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_MSB_MASK (0x80000000u)

enum kalimba_pio_int_timer_pio1_event_time_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio1_event_time_fine_posn_enum kalimba_pio_int_timer_pio1_event_time_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO1_EVENT_TIME_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio2_event_period_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_LSB_POSN   = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_MSB_POSN   = (int)15,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_MSB_POSN = (int)15
};
typedef enum kalimba_pio_int_timer_pio2_event_period_posn_enum kalimba_pio_int_timer_pio2_event_period_posn;

#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_MSB_MASK (0x00008000u)

enum kalimba_pio_int_timer_pio2_event_period_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio2_event_period_fine_posn_enum kalimba_pio_int_timer_pio2_event_period_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_PERIOD_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio2_event_time_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_LSB_POSN     = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_MSB_POSN     = (int)31,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_pio2_event_time_posn_enum kalimba_pio_int_timer_pio2_event_time_posn;

#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_MSB_MASK (0x80000000u)

enum kalimba_pio_int_timer_pio2_event_time_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio2_event_time_fine_posn_enum kalimba_pio_int_timer_pio2_event_time_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO2_EVENT_TIME_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio_strobe1_time_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_LSB_POSN    = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_MSB_POSN    = (int)31,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_pio_strobe1_time_posn_enum kalimba_pio_int_timer_pio_strobe1_time_posn;

#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_MSB_MASK (0x80000000u)

enum kalimba_pio_int_timer_pio_strobe1_time_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio_strobe1_time_fine_posn_enum kalimba_pio_int_timer_pio_strobe1_time_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE1_TIME_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_pio_strobe2_time_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_LSB_POSN    = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_MSB_POSN    = (int)31,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_pio_strobe2_time_posn_enum kalimba_pio_int_timer_pio_strobe2_time_posn;

#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_MSB_MASK (0x80000000u)

enum kalimba_pio_int_timer_pio_strobe2_time_fine_posn_enum
{
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_MSB_POSN = (int)6,
   KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_MSB_POSN = (int)6
};
typedef enum kalimba_pio_int_timer_pio_strobe2_time_fine_posn_enum kalimba_pio_int_timer_pio_strobe2_time_fine_posn;

#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_PIO_STROBE2_TIME_FINE_MSB_MASK (0x00000040u)

enum kalimba_pio_int_timer_sw3_event_time_posn_enum
{
   KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_LSB_POSN      = (int)0,
   KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_MSB_POSN      = (int)31,
   KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_sw3_event_time_posn_enum kalimba_pio_int_timer_sw3_event_time_posn;

#define KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_LSB_MASK (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_SW3_EVENT_TIME_MSB_MASK (0x80000000u)

enum kalimba_pio_int_timer_time2_posn_enum
{
   KALIMBA_PIO_INT_TIMER_TIME2_LSB_POSN               = (int)0,
   KALIMBA_PIO_INT_TIMER_TIME2_KALIMBA_PIO_INT_TIMER_TIME2_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_TIMER_TIME2_MSB_POSN               = (int)31,
   KALIMBA_PIO_INT_TIMER_TIME2_KALIMBA_PIO_INT_TIMER_TIME2_MSB_POSN = (int)31
};
typedef enum kalimba_pio_int_timer_time2_posn_enum kalimba_pio_int_timer_time2_posn;

#define KALIMBA_PIO_INT_TIMER_TIME2_LSB_MASK     (0x00000001u)
#define KALIMBA_PIO_INT_TIMER_TIME2_MSB_MASK     (0x80000000u)

enum kalimba_pio_int__access_ctrl_enum_posn_enum
{
   KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_POSN         = (int)0,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_POSN         = (int)1,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_POSN         = (int)2,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_POSN         = (int)3,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   KALIMBA_PIO_INT__ACCESS_CTRL_ENUM_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum kalimba_pio_int__access_ctrl_enum_posn_enum kalimba_pio_int__access_ctrl_enum_posn;

#define KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum kalimba_pio_int__p0_access_permission_enum
{
   KALIMBA_PIO_INT__P0_ACCESS_BLOCKED       = (int)0x0,
   KALIMBA_PIO_INT__P0_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION (0x2)
typedef enum kalimba_pio_int__p0_access_permission_enum kalimba_pio_int__p0_access_permission;


enum kalimba_pio_int__p1_access_permission_enum
{
   KALIMBA_PIO_INT__P1_ACCESS_BLOCKED       = (int)0x0,
   KALIMBA_PIO_INT__P1_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION (0x2)
typedef enum kalimba_pio_int__p1_access_permission_enum kalimba_pio_int__p1_access_permission;


enum kalimba_pio_int__p2_access_permission_enum
{
   KALIMBA_PIO_INT__P2_ACCESS_BLOCKED       = (int)0x0,
   KALIMBA_PIO_INT__P2_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION (0x2)
typedef enum kalimba_pio_int__p2_access_permission_enum kalimba_pio_int__p2_access_permission;


enum kalimba_pio_int__p3_access_permission_enum
{
   KALIMBA_PIO_INT__P3_ACCESS_BLOCKED       = (int)0x0,
   KALIMBA_PIO_INT__P3_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION (0x2)
typedef enum kalimba_pio_int__p3_access_permission_enum kalimba_pio_int__p3_access_permission;


enum kalimba_pio_int__mutex_lock_enum_enum
{
   KALIMBA_PIO_INT__MUTEX_AVAILABLE         = (int)0x0,
   KALIMBA_PIO_INT__MUTEX_CLAIMED_BY_P0     = (int)0x1,
   KALIMBA_PIO_INT__MUTEX_CLAIMED_BY_P1     = (int)0x2,
   KALIMBA_PIO_INT__MUTEX_CLAIMED_BY_P2     = (int)0x4,
   KALIMBA_PIO_INT__MUTEX_CLAIMED_BY_P3     = (int)0x8,
   KALIMBA_PIO_INT__MUTEX_DISABLED          = (int)0xF,
   MAX_KALIMBA_PIO_INT__MUTEX_LOCK_ENUM     = (int)0xF
};
typedef enum kalimba_pio_int__mutex_lock_enum_enum kalimba_pio_int__mutex_lock_enum;


enum kalimba_pio_int_access_ctrl_posn_enum
{
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   KALIMBA_PIO_INT_ACCESS_CTRL_KALIMBA_PIO_INT__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum kalimba_pio_int_access_ctrl_posn_enum kalimba_pio_int_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_KALIMBA_PIO_INT */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_KALIMBA_PIO_INT */


#if defined(IO_DEFS_MODULE_BUS_PROC_STREAM) 

#ifndef __IO_DEFS_H__IO_DEFS_MODULE_BUS_PROC_STREAM
#define __IO_DEFS_H__IO_DEFS_MODULE_BUS_PROC_STREAM

/* -- bus_proc_stream -- Processor-driven transaction stream generation control registers. -- */

enum bus_proc_stream_config_posn_enum
{
   BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_LSB_POSN        = (int)0,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_LSB_POSN = (int)0,
   BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_MSB_POSN        = (int)3,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_MSB_POSN = (int)3,
   BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_LSB_POSN      = (int)4,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_LSB_POSN = (int)4,
   BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_MSB_POSN      = (int)7,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_MSB_POSN = (int)7,
   BUS_PROC_STREAM_CONFIG_OPCODE_LSB_POSN             = (int)8,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_OPCODE_LSB_POSN = (int)8,
   BUS_PROC_STREAM_CONFIG_OPCODE_MSB_POSN             = (int)11,
   BUS_PROC_STREAM_CONFIG_BUS_PROC_STREAM_CONFIG_OPCODE_MSB_POSN = (int)11
};
typedef enum bus_proc_stream_config_posn_enum bus_proc_stream_config_posn;

#define BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_LSB_MASK (0x00000001u)
#define BUS_PROC_STREAM_CONFIG_SYS_DEST_ID_MSB_MASK (0x00000008u)
#define BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_LSB_MASK (0x00000010u)
#define BUS_PROC_STREAM_CONFIG_BLOCK_DEST_ID_MSB_MASK (0x00000080u)
#define BUS_PROC_STREAM_CONFIG_OPCODE_LSB_MASK   (0x00000100u)
#define BUS_PROC_STREAM_CONFIG_OPCODE_MSB_MASK   (0x00000800u)

enum bus_proc_stream_final_posn_enum
{
   BUS_PROC_STREAM_FINAL_PAYLOAD_LSB_POSN             = (int)0,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_PAYLOAD_LSB_POSN = (int)0,
   BUS_PROC_STREAM_FINAL_PAYLOAD_MSB_POSN             = (int)7,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_PAYLOAD_MSB_POSN = (int)7,
   BUS_PROC_STREAM_FINAL_TAG_LSB_POSN                 = (int)8,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_TAG_LSB_POSN = (int)8,
   BUS_PROC_STREAM_FINAL_TAG_MSB_POSN                 = (int)11,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_TAG_MSB_POSN = (int)11,
   BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_LSB_POSN     = (int)12,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_LSB_POSN = (int)12,
   BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_MSB_POSN     = (int)15,
   BUS_PROC_STREAM_FINAL_BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_MSB_POSN = (int)15
};
typedef enum bus_proc_stream_final_posn_enum bus_proc_stream_final_posn;

#define BUS_PROC_STREAM_FINAL_PAYLOAD_LSB_MASK   (0x00000001u)
#define BUS_PROC_STREAM_FINAL_PAYLOAD_MSB_MASK   (0x00000080u)
#define BUS_PROC_STREAM_FINAL_TAG_LSB_MASK       (0x00000100u)
#define BUS_PROC_STREAM_FINAL_TAG_MSB_MASK       (0x00000800u)
#define BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_LSB_MASK (0x00001000u)
#define BUS_PROC_STREAM_FINAL_BLOCK_SOURCE_ID_MSB_MASK (0x00008000u)

enum bus_proc_stream_payload0_posn_enum
{
   BUS_PROC_STREAM_PAYLOAD0_LSB_POSN                  = (int)0,
   BUS_PROC_STREAM_PAYLOAD0_BUS_PROC_STREAM_PAYLOAD0_LSB_POSN = (int)0,
   BUS_PROC_STREAM_PAYLOAD0_MSB_POSN                  = (int)31,
   BUS_PROC_STREAM_PAYLOAD0_BUS_PROC_STREAM_PAYLOAD0_MSB_POSN = (int)31
};
typedef enum bus_proc_stream_payload0_posn_enum bus_proc_stream_payload0_posn;

#define BUS_PROC_STREAM_PAYLOAD0_LSB_MASK        (0x00000001u)
#define BUS_PROC_STREAM_PAYLOAD0_MSB_MASK        (0x80000000u)

enum bus_proc_stream_payload1_posn_enum
{
   BUS_PROC_STREAM_PAYLOAD1_LSB_POSN                  = (int)0,
   BUS_PROC_STREAM_PAYLOAD1_BUS_PROC_STREAM_PAYLOAD1_LSB_POSN = (int)0,
   BUS_PROC_STREAM_PAYLOAD1_MSB_POSN                  = (int)31,
   BUS_PROC_STREAM_PAYLOAD1_BUS_PROC_STREAM_PAYLOAD1_MSB_POSN = (int)31
};
typedef enum bus_proc_stream_payload1_posn_enum bus_proc_stream_payload1_posn;

#define BUS_PROC_STREAM_PAYLOAD1_LSB_MASK        (0x00000001u)
#define BUS_PROC_STREAM_PAYLOAD1_MSB_MASK        (0x80000000u)

enum bus_proc_stream__access_ctrl_enum_posn_enum
{
   BUS_PROC_STREAM__P0_ACCESS_PERMISSION_POSN         = (int)0,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   BUS_PROC_STREAM__P1_ACCESS_PERMISSION_POSN         = (int)1,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   BUS_PROC_STREAM__P2_ACCESS_PERMISSION_POSN         = (int)2,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   BUS_PROC_STREAM__P3_ACCESS_PERMISSION_POSN         = (int)3,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   BUS_PROC_STREAM__ACCESS_CTRL_ENUM_BUS_PROC_STREAM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum bus_proc_stream__access_ctrl_enum_posn_enum bus_proc_stream__access_ctrl_enum_posn;

#define BUS_PROC_STREAM__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define BUS_PROC_STREAM__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define BUS_PROC_STREAM__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define BUS_PROC_STREAM__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum bus_proc_stream__p0_access_permission_enum
{
   BUS_PROC_STREAM__P0_ACCESS_BLOCKED       = (int)0x0,
   BUS_PROC_STREAM__P0_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_BUS_PROC_STREAM__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_BUS_PROC_STREAM__P0_ACCESS_PERMISSION (0x2)
typedef enum bus_proc_stream__p0_access_permission_enum bus_proc_stream__p0_access_permission;


enum bus_proc_stream__p1_access_permission_enum
{
   BUS_PROC_STREAM__P1_ACCESS_BLOCKED       = (int)0x0,
   BUS_PROC_STREAM__P1_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_BUS_PROC_STREAM__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_BUS_PROC_STREAM__P1_ACCESS_PERMISSION (0x2)
typedef enum bus_proc_stream__p1_access_permission_enum bus_proc_stream__p1_access_permission;


enum bus_proc_stream__p2_access_permission_enum
{
   BUS_PROC_STREAM__P2_ACCESS_BLOCKED       = (int)0x0,
   BUS_PROC_STREAM__P2_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_BUS_PROC_STREAM__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_BUS_PROC_STREAM__P2_ACCESS_PERMISSION (0x2)
typedef enum bus_proc_stream__p2_access_permission_enum bus_proc_stream__p2_access_permission;


enum bus_proc_stream__p3_access_permission_enum
{
   BUS_PROC_STREAM__P3_ACCESS_BLOCKED       = (int)0x0,
   BUS_PROC_STREAM__P3_ACCESS_UNBLOCKED     = (int)0x1,
   MAX_BUS_PROC_STREAM__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_BUS_PROC_STREAM__P3_ACCESS_PERMISSION (0x2)
typedef enum bus_proc_stream__p3_access_permission_enum bus_proc_stream__p3_access_permission;


enum bus_proc_stream__mutex_lock_enum_enum
{
   BUS_PROC_STREAM__MUTEX_AVAILABLE         = (int)0x0,
   BUS_PROC_STREAM__MUTEX_CLAIMED_BY_P0     = (int)0x1,
   BUS_PROC_STREAM__MUTEX_CLAIMED_BY_P1     = (int)0x2,
   BUS_PROC_STREAM__MUTEX_CLAIMED_BY_P2     = (int)0x4,
   BUS_PROC_STREAM__MUTEX_CLAIMED_BY_P3     = (int)0x8,
   BUS_PROC_STREAM__MUTEX_DISABLED          = (int)0xF,
   MAX_BUS_PROC_STREAM__MUTEX_LOCK_ENUM     = (int)0xF
};
typedef enum bus_proc_stream__mutex_lock_enum_enum bus_proc_stream__mutex_lock_enum;


enum bus_proc_stream_access_ctrl_posn_enum
{
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   BUS_PROC_STREAM_ACCESS_CTRL_BUS_PROC_STREAM__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum bus_proc_stream_access_ctrl_posn_enum bus_proc_stream_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_BUS_PROC_STREAM */
#endif /* __IO_DEFS_H__IO_DEFS_MODULE_BUS_PROC_STREAM */



