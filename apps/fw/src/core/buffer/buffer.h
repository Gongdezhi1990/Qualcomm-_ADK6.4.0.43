/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Application Subsystem buffer subsystem main public header file
*/

#ifndef BUFFER_H
#define BUFFER_H

/*lint -e750 -e962*/ #define IO_DEFS_MODULE_APPS_SYS_CPU0_VM /* for mmu_buffer_size */
#include "hydra/hydra_types.h"
#include "mmu/mmu.h" /* for mmu_* functions */
#include "io/io_defs.h" /* We need this for msize */
#include "hal/hal.h"


typedef struct {
/* Changed __KALIMBA__ to SUBSYTEM_AUDIO as this only for kalimba audio */
#ifdef __SUBSYTEM_AUDIO__
    uint16 dummy; /* Kalimba doesn't work this way */
#else
    mmu_proc_port_state     read_port_state;
    mmu_proc_port_state     write_port_state;
#endif
} buf_mapping_state;

void buf_save_state(buf_mapping_state *save_state);

/**
 * \c buf_save_state() simply calls the corresponding MMU routine.
 */
#define buf_save_state(save_state) ((void)(save_state))

void buf_restore_state(const buf_mapping_state *save_state);

/**
 * \c buf_restore_state() simply calls the corresponding MMU routine.
 */
#define buf_restore_state(restore_state) ((void)(restore_state))

/*@}*/

/* The BUFFER interface is defined within this file. */

/**
 * The buffer structure itself.  This encapsulates an MMU buffer.
 *
 * A \c BUFFER has only one handle, called \c handle.
 * It is used for both reading from and writing to the buffer.
 */
typedef struct BUFFER {
    /**
     * The MMU handle associated with the buffer.
     */
    mmu_handle handle;
    /**
     * The point up to which the buffer's data has been written.  This
     * pointer wraps at the size of buffer.
     */
    uint16 index;
    /**
     * The point up to which the buffer's data has been read.  This
     * follows \c index.  A non-zero difference (tested by
     * \c BUF_GET_AVAILABLE()) indicates there is data that has
     * not yet been read.
     */
    uint16 outdex;
    /**
     * The point up to which the buffer's data has been cleared
     * (and, if appropriate, previous pages freed).  This follows
     * \c index and \c outdex.  A non-zero difference between
     * \c index and \c tail (tested by \c BUF_GET_USED()) indicates that
     * not all data has been freed.
     *
     * The distinction between \c outdex and \c tail is largely for
     * the benefit of maintaining the state of peripherals.  When
     * a peripheral is supplied with data up to a certain point,
     * \c outdex is advanced.  When that memory is no longer in use
     * by the hardware, \c tail is advanced to the same point.
     */
    uint16 tail;
    /**
     * Mask to keep operations on indices within the power-of-two
     * buffer size.
     *
     * The capacity of the buffer is given by (\c size_mask \c + \c 1).
     */
    uint16 size_mask;
} BUFFER;

/*@{*/
/**
 * Set up a BUFFER structure around an already-created MMU handle
 * @param h The MMU handle that has been supplied
 * @param msize The size associated with the handle
 * @param buf The BUFFER structure to initialise
 */
extern void buf_init_from_handle(mmu_buffer_size msize, mmu_handle h,
                                 BUFFER *buf);

/**
 * Allocate and set up a BUFFER structure around an already-created MMU handle
 * @param h The MMU handle that has been supplied
 * @param msize The size associated with this handle
 * @return Initialised BUFFER structure wrapping the supplied handle
 */
extern BUFFER *buf_new_from_handle(mmu_buffer_size msize, mmu_handle h);

/**
 * Free a BUFFER structure allocated by \c buf_new_buffer_handle
 * @param buf The BUFFER structure to free
 * @return The mmu_handle that was wrapped; this may be useful for issuing a
 * separate request to free the handle
 */
extern mmu_handle buf_free_from_handle(BUFFER *buf);
/*@}*/


/**
 * @name Macros for pointer arithmetic and information retrieval.
 */
/*@{*/

/**
 * Subtract buffer indices with mask to take account of wrap.
 *
 * This is used when (as normal) the buffer size is a power of two.
 */
#define BUF_SUB_INDICES_MASK(hi, lo, mask) \
  ((uint16)(((hi) - (lo)) & (mask)))

/**
 * Fast operation to fetch the size of the buffer in octets.
 * The buffer is assumed to be no larger than 32k octets.
 * This is a pretty good bet.
 */
