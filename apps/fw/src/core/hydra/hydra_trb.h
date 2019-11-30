/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
#ifndef HYDRA_TRB_H
#define HYDRA_TRB_H

/*****************************************************************************
 * Dependencies
 ****************************************************************************/

#include "hydra/hydra_types.h"

#include "hydra/hydra_macros.h"
#include "hal/hal_transaction_types.h"

/*****************************************************************************
 * Interface
 ****************************************************************************/

#define STREAM_SRC_BLOCK_ID_DEEP_SLEEP          0
#define STREAM_SRC_BLOCK_ID_PC_TRACE            1
#define STREAM_SRC_BLOCK_ID_PC_TRACE_CPU_1      2
#define STREAM_SRC_BLOCK_ID_FIRMWARE_LOG        3
#define STREAM_SRC_BLOCK_ID_FIRMWARE_LOG_CPU_1  4
#define STREAM_SRC_BLOCK_ID_PUT_CHAR            5
#define STREAM_SRC_BLOCK_ID_PUT_CHARS           6
#define STREAM_SRC_BLOCK_ID_PROTOCOL_LOG        7

/**
 * Values to put in the STREAM/SAMPLE transactions for logging. These
 * correspond to the driver's concept of stream ID 254. The stream ID
 * is (Block_ID << 8) | Tag.
 * See //depot/devtools/main/util/trbtrans/private_common.h
 * definition of OFF_CHIP_SUBSYS_STREAM_ID (254).
 * and //depot/hydra/main/drivers/windows/trb/scarlet/driverMain.h
 * definition of TRB_STREAM_ID_SAMPLE (254)
 */
#define HYDRA_TRB_STREAM_ID_SAMPLE          254
#define HYDRA_TRB_DEFAULT_STREAM_BLOCK_ID   (HYDRA_TRB_STREAM_ID_SAMPLE >> 4)
#define HYDRA_TRB_DEFAULT_STREAM_TAG        (HYDRA_TRB_STREAM_ID_SAMPLE & 0xf)

/**
 * Hydra Transaction Bus Message.
 *
 * (As often carried in a Hydra Transaction body)
 */
typedef struct
{
    /**
     * 64 bits of Message data
     *
     * Big endian wrt Message Transaction Payload. Endianess is signifigant
     * when debugging hardware endianess problems - though applications
     * won't care a hoot - so long as it comes out the same order it went in!
     */
    uint16 data[4];
} hydra_trb_msg;

/**
 * Hydra Transaction Bus Transaction
 *
 * Format:-
 * Field                            Width
 * Opcode                           4   Major operation code
 * Sub-system source ID             4   Sub-system initiating the transaction
 * Sub-system block source ID       4   Block within the specified sub-system initiating the transaction
 * Sub-system destination ID        4   Sub-system receiving the transaction
 * Sub-system block destination ID  4   Block within the specified sub-system receiving the transaction
 * Tag                              4   A 4-bit general-purpose tag field. This field remains constant for the duration of a transaction exchange and can be used for transaction tracking.
 * Payload                          72  Opcode-specific payload format
 */
typedef struct
{
    /**
     * 96 bits of Transaction data.
     *
     * BigEndian wrt. the transaction fields (where Opcode is MS field).
     */
    uint16 data[6];
} hydra_trb_trx;

/**
 * Initialise Hydra Transaction Header fields.
 *
 * N.B. The "header" here includes the 1st byte of the payload. This is
 * just pragmatic as we are on a 16bit word machine and the header fields
 * take up 24 bits. In practice this 1st payload byte is typically the
 * header for a transaction sub-type. See hydra_trb_trx_debug_write_init for
 * example.
 *
 * \todo Implement in terms of generated macros once they have setters.
 * [B-120412]
 */
#define hydra_trb_trx_header_init(trx, opcode, src, src_block, dst, dst_block, tag, first_payload_byte) \
    \
    ((void)( \
        (trx)->data[0] = (uint16)((opcode) << 12 | (src) << 8 | (src_block) << 4 | (dst)), \
        (trx)->data[1] = (uint16)((dst_block) << 12 | (tag) << 8 | (first_payload_byte)) \
    ))

/**
 * Initialise Hydra Transaction Bus Debug Write Transaction
 *
 * Opcode   4   Debug type secondary opcode.
 * Length  2   Number of bytes to write. From 1 to 4. (????? 2 bits)
 * Unused  2   Unused bits.
 * Address 32  Remote subsystem address.
 * Data    32  Data bits.
 *
 * \todo Implement in terms of generated macros once they have setters.
 * [B-120412]
 */
#define hydra_trb_debug_write_trx_init(trx, src, dst, num_bytes, addr, data) \
    \
    ((void)( \
        hydra_trb_trx_header_init( \
            trx, \
            (uint16)T_TRANSACTION_MAJOR_OPCODE_T_DEBUG, src, 0, dst, 0, 0, \
            (uint16)((T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_WRITE << 4) \
                                                  | ((num_bytes)-1) << 2) \
        ), \
        (trx)->data[2] = UINT32_MSW(addr), \
        (trx)->data[3] = UINT32_LSW(addr), \
        (trx)->data[4] = UINT32_MSW(data), \
        (trx)->data[5] = UINT32_LSW(data) \
    ))

#endif /* ndef HYDRA_TRB_H */
