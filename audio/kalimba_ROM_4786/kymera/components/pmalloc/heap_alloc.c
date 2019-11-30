/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ****************************************************************************
 * \file heap_alloc.c
 * \ingroup pl_malloc
 *
 * Memory allocation/free functionality
 *
 ****************************************************************************/
/****************************************************************************
Include Files
*/

#include "pl_malloc_private.h"
#if !defined(__GNUC__) && defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD)
#include "mib/mib.h"
#endif

/****************************************************************************
Private Configuration Macro Declarations
*/
/* Heap overflow to free ram is disabled with GCC */
#if defined(__GNUC__)
#ifdef HEAP_OVERFLOW_TO_FREE_RAM
#undef HEAP_OVERFLOW_TO_FREE_RAM
#endif
#endif
/*
 * Directly accessing the linker provided symbol
 * as an address and typecasting to integer to make
 * the compiler happy
 */
#define HEAP_SIZE(x) ((unsigned)((uintptr_t)&(x)))

/*
 * If INSTALL_DYNAMIC_HEAP is not defined, the heap sizes are
 * initialised to the build time provided configuration of the fixed
 * heap size. INSTALL_DYNAMIC_HEAP allows to dynamically increase
 * the heap size in heap_config() API after reading the MIB keys
 *
 * HEAP_OVERFLOW_TO_FREE_RAM allows heap to claim the free space
 * beyond the fixed heap array size.
 *
 * while writing this,  INSTALL_DYNAMIC_HEAP is supported  only
 * for hydra platforms and  HEAP_OVERFLOW_TO_FREE_RAM is supported
 * for Crescendo ( will support on Stre too).
 */

/******************************* dm1 heap configuration  ******************************/
#ifdef HEAP_SIZE_DM1
/* heap on DM1. */
DM1_HEAP_LOCATION char          heap1[HEAP_SIZE_DM1];
/* Linker provides the calculated heapsize after
 * calculating the available free RAM.*/
extern unsigned                 _heap1_size;
#define HEAP1_MAX_SIZE          HEAP_SIZE(_heap1_size)
extern unsigned                 _single_mode_heap1_size;
#define HEAP1_MAX_SIZE_SINGLE_MODE  \
                                HEAP_SIZE(_single_mode_heap1_size)

#define INVALID_HEAP_DM1_SIZE(x) ((x) < HEAP_SIZE_DM1)

#else  /* HEAP_SIZE_DM1*/
#define heap1                   ((char*)NULL)
#define HEAP_SIZE_DM1           0
#define HEAP1_MAX_SIZE          0
#define HEAP1_MAX_SIZE_SINGLE_MODE 0
#define INVALID_HEAP_DM1_SIZE(x) FALSE

#endif /* HEAP_SIZE_DM1*/



/******************************** dm2 heap configuration  *****************************/
#ifdef HEAP_SIZE_DM2
DM2_HEAP_LOCATION char          heap2[HEAP_SIZE_DM2];
/* Linker provides the calculated heapsize after
 * calculating the available free RAM.*/
extern unsigned                 _heap2_size;
#define HEAP2_MAX_SIZE          HEAP_SIZE(_heap2_size)
extern unsigned                 _single_mode_heap2_size;
#define HEAP2_MAX_SIZE_SINGLE_MODE  \
                                HEAP_SIZE(_single_mode_heap2_size)

#define INVALID_HEAP_DM2_SIZE(x) ((x) < HEAP_SIZE_DM2)

#else  /* HEAP_SIZE_DM2 */
#define heap2                   ((char*)NULL)
#define HEAP_SIZE_DM2           0
#define HEAP2_MAX_SIZE          0
#define HEAP2_MAX_SIZE_SINGLE_MODE 0
#define INVALID_HEAP_DM2_SIZE(x)  FALSE

#endif /* HEAP_SIZE_DM2 */


/**************************** shared heap configuration *******************************/
#if (defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)) && !defined(UNIT_TEST_BUILD)  && !defined(__GNUC__) 
DM2_SHARED_HEAP_LOCATION char   heap3[HEAP_SIZE_SHARED_MAX/2];
/* Linker provides the calculated heapsize after
 * calculating the available free RAM.*/
extern unsigned                 _heap3_size;
#define HEAP3_MAX_SIZE          HEAP_SIZE(_heap3_size)
extern unsigned                 _single_mode_heap3_size;
#define HEAP3_MAX_SIZE_SINGLE_MODE  \
                                HEAP_SIZE(_single_mode_heap3_size)
/* Set the minim shared heap size to half of the max heap size to enable 
 * evenly distributing the memory between cores. */
#define HEAP_SIZE_SHARED        (HEAP3_MAX_SIZE / 2)

#else  /* (defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)) && !defined(UNIT_TEST_BUILD) */
/* Undefine the HEAP_SIZE_SHARED for unit tests. A normal config should not redefines this! */
#if defined(UNIT_TEST_BUILD)||defined(RUNNING_ON_KALSIM)
#undef HEAP_SIZE_SHARED
#endif

#define heap3                   ((char*)NULL)
#define HEAP_SIZE_SHARED        0
#define HEAP3_MAX_SIZE          0
#define HEAP3_MAX_SIZE_SINGLE_MODE 0

#endif /* (defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)) && !defined(UNIT_TEST_BUILD) */


/************************ Extended DM1 configuration **********************************/
#ifdef HEAP_SIZE_DM1_EXT
DM1_EXT_HEAP_LOCATION char      heap4[HEAP_SIZE_DM1_EXT];
/* Linker provides the calculated heapsize after
 * calculating the available free RAM.*/
extern unsigned                 _heap4_size;
#define HEAP4_MAX_SIZE          HEAP_SIZE(_heap4_size)
extern unsigned                 _single_mode_heap4_size;
#define HEAP4_MAX_SIZE_SINGLE_MODE  \
                                HEAP_SIZE(_single_mode_heap4_size)

#define INVALID_HEAP_DM1_EXT_SIZE(x) ((x) < HEAP_SIZE_DM1_EXT)

#else  /* HEAP_SIZE_DM1_EXT */
#define heap4                   ((char*)NULL)
#define HEAP_SIZE_DM1_EXT       0
#define HEAP4_MAX_SIZE          0
#define HEAP4_MAX_SIZE_SINGLE_MODE 0
#define INVALID_HEAP_DM1_EXT_SIZE(x)  FALSE

