// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// %%version
// *****************************************************************************




// *****************************************************************************
// NAME:
//    Memory operations
//
// DESCRIPTION:
//    This provides a set of functions that abstract certain memory operations on
//    the Bluecore platform.      
//       $mem.ext_window_unpack_to_ram
//       $mem.ext_window_copy_to_ram
//
// *****************************************************************************


// *****************************************************************************
// NAME:
//    Flash access Library
//
// DESCRIPTION:
//    This library provides an API to aid accessing data from the onchip flash
//    memory from kalimba.  Access to flash using BC3-MM is quite slow, it takes
//    approx 2us (64 DSP clock cycles) per word.  Using BC5-MM it is faster, but
//    the access time still depends on the speed of the flash part used and the
//    MCU's clock rate and workload.  Typically with a 70ns flash the DSP will
//    take 9 cycles (at 64MHz) to access a 16bit word from flash.
//
//    For BC5-MM this library also provides an API for initialising the hardware
//    to allow execution from flash program memory (PM).  Accessing flash
//    program memory is slower than RAM, again depending on the flash part
//    used and the MCU's clock rate and workload.  Typically with a 70ns flash
//    the DSP will take 17 cycles (at 64MHz) to access a 32bit instruction word
//    from flash.  There is a 64 word direct-mapped cache that is used when
//    accessing PM flash, and so if there is a cache hit the access time is just
//    a single clock cycle (same as accessing PM RAM).
//
//    For example use see the app in: apps/examples/kalimba_flash_access_example/
//
//   TODO_FILE_MSG_VM do we still support this?
//    This library also presents an API for DSP applications to request the
//    flash address of a file within the VM's read only file system.  DSP
//    applications request a file's flash address by calling
//    '$flash.get_file_address' with the VM file handle.  When the address has
//    been retrieved the associated handler is called.  The library uses
//    structures to hold the information it requires for each request. Each
//    structure should be of size $flash.get_file_address.STRUC_SIZE and
//    contains the following fields:
//      @verbatim
//        Name                                             Index
//        $flash.get_file_address.NEXT_ENTRY_FIELD           0
//        $flash.get_file_address.FILE_ID_FIELD              1
//        $flash.get_file_address.HANDLER_ADDR_FIELD         2
//      @endverbatim
//
// *****************************************************************************

#include "stack.h"
#ifndef TODO_FILE_MSG_VM
#include "message.h"
#include "kalimba_messages.h"
#include "flash.h"
#include "timer.h"
#endif   // TODO_FILE_MSG_VM



// *****************************************************************************
// MODULE:
//    $mem.ext_window_unpack_to_ram
//
//    unsigned mem_ext_window_unpack_to_ram(char *src,unsigned size,unsigned *dest);
//
// DESCRIPTION:
//    Will copy data "FORMAT_PACKED16" to RAM.
//
// INPUTS:
//    - r0 = address of data block to be copied
//    - r1 = size of data block in destination RAM
//    - r2 = destination address
//
// OUTPUTS:
//    - r0 = result:  (r0==0) FAILED, else PASSED 
//
// TRASHED REGISTERS:
//    r0, r1, r2
//
// *****************************************************************************

.MODULE $M.mem.ext_window_unpack_to_ram;
   .CODESEGMENT EXT_DEFINED_PM;

