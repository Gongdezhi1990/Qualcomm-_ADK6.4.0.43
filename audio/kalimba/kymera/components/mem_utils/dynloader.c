/****************************************************************************
 * Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file dynloader.c
 * \ingroup mem_util
 *
 * Handle the creation,management,and initialization of capability
 * data and scratch memory.   Also, handles shared constant variables. 
 */


/****************************************************************************
Include Files
*/
#include "string.h" // memcpy

#include "dynloader.h"
#include "types.h"
#include "scratch_memory.h"
#include "shared_memory.h"
#include "exported_constants.h"
#include "pmalloc/pl_malloc.h"
#include "const_data/const_data.h"
#include "hydra_log/hydra_log.h"
#include "patch/patch.h"

typedef uint16 dynmem16_t;
typedef unsigned int   dynmem32_t; 



/****************************************************************************
Internal Type Declarations
*/

typedef enum
{
   DYN_SECTION_TYPE_ALLOC_INST      =0x0000, /**< Peristent Allocation Section ID */
   DYN_SECTION_TYPE_ALLOC_SCRATCH   =0x0010, /**< Scratch Allocation Section ID */
   DYN_SECTION_TYPE_ALLOC_SHARE     =0x0020, /**< Shared Allocation Section ID */
   DYN_SECTION_TYPE_DATA_INST       =0x0001, /**< Initialization Data Section ID */
   DYN_SECTION_TYPE_DATA_SHARE      =0x0021, /**< Shared Initialization Data Section ID */
   DYN_SECTION_TYPE_RELOC_INST      =0x0002, /**< Relocation Pointer Section ID */
   DYN_SECTION_TYPE_RELOC_SCRATCH   =0x0012, /**< Scratch Relocation Pointer Section ID */
   DYN_SECTION_TYPE_RELOC_ROOT      =0x0004, /**< Root Object Relocation Pointer Section ID */
   DYN_SECTION_EXPORT_CONSTANTS     =0x0080, /**< Actual data is external */
   DYN_SECTION_TYPE_END             =0x5432  /**< Termination Section ID */
}DYN_SECTION_TYPE;

/* Control block for peristent allocation 
*/
typedef struct
{
   unsigned num_allocs;           /**< number of allocation in control block */
   uintptr_t *allocations[];      /**< array of memory allocations */
}DYN_PERSISTENT_ALLOC_BLOCK;

/* Control block for scratch allocation 
*/
typedef struct
{
   uintptr_t *alloc_section;            /**< cached allocation section */         
   uintptr_t *common_link_section;      /**< cached relocation section */  
   uintptr_t *variant_link_sections;    /**< cached relocation section */  

   unsigned first_banks;               /**< bank to use for first allocation*/  

   unsigned num_allocs;       /**< number of allocation in control block */
   uintptr_t *allocations[];   /**< array of memory allocations */
}DYN_SCRATCH_ALLOC_BLOCK;

/* Section Header and management
*/
typedef struct
{
   DYN_SECTION_TYPE        type;             /**< section type */  
   unsigned                identifier;       /**< section identifier: share_id,variant, or COMMON */  
   unsigned                length;           /**< length of section excluding header (in words) */  

   const_data_descriptor  *src_handle;       /**< source handle for script containing section */ 
   unsigned                script_pos;       /**< position in script containing section (in words) */ 
}DYN_SECTION_HDR;


/****************************************************************************
Internal Constant Declarations
*/

#define DYN_COMMON_SECTION  0xFFFF

#define DYN_SHDR_SIZE_IN_WORDS  3


/* A data initialization section may be quite large.
 * Need to break it up for efficient memory usage.  
 * Block Length is 8-bits so there is maximum of 255 elements in an initialization block.
 *  24-bit data is packed so 255 elements requires 383 words.  
 *  Add four words for block index and offset of two sections equals 387 words. */
#define MAX_DATA_INIT_BLOCK_SIZE  387


/****************************************************************************
Internal Function Definitions
*/

/* Using inline allows the compiler to do a decent job of optimization.
 * However, this optimization mangles the C debugger */
#define DYN_INLINE inline



/* Sign Extend 16-bit value to 24-bits 
*/
#ifdef __GNUC__
inline static int sign_ext_16bit(unsigned a)
{
   return( ((int)(a<<(DAWTH-16)))>>(DAWTH-16)); 
}
#else
asm int sign_ext_16bit(unsigned a)
{
    @{} = @{a} LSHIFT (DAWTH-16);
    @{} = @{} ASHIFT -(DAWTH-16);
}
#endif


/****************************************************************************
 *
 * GetSectionData
 *
 * Return pointer to section playload
 *
 */

DYN_INLINE static uintptr_t* GetSectionData16(DYN_SECTION_HDR *hdr)
{
   return((uintptr_t*)const_data_access(hdr->src_handle, hdr->script_pos*sizeof(dynmem16_t), NULL, hdr->length*sizeof(dynmem16_t) ) );
}

DYN_INLINE static uintptr_t* GetSectionData32(DYN_SECTION_HDR *hdr)
{
   return((uintptr_t*)const_data_access(hdr->src_handle, hdr->script_pos*sizeof(dynmem32_t), NULL, hdr->length*sizeof(dynmem32_t) ) );
}

/****************************************************************************
 *
 * CopySectionData
 *
 * Allocate memory and copy section payload
 *
 */