#endif /* HEAP_SIZE_DM1_EXT */


/******* Remove linker references if HEAP_OVERFLOW_TO_FREE_RAM is not active **********/
#if !defined(HEAP_OVERFLOW_TO_FREE_RAM)
#undef HEAP1_MAX_SIZE
#undef HEAP2_MAX_SIZE
#undef HEAP3_MAX_SIZE
#undef HEAP4_MAX_SIZE

#define HEAP1_MAX_SIZE          HEAP_SIZE_DM1
#define HEAP2_MAX_SIZE          HEAP_SIZE_DM2
#define HEAP3_MAX_SIZE          HEAP_SIZE_SHARED
#define HEAP4_MAX_SIZE          HEAP_SIZE_DM1_EXT

#endif

/****************************************************************************
Private Macro Declarations
*/

#define MAGIC_WORD 0xabcd01ul

#ifdef PMALLOC_DEBUG
#define HEAP_GUARD_WORD 0x987654ul
#define GUARD_SIZE (sizeof(unsigned int))
#else
#define GUARD_SIZE 0
#endif

#define MIN_SPARE 8

#define KIBYTE (1024)

/* If DM2 free heap is below this level, don't use it 
 * as a prefered heap  unless request with DM2 preference
 */
#define DM2_RESERVE_HEAP_WATERMARK 0x1000

/* Verify some assumptions and refactoring */
#if !defined(__GNUC__) && !defined(RUNNING_ON_KALSIM) && !defined(UNIT_TEST_BUILD)
STATIC_ASSERT(HEAP_NUM_PROCESSORS==CHIP_NUM_CORES,HEAP_NUM_PROCESSORS_equals_CHIP_NUM_CORES);
#endif

/****************************************************************************
Private Type Declarations
*/
typedef struct mem_node
{
    unsigned length;
    union
    {
        struct mem_node *next;
        unsigned magic;
    } u;
#ifdef PMALLOC_DEBUG
    const char *file;
    unsigned int line;
    unsigned int guard;
#endif
} mem_node;

/**
 * Type definition for heap allocation mib key values.
 */
typedef struct heap_allocation_mib_value
{
    uint8 p0_heap1_size;
    uint8 p0_heap2_size;
    uint8 p0_heap4_size;
    uint8 p1_heap1_size;
    uint8 p1_heap2_size;
    uint8 p1_heap4_size;
} heap_allocation_mib_value;

/**
 * Heap sizes for a processor.
 */
typedef struct heap_sizes
{
    unsigned int heap_size[HEAP_ARRAY_SIZE];
} heap_sizes;

/**
 * Dynamic heap configuration for the chip.
 */
typedef struct heap_dyn_size_config
{
    /* Processor */
    heap_sizes p[HEAP_NUM_PROCESSORS];
} heap_dyn_size_config;

/****************************************************************************
Private Function Declarations
*/
#ifndef __GNUC__
#if defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD)

static bool get_heap_allocation(heap_dyn_size_config* value);

#endif /* defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD) */

static void heap_configure_and_align(
    heap_config *main_core,
    heap_config *second_core,
    heap_sizes* heap_max_sizes,
    bool have_dyn_config,
    heap_dyn_size_config *dyn_heap_config,
    mem_node **freelist);

static inline char* round_up_to_dm_bank_addr(char* address);
static inline char* round_down_to_dm_bank_addr(char* address);
#endif /* __GNUC__ */

/****************************************************************************
Private Variable Definitions
*/

/* Free list and minimum heap arrays
 * Minimum heap array will be initialised at pmalloc_init() by P0
 * pmalloc_config() will extend the heap beyond this array if free
 * memory availoable as per the memory map.
 * heap boundaries are externally visible only for the minimum heap sizes.
 * For the expanded heap boundaries after configuration needs to be calculated
 * using the configured sizes.
 */
static mem_node *freelist[HEAP_ARRAY_SIZE];

/* The heap info is shared between P0 and P1 if IPC installed */
DM_SHARED_ZI heap_config  processor_heap_info_list[HEAP_NUM_PROCESSORS];

/* local pointer to the processor copy of heap_config */
static heap_config *pheap_info;


#ifdef HEAP_DEBUG
/* Total free memory, number of blocks on the free list and minimum number of free
 * blocks (this intended for getting memory watermarks) */
unsigned heap_debug_free, heap_debug_freenodes, heap_debug_min_free;
/* Total allocated memory and number of allocated blocks */
unsigned heap_debug_alloc, heap_debug_allocnodes;
#endif

/****************************************************************************
Private Function Definitions
*/
/**
 * NAME
 *   get_heap_num
 *
 * \brief Returns which heap the pointer points to. Panics if memory is not in the heap.
 *
 */
static unsigned get_heap_num(void *ptr)
{
    heap_info *heap_cfg = pheap_info->heap;
    unsigned heap_num;
    for (heap_num = 0; heap_num < HEAP_ARRAY_SIZE; heap_num++)
    {
        if ((heap_cfg[heap_num].heap_start <= (char*)ptr) && ((char*)ptr < heap_cfg[heap_num].heap_end))
        {
            return heap_num;
        }
    }

    PL_PRINT_P0(TR_PL_FREE, "Couldn't find anywhere\n");
    panic_diatribe(PANIC_AUDIO_FREE_INVALID, (DIATRIBE_TYPE)((uintptr_t)ptr));
#ifdef USE_DUMMY_PANIC
    /* panic_diatribe DoesNotReturn in standard builds, but it does for
     * some test builds where this is defined. */
    return INVALID_HEAP;
#endif
}

/**
 * NAME
 *   is_dm2_heap_loaded
 *
 * \brief Returns TRUE if DM2 heap has more heap space that DM1 heap
 *
 */