$mem.ext_window_unpack_to_ram:
$_mem_ext_window_unpack_to_ram:

   pushm <r3, r4, r5, r6, r10, rLink>;
   pushm <I0, I1>;

   I0 = r2;
   r6 = r0;       // flash address
   r5 = r1 AND 1; // is odd
   r4 = r1 ASHIFT - 1;// r4 = (even part) / 2
   page_loop:
      // map appropriate flash page in
      r0 = r6;
      r1 = r4 * 3 (int);
      push r1;

      call $mem.ext_window_access_as_ram; 

      // make sure even number of data is read each time
      I1 = r0;
      r10 = r4;
      pop r0;
      Null = r0 - r1;
      if EQ jump read_values;
         r1 = r1 * 2 (int);
         r1 = r1 * 0.33333337306976 (frac);
         r10 = r1 ASHIFT -1;
      read_values:

      // calculate remaining pairs of data to read
      r4 = r4 - r10;

      // update flash address
      r0 = r10 * 3 (int);
      r6 = r6 + r0;

      // copy data from flash to dm for this flash page
      do loop;
         // read MSB 0 (0-15)
         r0 = M[I1,1];
         // read LSB 0 (8-15), MSB 1 (0-7)
         r1 = M[I1,1];

         // -- reassemble 1st 24bit word --
         r0 = r0 LSHIFT 8;
         r3 = r1 LSHIFT -8;

         // mask off possible sign extension of flash reads
         r3 = r3 AND 0xFF;
         r3 = r3 OR r0,
          r0 = M[I1,1]; // read LSB 1 (0-15)

         // -- reassemble 2nd 24bit word --
         // mask off possible sign extension of flash reads
         r0 = r0 AND 0xFFFF;
         r1 = r1 LSHIFT 16;
         r0 = r0 + r1,
          M[I0,1] = r3;       // store 1st 24bit word

         // store 2nd 24bit word
         M[I0,1] = r0;
      loop:
      // if we need more data from another flash page loop around again
      Null = r4;
   if NZ jump page_loop;

   // if number of samples to be read is odd, read the last sample separately
   Null = r5;
   if Z jump jp_unpack_done;
      r0 = r6;
      r1 = 2;
      call $mem.ext_window_access_as_ram;
      I1 = r0;
      r0 = M[I1,1];
      r1 = M[I1,1];
      r0 = r0 LSHIFT 8;
      r1 = r1 LSHIFT -8;
      r1 = r1 AND 0xFF;
      r0 = r1 OR r0;
      M[I0, 1] = r0;
   jp_unpack_done:

   popm <I0, I1>;
   popm <r3, r4, r5, r6, r10, rLink>;
   r0 = 1;
   rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $mem.ext_window_copy_to_ram
//
//    unsigned mem_ext_window_copy_to_ram(char *src,unsigned size,unsigned *dest);
//
// DESCRIPTION:
//    Will copy data "FORMAT_16BIT_SIGN_EXT" to RAM.
//
//
// INPUTS:
//    - r0 = address of data block to be copied
//    - r1 = size of data block in destination RAM
//    - r2 = destination address
//
// OUTPUTS:
//    - r0 = result:  (r0==0) FAILED, else PASSED 
//
// TRASHED REGISTERS:
//    r0, r1, r2
//
// *****************************************************************************
.MODULE $M.mem.ext_window_copy_to_ram;
   .CODESEGMENT EXT_DEFINED_PM;

$mem.ext_window_copy_to_ram:
$_mem_ext_window_copy_to_ram:

   pushm <I0, I1>;
   pushm <r3, r4, r5, r10, rLink>;

   I0 = r2;
   r4 = r0;
   r5 = r1;
   page_loop:
      // map appropriate flash page in
      r0 = r4;
      r1 = r5;

      call $mem.ext_window_access_as_ram;
      // copy data from flash to dm for this flash page
      r10 = r1;
      I1 = r0;
      do loop;
         r0 = M[I1,1];
         M[I0,1] = r0;
      loop:
      // if we need more data from another flash page loop around again
      r4 = r4 + r1;
      r5 = r5 - r1;
   if NZ jump page_loop;

   popm <r3, r4, r5, r10, rLink>;
   popm <I0, I1>;
   r0 = 1;
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $mem.ext_window_access_as_ram
//
// DESCRIPTION:
//    Given the address of a 'flash segment' variable this routine will map in
// the appropriate flash page required and return a pointer to the DM2 flash
// window to access it.
//
// The function also assumes that the size of the copy you wish to make from
// the flash page is passed in r1. If r1 is greater than the flash page size
// it will be set to the flash page size. If you don't require this
// behaviour, any value can be passed in r1 and the result can be ignored.
//
//
// INPUTS:
//    - r0 = address of variable in 'flash segment'
//    - r1 = optional size of variable in 'flash segment'
//
// OUTPUTS:
//    - r0 = an address in the DM2 flash window to read the variable from
//    - r1 = an adjusted value of the size so that reads are kept within
//           a single flash page
//
// TRASHED REGISTERS:
//    r2,r3
//
//
// *****************************************************************************
.MODULE $M.mem.ext_window_access_as_ram;
   .CODESEGMENT FLASH_MAP_PAGE_INTO_DM_PM;
   .DATASEGMENT DM;

   // base address in flash of the data - filled out by the VM's KalimbaLoad()
   .VAR/DM_STATIC $flash.windowed_data16.address;

