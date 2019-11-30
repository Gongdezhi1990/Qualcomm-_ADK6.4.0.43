/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
 *
 *
 ****************************************************************************
 * \file heap_alloc_pm.c
 * Memory allocation/free functionality for program memory (PM)
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
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
#include "kip_mgr/kip_mgr.h"
#endif
#include "patch.h"
/****************************************************************************
Private Macro Declarations
*/

/* PM allocation from core 0 PM heap */
#define HEAP_PM_CORE_0_ALLOC(size)  \
            heap_alloc_internal_pm(size, PM_HEAP_SIZE, &freelist_pm) 

/* PM allocation from core 1 PM heap */
#define HEAP_PM_CORE_1_ALLOC(size)  \
            heap_alloc_internal_pm(size, PM_P1_HEAP_SIZE, &freelist_pm_p1) 

#if !defined(KAL_ARCH4)
#define READ_NODE_FROM_PM(addr32, node) \
        do { \
        (node)->raw_memory[0] = (((*(volatile uint16*)((volatile uint16*)PM_RAM_MS_WINDOW + (addr32))) & 0xFF) << 16); \
        (node)->raw_memory[0] |= ((*(volatile uint16*)((volatile uint16*)PM_RAM_LS_WINDOW + (addr32))) & 0xFFFF); \
        (node)->raw_memory[1] = (((*(volatile uint16*)((volatile uint16*)PM_RAM_MS_WINDOW + ((addr32) + 1))) & 0xFF) << 16); \
        (node)->raw_memory[1] |= ((*(volatile uint16*)((volatile uint16*)PM_RAM_LS_WINDOW + ((addr32) + 1))) & 0xFFFF); \
        } while (0)

#define WRITE_NODE_TO_PM(addr32, node) \
        do { \
        *(volatile unsigned*)((volatile unsigned*)PM_RAM_MS_WINDOW + (addr32)) = (uint16)((((node)->raw_memory[0])>>16) & 0xFF); \
        *(volatile unsigned*)((volatile unsigned*)PM_RAM_LS_WINDOW + (addr32)) = (uint16)((node)->raw_memory[0] & 0xFFFF); \
        *(volatile unsigned*)((volatile unsigned*)PM_RAM_MS_WINDOW + (addr32 + 1)) = (uint16)((((node)->raw_memory[1])>>16) & 0xFF); \
        *(volatile unsigned*)((volatile unsigned*)PM_RAM_LS_WINDOW + (addr32 + 1)) = (uint16)((node)->raw_memory[1] & 0xFFFF); \
        } while (0)
#else
#define READ_NODE_FROM_PM(addr32, node) \
        do { \
        (node)->raw_memory[0] = (*(volatile uint32*)((volatile uint32*)(PM_RAM_WINDOW - PM_RAM_START_ADDRESS) + (addr32))); \
        (node)->raw_memory[1] = (*(volatile uint32*)((volatile uint32*)(PM_RAM_WINDOW - PM_RAM_START_ADDRESS) + ((addr32) + 1))); \
        } while (0)

#define WRITE_NODE_TO_PM(addr32, node) \
        do { \
        *(volatile unsigned*)((volatile unsigned*)(PM_RAM_WINDOW - PM_RAM_START_ADDRESS) + (addr32)) = (uint32)((node)->raw_memory[0]); \
        *(volatile unsigned*)((volatile unsigned*)(PM_RAM_WINDOW - PM_RAM_START_ADDRESS) + (addr32) + 1)  = (uint32)((node)->raw_memory[1]); \
        } while (0)
#endif

#define IS_IN_PM_P0_HEAP(addr) (((addr) >= heap_pm_adjusted) && ((addr) <= (void_func_ptr)((uintptr_t)heap_pm_adjusted + PM_HEAP_SIZE*PC_PER_INSTRUCTION)))

#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)

#define IS_IN_PM_P1_HEAP(addr) (((addr) >= heap_pm_adjusted_p1) && ((addr) <= (void_func_ptr)((uintptr_t)heap_pm_adjusted_p1 + PM_P1_HEAP_SIZE*PC_PER_INSTRUCTION)))
#define IS_IN_PM_HEAP(addr) ((IS_IN_PM_P0_HEAP(addr))||(IS_IN_PM_P1_HEAP(addr)))

#else

#define IS_IN_PM_HEAP(addr) (IS_IN_PM_P0_HEAP(addr))