static inline bool is_dm2_heap_loaded(void)
{

    /* If DM2 space - reserved space is less than DM1 space
     * it returns TRUE suggesting that DM2 is loaded more
     * This reserved space is kept to satisfy any requests
     * for DM2 specific requests
     */
   
    return (( pheap_info->heap[HEAP_DM2].heap_free <  
               DM2_RESERVE_HEAP_WATERMARK ) ||
             (pheap_info->heap[HEAP_DM2].heap_free <  
             (pheap_info->heap[HEAP_DM1_EXT].heap_free + 
              pheap_info->heap[HEAP_DM1].heap_free )))? TRUE: FALSE;
    
}


/**
 * NAME
 *   set_heap_info
 *
 * \brief Set the heap information which consist of start, end address and size.
 *
 */
static void set_heap_info(heap_info* heap, char* start, char* end, unsigned size)
{
    PL_ASSERT(start + size == end);
    heap->heap_start = start;
    heap->heap_size = size;
    heap->heap_end = end;

    /* set heap_free = 0, It will be initialised while initialising nodes */
    heap->heap_free = 0;
}

/**
 * NAME
 *   init_heap_node
 *
 * \brief Initialise the heap mem node
 *
 */
static mem_node* init_heap_node(char* heap, unsigned heap_size)
{
    unsigned freelength, heap_num;
    mem_node *node = (mem_node *)heap;

    if (( heap_size <= sizeof(mem_node) ) || heap == NULL)
    {
        return NULL;
    }

    freelength = heap_size - sizeof(mem_node);
    node->u.next = NULL;
    node->length = freelength;

    /* find out the location of the free node */
    heap_num = get_heap_num((void*) heap);
    pheap_info->heap[heap_num].heap_free += freelength;

#ifdef HEAP_DEBUG
    heap_debug_free += freelength - GUARD_SIZE;
    heap_debug_freenodes++;
#endif

    return node;
}

/**
 * NAME
 *   allocate memory internally
 *
 * \brief internal call to allocate memory
 *
 */
static void *heap_alloc_internal(unsigned size, unsigned heap_num)
{
    mem_node **pbest = NULL;
    mem_node **pnode = &freelist[heap_num];
    unsigned bestsize = pheap_info->heap[heap_num].heap_size;

    /* Round up size to the nearest whole word */
    size = ROUND_UP_TO_WHOLE_WORDS(size) + GUARD_SIZE;

    /* Do all the list-traversal and update with interrupts blocked */
    LOCK_INTERRUPTS;

    /* Traverse the list looking for the best-fit free block
     * Best fit is the smallest one of at least the requested size
     * This will help to minimise wastage
     */
    while (*pnode != NULL)
    {
        if (((*pnode)->length >= size ) && ((*pnode)->length < bestsize))
        {
            pbest = pnode;
            bestsize = (*pnode)->length;
        }
        pnode = &(*pnode)->u.next;
    }
    if (pbest)
    {
        char *addr;
        mem_node *newnode;
        if (bestsize >= size + sizeof(mem_node) + MIN_SPARE)
        {
            /* There's enough space to allocate something else
             * so keep the existing free block and allocate the space at the top
             * In this case the allocation size is exactly what was requested
             */
            addr = (char *)(*pbest) + (*pbest)->length - size;
            (*pbest)->length -= (size + sizeof(mem_node));
        }
        else
        {
            /* Not enough extra space to be useful
             * Replace the free block with an allocated one
             * The allocation size is the whole free block
             */
            addr = (char *)(*pbest);
            size = (*pbest)->length;
            *pbest = (*pbest)->u.next;

#ifdef HEAP_DEBUG
            heap_debug_freenodes--;
            /* This node gets reused. To simplify the logic, temporarily add its size
             * to the available space. It gets taken off again below.
             */
            heap_debug_free += sizeof(mem_node) + GUARD_SIZE;
#endif
            pheap_info->heap[heap_num].heap_free += sizeof(mem_node);

        }
        /* Finally populate the header for the newly-allocated block */
        newnode = (mem_node *)addr;
        newnode->length = size - GUARD_SIZE;
        newnode->u.magic = MAGIC_WORD;
#ifdef HEAP_DEBUG
        heap_debug_allocnodes++;
        heap_debug_alloc += newnode->length;
        heap_debug_free -= (size + sizeof(mem_node));
        if (heap_debug_min_free > heap_debug_free)
        {
            heap_debug_min_free = heap_debug_free;
        }
#endif
        pheap_info->heap[heap_num].heap_free -= size;

        UNLOCK_INTERRUPTS;
        return addr + sizeof(mem_node);
    }
    /* No suitable block found */
    UNLOCK_INTERRUPTS;

    PL_PRINT_P0(TR_PL_MALLOC_FAIL,"heap alloc failed\n");
    return NULL;
}

/**
 * NAME
 *   coalesce_free_mem
 *
 * \brief claim back the free memory
 *
 */
static void  coalesce_free_mem(mem_node **pfreelist, char *free_mem, unsigned len)
{
    mem_node *curnode, **pnode;
    unsigned heap_num;

    /* Do all the list-traversal and update with interrupts blocked */
    LOCK_INTERRUPTS;

    curnode = *pfreelist;

    /* Traverse the free list to see if we can coalesce an
     * existing free block with this one */
    while (curnode != NULL)
    {
        if ( (char *) curnode + curnode->length +
              sizeof(mem_node) == (char *)free_mem)
        {
            /* Matching block found */
            break;
        }
        curnode = curnode->u.next;
    }

    /* find the associated heap number */
    heap_num = get_heap_num(free_mem);

    if (curnode != NULL)
    {
        /* The immediately-previous block is free
         * add the one now being freed to it
         */
        curnode->length += len;
#ifdef HEAP_DEBUG
        heap_debug_free += len;
#endif
        /* update free heap */
        pheap_info->heap[heap_num].heap_free += len;

    }
    else
    {
        /* Previous block wasn't free
         * so add the now-free block to the free list
         * Note length is unchanged from when it was allocated
         * (unless we have guard words, which get added to the free space)
         */
        curnode = init_heap_node( free_mem, len );

#ifdef PMALLOC_DEBUG
        if (curnode == NULL)
        {
            panic_diatribe(PANIC_AUDIO_FREE_INVALID,
                          (DIATRIBE_TYPE)((uintptr_t)free_mem));
        }
#endif
        /* it should not return NULL */
        curnode->u.next = *pfreelist;
        *pfreelist = curnode;
    }

    /* Now check if there is a free block immediately after the found / new one */
    pnode = pfreelist;
    while (*pnode != NULL)
    {
        if ((char*)(*pnode) == (char*)curnode + curnode->length + sizeof(mem_node))
        {
            /* Matching block found */
            break;
        }
        pnode = &(*pnode)->u.next;
    }
    if (*pnode != NULL)
    {
        /* The immediately-following block is free
         * add it to the current one and remove from the free list
         */
        curnode->length += (*pnode)->length + sizeof(mem_node);
        *pnode = (*pnode)->u.next;
#ifdef HEAP_DEBUG
        heap_debug_freenodes--;
        heap_debug_free += sizeof(mem_node) + GUARD_SIZE;
#endif
        
        /* update free heap */
        pheap_info->heap[heap_num].heap_free += sizeof(mem_node);

    }
    UNLOCK_INTERRUPTS;
}