$mem.ext_window_access_as_ram:

   r2 = M[$flash.windowed_data16.address];


   // In KAL_ARCH3, $FLASH_WINDOW3_START_ADDR is in units of 32 bits. However offset in
   // r0 is in 16 bits, so we need to transform it
   r3 = r0 ASHIFT -1;
   M[$FLASH_WINDOW3_START_ADDR] = r2 + r3;

   // if the address is odd we need to lose a word from the start of the window
   r3 = r0 AND 1;

   // limit r1 (requested amount to read) so that reads are kept within flash page
   r0 = $FLASHWIN3_SIZE - r3;
   r0 = r1 - r0;
   if POS r1 = r1 - r0;

   // address to read from is just the flashwin3 start address
   r0 = r3 + &$FLASHWIN3_START;

   rts;


.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $flash.copy_to_dm_32_to_24
//
// DESCRIPTION:
//    Given the address of a 'flash segment' variable and its size this routine
// will copy the variable to a place in data memory ram.  It will automatically
// switch flash pages as needed if the variable overlaps a flash page boundary.
// each 24-bit data, 0xABCDEF, is stored in 2 words in flash:
//
//    word 1: 0xABCD
//    word 2: 0x--EF
//
// where the 8 MSBits of the second word are ignored.
//
//
// INPUTS:
//    - r0 = address of variable in 'flash segment'
//    - r1 = size of variable in 'flash segment'
//    - r2 = address to copy flash data to in DM
//
// OUTPUTS:
//    - r0 = last word read from flash (useful if you're just reading 1 word)
//    - I0 = end address of copied data + 1
//
// TRASHED REGISTERS:
//    r0, r1, r3, r4, r5, I1, I0, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.flash.copy_to_dm_32_to_24;
   .CODESEGMENT FLASH_COPY_TO_DM_32_TO_24_PM;

   $flash.copy_to_dm_32_to_24:

   I0 = r2;

   // push rLink onto stack
   push rLink;

   r4 = r0;
   r5 = r1 * 2 (int);
   page_loop:
      // map appropriate flash page in
      r0 = r4;
      r1 = r5;
      call $mem.ext_window_access_as_ram;
      // copy data from flash to dm for this flash page
      r10 = r1 ASHIFT -1;
      r1 = r10 * 2 (int);
      I1 = r0;
      do loop;
         r0 = M[I1,1];
         r0 = r0 LSHIFT 8;
         r2 = M[I1,1];
         r2 = r2 AND 0xFF;
         r0 = r0 OR r2;
         M[I0,1] = r0;
      loop:
      // if we need more data from another flash page loop around again
      r4 = r4 + r1;
      r5 = r5 - r1;
   if NZ jump page_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;




#ifndef TODO_FILE_MSG_VM

