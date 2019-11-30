/*****************************************************************************
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*****************************************************************************/
/**
 * \defgroup ext_buffer Ext_buffer Subsystem
 * \file  ext_buffer.h
 *
 * \ingroup ext_buffer
 *
 * Public C header file for the ext_buffer interface
 *
 */

#ifndef EXTERNAL_BUFFER_PATCH_H
#define EXTERNAL_BUFFER_PATCH_H

#include "buffer/buffer.h"

#define EXT_BUFFER_CIRC_ACCESS_FLAG_MASK       0

#define EXT_BUFFER_RANDOM_ACCESS_FLAG_SHIFT    0
#define EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK     (1 << EXT_BUFFER_RANDOM_ACCESS_FLAG_SHIFT)
#define EXT_BUFFER_RANDOM_ACCESS_FLAG_SET(x)   ((x) |= EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK)
#define EXT_BUFFER_RANDOM_ACCESS_FLAG_UNSET(x) ((x) &= ~EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK)
#define EXT_BUFFER_RANDOM_ACCESS_FLAG(x)       ((x) & EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK)

typedef struct EXT_BUFFER EXT_BUFFER;

/****************************************************************************
 *
 * The external buffers are designed to work in two modes:
 *
 *  * RANDOM ACCESS
 *    + EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK set in the flags on creation
 *    + Functions available:
 *      - ext_buffer_random_access_read()           EXT -> DM RAM
 *      - ext_buffer_random_access_write()          DM RAM -> EXT
 *      - ext_buffer_destroy()
 *
 * * CIRCULAR
 *    + EXT_BUFFER_CIRC_ACCESS_FLAG_MASK set in the flags on creation
 *    + Functions available:
 *      - ext_buffer_get_read_offset()
 *      - ext_buffer_set_read_offset()
 *      - ext_buffer_get_write_offset()
 *      - ext_buffer_set_write_offset()
 *      - ext_buffer_amount_data()
 *      - ext_buffer_amount_space()
 *      - ext_buffer_circ_read()                    EXT -> DM RAM
 *      - ext_buffer_circ_write()                   DM RAM -> EXT
 *      - ext_buffer_pack()                         DM RAM -> EXT
 *      - ext_buffer_unpack()                       EXT -> DM RAM
 *      - ext_buffer_destroy()
 *
 * Notes:
 *  * Circular buffer functions called on random access buffers and
 *    vice versa will have no effect and/or return 0;
 *  * For random read and write accesses, the user's should ensure
 *    that there are no out of bounds accesses for the buffer residing
 *    in the DM RAM (e. g. ext_buffer_random_access_read() - the
 *    combination of dest_offset and amount should not access anything
 *    past the end of dest_buffer).
 *
 ****************************************************************************/


/****************************************************************************
 * ext_buffer_create - used to create SW buffers located out of DM RAM
 *
 * Input arguments:
 *      buffer_size         - in words
 *      flags               - collection of flags (only one supported):
 *                            * EXT_BUFFER_RANDOM_ACCESS_FLAG_MASK
 *                                ** random access buffer if set,
 *                                   circular buffer otherwise
 * Return value
 *      If successful - pointer to the created ext buffer structure else NULL
 *
 */
extern EXT_BUFFER *ext_buffer_create(unsigned int buffer_size, unsigned int flags);

/****************************************************************************
 * ext_buffer_destroy - destroys buffers created by ext_buffer_create
 *
 * Input argument:
 *      ext_buffer   - pointer to a external buffer to destroy
 *
 */
extern void ext_buffer_destroy(EXT_BUFFER *ext_buffer);

/****************************************************************************
 * get read offset from the start of the buffer [octets]
 *
 * Input arguments:
 *      eb                  - ext buffer pointer
 * Return value
 *      read offset
 */
extern unsigned int ext_buffer_get_read_offset(EXT_BUFFER * eb);

/****************************************************************************
 * set read offsetfrom the start of the buffer [octets]
 *
 * Input arguments:
 *      eb                  - ext buffer pointer
 *      offset              - read offset
 * Return value
 *      read offset
 */
extern void ext_buffer_set_read_offset(EXT_BUFFER * eb, unsigned int offset);

/****************************************************************************
 * get write offset from the start of the buffer [octets]
 *
 * Input arguments:
 *      eb                  - ext buffer pointer
 * Return value
 *      write offset
 */
extern unsigned int ext_buffer_get_write_offset(EXT_BUFFER * eb);

/****************************************************************************
 * set write offset from the start of the buffer [octets]
 *
 * Input arguments:
 *      eb                  - ext buffer pointer
 *      offset              - write offset
 * Return value
 *      read offset
 */
extern void ext_buffer_set_write_offset(EXT_BUFFER * eb, unsigned int offset);

/****************************************************************************
 * calculate the amount of data available [octets]
 *
 * Input argument:
 *      eb                  -  ext buffer pointer
 * Return value
 *      data available in octets
 */
extern unsigned ext_buffer_amount_data(EXT_BUFFER * eb);