#ifndef __GNUC__
#if defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD)
/**
 * NAME
 *   get_heap_allocation
 *
 * \brief Read the heap allocation configuration from MIB
 *
 */
static bool get_heap_allocation(heap_dyn_size_config* value)
{
    int16 len_read;
    heap_allocation_mib_value mib_value;
    memset(&mib_value, 0, sizeof(mib_value));

    len_read = mibgetreqstr(HEAPALLOCATION, (uint8*)&mib_value, sizeof(mib_value));
    if (len_read != sizeof(mib_value))
    {
        return FALSE;
    }

    value->p[HEAP_PROC_0].heap_size[HEAP_DM1] = mib_value.p0_heap1_size * KIBYTE;
    value->p[HEAP_PROC_0].heap_size[HEAP_DM2] = mib_value.p0_heap2_size * KIBYTE;
    /* Set the shared memory size to 0 to avoid any configuration later in
     * heap_dynamic_config. */
    value->p[HEAP_PROC_0].heap_size[HEAP_SHARED] = 0;
    value->p[HEAP_PROC_0].heap_size[HEAP_DM1_EXT] = mib_value.p0_heap4_size * KIBYTE;

    DBG_MEM_CFG_MSG3("\nMib heap configuration for processor 0\n"
            "heap HEAP_DM1 size  =      0x%05x \n"
            "heap HEAP_DM2 size  =      0x%05x \n"
            "heap HEAP_DM1_EXT size  = 0x%05x \n",
            mib_value.p0_heap1_size * KIBYTE,
            mib_value.p0_heap2_size * KIBYTE,
            mib_value.p0_heap4_size * KIBYTE);
#ifdef INSTALL_DUAL_CORE_SUPPORT
    value->p[HEAP_PROC_1].heap_size[HEAP_DM1] = mib_value.p1_heap1_size * KIBYTE;
    value->p[HEAP_PROC_1].heap_size[HEAP_DM2] = mib_value.p1_heap2_size * KIBYTE;
    value->p[HEAP_PROC_1].heap_size[HEAP_SHARED] = 0;
    value->p[HEAP_PROC_1].heap_size[HEAP_DM1_EXT]= mib_value.p1_heap4_size * KIBYTE;

    DBG_MEM_CFG_MSG3("Mib heap configuration for processor 1\n"
            "heap HEAP_DM1 size  =      0x%05x \n"
            "heap HEAP_DM2 size  =      0x%05x \n"
            "heap HEAP_DM1_EXT size  = 0x%05x \n",
            mib_value.p1_heap1_size * KIBYTE,
            mib_value.p1_heap2_size * KIBYTE,
            mib_value.p1_heap4_size * KIBYTE);
#endif /* INSTALL_DUAL_CORE_SUPPORT */

    if (INVALID_HEAP_DM1_SIZE(value->p[HEAP_PROC_0].heap_size[HEAP_DM1]) ||
        INVALID_HEAP_DM1_EXT_SIZE(value->p[HEAP_PROC_0].heap_size[HEAP_DM1_EXT]) ||
        INVALID_HEAP_DM2_SIZE(value->p[HEAP_PROC_0].heap_size[HEAP_DM2]))
    {
        WARN_MEM_CFG_MSG("Heap config: Invalid p0 heap config from mib");
        return FALSE;
    }

#ifdef INSTALL_DUAL_CORE_SUPPORT
    if (INVALID_HEAP_DM1_SIZE(value->p[HEAP_PROC_1].heap_size[HEAP_DM1]) ||
        INVALID_HEAP_DM1_EXT_SIZE(value->p[HEAP_PROC_1].heap_size[HEAP_DM1_EXT]) ||
        INVALID_HEAP_DM2_SIZE(value->p[HEAP_PROC_1].heap_size[HEAP_DM2]))
    {
        WARN_MEM_CFG_MSG("Heap config: Invalid p1 heap config from mib");
        return FALSE;
    }
    if ((value->p[HEAP_PROC_0].heap_size[HEAP_DM1]  + value->p[HEAP_PROC_1].heap_size[HEAP_DM1]  > HEAP1_MAX_SIZE) ||
        (value->p[HEAP_PROC_0].heap_size[HEAP_DM1_EXT]  + value->p[HEAP_PROC_1].heap_size[HEAP_DM1_EXT]  > HEAP4_MAX_SIZE) ||
        (value->p[HEAP_PROC_0].heap_size[HEAP_DM2] + value->p[HEAP_PROC_1].heap_size[HEAP_DM2]  > HEAP2_MAX_SIZE))
    {
        WARN_MEM_CFG_MSG("Heap config: Invalid heap size config from mib");
        return FALSE;
    }
#endif /* INSTALL_DUAL_CORE_SUPPORT */

    return TRUE;
}

/**
 * NAME
 *   heap_set_active_cores
 *
 * \brief Enables/disables cores other than the main core by setting the processor
 *        configuration to NULL.
 *
 *        Note: at the moment only two processors are supported hence the function only
 *        has one input.
 */
static bool heap_overflow_to_free_ram(void )
{
    bool overflow_to_free_ram = mibgetreqbool(HEAPOVERFLOWTOFREERAM);

    DBG_MEM_CFG_MSG1("!!!HEAPOVERFLOWTOFREERAM = %d. ", mibgetrequ16(HEAPOVERFLOWTOFREERAM));

    if (overflow_to_free_ram)
    {
        DBG_MEM_CFG_MSG("Heap overflow to free RAM enabled. ");
    }
    else
    {
        DBG_MEM_CFG_MSG("Heap overflow to free RAM disabled. ");
    }

    return overflow_to_free_ram;
}


