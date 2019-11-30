/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Routines dealing with the APPCMD
 *
 * \section appcmd DESCRIPTION
 *
 * This file contains all functions dealing with the APPCMD Implementation
*/
#define IO_DEFS_MODULE_K32_TIMERS /* for TIMER_TIME */
#include "appcmd/appcmd_private.h"

#include "assert.h"

#define APPCMD_MASK 0x003f

#define APPCMD_RSP_BUF_REMOTE_FLAG (1U << 31)

#define APPCMD_RESPOND(x)                                                   \
    do {                                                                    \
        appcmd_rsp =                                                        \
            (((uint16)((x)&0xffff)) | ((appcmd_cmd & 0x0040U) ? 0x8000U : 0x0000U)); \
    } while (0)

/** Interface variables */
volatile uint32 appcmd_cmd;
uint32 appcmd_parameters[APPCMD_MISC_VALUES_CMD_BUF_LEN];
uint32 appcmd_results[APPCMD_MISC_VALUES_CMD_BUF_LEN];
uint32 appcmd_rsp;
const uint32 appcmd_protocol_version = 0;

/** Array of test handlers */
static struct {
    appcmd_test_handler_fn *fn;
    APPCMD_TEST_ID           command;
} test_handlers[24];

/** Number of registered test handlers */
static uint32 handler_count = 0;

#ifdef ENABLE_SHALLOW_SLEEP_TIMING
/**
 * Variable declared in core/int/interrupt.asm that counts the microseconds
 * spent in shallow sleep.
 */
extern uint32 total_shallow_sleep_time;
#endif /* ENABLE_SHALLOW_SLEEP_TIMING */

/**
 * Call a handler for a test command if it exists. The test command
 * is assumed to be the first parameter value.
 * /returns TRUE if the handler exists.
 */
static bool call_test_handler_fn(void);

/*@{*/
/**
 * Return the \c mmu_handle of the indicate \c BUFFER_MSG structure in the first
 * parameter
 */
static void appcmd_buf_return_handle(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];
    /* Return just the handle, not the SSID */
    appcmd_results[0] = (uint32)(buf_msg->buf.handle & 0xff);
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Return the number of free bytes in the buffer, or 0 if there are no free
 * messages.
 */
static void appcmd_buf_return_free_space(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];
    /* Return the number of unused bytes in results 0 and the number of unused
     * messages in results 1 */
    L5_DBG_MSG3("Buffer pointers are: i: %d, o: %d, t: %d",
            buf_msg->buf.index, buf_msg->buf.outdex, buf_msg->buf.tail);
    appcmd_results[0] = (uint32)BUF_GET_FREESPACE(&buf_msg->buf);
    appcmd_results[1] = (uint32)BUF_NUM_MSGS_AVAILABLE(buf_msg);

    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

#define PAGE_OF(pointer) ((pointer) >> MMU_LOG_PAGE_BYTES)
/**
 * Ensure the requested number of new pages are mapped in.  Before mapping we
 * limit the number of pages to map in to the number
 */
static void appcmd_buf_page_in(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];
    uint16f i, num_pages = appcmd_parameters[1];
    uint8 *buf_at_index;
    uint32 index_offset;

    /* Get the offset from index to an arbitrary point in the page after the one
     *  that was last mapped
     */
    uint32 index_next_page_offset = MMU_PAGE_ADDRESSES - 1;

    buf_at_index = buf_raw_write_map_8bit(&buf_msg->buf);

    index_offset = index_next_page_offset;
    /* Now write one location on each page to be mapped in until the offset
     * wraps around to the page the tail is in. */
    for (i = 0;
         i < num_pages && (BUF_GET_USED(&buf_msg->buf) == 0 ||
             PAGE_OF(BUF_ADD_INDICES(&buf_msg->buf,
                                     buf_msg->buf.index,
                                   index_offset)) != PAGE_OF(buf_msg->buf.tail));
         ++i, index_offset += MMU_PAGE_ADDRESSES)
    {
        buf_at_index[index_offset] = 0;
        L4_DBG_MSG3("appcmd: paged in page %d (offset %d) of buffer at 0x%08x",
                    PAGE_OF(BUF_ADD_INDICES(&buf_msg->buf,
                                            buf_msg->buf.index,index_offset)),
                    BUF_ADD_INDICES(&buf_msg->buf,
                                               buf_msg->buf.index,index_offset),
                    buf_msg);
    }

    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Add a message of the indicated length to the front of the \c BUFFER_MSG's
 * message ring.
 */
