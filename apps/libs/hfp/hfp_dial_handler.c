/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_dial_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_common.h"
#include "hfp_dial_handler.h"
#include "hfp_indicators_handler.h"
#include "hfp_send_data.h"
#include "hfp_call_handler.h"

#include <panic.h>
#include <string.h>

static const char prefix[] = {'A','T','D','>'};
static const char suffix[] = {';','\r'};

/* 
    The code for creating the dial cmd for dialing a number and a memory location 
    is almost the same so pull out common code into this function.
*/
static void createDialCmd(hfp_link_data* link, bool memory, uint16 size_number, const uint8 *number)
{
    /* Only copy the '>' if doing memory dial */
    uint8 size_prefix = sizeof(prefix) - (memory ? 0 : 1);
    uint8 size_suffix = sizeof(suffix);
    unsigned size_dial_command = size_prefix + size_number + size_suffix;

    /* Create the dial request message */
    char *dial_command = (char *) PanicUnlessMalloc(size_dial_command);
    
    /* Locations we want to copy data to... */
    char* number_start = dial_command + size_prefix;
    char* suffix_start = number_start + size_number;

    /* Copy data to the command */
    memmove(dial_command, prefix, size_prefix);
    memmove(number_start, number, size_number);
    memmove(suffix_start, suffix, size_suffix);

    /* Send the message over the air */
    hfpSendAtCmd(link, (uint16)size_dial_command, dial_command, (hfp_at_cmd)(memory ? hfpAtdMemoryCmdPending : hfpAtdNumberCmdPending));

    /* Free up the allocated memory since the cmd has been copied into the RFC buffer */
    free(dial_command);
}


/****************************************************************************
NAME    
    hfpHandleLastNumberRedial

DESCRIPTION
    Issue a last number redial request.

RETURNS
    void
*/
void hfpHandleLastNumberRedial(hfp_link_data* link)
{
    char bldn[] = "AT+BLDN\r";

    /* Send the AT cmd over the air */
    hfpSendAtCmd(link, sizeof(bldn)-1, bldn, hfpBldnCmdPending);
}


/****************************************************************************
NAME    
    hfpHandleDialNumberRequest

DESCRIPTION
    Issue a request to dial the supplied number.

RETURNS
    void
*/
void hfpHandleDialNumberRequest(const HFP_INTERNAL_AT_ATD_NUMBER_REQ_T *req)
{
    /* Create and send the dial cmd */
    createDialCmd(req->link, FALSE, req->length, req->number);
}


/****************************************************************************
NAME    
    hfpHandleDialMemoryRequest

DESCRIPTION
    Send a dial memory location request to the AG.

RETURNS
    void
*/
void hfpHandleDialMemoryRequest(const HFP_INTERNAL_AT_ATD_MEMORY_REQ_T *req)
{
    /* Create and send the dial request */
    createDialCmd(req->link, TRUE, req->length, req->memory);
}