/****************************************************************************
 * calculate the amount of space available [octets]
 *
 * Input argument:
 *      eb                  -  ext buffer pointer
 * Return value
 *      space available in octets
 */
extern unsigned ext_buffer_amount_space(EXT_BUFFER * eb);

/****************************************************************************
 * Random access read from external buffer to DM RAM buffer
 *
 * Input argument:
 *      dest_buffer         -  destination buffer pointer
 *      src_eb              -  source buffer pointer (external)
 *      dest_offset         -  offset in destination buffer to write to [octets]
 *      src_offset          -  offset in source buffer from which to read [octets]
 *      num_octets          -  amount to read
 * Return value
 *      amount read = 0 if unsuccessful read, amount passed in at call otherwise
 */
extern unsigned int ext_buffer_random_access_read(int * dest_buffer, EXT_BUFFER * src_eb, unsigned int dest_offset, unsigned int src_offset, unsigned int num_octets);

/****************************************************************************
 * Random access write from DM RAM buffer to external buffer
 *
 * Input argument:
 *      dest_eb             -  destination buffer pointer (external)
 *      src_buffer          -  source buffer pointer
 *      dest_offset         -  offset in destination buffer to write to [octets]
 *      src_offset          -  offset in source buffer from which to read [octets]
 *      num_octets          -  amount to write
 * Return value
 *      amount read = 0 if unsuccessful write, amount passed in at call otherwise
 */
extern unsigned int ext_buffer_random_access_write(EXT_BUFFER * dest_eb, int * src_buffer, unsigned int dest_offset, unsigned int src_offset, unsigned int num_octets);

/****************************************************************************
 * Circular access - circular read from external buffer to destination cbuffer
 *
 * Input argument:
 *      dest                -  destination cbuffer pointer
 *      ext_src             -  source buffer pointer (external)
 *      num_octets          -  amount to read
 * Return value
 *      amount read = 0 if unsuccessful read, amount passed in at call otherwise
 */
extern unsigned int ext_buffer_circ_read(tCbuffer * dest_buffer, EXT_BUFFER * ext_src, unsigned int num_octets);

/****************************************************************************
 * Circular access - circular write from cbuffer to external buffer
 *
 * Input argument:
 *      ext_dest            -  destination buffer pointer (external)
 *      src                 -  source cbuffer pointer
 *      num_octets          -  amount to write
 * Return value
 *      amount read = 0 if unsuccessful write, amount passed in at call otherwise
 */
extern unsigned int ext_buffer_circ_write(EXT_BUFFER * ext_dest, tCbuffer * src_buffer, unsigned int num_octets);

/****************************************************************************
 * Circular access - discard data from external buffer
 *
 * Input argument:
 *      eb                  -  ext buffer pointer
 *      num_octets          -  amount to discard
 */
void ext_buffer_discard_data(EXT_BUFFER * eb, unsigned int num_octets);

/****************************************************************************
 * Circular access - pack from cbuffer to external buffer
 *
 *      |<----32bit----->|                 |<------------32bit------------->|
 * src  +----------------+     ext_dest    +--------------------------------+
 *      | W0             |                 | W1(top 16bits)| W0(top 16bits) |
 *      +----------------+                 +--------------------------------+
 *      | W1             |                 | W3(top 16bits)| W2(top 16bits) |
 *      +----------------+   ---------->   +--------------------------------+
 *      | W2             |                 | ...                            |
 *      +----------------+                 +--------------------------------+
 *      | W3             |                 | ...                            |
 *      +----------------+                 +--------------------------------+
 * Input argument:
 *      ext_dest            -  destination buffer pointer (external)
 *      src                 -  source cbuffer pointer
 *      num_words           -  amount to pack
 * Return value
 *      amount packed in words (0 if unsuccessful)
 */
unsigned int ext_buffer_pack(EXT_BUFFER * ext_dest, tCbuffer * src, unsigned int num_words);

/****************************************************************************
 * Circular access - unpack from external buffer to destination cbuffer
 *
 *          |<----32bit----->|            |<------------32bit------------->|
 * ext src  +----------------+     dest   +--------------------------------+
 *          | W0             |            | W0(bottom 16bits)| 16 bits of 0|
 *          +----------------+            +--------------------------------+
 *          | W1             |            | W0(top 16bits)   | 16 bits of 0|
 *          +----------------+   ----->   +--------------------------------+
 *          | ...            |            | W1(bottom 16bits)| 16 bits of 0|
 *          +----------------+            +--------------------------------+
 *          | ...            |            | W1(top 16bits)   | 16 bits of 0|
 *          +----------------+            +--------------------------------+

 * Input argument:
 *      dest                -  destination cbuffer pointer
 *      ext_src             -  source buffer pointer (external)
 *      num_words           -  amount to unpack
 * Return value
 *      amount unpacked in words (0 if unsuccessful)
 */
unsigned int ext_buffer_unpack(tCbuffer * dest, EXT_BUFFER * ext_src, unsigned int num_words);


#endif /* EXTERNAL_BUFFER_PATCH_H */