static void appcmd_buf_add_to_front(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];
    uint16 len = (uint16)appcmd_parameters[1];
    uint16 metadata_bytes = (uint16)(appcmd_parameters[1] >> 16);

    if(metadata_bytes)
    {
        uint8 * meta_p;
        assert(buf_msg->meta);
        meta_p = (uint8 *)buf_msg->meta + metadata_bytes * buf_msg->front;
        memcpy(meta_p, (uint8 *)&appcmd_parameters[2], metadata_bytes);
    }
    buf_add_to_front(buf_msg, len);

    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Return the number of unread messages in the \c BUFFER_MSG's message ring in
 * results[0].
 */
static void appcmd_buf_send_num_msg(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];

    appcmd_results[0] = BUF_NUM_MSGS_TO_SEND(buf_msg);
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Mark the current back message as read and return its length in results[0]
 */
static void appcmd_buf_update_back(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];
    uint16 metadata_bytes = (uint16)appcmd_parameters[1];
    /* Find the start of the back message's metadata based on what the client
     * says the metadata size is*/
    uint8 *back_meta = (uint8 *)buf_msg->meta + buf_msg->back * metadata_bytes;
    appcmd_results[0] = buf_get_back_msg_len(buf_msg);
    if(metadata_bytes)
    {
        assert(buf_msg->meta);
        assert(metadata_bytes < (sizeof(appcmd_results) - sizeof(uint32)));
        memcpy((uint8 *)&appcmd_results[1], back_meta, metadata_bytes);
    }
    buf_update_back(buf_msg);
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Free the behind message in the ring
 */
static void appcmd_buf_update_behind(void)
{
    BUFFER_MSG *buf_msg = (BUFFER_MSG *)appcmd_parameters[0];

    buf_update_behind_free(buf_msg);
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/*@}*/ /* appcmd_buf */


#ifdef MIB_MODULE_PRESENT
/**
 * appcmd_set_mib_key_handler
 *
 * This function is used to set an arbitrary numeric MIB
 * Assumes VLINT format
 */
static void appcmd_set_mib_key_handler(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    /* Note: it's conventional to pass uint8 arrays from host to firmware
     * packed into "little-endian" uint16s, i.e. as:
     *
     * word[i/2] = (octet[i+1]<<8 | octet[i])
     *
     * This is how ConfigCmd (Curator mode) (now) does it, if the -spread
     * option is not used.
     */
    mibkey mib_key = mibkeyfromid((mibid)(appcmd_parameters[0]));
    VLINT mib_val[MAXVLINTLEN];
    unsigned i,len = appcmd_parameters[1];
    
    memset(mib_val, 0, MAXVLINTLEN * sizeof(VLINT));
    
    for(i = 0; i < len; i++)
    {
        mib_val[2U*i] = appcmd_parameters[i+2] & 0xff;
        mib_val[2U*i+1] = (VLINT)(appcmd_parameters[i+2] >> 8);
    }

    if ((mib_key != MIBID_NULL))
    {
        uint32 value;
        if(vlint_negative(mib_val))
        {
            value = (uint32)vlint_to_int32(mib_val);
        }
        else
        {
            value = vlint_to_uint32(mib_val);
        }
        L2_DBG_MSG3("APPCMD MIB SET: KEY=0x%04x VAL=0x%x%04x",
                    appcmd_parameters[0], (uint16)(value >>16), (uint16)value);
        UNUSED(value);

        if (mibsetint(mib_key, mib_val))
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        }
        else
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        }
    }
    else
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
    }
#endif /* DESKTOP_TEST_BUILD */
}

/*
 * KEY_ID
 * POSITION
 * VALUE
 *
 * Fail if non octet string key.
 * Fail if erm ... dodgy position
 */