// *****************************************************************************
// MODULE:
//    $flash.get_file_address
//
// DESCRIPTION:
//    Registers a request to the firmware to obtain the flash address of a file
// from the filesystem given it's VM file handle.
//
//  INPUTS
//    - r1 = pointer to a structure that stores the 'get_file_address' handler
//           structure, should be of length $flash.get_file_address.STRUC_SIZE
//    - r2 = VM file handle number for which to retrieve the flash address
//    - r3 = address of routine to call with the flash address
//
//  OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-6, r10, DoLoop
//
// NOTES:
//    This is a request to the Firmware to retrieve the flash address of a file
// in the VM's read-only file system. This is not an instantaneous process. When
// you request a file's flash address the value is retrieved as a message and is
// sent to Kalimba.   This is then forwarded on to your registered handler.
//
//    This library allows queuing of requests, the structure you supply is used
// in a linked list, consequently you cannot use the same structure until it has
// been freed from the list by the library. When the handler is called the
// structure will have already been freed so you may use it to request another
// read at that point.
//
// *****************************************************************************
.MODULE $M.flash.get_file_address;
   .CODESEGMENT FLASH_GET_FILE_ADDRESS_PM;
   .DATASEGMENT DM;

   .VAR $flash.get_file_address.last_addr = $flash.get_file_address.LAST_ENTRY;
   .VAR $flash.get_file_address.message_struc[$message.STRUC_SIZE] =
                          $flash.get_file_address.MESSAGE_HANDLER_UNINITIALISED, 0 ...;
   .VAR $flash.get_file_address.reattempt_timer_struc[$timer.STRUC_SIZE];
   .VAR $flash.request_in_progress;

   #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
      .VAR $flash.get_file_address.debug_count;
   #endif

   $flash.get_file_address:

   // push rLink onto stack
   push rLink;

   // block interrupts
   call $block_interrupts;

   // r1 is lost so store it in r5
   r5 = r1;

   // save the VM file handle to retrieve
   M[r5 + $flash.get_file_address.FILE_ID_FIELD] = r2;
   M[r5 + $flash.get_file_address.HANDLER_ADDR_FIELD] = r3;
   // mark this as the end of the list
   r3 = $flash.get_file_address.LAST_ENTRY;
   M[r5 + $flash.get_file_address.NEXT_ENTRY_FIELD] = r3;

   // if it's the first call, then initialise the message handler
   r0 = M[$flash.get_file_address.message_struc];
   Null = r0 - $flash.get_file_address.MESSAGE_HANDLER_UNINITIALISED;
   if NZ jump message_handler_initiailised;
      // set up message handler for $MESSAGE_FILE_ADDRES message
      r1 = &$flash.get_file_address.message_struc;
      r2 = Null OR $MESSAGE_FILE_ADDRESS;
      r3 = &$flash.get_file_address.message_handler;
      call $message.register_handler;
   message_handler_initiailised:

   // load up the start of the list
   r0 = &$flash.get_file_address.last_addr;

   #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
      r3 = $flash.get_file_address.MAX_HANDLERS;
      M[$flash.get_file_address.debug_count] = r3;
   #endif

   find_end_of_list:
      #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
         // have we been round too many times
         r3 = M[$flash.get_file_address.debug_count];
         r3 = r3 - 1;
         if NEG call $error;
         M[$flash.get_file_address.debug_count] = r3;
      #endif

      r2 = r0;

      // get the next address in the list
      r0 = M[r2 + $flash.get_file_address.NEXT_ENTRY_FIELD];

      // is it the last one?
      Null = r0 - $flash.get_file_address.LAST_ENTRY;
   if NZ jump find_end_of_list;

   // r2 should now point to the last element in the list
   M[r2 + $flash.get_file_address.NEXT_ENTRY_FIELD] = r5;

   // if we're not already fetching a file's address then send a new request
   Null = r2 - &$flash.get_file_address.last_addr;
   if Z call $flash.private.get_file_address.send_request;

   // unblock interrupts
   call $unblock_interrupts;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $flash.get_file_address.message_handler
