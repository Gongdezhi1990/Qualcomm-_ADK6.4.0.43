/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup buffer Buffer Subsystem
 * \file  buffer_ipc.h
 * Public buffer subsystem header file for multi core 
 * \ingroup buffer
 *
 */

#ifndef BUFFER_IPC_H
#define BUFFER_IPC_H

/****************************************************************************
Include Files
*/

#include "buffer.h"

#if defined(INSTALL_DUAL_CORE_SUPPORT) || defined (AUDIO_SECOND_CORE)
/****************************************************************************
Public Function Declarations
*/

/**
 * Create a SW cbuffer from shared ram with malloc.
 *
 * This function mallocs a buffer of buffer_size words big for the
 * cbuffer data storage,
 * It also creates a tCbuffer object from the shared ram and populates its fields.
 *
 * \param buffer_size is the requested size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create_shared_with_malloc(unsigned int buffer_size, unsigned int descriptor);


/**
 * Create a SW cbuffer from shared ram with malloc from shared ram.
 *
 * This function mallocs a buffer of buffer_size words big for the
 * cbuffer data storage,
 * creates a tCbuffer object and populates its fields.
 * This function allocates memory from shared ram if it exists on the device.
 *
 * \param buffer_size is the requested size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create_shared_with_malloc_shared(unsigned int buffer_size, unsigned int descriptor);

/**
 * Create a SW cbuffer from shared ram with malloc from fast ram 
 *
 * This function mallocs a buffer buffer_size words big for the cbuffer
 * data storage,
 * creates a tCbuffer object and populates its fields.
 * This function allocates memory from shared and fast ram if it exists 
 * on the device.
 *
 * \param buffer_size is the requested size of the buffer in words.
 * \param descriptor is the bit field cbuffer descriptor.
 * \return pointer to tCbuffer object or NULL if unsuccessful. Could panic.
 */
extern tCbuffer *cbuffer_create_shared_with_malloc_fast(unsigned int buffer_size, unsigned int descriptor);

/**
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
extern tCbuffer *cbuffer_create_with_cbuf_preference(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor, unsigned cbuf_preference);

/**
 *
 * cbuffer_create_with_shared_cbuf - used to create SW buffers
 *
 * This function allocates cbuf struct with shared memory preference. No
 * data buffer is allocated, instead the parameter 'cbuffer_data_ptr' is used.
 *
 * Input argument
 * \param  cbuffer_data_ptr    - pointer to externally allocated memory to hold the
 *                               cbuffer data for SW buffers
 * \param  buffer_size         - in words
 * \param  descriptor          - the bit field cbuffer descriptor.
 *
 * \return If successful pointer to the created cbuffer structure else NULL
 *
 */
extern tCbuffer *cbuffer_create_with_shared_cbuf(void *cbuffer_data_ptr, unsigned int buffer_size, unsigned int descriptor);

/**
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
 * \param dst_cbuffer  - destination cbuffer structure
 * \param src_buffer   - source cbuffer structure
 *
 * \return TRUE if successful, FALSE upon error
 *
 */
extern bool cbuffer_buffer_sync(tCbuffer *dst_cbuffer, tCbuffer *src_cbuffer);

#endif /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
#endif /* IPC_BUFFER_H */