static void appcmd_set_mib_octet_key_handler(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    mibkey mib_key = mibkeyfromid((mibid)(appcmd_parameters[0]));
    uint16 position_in_octet_array = (uint16)appcmd_parameters[1];
    /* VLINT *mib_val = (VLINT *)&parameters[2]; */
    uint16 mib_val = (uint16)appcmd_parameters[2];
    static BMSG *my_bmsg = NULL;
    static mibkey current_mib_key = MIBKEY_NULL;

    L2_DBG_MSG3("APPCMD MIB OCTET SET: KEY=0x%04x POSN=0x%04x VAL=0x%04x",
                            appcmd_parameters[0], position_in_octet_array, mib_val);

    if(mib_key == MIBID_NULL)
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        return;
    }

    /* We only set string keys with this function */
    if(mibtypeint(mib_key))
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        return;
    }

    if((current_mib_key != MIBKEY_NULL) && (current_mib_key != mib_key))
    {
        /* Setting an octet in a new mib key before the previous one has been
         * submitted. Best course of action is to submit the pending one 
         * (ignoring any error the set may return).
         */
        L0_DBG_MSG1("APPCMD MIB SET WARNING: KEY=0x%04x implicitly submitted",
                                                    (uint16)current_mib_key);
        (void)mibsetos(current_mib_key, my_bmsg);
        bmsg_destroy(my_bmsg);
        my_bmsg = NULL;
        current_mib_key = MIBKEY_NULL;
    }

    if(current_mib_key == MIBKEY_NULL)
    {
        uint16 i;

        /*
         * The BMSG is an array of octets so it DOESN'T have the VLINT header
         * Create a bmsg
         * Pad to the index being set (assumes the highest index is set first)
         */
        my_bmsg = bmsg_create();
        if(!my_bmsg)
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
            return;
        }
     
        /* Zero pad value with VLINTs */
        for(i = 0; i <= position_in_octet_array; i++)
        {
            if(!bmsg_putoctet(my_bmsg, 0))
            {
                APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
                bmsg_destroy(my_bmsg);
                my_bmsg = NULL;
                current_mib_key = MIBKEY_NULL;
                return;
            }
        }
        current_mib_key = mib_key;
    }

    /* Poke in the octet we want to set */
    if(!bmsg_patch_octet(my_bmsg, position_in_octet_array, (uint8)mib_val))
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
        bmsg_destroy(my_bmsg);
        my_bmsg = NULL;
        current_mib_key = MIBKEY_NULL;
        return;
    }

    if(position_in_octet_array != 0)
    {
        /* Keep the array ready for the next octet */
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
    }
    else
    {
        /* Pass back to the MIB then destroy */
        L2_DBG_MSG1("APPCMD MIB OCTET SET: KEY=0x%04x submitted", appcmd_parameters[0]);
        if(mibsetos(mib_key, my_bmsg))
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        }
        else
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
        }
        bmsg_destroy(my_bmsg);
        my_bmsg = NULL;
        current_mib_key = MIBKEY_NULL;
    }
#endif /* DESKTOP_TEST_BUILD */
}

/**
 * appcmd_get_mib_key_handler
 *
 * This function is used to get an arbitrary numeric MIB
 * Assumes VLINT format
 */
static void appcmd_get_mib_key_handler(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    /* Note: it's conventional to pass uint8 arrays from host to firmware
     * packed into "little-endian" uint16s, i.e. as:
     *
     * word[i/2] = (octet[i+1]<<8 | octet[i])
     *
     * This is how ConfigCmd (Curator mode) (now) does it, if the -spread
     * option is not used.
     */
    mibkey mib_key = mibkeyfromid((mibid)(appcmd_parameters[0]));
    VLINT mib_val[MAXVLINTLEN];
    unsigned i,len;
    
    memset(appcmd_results, 0, APPCMD_MISC_VALUES_CMD_BUF_LEN * sizeof(uint32));
    
    if ((mib_key != MIBID_NULL))
    {
        if (mibgetint(mib_key, mib_val))
        {
            if(vlint_data_length(mib_val) == 0)
            {
                L2_DBG_MSG1("APPCMD MIB GET: KEY=0x%04x VAL=[]",
                            appcmd_parameters[0]);
            }
            else
            {
                uint32 value;
                if(vlint_negative(mib_val))
                {
                    value = (uint32)vlint_to_int32(mib_val);
                }
                else
                {
                    value = vlint_to_uint32(mib_val);
                }
                L2_DBG_MSG3("APPCMD MIB GET: KEY=0x%04x VAL=0x%x%04x",
                            appcmd_parameters[0],
                            (uint16)(value >>16),
                            (uint16)value);
                UNUSED(value);
            }
            appcmd_results[0] = mib_key;
            len = vlint_length(mib_val);
            appcmd_results[1] = (len + 1)/2;
            for(i = 0; i < len; i++)
            {
                appcmd_results[i+2] = mib_val[2U*i] | (mib_val[2U*i+1]<<8);
            }
            
            APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        }
        else
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        }
    
    }
    else
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
    }
    
