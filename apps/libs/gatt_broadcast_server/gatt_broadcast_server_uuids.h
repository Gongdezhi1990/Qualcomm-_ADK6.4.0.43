/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */
/*! The GATT Broadcast Service, as supplied, uses a 16-bit UUID purchased by Qualcomm for the Service UUID. 
      This 16-bit UUID may be used by our customers on condition that the service and its implementation in the 
      gatt_broadcast_client and gatt_broadcast_server libraries and sink application shall _not_ be modified by the 
      customer. Alternatively, if the customer wishes to customize the service, a new service UUID may be purchased 
      from the Bluetooth SIG and shall be used in place of our 16-bit UUID; 
      see "https://www.bluetooth.com/specifications/assigned-numbers/16-bit-uuids-for-members" */
#ifndef __GATT_BROADCAST_SERVER_UUIDS_H__
#define __GATT_BROADCAST_SERVER_UUIDS_H__

/* NOTE: The below 2 services UUIDs are mutally exclusive. So the server cannot define 
              both the service */
/* Reserved for ADK broadcaster */
#define UUID_BROADCAST_SERVICE                  0xFE8F
/* Reserved for Qualcomm mobile broadcaster */
#define UUID_BROADCAST_SERVICE_2              0xFE06


#define UUID_BROADCAST_BASE_A                   0x5B00A5A5UL
#define UUID_BROADCAST_BASE_B                   0x9B230002UL
#define UUID_BROADCAST_BASE_C                   0xD10211E1UL

#define UUID_BROADCAST_VERSION                  0x0000BCA4D10211E19B2300025B00A5A5
#define UUID_BROADCAST_VERSION_U32            0x0000BCA4UL

#define UUID_BROADCAST_ADDRESS                  0x0000BCA7D10211E19B2300025B00A5A5
#define UUID_BROADCAST_ADDRESS_U32            0x0000BCA7UL

#define UUID_BROADCAST_STATUS                   0x0000BCA5D10211E19B2300025B00A5A5
#define UUID_BROADCAST_STATUS_U32             0x0000BCA5UL

#define UUID_BROADCAST_SECKEY                   0x0000BCACD10211E19B2300025B00A5A5
#define UUID_BROADCAST_SECKEY_U32             0x0000BCACUL

#define UUID_BROADCAST_STREAM_SERVICE_RECORDS           0x0000BCA6D10211E19B2300025B00A5A5
#define UUID_BROADCAST_STREAM_SERVICE_RECORDS_U32     0x0000BCA6UL

#define UUID_BROADCAST_IDENTIFIER               0x0000BCA8D10211E19B2300025B00A5A5
#define UUID_BROADCAST_IDENTIFIER_U32         0x0000BCA8UL

#endif /* __GATT_BROADCAST_SERVER_UUIDS_H__ */