DYN_INLINE static uintptr_t* CopySectionData16(DYN_SECTION_HDR *hdr)
{
   uintptr_t *mem_ptr;
   /* Allocate memory to copy to (in words) */
   mem_ptr = (uintptr_t*)xppmalloc(hdr->length*sizeof(dynmem16_t),MALLOC_PREFERENCE_NONE);
   if(mem_ptr==NULL)
   {
      return(mem_ptr);
   }
   /* Copy external to internal */
   if(!const_data_copy(hdr->src_handle,hdr->script_pos * sizeof(dynmem16_t),mem_ptr,hdr->length*sizeof(dynmem16_t)))
   {
      // Access Error
      pfree(mem_ptr);
      mem_ptr=NULL;
   }
   return(mem_ptr);
}

/****************************************************************************
 *
 * AdvanceOverSection
 *
 * Advance position in script over section playload
 *
 */
DYN_INLINE static void AdvanceOverSection(DYN_SECTION_HDR *hdr)
{
   hdr->script_pos += hdr->length;
}

/****************************************************************************
 *
 * SetScriptSource
 *
 * Update script information in section header object
 *
 */
DYN_INLINE static void SetScriptSource(DYN_SECTION_HDR *hdr,const_data_descriptor *src)
{
   hdr->src_handle = src;
   hdr->script_pos = 0;
}

/****************************************************************************
 *
 * MapMemBank
 *
 * Translate script bank specifiers to Kymera 
 *
 */
DYN_INLINE static unsigned MapMemBank(unsigned bank)
{
   /* Bank is DM1=1,DM2=2, or DM=3 (format uses DM=0) */
   bank &= 0x3;
   if(bank==0)
   {
      bank=MALLOC_PREFERENCE_FAST;
   }
   return(bank);
}

/****************************************************************************
 *
 * GetSectionHeader
 *
 * Access section header at address
 *
 */

static uintptr_t* GetSectionHeader16(DYN_SECTION_HDR *hdr, uintptr_t *mem_ptr)
{
   dynmem16_t  *ms_ptr;

   ms_ptr = (dynmem16_t*)const_data_access(hdr->src_handle,hdr->script_pos * sizeof(dynmem16_t),mem_ptr,DYN_SHDR_SIZE_IN_WORDS * sizeof(dynmem16_t) );
   if(ms_ptr==NULL)
   {
      return((uintptr_t*)ms_ptr);
   }
   /* +-----------------------------------------+
    * | Section Type  | Identifier  | Length    |
    * +-----------------------------------------+*/
   hdr->type         = (DYN_SECTION_TYPE)(ms_ptr[0] & 0xFFFF);
   hdr->identifier   = (unsigned)(ms_ptr[1] & 0xFFFF);
   hdr->length       = (unsigned)(ms_ptr[2] & 0xFFFF);

   hdr->script_pos += DYN_SHDR_SIZE_IN_WORDS;

   return((uintptr_t*)ms_ptr);
}

static uintptr_t* GetSectionHeader32(DYN_SECTION_HDR *hdr, uintptr_t *mem_ptr)
{
   dynmem32_t  *ms_ptr;

   ms_ptr = (dynmem32_t*)const_data_access(hdr->src_handle,hdr->script_pos * sizeof(dynmem32_t),mem_ptr,DYN_SHDR_SIZE_IN_WORDS * sizeof(dynmem32_t));
   if(ms_ptr==NULL)
   {
      return((uintptr_t*)ms_ptr);
   }
   /* +-----------------------------------------+
    * | Section Type  | Identifier  | Length    |
    * +-----------------------------------------+*/
   hdr->type         = (DYN_SECTION_TYPE)(ms_ptr[0] & 0xFFFF);
   hdr->identifier   = (unsigned)(ms_ptr[1] & 0xFFFF);
   hdr->length       = (unsigned)(ms_ptr[2] & 0xFFFF);

   hdr->script_pos += DYN_SHDR_SIZE_IN_WORDS;

   return((uintptr_t*)ms_ptr);
}


/****************************************************************************
 *
 * DynAllocateBlocks
 *
 * Handle section for peristent data allocation
 *   
 */
DYN_INLINE static DYN_PERSISTENT_ALLOC_BLOCK* DynAllocateBlocks16(dynmem16_t *desc,uintptr_t *root,unsigned *root_offset)
{
   unsigned num_allocs,i;
   unsigned offset;
   unsigned size;
   unsigned banks;
   DYN_PERSISTENT_ALLOC_BLOCK *lpalloc_block;

   /* +------------------------------------------------------+ 
    * | Num allocs  | offset | bank1/bank2   | size1 | size2 | 
    * +------------------------------------------------------+*/
   num_allocs = (unsigned)(*(desc++) & 0xFFFF);
   offset     = (unsigned)(*(desc++) & 0xFFFF);
   banks      = (unsigned)(*(desc++) & 0xFFFF);
   /* Skip size of first block, using sizeof(DYN_PERSISTENT_ALLOC_BLOCK) instead */
   desc++;

   /* First allocation is control block. */
   size = sizeof(DYN_PERSISTENT_ALLOC_BLOCK) + sizeof(unsigned*)*num_allocs;
   lpalloc_block = (DYN_PERSISTENT_ALLOC_BLOCK*)xzppmalloc(size,MALLOC_PREFERENCE_NONE);
   
   *root_offset = offset;
   root[offset] = (uintptr_t)lpalloc_block;
   if(lpalloc_block == NULL)
   {
      return(NULL);
   }

   /* Setup control block */
   lpalloc_block->num_allocs     = num_allocs;
   lpalloc_block->allocations[0] = root;
    
   /* Get allocations.  First entry (root) is not allocated */
   for(i=1;i<num_allocs;i++)
   {
      unsigned current_bank;

      /* unpack bank to use when allocating block  */
      if(i&0x1)
      {
         current_bank = banks;
      }
      else
      {
         banks = (unsigned)(*(desc++));
         current_bank = (banks>>8);
      }
      /* Get size of block to allocate */
      size = (unsigned)(*(desc++) & 0xFFFF);
   
      if((lpalloc_block->allocations[i]=(uintptr_t*)xzppmalloc(size*sizeof(unsigned),MapMemBank(current_bank)))==NULL)
      {
         /* Allocation Failure.   */
         DynLoaderReleaseDynamicAllocations(lpalloc_block);
         root[offset] = 0;
         return(NULL);
      }
   }
   
   /* Success */
   return(lpalloc_block);
}

