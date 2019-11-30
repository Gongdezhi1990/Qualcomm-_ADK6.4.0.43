/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file const_data_a7da_kas.c
 * \ingroup const_data
 *
 * Atlas7DA/KAS platform implementation of constant data access API.
 */

#include "platform/pl_interrupt.h"
#include "kas_dma/kas_dma_req.h"
#include "kas_keyhole/kas_keyhole.h"
#include "const_data_private.h"
#include "sched_oxygen/sched_oxygen.h"

#ifdef INSTALL_CAP_DOWNLOAD_MGR
#include "cap_download_mgr.h"
#endif

#if !defined(__GNUC__)
#define DM_DRAM_CONST16_BASE _Pragma("datasection DM_DRAM_CONST16_BASE")
#define DM_DRAM_CONST32_BASE _Pragma("datasection DM_DRAM_CONST32_BASE")
#else
#define DM_DRAM_CONST16_BASE
#define DM_DRAM_CONST32_BASE
#endif

/****************************************************************************
Private definitions
*/

/* Structure for submitting a DMA request */
typedef struct{
   dma_request_entry req;
   kas_dma_regs      dma_regs;
}const_dma_req;

/* The node definition of the linked list for keeping track of memory allocations */
typedef struct work_buf_item_s
{
    struct work_buf_item_s * next;
    void* memory;
} work_buf_item;

/* The head of the linked list for keeping track of memory allocations */
work_buf_item * work_buf_top = (work_buf_item*) NULL;

/* The address of the allocated DRAM data, this should be set at Kymera boot time */
/* NOTE: downloaded capabilities should use its own DRAM base */
DM_DRAM_CONST16_BASE uint32 dram_const16_base = 0x0UL;
DM_DRAM_CONST32_BASE uint32 dram_const32_base = 0x0UL;

/* Retrieves the DRAM base from the descriptor. *
 * With capability download support on Atlas7/KAS we may have different DRAM bases,
 * which cannot be resolved at compile-time. So this function should ask the capability
 * download to resolve the const data DRAM base address of the allocated
 * memory block; a base of 0 denotes a global context (non-download context) which will
 * resolve to dram_const16_base or dram_const32_base. */
static uint32 get_const_data_dram_base(const_data_descriptor * source)
{
 	 if (source->base_id != 0)
    {
#ifdef INSTALL_CAP_DOWNLOAD_MGR
        if (((source->type) == MEM_TYPE_CONST16) || ((source->type) == MEM_TYPE_DMCONST_WINDOWED16))
        {
            return cap_download_mgr_get_const16_base(source->base_id);
        }
        else if ((source->type) == MEM_TYPE_CONST)
        {
            return cap_download_mgr_get_const_base(source->base_id);
        }
#endif
        return 0;
    }

    if (source->type==MEM_TYPE_CONST)
    {

         return dram_const32_base;
    }
    else  //if (source->type==MEM_TYPE_CONST16)
    {
         return dram_const16_base;

    }
}

static bool const_dma_initiate(dma_request_entry *req)
{
   const_dma_req  *const_req = (const_dma_req*)req;
   /* setup a normal transfer */
   kas_dma_normal(&const_req->dma_regs);
   return TRUE;
}

