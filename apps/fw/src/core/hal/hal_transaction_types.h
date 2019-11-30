/* -------------------------------------------------------------------------
 *
 * COMMERCIAL IN CONFIDENCE
Copyright (c) 2013 - 2016 Qualcomm Technologies International, Ltd.
  
 *
 * Qualcomm Technologies International, Ltd.
 * Churchill House,
 * Cambridge Business Park,
 * Cowley Park,
 * Cambridge, UK
 * http://www.csr.com
 *
 * DESCRIPTION
 *   Transaction bus protocol HAL header file.
 *   Auto-generated from bus_generator_hal.py.
*/

#ifndef TRANSACTION_TYPES_H
#define TRANSACTION_TYPES_H

#if 0
/*
 * Logical types for identifiers.  We don't actually need these
 * since we retrieve bits, so they are here for reference.
 */
typedef uint16 t_subsystem_id;

typedef uint16 t_block_id;

typedef uint16 t_tag;

typedef uint16 t_transaction_message;

#endif /* 0 */

/* Transaction base error codes */
typedef enum t_transaction_base_error_code
{
    T_TRANSACTION_BASE_ERROR_CODE_NO_ERROR = 0,
    T_TRANSACTION_BASE_ERROR_CODE_SUBSYSTEM_POWER_OFF = 1,
    T_TRANSACTION_BASE_ERROR_CODE_SUBSYSTEM_ASLEEP = 2,
    T_TRANSACTION_BASE_ERROR_CODE_ROUTING_ERROR = 3,
    T_TRANSACTION_BASE_ERROR_CODE_LOCK_ERROR = 4
} t_transaction_base_error_code;

/* Transaction data error codes */
typedef enum t_transaction_message_error_code
{
    T_TRANSACTION_MESSAGE_ERROR_CODE_NO_ERROR = 0,
    T_TRANSACTION_MESSAGE_ERROR_CODE_SUBSYSTEM_POWER_OFF = 1,
    T_TRANSACTION_MESSAGE_ERROR_CODE_SUBSYSTEM_ASLEEP = 2,
    T_TRANSACTION_MESSAGE_ERROR_CODE_ROUTING_ERROR = 3,
    T_TRANSACTION_MESSAGE_ERROR_CODE_LOCK_ERROR = 4,
    T_TRANSACTION_MESSAGE_ERROR_CODE_NO_ERROR_NO_INTERRUPT = 14,
    T_TRANSACTION_MESSAGE_ERROR_CODE_RETRY_REQUIRED = 15
} t_transaction_message_error_code;

/* Transaction data error codes */
typedef enum t_transaction_data_error_code
{
    T_TRANSACTION_DATA_ERROR_CODE_NO_ERROR = 0,
    T_TRANSACTION_DATA_ERROR_CODE_SUBSYSTEM_POWER_OFF = 1,
    T_TRANSACTION_DATA_ERROR_CODE_SUBSYSTEM_ASLEEP = 2,
    T_TRANSACTION_DATA_ERROR_CODE_ROUTING_ERROR = 3,
    T_TRANSACTION_DATA_ERROR_CODE_LOCK_ERROR = 4,
    T_TRANSACTION_DATA_ERROR_CODE_ACCESS_PROTECTION = 11,
    T_TRANSACTION_DATA_ERROR_CODE_NO_MEMORY_HERE = 12,
    T_TRANSACTION_DATA_ERROR_CODE_WRONG_LENGTH = 13,
    T_TRANSACTION_DATA_ERROR_CODE_NOT_WRITABLE = 14,
    T_TRANSACTION_DATA_ERROR_CODE_BAD_ALIGNMENT = 15
} t_transaction_data_error_code;

/* Virtual memory transaction error codes */
typedef enum t_transaction_vm_error_code
{
    T_TRANSACTION_VM_ERROR_CODE_NO_ERROR = 0,
    T_TRANSACTION_VM_ERROR_CODE_SUBSYSTEM_POWER_OFF = 1,
    T_TRANSACTION_VM_ERROR_CODE_SUBSYSTEM_ASLEEP = 2,
    T_TRANSACTION_VM_ERROR_CODE_ROUTING_ERROR = 3,
    T_TRANSACTION_VM_ERROR_CODE_LOCK_ERROR = 4,
    T_TRANSACTION_VM_ERROR_CODE_UNMAPPED_BUFFER = 7,
    T_TRANSACTION_VM_ERROR_CODE_MAPPING_PAGES_FOR_READ = 8,
    T_TRANSACTION_VM_ERROR_CODE_OUT_OF_FREE_PAGES = 9,
    T_TRANSACTION_VM_ERROR_CODE_ACCESS_PROTECTION = 10,
    T_TRANSACTION_VM_ERROR_CODE_NOT_SG_BUFFER = 11,
    T_TRANSACTION_VM_ERROR_CODE_SG_LIST_EXHAUSTED = 12,
    T_TRANSACTION_VM_ERROR_CODE_NULL_BUFFER_REQ_ERROR = 13,
    T_TRANSACTION_VM_ERROR_CODE_INVALID_HANDLE = 14,
    T_TRANSACTION_VM_ERROR_CODE_INVALID_PAGE_TABLE_OFFSET = 15
} t_transaction_vm_error_code;

/* Sample transaction sample size/packing */
typedef enum t_transaction_sample_size
{
    T_TRANSACTION_SAMPLE_SIZE_SAMP66 = 0,
    T_TRANSACTION_SAMPLE_SIZE_SAMP33 = 1,
    T_TRANSACTION_SAMPLE_SIZE_SAMP16 = 2,
    T_TRANSACTION_SAMPLE_SIZE_SAMP8 = 3,
    T_TRANSACTION_SAMPLE_SIZE_SAMP6 = 4,
    T_TRANSACTION_SAMPLE_SIZE_SAMP4 = 5,
    T_TRANSACTION_SAMPLE_SIZE_SAMP3 = 6,
    T_TRANSACTION_SAMPLE_SIZE_SAMP2 = 7
} t_transaction_sample_size;

