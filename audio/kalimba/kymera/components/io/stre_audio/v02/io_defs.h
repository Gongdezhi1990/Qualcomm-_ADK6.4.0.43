
/* *************************************************************************  *
   COMMERCIAL IN CONFIDENCE
   Copyright (C) 2017 Qualcomm Technologies International Ltd.

   Qualcomm Technologies International Ltd.
   Churchill House,
   Cambridge Business Park,
   Cowley Park,
   Cambridge, CB4 0WZ. UK
   http://www.csr.com

   $Id: //depot/dspsw/stre_rom_v02/kalimba/kymera/components/io/stre_audio/v02/io_defs.h#2 $
   $Name$
   $Source$

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

#define CHIP_REGISTER_HASH 0xB8A3




#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_TRACE) 
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


#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_DOLOOP_CACHE) 
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_DOLOOP_CACHE

/* -- k32_doloop_cache -- Kalimba 32-bit DoLoop Cache Control registers. -- */

enum doloop_cache_config_posn_enum
{
   DOLOOP_CACHE_CONFIG_DOLOOP_EN_POSN                 = (int)0,
   DOLOOP_CACHE_CONFIG_DOLOOP_CACHE_CONFIG_DOLOOP_EN_LSB_POSN = (int)0,
   DOLOOP_CACHE_CONFIG_DOLOOP_CACHE_CONFIG_DOLOOP_EN_MSB_POSN = (int)0,
   DOLOOP_CACHE_CONFIG_COUNTERS_EN_POSN               = (int)1,
   DOLOOP_CACHE_CONFIG_DOLOOP_CACHE_CONFIG_COUNTERS_EN_LSB_POSN = (int)1,
   DOLOOP_CACHE_CONFIG_DOLOOP_CACHE_CONFIG_COUNTERS_EN_MSB_POSN = (int)1
};
typedef enum doloop_cache_config_posn_enum doloop_cache_config_posn;

#define DOLOOP_CACHE_CONFIG_DOLOOP_EN_MASK       (0x00000001u)
#define DOLOOP_CACHE_CONFIG_COUNTERS_EN_MASK     (0x00000002u)

enum doloop_cache_fill_count_posn_enum
{
   DOLOOP_CACHE_FILL_COUNT_LSB_POSN                   = (int)0,
   DOLOOP_CACHE_FILL_COUNT_DOLOOP_CACHE_FILL_COUNT_LSB_POSN = (int)0,
   DOLOOP_CACHE_FILL_COUNT_MSB_POSN                   = (int)31,
   DOLOOP_CACHE_FILL_COUNT_DOLOOP_CACHE_FILL_COUNT_MSB_POSN = (int)31
};
typedef enum doloop_cache_fill_count_posn_enum doloop_cache_fill_count_posn;

#define DOLOOP_CACHE_FILL_COUNT_LSB_MASK         (0x00000001u)
#define DOLOOP_CACHE_FILL_COUNT_MSB_MASK         (0x80000000u)

enum doloop_cache_hit_count_posn_enum
{
   DOLOOP_CACHE_HIT_COUNT_LSB_POSN                    = (int)0,
   DOLOOP_CACHE_HIT_COUNT_DOLOOP_CACHE_HIT_COUNT_LSB_POSN = (int)0,
   DOLOOP_CACHE_HIT_COUNT_MSB_POSN                    = (int)31,
   DOLOOP_CACHE_HIT_COUNT_DOLOOP_CACHE_HIT_COUNT_MSB_POSN = (int)31
};
typedef enum doloop_cache_hit_count_posn_enum doloop_cache_hit_count_posn;

#define DOLOOP_CACHE_HIT_COUNT_LSB_MASK          (0x00000001u)
#define DOLOOP_CACHE_HIT_COUNT_MSB_MASK          (0x80000000u)

#endif /* IO_DEFS_MODULE_K32_DOLOOP_CACHE */





#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_TIMERS) 
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






#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_PREFETCH) 
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















#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_MC_INTER_PROC_KEYHOLE) 
#define __IO_DEFS_H__IO_DEFS_MODULE_K32_MC_INTER_PROC_KEYHOLE

/* -- k32_mc_inter_proc_keyhole -- Kalimba 32-bit Multicore inter-processor communication keyhole register block -- */