/****************************************************************************
 *
 * DynReserveScratch
 *
 * Handle section for scratch data reservation
 *
 */
DYN_INLINE static DYN_SCRATCH_ALLOC_BLOCK* DynReserveScratch16(dynmem16_t *desc,uintptr_t *root,unsigned *root_offset)
{
   unsigned num_allocs,i;
   unsigned offset;
   unsigned size;
   unsigned banks;
   DYN_SCRATCH_ALLOC_BLOCK *lpalloc_block;
   uintptr_t *alloc_sec = (uintptr_t*)desc;

   /* +------------------------------------------------------+ 
    * | Num allocs  | offset | bank1/bank2   | size1 | size2 | 
    * +------------------------------------------------------+*/
   num_allocs = (unsigned)(*(desc++) & 0xFFFF);
   offset     = (unsigned)(*(desc++) & 0xFFFF);
   banks      = (unsigned)(*(desc++) & 0xFFFF);
   /* Skip Size of first block, using sizeof(DYN_SCRATCH_ALLOC_BLOCK) instead */
   desc++;

   /* First allocation is control block. */
   size = sizeof(DYN_SCRATCH_ALLOC_BLOCK) + sizeof(unsigned*)*num_allocs;
   lpalloc_block = (DYN_SCRATCH_ALLOC_BLOCK*)xzppmalloc(size,MALLOC_PREFERENCE_NONE);
   
   *root_offset = offset;
   root[offset]=(uintptr_t)lpalloc_block;
   if(lpalloc_block == NULL)
   {
      /* Need to free alloctaed memory for section */
      pfree(alloc_sec);
      return(NULL);
   }

   // Setup control block
   lpalloc_block->alloc_section  = alloc_sec;
   lpalloc_block->first_banks    = banks; 
   lpalloc_block->num_allocs     = num_allocs;
   lpalloc_block->allocations[0] = root;
    
   /* Get allocations, First entry (root) is not allocated */
   for(i=1;i<num_allocs;i++)
   {
      unsigned current_bank;

      /* unpack bank to use when allocating block  */
      if(i&0x1)
      {
         current_bank = banks;
      }
      else
      {
         banks = (unsigned)(*(desc++));
         current_bank = (banks>>8);
      }
      /* Get size of block to allocate */
      size = (unsigned)(*(desc++) & 0xFFFF);

      if(!scratch_reserve(size*sizeof(unsigned),MapMemBank(current_bank)))
      {
         /* Reserve memory Failure.   */  
         DynLoaderDeregisterScratch(lpalloc_block);
         root[offset] = 0;
         return(NULL);
      }
   }
   /* Success */
   return(lpalloc_block);
}

/****************************************************************************
 *
 * DynReserveScratch
 *
 * Handle allocation scratch commit
 *
 */
DYN_INLINE static bool DynCommitScratch(DYN_SCRATCH_ALLOC_BLOCK *cntrl_block)
{
   unsigned size,banks;
   dynmem16_t *desc;
   int      i;
   bool     bHasChanged=FALSE;

   patch_fn_shared(mem_utils);

   /* Get allocation section pointer (skip num_allocs,offset, first banks & size) */
   desc  = (dynmem16_t*)cntrl_block->alloc_section;
   desc += 4;
   banks = cntrl_block->first_banks;

   /* Confirm allocations.  First entry (root) is not allocated */
   for(i=1;i<cntrl_block->num_allocs;i++)
   {
      unsigned current_bank;
      uintptr_t *scr_ptr;

      /* unpack bank to use when allocating block  */
      if(i&0x1)
      {
            current_bank = banks;   
          
      }
      else
      {
          banks = (unsigned)(*(desc++));
          current_bank = (banks>>8);     
      }
      /* Get size of block to allocate */
      size = (unsigned)(*(desc++) & 0xFFFF);
   
      /* This will not fail, memory allocated in reserve operation */
      scr_ptr = (uintptr_t*)scratch_commit(size*sizeof(unsigned),MapMemBank(current_bank));
      if(cntrl_block->allocations[i]!=scr_ptr)
      {
         bHasChanged = TRUE;
         cntrl_block->allocations[i]=scr_ptr;
      }
   }
   return bHasChanged;
}


/****************************************************************************
 *
 * DynInitializeBlocks
 *
 * Handle sections for data initialization
 *
 */