#define BUF_GET_SIZE_OCTETS(buf)         ((uint16)((buf)->size_mask + 1U))

/**
 * Subtract buffer indices given the buffer.
 */
#define BUF_SUB_INDICES(buf, hi, lo)                  \
    BUF_SUB_INDICES_MASK(hi, lo, (buf)->size_mask)

#define BUF_ADD_INDICES(buf, a, b) ((uint16)(((a) + (b)) & (buf)->size_mask))

/**
 * Update a buffer pointer by a certain amount.
 * Pointer is typically one of index, outdex or tail.  It must be an
 * lvalue.
 */
#define BUF_UPDATE_POINTER(buf, pointer, octets) \
    ((void)(pointer = (uint16)(((pointer) + (octets)) & ((buf)->size_mask))))

/* TODO: Mask not required because HW does it? */
#define BUF_SYNC_INDEX_TO_WR_HANDLE(buf) \
    ((void) ((buf)->index = (uint16)(mmu_get_handle_offset((buf)->handle) & \
                                                            (buf)->size_mask)))

#define BUF_SYNC_OUTDEX_TO_RD_HANDLE(buf) \
    ((void) ((buf)->outdex = (uint16)(mmu_get_handle_offset((buf)->handle) & \
                                                            (buf)->size_mask)))

/**
 * Return octets available to be read from a buffer.
 *
 * This is the difference between the input pointer (\c index)
 * and the output pointer (\c outdex) assuming wrapping pointers.
 * This value will usually be read by the thread that is managing
 * output from the buffer and hence the \c outdex pointer.  Note
 * that the \c index may be advanced asynchronously by the input thread.
 */
#define BUF_GET_AVAILABLE(buf) \
    BUF_SUB_INDICES_MASK((buf)->index, (buf)->outdex, (buf)->size_mask)

/**
 * Return octets in use in a buffer.
 *
 * This is the difference between the \c index and \c tail pointers,
 * so indicates if any data has been written the buffer without
 * being cleared.  Otherwise it is similar to \c BUF_GET_AVAILABLE().
 */
#define BUF_GET_USED(buf) \
    BUF_SUB_INDICES_MASK((buf)->index, (buf)->tail, (buf)->size_mask)

/**
 * Return octets that have been passed for further processing but not freed.
 *
 * This is the difference between the \c outdex and \c tail pointers,
 * so indicates if any data has been processed by advancing \c outdex
 * but not freed by advancing tail.  Otherwise it is similar to \c
 * BUF_GET_AVAILABLE().
 */
#define BUF_GET_UNFREED(buf) \
    BUF_SUB_INDICES_MASK((buf)->outdex, (buf)->tail, (buf)->size_mask)

/**
 * Return octets of free space available in a buffer.
 *
 * This is (buffer size - the difference between the input pointer (\c
 * index) and the output pointer (\c tail) assuming wrapping
 * pointers). This value will usually be read by the thread that is
 * writing data to the buffer to know whether it has enough space to
 * write the data.
 *
 * This has 1 subtracted in order that the buffer never becomes
 * full, which would cause the indices to become confused (there
 * is no distinction between the values that differ by a multiple
 * of the buffer size).
 *
 * For buffers with more than a single page
 * we need to ensure that the index does not advance onto the same page
 * as the tail, since then if we free data at the tail we will free
 * the page with the new data.  Hence mask tail back to the start
 * of the page (keeping the "-1" to simplify the calculation).
 * Buffers with a single page will never have that page freed during use
 * so the calculation is simplified in that case.
 */
#define BUF_GET_FREESPACE(buf)                                          \
    ((BUF_GET_SIZE_OCTETS(buf) -                                         \
     BUF_SUB_INDICES_MASK((buf)->index,                                 \
                          (BUF_GET_SIZE_OCTETS(buf) == MMU_PAGE_BYTES)? \
                          (buf)->tail :                                 \
                          (buf)->tail & ~(uint16)(MMU_PAGE_BYTES-1),    \
                          (buf)->size_mask)) - 1U)

/**
 * Return the maximum amount of data that can be stored in buffer.
 * For calculation refer comment of BUF_GET_FREESPACE().
 * TODO: Use of buffer pointer than buffer size to generalized this macro.
 */
#define BUF_GET_USABLESPACE(buf_size)             \
        (uint16)((((buf_size) == MMU_PAGE_BYTES)?    \
        (buf_size) :                                \
        (buf_size) - MMU_PAGE_BYTES) - 1U)