#else /* defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD) */
static bool get_heap_allocation(heap_dyn_size_config* value)
{
    UNUSED(value);
    return FALSE;
}
#define heap_overflow_to_free_ram()             (TRUE)
#endif /* defined(INSTALL_DYNAMIC_HEAP) && !defined(UNIT_TEST_BUILD) */

/**
 * NAME
 *   read_heap_max_sizes
 *
 * \brief Sets the maximum heap sizes. 
 * Note: The maximum heap sizes are biger in single core mode.
 */
static void set_heap_max_sizes(bool second_core_enabled, heap_sizes* heap_max_sizes)
{
#ifdef HEAP_SIZE_DM1_MAX
    DBG_MEM_CFG_MSG3("Heap config: Heap 1 size: internal 0x%05x, linker 0x%05x, single mode 0x%05x",
            HEAP_SIZE_DM1_MAX, HEAP1_MAX_SIZE, HEAP1_MAX_SIZE_SINGLE_MODE);
#endif
#ifdef HEAP_SIZE_DM2_MAX
    DBG_MEM_CFG_MSG3("Heap config: Heap 2 size: internal 0x%05x, linker 0x%05x, single mode 0x%05x",
            HEAP_SIZE_DM2_MAX, HEAP2_MAX_SIZE, HEAP2_MAX_SIZE_SINGLE_MODE);
#endif
#ifdef HEAP_SIZE_SHARED_MAX
    DBG_MEM_CFG_MSG3("Heap config: Heap 3 size: internal 0x%05x, linker 0x%05x, single mode 0x%05x",
            HEAP_SIZE_SHARED_MAX, HEAP3_MAX_SIZE, HEAP3_MAX_SIZE_SINGLE_MODE);
#endif
#ifdef HEAP_SIZE_DM1_EXT_MAX
    DBG_MEM_CFG_MSG3("Heap config: Heap 4 size: internal 0x%05x, linker 0x%05x, single mode 0x%05x",
            HEAP_SIZE_DM1_EXT_MAX, HEAP4_MAX_SIZE, HEAP4_MAX_SIZE_SINGLE_MODE);
#endif

    if (second_core_enabled)
    {
        heap_max_sizes->heap_size[HEAP_DM1] =        HEAP1_MAX_SIZE;
        heap_max_sizes->heap_size[HEAP_DM2] =        HEAP2_MAX_SIZE;
        heap_max_sizes->heap_size[HEAP_SHARED] =     HEAP3_MAX_SIZE;
        heap_max_sizes->heap_size[HEAP_DM1_EXT] =   HEAP4_MAX_SIZE;
    }
    else
    {
        heap_max_sizes->heap_size[HEAP_DM1] =        HEAP1_MAX_SIZE_SINGLE_MODE;
        heap_max_sizes->heap_size[HEAP_DM2] =        HEAP2_MAX_SIZE_SINGLE_MODE;
        heap_max_sizes->heap_size[HEAP_SHARED] =     HEAP3_MAX_SIZE_SINGLE_MODE;
        heap_max_sizes->heap_size[HEAP_DM1_EXT] =   HEAP4_MAX_SIZE_SINGLE_MODE;
    }
}

/**
 * NAME
 *   heap_boundary_align
 *
 * \brief Attempt to extend configured heap boundaries to a block boundary.
 */
static void heap_configure_and_align(
    heap_config *main_core,
    heap_config *second_core,
    heap_sizes* heap_max_sizes,
    bool have_dyn_config,
    heap_dyn_size_config *dyn_heap_config,
    mem_node **freelist)
{
    /* The main core (processor 0) configuration data. */
    heap_info* p0_size_info = NULL;
    char* p0_start;
    char* p0_end;
    unsigned p0_size;
    unsigned p0_dyn_size;

    /* The second core (processor 1) configuration data. */
    heap_info* p1_size_info = NULL;
    char* p1_start;
    char* p1_end;
    unsigned p1_size;
    unsigned p1_dyn_size;

    /* Variables used in the for loops.  */
    mem_node** pfreelist;
    unsigned heap_min;
    unsigned current_heap_max_size;
    unsigned heap_num;

    /* Configure and align to memory bank boundary for all the four heaps. */
    for (heap_num = 0; heap_num < HEAP_ARRAY_SIZE; heap_num++)
    {
        /* init_heap set the minimum heap sizes for p0 to enable memory allocation for
         * some services. If the minimum heap size is zero then the heap is disabled. */
        p0_size_info = &main_core->heap[heap_num];
        heap_min = p0_size = p0_size_info->heap_size;
        if (heap_min == 0)
        {
            DBG_MEM_CFG_MSG1("Heap %d disabled for this build.", heap_num);
            continue;
        }

        /* Check if the second core is enabled. */
        if (second_core != NULL)
        {
            /* Read the size of the second core heap.*/
            p1_size_info = &second_core->heap[heap_num];
            p1_size = p1_size_info->heap_size;
        }
        else
        {
            /* Disable the second core by setting the p1 heap size to zero. */
            p1_size = 0;
        }

        /* Increment the required heap size to the dynamic heap configuration values if
         * it is bigger than the minimum heap size. */
        if (have_dyn_config)
        {
            p0_dyn_size = dyn_heap_config->p[HEAP_PROC_0].heap_size[heap_num];
            p0_size = MAX(p0_dyn_size, heap_min);

            if (second_core != NULL)
            {
#ifdef INSTALL_DUAL_CORE_SUPPORT
                p1_dyn_size = dyn_heap_config->p[HEAP_PROC_1].heap_size[heap_num];
#else
                p1_dyn_size = 0;
#endif
                p1_size = MAX(p1_dyn_size, heap_min);
            }
        }

        /* Read the maximum size of the current heap. */
        current_heap_max_size = heap_max_sizes->heap_size[heap_num];

        /* Calculate the P0 heap start, end address and size. */
        p0_start = p0_size_info->heap_start;
        p0_end = p0_start + p0_size;
        /* The shared heap boundary between p0 and p1 doesn't have to be in a memory
         * bank boundary. */
        if (heap_num != HEAP_SHARED)
        {
            p0_end = round_up_to_dm_bank_addr(p0_end);
            p0_size =  p0_end - p0_start;
        }

        /* This will allow us to round down the heap start address to a memory bank
         * boundary. Note this can possibly increasing the heap size.*/
        p1_end = p0_start + current_heap_max_size;
        p1_start = p1_end - p1_size;
        if (heap_num != HEAP_SHARED)
        {
            p1_start = round_down_to_dm_bank_addr(p1_start);
            p1_size = p1_end - p1_start;
        }

        if (p0_end > p1_start)
        {
            WARN_MEM_CFG_MSG3("Heap %d configuration error while aligning\n"
                "p0 and p1 heap to mem banks boundary!\n"
                "p0 heap ends at: 0x%05x, p1 heap starts at: 0x%05x\n"
                "p1 heap start will be moved to p0 heap end.",
               heap_num, p0_end, p1_start);
            p1_start = p0_end;
            p1_size = p1_end - p1_start;
        }


        if (heap_overflow_to_free_ram())
        {
            /* Use up the free RAM by setting the end of P0 heap to the start of the
             * p1. This will give all the free memory to P0. */
            p0_end = p1_start;
            p0_size =  p0_end - p0_start;
        }

        /* Now save the heap configuration for both processors. */
        set_heap_info(p0_size_info, p0_start, p0_end, p0_size);
        if (second_core != NULL)
        {
            set_heap_info(p1_size_info, p1_start, p1_end, p1_size);
        }

        /* P0 can claim the free memory */
        pfreelist = &freelist[heap_num];
        /* heap_min was the initial size of the heap set in init_heap*/
        if (p0_size_info->heap_size > heap_min)
        {
            /* Update the P0 freelist */
            coalesce_free_mem( pfreelist,
                               p0_start + heap_min,
                               p0_size_info->heap_size - heap_min);
        }
    }

}