static int DynInitializeBlocks16(DYN_SECTION_HDR *lpsection,uintptr_t **allocations)
{
   uintptr_t  *section_data_ptr;
   dynmem16_t *desc;
   unsigned    chunk_size,remainder;

   patch_fn_shared(mem_utils);
   /* Read a chunk of data.   May or may not be complete section */
   remainder  = lpsection->length;
   chunk_size = (remainder>MAX_DATA_INIT_BLOCK_SIZE) ? MAX_DATA_INIT_BLOCK_SIZE : remainder;
   section_data_ptr = (uintptr_t*)const_data_access(lpsection->src_handle,lpsection->script_pos * sizeof(dynmem16_t),NULL,chunk_size*sizeof(dynmem16_t));
   /* Verify access to Data */
   if(section_data_ptr==NULL)
   { 
      return(0);
   }
   /* Amount of data remaining after chunk */
   chunk_size = remainder - chunk_size;          
   desc       = (dynmem16_t*)section_data_ptr;

   do
   {
      unsigned blk_idx;
      unsigned seg_len,i;
      unsigned offset;
#ifndef K32
      unsigned msb_vals=0;
#endif /* K32 */
      unsigned value;
      uintptr_t *dest_ptr;
      unsigned block_len;

      /* +------------------------------------------------------+ 
       * | blk idx/len  | offset | msb1/msb2   | lsw1 | lsw2    | 
       * +------------------------------------------------------+*/
      offset = (unsigned)(*(desc++) & 0xFFFF);
      blk_idx = (offset>>8);          
      seg_len = (offset&0xFF);   
      offset = (unsigned)(*(desc++) & 0xFFFF);     
      
      /* Note:  blk_idx is always zero for shared variables. */
      dest_ptr = allocations[blk_idx] + offset;

      /* Updated remainder for above reads */
      remainder  -= 2;

#ifdef K32
      block_len = seg_len * 2;
#else
      block_len = seg_len + ((seg_len+1)>>1);
#endif /* K32 */

      /* Check if we need to reload section */
      if(block_len > (remainder - chunk_size) )
      {
         unsigned script_pos;
         
         script_pos = lpsection->script_pos + (lpsection->length - remainder);
         chunk_size = (remainder>MAX_DATA_INIT_BLOCK_SIZE) ? MAX_DATA_INIT_BLOCK_SIZE : remainder;

         section_data_ptr = (uintptr_t*)const_data_access(lpsection->src_handle,script_pos * sizeof(dynmem16_t),section_data_ptr,chunk_size*sizeof(dynmem16_t));
         /* Verify access to Data */
         if(section_data_ptr==NULL)
         { 
            return(0);
         }
         /* working pointer */
         chunk_size = remainder - chunk_size;   
         desc       = (dynmem16_t*)section_data_ptr;
      }

      /* Update Remainder for block */
      remainder  -= block_len;

      /* Unpack initialization block */
      for(i=0;i<seg_len;i++)
      {
         /* Unpack data */
#ifdef K32
         /* 2:1 packing of initialization data is used on the 32-bit Kalimba */
         value = (unsigned)(*(desc++) & 0xFFFF);
#else
         /* 3:2 packing of initialization data is used on the 24-bit Kalimba */
         if(i&0x1)
         {
            value    = (msb_vals&0xFF); 
         }
         else
         {
            msb_vals = (unsigned)(*(desc++) & 0xFFFF);
            value    = (msb_vals>>8);
         }
#endif /* K32 */
         value  = value << 16;                     // msb
         value |= (unsigned)(*(desc++) & 0xFFFF);  // lsw

         /* Save value */
         *(dest_ptr++) = value;
      }
   }while(remainder>0);

   /* Release Section accessor */
   const_data_release(section_data_ptr);
   return(1);
}


/****************************************************************************
 *
 * DynResolveInternalLinks
 *
 * Handle sections for pointer resolution
 *
 */
static void DynResolveInternalLinks16(dynmem16_t *desc,uintptr_t **dst_allocations,uintptr_t **src_allocations)
{
   unsigned num_links;
   int      i;

   /* +------------------------------------------------------------+ 
    * | num    | persist | scratch  | dest\Value | Dest   | Value  | 
    * | links  | offset  | offset   | index      | Offset | Offset | 
    * +------------------------------------------------------------+ */

   num_links  = (unsigned)(*(desc++) & 0xFFFF);

   /* Don't need offsets into root object for persistent and scratch 
      allocation tables.   They are passed into the function as parameters,
      skip over offsets */
   desc += 2;

   /* Process Links */
   for(i=0;i<num_links;i++)
   {
      unsigned blk_idx;
      int      dst_offset,val_offset;

      blk_idx     = (unsigned)(*(desc++) & 0xFFFF);
      dst_offset  = sign_ext_16bit(*(desc++));
      val_offset  = sign_ext_16bit(*(desc++));

      /* Resolve Link */
      *(dst_allocations[blk_idx>>8] + dst_offset) = (uintptr_t)(val_offset + src_allocations[blk_idx&0xFF]);
   }
}

/****************************************************************************
 *
 * DynResolveExternalLinks
 *
 * Handle sections for external pointer resolution
 *
 */
DYN_INLINE static void DynResolveExternalLinks32(dynmem32_t *desc,uintptr_t **allocations,unsigned length)
{
   int i;

   for(i=length>>1;i>0;i--)
   {
      unsigned blk_idx;
      int offset;

      /* block index is upper 8-bits, offset is lsw (sign extended) */
      blk_idx  = (unsigned)(*(desc++));
      offset   = sign_ext_16bit(blk_idx);

      /* Assign Value */
      *(allocations[blk_idx>>16] +  offset) = (*(desc++)); 
   }
}

/****************************************************************************
 *
 * DynResolveExternalShare
 *
 * Handle sections for external pointer resolution in shared variables
 *
 */
DYN_INLINE static void DynResolveExternalShare32(dynmem32_t *desc,uintptr_t *mem_block,unsigned length)
{
   int i,offset;

   for(i=length>>1;i>0;i--)
   {
      /* offset is lsw (sign extended) */
      offset   = sign_ext_16bit(*(desc++));

      /* Assign Value */
      *(mem_block +  offset) = (*(desc++)); 
   }
}

/****************************************************************************
 *
 * DynResolveRootLinks
 *
 * Handle section for transfering information from the root object
 *
 */
