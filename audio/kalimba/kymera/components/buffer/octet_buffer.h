/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  octet_buffer.h
 *
 * \ingroup buffer
 *
 * Public C header file for the octet_buffer interface
 *
 */

#ifndef OCTET_BUFFER_C_H
#define OCTET_BUFFER_C_H

/****************************************************************************
Include Files
*/
#include "buffer/cbuffer_c.h"

/****************************************************************************
Public Type Declarations
*/


/**
 * Type used to define the endianness of the buffer.
 */
typedef enum
{
    BIG_ENDIAN_BUFFER = 0,                      /**< Big end first */
    LITTLE_ENDIAN_BUFFER = 1,                   /**< Little end first */
}ENDIANNESS;

/**
 * Big endian masks
 */
#define BIG_ENDIAN_32BIT_1_OCTET      (0xFF000000)
#define BIG_ENDIAN_32BIT_2_OCTET      (0x00FF0000)
#define BIG_ENDIAN_32BIT_3_OCTET      (0x0000FF00)
#define BIG_ENDIAN_32BIT_4_OCTET      (0x000000FF)

#define BIG_ENDIAN_24BIT_1_OCTET      (0x00FF0000)
#define BIG_ENDIAN_24BIT_2_OCTET      (0x0000FF00)
#define BIG_ENDIAN_24BIT_3_OCTET      (0x000000FF)

#define BIG_ENDIAN_16BIT_1_OCTET      (0x0000FF00)
#define BIG_ENDIAN_16BIT_2_OCTET      (0x000000FF)


/**
 * Little endian masks
 */
#define LITTLE_ENDIAN_1_OCTET    (0x000000FF)
#define LITTLE_ENDIAN_2_OCTET    (0x0000FF00)
#define LITTLE_ENDIAN_3_OCTET    (0x00FF0000)
#define LITTLE_ENDIAN_4_OCTET    (0xFF000000)

/**
 * Defines the available packing for a buffer.
 */
typedef enum
{
   USE_8BIT_PER_WORD = 0,               /**< Only one octets is used per word. This also
                                           can be called unpaked.*/
   USE_16BIT_PER_WORD = 1,              /**< Two octets per word.*/
   USE_24BIT_PER_WORD = 2,              /**< Three octets per word.*/
   USE_32BIT_PER_WORD = 3,              /**< Only available for kal arch 4.*/
}PACKING;

#define MULTIPLE_PACKING
/**
 * Macro returns the number of octets in a word giving the packing.
 */
#define NR_OF_OCTETS_IN_WORD(WORD_PACKING) (WORD_PACKING + 1)


/**
 * octet_buffer structure
 *
 * The octet_buffer structure can encapsulate a simple SW buffer to allow octet
 * base memory access.
 */
typedef struct octet_buffer_struct
{
    PACKING packing:2;                  /**< Packing type. Only 16bit unpacked are supported. */
    ENDIANNESS endianness:1;            /**< Endianness. Currently only big endian buffers are supported. */
    unsigned int read_offset;           /**< Read octets from the current (cbuffer read pointer) word. */
    unsigned int write_offset;          /**< Unwritten bytes in the previous (cbuffer write pointer -1) word. */
    tCbuffer *buffer;                   /**< Pointer to the cbuffer structure */
} octet_buffer;


/****************************************************************************
Public Constant Declarations
*/

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Variable Definitions
*/

/****************************************************************************
Public Function Declarations
*/


/**
 * Create an octet buffer.
 *
 * \param packing The packing of the octet buffer.
 * \param endianness The endianness of the octet buffer.
 *
 * \return pointer to octet_buffer object or NULL.
 */
extern octet_buffer *octet_buffer_create(PACKING packing, ENDIANNESS endianness);

/**
 * Destroys an octet buffer.
 *
 * Frees up the memory allocated for the octet buffer structure.
 * todo Extend the destroy logic.
 *
 *
 * \param obuffer pointer to the octet_buffer structure to be destroyed.
 * \return none.
 */