/*************************** Set the memory bank size  ********************************/
/* RAM is not mapped to DM banks in all
 * platforms. For crescendo/stre it is
 * 32K. For the rest, it doesn't have any effect
 */
#ifndef DM_RAM_BANK_SIZE
#define DM_RAM_BANK_SIZE 1
#endif

/* Rounds up the address to the next memory bank boundary. */
static inline char* round_up_to_dm_bank_addr(char* address)
{
    uintptr_t retval = (uintptr_t)(address) + DM_RAM_BANK_SIZE - 1;
    retval = (retval / DM_RAM_BANK_SIZE) * DM_RAM_BANK_SIZE;
    return (char*) retval;
}

/* Rounds down the address to the bank start address. In other words it returns the
 * start address of the bank that contains the address given as argument*/
static inline char* round_down_to_dm_bank_addr(char* address)
{
    uintptr_t retval = (uintptr_t)(address);
    retval = (retval / DM_RAM_BANK_SIZE) * DM_RAM_BANK_SIZE;
    return (char*) retval;
}

#endif /* __GNUC__ */

/****************************************************************************
Public Function Definitions
*/

/**
 * NAME
 *   init_heap
 *
 * \brief Initialise the memory heap
 *
 */
void init_heap(void)
{
#if (defined(INSTALL_DUAL_CORE_SUPPORT) || defined(AUDIO_SECOND_CORE)) \
        && !defined(__GNUC__)  && !defined(UNIT_TEST_BUILD)
    unsigned proc_id;

    /* initialise the heap config*/
    if ((proc_id = hal_get_reg_processor_id()) != 0)
    {
        heap_config *config = NULL;
        /* Secondary processor must be getting the heap info struct
         * pointer from ipc LUT instead of accessing heap_info_list
         * directly.
         */
        if (ipc_get_lookup_addr(IPC_LUT_ID_HEAP_CONFIG_SHARED_ADDR,
                              (uintptr_t*)&config) != IPC_SUCCESS)
        {
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_HEAP_CONFIG_SHARED_ADDR);
        }

#ifdef PMALLOC_DEBUG
        if (config == NULL)
        {
            /* config must not be 0 if coming here */
            panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ERROR_CONFIG_NULL);
        }
#endif /* PMALLOC_CONFIG */

        /* processor 1 init- More than 2 processors are not supported now*/
        pheap_info =  &(config[proc_id]);

        /* processor 0 must have configured all
         * heaps. so nothing to do here
         */
    }
    else
#endif  /* INSTALL_DUAL_CORE_SUPPORT || AUDIO_SECOND_CORE */
    {
        /* processor 0 init */
        pheap_info = &processor_heap_info_list[0];
        /* Set the heap information for p0 */
        set_heap_info(&pheap_info->heap[HEAP_DM1],      heap1, heap1 + HEAP_SIZE_DM1,     HEAP_SIZE_DM1);
        set_heap_info(&pheap_info->heap[HEAP_DM2],      heap2, heap2 + HEAP_SIZE_DM2,     HEAP_SIZE_DM2);
        set_heap_info(&pheap_info->heap[HEAP_SHARED],   heap3, heap3 + HEAP_SIZE_SHARED,  HEAP_SIZE_SHARED);
        set_heap_info(&pheap_info->heap[HEAP_DM1_EXT], heap4, heap4 + HEAP_SIZE_DM1_EXT, HEAP_SIZE_DM1_EXT);
    }

    {
        unsigned i;
        /* pheap_info is processor specific static global and it
         * now points to the heap_config specific to the current
         * processor
         */
        for (i = 0; i < HEAP_ARRAY_SIZE; i++)
        {
            freelist[i] = init_heap_node(pheap_info->heap[i].heap_start,
                                            pheap_info->heap[i].heap_size);

        }
    }

#ifdef HEAP_DEBUG
    heap_debug_min_free = heap_debug_free;
    heap_debug_alloc = 0;
    heap_debug_allocnodes = 0;
#endif
}

/**
 * NAME
 *   config_heap
 *
 * \brief Configure the memory heap. This is called after booting up
 *        the subsystem to claim the remaining memory area in the memory
 *        map as heap.
 *
 *        It also reads the MIB key to decide how much memory to be allocated
 *        to each processors.
 *
 *        Currently only dual core is handled
 */