#endif

#define GET_NODE_ADDR32(heap_pm) \
        (((uintptr_t)(heap_pm)) / PC_PER_INSTRUCTION)

/*
 * Directly accessing the linker provided symbol
 * as an address and typecasting to integer to make
 * the compiler happy
 */
#define HEAP_SIZE(x) ((unsigned)((uintptr_t)&(x)))

#define PM_HEAP_SIZE    ((HEAP_SIZE(_pm_heap_size) - pm_reserved_size) / PC_PER_INSTRUCTION)
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
#define PM_P1_HEAP_SIZE (HEAP_SIZE(_pm_p1_heap_size) / PC_PER_INSTRUCTION)
#endif

/* This macro gets the size of a variable in DM, and normalises it into platform words
 * It's assumed that a word that fits in DM, will fit in PM (e.g. 24 or 32 bit DM words will
 * fit in 32-bit PM words) */
#define SIZE_OF_DM_VAR_IN_PM_32(x) (sizeof(x)/ADDR_PER_WORD)
#define BYTES_INTO_PM_32(x) (x/PC_PER_INSTRUCTION)


#define MAGIC_WORD 0xabcd01ul
#define MIN_SPARE_32 BYTES_INTO_PM_32(32)

/****************************************************************************
Private Type Declarations
*/

typedef union
{
    struct struct_mem_node
    {
        unsigned length_32;
        union
        {
            void_func_ptr next;
            unsigned magic;
        } u;
    } struct_mem_node;
    unsigned raw_memory[2];
} mem_node_pm;

/****************************************************************************
Private Variable Definitions
*/
static unsigned int pm_reserved_size = 0;

static void_func_ptr freelist_pm;
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
static void_func_ptr freelist_pm_p1;
#endif

/* We might need to correct the address (i.e. Minim addresses set the bottom bit to 1) */
static void_func_ptr heap_pm_adjusted;
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
static void_func_ptr heap_pm_adjusted_p1;
#endif

/****************************************************************************
Private Function Definitions
*/
/* This is where the PM heap starts */
extern PM_MALLOC void heap_pm(void);
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
extern PM_MALLOC_P1 void heap_pm_p1(void);
#endif

static void_func_ptr init_and_adjust_pm_heap(unsigned pm_heap_size, void_func_ptr heap_addr)
{
    patch_fn_shared(heap_alloc_pm_init);
    unsigned int pm_win_value;
    void_func_ptr adjusted_heap_pm;
    /* pm_heap_size is in 32-bit words */
    unsigned freelength = pm_heap_size - SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm);
    mem_node_pm heap_node;

    LOCK_INTERRUPTS;
    /* Save initial state */
    pm_win_value = PMWIN_ENABLE;
    /* Enable access to PM through DM window */
    PMWIN_ENABLE = 1;

    heap_node.struct_mem_node.u.next= NULL;
    heap_node.struct_mem_node.length_32 = freelength;
    adjusted_heap_pm = heap_addr;
    #if PC_PER_INSTRUCTION == 4
    /* 8-bit addressable architecture, make sure the MiniMode bit is cleared */
    adjusted_heap_pm = (void_func_ptr)(((uintptr_t)adjusted_heap_pm) & ~1);
    #endif
    WRITE_NODE_TO_PM(GET_NODE_ADDR32(adjusted_heap_pm), &heap_node);
    /* Restore initial state */
    PMWIN_ENABLE = pm_win_value;
    UNLOCK_INTERRUPTS;
    return adjusted_heap_pm;
}

/****************************************************************************
Public Function Definitions
*/
/**
 * NAME
 *   init_heap_pm
 *
 * \brief Initialise the memory heap
 *
 */

void init_heap_pm(unsigned int reservedSize)
{
    patch_fn_shared(heap_alloc_pm_init);
    /* Initialise the PM heap
     * Create a single free block of maximum length
     * and point the free list at it
     */
    pm_reserved_size = reservedSize;
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
    if(KIP_PRIMARY_CONTEXT())
    {
        heap_pm_adjusted = init_and_adjust_pm_heap(PM_HEAP_SIZE, (void_func_ptr) (((uintptr_t)heap_pm) + reservedSize));
        freelist_pm = heap_pm_adjusted;
        heap_pm_adjusted_p1 = init_and_adjust_pm_heap(PM_P1_HEAP_SIZE, heap_pm_p1);
        freelist_pm_p1 = heap_pm_adjusted_p1;
    }
    else
    {
        /* When P1 boots, at this point it'll have overwritten P1 heap, so update it again
         * No need to get the freelist_pm_p1 here as it'll all be handled by P0 */
        init_and_adjust_pm_heap(PM_P1_HEAP_SIZE, heap_pm_p1);
    }
#else
    heap_pm_adjusted = init_and_adjust_pm_heap(PM_HEAP_SIZE, (void_func_ptr) (((uintptr_t)heap_pm) + reservedSize));
    freelist_pm = heap_pm_adjusted;
#endif
}