/**
 * Return octets of free space available for statically managed buffer.
 *
 * This differs from \c BUF_GET_FREESPACE() in that it should only be
 * used in \c BUFFER structures where memory is never freed dynamically
 * (only on buffer destruction).  In that case we do not need to take
 * account of whether \c index and \c tail are on the same page.
 */
#define BUF_GET_FREESPACE_STATIC(buf)                                   \
    (BUF_GET_SIZE_OCTETS(buf) -                                         \
     (BUF_SUB_INDICES_MASK((buf)->index, (buf)->tail, (buf)->size_mask) + 1U))

/**
 * Return octets of free space available for a statically managed buffer
 * using an index value outside the normal buffer structure.
 *
 * As with \c BUF_GET_FREESPACE_STATIC() it should only be
 * used in \c BUFFER structures where memory is never freed dynamically
 * (only on buffer destruction).  In that case we do not need to take
 * account of whether \c index and \c tail are on the same page.
 */
#define BUF_GET_FREESPACE_STATIC_FROM_INDEX(buf, index)                 \
    (BUF_GET_SIZE_OCTETS(buf) -                                         \
     BUF_SUB_INDICES_MASK((index), (buf)->tail, (buf)->size_mask) - 1U)

/*@}*/

/**
 * @name Use of buffer pointers "raw" (without application framing).
 *
 * The code below updates buffer pointers based on lengths
 * passed directly.  For the \c BUFFER interface this is the only
 * available method.
 */
/*@{*/

/**
 * Return an 8-bit pointer to write data to a buffer.
 *
 * The buffer is mapped for read/write access at the input location
 * (\c index) and a pointer is returned.  The view of the data
 * is 8-bit-wide.
 *
 * After the data is written, \c buf_raw_write_update() should be
 * called to indicate how many octets were written.  This updates
 * \c index.
 */
extern uint8 *
buf_raw_write_map_8bit(BUFFER *buf);

#define buf_raw_write_map_8bit(buf)\
        buf_raw_write_map_8bit_position(buf, (buf)->index)

/**
 * Generalisation of \c buf_raw_write_map_8bit to return read/write access at
 * an arbitrary position within the buffer.
 * WARNING: No check is performed on the validity of \p position wrt mapping in
 * of pages!
 */
extern uint8 *
buf_raw_write_map_8bit_position(BUFFER *buf, uint16 offset);



/**
 * Return a 16-bit pointer to write data to a buffer.
 *
 * The buffer is mapped for read/write access at the input location
 * (\c index) and a pointer is returned.  The view of the data
 * is 16-bit-wide.  \c index must be even or this causes a panic;
 * the caller must ensure all updates to buffers that will be
 * accessed in 16-bit mode use even byte counts.
 *
 * After the data is written, \c buf_raw_write_update() should be
 * called to indicate how many octets were written.  This updates
 * \c index.  Note the argument really is in octets even though
 * half that number of words will have been written.
 */
extern uint16 *
buf_raw_write_map_16bit(BUFFER *buf);

/**
 * As \c buf_raw_write_map_16bit() but map words big-endian.
 */
extern uint16 *
buf_raw_write_map_16bit_be(BUFFER *buf);

/****************************************************************************
NAME
    buf_raw_write_only_unmap - unmap the buffer for writing ONLY.

FUNCTION
    This function unmaps the buffer from the write port. A buffer
    that is mapped using any of the nbuf_raw_write_only_map_xxx functions
    MUST be unmapped once it is done with the writing of this buffer.

    If a nested function is called after mapping a buffer but before
    unmapping it, the nested function MUST use xxx_save_state functions
    to map the buffer. The unmapping is done using the appropriate unmap
    function. Additionally the buf state MUST be restored using one of the
    appropriate restore functions. If a write update is required, it
    can use xxx_update_restore fucntions, otherwise it can use
    buf_raw_buf_restore_state()

    Note that the updation of the message length is unaffected by this
    new buffer usage model

    Example usage:

    fn1
    {
        uint8 *p_data1;
        ......
        p_data1 = buf_raw_write_only_map_8bit(buffer1);

        write few bytes to the buffer1

        fn2(buffer2); // nested function

        buf_raw_write_only_unmap(buffer1);

        // This is required only if the index needs to be updated
        buf_raw_write_update(buffer1);
    }

    fn2(buffer2)
    {
        uint8 *p_data2;
        ......

        p_data2 = buf_raw_write_only_map_8bit_save_state(buffer2);

        write few bytes to the buffer2

        buf_raw_write_only_unmap(buffer2);

        //This is required only if the index needs to be updated
        buf_raw_write_update_restore(buffer2);
                        OR
        // If neither index nor front needs to be updated
        buf_raw_buf_restore_state();
    }

    See the buf_raw_read_unmap() header for an example of a nested function
    that copies one section of the buffer to another non-overlapped section
    of the same buffer.

 */