extern void octet_buffer_destroy(octet_buffer *buffer);


/**
 * Calculate amount of space in a buffer, in octets.
 *
 * Note: cbuffer_calc_amount_space_in_words returns one word less to avoid
 * completly filling up the buffer. Because our write pointer can be ahead and the read
 * pointer can be behind (the read and write offset) we need to reduce the number of
 * available octets with NR_OF_OCTETS_IN_WORD(obuffer->packing)-1. It is not one
 * word worth of octets because than the offset is 0 and the read and write pointer is
 * not behind.
 *
 * \param obuffer structure to extract information from.
 * \return free space in the buffer in octets
 */
extern unsigned int obuffer_calc_amount_space_in_octets (octet_buffer *obuffer);


/**
 * Calculate amount of data in a buffer, in octets.
 *
 * \param obuffer structure to extract information from.
 * \return amount of data in the buffer in octets
 */
extern unsigned int obuffer_calc_amount_data_in_octets (octet_buffer *obuffer);

/**
 * \brief Get the size of a obuffer, in octets.
 *
 * TODO: If only 16bit unpacked data will be used this function and others can be optimised.
 *
 * \param obuffer structure to extract information from.
 * \return size of the buffer, in addresses (or locations)
 */
extern unsigned int obuffer_get_size_in_octets(octet_buffer *obuffer);

/**
 * Copy data from one obuffer to another.
 *
 * Copies min(amount_to_copy, data available in cbuffer_src, space_available in
 * cbuffer_dest) words of data from cbuffer_src to cbuffer_dest.
 *
 * \param obuffer_dest structure to write data to.
 * \param obuffer_src structure to read data from.
 * \param amount_to_copy words to copy.
 * \return how much data we actually copied (can be different from amount_to_copy).
 */
extern unsigned int obuffer_copy_octets(octet_buffer *obuffer_dest, octet_buffer *obuffer_src, unsigned int amount_to_copy);


/**
 * Read data from an octet buffer unpacks it and puts to the destination memory address.
 * 
 * NOTE: This function advances the read pointer of the buffer.
 *
 * \param dest destination of data.
 * \param obuffer_src structure to read data from.
 * \param amount_to_read octets to read.
 * \return how much data we actually read (can be different from amount_to_read).
 */
extern unsigned int obuffer_unpack(int *dest, octet_buffer *obuffer_src, unsigned int amount_to_copy);

/**
 * Read data from a given offset in an octet buffer unpacks it and puts to the
 * destination memory address.
 *
 * NOTE: This function does NOT advances the read pointer of the buffer.
 *
 * \param dest destination of data.
 * \param obuffer_src structure to read data from.
 * \param offset .
 * \param amount_to_read octets to read.
 * \return how much data we actually read (can be different from amount_to_read).
 */
extern unsigned int obuffer_unpack_from_offset(int *dest, octet_buffer *obuffer_src,
        unsigned int offset, unsigned int amount_to_copy);


/**
 * Discard a specified amount of data. 
 *
 * TODO: Only works on 16-bit unpacked, Return the discarded amount. 
 * 
 * \param obuffer pointer to a obuffer structure.
 * \param discard_amount Amount of data to discard.
 * \return none
 */
extern void obuffer_discard_data(octet_buffer *obuffer, unsigned int discard_amount);

/**
 * Function return the read indext of the octet buffer. The octet buffer only indexes the
 * usable octets in the Cbuffer. For example in 16bit unpacked only two octets are
 * indexed per word.
 *
 * \param obuffer pointer to a obuffer structure.
 * \return the read index (only usable octets are counted)
 */
extern unsigned int obuffer_get_read_index(octet_buffer *obuffer);

/**
 *  Advances write index of a octet buffer by a supplied value (in octets).
 *
 * \param obuffer pointer to cbuffer to modify
 * \param amount value by which the write pointer advances
 * \return none
 *
 */
extern void obuffer_advance_write_ptr(octet_buffer *obuffer, unsigned int amount);


#endif /* OCTET_BUFFER_C_H */