/* The type of count carried in a sample info transaction. */
typedef enum t_transaction_sample_count_type
{
    T_TRANSACTION_SAMPLE_COUNT_TYPE_SAMPLE_COUNT = 0,
    T_TRANSACTION_SAMPLE_COUNT_TYPE_SYSTEM_TIME = 1
} t_transaction_sample_count_type;

/* Coding used in the data samples. */
typedef enum t_transaction_sample_coding
{
    T_TRANSACTION_SAMPLE_CODING_UNSIGNED = 0,
    T_TRANSACTION_SAMPLE_CODING_ONES_COMP = 1,
    T_TRANSACTION_SAMPLE_CODING_TWOS_COMP = 2,
    T_TRANSACTION_SAMPLE_CODING_NONE = 3
} t_transaction_sample_coding;

/* Format used in the data samples. */
typedef enum t_transaction_sample_format
{
    T_TRANSACTION_SAMPLE_FORMAT_COMPONENTS_1 = 0,
    T_TRANSACTION_SAMPLE_FORMAT_COMPONENTS_2 = 1,
    T_TRANSACTION_SAMPLE_FORMAT_COMPONENTS_3 = 2
} t_transaction_sample_format;

/* Major opcodes */
typedef enum t_transaction_major_opcode
{
    T_TRANSACTION_MAJOR_OPCODE_T_IDLE = 0,
    T_TRANSACTION_MAJOR_OPCODE_T_VM_LOOKUP_REQ = 1,
    T_TRANSACTION_MAJOR_OPCODE_T_VM_LOOKUP_RESP = 2,
    T_TRANSACTION_MAJOR_OPCODE_T_VM_WRITEBACK = 3,
    T_TRANSACTION_MAJOR_OPCODE_T_DATA_WRITE = 4,
    T_TRANSACTION_MAJOR_OPCODE_T_DATA_WRITE_REQ = 5,
    T_TRANSACTION_MAJOR_OPCODE_T_DATA_WRITE_RESP = 6,
    T_TRANSACTION_MAJOR_OPCODE_T_DATA_READ_REQ = 7,
    T_TRANSACTION_MAJOR_OPCODE_T_DATA_READ_RESP = 8,
    T_TRANSACTION_MAJOR_OPCODE_T_EXTENDED = 9,
    T_TRANSACTION_MAJOR_OPCODE_T_SAMPLE = 10,
    T_TRANSACTION_MAJOR_OPCODE_T_DEBUG = 11
} t_transaction_major_opcode;

/* Extended minor opcodes */
typedef enum t_transaction_minor_opcode
{
    T_TRANSACTION_MINOR_OPCODE_T_INTERRUPT_EVENT = 0,
    T_TRANSACTION_MINOR_OPCODE_T_TIME_SYNC = 1,
    T_TRANSACTION_MINOR_OPCODE_T_TIME_SYNC_RESP = 2,
    T_TRANSACTION_MINOR_OPCODE_T_MESSAGE = 3,
    T_TRANSACTION_MINOR_OPCODE_T_MESSAGE_RESP = 4,
    T_TRANSACTION_MINOR_OPCODE_T_SYS_RESET = 5,
    T_TRANSACTION_MINOR_OPCODE_T_HIT_BREAKPOINT = 6,
    T_TRANSACTION_MINOR_OPCODE_T_COEXISTENCE = 7,
    T_TRANSACTION_MINOR_OPCODE_T_RESERVATION = 8,
    T_TRANSACTION_MINOR_OPCODE_T_SAMPLE_INFO = 9,
    T_TRANSACTION_MINOR_OPCODE_T_PING = 10
} t_transaction_minor_opcode;

/* VM lookup minor opcodes */
typedef enum t_transaction_vm_lookup_req_opcode
{
    T_TRANSACTION_VM_LOOKUP_REQ_OPCODE_T_REQ_NO_OFFSET = 0,
    T_TRANSACTION_VM_LOOKUP_REQ_OPCODE_T_REQ_OFFSET = 1
} t_transaction_vm_lookup_req_opcode;

/* VM lookup minor opcodes */
typedef enum t_transaction_vm_lookup_resp_opcode
{
    T_TRANSACTION_VM_LOOKUP_RESP_OPCODE_T_RESP_NO_OFFSET = 0,
    T_TRANSACTION_VM_LOOKUP_RESP_OPCODE_T_RESP_OFFSET = 1
} t_transaction_vm_lookup_resp_opcode;

/* Debug minor opcodes */
typedef enum t_transaction_debug_minor_opcode
{
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_WRITE = 0,
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_WRITE_REQ = 1,
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_WRITE_RESP = 2,
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_READ_REQ = 3,
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DEBUG_READ_RESP = 4,
    T_TRANSACTION_DEBUG_MINOR_OPCODE_T_DROP_NOTIFICATION = 5
} t_transaction_debug_minor_opcode;

/* Coex opcodes */
typedef enum t_transaction_coex_opcode
{
    T_TRANSACTION_COEX_OPCODE_T_COEX_SET_AS = 0,
    T_TRANSACTION_COEX_OPCODE_T_COEX_FRONT_END_CFG = 1
} t_transaction_coex_opcode;