DYN_INLINE static void DynResolveRootLinks16(dynmem16_t *desc,uintptr_t **allocations,unsigned size)
{
   dynmem16_t *end_ptr  = desc + size;
   uintptr_t *root_ptr = allocations[0];

   /* +-------------------------------------------------+ 
    * | root    | Num       | Persist | Dest   | Value  | 
    * | offset  | Refereces | Index   | Offset | Offset | 
    * +-------------------------------------------------+ */

   do
   {
      unsigned num_refs,i,blk_idx;
      int dst_offset,val_offset,root_offset;
      unsigned int* field_ptr;

      root_offset = (int)(*(desc++) & 0xFFFF);
      num_refs    = (unsigned)(*(desc++) & 0xFFFF);     
      
      /* Get Value from Root */
      field_ptr = (unsigned int*) root_ptr[root_offset];

      for(i=0;i<num_refs;i++)
      {
         blk_idx = (unsigned)(*(desc++) & 0xFFFF);    
         dst_offset  = sign_ext_16bit(*(desc++));
         val_offset  = sign_ext_16bit(*(desc++));

         /* Assign value from root object */
         *(allocations[blk_idx] + dst_offset) = (uintptr_t) (field_ptr + val_offset);
      }

   }while(desc<end_ptr);
}


/****************************************************************************
 *
 * ReleaseSharedReferences
 *
 * Release Shared variables internal to a shared variable
 *
 */
static void ReleaseSharedReferences16(void *src_handle,dynmem16_t *lprefs,unsigned num_refs)
{
   int i;

   /* +-------------------------------------------------------------------+ 
    * | Alloc size  | Bank/Num Refs | Ref ID    | Ref Offset | Ref Addend | 
    * +-------------------------------------------------------------------+*/
   
   /* Skip to 1st Ref ID */
   lprefs += 2;

   /* Allocate references */
   for(i=0;i<num_refs;i++)
   {
      unsigned ref_id;

      ref_id = (unsigned)(*(lprefs++) & 0xFFFF);
      DynLoaderReleaseSharedAllocations(ref_id,src_handle);

      /* Skip offset & Addend */
      lprefs += 2;
   }
}


/****************************************************************************
Public Function Definitions
*/

/****************************************************************************
 * DynLoaderProcessDynamicAllocations
 */
int DynLoaderProcessDynamicAllocations(uintptr_t *root,void *desc,void *ext_desc,unsigned variant)
{
   DYN_SECTION_HDR   sec_header;
   DYN_PERSISTENT_ALLOC_BLOCK *lpcntrl_block=NULL;
   unsigned          root_offset=0;
   uintptr_t         *section_hdr_ptr=NULL;
   uintptr_t         *section_data_ptr;

   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_16BIT_ZERO_PAD,desc);

   patch_fn_shared(mem_utils);

   if (!external_constant_map_descriptor(&mem_desc))
      goto aBort;
  
   /* Handle allocation script */
   SetScriptSource(&sec_header,&mem_desc);
   while(1)
   {
      /* Access Section Header */
      section_hdr_ptr = GetSectionHeader16(&sec_header,section_hdr_ptr);
      /* Verify section header access */
      if(section_hdr_ptr==NULL)
      {
         goto aBort;   
      }
      if(sec_header.type == DYN_SECTION_TYPE_END)
      {
         break;
      }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
      if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
      {
          goto aBort;   
      }
#endif
      /* Note:  There is no need to validate order of sections.  The alloc section
               will always occur be before data sections. */

      if((sec_header.identifier==DYN_COMMON_SECTION)||(sec_header.identifier==variant) )
      {
         switch(sec_header.type)
         {
         case DYN_SECTION_TYPE_ALLOC_INST:
            /* Access Section Data */
            section_data_ptr  = GetSectionData16(&sec_header);
            /* Verify access to Data */
            if(section_data_ptr==NULL)
            { 
               goto aBort;
            }
            /* Allocate Memory */
            lpcntrl_block = DynAllocateBlocks16((dynmem16_t*)section_data_ptr,root,&root_offset);
            /* Release Section accessor */
            const_data_release(section_data_ptr);
            /* Check for allocation error */
            if(lpcntrl_block==NULL)
            {
               goto aBort;
            }
            break;
         case DYN_SECTION_TYPE_DATA_INST:
            /* Initialize Data */
            if(!DynInitializeBlocks16(&sec_header,lpcntrl_block->allocations))
            {
               goto aBort;
            }
            break;
         case DYN_SECTION_TYPE_RELOC_INST:
            /* Access Section Data */
            section_data_ptr = GetSectionData16(&sec_header);
            /* Verify access to Data */
            if(section_data_ptr==NULL)
            { 
               goto aBort;
            }
            /* Resolve Internal references */
            DynResolveInternalLinks16((dynmem16_t*)section_data_ptr,lpcntrl_block->allocations,lpcntrl_block->allocations);
            /* Release Section accessor */
            const_data_release(section_data_ptr);
            break;
         case DYN_SECTION_TYPE_RELOC_ROOT:
            /* Access Section Data */
            section_data_ptr = GetSectionData16(&sec_header);
            /* Verify access to Data */
            if(section_data_ptr==NULL)
            { 
               goto aBort;
            }
            /* Resolve links from Root */
            DynResolveRootLinks16((dynmem16_t*)section_data_ptr,lpcntrl_block->allocations,sec_header.length);
            /* Release Section accessor */
            const_data_release(section_data_ptr);
            break;
         default:
            break;
         }
      }
     
      /* Skip to next section */
      AdvanceOverSection(&sec_header);
   }

   /* Secontion header may be different size for external data table */
   const_data_release(section_hdr_ptr);
   section_hdr_ptr = NULL;

   /* Handle External References */
   if(ext_desc)
   {
       const_data_descriptor   mem_ext_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,ext_desc);
       if (!external_constant_map_descriptor(&mem_ext_desc))
          goto aBort;

       SetScriptSource(&sec_header,&mem_ext_desc);
       while(1)
       {
          /* Access Section Header */
          section_hdr_ptr = GetSectionHeader32(&sec_header,section_hdr_ptr);
          /* Verify section header access */
          if(section_hdr_ptr==NULL)
          {
             goto aBort;   
          }
          if(sec_header.type == DYN_SECTION_TYPE_END)
          {
             break;
          }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
          if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
          {
             goto aBort;   
          }
#endif
          if( ((sec_header.identifier==DYN_COMMON_SECTION)||(sec_header.identifier==variant) ) &&
              (sec_header.type==DYN_SECTION_TYPE_DATA_INST) )
          {
             /* Access Section Data */
             section_data_ptr = GetSectionData32(&sec_header);
             /* Verify access to Data */
             if(section_data_ptr==NULL)
             { 
                goto aBort;
             }
             /* Resolve External Links  */ 
             DynResolveExternalLinks32((dynmem32_t*)section_data_ptr,lpcntrl_block->allocations,sec_header.length);
             /* Release Section accessor */
             const_data_release(section_data_ptr);
             break;
          }
          /* Skip to next section */
          AdvanceOverSection(&sec_header);
       }
   }

   /* Success */
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }
   return(1);
   /* Failure */