#endif /* DESKTOP_TEST_BUILD */
}

/**
 * appcmd_get_mib_octet_key_handler
 *
 * This function is used to get an arbitrary octet string MIB.
 * Fails if MIB is an integer.
 */
static void appcmd_get_mib_octet_key_handler(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    mibkey mib_key = mibkeyfromid((mibid)(appcmd_parameters[0]));
    uint16 position_in_octet_array = (uint16)appcmd_parameters[1];
    BMSG *my_bmsg;
    unsigned tmp_outdex;
    
    memset(appcmd_results, 0, APPCMD_MISC_VALUES_CMD_BUF_LEN * sizeof(uint32));

    if(mib_key == MIBID_NULL)
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        return;
    }

    /* We only get string keys with this function */
    if(mibtypeint(mib_key))
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
        return;
    }

    /*
     * The BMSG is an array of octets so it DOESN'T have the VLINT header
     * Create a bmsg
     */
    my_bmsg = bmsg_create();
    if(!my_bmsg)
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
        return;
    }
        
    /*get octet string*/
    if(!mibgetos(mib_key, my_bmsg))
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
        bmsg_destroy(my_bmsg);
        return;
    }

    appcmd_results[0] = appcmd_parameters[0];
    appcmd_results[1] = position_in_octet_array;
    tmp_outdex = bmsg_get_outdex(my_bmsg);
    bmsg_set_outdex(my_bmsg, position_in_octet_array);
    if(!bmsg_getoctet(my_bmsg, (uint8 *)&appcmd_results[2]))
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_UNSPECIFIED);
        bmsg_destroy(my_bmsg);
        return;
    }
    bmsg_set_outdex(my_bmsg, tmp_outdex);
    
    L2_DBG_MSG3("APPCMD MIB OCTET GET: KEY=0x%04x POSN=0x%04x VAL=0x%04x",
                        appcmd_results[0], appcmd_results[1], appcmd_results[2]);
                        
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);

    bmsg_destroy(my_bmsg);
#endif /* DESKTOP_TEST_BUILD */
}

/**
 * appcmd_get_mib_octet_length_handler
 *
 * This function is used to get the length of an arbitrary octet string MIB.
 * Fails if MIB is an integer or not set.
 */
static void appcmd_get_mib_octet_length_handler(void)
{
#if !defined(DESKTOP_TEST_BUILD)
    mibkey mib_key = mibkeyfromid((mibid)(appcmd_parameters[0]));
    int16 length = mibstrlen(mib_key);
        
    memset(appcmd_results, 0, APPCMD_MISC_VALUES_CMD_BUF_LEN * sizeof(uint32));
    
    if(length >= 0)
    {
        appcmd_results[0] = appcmd_parameters[0];
        appcmd_results[1] = (uint16)length;
        L2_DBG_MSG2("APPCMD MIB OCTET LENGTH: KEY=0x%04x LEN=0x%04x",
                    appcmd_results[0], appcmd_results[1]);
                    
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
    }
    else
    {
        APPCMD_RESPOND(APPCMD_RESPONSE_INVALID_PARAMETERS);
    }
#endif /* DESKTOP_TEST_BUILD */
}

#endif /* MIB_MODULE_PRESENT */

/**
 * appcmd_get_buffer_size_handler
 *
 * This function is used to get the buffer size.
 */
static void appcmd_get_buffer_size_handler(void)
{
    appcmd_results[0] = APPCMD_MISC_VALUES_CMD_BUF_LEN * sizeof(uint32);
    APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
}

/**
 * Calls the built-in memset
 * @param at Address to write at
 * @param value Value to write
 * @param length Number of bytes to write
 */
static void appcmd_memset(void *at, unsigned char value, size_t length)
{
    memset(at, value, length);
}

/**
 * Checks that a given block of memory contains the given value in all bytes
 * @param at Start address
 * @param value Value to check for
 * @param length Length of the block
 * @return TRUE if it does, FALSE if it doesn't.
 */
