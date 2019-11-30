/****************************************************************************
 * Copyright (c) 2011 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file cbuffer.c
 * \ingroup buffer
 *
 * cbuffer 'C' code.
 */

/****************************************************************************
Include Files
*/

#include "buffer_private.h"
#include "platform/pl_trace.h"


/****************************************************************************
Public Function Definitions
*/
/****************************************************************************
 *
 * cbuffer_create - used to create SW buffers
 *
 * The user is responsible for allocating the buffer data space
 *
 * Input arguments:
 *      cbuffer_data_ptr    - pointer to externally allocated memory to hold the cbuffer data for SW buffers
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor)
{
    tCbuffer *cbuffer_struc_ptr;
    patch_fn_shared(cbuffer_create);

    cbuffer_struc_ptr = xzpnew(tCbuffer);
    if (cbuffer_struc_ptr != NULL)
    {
        cbuffer_buffer_configure(cbuffer_struc_ptr, cbuffer_data_ptr, buffer_size, descriptor);
    }

    PL_PRINT_P0(TR_CBUFFER, "cbuffer_create: ");
    PL_PRINT_BUFFER(TR_CBUFFER, cbuffer_struc_ptr);
    return cbuffer_struc_ptr;
}

/****************************************************************************
 *
 * cbuffer_create_with_malloc_preference - used to create SW buffers
 *
 * The user is responsible for allocating the buffer data space with malloc preference
 * for cbuffer and the underlying buffer resources.
 *
 * Input arguments:
 *      cbuffer_data_ptr    - pointer to externally allocated memory to hold the cbuffer data for SW buffers
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 *      cbuf_preference     - malloc preference for cbuffer
 *      malloc_preference   - preference for the underlying buffer resource.
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */

tCbuffer *cbuffer_create_with_malloc_preference(unsigned int buffer_size, unsigned int descriptor, unsigned cbuf_preference, unsigned malloc_preference)
{
    tCbuffer *cbuffer_struc_ptr;
    int *cbuffer_data_ptr;

    patch_fn_shared(cbuffer_create);

    cbuffer_struc_ptr = xzppnew(tCbuffer, cbuf_preference);
    if (cbuffer_struc_ptr == NULL)
    {
        return NULL;
    }

    cbuffer_data_ptr = (int*) xppmalloc(sizeof(int)*buffer_size, malloc_preference);
    if(!cbuffer_data_ptr)
    {
        pdelete(cbuffer_struc_ptr);
        return NULL;
    }
    else
    {
        cbuffer_buffer_configure(cbuffer_struc_ptr, cbuffer_data_ptr, buffer_size, descriptor);
        PL_PRINT_P0(TR_CBUFFER, "cbuffer_create_with_malloc: ");
        PL_PRINT_BUFFER(TR_CBUFFER, cbuffer_struc_ptr);
        return cbuffer_struc_ptr;
    }
}

/****************************************************************************
 *
 * cbuffer_create_with_malloc - used to create SW buffers
 *
 * As the name suggests the buffer data space will be allocated inside the function
 * This function allocates memory from SLOW ram if it exists on the device.
 *
 * Input argument
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_with_malloc(unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_malloc_preference(buffer_size, descriptor, MALLOC_PREFERENCE_NONE, MALLOC_PREFERENCE_NONE);
}

/****************************************************************************
 *
 * cbuffer_create_with_malloc - used to create SW buffers
 *
 * As the name suggests the buffer data space will be allocated inside the function
 * This function allocates memory from FAST ram if it exists on the device.
 *
 * Input argument
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_with_malloc_fast(unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_malloc_preference(buffer_size, descriptor, MALLOC_PREFERENCE_NONE, MALLOC_PREFERENCE_FAST);
}

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)

/****************************************************************************
 *
 * cbuffer_create_shared_with_malloc - used to create SW buffers
 *
 * As the name suggests the buffer data space will be allocated inside the function
 * This function allocates memory with default preference but the
 * cbuffer from shared ram if exists.
 *
 * Input argument
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_shared_with_malloc(unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_malloc_preference(buffer_size, descriptor, MALLOC_PREFERENCE_SHARED, MALLOC_PREFERENCE_NONE);
}

/****************************************************************************
 *
 * cbuffer_create_shared_with_malloc_fast - used to create SW buffers
 *
 * As the name suggests the buffer data space will be allocated inside the function
 * This function allocates memory from FAST ram if it exists on the device and the
 * cbuffer from shared ram if exists.
 *
 * Input argument
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_shared_with_malloc_fast(unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_malloc_preference(buffer_size, descriptor, MALLOC_PREFERENCE_SHARED, MALLOC_PREFERENCE_FAST);
}

/****************************************************************************
 *
 * cbuffer_create_shared_with_malloc_shared - used to create SW buffers
 *
 * As the name suggests the buffer data space will be allocated inside the function
 * This function allocates memory from shared ram if it exists on the device and
 * the  cbuffer from shared ram if exists.
 *
 * Input argument
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_shared_with_malloc_shared(unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_malloc_preference(buffer_size, descriptor, MALLOC_PREFERENCE_SHARED, MALLOC_PREFERENCE_SHARED);
}

/****************************************************************************
 *
 * cbuffer_create_with_cbuf_preference - used to create SW buffers
 *
 * The user is responsible for allocating the buffer data space with malloc preference
 * for cbuffer and the underlying buffer resources.
 *
 * Input arguments:
 *      cbuffer_data_ptr    - pointer to externally allocated memory to hold the cbuffer data for SW buffers
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 *      cbuf_preference     - malloc preference for cbuffer
 *
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_with_cbuf_preference(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor, unsigned cbuf_preference)
{
    tCbuffer *cbuffer_struc_ptr;

    patch_fn_shared(cbuffer_create);

    cbuffer_struc_ptr = xzppnew(tCbuffer, cbuf_preference);
    if (cbuffer_struc_ptr != NULL)
    {
        cbuffer_buffer_configure(cbuffer_struc_ptr, cbuffer_data_ptr, buffer_size, descriptor);
    }

    PL_PRINT_P0(TR_CBUFFER, "cbuffer_create_with_cbuf_preference: ");
    PL_PRINT_BUFFER(TR_CBUFFER, cbuffer_struc_ptr);
    return cbuffer_struc_ptr;
}

/****************************************************************************
 *
 * cbuffer_create_with_shared_cbuf - used to create SW buffers
 *
 * This function allocates cbuf struct with shared memory preference. No
 * data buffer is allocated, instead the parameter 'cbuffer_data_ptr' is used.
 *
 * Input argument
 *      cbuffer_data_ptr    - pointer to externally allocated memory to hold the
 *                            cbuffer data for SW buffers
 *      buffer_size         - in words
 *      descriptor          - the bit field cbuffer descriptor.
 * Return value
 *      If successful - pointer to the created cbuffer structure else NULL
 *
 */
