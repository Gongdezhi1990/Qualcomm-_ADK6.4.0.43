/*****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    imu_ppg_init.c

DESCRIPTION
    This file contains the initialisation code for the IMU and PPG device
    library.

NOTES

*/

/*****************************************************************************
    Header files
*/
#include "imu_ppg_private.h"
#include "sport_health_driver.h"
#include "imu_ppg_message_handler.h"
#include "imu_ppg_profile_handler.h"

#include <panic.h>
#include <print.h>
#include <stdlib.h>
#include "macros.h"

/** The one, and only, IMUPPG instance */
IMUPPG *pImuPpg = 0;  /** Pointer located in global space */
/** The one, and only, p_fifo_data instance */
uint8 *p_fifo_data;  /** Pointer located in global space */
/** Static allocation for sensor data working buffer */
uint32 sensor_working_buff[FIFO_DEPTH_WORDS] = 0;


/**
 * @brief Function to send IMUPPG_INIT_CFM based on successful
 *        initilisation of sensors
 * @param imu_status Status of IMU sensor inititalisation TRUE/FALSE
 * @param ppg_status Status of PPG sensor inititalisation TRUE/FALSE
 */
static void imu_ppg_send_init_cfm_to_client(imu_status_code_t imu_status, ppg_status_code_t ppg_status)
{
    MAKE_IMUPPG_MESSAGE(IMUPPG_INIT_CFM);
    message->imu_status = imu_status;
    message->ppg_status = ppg_status;
    message->task       = (Task)&pImuPpg->task;
    MessageSend(pImuPpg->app_task, IMUPPG_INIT_CFM, message);

    /** If the initialisation failed, free the allocated task */
    if ((imu_status) && (ppg_status))
    {
        free(pImuPpg);
        pImuPpg = 0;
    }
}
/**
 * @brief Initialises the IMU and PPG sensor
 * @param client_task Task Id for app task
 * @param sensor_configuration the configuration data for sensors
 * @return IMUPGG task ID
 */
Task SportHealthDriverImuPpgInitTask(Task client_task, sensor_config_data_t* sensor_configuration)
{
    uint8 imu_status = imu_failure;
    uint8 ppg_status = ppg_failure;
    if ( !pImuPpg )
    {
        pImuPpg = PanicUnlessNew(IMUPPG);
        memset(pImuPpg, 0, sizeof(IMUPPG));

        /** Set the handler function */
        pImuPpg->task.handler = imu_ppg_profile_handler;
        /** Set up the app task */
        pImuPpg->app_task = client_task;
        /* Initialize the device driver data structure */
        imu_ppg_data_init();
        /* Initialize the IMU sensor */
        imu_status = imu_initialize_sensor(sensor_configuration);
        /* Initialize the PPG sensor */
        ppg_status = ppg_initialize_sensor(sensor_configuration);
        if ((!imu_status) || (!ppg_status))
        {
            /** Assign memory to store the FIFO data */
            p_fifo_data = (uint8 *)sensor_working_buff;
        }
        if(p_fifo_data != NULL)
        {
            imu_ppg_send_init_cfm_to_client(imu_status, ppg_status);
            return ((Task)&pImuPpg->task);
        }
    }
    return 0;
}
/**
 * @brief Register the hub task id to the IMUPPG task
 * @param clientTask [in] Task Id for the hub task
 */
void SportHealthDriverImuPpgHubCfg(Task client_task)
{
    /** Set up the app task */
    pImuPpg->hub_task = client_task;
}
/**
 * @brief Delete the IMUPPG task as application has triggered
 *        teardown
 */
void imu_ppg_delete(void)
{
    if(pImuPpg)
    {
        free(pImuPpg);
        pImuPpg = NULL;
    }
}
