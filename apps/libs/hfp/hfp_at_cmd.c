/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_at_cmd.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include <string.h>

/****************************************************************************
NAME    
    HfpAtCmdRequest

DESCRIPTION
    Send an AT command from the application

MESSAGE RETURNED
    HFP_AT_CMD_CFM will be sent on OK/ERROR from the AG. Any other response 
    from the AG will be returned via HFP_UNRECOGNISED_AT_CMD_IND

RETURNS
    void
*/
void HfpAtCmdRequest(hfp_link_priority priority, const char* cmd)
{
    uint8 size_cmd = (uint8)(cmd ?
                    /* cmd is not NULL, so is the string too long? */
                    (strlen(cmd) > HFP_MAX_ARRAY_LEN ?
                        /* The string is too long: truncate it. */
                        HFP_MAX_ARRAY_LEN :
                        /* The string is not too long: use it all. */
                        strlen(cmd))
                    /* cmd is NULL. */
                    : 0);
    MAKE_HFP_MESSAGE_WITH_LEN(HFP_INTERNAL_AT_CMD_REQ, size_cmd + 1);
    message->link = hfpGetLinkFromPriority(priority);
    message->size_cmd = size_cmd;
    if(cmd) 
    {
        strncpy(message->cmd, cmd, (size_t) size_cmd);
    }
    /* Ensure terminating NULL on the string */
    message->cmd[size_cmd] = '\0';
    MessageSend(&theHfp->task, HFP_INTERNAL_AT_CMD_REQ, message);
}