tCbuffer *cbuffer_create_with_shared_cbuf(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor)
{
    return cbuffer_create_with_cbuf_preference(cbuffer_data_ptr, buffer_size, descriptor, MALLOC_PREFERENCE_SHARED);
}

/****************************************************************************
 *
 * cbuffer_buffer_sync - Synchronise two cbuffers that use the same data buffer
 *
 * This function synchronises two cbuffers, so that the read and the write
 * pointers are identical. It is used for dual-core cbuffers, where there
 * is a cbuf struct in private memory (in one cbuffer) and a cbuf struct
 * in shared memory (in another cbuffer) which both use the same data buffer.
 * The shared memory cbuf is used for multi-cpu access, where one cpu reads
 * and the other writes the cbuffer, and vice versa.
 *
 * Input argument
 *     dst_cbuffer  - destination cbuffer structure
 *     src_buffer   - source cbuffer structure
 *
 * Return value - TRUE if successful, FALSE upon error
 *
 *
 */
bool cbuffer_buffer_sync(tCbuffer *dst_cbuffer, tCbuffer *src_cbuffer)
{
    unsigned int retval = FALSE;

    patch_fn_shared(cbuffer_create);

    if ((dst_cbuffer != NULL) && (src_cbuffer != NULL))
    {
        if (dst_cbuffer->base_addr == src_cbuffer->base_addr)
        {
            dst_cbuffer->write_ptr = src_cbuffer->write_ptr;
            src_cbuffer->read_ptr  = dst_cbuffer->read_ptr;
            retval = TRUE;
        }
    }

    return retval;
}

#endif /* INSTALL_DUAL_CORE_SUPPORT) || AUDIO_SECOND_CORE */


/****************************************************************************
 *
 * cbuffer_buffer_configure - Configure a cbuffer
 */
extern void cbuffer_buffer_configure (tCbuffer *cbuffer, int *buffer,
                         unsigned int buffer_size, unsigned int descriptor)
{
    patch_fn_shared(cbuffer_create);

    cbuffer->base_addr = buffer;
    cbuffer->read_ptr = buffer;
    cbuffer->write_ptr = buffer;
    cbuffer->aux_ptr = NULL;
    cbuffer->descriptor = descriptor;
    cbuffer->size = buffer_size*sizeof(int);    /* buffer module internal sizes in memory allocation units */

#ifdef INSTALL_METADATA
    if (BUFF_METADATA(cbuffer))
    {
        /* in case this is a re-configure, we must preserve
            a previous setting of metadata->buffer_size (in octets) */
        if (buff_metadata_get_buffer_size(cbuffer) != 0)
        {
            /* might not use all octets in a word */
            unsigned usable_octets;
            usable_octets = buff_metadata_get_usable_octets( cbuffer);
            /* up to 4 octets (32-bit words) */
            PL_ASSERT(usable_octets <= 4);
            /* set usable_octets and adjust the cached buffer_size */
            buff_metadata_set_usable_octets(cbuffer, usable_octets);
        }
    }
#endif     /* INSTALL_METADATA */
}


/****************************************************************************
 *
 * cbuffer_destroy - destroys buffers created by cbuffer_create and cbuffer_wrap functions
 *
 * Input argument:
 *      cbuffer   - pointer to a cbuffer structure to destroy
 *
 * Note
 *    If any of the read/write pointers point to a local MMU buffer handle the
 *    MMU buffer will be destroyed first.
 *
 */
