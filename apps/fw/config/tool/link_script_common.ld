/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*   %%version */
/* Common linker script items */

/* This magic layout offset for $CODE_MAGIC$ tells the linker that this is
 * in the program address space rather than the data address space */
$CODE_MAGIC$            = 0x80000000;


CHECKSUM_DATA_SECTION(checksum);

SECTIONS
{
    /* Code starts at ROM address zero (aka LMA - Load Memory Address) */
    __CODE_rom_address = 0;

    . = ALIGN($CODE_MAGIC$, 0x4);
    .text_reset : AT( __CODE_rom_address )
    {
        $MEM_MAP_CODE_START = 0 ;
        *(PM_RST*)

        /* The checksum should not be in data RAM as it may not have been loaded
         * by the time applications wish to read it. It should also be near the
         * start of memory so that the checksum is always overwritten when a new
         * image is flashed.
         */
        *(checksum*)

    }
    /* The slt comes after, at address 0x80
     * It needs to live at a data address from a processor point of view
     * And also from the point of view of code reading it over the TBus
     */
    __CODE_slt_address = 0x80;
    . = $DATA_ROM_WINDOW + __CODE_slt_address;
    .slt : AT ( __CODE_slt_address )
    {
        *(CONST_SLT*)
    }
    SLT_SIZE = SIZEOF(.slt);
    
    /* The const_tcm follows slt, it needs to live
     * close to the start of the image to end up in the TCM
     */        
    .const_tcm : AT ( __CODE_slt_address + SLT_SIZE )
    {
        /*
         * This is a window in the data space that maps onto the ROM
         * so we advance into it the same amount as the offset in the
         * program memory.
         */
        $MEM_MAP_CONST_TCM_START = ABSOLUTE(.) ;
        *(dmtcm\?*)
        $MEM_MAP_CONST_TCM_END = ABSOLUTE(.) ;
    }
    CONST_TCM_SIZE = SIZEOF(.const_tcm);
    
    /*
     * Now we have to go back to CODE_MAGIC,
     * with an offset to account for the space taken by this bit of slt
     * and where it started (ie 8 + 0x80)
     */
    __CODE_text_address = ALIGN(__CODE_slt_address + SLT_SIZE + CONST_TCM_SIZE, 4);
    . = ALIGN($CODE_MAGIC$ + __CODE_text_address, 0x4);
        
    .text BLOCK(2) : AT( __CODE_text_address )
    {
        *(INTERRUPT_*_PM*)
        $MEM_MAP_TCM_CODE_START = ABSOLUTE(.) ;
        *(tcm*)
        $MEM_MAP_TCM_CODE_END = ABSOLUTE(.) ;
        *(PM*)
        *(pm*)
        $MEM_MAP_CODE_END = . ;
    }

    /* Next ROM address (LMA) immediately follows on from last LMA of .text */
    __CODE_ROM_SIZE = __CODE_text_address + SIZEOF(.text);
    __CONST_rom_address = ALIGN(__CODE_rom_address + __CODE_ROM_SIZE , 4);
    . = $DATA_ROM_WINDOW + __CONST_rom_address;

    .const : AT ( __CONST_rom_address )
    {
        /*
         * This is a window in the data space that maps onto the ROM
         * so we advance into it the same amount as the offset in the
         * program memory.
         */
        $MEM_MAP_CONST_START = ABSOLUTE(.) ;
        $_pmalloc_app_begin = ABSOLUTE (.) ;
        KEEP(*(apppool\?*))
        $_pmalloc_app_size = ABSOLUTE (.) - $_pmalloc_app_begin ;
        $_pmalloc_app_end = ABSOLUTE (.) ;
        *(dmconst\?*)
        $MEM_MAP_CONST_END = ABSOLUTE(.) ;
    }

    /* Next ROM address (LMA) immediately follows on from last LMA of .const */
    __DATA_rom_address = ALIGN(__CONST_rom_address + SIZEOF(.const), 4);

    /* But the VMA is different: the init data is copied to RAM so the VMA
     * doesn't depend on the LMA */
    . = $DATA_RAM_START_ADDRESS;

    .zeroinit (NOLOAD) : AT (0)
    {
        $MEM_MAP_BSS_START = ABSOLUTE(0) ;
        /* Skip the first dword so we can detect writes to a NULL pointer */
        LONG(0)
        /* Put the firmware log buffer at the start of memory as it is
         * not catastrophic if it gets corrupted.
         */
        *hydra_log_firm.o (dmzi*)
        *(dmzi*)
        $MEM_MAP_BSS_END = ABSOLUTE(.) ;
    }

    .initc : AT ( __DATA_rom_address )
    {
        $MEM_MAP_INITC_START = ABSOLUTE(0) ;
        *(DM\?*)
        *(dm\?*)
        $MEM_MAP_INITC_END =  ABSOLUTE(.) ;
    }

    /* Put the stack at the top of the data ram */
    . = $DATA_RAM_END_ADDRESS - $STACK_SIZE_BYTES ;
    .stack (NOLOAD) :
    {
        $MEM_MAP_STACK_START = ABSOLUTE(.) ;
        . = . + $STACK_SIZE_BYTES;
        $MEM_MAP_STACK_END = ABSOLUTE(.) ;
    }

    /* Fill the space between variables and stack with the heap */
    .pmalloc :
    {
        $MEM_MAP_PMALLOC_START = $MEM_MAP_INITC_END;
        $MEM_MAP_PMALLOC_END = $MEM_MAP_STACK_START;
        /* Used by init_pmalloc C code */
        $_pmalloc_begin = $MEM_MAP_PMALLOC_START;
        $_pmalloc_end = $MEM_MAP_PMALLOC_END;
    }


    . = $SHARED_RAM_BEGIN;
    .mmu (NOLOAD) :
    {
        $MEM_MAP_MMU_START = ABSOLUTE(.) ;
        $MEM_MAP_MMU_HANDLES_START = ABSOLUTE(.) ;
        *(mmu_handle_section*)
        $MEM_MAP_MMU_HANDLES_END = ABSOLUTE(.) ;
        $MEM_MAP_SHARED_MEMORY_START = ABSOLUTE(.) ;
        *(shared_memory_section*)
        $MEM_MAP_SHARED_MEMORY_END = ABSOLUTE(.) ;
        . = ALIGN(128);
        $MEM_MAP_MMU_PAGES_START = ABSOLUTE(.) ;
        *(mmu_memory_pages_section*)
        $MEM_MAP_MMU_PAGES_END = ABSOLUTE(.) ;
        $MEM_MAP_MMU_END = ABSOLUTE(.);
    }

    /* Debug strings should be present in the elf file but not loaded onto the
     * chip. Therefore where we put them in the image doesn't matter,
     * so we choose an address out of the way of real code so that the linker
     * doesn't complain about overlapped sections and the value is not as
     * likely to crop up by accident.
     */
    . = 0x13500000;
    debug_strings (INFO) :
    {
        *(DBG_STRING*)
    }
    
    /* /DISCARD/ is a special section whose contents the linker throws away */
    /DISCARD/ :
    {
        *(UNREFERENCED*)
    }

    /* Create some useful symbols for the crt code to use */
    $MEM_MAP_BSS_LENGTH_DWORDS = (SIZEOF (.zeroinit) + 3) / 4 ;
    $MEM_MAP_INITC_ROM_ADDR = $DATA_ROM_WINDOW + ABSOLUTE(__DATA_rom_address) ;
    $MEM_MAP_INITC_LENGTH_DWORDS = (SIZEOF (.initc) + 3) / 4 ;
}
