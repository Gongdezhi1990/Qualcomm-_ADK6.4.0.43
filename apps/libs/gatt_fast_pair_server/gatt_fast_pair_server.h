/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */ /*  */ 
/*!
@file    gatt_fast_pair_server.h
@brief   Header file for the GATT Fast Pair Server library.

        This file provides documentation for the GATT Fast Pair Server library
        API (library name: gatt_fast_pair_server).
*/

#ifndef GATT_FAST_PAIR_SERVER_H_
#define GATT_FAST_PAIR_SERVER_H_

#include <csrtypes.h>
#include <message.h>
#include <library.h>

#include "gatt_manager.h"

#define FAST_PAIR_VALUE_SIZE (16)

/*! @brief The Fast Pair service internal structure for the server role.

    This structure is visible to the application as it is responsible for 
    managing resource to pass to the Fast Pair library.
    The elements of this structure are only modified by the Fast Pair library.
    The application SHOULD NOT modify the values at any time as it could lead 
    to undefined behaviour.

 */
typedef struct __GFPS
{
    TaskData    lib_task;
    Task        app_task;
    /* Server library context data to be determined. */
} GFPS;


/*! @brief Key-based Pairing characteristic value data, when written by a
    client. This characteristic can ONLY be written.
 */
typedef struct __GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND_T
{
    const GFPS  *fast_pair_server;
    uint16      cid;
    uint16      size_value;
    uint8       value[1];
} GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND_T;


/*! @brief Key-based Pairing characteristic Client Configuration attribute
   read indication.
 */
typedef struct __GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND_T
{
    const GFPS  *fast_pair_server;
    uint16      cid;
} GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND_T;


/*! @brief Key-based Pairing characteristic Client Configuration attribute
    value, when configured by the client.
 */
typedef struct __GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND_T
{
    const GFPS  *fast_pair_server;
    uint16      cid;
    uint16      config_value;
} GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND_T;


/*! @brief Passkey characteristic value data, when written by a
    client.
 */
typedef struct __GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND_T
{
    const GFPS  *fast_pair_server;
    uint16      cid;
    uint8       value[FAST_PAIR_VALUE_SIZE];
} GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND_T;


/*! @brief Passkey characteristic Client Configuration attribute read 
    indication.
 */
typedef GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND_T 
            GATT_FAST_PAIR_SERVER_READ_PASSKEY_CONFIG_IND_T;


/*! @brief Passkey characteristic Client Configuration attribute
    value, when configured by the client.
 */
typedef GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND_T 
            GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_CONFIG_IND_T;


/*! @brief Account Key characteristic value data, when written by a
    client.
 */
typedef GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND_T 
            GATT_FAST_PAIR_SERVER_WRITE_ACCOUNT_KEY_IND_T;
 

/*! @brief Key-based Pairing notification confirmation
 */
typedef struct __GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM
{
    const GFPS      *fast_pair_server;
    gatt_status_t   status;
    uint16          cid;
} GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM_T;


/*! @Brief Passkey notification confirmation.
 */
typedef struct GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM_T
    GATT_FAST_PAIR_SERVER_PASSKEY_NOTIFICATION_CFM_T;


/*! @brief Enumeration of messages an application task can receive from the
    Fast Pair Server library
 */
typedef enum {
    GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND = GATT_FAST_PAIR_SERVER_MESSAGE_BASE,
    GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND,
    GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND,
    GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND,
    GATT_FAST_PAIR_SERVER_READ_PASSKEY_CONFIG_IND,
    GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_CONFIG_IND,
    GATT_FAST_PAIR_SERVER_WRITE_ACCOUNT_KEY_IND,
    GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM,
    GATT_FAST_PAIR_SERVER_PASSKEY_NOTIFICATION_CFM,
    GATT_FAST_PAIR_SERVER_MESSAGE_TOP
} gatt_fps_server_message_id_t;


/*! @brief Initialise the Fast Pair Service Library in the Server role.
 
    @param fast_pair_server Pointer to the server data.
    @param app_task The Task that will receive the messages from this library.
    @param start_handle First handle in the GATT database for this service.
    @param end_handle Last handle in the GATT database for this service.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerInit(
        GFPS    *fast_pair_server,
        Task    app_task,
        uint16  start_handle,
        uint16  end_handle
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND_T.
 
    This value can ONLY be written, therefore the response is only to indicate
    the result of the GATT Write.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param result Result of the Access Indication.
    
    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerWriteKeybasedPairingResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND_T

    Response to the Key-based Pairing Client Configuration read indication.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param client_config The config value for this client connection.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerReadKeybasedPairingConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      client_config
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND_T
 
    Response to the Key-based Pairing Client Configuration write indication.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param result Result of the Client Configuration Write.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerWriteKeybasedPairingConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        );


/*! @brief Notify the Key-based Pairing characteristic value to the client.
 
    The value notified is always 16-octets. Notification is only to one connected
    client at a time.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param value Characteristic value to notify, always 16-octets.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerKeybasedPairingNotification(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint8       value[FAST_PAIR_VALUE_SIZE]
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND_T.
 
    This value can ONLY be written, therefore the response is only to indicate
    the result of the GATT Write.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param result Result of the Access Indication.
    
    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerWritePasskeyResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_READ_PASSKEY_CONFIG_IND_T

    Response to the Passkey Client Configuration read indication.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param client_config The config value for this client connection.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerReadPasskeyConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      client_config
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND_T
 
    Response to the Passkey Client Configuration write indication.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param result Result of the Client Configuration Write.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerWritePasskeyConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        );


/*! @brief Notify the Passkey characteristic value to the client.
 
    The value notified is always 16-octets. Notification is only to one connected
    client at a time.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param value Characteristic value to notify, always 16-octets.

    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerPasskeyNotification(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint8       value[FAST_PAIR_VALUE_SIZE]
        );


/*! @brief Response to GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_ACCOUNT_KEY_IND_T.
 
    This value can ONLY be written, therefore the response is only to indicate
    the result of the GATT Write.

    @param fast_pair_server Pointer to the server data.
    @param cid Connection identifier of the client.
    @param result Result of the Access Indication.
    
    @return TRUE if successful, otherwise FALSE.
*/
bool GattFastPairServerWriteAccountKeyResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        );


#endif /* GATT_FAST_PAIR_SERVER_H_ */
