/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Header file for global message definitions.
*/
#ifndef _AV_HEADSET_MESSAGE_H_
#define _AV_HEADSET_MESSAGE_H_

/*!@{   @name Message ID allocations for each application module
        @brief Each module in the application that sends messages is assigned
               a base message ID. Each module then defines message IDs starting
               from that base ID.
        @note There is no checking that the messages assigned by one module do
              not overrun into the next module's message ID allocation.
*/
#define INTERNAL_MESSAGE_BASE       (0x000)     /*!< Message internal to a module can all re-use 0 on */

#define INIT_MESSAGE_BASE           (0x200)     /*!< Initialisation module messages. Note limited range. See #av_headset_init_messages */
#define AV_MESSAGE_BASE             (0x300)     /*!< AV (A2DP/AVRCP) module messages. See #av_headset_av_messages */
#define APP_HFP_MESSAGE_BASE        (0x400)     /*!< Hands Free Profile (and SCO) messages. See #av_headset_hfp_messages */
#define PAIRING_MESSAGE_BASE        (0x500)     /*!< Pairing module messages. See #av_headset_pairing_messages */
#define AV_GAIA_MESSAGE_BASE        (0x580)     /*!< Gaia module messages. See #av_headet_gaia_messages */
#define AV_UPGRADE_MESSAGE_BASE     (0x600)     /*!< Upgrade module messages. See #av_headet_upgrade_messages */
#define CON_MANAGER_MESSAGE_BASE    (0x700)     /*!< Connection manager module messages. See #av_headset_conn_manager_messages. */
#define PEER_SIG_MESSAGE_BASE       (0x800)     /*!< Peer signalling module messages. See #av_headset_peer_signalling_messages */
#define HANDSET_SIG_MESSAGE_BASE    (0x900)     /*!< Handset signalling module messages. See #av_headset_handset_signalling_messages */
#define PHY_STATE_MESSAGE_BASE      (0xa00)     /*!< Physical state module messages. See #av_headset_phy_state_messages */
#define CONN_RULES_MESSAGE_BASE     (0xb00)     /*!< Connection rules module messages. See #av_headset_conn_rules_messages */
#define EB_BATTERY_MESSAGE_BASE     (0xc00)     /*!< Battery module messages. See #battery_messages */
#define ADV_MANAGER_MESSAGE_BASE    (0xd00)     /*!< Messages sent by advertising manager. See #av_headset_advertising_messages */
#define SFWD_MESSAGE_BASE           (0xe00)     /*!< SCO Forwarding management module. See #av_headset_scofwd_messages */
#define PROXIMITY_MESSAGE_BASE      (0xf00)     /*!< Proximity module messages. Note limited range. See #proximity_messages */
#define ACCELEROMETER_MESSAGE_BASE  (0xf10)     /*!< Accelerometer module messages. Note limited range. See #accelerometer_messages */
#define CHARGER_MESSAGE_BASE        (0xf20)     /*!< Charger module messages. Note limited range. See #chargerMessages */
#define DEVICE_MESSAGE_BASE         (0xf30)     /*!< Device manager messages. Note limited range. See #deviceMessages */
#define KYMERA_MESSAGE_BASE         (0xf40)     /*!< Kymera module messages. Note limited range. See #kymeraMessages */
#define EB_POWER_MESSAGE_BASE       (0xf50)     /*!< Power module messages. Note limited range. See #powerMessages */
#define TEMPERATURE_MESSAGE_BASE    (0xf60)     /*!< Temperature module messages. Note limited range. See #temperatureMessages */
#define PEER_SYNC_MESSAGE_BASE      (0xf70)     /*!< Peer sync module messages. Note limited range. See #peerSyncMessages */
#define APP_GATT_MESSAGE_BASE       (0xf80)     /*!< Messages for the GATT support module. Note limited range. See #av_headet_gatt_messages */
/*@} */
#endif
