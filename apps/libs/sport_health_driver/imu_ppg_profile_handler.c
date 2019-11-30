/****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    imu_ppg_profile_handler.c

DESCRIPTION
    This file contains the profile handler for the IMU and PPG device
    library.

NOTES

*/

/****************************************************************************
    Header files
*/
#include "imu_ppg_private.h"
#include "sport_health_driver.h"
#include "imu_ppg_message_handler.h"
#include "imu_ppg_profile_handler.h"

#include <message.h>
#include <print.h>
#include "macros.h"
#include "hydra_log.h"
/*****************************************************************************/
bool bitSerialTransferComplete = 1;

/** Handles any unexpected message */
static void handle_unexpected(uint16 type)
{
#if 0
    IMUPPG_FATAL_IN_DEBUG(("IMUPPG handleUnexpected - MsgId 0x%x\n", type));
#endif
    UNUSED(type); /* only used in debug */
}

/**
 * @brief Message handler for IMUPPG task
 * @param task Task ID
 * @param id Message ID
 * @param message Message content
 */
void imu_ppg_profile_handler(Task task, MessageId id, Message message)
{

    UNUSED(task);
    #ifdef DEBUG_MSG_FLOW
    L0_DBG_MSG("IMUPPG Task: Reached message handler");
    #endif
    switch (id)
    {
    case MESSAGE_BITSERIAL_EVENT:
        PRINT(("MESSAGE_BITSERIAL_EVENT\n"));
        bitSerialTransferComplete = 0;
        break;

    case IMU_CONFIG_REQ:
        PRINT(("IMU_CONFIG_REQ\n"));
        imu_handle_config_req((const IMU_CONFIG_REQ_T *) message);
        break;

    case IMU_CONFIG_UPDATE_REQ:
        PRINT(("IMU_CONFIG_UPDATE_REQ\n"));
        imu_handle_config_update_req((const IMU_CONFIG_UPDATE_REQ_T *) message);
        break;

    case IMU_READ_FIFO_REQ:
        PRINT(("IMU_READ_FIFO_REQ\n"));
        imu_handle_read_fifo_header_mode_data_req((const IMU_READ_FIFO_REQ_T *) message);
        break;

    case PPG_CONFIG_REQ:
        PRINT(("PPG_CONFIG_REQ\n"));
        ppg_handle_config_req((const PPG_CONFIG_REQ_T *) message);
        break;

    case PPG_READ_FIFO_REQ:
        PRINT(("PPG_READ_FIFO_REQ\n"));
        ppg_handle_read_fifo_header_mode_data_req((const PPG_READ_FIFO_REQ_T *) message);
        break;

    case PPG_SUSPEND_REQ:
        PRINT(("PPG_SUSPEND_REQ\n"));
        ppg_handle_suspend_req((const PPG_SUSPEND_REQ_T *) message);
        break;

    case PROX_CONFIG_REQ:
        PRINT(("PROX_CONFIG_REQ\n"));
        prox_handle_config_req((const PROX_CONFIG_REQ_T *) message);
        break;

    case PROX_READ_FIFO_REQ:
        PRINT(("PROX_READ_FIFO_REQ\n"));
        prox_handle_read_fifo_data_req((const PROX_READ_FIFO_REQ_T *) message);
        break;

    case IMU_INTERRUPT_IND:
        PRINT(("IMU_INTERRUPT_IND\n"));
        imu_interrupt_handler();
        break;

    default:
        handle_unexpected(id);
        break;

    }

}