#ifndef __GNUC__
void config_heap(void)
{
    heap_config *second_core_config;
    heap_dyn_size_config dyn_heap_config;
    bool have_dyn_config = FALSE;
    heap_sizes heap_max_sizes;

#ifdef INSTALL_DUAL_CORE_SUPPORT

    /* P0 is only allowed to config the heap */
    if (hal_get_reg_processor_id() != 0)
    {
        return;
    }

    /**
     * Second processor could be disabled after boot via an ACCMD command. That
     * would get us into this function for a second time. So while
     * processor_heap_info_list is zero-initialised We need to make sure its
     * irelevant parts are in fact zero.
     */
    memset(processor_heap_info_list + HEAP_PROC_1, 0, sizeof(heap_config));

    /* if second processor is not active, all heap is with core0. Note: Only
     * P0 will arrive here. */
    if (hal_get_active_num_cores() > 1)
    {
        /* Currently only 2 cores are expected */
        second_core_config = &processor_heap_info_list[HEAP_PROC_1];
    }
    else
#endif /*INSTALL_DUAL_CORE_SUPPORT */
    {
        /* heap_dynamic_config and heap_fixed_config will ignore the second core if
         * the pointer is set to NULL. */
        second_core_config = NULL;
    }

    /* Only P0 reaches here. set the maximum heap sizes allowed */
    /* Read any dynamic heap configuration */
    have_dyn_config = get_heap_allocation(&dyn_heap_config);

    /* Sets the maximum heap sizes. */
    set_heap_max_sizes(second_core_config != NULL ? TRUE: FALSE, &heap_max_sizes);

    heap_configure_and_align(
        pheap_info,
        second_core_config,
        &heap_max_sizes,
        have_dyn_config,
        &dyn_heap_config,
        freelist);

#if defined(INSTALL_DUAL_CORE_SUPPORT) && !defined(UNIT_TEST_BUILD)
    if (second_core_config != NULL)
    {
        /* update the bank access permissions for crescendo */
        set_memory_bank_permissions();

        /* Now populate the heap_config location in the IPC LUT
         * for the second core to complete its heap initialisation
         */
        if (ipc_set_lookup_addr(IPC_LUT_ID_HEAP_CONFIG_SHARED_ADDR,
                               (uintptr_t)processor_heap_info_list) != IPC_SUCCESS )
        {
           panic_diatribe(PANIC_IPC_LUT_CORRUPTED, IPC_LUT_ID_HEAP_CONFIG_SHARED_ADDR);
        }
    }
#endif  /* INSTALL_DUAL_CORE_SUPPORT */

    /* core1 will populate its freelist while its boots up.
     * Nothing to do here */
}
#endif /* __GNUC__ */

/**
 * NAME
 *   is_addr_in_heap
 *
 *  \brief check whether the pointer is in heap or not
 */
bool is_addr_in_heap( void* addr)
{
    unsigned heap_num;
    heap_info *heap_cfg = pheap_info->heap;

    for (heap_num = 0; heap_num < HEAP_ARRAY_SIZE; heap_num++)
    {
        if ((heap_cfg[heap_num].heap_start <= (char*)addr) && ((char*)addr < heap_cfg[heap_num].heap_end))
        {
            return TRUE;
        }
    }
    return FALSE;
}


/**
 * NAME
 *   heap_alloc
 *
 * \brief Allocate a block of (at least) the requested size
 *
 */
#ifdef PMALLOC_DEBUG
void *heap_alloc_debug( unsigned size, unsigned preference,
                        const char *file, unsigned int line)
#else
void *heap_alloc(unsigned size, unsigned preference)
#endif
{
    void *addr;
    bool pref_dm1 = is_dm2_heap_loaded();

#ifdef CHIP_HAS_NO_SLOW_DM_RAM
    /* If chip has no slow RAM, request for FAST is treated as
     * No preference
     */ 
    if( preference == MALLOC_PREFERENCE_FAST) 
    {
        preference =  MALLOC_PREFERENCE_NONE;
    }
#endif

    /* Don't do anything if zero size requested */
    if (size == 0)
    {
        return NULL;
    }
    PL_PRINT_P1(TR_PL_MALLOC, "Allocating size %d ", size);

    /* If last time DM1 alloc failed prefer doing DM2 first
     * Note that we specifically DON'T want to try to find the
     * 'best-fit' block across all heaps - in a mostly-unallocated
     * state that would tend just fill up the one with less free space.
     */
    switch(preference)
    {
    case MALLOC_PREFERENCE_DM1:
        if ((addr = heap_alloc_internal(size, HEAP_DM1_EXT)) != NULL)
        {
            break;
        }
        addr = heap_alloc_internal(size, HEAP_DM1);
        break;

    case MALLOC_PREFERENCE_DM2:
        if ((addr = heap_alloc_internal(size, HEAP_DM2)) != NULL)
        {
            break;
        }
        addr = heap_alloc_internal(size, HEAP_DM1);
        break;
    case MALLOC_PREFERENCE_FAST:
        if (pref_dm1)
        {
            if ((addr = heap_alloc_internal(size, HEAP_DM1_EXT)) != NULL)
            {
                break;
            }

            /* DM1 fast is full */
            if ((addr = heap_alloc_internal(size, HEAP_DM2)) != NULL)
            {
                break;
            }
        }
        else
        {
            if ((addr = heap_alloc_internal(size, HEAP_DM2)) != NULL)
            {
                break;
            }

            /* DM2 fast is full */
            if ((addr = heap_alloc_internal(size, HEAP_DM1_EXT)) != NULL)
            {
                break;
            }

        }

        /* fall back to slow ram */
        addr = heap_alloc_internal(size, HEAP_DM1);
        break;

    case MALLOC_PREFERENCE_SHARED:
        addr = heap_alloc_internal(size, HEAP_SHARED);
        break;

    case MALLOC_PREFERENCE_NONE:
    default:
        if (pref_dm1)
        {
            /* DM1 slow ram heap first*/
            if ((addr = heap_alloc_internal(size, HEAP_DM1)) != NULL)
            {
                break;
            }

            /* DM1 slow heap is full */
            if ((addr = heap_alloc_internal(size, HEAP_DM1_EXT)) != NULL)
            {
                break;
            }

            /* DM1 heap is full */
            addr = heap_alloc_internal(size, HEAP_DM2);
        }
        else
        {
            /* get from DM2 fast ram */
            if ((addr = heap_alloc_internal(size, HEAP_DM2)) != NULL)
            {
                break;
            }

            /* DM2 heap is full */
            if ((addr = heap_alloc_internal(size, HEAP_DM1)) != NULL)
            {
                break;
            }
            PL_PRINT_P0(TR_PL_MALLOC, "DM1 fallback Allocate from heap4");
            addr = heap_alloc_internal(size, HEAP_DM1_EXT);
        }
        break;
    }

#ifdef PMALLOC_DEBUG
    if (addr != NULL)
    {
        /* Record where this block was allocated from */
        mem_node *node = (mem_node *)((char *)addr - sizeof(mem_node));

        node->file = file;
        node->line = line;
        node->guard = HEAP_GUARD_WORD;
        *((unsigned int *)((char *)addr + node->length)) = HEAP_GUARD_WORD;
    }
#endif
    PL_PRINT_P1(TR_PL_MALLOC,"Allocated address from heap = %lx\n", (uintptr_t)addr);

    return addr;
}

