/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_channel_selection_algorithm.c      

DESCRIPTION
    Message handler function for forwarding the Channel Selection Algorithm
    Indication message from Bluestack to the Application task.

NOTES

*/

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "bdaddr.h"
#include "dm_ble_channel_selection_algorithm.h"

#include <vm.h>

#ifndef DISABLE_BLE

/*******************************************************************************
 *                      Message handling function prototypes                   *
 *******************************************************************************/
void connectionHandleDmBleChannelSelectionAlgorithmInd( 
        const DM_HCI_ULP_CHANNEL_SELECTION_ALGORITHM_IND_T *ind
        );

/****************************************************************************
NAME    
    connectionHandleDmBleChannelSelectionAlgorithmInd

DESCRIPTION
    Handle the DM_HCI_ULP_CHANNEL_SELECTION_ALGORITH_IND message from Bluestack
    and pass it to the application. 

RETURNS
    void
*/
void connectionHandleDmBleChannelSelectionAlgorithmInd( 
        const DM_HCI_ULP_CHANNEL_SELECTION_ALGORITHM_IND_T *ind
        ) 
{
    MAKE_CL_MESSAGE(CL_DM_BLE_CHANNEL_SELECTION_ALGORITHM_IND);
    
    /* Convert the Bluestack TP_BD_ADDR_T address */
    BdaddrConvertTpBluestackToVm(&message->tpaddr, &ind->tp_addrt);
    
    /* Convert the channel selection algorithm - uint8 value - to CL
       enum. */
    switch (ind->channel_selection_algorithm)
    {
        case 0x00:
            message->selected_algorithm = channel_selection_algorithm_1;
            break;
        case 0x01:
            message->selected_algorithm = channel_selection_algorithm_2;
            break;
        default:
            message->selected_algorithm = channel_selection_algorithm_invalid;
            break;
    }

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_CHANNEL_SELECTION_ALGORITHM_IND,
            message);
}

/****************************************************************************
NAME    
    connectionBluestackHandlerDmChannelSelectionAlgorithm

DESCRIPTION
    Handler for Channel Selection Algorithm messages

RETURNS
    TRUE if message handled, otherwise false
*/
bool connectionBluestackHandlerDmChannelSelectionAlgorithm(const DM_UPRIM_T *message)
{
    switch (message->type)
        {
        case DM_HCI_ULP_CHANNEL_SELECTION_ALGORITHM_IND :
                CL_DEBUG_INFO(("DM_HCI_ULP_CHANNEL_SELECTION_ALGORITHM_IND\n"));
                connectionHandleDmBleChannelSelectionAlgorithmInd(
                        (DM_HCI_ULP_CHANNEL_SELECTION_ALGORITHM_IND_T *)message );
                return TRUE;
        default:
                return FALSE;
        }
}

#endif /* DISABLE_BLE */

/* End-of-File */