/* *
 * Declare heap_alloc_internal_pm as a static function (unsigned, unsigned, pointer to pointer to function (void) returning void)
 * returning a pointer to function (void) returning void
 * */
static void_func_ptr heap_alloc_internal_pm(unsigned size_byte, unsigned heapsize_32, void_func_ptr *pfreelist)
{
    patch_fn(heap_alloc_pm_heap_alloc_internal_pm);
    unsigned int pm_win_value;
    mem_node_pm node, node2;
    /* The following two pointers can use the same memory as we don't need to have
     * them loaded at the same time */
    mem_node_pm *curnode = &node;
    mem_node_pm *bestnode = &node;
    /* This pointer needs separate memory */
    mem_node_pm *tempnode = &node2;
    unsigned bestsize_32 = heapsize_32;
    void_func_ptr bestnode_pm_addr = NULL;
    void_func_ptr curnode_pm_addr;
    void_func_ptr prevnode_pm_addr = NULL;
    void_func_ptr bestnode_prev_pm_addr = NULL;

    curnode_pm_addr = *pfreelist;

    /* Do all the list-traversal and update with interrupts blocked */
    LOCK_INTERRUPTS;
    /* Save initial state */
    pm_win_value = PMWIN_ENABLE;
    /* Enable access to PM through DM window */
    PMWIN_ENABLE = 1;

    /* Traverse the list looking for the best-fit free block
     * Best fit is the smallest one of at least the requested size
     * This will help to minimise wastage
     */
    while (curnode_pm_addr != NULL)
    {
        READ_NODE_FROM_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);
        if (((curnode)->struct_mem_node.length_32 >= BYTES_INTO_PM_32(size_byte)) &&
                ((curnode)->struct_mem_node.length_32 < bestsize_32))
        {
            bestnode_pm_addr = curnode_pm_addr;
            /* Save the pointer that pointed to the best node, if it was the first one in DM, this would be NULL */
            bestnode_prev_pm_addr = prevnode_pm_addr;
            bestsize_32 = curnode->struct_mem_node.length_32;
        }
        /* Save the node (PM Address) that will point to next one */
        prevnode_pm_addr = curnode_pm_addr;
        curnode_pm_addr = (curnode)->struct_mem_node.u.next;
    }

    if (bestnode_pm_addr)
    {
        void_func_ptr addr;
        READ_NODE_FROM_PM(GET_NODE_ADDR32(bestnode_pm_addr), bestnode);
        if (bestsize_32 >= (BYTES_INTO_PM_32(size_byte)) + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm) + MIN_SPARE_32)
        {
            /* There's enough space to allocate something else
             * so keep the existing free block and allocate the space at the top
             * In this case the allocation size is exactly what was requested
             */
            addr = (void_func_ptr) ((uintptr_t)(bestnode_pm_addr) +
                    (bestnode->struct_mem_node.length_32 - BYTES_INTO_PM_32(size_byte))*PC_PER_INSTRUCTION);
            bestnode->struct_mem_node.length_32 -= (BYTES_INTO_PM_32(size_byte) + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm));
            WRITE_NODE_TO_PM(GET_NODE_ADDR32(bestnode_pm_addr), bestnode);
        }
        else
        {
            /* Not enough extra space to be useful
             * Replace the free block with an allocated one
             * The allocation size is the whole free block
             */
            addr = bestnode_pm_addr;
            size_byte = (bestnode->struct_mem_node.length_32)*PC_PER_INSTRUCTION;

            /* Update pointer that pointed to the best node */
            if (bestnode_prev_pm_addr != NULL)
            {
                READ_NODE_FROM_PM(GET_NODE_ADDR32(bestnode_prev_pm_addr), tempnode);
                tempnode->struct_mem_node.u.next = bestnode->struct_mem_node.u.next;
                WRITE_NODE_TO_PM(GET_NODE_ADDR32(bestnode_prev_pm_addr), tempnode);
            }
            else
            {
                /* This node was pointed to by an address in DM, update it */
                *pfreelist = bestnode->struct_mem_node.u.next;
            }
        }
        /* Finally populate the header for the newly-allocated block */
        bestnode->struct_mem_node.length_32 = BYTES_INTO_PM_32(size_byte);
        bestnode->struct_mem_node.u.magic = MAGIC_WORD;
        WRITE_NODE_TO_PM(GET_NODE_ADDR32(addr), bestnode);
        /* Restore initial state */
        PMWIN_ENABLE = pm_win_value;
        UNLOCK_INTERRUPTS;
        return (void_func_ptr) ((uintptr_t)addr + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm)*PC_PER_INSTRUCTION);
    }
    /* No suitable block found */
    /* Restore initial state */
    PMWIN_ENABLE = pm_win_value;
    UNLOCK_INTERRUPTS;
    return NULL;
}

