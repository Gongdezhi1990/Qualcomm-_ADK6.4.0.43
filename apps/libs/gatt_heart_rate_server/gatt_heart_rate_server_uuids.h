/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef __GATT_HR_SERVER_UUIDS_H__
#define __GATT_HR_SERVER_UUIDS_H__

/* UUIDs for Heart Rate Service and Characteristics*/

#define UUID_HEART_RATE_SERVICE                       0x180D

#define UUID_HEART_RATE_MEASUREMENT                                       0x2A37

#define UUID_HEART_RATE_BODY_SENSOR_LOCATION                               0x2A38

/* Heart Rate Sensor Location characteristic values */

/* Body Sensor Location unknown */
#define BODY_SENSOR_LOCATION_OTHER                    0x00
/* Body Sensor Location chest */
#define BODY_SENSOR_LOCATION_CHEST                    0x01
/* Body Sensor Location wrist */
#define BODY_SENSOR_LOCATION_WRIST                    0x02
/* Body Sensor Location finger */
#define BODY_SENSOR_LOCATION_FINGER                   0x03
/* Body Sensor Location hand */
#define BODY_SENSOR_LOCATION_HAND                     0x04
/* Body Sensor Location ear lobe */
#define BODY_SENSOR_LOCATION_EAR_LOBE                 0x05
/* Body Sensor Location foot */
#define BODY_SENSOR_LOCATION_FOOT                     0x06

#define BODY_SENSOR_LOCATION_VALUE  BODY_SENSOR_LOCATION_EAR_LOBE

#endif /* __GATT_HR_SERVER_UUIDS_H__ */