//
// DESCRIPTION:
//    Message handler to receive messages from Firmware and forward onto
//  the registered 'get_file_address' handler.
//
//  INPUTS:
//      - r0 = message ID     ($MESSAGE_FILE_ADDRESS)
//      - r1 = message Data 0 (VM file handle)
//      - r2 = message Data 1 (lower 16bits of address)
//      - r3 = message Data 2 (upper 16bits of address)
//
//  OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    Assume everything
//
// NOTES:
//    The registered handler is called with the registers populated as follows:
//@verbatim
//    Register         Description
//    --------         ---------------
//       r1            VM file handle
//       r2            24-bit absolute flash address of start of file
//@endverbatim
// *****************************************************************************
.MODULE $M.flash.get_file_address.message_handler;
   .CODESEGMENT FLASH_GET_FILE_ADDRESS_MESSAGE_HANDLER_PM;

   $flash.get_file_address.message_handler:

   // clear the read in progress flag
   M[$flash.request_in_progress] = Null;

   // convert from MS and LS words to a 24bit word
   r3 = r3 LSHIFT 16;
   r2 = r2 AND 0xFFFF;
   r2 = r2 + r3;

   // ** work out who to forward the message to **
   r5 = &$flash.get_file_address.last_addr;
   r0 = M[$flash.get_file_address.last_addr + $flash.get_file_address.NEXT_ENTRY_FIELD];

   #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
      Null = r0 - $flash.get_file_address.LAST_ENTRY;
      if Z call $error;

      r6 = $flash.get_file_address.MAX_HANDLERS;
      M[$flash.get_file_address.debug_count] = r6;
   #endif

   // this should always be the first one but might not be
   find_structure_loop:
      #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
         // have we been round too many times
         r6 = M[$flash.get_file_address.debug_count];
         r6 = r6 - 1;
         if NEG call $error;
         M[$flash.get_file_address.debug_count] = r6;
      #endif

      r4 = M[r0 + $flash.get_file_address.FILE_ID_FIELD];
      Null = r1 - r4;
      if Z jump structure_found;

      // store the previous structure address
      r5 = r0;

      r0 = M[r0 + $flash.get_file_address.NEXT_ENTRY_FIELD];
      Null = r0 - $flash.get_file_address.LAST_ENTRY;
   if NZ jump find_structure_loop;

   // ** we don't appear to have asked for this file ID **
   #ifdef FLASH_GET_FILE_ADDRESS_DEBUG_ON
      call $error;
   #else
      rts;
   #endif

   structure_found:
   // push rLink onto stack
   push rLink;

   // remove this handler from the list
   r4 = M[r0 + $flash.get_file_address.NEXT_ENTRY_FIELD];
   M[r5 + $flash.get_file_address.NEXT_ENTRY_FIELD] = r4;

   // call the relevant handler
   r0 = M[r0 + $flash.get_file_address.HANDLER_ADDR_FIELD];
   call r0;

   // check if there are any more file flash addresses to retrieve
   r1 = M[$flash.get_file_address.last_addr];
   Null = r1 - $flash.get_file_address.LAST_ENTRY;
   if NZ call $flash.private.get_file_address.send_request;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;





// $****************************************************************************
// MODULE:
//    $flash.private.get_file_address.send_request
//
// DESCRIPTION:
//    This routine handles the sending of the message request to the firmware,
// if the message queue is full it will set a re-attempt timer and continue to
// try and send the message until it succeeds.
//
//    This is a private function and should never be
// called by an application - use $flash.get_file_address instead.
//
//  INPUTS:
//    - none
//
//  OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-6, r10, DoLoop
//
// NOTES:
//    This is a private function, use $flash.get_file_address instead.
//
// *****************************************************************************
.MODULE $M.flash.private.get_file_address.send_request;
   .CODESEGMENT FLASH_PRIVATE_GET_FILE_ADDRESS_SEND_REQUEST_PM;

   $flash.private.get_file_address.send_request:

   // are we doing an access
   Null = M[$flash.request_in_progress];
   if NZ rts;

   // push rLink onto stack
   push rLink;

   // remove this timer from the list
   r2 = M[$flash.get_file_address.reattempt_timer_struc + $timer.ID_FIELD];
   call $timer.cancel_event;

   // check there is space in the message queue
   call $message.send_queue_space;
   Null = r0 - 5;
   if NEG jump try_to_send_later;
      // send a message to the firmware requesting the file's flash address
      r1 = M[$flash.get_file_address.last_addr];
      r2 = Null OR $MESSAGE_FILE_ADDRESS;
      r3 = M[r1 + $flash.get_file_address.FILE_ID_FIELD];
      call $message.send_short;

      // set read in progress
      r0  = 1;
      M[$flash.request_in_progress] = r0;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

   try_to_send_later:
      // set a timer to call us again
      r1 = &$flash.get_file_address.reattempt_timer_struc;
      r2 = $flash.get_file_address.REATTEMPT_TIME_PERIOD;
      r3 = &$flash.private.get_file_address.send_request;
      call $timer.schedule_event_in;
      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;

#endif   // TODO_FILE_MSG_VM