/**
 * NAME
 *   heap_alloc_pm
 *
 * \brief Allocate a block of (at least) the requested size
 *
 */
void_func_ptr heap_alloc_pm(unsigned size_byte, unsigned preference_core)
{
    patch_fn_shared(heap_alloc_pm);
    void_func_ptr addr;

    /* Don't do anything if zero size requested */
    if (size_byte == 0)
    {
        return NULL;
    }
    /* Make sure we always try to allocate a 32bit-aligned amount of
     * PM memory, even on 8-bit addressable PMs */
    else if ((size_byte % PC_PER_INSTRUCTION) != 0)
    {
        size_byte += PC_PER_INSTRUCTION - (size_byte % PC_PER_INSTRUCTION);
    }
    switch(preference_core)
    {
        case MALLOC_PM_PREFERENCE_CORE_0:
            addr = HEAP_PM_CORE_0_ALLOC(size_byte);
            break;
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
        case MALLOC_PM_PREFERENCE_CORE_1:
            addr = HEAP_PM_CORE_1_ALLOC(size_byte);
            break;
#endif
        default:
            addr = HEAP_PM_CORE_0_ALLOC(size_byte);
            break;
    }

    return addr;
}

/**
 * NAME
 *   heap_free_pm
 *
 * \brief Free a previously-allocated block
 *
 */
