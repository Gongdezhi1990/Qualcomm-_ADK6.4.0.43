/****************************************************************************
Copyright (c) 2018 - 2019 Qualcomm Technologies International, Ltd 


FILE NAME
    dm_ble_latency.c      

DESCRIPTION
    This file contains the implementation of configuration of latency control.

NOTES

*/



#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_latency.h"

#include <vm.h>

#ifndef DISABLE_BLE

/*******************************************************************************
 *                      Message handling function prototypes                   *
 *******************************************************************************/
void connectionHandleDmUlpEnableSlaveLatencyCfm( 
        const DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM_T *ind
        );

/****************************************************************************
NAME    
    ConnectionDmUlpEnableZeroSlaveLatency

DESCRIPTION
    Send a Zero Slave Latency request

RETURNS
   void
*/
void ConnectionDmUlpEnableZeroSlaveLatency( 
        const tp_bdaddr  *tpaddr,
        bool  zero_latency
        )
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_T(DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_REQ);
        
        prim->phandle = 0;
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, tpaddr);
        prim->zero_latency = zero_latency;

        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME    
    connectionHandleDmUlpEnableSlaveLatencyCfm

DESCRIPTION
    Handle the DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM_T message from Bluestack and pass it
    on to the application that initialised the CL.

RETURNS
    void
*/
void connectionHandleDmUlpEnableSlaveLatencyCfm( 
        const DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM_T *ind
        ) 
{
    MAKE_CL_MESSAGE(CL_DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM);
    
    /* Convert the Bluestack TP_BD_ADDR_T address */
    BdaddrConvertTpBluestackToVm(&message->taddr, &ind->tp_addrt);
    
    message->zero_latency = ind->zero_latency;
    message->status = ind->status;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM,
            message);
}

/****************************************************************************
NAME    
    connectionBluestackHandlerDmLatency

DESCRIPTION
    Handler for Latency messages

RETURNS
    TRUE if message handled, otherwise false
*/
bool connectionBluestackHandlerDmLatency(const DM_UPRIM_T *message)
{
    switch (message->type)
        {
        case DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM:
                CL_DEBUG_INFO(("DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM\n"));
                connectionHandleDmUlpEnableSlaveLatencyCfm(
                        (DM_ULP_ENABLE_ZERO_SLAVE_LATENCY_CFM_T *)message );  
                return TRUE;
        default:
                return FALSE;
        }    
}


#endif /* DISABLE_BLE */

