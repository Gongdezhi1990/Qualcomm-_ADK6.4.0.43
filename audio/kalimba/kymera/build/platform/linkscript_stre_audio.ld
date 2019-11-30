/*
 * Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 * All Rights Reserved.
 * Qualcomm Technologies International, Ltd. Confidential and Proprietary.
 * Notifications and licenses are retained for attribution purposes only
 */
/*
 * Common link script for running P0 and P1 fron common image or from
 * separate images.
 * All Memory sizes in the link script are in Bytes.
 */
OUTPUT_ARCH(QCC302x_audio)

ENTRY($reset)

/*
 * Generate a simple crc32 of the contents of all code and data sections
 * in the final linked image.
 * The global symbol __devtools_image_checksum is added to the symbol table
 * with the crc32 value in
 * section dm. For more info see https://ukbugdb/B-198383
 */
CHECKSUM_DATA_SECTION(.const);

/* Description of the physical characteristics of the chip: data memory */
$PHYSICAL_DMRAM_INSTANCE_SIZE = 0x8000;    /* 32 KiB */
$DATA_BANK_NUMBER             = 8;

/* Description of the physical characteristics of the chip: program memory */
$PHYSICAL_PMRAM_INSTANCE_SIZE = 0x2000;  /* 8 KiB */
$PM_BANK_NUMBER               = 10;

/* Description of the physical characteristics of the chip: cache */
$DIRECT_CACHE_SIZE                = 0x0800; /* 2 KiB */
$DIRECT_CACHE_SIZE_IN_BANKS       = 1;
$TWO_WAY_CACHE_SIZE               = 0x8800; /* 34 KiB */
$TWO_WAY_CACHE_SIZE_IN_BANKS      = 5;
$TWO_WAY_HALF_CACHE_SIZE_IN_BANKS = 3;
$TWO_WAY_HALF_CACHE_SIZE          = $TWO_WAY_HALF_CACHE_SIZE_IN_BANKS * $PHYSICAL_PMRAM_INSTANCE_SIZE;

/* Base Addresses */
$DM1_RAM_BASE_ADDRESS         = 0x00000000; /* Start of DM RAM accessed by DM1 bus */
$DM2_RAM_BASE_ADDRESS         = 0xFFF00000; /* Start of DM RAM accessed by DM2 bus */
$PM_RAM_START_ADDRESS         = 0x04000000; /* Start of PM RAM accessed by PM bus */
$DM_WINDOW_TO_PM_BASE_ADDRESS = 0x00100000; /* Start of PM RAM accessed by DM bus */
$DM_NVRAM0_START_ADDRESS      = 0xF8000000; /* Start of ROM or SQIF accessed by DM bus */

$PM_RAM_END_ADDRESS           = $PM_RAM_START_ADDRESS +
                                $PM_BANK_NUMBER * $PHYSICAL_PMRAM_INSTANCE_SIZE;


$DM_MEM_BLOCK_SIZE           = 0x1000;    /* 4 KiB = private memory size*/


/* Private memory on DM1*/
$DM1_P0_PRIVATE_MEMORY_END_ADDRESS = $DM1_RAM_BASE_ADDRESS + $DM_MEM_BLOCK_SIZE;
$DM1_P1_PRIVATE_MEMORY_END_ADDRESS = $DM1_P0_PRIVATE_MEMORY_END_ADDRESS + $DM_MEM_BLOCK_SIZE;
$DM1_PRIVATE_MEMORY_END_ADDRESS    = DEFINED( P1_STANDALONE ) ?
                                     $DM1_P1_PRIVATE_MEMORY_END_ADDRESS :
                                     $DM1_P0_PRIVATE_MEMORY_END_ADDRESS ;

/* If dual core is not enabled keep all statics together, so give space */
$DM1_PUBLIC_MEM_START_ADDRESS  = $DM1_P1_PRIVATE_MEMORY_END_ADDRESS ;
$DM1_PRIVATE_GUARD_END_ADDRESS = $DM1_PRIVATE_MEMORY_END_ADDRESS ;
$DM1_PRIVATE_GUARD_ADDRESS     = $DM1_PRIVATE_GUARD_END_ADDRESS - 4;

/* Handle the defined symbols */
$CODE_ROM_BASE                 = 0x00000000;
$MEM_MAP_P1_CODE_START_ADDRESS = DEFINED ( P1_CODE_START_ADDRESS ) ?
                                 P1_CODE_START_ADDRESS : 0;
$CODE_P0_BASE                  = $CODE_ROM_BASE;
$CODE_BASE                     = DEFINED (P1_STANDALONE)?
                                 $MEM_MAP_P1_CODE_START_ADDRESS :
                                 $CODE_P0_BASE;