enum inter_proc_keyhole_ctrl_posn_enum
{
   INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_LSB_POSN          = (int)0,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_LSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_MSB_POSN          = (int)3,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_MSB_POSN = (int)3,
   INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL_POSN        = (int)4,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL_LSB_POSN = (int)4,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL_MSB_POSN = (int)4,
   INTER_PROC_KEYHOLE_CTRL_CPU_SEL_LSB_POSN           = (int)5,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_CPU_SEL_LSB_POSN = (int)5,
   INTER_PROC_KEYHOLE_CTRL_CPU_SEL_MSB_POSN           = (int)6,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_CPU_SEL_MSB_POSN = (int)6,
   INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL_POSN             = (int)7,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL_LSB_POSN = (int)7,
   INTER_PROC_KEYHOLE_CTRL_INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL_MSB_POSN = (int)7
};
typedef enum inter_proc_keyhole_ctrl_posn_enum inter_proc_keyhole_ctrl_posn;

#define INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_LSB_MASK (0x00000001u)
#define INTER_PROC_KEYHOLE_CTRL_BYTE_SEL_MSB_MASK (0x00000008u)
#define INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL_MASK (0x00000010u)
#define INTER_PROC_KEYHOLE_CTRL_CPU_SEL_LSB_MASK (0x00000020u)
#define INTER_PROC_KEYHOLE_CTRL_CPU_SEL_MSB_MASK (0x00000040u)
#define INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL_MASK   (0x00000080u)

enum inter_proc_keyhole_ctrl_read_write_sel_enum
{
   INTER_PROC_KEYHOLE_CTRL_READ             = (int)0x0,
   INTER_PROC_KEYHOLE_CTRL_WRITE            = (int)0x1,
   MAX_INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL = (int)0x1
};
#define NUM_INTER_PROC_KEYHOLE_CTRL_READ_WRITE_SEL (0x2)
typedef enum inter_proc_keyhole_ctrl_read_write_sel_enum inter_proc_keyhole_ctrl_read_write_sel;


enum inter_proc_keyhole_ctrl_pm_dm_sel_enum
{
   INTER_PROC_KEYHOLE_CTRL_DM               = (int)0x0,
   INTER_PROC_KEYHOLE_CTRL_PM               = (int)0x1,
   MAX_INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL    = (int)0x1
};
#define NUM_INTER_PROC_KEYHOLE_CTRL_PM_DM_SEL (0x2)
typedef enum inter_proc_keyhole_ctrl_pm_dm_sel_enum inter_proc_keyhole_ctrl_pm_dm_sel;


enum inter_proc_keyhole_status_enum
{
   INTER_PROC_KEYHOLE_FREE                  = (int)0x0,
   INTER_PROC_KEYHOLE_BUSY                  = (int)0x1,
   MAX_INTER_PROC_KEYHOLE_STATUS            = (int)0x1
};
#define NUM_INTER_PROC_KEYHOLE_STATUS (0x2)
typedef enum inter_proc_keyhole_status_enum inter_proc_keyhole_status;


enum inter_proc_keyhole_addr_posn_enum
{
   INTER_PROC_KEYHOLE_ADDR_LSB_POSN                   = (int)0,
   INTER_PROC_KEYHOLE_ADDR_INTER_PROC_KEYHOLE_ADDR_LSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_ADDR_MSB_POSN                   = (int)31,
   INTER_PROC_KEYHOLE_ADDR_INTER_PROC_KEYHOLE_ADDR_MSB_POSN = (int)31
};
typedef enum inter_proc_keyhole_addr_posn_enum inter_proc_keyhole_addr_posn;

#define INTER_PROC_KEYHOLE_ADDR_LSB_MASK         (0x00000001u)
#define INTER_PROC_KEYHOLE_ADDR_MSB_MASK         (0x80000000u)

enum inter_proc_keyhole_read_data_posn_enum
{
   INTER_PROC_KEYHOLE_READ_DATA_LSB_POSN              = (int)0,
   INTER_PROC_KEYHOLE_READ_DATA_INTER_PROC_KEYHOLE_READ_DATA_LSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_READ_DATA_MSB_POSN              = (int)31,
   INTER_PROC_KEYHOLE_READ_DATA_INTER_PROC_KEYHOLE_READ_DATA_MSB_POSN = (int)31
};
typedef enum inter_proc_keyhole_read_data_posn_enum inter_proc_keyhole_read_data_posn;

#define INTER_PROC_KEYHOLE_READ_DATA_LSB_MASK    (0x00000001u)
#define INTER_PROC_KEYHOLE_READ_DATA_MSB_MASK    (0x80000000u)

enum inter_proc_keyhole_write_data_posn_enum
{
   INTER_PROC_KEYHOLE_WRITE_DATA_LSB_POSN             = (int)0,
   INTER_PROC_KEYHOLE_WRITE_DATA_INTER_PROC_KEYHOLE_WRITE_DATA_LSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_WRITE_DATA_MSB_POSN             = (int)31,
   INTER_PROC_KEYHOLE_WRITE_DATA_INTER_PROC_KEYHOLE_WRITE_DATA_MSB_POSN = (int)31
};
typedef enum inter_proc_keyhole_write_data_posn_enum inter_proc_keyhole_write_data_posn;

