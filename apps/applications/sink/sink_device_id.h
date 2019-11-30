/*
Copyright (c) 2011 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file    sink_device_id.h
@ingroup sink_app
@brief   Interface to the sink Device Id profile functions. 
*/

#ifndef _SINK_DEVICE_ID_H_
#define _SINK_DEVICE_ID_H_

#include "sink_inquiry.h"

#define VENDOR_ID_SOURCE_BT_SIG    0x0001   /* Indicates the Vendor Id has been assigned by the Bluetooth Special Interest Group */
#define VENDOR_ID_SOURCE_USB_IF    0x0002   /* Indicates the Vendor Id has been assigned by the USB Implementers Forum           */
#define VENDOR_ID_SOURCE_UNDEFINED 0xFFFF


/* If DEVICE_ID_CONST is defined, the registered Device Id service record will use the following defines */
#define DEVICE_ID_VENDOR_ID_SOURCE VENDOR_ID_SOURCE_UNDEFINED
#define DEVICE_ID_VENDOR_ID        0xFFFF   /* As assigned by the appropriate source */
#define DEVICE_ID_PRODUCT_ID       0xFFFF   /* Customer specific                     */
#define DEVICE_ID_BCD_VERSION      0xFFFF   /* Where 0xABCD = vAB.C.D                */

#define SINK_DEVICE_ID_STRICT_SIZE     (sizeof(uint16) * 4)
#define SINK_DEVICE_ID_SW_VERSION_SIZE 4

typedef struct
{
    uint16 vendor_id_source;
    uint16 vendor_id;
    uint16 product_id;
    uint16 bcd_version;
    uint16 sw_version[SINK_DEVICE_ID_SW_VERSION_SIZE];   /* Original software version number, which is not part of the Device ID spec */
} sink_device_id;

bool RequestRemoteDeviceId (Task task, const bdaddr *bd_addr);
remote_device CheckRemoteDeviceId (const uint8 *attr_data, uint8 attr_data_size);
void RegisterDeviceIdServiceRecord( void );
uint16 GetDeviceIdEirDataSize( void );
uint16 WriteDeviceIdEirData( uint8 *p );
bool CheckEirDeviceIdData (uint16 size_eir_data, const uint8 *eir_data);
void ValidatePeerUseDeviceIdFeature(void);
void GetDeviceIdFullVersion(uint16 *buffer);
/****************************************************************************
NAME	
	SinkDeviceIdRetrieveConfiguration
    
DESCRIPTION
    Retrieve the Device ID configuration

PARAMS
    void* data
    
RETURNS
	uint16
*/
uint16 SinkDeviceIDRetrieveConfiguration(void* data);

#endif