/*
    Setups the DMA transfer from DRAM to DM

    This function requires externally defined constants DRAM_CONST16_REGION_START and DRAM_CONST32_REGION_START
*/
static bool const_data_dma_setup( const_data_descriptor * source, unsigned offset,
                                    void * destination, unsigned size)
{
    /* panic if we are in interrupt space */
    if (is_current_context_interrupt())
    {
#ifdef DEBUG
        panic(PANIC_AUDIO_DEBUG_ASSERT_FAILED);
#endif
        return FALSE;
    }

    /* we only support right aligned 24 bit or dual 16bit packed word formats */
    unsigned word_format;
    /* data alignment for 16 bit sources */
    unsigned align;
    /* sign extension for 16bit right aligned sources */
    unsigned sign_extend;
    /* word swap for 16bit variables */
    unsigned word_swap = TRANSLATE_WORD_SWAP_DISABLED;

    /* format conversion: const data format -> KAS translate settings */
    switch (source->format)
    {
        case FORMAT_DSP_NATIVE:
            if (source->type==MEM_TYPE_CONST)
            {
                word_format = TRANSLATE_WORD_FORMAT_RIGHT_ALIGNED_24;
                align = TRANSLATE_LEFT_ALIGNED;
                sign_extend = TRANSLATE_SIGN_EXTEND_DISABLED;
            }
            else if ((source->type==MEM_TYPE_CONST16) || (source->type==MEM_TYPE_DMCONST_WINDOWED16) )
            {
                word_format = TRANSLATE_WORD_FORMAT_DUAL_16BIT;
                align = TRANSLATE_LEFT_ALIGNED;
                sign_extend = TRANSLATE_SIGN_EXTEND_DISABLED;
            }
            else
            {
                /* invalid combination of settings */
                return FALSE;
            }
            break;
        case FORMAT_PACKED16:
        case FORMAT_16BIT_ZERO_PAD:
            word_format = TRANSLATE_WORD_FORMAT_DUAL_16BIT;
            align = TRANSLATE_RIGHT_ALIGNED;
            sign_extend = TRANSLATE_SIGN_EXTEND_DISABLED;
            break;
        case FORMAT_16BIT_SIGN_EXT:
            word_format = TRANSLATE_WORD_FORMAT_DUAL_16BIT;
            align = TRANSLATE_RIGHT_ALIGNED;
            sign_extend = TRANSLATE_SIGN_EXTEND_ENABLED;
            break;
        default:
            /* invalid combination of settings */
            return FALSE;
    }

    /* dram offset (in words) = the constant data array address in mmap
     *                          - offset within the memory map
     *                          + user offset within the array
     */
    unsigned address = (*((unsigned *)&source->address))
                        - ((source->type==MEM_TYPE_CONST) ? DRAM_CONST32_REGION_START
                                                            : DRAM_CONST16_REGION_START)
                        + offset;

    /* get the DRAM base from the descriptor */
    uint32 dram_base = get_const_data_dram_base(source);

    if (word_format==TRANSLATE_WORD_FORMAT_DUAL_16BIT)
    {
        /* If transfers are not aligned use keyhole to access the unaligned 16-bit data */
        /* NOTE: instead of using keyholes, this could be optimised by allowing to write
                 outside the buffer boundary, and then restoring the old values. However,
                 it would impose extra restrictions on how the buffer should be allocated.*/
        uint32 data32b;

        if (address & 1)
        {
            size--;
            keyhole_read_32b_addr(dram_base + ((address/2)<<2), &data32b);
            *((unsigned *)destination) = (unsigned)(data32b & 0xFFFF);
            address++;
            destination = (unsigned *)destination+1;
        }

        if (size & 1)
        {
            size--;
            keyhole_read_32b_addr(dram_base + (((address/2) + (size/2))<<2), &data32b);
            *((unsigned *)destination+size) = (unsigned)(data32b>>16);
        }

        if (size==0)
        {
            return TRUE;
        }

        address/=2;
        size/=2;
        word_swap = TRANSLATE_WORD_SWAP_ENABLED;
    }

    const_dma_req dma_req;

    dma_req.req.dma_initiate_func = const_dma_initiate;
    dma_req.req.dma_complete_func = NULL;
    dma_req.req.state=DMA_REQ_STATE_AVAILABLE;

    dma_req.dma_regs.translate = SET_KAS_TRANSLATE(word_format, align, sign_extend,
                           word_swap, TRANSLATE_BYTE_SWAP_DISABLED);
    dma_req.dma_regs.base      = destination;
    dma_req.dma_regs.addr      = destination;
    dma_req.dma_regs.modulo    = 0;
    dma_req.dma_regs.inc       = 0;
    dma_req.dma_regs.xlen      = size;
    dma_req.dma_regs.dram_addr = dram_base + (address<<2);
    dma_req.dma_regs.burst_len = DMA_CTRL_BURST_LEN_16;
    dma_req.dma_regs.direction = DMA_CTRL_DIR_TO_SRAM;

    submit_dma_request_and_wait((dma_request_entry*)&dma_req);

    return TRUE;
}

/**
 * \brief Allocates a working buffer and appends its reference to a linked list
 * \param size - the size of the buffer to be allocated
 */
static void * work_buf_alloc( unsigned size )
{
    work_buf_item * work_buf;

    if ((work_buf = (work_buf_item *)xpmalloc(size+sizeof(work_buf_item)-1)) == NULL)
    {
        return NULL;
    }
    interrupt_block();
    if (work_buf_top == NULL)
    {
        work_buf->next = (work_buf_item *) NULL;
        work_buf_top = work_buf;
    }
    else
    {
        work_buf->next = work_buf_top;
        work_buf_top = work_buf;
    }
    interrupt_unblock();
    return &work_buf->memory;
}

/****************************************************************************
Public function definitions
*/

/** \brief Copy constant data.
 * Constant data will be copied from the source location to the 'destination'.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param destination - the destination memory block pointer, should be at least 'size' memory units long
 * \param size - the size of the data to be transferred
 * Note: the user is responsible for allocation and freeing of the memory pointed by 'destination'.
 */