/* The caches can work in 3 different modes:
   - Direct cache consume 2 KiB in the first and last Program Memory (PM) bank.
   It is useful when running from ROM as it gives good enough performance
   and leave plenty of space for Downloadable capabilities.
   - 2-Way cache consume an additional 32 KiB per processor in the following
   banks at the bottom and the top. It is useful when running from SQIF as
   it mitigates the performance loss.
   - 2-Way Half cache is similar to the previous but consumes only 16 KiB
   per processor. It is useful when running from SQIF on a chip with little
   PM available.

   Bank 0 is a special case. It contains both some cache and some function needed
   to activate and desactive the caches. The banks not used for cache are split
   between the 2 processors either evenly or according to the
   "PM_RAM_CODE_P0_END_BANK" configuration variable. */

/* On Stre, PM banks are 8 KiB so:
   - 2-Way cache uses banks 0 to 4 for P0 and banks 5 to 9 for P1. Leaving
   no space for downloadable capabilities.
   - 2-Way Half cache uses banks 0 to 2 for P0 and banks 7 to 9 for P1. Sadly
   there seem to be a bug in hardware that makes it unusable. */
$CACHE_SIZE_IN_BANKS           = DEFINED(USE_2WAYHALF_CACHE) ?
                                 $TWO_WAY_HALF_CACHE_SIZE_IN_BANKS :
                                 $DIRECT_CACHE_SIZE_IN_BANKS;
$CACHE_SIZE_IN_BANKS           = DEFINED(USE_2WAY_CACHE) ?
                                 $TWO_WAY_CACHE_SIZE_IN_BANKS :
                                 $CACHE_SIZE_IN_BANKS;

/* Give one extra bank for patches only if 2 WAY CACHE is not defined */
$PM_PATCH_BANK_SIZE           =  DEFINED(USE_2WAY_CACHE) ? 0: 1;


$PM_P0_LAST_BANK               = DEFINED ( PM_RAM_CODE_P0_END_BANK ) ?
                                 PM_RAM_CODE_P0_END_BANK :
                                 ($PM_BANK_NUMBER / 2) + $PM_PATCH_BANK_SIZE;

ASSERT(($PM_P0_LAST_BANK >= $CACHE_SIZE_IN_BANKS) && 
       ($PM_P0_LAST_BANK <= ($PM_BANK_NUMBER - $CACHE_SIZE_IN_BANKS)),
	   "$PM_P0_LAST_BANK must be outside of the area reserved for the caches.")
$PM_RAM_CODE_P0_2WAYCACHE_START_ADDRESS  = $PM_RAM_START_ADDRESS + $TWO_WAY_HALF_CACHE_SIZE;
$PM_RAM_CODE_P0_END_ADDRESS    = $PM_RAM_START_ADDRESS +
                                 $PM_P0_LAST_BANK * $PHYSICAL_PMRAM_INSTANCE_SIZE;

$CACHE_SIZE_OFFSET             = DEFINED(USE_2WAYHALF_CACHE)? $TWO_WAY_HALF_CACHE_SIZE : $DIRECT_CACHE_SIZE; 
$CACHE_SIZE_OFFSET             = DEFINED(USE_2WAY_CACHE)? $TWO_WAY_CACHE_SIZE : $CACHE_SIZE_OFFSET;
                                 

$PM_RAM_CODE_P1_START_ADDRESS  = $PM_RAM_CODE_P0_END_ADDRESS;
$PM_RAM_CODE_P1_END_ADDRESS    = $PM_RAM_END_ADDRESS - $CACHE_SIZE_OFFSET;

/* The PM bank used either by the direct cache of P0 or the tags of the 2-Way cache
   and 2-Way half cache also contains functions to enable and disable the cache.
   The functions are called once so this will not degrade performance. */
$PM_CACHE_FUNC_OFFSET          = DEFINED(USE_2WAY_CACHE) ? $TWO_WAY_CACHE_SIZE : $DIRECT_CACHE_SIZE;
$PM_CACHE_FUNC_OFFSET          = DEFINED(USE_2WAYHALF_CACHE) ? $TWO_WAY_HALF_CACHE_SIZE : $PM_CACHE_FUNC_OFFSET;
$PM_CACHE_FUNC_START           = $PM_RAM_START_ADDRESS + $PM_CACHE_FUNC_OFFSET;
								 
$DM1_RAM_START_ADDRESS         = DEFINED( P1_STANDALONE )?
                                 $DM1_P0_PRIVATE_MEMORY_END_ADDRESS :
                                 $DM1_RAM_BASE_ADDRESS;
$DATA_START_ADDRESS            = $DM_NVRAM0_START_ADDRESS;
$CONST_START_BASE              = $DM_NVRAM0_START_ADDRESS + $CODE_BASE;
$PM_DATA_START_BASE            = $DM_NVRAM0_START_ADDRESS;

/* Stack, preserved areas, pools and heaps are defined relative
   to 4 fixed anchors. */
$ANCHOR_DM1_START = 4 * $PHYSICAL_DMRAM_INSTANCE_SIZE;
$ANCHOR_DM1_END   = 8 * $PHYSICAL_DMRAM_INSTANCE_SIZE;
$ANCHOR_DM2_START = $DM2_RAM_BASE_ADDRESS +
                    1 * $PHYSICAL_DMRAM_INSTANCE_SIZE;
$ANCHOR_DM2_END   = $DM2_RAM_BASE_ADDRESS +
                    4 * $PHYSICAL_DMRAM_INSTANCE_SIZE;

