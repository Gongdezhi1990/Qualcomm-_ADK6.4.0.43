/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file    local_device.h
@brief   A library to read and store information about the local device for 
         immediate access by applications.
*/


#ifndef LOCAL_DEVICE_
#define LOCAL_DEVICE_

#include <bdaddr.h>
#include <library.h>
#include <message.h>


#define LOCAL_DEVICE_FEATURE_BLE_TX_POWER   (1U << 1)
#define LOCAL_DEVICE_FEATURE_BDADDR         (1U << 2)

typedef enum
{
    LOCAL_DEVICE_INIT_CFM = LOCAL_DEVICE_MESSAGE_BASE,
    LOCAL_DEVICE_MESSAGE_TOP
} local_device_message_id_t;

/*!
    @brief Initialise local device library with all values required

    @param task Task which confirmation message is sent to
    @param reading Mask inidicating which values need to be accessed
*/
void LocalDeviceInit(Task task);

/*!
    @brief Destroy local device library
*/
void LocalDeviceDestroy(void);

/*!
    @brief Get the local device BLE advertising transmit power
*/
int8 LocalDeviceGetAdvertisingTransmitPower(void);

/*!
    @brief Get the local device Bluetooth Address
*/
bdaddr LocalDeviceGetBdAddr(void);

/*!
    @brief Enable features handled by the local device library.
           These are defined as LOCAL_DEVICE_FEATURE_xx in this header file.
           This API can only be used before calling LocalDeviceInit().
    
    @param features Bitmask of features to be enabled.
*/
void LocalDeviceEnableFeatures(uint16 features);

#endif /* LOCAL_DEVICE_ */