extern void buf_raw_write_only_unmap(void);

/****************************************************************************
NAME
    buf_raw_write_unmap - unmap the buffer from reading and writing.

FUNCTION
    This function unmaps the buffer from the read and write ports. A buffer
    that is mapped using any of the buf_raw_write_map_xxx functions MUST be
    unmapped once it is done with the reading/writing of this buffer.

    If a nested function is called after mapping a buffer but before
    unmapping it, the nested function MUST use xxx_save_state functions
    to map the buffer. The unmapping is done using the appropriate unmap
    function. Additionally the buf state MUST be restored using one of the
    appropriate restore functions. If a write update is required, it
    can use xxx_update_restore functions, otherwise it can use
    buf_raw_buf_restore_state()

    Note that the updation of the message length is unaffected by this
    new buffer usage model

    Example usage:

    fn1
    {
        uint8 *p_data1;
        ......
        p_data1 = buf_raw_write_map_8bit(buffer1);

        write/read few bytes to/from the buffer1

        fn2(buffer2); // nested function

        buf_raw_write_unmap(buffer1);

        //This is required only if the index needs to be updated
        buf_raw_write_update(buffer1);
    }

    fn2(buffer2)
    {
        uint8 *p_data2;
        ......

        p_data2 = buf_raw_write_map_8bit_save_state(buffer2);

        write/read few bytes to/from the buffer2

        buf_raw_write_unmap(buffer2);

        //This is required only if the index needs to be updated
        buf_raw_write_update_restore(buffer2);
                        OR
        // In case neither index nor front needs to be updated
        buf_raw_buf_restore_state();
    }
 */

extern void buf_raw_write_unmap(void);

/**
 * As \c buf_raw_write_map_8bit(), but map write only, not read/write.
 *
 * This is necessary for copying between buffers when a simultaneous
 * read mapping is used.
 */
extern uint8 *
buf_raw_write_only_map_8bit(BUFFER *buf);

/**
 * As \c buf_raw_write_only_map_8bit(), but save the buffer state.
 *
 * This is necessary for copying between buffers when a simultaneous
 * read mapping is used.
 */
extern uint8 *
buf_raw_write_only_map_8bit_save_state(BUFFER *buf, buf_mapping_state *save_state);

/**
 * As \c buf_raw_write_map_16bit(), but map write only, not read/write.
 *
 * This is necessary for copying between buffers when a simultaneous
 * read mapping is used.
 */
extern uint16 *
buf_raw_write_only_map_16bit(BUFFER *buf);

/**
 * As \c buf_raw_write_only_map_16bit(), but map words big-endian.
 */
extern uint16 *
buf_raw_write_only_map_16bit_be(BUFFER *buf);

/**
 * As \c buf_raw_write_map_8bit(), but save the buffer state.
 *
 * \c save_state will be used to save buffer state
 * that can be restored later.
 */
extern uint8 *
buf_raw_write_map_8bit_save_state(BUFFER *buf, buf_mapping_state *save_state);

/**
 * As \c buf_raw_write_map_16bit(), but save the buffer state.
 *
 * \c save_state will be used to save buffer state
 * that can be restored later.
 */
extern uint16 *
buf_raw_write_map_16bit_save_state(BUFFER *buf, buf_mapping_state *save_state);

/**
 * Update \c index for a buffer after writing \c octets octets to it.
 * If the buffer was written in 16-bit mode \c octets must be even.
 */
extern void
buf_raw_write_update(BUFFER *buf, uint16 octets);

/** Updating the index can be done as a macro */
#define buf_raw_write_update(buf, octets) \
    BUF_UPDATE_POINTER(buf, (buf)->index, octets)

/**
 * As \c buf_raw_write_update() but restore a previously saved buffer state.
 */
extern void
buf_raw_write_update_restore_state(BUFFER *buf, uint16 octets,
                                   const buf_mapping_state *save_state);

/**
 * Return an 8-bit pointer to read data from a buffer.
 *
 * The buffer is mapped for read access at the output location
 * (\c outdex) and a pointer is returned.  The view of the data
 * is 8-bit-wide.
 *
 * After the data is read, \c buf_raw_read_update() should be
 * called to indicate how many octets were read.  This updates
 * \c outdex.
 */
