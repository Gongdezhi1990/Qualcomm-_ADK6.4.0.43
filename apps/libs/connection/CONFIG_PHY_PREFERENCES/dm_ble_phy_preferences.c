/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_phy_preferences.c      

DESCRIPTION
    Handler functions for PHY Preferences, this allows the configuration
    of 2Mbps LE.

NOTES

*/

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_phy_preferences.h"

#include <vm.h>

#ifndef DISABLE_BLE

/*******************************************************************************
 *                      Message handling function prototypes                   *
 *******************************************************************************/
void connectionHandleDmUlpPhyUpdateInd( 
        const DM_ULP_PHY_UPDATE_IND_T *ind
        );
        
void connectionHandleDmUlpSetPhyCfm( 
        const DM_ULP_SET_PHY_CFM_T *ind
        );        
        
void connectionHandleDmUlpSetDefaultPhyCfm( 
        const DM_ULP_SET_DEFAULT_PHY_CFM_T *ind
        );      
        
/****************************************************************************
NAME    
    ConnectionDmUlpSetPhy

DESCRIPTION
    Set PHY preferences

RETURNS
   void
*/
void ConnectionDmUlpSetPhy( 
        const tp_bdaddr  *tpaddr,
        const uint8 min_tx,
        const uint8 max_tx,
        const uint8 min_rx,
        const uint8 max_rx,
        const uint8 flags
        )
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_T(DM_ULP_SET_PHY_REQ);
        
        prim->phandle = 0;
        BdaddrConvertTpVmToBluestack(&prim->tp_addrt, tpaddr);

        prim->min_tx = (phy_rate_t)min_tx;        /* Minimum preferred tx rate */
        prim->max_tx = (phy_rate_t)max_tx;        /* Maximum preferred tx rate */
        prim->min_rx = (phy_rate_t)min_rx;        /* Minimum preferred rx rate */
        prim->max_rx = (phy_rate_t)max_rx;        /* Maximum preferred rx rate */
        prim->flags  = (phy_rate_t)flags;         /* Flags for additional preferences */

        VmSendDmPrim(prim);
    }
}        

/****************************************************************************
NAME    
    ConnectionDmUlpSetDefaultPhy

DESCRIPTION
    Set Default PHY preferences

RETURNS
   void
*/
void ConnectionDmUlpSetDefaultPhy( 
        const uint8 min_tx,
        const uint8 max_tx,
        const uint8 min_rx,
        const uint8 max_rx,
        const uint8 flags
        )
{
    if (connectionGetBtVersion() < bluetooth4_0)
    {
        CL_DEBUG(("Bluestack does not support low energy (BT 4.0)\n"));
    }
    else
    {
        MAKE_PRIM_T(DM_ULP_SET_DEFAULT_PHY_REQ);
        
        prim->phandle = 0;

        prim->min_tx = (phy_rate_t)min_tx;        /* Minimum preferred tx rate */
        prim->max_tx = (phy_rate_t)max_tx;        /* Maximum preferred tx rate */
        prim->min_rx = (phy_rate_t)min_rx;        /* Minimum preferred rx rate */
        prim->max_rx = (phy_rate_t)max_rx;        /* Maximum preferred rx rate */
        prim->flags  = (phy_rate_t)flags;         /* Flags for additional preferences */

        VmSendDmPrim(prim);
    }
}  

/****************************************************************************
NAME    
    connectionHandleDmUlpPhyUpdateInd

DESCRIPTION
    Handle the DM_ULP_PHY_UPDATE_IND message from Bluestack
    and pass it to the application. 

RETURNS
    void
*/
void connectionHandleDmUlpPhyUpdateInd( 
        const DM_ULP_PHY_UPDATE_IND_T *ind
        ) 
{
    MAKE_CL_MESSAGE(CL_DM_ULP_PHY_UPDATE_IND);

    /* Convert the Bluestack TP_BD_ADDR_T address */
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    
    /* PHY types updated */
    message->tx_phy_type = ind->tx_phy_type;
    message->rx_phy_type = ind->rx_phy_type;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_ULP_PHY_UPDATE_IND,
            message);
}

void connectionHandleDmUlpSetPhyCfm( 
        const DM_ULP_SET_PHY_CFM_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_ULP_SET_PHY_CFM);
    
    /* Convert the Bluestack TP_BD_ADDR_T address */
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    
    /* PHY types updated */
    message->tx_phy_type = ind->tx_phy_type;
    message->rx_phy_type = ind->rx_phy_type;

    message->status      = ind->status;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_ULP_SET_PHY_CFM,
            message);
}

     
void connectionHandleDmUlpSetDefaultPhyCfm( 
        const DM_ULP_SET_DEFAULT_PHY_CFM_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_ULP_SET_DEFAULT_PHY_CFM);
    
    message->status = ind->status;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_ULP_SET_DEFAULT_PHY_CFM,
            message);
}        

/****************************************************************************
NAME    
    connectionBluestackHandlerDmPhyPreferences

DESCRIPTION
    Handler for PHY Preferences messages

RETURNS
    TRUE if message handled, otherwise FALSE
*/
bool connectionBluestackHandlerDmPhyPreferences(const DM_UPRIM_T *message)
{
    switch (message->type)
        {
        case DM_ULP_PHY_UPDATE_IND :
                CL_DEBUG_INFO(("DM_ULP_PHY_UPDATE_IND\n"));
                connectionHandleDmUlpPhyUpdateInd(
                        (DM_ULP_PHY_UPDATE_IND_T *)message );
                return TRUE;
        case DM_ULP_SET_PHY_CFM:
                CL_DEBUG_INFO(("DM_ULP_SET_PHY_CFM\n"));
                connectionHandleDmUlpSetPhyCfm(
                        (DM_ULP_SET_PHY_CFM_T *)message );
                return TRUE;                     
        case DM_ULP_SET_DEFAULT_PHY_CFM:
                CL_DEBUG_INFO(("DM_ULP_SET_DEFAULT_PHY_CFM\n"));
                connectionHandleDmUlpSetDefaultPhyCfm(
                        (DM_ULP_SET_DEFAULT_PHY_CFM_T *)message );
                return TRUE;                     
        default:
                return FALSE;
        }
}

#endif /* DISABLE_BLE */

/* End-of-File */