/**
 * NAME
 *   heap_free
 *
 * \brief Free a previously-allocated block
 *
 */
void heap_free(void *ptr)
{
    mem_node **pfreelist=NULL;
    mem_node *node = (mem_node *)((char *)ptr - sizeof(mem_node));
    unsigned heap_num;
    if (ptr == NULL)
    {
        /* free(NULL) is a no-op  */
        return;
    }
    PL_PRINT_P1(TR_PL_FREE, "ptr to be freed %lx..", (uintptr_t)ptr);

    heap_num = get_heap_num(ptr);
    PL_PRINT_P1(TR_PL_FREE, "is in heap %d\n",heap_num);

    pfreelist = &freelist[heap_num];


    /* Check that the address being freed looks sensible */
    if (node->u.magic != MAGIC_WORD)
    {
        panic_diatribe(PANIC_AUDIO_FREE_INVALID, (DIATRIBE_TYPE)((uintptr_t)ptr));
    }

    /* Check that the length seems plausible. Function will panic with
     * PANIC_AUDIO_FREE_INVALID if memory is not in the heap. */
    get_heap_num((char *)ptr + node->length - 1);

#ifdef PMALLOC_DEBUG
    if (node->file == NULL)
    {
        panic_diatribe(PANIC_AUDIO_FREE_INVALID, (DIATRIBE_TYPE)((uintptr_t)node));
    }
    if (node->guard != HEAP_GUARD_WORD)
    {
        panic_diatribe(PANIC_AUDIO_DEBUG_MEMORY_CORRUPTION, (DIATRIBE_TYPE)((uintptr_t)node));
    }
    if (*((unsigned int *)((char *)ptr + node->length)) != HEAP_GUARD_WORD)
    {
        panic_diatribe(PANIC_AUDIO_DEBUG_MEMORY_CORRUPTION, (DIATRIBE_TYPE)((uintptr_t)node));
    }
    node->file = NULL;
    node->line = 0;
    node->guard = 0;
#endif

    node->u.magic = 0;

#ifdef HEAP_DEBUG
    heap_debug_allocnodes--;
    heap_debug_alloc -= node->length;
#endif

    /* coalsce the freed block */
    coalesce_free_mem( pfreelist,(char*) node,
                      node->length + sizeof(mem_node)+ GUARD_SIZE);

}


/**
 * NAME
 *   heap_sizeof
 *
 * \brief Get the size of a previously-allocated block
 *
 */
unsigned heap_sizeof(void *ptr)
{
    mem_node *node = (mem_node *)((char *)ptr - sizeof(mem_node));

    if (ptr == NULL)
    {
        return 0;
    }

    /* Check that the address looks sensible */
    if (node->u.magic != MAGIC_WORD)
    {
        /* Might want a (debug-only?) panic here */
        return 0;
    }

    return node->length;
}
/**
 * NAME
 *   heap_size
 *
 * \brief Heap size in words
 *
 */

unsigned heap_size(void)
{
    unsigned i, size = 0;

    for (i = 0; i < HEAP_ARRAY_SIZE; i++)
    {
        size += pheap_info->heap[i].heap_size;
    }

    return (size >> LOG2_ADDR_PER_WORD);
}

#ifdef PMALLOC_DEBUG
/**
 * NAME
 *   heap_validate
 *
 * \brief Check that a previously-allocated block looks sensible
 *
 */
void heap_validate(void *ptr)
{
    mem_node *node = (mem_node *)((char *)ptr - sizeof(mem_node));

    if (ptr == NULL)
    {
        /* Shouldn't happen, but don't bother checking if NULL */
        return;
    }

    /* Check that the address looks sensible */
    if (node->u.magic != MAGIC_WORD)
    {
        panic_diatribe(PANIC_AUDIO_DEBUG_MEMORY_CORRUPTION, (DIATRIBE_TYPE)((uintptr_t)ptr));
    }

    /* Check that the length seems plausible. get_heap_num panics if the pointer is not
     * in any heap memory. */
    get_heap_num((char *)ptr + node->length - 1);
}
#endif /* PMALLOC_DEBUG */


#ifdef DESKTOP_TEST_BUILD
/**
 * NAME
 *   heap_get_freestats
 *
 * \brief Test-only function to get total and largest free space
 *
 */
void heap_get_freestats(unsigned *maxfree, unsigned *totfree)
{
    unsigned heap_num, tot_size = 0, max_size = 0;
    mem_node *curnode;

    for (heap_num = 0; heap_num < HEAP_ARRAY_SIZE; heap_num++)
    {
        curnode = freelist[heap_num];
        while (curnode != NULL)
        {
            if (curnode->length -GUARD_SIZE > max_size)
            {
                max_size = curnode->length - GUARD_SIZE;
            }
            tot_size += curnode->length - GUARD_SIZE;
            curnode = curnode->u.next;
        }
    }
    *maxfree = max_size;
    *totfree = tot_size;
}

#endif /* DESKTOP_TEST_BUILD */