extern const uint8 *buf_raw_read_map_8bit(const BUFFER *buf);

/**
 * Return an 8-bit pointer to read and write data in a buffer.
 *
 * As \c buf_raw_read_map_8bit() but additionally with write mapping.
 */
extern uint8 *
buf_raw_read_write_map_8bit(BUFFER *buf);

/**
 * Return a 16-bit pointer to read data from a buffer.
 *
 * The buffer is mapped for read access at the output location
 * (\c outdex) and a pointer is returned.  The view of the data
 * is 16-bit-wide.  \c outdex must be even or this causes a panic;
 * the caller must ensure all updates to buffers that will be
 * accessed in 16-bit mode use even byte counts.
 *
 * After the data is read, \c buf_raw_read_update() should be
 * called to indicate how many octets were read.  This updates
 * \c outdex.  Note the argument really is in octets even though
 * half that number of words will have been written.
 */
extern const uint16 *buf_raw_read_map_16bit(const BUFFER *buf);

/**
 * As \c buf_raw_read_map_16bit(), but map words big-endian.
 */
extern const uint16 *buf_raw_read_map_16bit_be(const BUFFER *buf);

/**
 * Return an 16-bit pointer to read and write data in a buffer.
 *
 * As \c buf_raw_read_map_16bit() but additionally with write mapping.
 */
extern uint16 *
buf_raw_read_write_map_16bit(BUFFER *buf);

/**
 * As \c buf_raw_read_write_map_16bit() but map words big-endian.
 */
extern uint16 *
buf_raw_read_write_map_16bit_be(BUFFER *buf);

/****************************************************************************
NAME
    buf_raw_read_only_unmap - unmap the buffer from reading ONLY.

FUNCTION
    This function unmaps the buffer from the read port. A buffer
    that is mapped using any of the buf_raw_read_only_map_xxx functions MUST be
    unmapped once it is done with the reading of this buffer.

    If a nested function is called after mapping a buffer but before
    unmapping it, the nested function MUST use xxx_save_state functions
    to map the buffer. The unmapping is done using the appropriate unmap
    function. Additionally the buf state MUST be restored using one of the
    appropriate restore functions. If a read/write update is required, it
    can use xxx_update_restore fucntions, otherwise it can use
    buf_raw_buf_restore_state().

    Note that the updation of the message length is unaffected by this
    new buffer usage model

    Example usage:

    fn1
    {
        uint8 *p_data1;
        ......
        p_data1 = buf_raw_read_only_map_8bit(buffer1);

        read few bytes from the buffer1

        fn2(buffer2); // nested function

        buf_raw_read_only_unmap();

        //This is required only if the outdex needs to be updated
        buf_raw_read_update(buffer1);
    }

    fn2(buffer2)
    {
        uint8 *p_data2;
        ......

        // Note that the distance between index and outdex is more
        // than the memcpy length to ensure the source and destination
        // buffer sections are non-overlapping
        p_rdata = buf_raw_read_map_8bit_save_state(buffer2);

        p_wdata = buf_raw_write_only_map_8bit(buffer2);
        memcpy(p_wdata, p_rdata, len);
        buf_raw_write_only_unmap();

        //This is required only if the index needs to be updated
        buf_raw_write_update(buffer2);

        nbuf_raw_read_unmap();

        //This is required only if index needs to be updated.
        buf_raw_read_update_restore(buffer2)

        // If neither outdex nor back needs to be updated
        buf_raw_buf_restore_state();
    }

 */
extern void buf_raw_read_unmap(void);

/****************************************************************************
NAME
    buf_raw_read_write_unmap - unmap the buffer from reading and writing.

FUNCTION
    This function unmaps the buffer from the read and write ports. A buffer
    that is mapped using any of the buf_raw_read_write_map_xxx functions
    MUST be unmapped once it is done with the reading/writing of this buffer.

    If a nested function is called after mapping a buffer but before
    unmapping it, the nested function MUST use xxx_save_state functions
    to map the buffer. The unmapping is done using the appropriate unmap
    function. Additionally the buf state MUST be restored using one of the
    appropriate restore functions. If a read/write update is required, it
    can use xxx_update_restore fucntions, otherwise it can use
    buf_raw_buf_restore_state()

    Note that the updation of the message length is unaffected by this
    new buffer usage model

    Example usage:

    fn1
    {
        uint8 *p_data1;
        ......
        p_data1 = buf_raw_read_write_map_8bit(buffer1);

        read/write few bytes from the buffer1

        fn2(buffer2); // nested function

        buf_raw_read_write_unmap();

        //This is required only if the outdex needs to be updated
        buf_raw_read_update(buffer1);
    }

    fn2(buffer2)
    {
        uint8 *p_rdata;
        ......

        p_data2 = buf_raw_read_write_map_8bit_save_state(buffer2);

        read/write few bytes from the buffer2

        buf_raw_read_write_unmap();

        //This is required only if the outdex needs to be updated
        buf_raw_read_update_restore(buffer2);
                        OR
        // If neither outdex nor back needs to be updated
        buf_raw_buf_restore_state();
    }
 */