/* Root transaction type */
/* Opcode */
#define T_TRANSACTION__OPCODE__OFFSET (0)
#define T_TRANSACTION__OPCODE__WIDTH (4)
#define hal_get_t_transaction_opcode(reg) \
        (((reg ## 5) >> 12U) & 0x000fU)

/* SysSourceId */
#define T_TRANSACTION__SYSSOURCEID__OFFSET (4)
#define T_TRANSACTION__SYSSOURCEID__WIDTH (4)
#define hal_get_t_transaction_syssourceid(reg) \
        (((reg ## 5) >> 8U) & 0x000fU)

/* BlockSourceId */
#define T_TRANSACTION__BLOCKSOURCEID__OFFSET (8)
#define T_TRANSACTION__BLOCKSOURCEID__WIDTH (4)
#define hal_get_t_transaction_blocksourceid(reg) \
        (((reg ## 5) >> 4U) & 0x000fU)

/* SysDestId */
#define T_TRANSACTION__SYSDESTID__OFFSET (12)
#define T_TRANSACTION__SYSDESTID__WIDTH (4)
#define hal_get_t_transaction_sysdestid(reg) \
        ((reg ## 5) & 0x000fU)

/* BlockDestId */
#define T_TRANSACTION__BLOCKDESTID__OFFSET (16)
#define T_TRANSACTION__BLOCKDESTID__WIDTH (4)
#define hal_get_t_transaction_blockdestid(reg) \
        (((reg ## 4) >> 12U) & 0x000fU)

/* Tag */
#define T_TRANSACTION__TAG__OFFSET (20)
#define T_TRANSACTION__TAG__WIDTH (4)
#define hal_get_t_transaction_tag(reg) \
        (((reg ## 4) >> 8U) & 0x000fU)

/* Payload */
#define T_TRANSACTION__PAYLOAD__OFFSET (24)
#define T_TRANSACTION__PAYLOAD__WIDTH (72)
#define hal_get_t_transaction_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_4(reg) \
        ((reg ## 4) & 0x00ffU)


/* Idle payload type */
/* Payload */
#define T_TRANSACTION_PAYLOAD_IDLE__PAYLOAD__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_IDLE__PAYLOAD__WIDTH (72)
#define hal_get_t_transaction_payload_idle_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_idle_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_idle_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_idle_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_idle_payload_4(reg) \
        ((reg ## 4) & 0x00ffU)


/* VM lookup request payload type */
/* Opcode */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ__OPCODE__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ__OPCODE__WIDTH (1)
#define hal_get_t_transaction_payload_vm_lookup_req_opcode(reg) \
        (((reg ## 4) >> 7U) & 0x0001U)

/* Payload */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ__PAYLOAD__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ__PAYLOAD__WIDTH (71)
#define hal_get_t_transaction_payload_vm_lookup_req_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_vm_lookup_req_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_vm_lookup_req_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_vm_lookup_req_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_vm_lookup_req_payload_4(reg) \
        ((reg ## 4) & 0x007fU)


/* VM lookup no offset request payload type */
/* Buffer */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__BUFFER__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__BUFFER__WIDTH (8)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_buffer(reg) \
        (((reg ## 3) >> 15) | (((reg ## 4) & 0x007fU) << 1U))

/* Rwb */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__RWB__OFFSET (33)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__RWB__WIDTH (1)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_rwb(reg) \
        (((reg ## 3) >> 14U) & 0x0001U)

/* OffsetOnly */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__OFFSETONLY__OFFSET (34)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__OFFSETONLY__WIDTH (1)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_offsetonly(reg) \
        (((reg ## 3) >> 13U) & 0x0001U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__UNUSED__OFFSET (35)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_NO_OFFSET__UNUSED__WIDTH (61)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_vm_lookup_req_no_offset_unused_3(reg) \
        ((reg ## 3) & 0x1fffU)


/* VM lookup offset request payload type */
/* Buffer */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__BUFFER__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__BUFFER__WIDTH (8)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_buffer(reg) \
        (((reg ## 3) >> 15) | (((reg ## 4) & 0x007fU) << 1U))

/* Rwb */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__RWB__OFFSET (33)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__RWB__WIDTH (1)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_rwb(reg) \
        (((reg ## 3) >> 14U) & 0x0001U)

/* Offset */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__OFFSET__OFFSET (34)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__OFFSET__WIDTH (18)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_offset_0(reg) \
        (((reg ## 2) >> 12) | (((reg ## 3) & 0x0fffU) << 4U))
#define hal_get_t_transaction_payload_vm_lookup_req_offset_offset_1(reg) \
        (((reg ## 3) & 0x3fffU) >> 12)

/* Unused */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__UNUSED__OFFSET (52)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_REQ_OFFSET__UNUSED__WIDTH (44)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_vm_lookup_req_offset_unused_2(reg) \
        ((reg ## 2) & 0x0fffU)


/* VM lookup response payload type */
/* Opcode */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP__OPCODE__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP__OPCODE__WIDTH (1)
#define hal_get_t_transaction_payload_vm_lookup_resp_opcode(reg) \
        (((reg ## 4) >> 7U) & 0x0001U)

/* Payload */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP__PAYLOAD__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP__PAYLOAD__WIDTH (71)
#define hal_get_t_transaction_payload_vm_lookup_resp_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_vm_lookup_resp_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_vm_lookup_resp_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_vm_lookup_resp_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_vm_lookup_resp_payload_4(reg) \
        ((reg ## 4) & 0x007fU)


/* VM lookup no offset response payload type */
/* Unused */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__UNUSED__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__UNUSED__WIDTH (3)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_unused(reg) \
        (((reg ## 4) >> 4U) & 0x0007U)

/* Status */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Address */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__ADDRESS__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_address_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_address_1(reg) \
        (reg ## 3)

/* ValidityWindow */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__VALIDITYWINDOW__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__VALIDITYWINDOW__WIDTH (10)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_validitywindow(reg) \
        (((reg ## 1) >> 6U) & 0x03ffU)

/* Offset */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__OFFSET__OFFSET (74)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__OFFSET__WIDTH (18)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_offset_0(reg) \
        (((reg ## 0) >> 4) | (((reg ## 1) & 0x000fU) << 12U))
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_offset_1(reg) \
        (((reg ## 1) & 0x003fU) >> 4)

/* Unused2 */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__UNUSED2__OFFSET (92)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_NO_OFFSET__UNUSED2__WIDTH (4)
#define hal_get_t_transaction_payload_vm_lookup_resp_no_offset_unused2(reg) \
        ((reg ## 0) & 0x000fU)


/* VM lookup offset response payload type */
/* Unused */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__UNUSED__OFFSET (25)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__UNUSED__WIDTH (3)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_unused(reg) \
        (((reg ## 4) >> 4U) & 0x0007U)

/* Status */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Address */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__ADDRESS__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_address_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_address_1(reg) \
        (reg ## 3)

/* ValidityWindow */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__VALIDITYWINDOW__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__VALIDITYWINDOW__WIDTH (10)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_validitywindow(reg) \
        (((reg ## 1) >> 6U) & 0x03ffU)

/* ValidityWindowOffset */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__VALIDITYWINDOWOFFSET__OFFSET (74)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__VALIDITYWINDOWOFFSET__WIDTH (10)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_validitywindowoffset(reg) \
        (((reg ## 0) >> 12) | (((reg ## 1) & 0x003fU) << 4U))

/* Unused2 */
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__UNUSED2__OFFSET (84)
#define T_TRANSACTION_PAYLOAD_VM_LOOKUP_RESP_OFFSET__UNUSED2__WIDTH (12)
#define hal_get_t_transaction_payload_vm_lookup_resp_offset_unused2(reg) \
        ((reg ## 0) & 0x0fffU)


/* VM writeback payload type */
/* Buffer */
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__BUFFER__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__BUFFER__WIDTH (8)
#define hal_get_t_transaction_payload_vm_writeback_buffer(reg) \
        ((reg ## 4) & 0x00ffU)

/* Offset */
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__OFFSET__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__OFFSET__WIDTH (18)
#define hal_get_t_transaction_payload_vm_writeback_offset_0(reg) \
        (((reg ## 2) >> 14) | (((reg ## 3) & 0x3fffU) << 2U))
#define hal_get_t_transaction_payload_vm_writeback_offset_1(reg) \
        ((reg ## 3) >> 14U)

/* MarkInvalid */
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__MARKINVALID__OFFSET (50)
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__MARKINVALID__WIDTH (1)
#define hal_get_t_transaction_payload_vm_writeback_markinvalid(reg) \
        (((reg ## 2) >> 13U) & 0x0001U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__UNUSED__OFFSET (51)
#define T_TRANSACTION_PAYLOAD_VM_WRITEBACK__UNUSED__WIDTH (45)
#define hal_get_t_transaction_payload_vm_writeback_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_vm_writeback_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_vm_writeback_unused_2(reg) \
        ((reg ## 2) & 0x1fffU)


/* Data write payload type */
/* Address */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__ADDRESS__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_data_write_address_0(reg) \
        (((reg ## 2) >> 8) | (((reg ## 3) & 0x00ffU) << 8U))
#define hal_get_t_transaction_payload_data_write_address_1(reg) \
        (((reg ## 3) >> 8) | (((reg ## 4) & 0x00ffU) << 8U))
/* Length */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__LENGTH__OFFSET (56)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__LENGTH__WIDTH (3)
#define hal_get_t_transaction_payload_data_write_length(reg) \
        (((reg ## 2) >> 5U) & 0x0007U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__UNUSED__OFFSET (59)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__UNUSED__WIDTH (5)
#define hal_get_t_transaction_payload_data_write_unused(reg) \
        ((reg ## 2) & 0x001fU)

/* Data */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__DATA__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE__DATA__WIDTH (32)
#define hal_get_t_transaction_payload_data_write_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_data_write_data_1(reg) \
        (reg ## 1)


/* Data write request payload type */
/* Address */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__ADDRESS__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_data_write_req_address_0(reg) \
        (((reg ## 2) >> 8) | (((reg ## 3) & 0x00ffU) << 8U))
#define hal_get_t_transaction_payload_data_write_req_address_1(reg) \
        (((reg ## 3) >> 8) | (((reg ## 4) & 0x00ffU) << 8U))
/* Length */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__LENGTH__OFFSET (56)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__LENGTH__WIDTH (3)
#define hal_get_t_transaction_payload_data_write_req_length(reg) \
        (((reg ## 2) >> 5U) & 0x0007U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__UNUSED__OFFSET (59)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__UNUSED__WIDTH (5)
#define hal_get_t_transaction_payload_data_write_req_unused(reg) \
        ((reg ## 2) & 0x001fU)

/* Data */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__DATA__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_REQ__DATA__WIDTH (32)
#define hal_get_t_transaction_payload_data_write_req_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_data_write_req_data_1(reg) \
        (reg ## 1)


/* Data write response payload type */
/* Status */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_RESP__STATUS__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_data_write_resp_status(reg) \
        (((reg ## 4) >> 4U) & 0x000fU)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_RESP__UNUSED__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DATA_WRITE_RESP__UNUSED__WIDTH (68)
#define hal_get_t_transaction_payload_data_write_resp_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_data_write_resp_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_data_write_resp_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_data_write_resp_unused_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_data_write_resp_unused_4(reg) \
        ((reg ## 4) & 0x000fU)


/* Data read request payload type */
/* Address */
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__ADDRESS__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_data_read_req_address_0(reg) \
        (((reg ## 2) >> 8) | (((reg ## 3) & 0x00ffU) << 8U))
#define hal_get_t_transaction_payload_data_read_req_address_1(reg) \
        (((reg ## 3) >> 8) | (((reg ## 4) & 0x00ffU) << 8U))
/* Length */
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__LENGTH__OFFSET (56)
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__LENGTH__WIDTH (3)
#define hal_get_t_transaction_payload_data_read_req_length(reg) \
        (((reg ## 2) >> 5U) & 0x0007U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__UNUSED__OFFSET (59)
#define T_TRANSACTION_PAYLOAD_DATA_READ_REQ__UNUSED__WIDTH (37)
#define hal_get_t_transaction_payload_data_read_req_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_data_read_req_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_data_read_req_unused_2(reg) \
        ((reg ## 2) & 0x001fU)


/* Data read response payload type */
/* Status */
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__STATUS__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_data_read_resp_status(reg) \
        (((reg ## 4) >> 4U) & 0x000fU)

/* Length */
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__LENGTH__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__LENGTH__WIDTH (3)
#define hal_get_t_transaction_payload_data_read_resp_length(reg) \
        (((reg ## 4) >> 1U) & 0x0007U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__UNUSED__OFFSET (31)
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__UNUSED__WIDTH (1)
#define hal_get_t_transaction_payload_data_read_resp_unused(reg) \
        ((reg ## 4) & 0x0001U)

/* Data */
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__DATA__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DATA_READ_RESP__DATA__WIDTH (64)
#define hal_get_t_transaction_payload_data_read_resp_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_data_read_resp_data_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_data_read_resp_data_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_data_read_resp_data_3(reg) \
        (reg ## 3)


/* Sample payload type */
/* NumberOfSamples */
#define T_TRANSACTION_PAYLOAD_SAMPLE__NUMBEROFSAMPLES__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_SAMPLE__NUMBEROFSAMPLES__WIDTH (5)
#define hal_get_t_transaction_payload_sample_numberofsamples(reg) \
        (((reg ## 4) >> 3U) & 0x001fU)

/* GoldSample */
#define T_TRANSACTION_PAYLOAD_SAMPLE__GOLDSAMPLE__OFFSET (29)
#define T_TRANSACTION_PAYLOAD_SAMPLE__GOLDSAMPLE__WIDTH (1)
#define hal_get_t_transaction_payload_sample_goldsample(reg) \
        (((reg ## 4) >> 2U) & 0x0001U)

/* Samples */
#define T_TRANSACTION_PAYLOAD_SAMPLE__SAMPLES__OFFSET (30)
#define T_TRANSACTION_PAYLOAD_SAMPLE__SAMPLES__WIDTH (66)
#define hal_get_t_transaction_payload_sample_samples_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_sample_samples_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_sample_samples_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_sample_samples_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_sample_samples_4(reg) \
        ((reg ## 4) & 0x0003U)


/* Extended payload type */
/* Opcode */
#define T_TRANSACTION_PAYLOAD_EXT__OPCODE__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_EXT__OPCODE__WIDTH (4)
#define hal_get_t_transaction_payload_ext_opcode(reg) \
        (((reg ## 4) >> 4U) & 0x000fU)

/* Payload */
#define T_TRANSACTION_PAYLOAD_EXT__PAYLOAD__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT__PAYLOAD__WIDTH (68)
#define hal_get_t_transaction_payload_ext_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_ext_payload_4(reg) \
        ((reg ## 4) & 0x000fU)


/* Interrupt event extended payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_EXT_INTERRUPT_EVENT__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_INTERRUPT_EVENT__STATUS__WIDTH (16)
#define hal_get_t_transaction_payload_ext_interrupt_event_status(reg) \
        (((reg ## 3) >> 4) | (((reg ## 4) & 0x000fU) << 12U))

/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_INTERRUPT_EVENT__UNUSED__OFFSET (44)
#define T_TRANSACTION_PAYLOAD_EXT_INTERRUPT_EVENT__UNUSED__WIDTH (52)
#define hal_get_t_transaction_payload_ext_interrupt_event_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_interrupt_event_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_interrupt_event_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_interrupt_event_unused_3(reg) \
        ((reg ## 3) & 0x000fU)


/* Time synchronisation extended payload */
/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__UNUSED__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__UNUSED__WIDTH (4)
#define hal_get_t_transaction_payload_ext_time_sync_unused(reg) \
        ((reg ## 4) & 0x000fU)

/* Time */
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__TIME__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__TIME__WIDTH (32)
#define hal_get_t_transaction_payload_ext_time_sync_time_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_time_sync_time_1(reg) \
        (reg ## 3)

/* Unused2 */
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__UNUSED2__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC__UNUSED2__WIDTH (32)
#define hal_get_t_transaction_payload_ext_time_sync_unused2_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_time_sync_unused2_1(reg) \
        (reg ## 1)


/* Time synchronisation response payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC_RESP__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_ext_time_sync_resp_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC_RESP__UNUSED__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_TIME_SYNC_RESP__UNUSED__WIDTH (64)
#define hal_get_t_transaction_payload_ext_time_sync_resp_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_time_sync_resp_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_time_sync_resp_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_time_sync_resp_unused_3(reg) \
        (reg ## 3)


/* Message request payload */
/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__UNUSED__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__UNUSED__WIDTH (3)
#define hal_get_t_transaction_payload_ext_message_req_unused(reg) \
        (((reg ## 4) >> 1U) & 0x0007U)

/* NoInterrupt */
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__NOINTERRUPT__OFFSET (31)
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__NOINTERRUPT__WIDTH (1)
#define hal_get_t_transaction_payload_ext_message_req_nointerrupt(reg) \
        ((reg ## 4) & 0x0001U)

/* Message */
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__MESSAGE__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_REQ__MESSAGE__WIDTH (64)
#define hal_get_t_transaction_payload_ext_message_req_message_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_message_req_message_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_message_req_message_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_message_req_message_3(reg) \
        (reg ## 3)


/* Message response payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_RESP__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_ext_message_resp_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Message */
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_RESP__MESSAGE__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_MESSAGE_RESP__MESSAGE__WIDTH (64)
#define hal_get_t_transaction_payload_ext_message_resp_message_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_message_resp_message_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_message_resp_message_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_message_resp_message_3(reg) \
        (reg ## 3)


/* Subsystem reset payload */
/* Cause */
#define T_TRANSACTION_PAYLOAD_EXT_SYS_RESET__CAUSE__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_SYS_RESET__CAUSE__WIDTH (4)
#define hal_get_t_transaction_payload_ext_sys_reset_cause(reg) \
        ((reg ## 4) & 0x000fU)

/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_SYS_RESET__UNUSED__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_SYS_RESET__UNUSED__WIDTH (64)
#define hal_get_t_transaction_payload_ext_sys_reset_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_sys_reset_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_sys_reset_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_sys_reset_unused_3(reg) \
        (reg ## 3)


/* Hit breakpoint payload */
/* Address */
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__ADDRESS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__ADDRESS__WIDTH (24)
#define hal_get_t_transaction_payload_ext_hit_breakpoint_address_0(reg) \
        (((reg ## 2) >> 12) | (((reg ## 3) & 0x0fffU) << 4U))
#define hal_get_t_transaction_payload_ext_hit_breakpoint_address_1(reg) \
        (((reg ## 3) >> 12) | (((reg ## 4) & 0x000fU) << 4U))
/* ProcNumber */
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__PROCNUMBER__OFFSET (52)
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__PROCNUMBER__WIDTH (2)
#define hal_get_t_transaction_payload_ext_hit_breakpoint_procnumber(reg) \
        (((reg ## 2) >> 10U) & 0x0003U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__UNUSED__OFFSET (54)
#define T_TRANSACTION_PAYLOAD_EXT_HIT_BREAKPOINT__UNUSED__WIDTH (42)
#define hal_get_t_transaction_payload_ext_hit_breakpoint_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_hit_breakpoint_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_hit_breakpoint_unused_2(reg) \
        ((reg ## 2) & 0x03ffU)


/* Coexistence payload */
/* Opcode */
#define T_TRANSACTION_PAYLOAD_EXT_COEXISTENCE__OPCODE__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_COEXISTENCE__OPCODE__WIDTH (4)
#define hal_get_t_transaction_payload_ext_coexistence_opcode(reg) \
        ((reg ## 4) & 0x000fU)

/* Payload */
#define T_TRANSACTION_PAYLOAD_EXT_COEXISTENCE__PAYLOAD__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_EXT_COEXISTENCE__PAYLOAD__WIDTH (64)
#define hal_get_t_transaction_payload_ext_coexistence_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_coexistence_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_coexistence_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_coexistence_payload_3(reg) \
        (reg ## 3)


/* Coex activity set payload */
/* Index */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__INDEX__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__INDEX__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_index(reg) \
        (((reg ## 3) >> 15U) & 0x0001U)

/* NextAsIndex */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__NEXTASINDEX__OFFSET (33)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__NEXTASINDEX__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_nextasindex(reg) \
        (((reg ## 3) >> 14U) & 0x0001U)

/* LocalAna */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__LOCALANA__OFFSET (34)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__LOCALANA__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_localana(reg) \
        (((reg ## 3) >> 13U) & 0x0001U)

/* RemoteAna */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__REMOTEANA__OFFSET (35)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__REMOTEANA__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_remoteana(reg) \
        (((reg ## 3) >> 12U) & 0x0001U)

/* Priority */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__PRIORITY__OFFSET (36)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__PRIORITY__WIDTH (4)
#define hal_get_t_transaction_payload_coex_set_as_priority(reg) \
        (((reg ## 3) >> 8U) & 0x000fU)

/* IsValid */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__ISVALID__OFFSET (40)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__ISVALID__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_isvalid(reg) \
        (((reg ## 3) >> 7U) & 0x0001U)

/* IsTx */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__ISTX__OFFSET (41)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__ISTX__WIDTH (1)
#define hal_get_t_transaction_payload_coex_set_as_istx(reg) \
        (((reg ## 3) >> 6U) & 0x0001U)

/* ChannelLow */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__CHANNELLOW__OFFSET (42)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__CHANNELLOW__WIDTH (7)
#define hal_get_t_transaction_payload_coex_set_as_channellow(reg) \
        (((reg ## 2) >> 15) | (((reg ## 3) & 0x003fU) << 1U))

/* ChannelHigh */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__CHANNELHIGH__OFFSET (49)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__CHANNELHIGH__WIDTH (7)
#define hal_get_t_transaction_payload_coex_set_as_channelhigh(reg) \
        (((reg ## 2) >> 8U) & 0x007fU)

/* StartTime */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__STARTTIME__OFFSET (56)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__STARTTIME__WIDTH (22)
#define hal_get_t_transaction_payload_coex_set_as_starttime_0(reg) \
        (((reg ## 1) >> 2) | (((reg ## 2) & 0x0003U) << 14U))
#define hal_get_t_transaction_payload_coex_set_as_starttime_1(reg) \
        (((reg ## 2) & 0x00ffU) >> 2)

/* Duration */
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__DURATION__OFFSET (78)
#define T_TRANSACTION_PAYLOAD_COEX_SET_AS__DURATION__WIDTH (18)
#define hal_get_t_transaction_payload_coex_set_as_duration_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_coex_set_as_duration_1(reg) \
        ((reg ## 1) & 0x0003U)


/* Coex front end configuration */
/* IsTx */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__ISTX__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__ISTX__WIDTH (1)
#define hal_get_t_transaction_payload_coex_fe_cfg_istx(reg) \
        (((reg ## 3) >> 15U) & 0x0001U)

/* IsValid */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__ISVALID__OFFSET (33)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__ISVALID__WIDTH (1)
#define hal_get_t_transaction_payload_coex_fe_cfg_isvalid(reg) \
        (((reg ## 3) >> 14U) & 0x0001U)

/* FemLnaGain */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__FEMLNAGAIN__OFFSET (34)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__FEMLNAGAIN__WIDTH (1)
#define hal_get_t_transaction_payload_coex_fe_cfg_femlnagain(reg) \
        (((reg ## 3) >> 13U) & 0x0001U)

/* FemAntSel */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__FEMANTSEL__OFFSET (35)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__FEMANTSEL__WIDTH (1)
#define hal_get_t_transaction_payload_coex_fe_cfg_femantsel(reg) \
        (((reg ## 3) >> 12U) & 0x0001U)

/* Remote5G */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__REMOTE5G__OFFSET (36)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__REMOTE5G__WIDTH (1)
#define hal_get_t_transaction_payload_coex_fe_cfg_remote5g(reg) \
        (((reg ## 3) >> 11U) & 0x0001U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__UNUSED__OFFSET (37)
#define T_TRANSACTION_PAYLOAD_COEX_FE_CFG__UNUSED__WIDTH (59)
#define hal_get_t_transaction_payload_coex_fe_cfg_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_coex_fe_cfg_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_coex_fe_cfg_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_coex_fe_cfg_unused_3(reg) \
        ((reg ## 3) & 0x07ffU)


/* Reservation payload */
/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_RESERVATION__UNUSED__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_RESERVATION__UNUSED__WIDTH (68)
#define hal_get_t_transaction_payload_ext_reservation_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_reservation_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_reservation_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_reservation_unused_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_ext_reservation_unused_4(reg) \
        ((reg ## 4) & 0x000fU)


/* Sample info payload */
/* Size */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__SIZE__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__SIZE__WIDTH (8)
#define hal_get_t_transaction_payload_ext_sample_info_size(reg) \
        (((reg ## 3) >> 12) | (((reg ## 4) & 0x000fU) << 4U))

/* Start */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__START__OFFSET (36)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__START__WIDTH (1)
#define hal_get_t_transaction_payload_ext_sample_info_start(reg) \
        (((reg ## 3) >> 11U) & 0x0001U)

/* Stop */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__STOP__OFFSET (37)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__STOP__WIDTH (1)
#define hal_get_t_transaction_payload_ext_sample_info_stop(reg) \
        (((reg ## 3) >> 10U) & 0x0001U)

/* Trigger */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__TRIGGER__OFFSET (38)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__TRIGGER__WIDTH (4)
#define hal_get_t_transaction_payload_ext_sample_info_trigger(reg) \
        (((reg ## 3) >> 6U) & 0x000fU)

/* CountType */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTTYPE__OFFSET (42)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTTYPE__WIDTH (2)
#define hal_get_t_transaction_payload_ext_sample_info_counttype(reg) \
        (((reg ## 3) >> 4U) & 0x0003U)

/* CountValid */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTVALID__OFFSET (44)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTVALID__WIDTH (1)
#define hal_get_t_transaction_payload_ext_sample_info_countvalid(reg) \
        (((reg ## 3) >> 3U) & 0x0001U)

/* CountValue */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTVALUE__OFFSET (45)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__COUNTVALUE__WIDTH (40)
#define hal_get_t_transaction_payload_ext_sample_info_countvalue_0(reg) \
        (((reg ## 0) >> 11) | (((reg ## 1) & 0x07ffU) << 5U))
#define hal_get_t_transaction_payload_ext_sample_info_countvalue_1(reg) \
        (((reg ## 1) >> 11) | (((reg ## 2) & 0x07ffU) << 5U))
#define hal_get_t_transaction_payload_ext_sample_info_countvalue_2(reg) \
        (((reg ## 2) >> 11) | (((reg ## 3) & 0x0007U) << 5U))
/* Coding */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__CODING__OFFSET (85)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__CODING__WIDTH (2)
#define hal_get_t_transaction_payload_ext_sample_info_coding(reg) \
        (((reg ## 0) >> 9U) & 0x0003U)

/* Format */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__FORMAT__OFFSET (87)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__FORMAT__WIDTH (3)
#define hal_get_t_transaction_payload_ext_sample_info_format(reg) \
        (((reg ## 0) >> 6U) & 0x0007U)

/* User */
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__USER__OFFSET (90)
#define T_TRANSACTION_PAYLOAD_EXT_SAMPLE_INFO__USER__WIDTH (6)
#define hal_get_t_transaction_payload_ext_sample_info_user(reg) \
        ((reg ## 0) & 0x003fU)


/* Ping payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_EXT_PING__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_EXT_PING__STATUS__WIDTH (16)
#define hal_get_t_transaction_payload_ext_ping_status(reg) \
        (((reg ## 3) >> 4) | (((reg ## 4) & 0x000fU) << 12U))

/* Unused */
#define T_TRANSACTION_PAYLOAD_EXT_PING__UNUSED__OFFSET (44)
#define T_TRANSACTION_PAYLOAD_EXT_PING__UNUSED__WIDTH (52)
#define hal_get_t_transaction_payload_ext_ping_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_ext_ping_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_ext_ping_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_ext_ping_unused_3(reg) \
        ((reg ## 3) & 0x000fU)


/* Debug payload type */
/* Opcode */
#define T_TRANSACTION_PAYLOAD_DEBUG__OPCODE__OFFSET (24)
#define T_TRANSACTION_PAYLOAD_DEBUG__OPCODE__WIDTH (4)
#define hal_get_t_transaction_payload_debug_opcode(reg) \
        (((reg ## 4) >> 4U) & 0x000fU)

/* Payload */
#define T_TRANSACTION_PAYLOAD_DEBUG__PAYLOAD__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG__PAYLOAD__WIDTH (68)
#define hal_get_t_transaction_payload_debug_payload_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_payload_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_debug_payload_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_payload_3(reg) \
        (reg ## 3)
#define hal_get_t_transaction_payload_debug_payload_4(reg) \
        ((reg ## 4) & 0x000fU)


/* Debug write payload */
/* Length */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__LENGTH__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__LENGTH__WIDTH (2)
#define hal_get_t_transaction_payload_debug_write_length(reg) \
        (((reg ## 4) >> 2U) & 0x0003U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__UNUSED__OFFSET (30)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__UNUSED__WIDTH (2)
#define hal_get_t_transaction_payload_debug_write_unused(reg) \
        ((reg ## 4) & 0x0003U)

/* Address */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__ADDRESS__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_debug_write_address_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_write_address_1(reg) \
        (reg ## 3)

/* Data */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__DATA__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE__DATA__WIDTH (32)
#define hal_get_t_transaction_payload_debug_write_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_write_data_1(reg) \
        (reg ## 1)


/* Debug write request payload */
/* Length */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__LENGTH__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__LENGTH__WIDTH (2)
#define hal_get_t_transaction_payload_debug_write_req_length(reg) \
        (((reg ## 4) >> 2U) & 0x0003U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__UNUSED__OFFSET (30)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__UNUSED__WIDTH (2)
#define hal_get_t_transaction_payload_debug_write_req_unused(reg) \
        ((reg ## 4) & 0x0003U)

/* Address */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__ADDRESS__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_debug_write_req_address_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_write_req_address_1(reg) \
        (reg ## 3)

/* Data */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__DATA__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_REQ__DATA__WIDTH (32)
#define hal_get_t_transaction_payload_debug_write_req_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_write_req_data_1(reg) \
        (reg ## 1)


/* Debug write response payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_RESP__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_debug_write_resp_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_RESP__UNUSED__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_WRITE_RESP__UNUSED__WIDTH (64)
#define hal_get_t_transaction_payload_debug_write_resp_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_write_resp_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_debug_write_resp_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_write_resp_unused_3(reg) \
        (reg ## 3)


/* Debug read request payload */
/* Length */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__LENGTH__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__LENGTH__WIDTH (2)
#define hal_get_t_transaction_payload_debug_read_req_length(reg) \
        (((reg ## 4) >> 2U) & 0x0003U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__UNUSED__OFFSET (30)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__UNUSED__WIDTH (2)
#define hal_get_t_transaction_payload_debug_read_req_unused(reg) \
        ((reg ## 4) & 0x0003U)

/* Address */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__ADDRESS__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__ADDRESS__WIDTH (32)
#define hal_get_t_transaction_payload_debug_read_req_address_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_read_req_address_1(reg) \
        (reg ## 3)

/* Unused2 */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__UNUSED2__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_REQ__UNUSED2__WIDTH (32)
#define hal_get_t_transaction_payload_debug_read_req_unused2_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_read_req_unused2_1(reg) \
        (reg ## 1)


/* Debug read response payload */
/* Status */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__STATUS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__STATUS__WIDTH (4)
#define hal_get_t_transaction_payload_debug_read_resp_status(reg) \
        ((reg ## 4) & 0x000fU)

/* Length */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__LENGTH__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__LENGTH__WIDTH (2)
#define hal_get_t_transaction_payload_debug_read_resp_length(reg) \
        (((reg ## 3) >> 14U) & 0x0003U)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__UNUSED__OFFSET (34)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__UNUSED__WIDTH (30)
#define hal_get_t_transaction_payload_debug_read_resp_unused_0(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_read_resp_unused_1(reg) \
        ((reg ## 3) & 0x3fffU)

/* Data */
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__DATA__OFFSET (64)
#define T_TRANSACTION_PAYLOAD_DEBUG_READ_RESP__DATA__WIDTH (32)
#define hal_get_t_transaction_payload_debug_read_resp_data_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_read_resp_data_1(reg) \
        (reg ## 1)


/* Drop notification payload */
/* NumDroppedLSBs */
#define T_TRANSACTION_PAYLOAD_DEBUG_DROP_NOTIFICATION__NUMDROPPEDLSBS__OFFSET (28)
#define T_TRANSACTION_PAYLOAD_DEBUG_DROP_NOTIFICATION__NUMDROPPEDLSBS__WIDTH (4)
#define hal_get_t_transaction_payload_debug_drop_notification_numdroppedlsbs(reg) \
        ((reg ## 4) & 0x000fU)

/* Unused */
#define T_TRANSACTION_PAYLOAD_DEBUG_DROP_NOTIFICATION__UNUSED__OFFSET (32)
#define T_TRANSACTION_PAYLOAD_DEBUG_DROP_NOTIFICATION__UNUSED__WIDTH (64)
#define hal_get_t_transaction_payload_debug_drop_notification_unused_0(reg) \
        (reg ## 0)
#define hal_get_t_transaction_payload_debug_drop_notification_unused_1(reg) \
        (reg ## 1)
#define hal_get_t_transaction_payload_debug_drop_notification_unused_2(reg) \
        (reg ## 2)
#define hal_get_t_transaction_payload_debug_drop_notification_unused_3(reg) \
        (reg ## 3)


#endif /* TRANSACTION_TYPES_H */
