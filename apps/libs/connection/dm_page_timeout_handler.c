/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_page_timeout_handler.c        

DESCRIPTION
    This file contains the handler for the internal functions used
    to configure the page timeout.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_page_timeout_handler.h"

#include <vm.h>


/****************************************************************************/
void connectionHandleWritePageTimeout(const CL_INTERNAL_DM_WRITE_PAGE_TIMEOUT_REQ_T *req)
{
    MAKE_PRIM_C(DM_HCI_WRITE_PAGE_TIMEOUT_REQ);     
    prim->page_timeout = req->timeout;
    VmSendDmPrim(prim);
}