$DM1_RESERVED_PATCH_SIZE = PATCH_RESERVED_RAM;

/* Preserve block and stack. The size here is sufficiently large,
 * actual preserve block is only 24 bytes (6 words)
 * keep a gap between heap and stack. The hardware must be protecting the
 * stack from heap
 * overwrite,but still keep a gap
 */
$SPACE_FOR_STACK           = 0x0920; /* 2304 stack + 24 for preserve block + 8 gap*/
$STACK_GAP                 = 0x0008; /* Keep an extra double word as stack gap */
$P0_STACK_START_ADDRESS    = $ANCHOR_DM1_START + $STACK_GAP;
$P1_STACK_START_ADDRESS    = $ANCHOR_DM2_END - $SPACE_FOR_STACK;
                             
/* Symbol lookup table (SLT) offset from start of ROM */
$AUDIO_SLT_OFFSET = 0x20;

/* This magic layout offset for $CODE_MAGIC$ tells the linker that this is
 * in the program address space rather than the data address space */
$CODE_MAGIC$      = 0x80000000;


SECTIONS
{
    /* Code starts at ROM address zero (aka LMA - Load Memory Address) */
    __CODE_load_address = $CODE_BASE;

    . = ALIGN($CODE_MAGIC$ + __CODE_load_address, 0x4);
    /* Kalimba objects create a unique section name for each function/symbol
     * These are of the form <CODESEGMENT_NAME>?<function/symbol_name>
     * Hence always match the '?' and trailing chars
     *
     * Reset code is usually maxim and then it changes to minim. So we need
     * different alignment for the different sections     */
    .text_reset : AT( __CODE_load_address )
    {
        $MEM_MAP_CODE_START = ABSOLUTE(.);
        *(RESET_PM\?*)
    }

    __CODE_reset_size = SIZEOF(.text_reset);

    /* This is a duplicate copy for P1 but the code should look in the
     * P0 location
     */
    . = ALIGN( $CODE_MAGIC$ + __CODE_load_address + $AUDIO_SLT_OFFSET , 0x4);
    __CONST_SLT_rom_address = ( __CODE_load_address + $AUDIO_SLT_OFFSET );
    .const_slt : AT( __CONST_SLT_rom_address )
    {
        /* Put the symbol look up table between reset code and rest of code */
        KEEP(*(CONST_SLT\?*))
    }

    __CONST_SLT_size = SIZEOF(.const_slt);

    .text_minim :
    {
        KEEP(*.o(PM_KEEP\?*__minim))
        *.o(*pm*\?*__minim)
        *.o(*PM*\?*__minim)
    }

    .text_minim_private ALIGN(4):
    {
        /* Separate code section for scrambled libraries */
        KEEP(*.po(PM_KEEP\?*__minim))
        *.po(*pm*\?*__minim)
        *.po(*PM*\?*__minim)
        *plc100.po(*pm\?*)
    }

    .text_maxim ALIGN(4):
    {
        /* we want to include all the sections except the PM_RAM ones in
         * .text_maxim. PM_RAM needs to after all this, so that the
         * rest of the build process can pick it up and do things with
         * it (dm_append, etc). Thats why we dont use the only and only rule
         * *(*__maxim)
         */
        *.o(PM_MAXIM\?*__maxim)
        *.o(*PM_FLASH\?*__maxim)
        *.o(*PM\?*__maxim)
    }
    .text_maxim_private ALIGN(4):
    {
        /* Separate code section for scrambled libraries*/
        *.po(*PM_FLASH\?*)
        *.po(*PM\?*)
        $MEM_MAP_CODE_END = ABSOLUTE(.);
    }

    /* Next ROM address (LMA) immediately follows on from last LMA of .text */
    __CODE_ROM_SIZE = ( $MEM_MAP_CODE_END - $MEM_MAP_CODE_START );

    /* Next section is Const section, which immediately follows on from last
     * LMA of .text.  */
    __CONST_rom_address = ALIGN(__CODE_load_address + __CODE_ROM_SIZE , 4);


    /* Const values are accessed via the NVMEM window in DM space.
     * This is a window in the data space that maps onto the ROM
     * so we advance into it the same amount as the offset in the
     * program memory.
     */
    . = $CONST_START_BASE + __CODE_ROM_SIZE;

    .const ALIGN(4): AT ( __CONST_rom_address )
    {
        $MEM_MAP_CONST_START = ABSOLUTE(.);
        *.o(DMCONST*)
        *.o(dmconst*)
        *.o(const*)
        *.o(CONST*)
    }
    .const_private ALIGN(4):
    {
        *.po(DMCONST*)
        *.po(dmconst*)
        *.po(const*)
        *.po(CONST*)
        $MEM_MAP_CONST_END = ABSOLUTE(.);
    }

    __CACHE_INIT_address = __CONST_rom_address +
                           ALIGN(SIZEOF(.const), 4) +
						   ALIGN(SIZEOF(.const_private), 4);
    . = ALIGN($CODE_MAGIC$ + $PM_CACHE_FUNC_START, 4);
    .text_cacheinit : AT(__CACHE_INIT_address)
    {
        $PM_RAM_CACHE_INIT_START = ABSOLUTE(.) - $CODE_MAGIC$;
        *(PM_CACHEINIT\?*)
    }

    /* Next ROM address (LMA) immediately follows on from last LMA of .const
     * Next section is code, but needs to be loaded into PM RAM */
    __CODE_RAM_address = __CONST_rom_address +
                         ALIGN(SIZEOF(.const), 4) +
                         ALIGN(SIZEOF(.const_private), 4) +
                         ALIGN(SIZEOF(.text_cacheinit), 4);

    $PM_RAM_CODE_P0_START_ADDRESS = DEFINED( USE_2WAYHALF_CACHE) ?
                                 $PM_RAM_CODE_P0_2WAYCACHE_START_ADDRESS :
                                 ABSOLUTE(.) - $CODE_MAGIC$;

    __CODE_RAM_load_address = $CODE_MAGIC$ + $PM_RAM_CODE_P0_START_ADDRESS;

    . = ALIGN(__CODE_RAM_load_address, 4);
    .text_ram : AT(__CODE_RAM_address)
    {
        $PM_RAM_P0_CODE_START = ABSOLUTE(.) - $CODE_MAGIC$;
        *(PM_RAM\?*)
    }

    /* Use a different section for the pm heap to keep 32-bit alignment
     * It needs to be copied to ram, so set PM_RAM_CODE_END at the
     * end of this section
     * This section needs to be the last PM instruction */
    __MALLOC_PM_address = ALIGN(__CODE_RAM_address + SIZEOF(.text_ram), 4);
    /* Keep VMA in sync with LMA */

    $PM_HEAP_SIZE = $PM_RAM_CODE_P0_END_ADDRESS -
                    $PM_RAM_P0_CODE_START -
                    SIZEOF(.text_ram);

    .text_malloc_pm : AT(__MALLOC_PM_address)
    {
        $PM_RAM_PATCH_CODE_START = ABSOLUTE(.);
        *(CODE_HEAP\?*)
    }

    __CODE_RAM_SIZE = SIZEOF(.text_ram) + SIZEOF(.text_malloc_pm);

    __CODE_P1_RAM_address = ALIGN( __CODE_RAM_address + __CODE_RAM_SIZE , 4);

    /* Do the same for P1 */
    $PM_RAM_CODE_P1_START_ADDRESS = $PM_RAM_CODE_P1_START_ADDRESS;

    . = ALIGN( $CODE_MAGIC$ + $PM_RAM_CODE_P1_START_ADDRESS, 4 );
    .text_p1_ram : AT(__CODE_P1_RAM_address)
    {
        $PM_RAM_P1_CODE_START = ABSOLUTE(.) - $CODE_MAGIC$;
        *(PM_RAM_P1\?*)
    }

    $PM_P1_HEAP_SIZE = $PM_RAM_CODE_P1_END_ADDRESS -
                       $PM_RAM_CODE_P1_START_ADDRESS -
                       SIZEOF(.text_p1_ram);

    /* P1 PM heap */
    __MALLOC_P1_PM_address = ALIGN(__CODE_P1_RAM_address + SIZEOF(.text_p1_ram), 4);
    .text_p1_malloc_pm : AT(__MALLOC_P1_PM_address)
    {
        *(CODE_HEAP_P1\?*)
        $PM_RAM_P1_CODE_END = ABSOLUTE(.) - $CODE_MAGIC$;
    }

    /* Length of code which crt1 (P1) has to copy from NV to PM RAM */
    __CODE_P1_RAM_SIZE = SIZEOF(.text_p1_ram) + SIZEOF(.text_p1_malloc_pm);

    /* Next ROM address (LMA) immediately follows on from last LMA of .text_ram */
    __DM2_rom_address = ALIGN(__CODE_P1_RAM_address + __CODE_P1_RAM_SIZE, 4);

    /* But the VMA is different: the init data is copied to RAM so the VMA
     * doesn't depend on the LMA */
    . = $DM1_RAM_START_ADDRESS + $DM2_RAM_BASE_ADDRESS;

    /* OBPM requires info_block_ptr (marked as DM_SPI_COMMS) to be at start of RAM */
    .initc_dm2_private : AT (__DM2_rom_address)
    {
        $MEM_MAP_DM2_INITC_START = ABSOLUTE(.);
        *(*DM_SPI_COMMS*)
        *.po(*DM2\?*)
        *.po(*dm2\?*)
    }
    .initc_dm2 ALIGN(4):
    {
        *.o(*DM2\?*)
        *.o(*dm2\?*)
    }

    /* zeroinit DM2 */
    .zeroinit_dm2 ALIGN(4) (NOLOAD) :
    {
        $MEM_MAP_DM2_BSS_START = ABSOLUTE(.);
        *.o(dm2zi*)
        *.o(DM2ZI*)
    }
    .zeroinit_dm2_private ALIGN(4) (NOLOAD) :
    {
        *.po(dm2zi*)
        *.po(DM2ZI*)
        $MEM_MAP_DM2_BSS_END = ABSOLUTE(.);
    }

    __DM1_rom_address = __DM2_rom_address +
                        ALIGN(SIZEOF(.initc_dm2_private),4) + ALIGN(SIZEOF(.initc_dm2), 4);

    . = $DM1_RAM_START_ADDRESS +
        ALIGN(SIZEOF(.initc_dm2_private), 4) + ALIGN(SIZEOF(.initc_dm2),4) +
        ALIGN(SIZEOF(.zeroinit_dm2), 4) + ALIGN(SIZEOF(.zeroinit_dm2_private), 4);

    .initc_dm1 : AT (__DM1_rom_address)
    {
        $MEM_MAP_DM1_INITC_START = ABSOLUTE(.);
        *.o(*DM1\?*)
        *.o(*dm1\?*)
        *.o(DM\?*)
        *.o(dm\?*)
    }
    .initc_dm1_private ALIGN(4) :
    {
        *.po(*DM1\?*)
        *.po(*dm1\?*)
        *.po(DM\?*)
        *.po(dm\?*)
        $MEM_MAP_DM1_INITC_END = ABSOLUTE(.);
    }

    /* zeroinit DM1 */
    .zeroinit_dm1 ALIGN(4) (NOLOAD):
    {
        $MEM_MAP_DM1_BSS_START = ABSOLUTE(.);
        *.o(dmzi*)
        *.o(DMZI*)
    }
    .zeroinit_dm1_private ALIGN(4) (NOLOAD):
    {
        *.po(dmzi*)
        *.po(DMZI*)
        $MEM_MAP_DM1_BSS_END = ABSOLUTE(.);
    }

    /* Make sure that the end of zeroinit DM1 in private RAM has not gone past the private memory guard
     * Note that ASSERT does not require a semicolon at the end */
    ASSERT(($MEM_MAP_DM1_BSS_END<=$DM1_PRIVATE_GUARD_ADDRESS),"DM1 zero init extends past private memory guard")

    __DM1_guard_rom_address = __DM1_rom_address + ALIGN(SIZEOF(.initc_dm1),4) + ALIGN(SIZEOF(.initc_dm1_private), 4);
    . = $DM1_PRIVATE_GUARD_ADDRESS;

    /* Add a private window guard to ensure it doesn't overlap with any other
     * region
     **/
    .initc_mem_guard : AT( __DM1_guard_rom_address )
    {
        $MEM_MAP_DM_GUARD_START = ABSOLUTE(.);
        *(*DM_MEM_GUARD\?*)
        $MEM_MAP_DM_GUARD_END = ABSOLUTE(.);
    }

    /* Area for IPC_LUT and P1 only static, P0 never access this area */
    __DM1_P0_rom_address = ALIGN( __DM1_guard_rom_address + SIZEOF(.initc_mem_guard), 4);

    . = ALIGN($DM1_PUBLIC_MEM_START_ADDRESS , 4);
    .initc_p0 : AT( __DM1_P0_rom_address )
    {
        $MEM_MAP_DM1_P0_INITC_START = ABSOLUTE(.);
        *.o(*DM_SHARED\?*)
        *.o(*DM1_P0_RW\?*)
        *.o(*DM_P0_RW\?*)
        *.o(*DM_IPC_LUT*)
    }

    .initc_p0_private ALIGN(4):
    {
        *.po(*DM_SHARED\?*)
        *.po(*DM1_P0_RW\?*)
        *.po(*DM_P0_RW\?*)
        *.po(*DM_IPC_LUT*)
        $MEM_MAP_DM1_P0_PRIVATE_INITC_END = ABSOLUTE(.);
    }
    $MEM_MAP_DM1_P0_PRIVATE_INITC_END = ALIGN($MEM_MAP_DM1_P0_PRIVATE_INITC_END, 4);

    .zeroinit_p0 ALIGN(4) (NOLOAD) :
    {
        $MEM_MAP_DM1_P0_BSS_START = ABSOLUTE(.);
        *.o(*DM_SHARED_ZI*)
        *.o(*DM1_P0_RW_ZI*)
        *.o(*DM_P0_RW_ZI*)
    }
    .zeroinit_p0_private ALIGN(4) (NOLOAD) :
    {
        *.po(*DM_SHARED_ZI*)
        *.po(*DM1_P0_RW_ZI*)
        *.po(*DM_P0_RW_ZI*)
        $MEM_MAP_DM1_P0_BSS_END_ACTUAL = ABSOLUTE(.);
        . = ALIGN(.,4);
        $PATCH_RESERVED_DM_START = .;
        . = . + $DM1_RESERVED_PATCH_SIZE;
        $PATCH_RESERVED_DM_END = .;
        $MEM_MAP_DM1_P0_BSS_END = ABSOLUTE(.);
    }

    __DM2_P1_rom_address = ALIGN(__DM1_P0_rom_address + SIZEOF(.initc_p0), 4);
    $DM2_P1_INITC_START_ADDRESS  = $DM2_RAM_BASE_ADDRESS + $MEM_MAP_DM1_P0_BSS_END;
    . = ALIGN($DM2_P1_INITC_START_ADDRESS, 4);

    .initc_p1 : AT (__DM2_P1_rom_address)
    {
        $MEM_MAP_DM2_P1_INITC_START = ABSOLUTE(.);
        *(*DM2_P1_RW\?*)
        *(*DM_P1_RW\?*)
        $MEM_MAP_DM2_P1_INITC_END = ABSOLUTE(.);
    }

    __initc_p1_address = ALIGN(__DM2_P1_rom_address + SIZEOF(.initc_p1), 4);
    .zeroinit_p1 (NOLOAD) : AT (__initc_p1_address)
    {
        $MEM_MAP_DM2_P1_BSS_START = ABSOLUTE(.);
        *.o(*DM2_P1_RW_ZI*)
        *.o(*DM_P1_RW_ZI\?*)
    }
    .zeroinit_p1_private (NOLOAD) :
    {
        *.po(*DM2_P1_RW_ZI*)
        *.po(*DM_P1_RW_ZI\?*)
        $MEM_MAP_DM2_P1_BSS_END = ABSOLUTE(.);
    }

    /*******************************************
	  Dynamic DM1 Area:
	  - P0 stack
	  - P0 preserved area
	  - DM1 pools for P0
	  - DM1 heap for P0
	  - DM1 heap for P1
	  - DM1 pools for P1

	  The size of both pools MUST be identical.
	*******************************************/

    . = $P0_STACK_START_ADDRESS;
    .stack (NOLOAD) : AT (__initc_p1_address)
    {
        $MEM_MAP_P0_STACK_START = ABSOLUTE(.);
        *(STACK_DM*)
        $MEM_MAP_P0_STACK_END = ABSOLUTE(.);
    }

    .preserve (NOLOAD) :
    {
        $MEM_P0_PRESERVE_START = ABSOLUTE(.);
        *(DM_PRESERVE_BLOCK*)
        $MEM_P0_PRESERVE_END = ABSOLUTE(.);
    }

    .mempool_dm1_p0 (NOLOAD) :
    {
        $MEM_MAP_DM1_P0_POOL_START = ABSOLUTE(.);
        *(*DM1_ZI_POOLS0*)
        *(*DM1_ZI\?*)
        $MEM_MAP_DM1_P0_POOL_END = ABSOLUTE(.);
    }

    .slow_heap_dm1 (NOLOAD) :
    {
        $MEM_MAP_DM1_HEAP_START = ABSOLUTE(.);
        *(*DM1_HEAP*)
    }

    . = ALIGN($ANCHOR_DM1_END - SIZEOF(.mempool_dm1_p0), 4);
    .mempool_dm1_p1 (NOLOAD) :
    {
        $MEM_MAP_DM1_P1_POOL_START = ABSOLUTE(.);
        *(*DM1_ZI_POOLS1*)
        *(*DM1_ZI*)
        $MEM_MAP_DM1_P1_POOL_END = ABSOLUTE(.);
    }

    $DM1_HEAP_SIZE = $MEM_MAP_DM1_P1_POOL_START -
                     $MEM_MAP_DM1_HEAP_START;
                          
    $SINGLE_MODE_DM1_HEAP_SIZE = $MEM_MAP_DM1_P1_POOL_END -
                                 $MEM_MAP_DM1_HEAP_START;

    . = ALIGN($ANCHOR_DM1_END, $PHYSICAL_DMRAM_INSTANCE_SIZE);

    /*******************************************
	  Shared DM2 Area:
    *******************************************/
    . = ALIGN($MEM_MAP_DM2_P1_BSS_END, 8);

    .shared_heap_dm2 (NOLOAD) : AT (__initc_p1_address)
    {
        $MEM_MAP_DM2_SHARED_HEAP_START = ABSOLUTE(.);
        *(*DM2_SHARED_HEAP*)
        $MEM_MAP_DM2_SHARED_HEAP_END = ABSOLUTE(.);
    }
    /* double the share heap for dual core */
    $DM2_MIN_SHARED_HEAP_SIZE  = 2 * SIZEOF(.shared_heap_dm2 );
    $DM_SHARED_END  = $MEM_MAP_DM2_SHARED_HEAP_END + $DM2_MIN_SHARED_HEAP_SIZE;

    oversize_assertion = ASSERT($MEM_MAP_DM2_SHARED_HEAP_END <= $ANCHOR_DM2_START, "Shared heap overflowed to P0 bank");
    $DM2_SHARED_HEAP_SIZE  = $ANCHOR_DM2_START - $MEM_MAP_DM2_SHARED_HEAP_START;

    /*******************************************
	  Dynamic DM2 Area:
	  - DM2 pools for P0
	  - DM2 heap for P0
	  - DM2 heap for P1
	  - DM2 pools for P1
	  - P0 stack
	  - P0 preserved area

	  The size of both pools MUST be identical.
	*******************************************/

    . = ALIGN($ANCHOR_DM2_START, 4);
    .mempool_dm2_p0 (NOLOAD) :
    {
        $MEM_MAP_DM2_P0_POOL_START = ABSOLUTE(.);
        *(*DM2_ZI_POOLS0*)
        *(*DM2_P0_ZI*)
        $MEM_MAP_DM2_P0_POOL_END = ABSOLUTE(.);
    }

    .fast_heap_dm2 (NOLOAD) :
    {
        $MEM_MAP_DM2_HEAP_START = ABSOLUTE(.);
        *(*DM2_HEAP*)
    }

    . = ALIGN($P1_STACK_START_ADDRESS - $STACK_GAP - SIZEOF(.mempool_dm2_p0), 4);
    .mempool_dm2_p1 (NOLOAD) :
    {
        $MEM_MAP_DM2_P1_POOL_START = ABSOLUTE(.);
        *(*DM2_ZI_POOLS1*)
        *(*DM2_P1_ZI*)
        $MEM_MAP_DM2_P1_POOL_END = ABSOLUTE(.);
    }

    $DM2_RAM_HEAP_SIZE = $MEM_MAP_DM2_P1_POOL_START -
                         $MEM_MAP_DM2_HEAP_START;

    . = $P1_STACK_START_ADDRESS;
    .p1_stack (NOLOAD) :
    {
        $MEM_MAP_P1_STACK_START = ABSOLUTE(.);
        *(STACK_P1_DM*)
        $MEM_MAP_P1_STACK_END = ABSOLUTE(.);
    }

    .p1_preserve (NOLOAD) :
    {
        $MEM_P1_PRESERVE_START = ABSOLUTE(.);
        *(DM_P1_PRESERVE_BLOCK*)
        $MEM_P1_PRESERVE_END = ABSOLUTE(.);
    }

    /* The maximum heap size in single core mode is bigger. */
    $SINGLE_MODE_DM2_RAM_HEAP_SIZE = ALIGN($MEM_P1_PRESERVE_END, $PHYSICAL_DMRAM_INSTANCE_SIZE) -
                                     $MEM_MAP_DM2_HEAP_START;

    /* Debug strings should be present in the elf file but not loaded onto the
     * chip. Therefore where we put them in the image doesn't matter,
     * so we choose an address out of the way of real code so that the linker
     * doesn't complain about overlapped sections and the value is not as
     * likely to crop up by accident. NOTE: This is different for downloadable
     * capabilities to make easier to distinguish between downloadable and
     * normal logs.
     */
    . = 0x13500000;
    debug_strings (INFO) :
    {
        KEEP(*(GLOBAL_DEBUG_STRINGS*))
        *(DEBUG_TRACE_STRINGS*)
    }

    /* Create some useful symbols for the crt code to use */
    /* Size of each regions */
    $DM2_INIT_SIZE_DWORDS = (SIZEOF (.initc_dm2) + 3) / 4 + (SIZEOF (.initc_dm2_private) + 3) / 4;
    $DM1_INIT_SIZE_DWORDS = (SIZEOF (.initc_dm1) + 3) / 4 + (SIZEOF (.initc_dm1_private) + 3) / 4;
    $DM1_INIT_P0_SIZE_DWORDS = (SIZEOF (.initc_p0) + 3) + (SIZEOF(.initc_p0_private) + 3) / 4;
    $DM1_INIT_P1_SIZE_DWORDS = (SIZEOF (.initc_p1) + 3) / 4;
    $DM_INIT_MEM_GUARD_SIZE_DWORDS = (SIZEOF (.initc_mem_guard) + 3) / 4;

    /* ROM address on each init regions */

    $MEM_MAP_DM1_INITC_ROM_ADDR = $DATA_START_ADDRESS + ABSOLUTE(__DM1_rom_address);
    $MEM_MAP_DM2_INITC_ROM_ADDR = $DATA_START_ADDRESS + ABSOLUTE(__DM2_rom_address);
    $MEM_MAP_DM1_INITC_P0_ROM_ADDR = $DATA_START_ADDRESS + ABSOLUTE(__DM1_P0_rom_address);
    $MEM_MAP_DM2_INITC_P1_ROM_ADDR = $DATA_START_ADDRESS + ABSOLUTE(__DM2_P1_rom_address);
    $MEM_MAP_DM_INITC_MEM_GUARD_ROM_ADDR = $DATA_START_ADDRESS + ABSOLUTE(__DM1_guard_rom_address);

    /* BSS Symbols */
    $MEM_MAP_DM1_BSS_FREE_START = $MEM_MAP_DM1_P0_POOL_START;
    $MEM_MAP_DM1_BSS_FREE_LENGTH_DWORDS = (($MEM_MAP_DM1_P1_POOL_END - $MEM_MAP_DM1_P0_POOL_START) + 3) / 4;
    $MEM_MAP_DM1_BSS_LENGTH_DWORDS = (ALIGN(SIZEOF(.zeroinit_dm1),4) + ALIGN(SIZEOF(.zeroinit_dm1_private),4)) / 4;
    $MEM_MAP_DM2_BSS_LENGTH_DWORDS = (ALIGN(SIZEOF(.zeroinit_dm2),4) + ALIGN(SIZEOF(.zeroinit_dm2_private),4)) / 4;
    $MEM_MAP_DM1_P0_BSS_LENGTH_DWORDS = (ALIGN(SIZEOF(.zeroinit_p0),4) + ALIGN(SIZEOF(.zeroinit_p0_private),4)) / 4;
    $MEM_MAP_DM2_P1_BSS_LENGTH_DWORDS = (ALIGN(SIZEOF(.zeroinit_p1),4) + ALIGN(SIZEOF(.zeroinit_p1_private),4)) / 4;

    $MEM_MAP_DM2_BSS_FREE_START = $MEM_MAP_DM2_P0_POOL_START;
    $MEM_MAP_DM2_BSS_FREE_LENGTH_DWORDS = (($MEM_MAP_DM2_P1_POOL_END - $MEM_MAP_DM2_P0_POOL_START) + 3) / 4;

    $MEM_MAP_DM2_SHEAP_LENGTH_DWORDS = $DM2_SHARED_HEAP_SIZE / 4;
    $MEM_MAP_DM2_FHEAP_LENGTH_DWORDS = $DM2_RAM_HEAP_SIZE / 4;

    /* Cache Miss logs location */
    $MEM_MAP_P0_CACHE_MISS_LOG_START_ADDRESS = $MEM_MAP_DM2_P0_POOL_END;
    $MEM_MAP_P0_CMISS_LENGTH_DWORDS = (($MEM_MAP_DM2_P0_POOL_START - $MEM_MAP_DM2_P1_POOL_END) + 3) / 4;
    $MEM_MAP_P1_CACHE_MISS_LOG_START_ADDRESS = $MEM_MAP_DM2_P1_POOL_END;

    /* PM RAM symbols */
    $PM_INIT_SIZE_DWORDS = (__CODE_RAM_SIZE + 3) / 4;
    $MEM_MAP_PM_INIT_ROM_ADDR = $PM_DATA_START_BASE + ABSOLUTE(__CODE_RAM_address);
    $MEM_MAP_RAM_CODE_START = ($PM_RAM_P0_CODE_START - $PM_RAM_START_ADDRESS) + $DM_WINDOW_TO_PM_BASE_ADDRESS;

    $MEM_MAP_P1_PM_INIT_SIZE_DWORDS = (__CODE_P1_RAM_SIZE + 3) / 4;
    $MEM_MAP_P1_PM_INIT_ROM_ADDR = $PM_DATA_START_BASE + ABSOLUTE(__CODE_P1_RAM_address);
    $MEM_MAP_P1_RAM_CODE_START = ($PM_RAM_P1_CODE_START - $PM_RAM_START_ADDRESS) + $DM_WINDOW_TO_PM_BASE_ADDRESS;

    $MEM_MAP_CACHE_INIT_SIZE_DWORDS = (SIZEOF(.text_cacheinit) + 3) / 4;
    $MEM_MAP_CACHE_INIT_ROM_ADDR = $PM_DATA_START_BASE + ABSOLUTE(__CACHE_INIT_address);
    $MEM_MAP_CACHE_CODE_START = ($PM_RAM_CACHE_INIT_START - $PM_RAM_START_ADDRESS) + $DM_WINDOW_TO_PM_BASE_ADDRESS;

    /* Heap sizes used in the code */
    $__heap1_size = $DM1_HEAP_SIZE;
    $__heap2_size = $DM2_RAM_HEAP_SIZE;
    $__heap3_size = $DM2_SHARED_HEAP_SIZE;
    
    /* Maximum heap sizes if  in the code */
    $__single_mode_heap1_size = $SINGLE_MODE_DM1_HEAP_SIZE;
    $__single_mode_heap2_size = $SINGLE_MODE_DM2_RAM_HEAP_SIZE;
    $__single_mode_heap3_size = $DM2_SHARED_HEAP_SIZE;

    $__pm_heap_size = $PM_HEAP_SIZE;
    $__pm_p1_heap_size = $PM_P1_HEAP_SIZE;

    /* Expose P1 start addr to the C code */
    $__mem_map_p1_code_start_address = $MEM_MAP_P1_CODE_START_ADDRESS;

    /* Export the checksum symbol so it can be referenced in C and included in the SLT */
    $__devtools_image_checksum = __devtools_image_checksum;

    /* Export limits of area where pointers to memory allocated on the pools or heaps can be stored. */
    $ACAT_DM1_START = $DM1_RAM_BASE_ADDRESS;
    $ACAT_DM1_LIMIT = $MEM_MAP_DM1_P1_POOL_END;
    $ACAT_DM2_START = $DM2_RAM_BASE_ADDRESS;
    $ACAT_DM2_LIMIT = $MEM_MAP_P1_STACK_END;
	
    /* Used by AudioPatchMaker. */
    $DATA_ROM_WINDOW = $DM_NVRAM0_START_ADDRESS;

    /* Used by ACAT. */
    $PM_RAM_DM_WINDOW_BASE_ADDRESS = $DM_WINDOW_TO_PM_BASE_ADDRESS;
}