static bool appcmd_memcheck(void *at, unsigned char value, size_t length)
{
    unsigned i;
    for (i = 0; i < length; ++i)
    {
        if (*((unsigned char *)at + i) != value)
        {
            return FALSE;
        }
    }
    return TRUE;
}



void appcmd_background_handler(void)
{
    APPCMD_COMMAND_ID cmd = (APPCMD_COMMAND_ID)(appcmd_cmd & APPCMD_MASK);

    switch(cmd)
    {
#ifdef MIB_MODULE_PRESENT
    case APPCMD_SET_MIB_KEY:
        appcmd_set_mib_key_handler();
        break;
    case APPCMD_SET_MIB_OCTET_KEY:
        appcmd_set_mib_octet_key_handler();
        break;
    case APPCMD_GET_MIB_KEY:
        appcmd_get_mib_key_handler();
        break;
    case APPCMD_GET_MIB_OCTET_KEY:
        appcmd_get_mib_octet_key_handler();
        break;
    case APPCMD_GET_MIB_OCTET_LENGTH:
        appcmd_get_mib_octet_length_handler();
        break;
#endif /* MIB_MODULE_PRESENT */
    case APPCMD_GET_BUFFER_SIZE:
        appcmd_get_buffer_size_handler();
        break;
    case APPCMD_START_TEST:
        if(!call_test_handler_fn())
        {
            APPCMD_RESPOND(APPCMD_RESPONSE_UNIMPLEMENTED);
        }
        break;
    case APPCMD_BUF_MSG_HANDLE_REQ:
        appcmd_buf_return_handle();
        break;
    case APPCMD_BUF_MSG_FREE_SPACE_REQ:
        appcmd_buf_return_free_space();
        break;
    case APPCMD_BUF_MSG_PAGE_REQ:
        appcmd_buf_page_in();
        break;
    case APPCMD_BUF_MSG_WRITE_MSG_IND:
        appcmd_buf_add_to_front();
        break;
    case APPCMD_BUF_MSG_NUM_MSG_REQ:
        appcmd_buf_send_num_msg();
        break;
    case APPCMD_BUF_MSG_READ_MSG_REQ:
        appcmd_buf_update_back();
        break;
    case APPCMD_BUF_MSG_CONSUMED_MSG_IND:
        appcmd_buf_update_behind();
        break;
#ifndef DESKTOP_TEST_BUILD
    case APPCMD_CALL_FUNCTION:
        /* This is an assembly function - see appcmd.asm */
        appcmd_call_function();
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        break;
#ifdef ENABLE_SHALLOW_SLEEP_TIMING
    case APPCMD_GET_CPU_USAGE:
        appcmd_results[0] = hal_get_reg_timer_time();
        appcmd_results[1] = total_shallow_sleep_time;
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        break;
#endif /* ENABLE_SHALLOW_SLEEP_TIMING */
#endif
    case APPCMD_MEMSET:
        appcmd_memset((void *)appcmd_parameters[0],
                      (unsigned char)appcmd_parameters[1],
                      (size_t)appcmd_parameters[2]);
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        break;
    case APPCMD_MEMCHECK:
        appcmd_results[0] = appcmd_memcheck((void *)appcmd_parameters[0],
                                            (unsigned char)appcmd_parameters[1],
                                            (size_t)appcmd_parameters[2]);
        APPCMD_RESPOND(APPCMD_RESPONSE_SUCCESS);
        break;
    default:
        L0_DBG_MSG1("APPCMD: Cmd 0x%x not recognised", cmd);
        APPCMD_RESPOND(APPCMD_RESPONSE_UNKNOWN_COMMAND);
        break;
    }
}

void appcmd_test_cb(void)
{

}

/**
 * appcmd_init
 *
 * Initialise the APPCMD Implementation
 *
 */
void appcmd_init(void)
{
    appcmd_enable_interrupts(TRUE);
    appcmd_rsp = (uint16)APPCMD_RESPONSE_SUCCESS | 0x8000;
    (void)appcmd_rsp;

}

/**
 * appcmd_interrupt_handler
 *
 * Message the background with the user's command.
 *
 */
#ifndef DESKTOP_TEST_BUILD
static void appcmd_interrupt_handler(void)
{
    GEN_BG_INT(appcmd_bg_handler);
}
#endif

/**
 * appcmd_enable_interrupts
 *
 * Enable or disable interrupts for appcmd events
 *
 */