bool const_data_copy ( const_data_descriptor * source, unsigned offset,
                        void * destination,  unsigned size )
{
    if (size==0)
    {
        return FALSE;
    }
    else if (source->type==MEM_TYPE_DMCONST || source->type==MEM_TYPE_DMCONST16)
    {
        if (source->format!=FORMAT_DSP_NATIVE)
        {
            /* TODO - Format not currently used */
            return FALSE;
        }

        /* copy the memory */
        memcpy(destination, (char*)(source->address) + offset, size);
        return TRUE;
    }
    else if (source->format==FORMAT_PACKED16)
    {
         unsigned  rdsize = ((size+1)>>1) + size;
         unsigned *work_buf = (unsigned*)xpmalloc(rdsize);

         /* Packet Data is 3 16-bit words containing 2  24-bit words
            read size = 3*size/2 into temporary buffer     */
         if (!work_buf)
         {
               return FALSE;
         }
         if (!const_data_dma_setup(source, offset, work_buf, rdsize))
         {
               pfree(work_buf);
               return FALSE;
         }
         /* Unpack to destination RAM */
         mem_unpack16(work_buf,size,(unsigned *)destination);

         /* release tempoary buffer */
         pfree(work_buf);
         return TRUE;
    }
    else
    {
        /* Copy external 16 or 32 bit data into 24-bit ram */

        /* setup a dram -> dm copy */
        if (!const_data_dma_setup(source, offset, destination, size))
        {
            return FALSE;
        }
        /* If data has been fetched from DRAM, it might need to get patched by cap_download_mgr */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
        if (source->base_id != 0)
        {
            cap_download_mgr_patch_consts(source->base_id, source->var_id, destination, offset, size);
        }
#endif
        return TRUE;
    }
}



/** \brief Access constant data.
 * A pointer to a block of constant data described by 'source' will be returned.
 * \param source - the source descriptor containing information about the constant data location and format
 * \param offset - the offset in the source memory where the data read should start from
 * \param work_buffer - the memory block which will act as a working buffer; if this parameter is NULL a new buffer will be allocated,
          on successive calls this parameter should contain the returned value from the first call to const_data_access. This is required
          due to Kalimba platform differences where locations may or may not be memory mapped (or directly accessible).
 * \param size - the size of the data to be transferred
 */
void * const_data_access ( const_data_descriptor * source, unsigned offset,
                            void * work_buf, unsigned size )
{
    if (size==0)
    {
        /* Unsupported on A7DA */
        return NULL;
    }
    else if (source->type==MEM_TYPE_DMCONST|| source->type==MEM_TYPE_DMCONST16)
    {
        /* Data is not external, it is already in RAM.  Just return pointer */
        if (source->format!=FORMAT_DSP_NATIVE)
        {
            /* can not reformat data*/
            return NULL;
        }
        return (void*) ( (unsigned *)source->address + offset);
    }
    else if (source->type==MEM_TYPE_CONST16 || source->type==MEM_TYPE_CONST || source->type==MEM_TYPE_DMCONST_WINDOWED16 )
    {
        if(source->format==FORMAT_PACKED16)
        {
           /* Packet Data is 3 16-bit words containing 2  24-bit words size = 3*size/2 */
            size = ((size+1)>>1) + size;
        }

        /* allocate a working buffer if NULL was passed */
        if (!work_buf)
        {
            if ((work_buf = work_buf_alloc(size)) == NULL)
            {
                return NULL;
            }
        }

        /* setup a dram -> dm copy */
        if (!const_data_dma_setup(source, offset, work_buf, size))
        {
            const_data_release(work_buf);
            return NULL;
        }
        /* If data has been fetched from DRAM, it might need to get patched by cap_download_mgr */
#ifdef INSTALL_CAP_DOWNLOAD_MGR
        if (source->base_id != 0)
        {
            cap_download_mgr_patch_consts(source->base_id, source->var_id, work_buf, offset, size);
        }
#endif
        return work_buf;
    }

    return NULL;
}

/** \brief Release (free) memory block allocated by const_data_access function.
 * \param ptr - pointer to the memory as returned by const_data_access.
 */
void const_data_release ( void * ptr )
{
    /* Check if the pointer was allocated by const_data_access and free the memory pointed by it */
    if (work_buf_top == NULL)
    {
        return;
    }
    else
    {
        work_buf_item* prev = (work_buf_item*) &work_buf_top;
        work_buf_item* iter = work_buf_top;
        interrupt_block();
        while(iter != NULL)
        {
            if (&iter->memory == ptr)
            {
                prev->next = iter->next;
                pfree(iter);
                interrupt_unblock();
                return;
            }
            prev = iter;
            iter = iter->next;
        }
        interrupt_unblock();
    }
}
