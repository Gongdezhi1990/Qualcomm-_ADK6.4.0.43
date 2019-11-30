/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/


#ifndef PLC100_C_H
#define PLC100_C_H
/*****************************************************************************
Include Files
*/
#include "mem_utils/shared_memory_ids.h"
#include "buffer/cbuffer_c.h"
#include "platform/pl_fractional.h"

/*****************************************************************************
Public Type Definitions
*/
typedef struct PLC100_CONSTANTS PLC100_CONSTANTS;

/** TODO: PLC100 data structure, should use structure sharing. */
typedef struct
{
    int *speech_buf_start;
    //   internal
    int *speech_buf;
    //   pointer to ola buffer.   length of the buffer is specified by constant: $plc100.ola_len_nb or $plc100.ola_len_wb
    int* ola_buf;
    //   pointer to output cbuffer structure
    tCbuffer* output;
    //   pointer to internal constant table.   nb:&$plc100.nb_consts.    wb:&$plc100.wb_consts
    PLC100_CONSTANTS *consts;
    //   ola attenuation factor, default using $plc100.initial_attenuation.
    unsigned attenuation;
    //   Packet error rate threhold field: If the packet error rate exceeds this threshold,
    //   the PLC algorithm is bypassed and damaged packets are passed through to the output unchanged.
    unsigned per_threshold;
    unsigned packet_len;
    unsigned bfi;
    unsigned bfi1;
    unsigned bfi_cur;
    unsigned per;
    unsigned pitch;
    unsigned bfi_prev;
    unsigned harm;
    unsigned tonality;
    unsigned loop_limit;
    unsigned sub_packet_len;
    unsigned loop_counter;

} PLC100_STRUC;

/*****************************************************************************
Public Constant Definitions
*/
#define SP_BUF_LEN_NB                  226
#define OLA_LEN_NB                     10

#define SP_BUF_LEN_WB                  452
#define OLA_LEN_WB                     24

/* 0.8 in Q23 */
#define PLC100_INITIAL_ATTENUATION     FRACTIONAL(0.8)
/* 0.3 in Q23 */
#define PLC100_PER_THRESHOLD           FRACTIONAL(0.3)

/*****************************************************************************
Public Function Definitions
*/
/**
 * \brief Gets the PLC constants for a given variant of PLC. Manages the allocation
 * and sharing between instances.
 *
 * \param variant Shared memory ID of the PLC constants to return.
 *
 * \return The constants for the specified variant. NULL if failure.
 */
extern PLC100_CONSTANTS *get_plc100_constants(unsigned variant);

/**
 * \brief Releases PLC constants obtained using get_plc100_constants.
 *
 * \param consts The constants to release.
 */
extern void release_plc100_constants(PLC100_CONSTANTS *consts);

/**
 * \brief This function performs the packet loss concealment process.
 *
 * \param plc_data - Pointer to plc100 data object
 */
extern void plc100_process(PLC100_STRUC *plc_data);

#endif /* PLC100_C_H */