void appcmd_enable_interrupts(bool enable)
{
#if defined(DESKTOP_TEST_BUILD)
    UNUSED(enable);
#else
    if (enable)
    {
        /*
         * Enable the interrupt vectored into the handler above.
         */
        configure_interrupt(SW_INT_SOURCE_APPCMD, INT_LEVEL_FG,
                            appcmd_interrupt_handler);
    }
    else
    {
        /*
         * Disable the interrupt; we don't need to remove the
         * handler.
         */
        enable_interrupt(SW_INT_SOURCE_APPCMD, FALSE);
    }
#endif
}

bool appcmd_add_test_handler(APPCMD_TEST_ID command,
                             appcmd_test_handler_fn * fn)
{
    bool room_for_test_handlers = (handler_count < sizeof(test_handlers)/sizeof(test_handlers[0]));
    if (room_for_test_handlers)
    {
        test_handlers[handler_count].command = command;
        test_handlers[handler_count].fn = fn;
        ++handler_count;
    }
    assert(room_for_test_handlers);
    return room_for_test_handlers;
}

static bool call_test_handler_fn(void)
{
    uint32 i;
    APPCMD_TEST_ID test_id = (APPCMD_TEST_ID)appcmd_parameters[0];
    for(i=0; i<handler_count; ++i)
    {
        if(test_handlers[i].command == test_id)
        {
            APPCMD_RESPOND(test_handlers[i].fn(test_id,
                                               &appcmd_parameters[1],
                                               appcmd_results));
            return TRUE;
        }
    }
    L0_DBG_MSG1("APPCMD: No Test for test command 0x%x",
                                                appcmd_parameters[0]);
    return FALSE;
}


#define POPULATE_APPCMD_RSP_PACKET(pkt, cmd, rsp, results, num_results)\
    do {\
        (pkt)[0] = (cmd);\
        (pkt)[1] = (rsp);\
        memcpy(&(pkt)[2], (results), (num_results)*sizeof(uint32));\
    } while (0)


void appcmd_add_test_rsp_packet_source(uint32 command /* test ID */,
                                       uint32 rsp,
                                       const uint32 *results,
                                       uint16f num_results,
                                       bool remote_packet)
{
    /* Until we have the real test tunnel, we have an internal appcmd buffer
     * that the results go into.  This is shared by p0 and p1 and lives on p0. */

    uint16 appcmd_pkt_len_bytes = (uint16)((2 + num_results)*sizeof(uint32));

    /* On P1 we have to submit the packet to IPC as a real test tunnel packet.
     * Hence we won't notice when P0 switches from the appcmd_rsp_buf to the
     * real test tunnel */
    uint16f tnl_pkt_len_bytes = TEST_TUNNEL_HEADER_WORD_SIZE *sizeof(uint32) +
                                                        appcmd_pkt_len_bytes;
    uint32 *test_tunnel_pkt = pmalloc(tnl_pkt_len_bytes);
    TEST_TUNNEL_HEADER *test_tunnel_hdr = (TEST_TUNNEL_HEADER *)test_tunnel_pkt;

    assert(!remote_packet); /* It doesn't make sense for P1 to submit a packet
                               that arrived over IPC */

    TEST_TUNNEL_HEADER_TUNNEL_ID_SET(test_tunnel_hdr, TEST_TUNNEL_ID_APPS1_FW_TEST);
    POPULATE_APPCMD_RSP_PACKET(&test_tunnel_pkt[TEST_TUNNEL_HEADER_WORD_SIZE],
                               command, rsp, results, num_results);
    ipc_send_outband(IPC_SIGNAL_ID_TEST_TUNNEL_PRIM, test_tunnel_pkt,
                                                             tnl_pkt_len_bytes);
}

/*@{*/
void appcmd_sched_init(void **gash)
{
    UNUSED(gash);
    L4_DBG_MSG("APPCMD: Initialising task");
}

void appcmd_sched_msg_handler(void **gash)
{
    uint16 mi;
    void *mv;

    UNUSED(gash);

    L4_DBG_MSG("APPCMD: Task handler invoked!");
    if (get_message(appcmd_sched_queue_id, &mi, &mv))
    {
        L4_DBG_MSG2("APPCMD: Got a message with mi = 0x%x and mv = 0x%08x",
                    mi, mv);
    }
    else
    {
        L1_DBG_MSG("APPCMD: Message handler invoked but get_message failed!");
    }
}

/*@}*/