aBort:
   /* Release Memory and clear pointer in root object */
   if(lpcntrl_block)
   {
      DynLoaderReleaseDynamicAllocations(lpcntrl_block);
      root[root_offset] = 0;
   }
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }
   return(0);
}

/****************************************************************************
 * DynLoaderReleaseDynamicAllocations
 */
void DynLoaderReleaseDynamicAllocations(void *ptr)
{
   DYN_PERSISTENT_ALLOC_BLOCK *lpcntrl_block=(DYN_PERSISTENT_ALLOC_BLOCK*)ptr;

   if(lpcntrl_block)
   {
      int      i;
      /* First entry (root) is not alloctaed.   Don't free */
      for(i=1;i<lpcntrl_block->num_allocs;i++)
      {
         if(lpcntrl_block->allocations[i])
         {
            pfree(lpcntrl_block->allocations[i]);
            lpcntrl_block->allocations[i]=NULL;
         }
      }
      /* Release Control Block */
      pfree(lpcntrl_block);

   }
}

/****************************************************************************
 * DynLoaderScratchReserve
 */
int DynLoaderScratchReserve(uintptr_t *root,void *desc,unsigned variant)
{
   DYN_SECTION_HDR   sec_header;
   DYN_SCRATCH_ALLOC_BLOCK *lpcntrl_block=NULL;
   unsigned          root_offset=0;
   uintptr_t         *section_hdr_ptr=NULL;
   uintptr_t         *section_data_ptr;
   int               retval=0;   

   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_16BIT_ZERO_PAD,desc);

   patch_fn_shared(mem_utils);

   if (!external_constant_map_descriptor(&mem_desc))
      goto aBort;

   /* Register with Scratch SubSystem */
   if(!scratch_register())
   {
      return(0);
   }

   /* The scratch sections of the descriptor are
     cashed in RAM for access speed */

   /* Reserve sets up the control block and
      reserves the defined scratch memory */

   /* Handle allocation script */
   SetScriptSource(&sec_header,&mem_desc);
   while(1)
   {
      /* Access Section Header */
      section_hdr_ptr = GetSectionHeader16(&sec_header,section_hdr_ptr);
      /* Verify section header access */
      if(section_hdr_ptr==NULL)
      {
         goto aBort;   
      }
      if(sec_header.type == DYN_SECTION_TYPE_END)
      {
         break;
      }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
      if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
      {
         goto aBort;   
      }
#endif
      /* Note:  There is no need to validate order of sections.  The alloc section
                will always occur be before data sections. */

      if((sec_header.identifier==DYN_COMMON_SECTION)||(sec_header.identifier==variant) )
      {
         switch(sec_header.type)
         {
         case DYN_SECTION_TYPE_ALLOC_SCRATCH:
            /* Allocate a copy of the section */
            section_data_ptr = CopySectionData16(&sec_header);
            /* Verify allocation */
            if(section_data_ptr==NULL)
            {
               goto aBort;
            }
            /* Reserve Scratch Memory */
            lpcntrl_block = DynReserveScratch16((dynmem16_t*)section_data_ptr,root,&root_offset);
            if(lpcntrl_block==NULL)
            {
               goto aBort;
            }
            break;
         case DYN_SECTION_TYPE_RELOC_SCRATCH:
            /* Allocate a copy of the section */
            section_data_ptr = CopySectionData16(&sec_header);
            /* Verify allocation */
            if(section_data_ptr==NULL)
            {
               goto aBort;
            }
            /* Save section pointer - used in commit */
            if(sec_header.identifier==DYN_COMMON_SECTION)
            {
               lpcntrl_block->common_link_section   = section_data_ptr;
            }
            else
            {
               lpcntrl_block->variant_link_sections = section_data_ptr;
            }
            break;
         default:
            break;
         }
      }
      /* Skip to next section */
      AdvanceOverSection(&sec_header);
   }

   /* Success */
   retval = 1;

aBort:
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }

   if(lpcntrl_block==NULL)
   {
      /* Cleanup, for error before contrl block is allocated or no scratch */
      scratch_deregister();
   }
   else if(!retval)
   {
      /* Error after cntrl block was allocated. */
      DynLoaderDeregisterScratch(lpcntrl_block);
      root[root_offset] = 0;
   }
   return(retval);
}

/****************************************************************************
 * DynLoaderScratchCommit
 */