#define INTER_PROC_KEYHOLE_WRITE_DATA_LSB_MASK   (0x00000001u)
#define INTER_PROC_KEYHOLE_WRITE_DATA_MSB_MASK   (0x80000000u)

enum k32_mc_inter_proc_keyhole__access_ctrl_enum_posn_enum
{
   K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_POSN = (int)0,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_POSN = (int)1,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_POSN = (int)2,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_POSN = (int)3,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   K32_MC_INTER_PROC_KEYHOLE__ACCESS_CTRL_ENUM_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum k32_mc_inter_proc_keyhole__access_ctrl_enum_posn_enum k32_mc_inter_proc_keyhole__access_ctrl_enum_posn;

#define K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_MASK (0x00000001u)
#define K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_MASK (0x00000002u)
#define K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_MASK (0x00000004u)
#define K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_MASK (0x00000008u)

enum k32_mc_inter_proc_keyhole__p0_access_permission_enum
{
   K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_BLOCKED = (int)0x0,
   K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_UNBLOCKED = (int)0x1,
   MAX_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION = (int)0x1
};
#define NUM_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION (0x2)
typedef enum k32_mc_inter_proc_keyhole__p0_access_permission_enum k32_mc_inter_proc_keyhole__p0_access_permission;


enum k32_mc_inter_proc_keyhole__p1_access_permission_enum
{
   K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_BLOCKED = (int)0x0,
   K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_UNBLOCKED = (int)0x1,
   MAX_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION = (int)0x1
};
#define NUM_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION (0x2)
typedef enum k32_mc_inter_proc_keyhole__p1_access_permission_enum k32_mc_inter_proc_keyhole__p1_access_permission;


enum k32_mc_inter_proc_keyhole__p2_access_permission_enum
{
   K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_BLOCKED = (int)0x0,
   K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_UNBLOCKED = (int)0x1,
   MAX_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION = (int)0x1
};
#define NUM_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION (0x2)
typedef enum k32_mc_inter_proc_keyhole__p2_access_permission_enum k32_mc_inter_proc_keyhole__p2_access_permission;


enum k32_mc_inter_proc_keyhole__p3_access_permission_enum
{
   K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_BLOCKED = (int)0x0,
   K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_UNBLOCKED = (int)0x1,
   MAX_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION = (int)0x1
};
#define NUM_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION (0x2)
typedef enum k32_mc_inter_proc_keyhole__p3_access_permission_enum k32_mc_inter_proc_keyhole__p3_access_permission;


enum k32_mc_inter_proc_keyhole__mutex_lock_enum_enum
{
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_AVAILABLE = (int)0x0,
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_CLAIMED_BY_P0 = (int)0x1,
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_CLAIMED_BY_P1 = (int)0x2,
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_CLAIMED_BY_P2 = (int)0x4,
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_CLAIMED_BY_P3 = (int)0x8,
   K32_MC_INTER_PROC_KEYHOLE__MUTEX_DISABLED = (int)0xF,
   MAX_K32_MC_INTER_PROC_KEYHOLE__MUTEX_LOCK_ENUM = (int)0xF
};
typedef enum k32_mc_inter_proc_keyhole__mutex_lock_enum_enum k32_mc_inter_proc_keyhole__mutex_lock_enum;


enum inter_proc_keyhole_access_ctrl_posn_enum
{
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_LSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P0_ACCESS_PERMISSION_MSB_POSN = (int)0,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_LSB_POSN = (int)1,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P1_ACCESS_PERMISSION_MSB_POSN = (int)1,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_LSB_POSN = (int)2,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P2_ACCESS_PERMISSION_MSB_POSN = (int)2,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_LSB_POSN = (int)3,
   INTER_PROC_KEYHOLE_ACCESS_CTRL_K32_MC_INTER_PROC_KEYHOLE__P3_ACCESS_PERMISSION_MSB_POSN = (int)3
};
typedef enum inter_proc_keyhole_access_ctrl_posn_enum inter_proc_keyhole_access_ctrl_posn;


#endif /* IO_DEFS_MODULE_K32_MC_INTER_PROC_KEYHOLE */





#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_CHIP) 
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













#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_CORE) 
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





#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_MONITOR) 
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













#if !defined(__IO_DEFS_H__IO_DEFS_MODULE_K32_MISC) 
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


