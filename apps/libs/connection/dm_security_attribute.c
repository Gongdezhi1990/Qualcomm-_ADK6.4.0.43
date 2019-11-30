/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_attribute.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "connection_tdl.h"

#include <message.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionSmPutAttributeReq(
        uint16 ps_base, 
        uint8 addr_type,
        const bdaddr* bd_addr, 
        uint16 size_psdata, 
        const uint8* psdata
        )
{
    connectionAuthPutAttribute(
            ps_base, 
            addr_type, 
            bd_addr, 
            size_psdata, 
            psdata
            );
}


/*****************************************************************************/
void ConnectionSmGetAttributeReq(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr,
        uint16 size_psdata
        )
{
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_ATTRIBUTE_REQ);
        message->addr_type      = addr_type;
        message->bd_addr        = *bd_addr;
        message->ps_base        = ps_base;
        message->size_psdata    = size_psdata;
        MessageSend(
                connectionGetCmTask(),
                CL_INTERNAL_SM_GET_ATTRIBUTE_REQ,
                message
                );
    }
}

/*****************************************************************************/
void ConnectionSmGetIndexedAttribute(
        uint16 ps_base,
        uint16 index,
        uint16 size_psdata
        )
{
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ);
        message->index = index;
        message->ps_base = ps_base;
        message->size_psdata = size_psdata;
        MessageSend(
                connectionGetCmTask(),
                CL_INTERNAL_SM_GET_INDEXED_ATTRIBUTE_REQ,
                message
                );
    }
}


/*****************************************************************************/
bool ConnectionSmGetAttributeNowReq(
        uint16 ps_base,
        uint8 addr_type,
        const bdaddr* bd_addr, 
        uint16 size_psdata, 
        uint8 *psdata
        )
{
	return connectionAuthGetAttributeNow(
            ps_base,
            addr_type,
            bd_addr,
            size_psdata,
            psdata
            );
}


/*****************************************************************************/
bool ConnectionSmGetIndexedAttributeNowReq(
        uint16 ps_base, 
        uint16 index,
        uint16 size_psdata,
        uint8 *psdata,
        typed_bdaddr *taddr
        )
{
	return connectionAuthGetIndexedAttributeNow(
            ps_base,
            index,
            size_psdata,
            psdata,
            taddr
            );
}

	
