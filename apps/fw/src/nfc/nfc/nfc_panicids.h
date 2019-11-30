/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/* This header fragment is to be included by panicids.h
 * It cannot stand alone */

    /** 
     * For specialised buffer where the total length of zero 
     * for a message is invalid (e.g. NFC buffer)
     */
    PANIC_BUFFER_INVALID_MSG_LENGTH = 0x3000,

   /**
    * Used for debugging NBUFFER
    */
    PANIC_BUFFER_WRONG_METADATA = 0x3001,

    /**
     * Used for debugging NBUFFER
     */
    PANIC_BUFFER_NOT_INCOMPLETE = 0x3002,