void DynLoaderScratchCommit(void *inst_ptr,void *scr_ptr)
{
   DYN_SCRATCH_ALLOC_BLOCK    *lpcntrl_block = (DYN_SCRATCH_ALLOC_BLOCK*)scr_ptr;
   DYN_PERSISTENT_ALLOC_BLOCK *lp_instcntrl  = (DYN_PERSISTENT_ALLOC_BLOCK*)inst_ptr;

   if(lpcntrl_block==NULL)
   {
      return;
   }

   /* handle chached sections */
   if(!DynCommitScratch(lpcntrl_block))
   {
      return;
   }

   /* Resolve links to scratch memory */
   if(lpcntrl_block->common_link_section)
   {
      DynResolveInternalLinks16((dynmem16_t*)lpcntrl_block->common_link_section,lp_instcntrl->allocations,lpcntrl_block->allocations);
   }
   if(lpcntrl_block->variant_link_sections)
   {
      DynResolveInternalLinks16((dynmem16_t*)lpcntrl_block->variant_link_sections,lp_instcntrl->allocations,lpcntrl_block->allocations);
   }
}

/****************************************************************************
 * DynLoaderDeregisterScratch
 */
void DynLoaderDeregisterScratch(void *ptr)
{
   DYN_SCRATCH_ALLOC_BLOCK *lpcntrl_block=(DYN_SCRATCH_ALLOC_BLOCK*)ptr;   
   
   if(!lpcntrl_block)
   {
      return;
   }

   /* Deregister Scratch Access */
   scratch_deregister();

   /* Release Cached sections */
   if(lpcntrl_block->alloc_section)
   {
      pfree(lpcntrl_block->alloc_section);
   }
   if(lpcntrl_block->common_link_section)
   {
      pfree(lpcntrl_block->common_link_section);
   }
   if(lpcntrl_block->variant_link_sections)
   {
      pfree(lpcntrl_block->variant_link_sections);
   }
   /* Release Control Block */
   pfree(lpcntrl_block);
}



/****************************************************************************
 * DynLoaderProcessSharedAllocations
 */
int DynLoaderProcessSharedAllocations(uintptr_t **address,void *desc,void *ext_desc,unsigned share_id)
{
   DYN_SECTION_HDR   sec_header;
   unsigned          num_refs=0;
   bool              bNewBlock=TRUE;
   dynmem16_t       *share_refs=NULL;
   uintptr_t        *section_hdr_ptr=NULL;
   uintptr_t        *section_data_ptr;
   int               retval=0;
   uintptr_t        *share_mem_ptr=NULL;

   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_16BIT_ZERO_PAD,desc);

   patch_fn_shared(mem_utils);

   if (!external_constant_map_descriptor(&mem_desc))
      goto aBort;

   /* Handle allocation script */
   SetScriptSource(&sec_header,&mem_desc);
   while(bNewBlock)
   {
      /* Access Section Header */
      section_hdr_ptr = GetSectionHeader16(&sec_header,section_hdr_ptr);
      /* Verify section header access */
      if(section_hdr_ptr==NULL)
      {
         goto aBort;   
      }
      if(sec_header.type == DYN_SECTION_TYPE_END)
      {
         break;
      }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
      if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
      {
         goto aBort;   
      }
#endif

      /* Note:  There is no need to validate order of sections.  The alloc section
                will always occur be before data sections. */

      if(sec_header.identifier==share_id)
      {
         if(sec_header.type==DYN_SECTION_TYPE_ALLOC_SHARE)
         {
            unsigned size,bank;
            
            /* +-------------------------------------------------------------------+ 
             * | Alloc size  | Bank/Num Refs | Ref ID    | Ref Offset | Ref Addend | 
             * +-------------------------------------------------------------------+ */
            
            /* Access Section Data - Remember section for later usage */
            share_refs  = (dynmem16_t*)GetSectionData16(&sec_header);
            /* Verify access to Data */
            if(share_refs==NULL)
            { 
                goto aBort; 
            }
            /* Allocate shared memory */
            size = (unsigned)(share_refs[0] & 0xFFFF);
            bank = (unsigned)(share_refs[1] & 0xFFFF);
            num_refs = bank & 0xFF;
            
            if((share_mem_ptr = (uintptr_t*)shared_zmalloc(size*sizeof(unsigned),MapMemBank(bank>>8),share_id,&bNewBlock))==NULL)
            {
                /* Failed to allocate shared variable.  Release Section accessor */
                goto aBort; 
            }
         }
         else if(sec_header.type==DYN_SECTION_TYPE_DATA_SHARE)
         {
            /* Initialize Data */
            if(!DynInitializeBlocks16(&sec_header,&share_mem_ptr))
            {
               goto aBort;
            }  
            /* There is only one data section per share id, terminate parsing of sections */
            break; 
         }
      }
     
      /* Skip to next section */
      AdvanceOverSection(&sec_header);
   }

   /* Secontion header may be different size for external data table */
   const_data_release(section_hdr_ptr);
   section_hdr_ptr = NULL;

   /* Handle External References in shared */
   if(ext_desc)
   {
       const_data_descriptor   mem_ext_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST,FORMAT_DSP_NATIVE,ext_desc);
       if (!external_constant_map_descriptor(&mem_ext_desc))
          goto aBort;

       SetScriptSource(&sec_header,&mem_ext_desc);
       while(bNewBlock)
       {
          /* Access Section Header */
          section_hdr_ptr = GetSectionHeader32(&sec_header,section_hdr_ptr);
          /* Verify section header access */
          if(section_hdr_ptr==NULL)
          {
             goto aBort;   
          }
          if(sec_header.type == DYN_SECTION_TYPE_END)
          {
             break;
          }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
          if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
          {
             goto aBort;   
          }
#endif

          if( (sec_header.identifier==share_id) && (sec_header.type==DYN_SECTION_TYPE_DATA_SHARE) )
          {
             /* Access Section Data */
             section_data_ptr = GetSectionData32(&sec_header);
             /* Verify access to Data */
             if(section_data_ptr==NULL)
             { 
                goto aBort;
             }
             /* Resolve External Links  */ 
             DynResolveExternalShare32((dynmem32_t*)section_data_ptr,share_mem_ptr,sec_header.length);
             /* Release Section accessor */
             const_data_release(section_data_ptr);
             
             /* There is only one data section per share id, terminate parsing of sections */
             break; 
          }
          /* Skip to next section */
          AdvanceOverSection(&sec_header);
       }
   }

   /* Handle references to other shared variables */

   /* if(bNewBlock)   TODO - when shared detructor is supported */
   {
      int      i;
      dynmem16_t *lprefs;
            
      /* +-------------------------------------------------------------------+ 
       * | Alloc size  | Bank/Num Refs | Ref ID    | Ref Offset | Ref Addend | 
       * +-------------------------------------------------------------------+*/

      /* Skip over allocation info */
      lprefs = share_refs+2;

      /* Allocate references */
      for(i=0;i<num_refs;i++)
      {
         unsigned ref_id;
         uintptr_t *addr;
         unsigned offset;
         int      Addend;

         ref_id = (unsigned)(*(lprefs++) & 0xFFFF);

         if(!DynLoaderProcessSharedAllocations(&addr,desc,ext_desc,ref_id))
         {
            /* Error.  
               Need to release referenced shares that were successfully allocated  */
            ReleaseSharedReferences16(desc,share_refs,i);
            goto aBort;
         }

         /* Save Reference */
         offset  = (unsigned)(*(lprefs++) & 0xFFFF);
         Addend  = sign_ext_16bit(*(lprefs++));

         *(share_mem_ptr + offset) = (uintptr_t)(addr + Addend);
      }
   }
   
   /* Success */
   retval=1;
   
 aBort:
   if(!retval && share_mem_ptr)
   {
       /* Release Shared Memory (no Destructor) */
       shared_free(share_mem_ptr);
       share_mem_ptr = NULL;
   }
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }
   if(share_refs)
   {
      const_data_release(share_refs);
   }
   *address = share_mem_ptr;
   return(retval);
}