extern void buf_raw_read_write_unmap(void);

/**
 * As \c buf_raw_read_map_8bit(), but save the buffer state.
 *
 * \c save_state should point to memory at least \c
 * MMU_SAVE_STATE_WORDS long.  This will be used to save buffer state
 * that can be restored later.
 */
extern const uint8 *buf_raw_read_map_8bit_save_state(const BUFFER *buf,
                                                     buf_mapping_state *save_state);

/**
 * As \c buf_raw_read_map_8bit_save_state(), but mapped read/write.
 */
extern uint8 *
buf_raw_read_write_map_8bit_save_state(BUFFER *buf, buf_mapping_state *save_state);

/**
 * As \c buf_raw_read_map_16bit(), but save the buffer state.
 *
 * \c save_state will be used to save buffer state
 * that can be restored later.
 */
extern const uint16 *buf_raw_read_map_16bit_save_state(const BUFFER *buf,
                                                       buf_mapping_state *save_state);

/**
 * As \c buf_raw_read_map_16bit_save_state(), but mapped read/write.
 */
extern uint16 *
buf_raw_read_write_map_16bit_save_state(BUFFER *buf, buf_mapping_state *save_state);

/**
 * Update \c outdex for a buffer after reading \c octets octets from it.
 * If the buffer was read in 16-bit mode \c octets must be even.
 */
extern void
buf_raw_read_update(BUFFER *buf, uint16 octets);

/** Updating the outdex can be done as a macro */
#define buf_raw_read_update(buf, octets) \
    BUF_UPDATE_POINTER(buf, (buf)->outdex, octets)

/**
 * As \c buf_raw_read_update() but restore a previously saved buffer state.
 * \c save_state is the same pointer passed to the read map call.
 */
extern void
buf_raw_read_update_restore_state(BUFFER *buf, uint16 octets,
                                  const buf_mapping_state *save_state);

/**
 * Update the tail pointer, freeing any memory no longer needed.
 *
 * The new pointer is used to replace the old tail pointer.  Any
 * memory between the two pointers is a candidate for freeing.
 * It will only be freed if a complete page is now unused; this
 * is handled by \c mmu_free_buffer_memory().
 *
 * This function does not map buffer memory.
 *
 * Note the difference in parameters between this and functions
 * that update after writing (\c index) or reading (\c outdex):
 * in this case, the second parameter is the new value of the
 * \c tail, not the length to add to the old \c tail.  I'm
 * not sure why that is.
 */
extern void
buf_raw_update_tail_free(BUFFER *buf, uint16 new_tail);

/**
 * Update the tail pointer
 *
 * The new pointer is used to replace the old tail pointer.  Any
 * memory between the two pointers may be reused but isn't freed.
 *
 * This function does not map buffer memory.
 *
 * Note the difference in parameters between this and functions
 * that update after writing (\c index) or reading (\c outdex):
 * in this case, the second parameter is the new value of the
 * \c tail, not the length to add to the old \c tail.  I'm
 * not sure why that is.
 */
#define buf_raw_update_tail_no_free(buf, new_tail)              \
    do{ (buf)->tail = (uint16)((new_tail) & (BUF_GET_SIZE_OCTETS((buf)) - 1)); }while(0)

/*@}*/

/**
 * Update the index of a clone buffer to match that of the buffer it was
 * cloned from.
 */
extern void
buf_clone_update_index(BUFFER *clone_buf, const BUFFER *source_buf);

/** Macro implementation */
#define buf_clone_update_index(clone,src)   ((clone)->index = (src)->index)

/** Make mmu read port close visible at the buffer level */
#define buf_read_port_close()       mmu_read_port_close()

/** Make mmu write port close visible at the buffer level */
#define buf_write_port_close()      mmu_write_port_close()


#include "buffer/buffer_msg.h"

#endif /* BUFFER_H */