void cbuffer_destroy(tCbuffer *cbuffer)
{
    int* buff_to_free = NULL;

    patch_fn_shared(cbuffer_create);

    if (cbuffer == NULL)
    {
        return;
    }

    if (BUF_DESC_IS_REMOTE_MMU(cbuffer->descriptor))
    {
        /* Nothing to do here */
    }
    else if (!BUF_DESC_BUFFER_TYPE_MMU(cbuffer->descriptor))
    {
        /* Pure SW buffer, use the base address to identify it */
        buff_to_free = cbuffer->base_addr;
    }
#ifdef CHIP_BASE_HYDRA
    else
    {
        /* MMU buffer, could be RD, WR, AUX handle */
        mmu_release_handle(cbuffer);
        buff_to_free = cbuffer->base_addr;
    }
#endif
#ifdef CHIP_BASE_NAPIER
    {
        // /* Nothing to do here */ --> AUDIO_SS is not owner of buffer
    }
#endif
    /* Free the buffer memory. */
    /* TODO: We might want to panic if buff_to_free is NULL at this point
     * (indicates an error releasing the handle) */
    pfree(buff_to_free);

    cbuffer_destroy_struct(cbuffer);

    return;
}


/****************************************************************************
 * cbuffer_destroy_struct
 */
void cbuffer_destroy_struct(tCbuffer *cbuffer)
{
    patch_fn_shared(cbuffer_create);

#ifdef INSTALL_METADATA
    /* The buffer may have metadata so make sure that gets freed */
    buff_metadata_release(cbuffer);
#endif /* INSTALL_METADATA */

    /* In all cases, free the memory for the cbuffer structure. */
    pdelete(cbuffer);
}


/****************************************************************************
 * cbuffer_scratch_commit_update
 */
void cbuffer_scratch_commit_update(tCbuffer *cbuffer, void *scratch_mem)
{
    /* Initialise to empty as the content of the scratch buffer is random */
    cbuffer->base_addr = scratch_mem;
    cbuffer->read_ptr = scratch_mem;
    cbuffer->write_ptr = scratch_mem;
}


/****************************************************************************
 *
 * cbuffer_get_size - Get the size of a cbuffer
 *
 * Input arguments:
 *      cbuffer   - pointer to a cbuffer structure to read from
 *
 * Return value
 *      Size of the buffer
 *          in words (cbuffer_get_size_in_words)
 *          or
 *          in addresses | locations (cbuffer_get_size_in_addrs)
 *          or
 *          in octets (cbuffer_get_size_in_octets)
 *
 * Note
 *     On 24-bit DSPs this function returns the same value for words & addrs.
 */
unsigned int cbuffer_get_size_in_words(tCbuffer *cbuffer)
{
    return ((cbuffer->size)>>LOG2_ADDR_PER_WORD);
}
unsigned int cbuffer_get_size_in_addrs(tCbuffer *cbuffer)
{
    return cbuffer->size;
}

unsigned int cbuffer_get_size_in_octets(tCbuffer *cbuffer)
{
#ifdef KAL_ARCH4
    /* On ARCH4 this is the same as addressable units */
    return cbuffer->size;
#else /* KAL_ARCH4 */
    /* On 24bit platforms there are 3 octets per addressable unit */
    return cbuffer->size * OCTETS_PER_SAMPLE;
#endif /* KAL_ARCH4 */
}

#if defined(INSTALL_METADATA) && !defined(DESKTOP_TEST_BUILD)
/**
 * cbuffer_empty_buffer_and_metadata
 * \brief clearing cbuffer and its associated metadata
 * \param cbuffer input buffer
 */
void cbuffer_empty_buffer_and_metadata(tCbuffer *cbuffer)
{
    if(buff_has_metadata(cbuffer))
    {
        /* buffer has metadata, metadata and actual data need
         * to be cleared syncronously
         */
        LOCK_INTERRUPTS;
        if(cbuffer->metadata->tags.head != NULL)
        {
            buff_metadata_tag_list_delete(cbuffer->metadata->tags.head);
            cbuffer->metadata->tags.head = NULL;
            cbuffer->metadata->tags.tail = NULL;
        }
        cbuffer->metadata->prev_rd_index = cbuffer->metadata->prev_wr_index;
        cbuffer_empty_buffer(cbuffer);
        UNLOCK_INTERRUPTS;
    }
    else
    {
        /* no metadata, just empty data buffer */
        cbuffer_empty_buffer(cbuffer);
    }
}
#endif  /* INSTALL_METADATA */

/****************************************************************************/
#ifdef PL_TRACE_PRINTF
#include <stdio.h>
void cbuffer_print(tCbuffer *cbuffer)
{
    printf("Buffer %p: {base %p, size %d, ", cbuffer, cbuffer->base_addr, cbuffer->size);

    /* Ignore the aux handle; in fact we might have already used it
     * in the calls to get_*_offset above */

    printf("descriptor %06x} \n", cbuffer->descriptor);
}
#endif

