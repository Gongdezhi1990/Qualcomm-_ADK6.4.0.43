/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp_sdp.c

DESCRIPTION


NOTES

*/


/****************************************************************************
    Header files
*/
#include "a2dp.h"
#include "a2dp_init.h"
#include "a2dp_l2cap_handler.h"
#include "a2dp_private.h"
#include "a2dp_sdp.h"

#include <connection.h>


/*****************************************************************************/
void a2dpHandleSdpRegisterCfm(const CL_SDP_REGISTER_CFM_T *cfm)
{
    /* Log received cfm message */
    a2dp->bitfields.sdp_register_outstanding--;

    if (cfm->status==success)
    {
        /* Register the l2cap psm if all service records have been registered */
        if ( a2dp->bitfields.sdp_register_outstanding==0 )
        {
            a2dpRegisterL2cap();
        }
    }
    else
    {
        /* Send indication that the initialisation failed */
        a2dpSendInitCfmToClient(a2dp_sdp_fail);
    }
}
