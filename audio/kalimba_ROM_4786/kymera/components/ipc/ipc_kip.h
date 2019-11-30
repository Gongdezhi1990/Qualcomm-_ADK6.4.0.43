/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/** \file
 *
 *  This is the main public project header for the \c ipc LUT library.
 *
 */
/****************************************************************************
Include Files
*/

#ifndef IPC_KIP_H
#define IPC_KIP_H

#include "ipc_lut.h"

/****************************************************************************
Public Type Declarations
*/

/**
 * Enumeration for processor identity number
 *
 * Note: This is same as hal_get_reg_processor_num(). We are defining this
 * only because io_defs.h hasn't provided this enum. Once io_defs.h provides this
 * this enum must be typedef'd to that.
 */
typedef enum
{
    IPC_PROCESSOR_0 = 0, /* Processor 0 */
    IPC_PROCESSOR_1 = 1,  /* Processor 1 */
    IPC_PROCESSOR_MAX

} IPC_PROCESSOR_ID_NUM;

/*
 * What 'target' to boot secondary processor from: ROM, SQIF, ...
 * Ultimately this will lead to a choice of 'nvmem_win_config_type_enum'.
 * Supported types may differ between chip types.
 */
typedef enum
{
    IPC_NVMEM_WIN_CONFIG_TYPE_ROM                 = 0x00,
    IPC_NVMEM_WIN_CONFIG_TYPE_SQIF_FLASH          = 0x01,
    IPC_NVMEM_WIN_CONFIG_TYPE_SQIF_SRAM           = 0x02,
    IPC_NVMEM_WIN_CONFIG_TYPE_MAX                 = 0x03,

} IPC_NVMEM_WIN_CONFIG_TYPE;

typedef struct
{
    uint16         seqno;
    uint16         msgid;
    uint16         length;
    uint16         *message;

} ipc_msg;

typedef enum IPC_DATA_DIRECTION
{
    IPC_DATA_CHANNEL_READ      = 0,
    IPC_DATA_CHANNEL_WRITE     = 1

} IPC_DATA_DIRECTION;

/****************************************************************************
Public Constant and macros
*/

/*
 * Sequence number macro
 */
#define IPC_NEXT_SEQNO()                                             \
                      (ipc_next_seqno())

/****************************************************************************
Public Variable Declarations
*/

/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Start IPC
 *
 */
extern void ipc_init(void);

/**
 * \brief ipc_next_seqno
 *
 * \return  next sequence number for external messages
 *
 */
extern uint16 ipc_next_seqno(void);

/**
 * \brief Send an external IPC message
 *
 * \param[in] channel id - Message channel ID (CS-336170-DD Figure 14 in 10.1.3)
 * \param[in] msg        - Pointer to message structure which contains :
 *                         > seqno   - The IPC message sequence ID
 *                         > msgid   - The IPC message ID
 *                         > length  - the number of uint16s in 'message' buffer
 *                         > message - Pointer to buffer of data to send over IPC
 *
 * \return  0 if successfully sent external message else error code
 */
extern IPC_STATUS ipc_send_message(uint16 channel_id, ipc_msg *msg);

#if defined(INSTALL_EFUSE_DUALCORE)
/**
 * \brief  Return pointer to copy of efuse block in aux_states.
 *         The original efuse block was retrieved from curator
 *         but is not visible from P1. Upon P1 start, when the
 *         aux_states are alocated, a copy of the efuses is
 *         kept there. This function returns a pointer to that
 *         copy. 
 *
 * \return Pointer to copy of efuses block
 */
extern uint16 *ipc_get_ptr_to_efuse_copy(void);
#endif

#endif /* IPC_KIP_H */