void heap_free_pm(void_func_ptr ptr)
{
    patch_fn(heap_alloc_pm_heap_free_pm);
    unsigned int pm_win_value;
    void_func_ptr *pfreelist = NULL;

    mem_node_pm node_cur;
    mem_node_pm *curnode = &node_cur;
    void_func_ptr curnode_pm_addr = NULL;

    mem_node_pm node_;
    mem_node_pm *node = &node_;
    void_func_ptr node_pm_addr = NULL;

    LOCK_INTERRUPTS;
    /* Save initial state */
    pm_win_value = PMWIN_ENABLE;
    /* Enable access to PM through DM window */
    PMWIN_ENABLE = 1;

    if (ptr == NULL)
    {
        /* free(NULL) is a no-op  */
        return;
    }
    PL_PRINT_P1(TR_PL_FREE, "PM ptr to be freed %lx..", (uintptr_t)ptr);

    if (IS_IN_PM_HEAP(ptr))
    {
        PL_PRINT_P0(TR_PL_FREE, "is in main PM heap\n");
        pfreelist = &freelist_pm;
    }
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
    else if (IS_IN_PM_P1_HEAP(ptr))
    {
        PL_PRINT_P0(TR_PL_FREE, "is in PM P1 heap\n");
        pfreelist = &freelist_pm_p1;
    }
#endif
    else
    {
        PL_PRINT_P0(TR_PL_FREE, "Couldn't find in any PM heap\n");
        panic_diatribe(PANIC_AUDIO_FREE_INVALID, (uintptr_t)ptr);
    }

    node_pm_addr = (void_func_ptr) ((uintptr_t)ptr - SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm)*PC_PER_INSTRUCTION);
    READ_NODE_FROM_PM(GET_NODE_ADDR32(node_pm_addr), node);

    /* Check that the address being freed looks sensible */
    if (node->struct_mem_node.u.magic != MAGIC_WORD)
    {
        panic_diatribe(PANIC_AUDIO_FREE_INVALID, (uintptr_t)ptr);
    }

    /* Check that the length seems plausible */
    if (!IS_IN_PM_HEAP((void_func_ptr)((uintptr_t)ptr + (node->struct_mem_node.length_32 - 1)*PC_PER_INSTRUCTION)))
    {
        panic_diatribe(PANIC_AUDIO_FREE_INVALID, (uintptr_t)ptr);
    }

    node->struct_mem_node.u.magic = 0;
    WRITE_NODE_TO_PM(GET_NODE_ADDR32(node_pm_addr), node);

    curnode_pm_addr = *pfreelist;

    /* Traverse the free list to see if we can coalesce an existing free block with this one */
    while (curnode_pm_addr != NULL)
    {
        READ_NODE_FROM_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);
        if ((void_func_ptr)((uintptr_t)curnode_pm_addr + (curnode->struct_mem_node.length_32 + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm))*PC_PER_INSTRUCTION) == node_pm_addr)
        {
            /* Matching block found */
            break;
        }
        curnode_pm_addr = curnode->struct_mem_node.u.next;
    }

    if (curnode_pm_addr != NULL)
    {
        /* The immediately-previous block is free
         * add the one now being freed to it
         */
        curnode->struct_mem_node.length_32 += node->struct_mem_node.length_32 + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm);
        WRITE_NODE_TO_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);
    }
    else
    {
        /* Previous block wasn't free
         * so add the now-free block to the free list
         * Note length is unchanged from when it was allocated
         */
        curnode_pm_addr = node_pm_addr;
        READ_NODE_FROM_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);

        curnode->struct_mem_node.u.next = *pfreelist;
        WRITE_NODE_TO_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);

        *pfreelist = curnode_pm_addr;
    }
    /* Now check if there is a free block immediately after the found / new one */
    node_pm_addr = *pfreelist;
    while (node_pm_addr != NULL)
    {
        READ_NODE_FROM_PM(GET_NODE_ADDR32(node_pm_addr), node);
        if ( node_pm_addr == (void_func_ptr)((uintptr_t)curnode_pm_addr +
                (curnode->struct_mem_node.length_32 + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm))*PC_PER_INSTRUCTION))
        {
            /* Matching block found */
            break;
        }
        node_pm_addr = node->struct_mem_node.u.next;
    }
    if (node_pm_addr != NULL)
    {
        /* The immediately-following block is free
         * add it to the current one and remove from the free list
         */
        curnode->struct_mem_node.length_32 += node->struct_mem_node.length_32 + SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm);
        WRITE_NODE_TO_PM(GET_NODE_ADDR32(curnode_pm_addr), curnode);
        *pfreelist = node->struct_mem_node.u.next;
    }
    /* Restore initial state */
    PMWIN_ENABLE = pm_win_value;
    UNLOCK_INTERRUPTS;
}


/**
 * NAME
 *   heap_sizeof_pm
 *
 * \brief Get the size of a previously-allocated block
 *
 */
unsigned heap_sizeof_pm(void_func_ptr ptr)
{
    patch_fn_shared(heap_alloc_pm);
    unsigned int pm_win_value;
    void_func_ptr node_pm_addr;
    mem_node_pm node;

    if (ptr == NULL)
    {
        return 0;
    }
    node_pm_addr = (void_func_ptr) ((uintptr_t)ptr - SIZE_OF_DM_VAR_IN_PM_32(mem_node_pm)*PC_PER_INSTRUCTION);
    LOCK_INTERRUPTS;
    /* Save initial state */
    pm_win_value = PMWIN_ENABLE;
    /* Enable access to PM through DM window */
    PMWIN_ENABLE = 1;
    READ_NODE_FROM_PM(GET_NODE_ADDR32(node_pm_addr), &node);
    /* Restore initial state */
    PMWIN_ENABLE = pm_win_value;
    UNLOCK_INTERRUPTS;

    /* Check that the address looks sensible */
    if (node.struct_mem_node.u.magic != MAGIC_WORD)
    {
        /* Might want a (debug-only?) panic here */
        return 0;
    }
    return (node.struct_mem_node.length_32 * PC_PER_INSTRUCTION);
}

bool is_in_pm_heap(void_func_ptr ptr)
{
    patch_fn_shared(heap_alloc_pm);
    return IS_IN_PM_HEAP(ptr);
}
