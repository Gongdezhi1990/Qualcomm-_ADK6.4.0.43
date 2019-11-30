/****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    imu_ppg_profile_handler.h

DESCRIPTION
    Header file for the profile handler for the IMU and PPG device
    library.

NOTES

****************************************************************************/
#ifndef IMU_PPG_PROFILE_HANDLER_H
#define IMU_PPG_PROFILE_HANDLER_H
/**
 * @brief Message handler for IMUPPG task
 * @param task Task ID
 * @param id Message ID
 * @param message Message content
 */
void imu_ppg_profile_handler(Task task, MessageId id, Message message);
#endif /* IMU_PPG_HANDLER_H */
