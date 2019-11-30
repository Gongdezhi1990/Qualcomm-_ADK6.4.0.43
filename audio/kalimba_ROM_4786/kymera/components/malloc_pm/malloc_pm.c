/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 *
 *
 ****************************************************************************
 * \file malloc_pm.c
 * PM memory allocation/free functionality
 *
 * MODULE : malloc_pm
 *
 * \ingroup malloc_pm
 *
 ****************************************************************************/


/****************************************************************************
Include Files
*/
#include "malloc_pm_private.h"
#include "patch.h"

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Public Function Definitions
*/

/**
 * NAME
 *   init_malloc_pm
 *
 * \brief Initialise memory heap
 *
 * \param[in] reservedSize number of bytes (see notes for what that means) reserved for
 * other systems (e.g. patchpoints) to be subtracted from P0's heap
 *
 *
 * \note
 *   The reservedSize is specified in units of the smallest addressable storage unit
 *   in PM on the processor used. This is 8 bits on KAL_ARCH4 and KAL_ARCH5 or 32 bits
 *   on KAL_ARCH3.
 *
 */
void init_malloc_pm(unsigned int reservedSize)
{
    init_heap_pm(reservedSize);
}

/**
 * NAME
 *   xpmalloc_pm
 *
 * \brief PM memory allocation, based on heap (does not panic, does not zero)
 *
 * FUNCTION
 *   Allocate a chunk of memory from the heap. Returns a Null pointer if not
 *   enough memory is available. The memory is not initialised.
 *
 * \param[in] numBytes number of bytes required. See NOTES for details of what
 *  a byte is (its not necessarily 8 bits!)
 *
 * \param[in] preference_core core in which we want to allocate the memory
 *
 * \return pointer to the block of memory allocated
 *
 * \note
 *   Here a "byte" is the smallest addressable storage unit in PM on the
 *   processor used. This is 8 bits on KAL_ARCH4 and KAL_ARCH5 or 32 bits
 *   on KAL_ARCH3.
 *
 */
void_func_ptr xpmalloc_pm(unsigned int numBytes, unsigned int preference_core)
{
    /* xpmalloc_pm must return Null if requested to allocate Zero bytes */
    if(numBytes <= 0)
    {
        return(NULL);
    }
    return heap_alloc_pm(numBytes, preference_core);
}


/**
 * NAME
 *   pmalloc_pm
 *
 * \brief Memory allocation (panics if no memory)
 *
 * FUNCTION
 *   As xpmalloc_pm, except that it panics if it does not have enough memory to allocate.
 *
 * \param[in] numBytes number of bytes required.
 * See xpmalloc_pm for details of what a "byte" is.
 *
 * \param[in] preference_core core in which we want to allocate the memory
 *
 * \return pointer to the block of memory allocated
 *
 */
void_func_ptr pmalloc_pm(unsigned int numBytes, unsigned int preference_core)
{
    /* [p]malloc must panic if requested to allocate Zero bytes */
    if(numBytes == 0)
    {
        panic(PANIC_AUDIO_REQ_ZERO_MEMORY);
    }
    void_func_ptr ptr = xpmalloc_pm(numBytes, preference_core);
    /* panic out of memory */
    if(ptr == NULL)
    {
        panic_diatribe(PANIC_AUDIO_HEAP_EXHAUSTION, numBytes);
    }

    return ptr;
}



/**
 * NAME
 *   free_pm
 *
 * \brief free memory allocated using versions of malloc (does not panic if passed Null)
 *
 * FUNCTION
 *   Returns previously allocated memory to the PM memory heap.
 *   NOTE requires that the memory just above the top of the
 *   buffer pointed to by pMemory contains the block header information.
 *   This will be the case assuming the memory was allocated using xpmalloc_pm
 *   and there has been not been memory corruption
 *
 * \param[in] pMemory pointer to the memory to be freed
 *
 */
void free_pm(void_func_ptr pMemory)
{
    patch_fn_shared(malloc_pm);
    if(pMemory == NULL)
    {
        return;
    }
    PL_PRINT_P1(TR_PL_FREE,"freeing PM memory %lx\n",(uintptr_t)pMemory);
    if (is_in_pm_heap(pMemory))
    {
        PL_PRINT_P0(TR_PL_FREE,"freeing PM memory from heap\n");
        heap_free_pm(pMemory);
        return;
    }
}

/**
 * NAME
 *   sizeof_pm
 *
 * \brief get the actual size of a dynamically-allocated block
 *
 * FUNCTION
 *   Looks up which memory pool this block cames from, and returns
 *   the block size for the pool.
 *   NOTE requires that the memory just above the top of the
 *   buffer pointed to by pMemory contains the block header information.
 *   This will be the case assuming the memory was allocated using xpmalloc
 *   and there has been not been memory corruption
 *
 * \param[in] pMemory pointer to the block to get the size of
 *
 */
int sizeof_pm(void_func_ptr pMemory)
{
    patch_fn_shared(malloc_pm);
    if(pMemory == NULL)
    {
        return 0;
    }

    if (is_in_pm_heap(pMemory))
    {
        return heap_sizeof_pm(pMemory);
    }
    return 0;
}