/****************************************************************************
 * DynLoaderReleaseSharedAllocations
 */
int DynLoaderReleaseSharedAllocations(unsigned share_id,void *desc)
{
   DYN_SECTION_HDR   sec_header;
   uintptr_t         *section_hdr_ptr=NULL;
   uintptr_t         *section_data_ptr;

   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_16BIT_ZERO_PAD,desc);

   patch_fn_shared(mem_utils);

   if (!external_constant_map_descriptor(&mem_desc))
      goto aBort;

   /* Handle allocation script */
   SetScriptSource(&sec_header,&mem_desc);
   while(1)
   {
      /* Access Section Header */
      section_hdr_ptr = GetSectionHeader16(&sec_header,section_hdr_ptr);
      /* Verify section header access */
      if(section_hdr_ptr==NULL)
      {
         goto aBort;   
      }
      if(sec_header.type == DYN_SECTION_TYPE_END)
      {
         break;
      }
#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT
      if(sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
      {
         goto aBort;   
      }
#endif
      /* Note:  There is no need to validate order of sections.  The alloc section
                will always occur be before data sections. */
      
      if( ((sec_header.identifier==DYN_COMMON_SECTION)||(sec_header.identifier==share_id) ) &&
          (sec_header.type==DYN_SECTION_TYPE_ALLOC_SHARE) )
      {
            unsigned    num_refs;
            dynmem16_t *share_data_ptr;

            /* Access Section Data */
            section_data_ptr = GetSectionData16(&sec_header);
            /* Verify access to Data */
            if(section_data_ptr==NULL)
            { 
               goto aBort;
            }


            /* TODO - move references into destructor when allowed */

            shared_free_by_id(share_id);

            share_data_ptr = (dynmem16_t*)section_data_ptr;
            /* +-------------------------------------------------------------------+ 
             * | Alloc size  | Bank/Num Refs | Ref ID    | Ref Offset | Ref Addend | 
             * +-------------------------------------------------------------------+*/
            num_refs = (unsigned)(share_data_ptr[1] & 0xFF);
            ReleaseSharedReferences16(desc,share_data_ptr,num_refs);
            /* Release Section accessor */
            const_data_release(section_data_ptr);
            break;
      }
     
      /* Skip to next section */
      AdvanceOverSection(&sec_header);
   }
   
   /* Success */
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }
   return(1);
   /* Failure */
aBort:
   if(section_hdr_ptr)
   {
      const_data_release(section_hdr_ptr);
   }

   /* TODO - should panic????,  Failure in accessor memory not released*/
   return(0);
}

#ifdef INSTALL_CAPABILITY_CONSTANT_EXPORT

bool DynLoaderHasExternalRedirect( void *table)
{
    return DynLoaderGetExternalRedirectFile(table,NULL);
}

/* Does the referenced table start with an entry indicating exported to filesystem */
bool DynLoaderGetExternalRedirectFile(void *table,uint16 *pFile)
{
   DYN_SECTION_HDR   sec_header;
   uintptr_t        *section_hdr_ptr=NULL;

   const_data_descriptor   mem_desc = DEFINE_CONST_DATA_DESCRIPTOR(MEM_TYPE_CONST16,FORMAT_16BIT_ZERO_PAD,table);
   SetScriptSource(&sec_header,&mem_desc);
   section_hdr_ptr = GetSectionHeader16(&sec_header,section_hdr_ptr);
   if (sec_header.type == DYN_SECTION_EXPORT_CONSTANTS)
   {
      if (pFile)
      {
          uint16 *data = (uint16*)GetSectionData16(&sec_header);
          *pFile = *data;
      }
      return TRUE;
   }
   return FALSE;
}

#endif




