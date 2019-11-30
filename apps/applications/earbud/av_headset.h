/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset.h
\brief      Main application header file

\mainpage   Earbuds application

Legal information
=================
<CENTER><b>For additional information or to submit technical questions, go to: https://createpoint.qti.qualcomm.com</b></CENTER>

<CENTER><b>Confidential and Proprietary – Qualcomm Technologies International, Ltd.</b></CENTER>

<b>NO PUBLIC DISCLOSURE PERMITTED:</b> Please report postings of this document on public servers or websites to: DocCtrlAgent@qualcomm.com.  

<b>Restricted Distribution:</b> Not to be distributed to anyone who is not an employee of either Qualcomm Technologies International, Ltd. or its affiliated companies without the express approval of Qualcomm Configuration Management.

Not to be used, copied, reproduced, or modified in whole or in part, nor its contents revealed in any manner to others without the express written permission of Qualcomm Technologies International, Ltd. 

All Qualcomm products mentioned herein are products of Qualcomm Technologies, Inc. and/or its subsidiaries.

Qualcomm is a trademark of Qualcomm Incorporated, registered in the United States and other countries. Other product and brand names may be trademarks or registered trademarks of their respective owners.
This technical data may be subject to U.S. and international export, re-export, or transfer (“export”) laws. Diversion contrary to U.S. and international law is strictly prohibited.

Qualcomm Technologies International, Ltd. (formerly known as Cambridge Silicon Radio Limited) is a company registered in England and Wales with a registered office at: Churchill House, Cambridge Business Park, Cowley Road, Cambridge, CB4 0WZ, United Kingdom. Registered Number: 3665875 | VAT number: GB787433096.

<CENTER><b>© 2018 Qualcomm Technologies, Inc. and/or its subsidiaries. All rights reserved.</b></CENTER>

Overview
========
This is a working sample application for earbuds supporting
Qualcomm TrueWireless&trade; Stereo (TWS) and Qualcomm TrueWireless&trade; Stereo Plus (TWS+).

The application is usable as delivered, but expectation is that
customers will adapt the application. Options for doing this
include
\li Build defines to control features included
\li Changes to the configuration (\ref av_headset_config.h)
\li Software changes

			@startuml

			class init << (T,Green) >> #paleGreen {  
				Initialise the application
				....
				Initialises all other modules
				at startup
			}
			class led << (T,lightblue) >> {     
				Manages the LEDs
				}
			class hfp << (T,lightblue) >> #Orange { 
				Manages HFP connections 
				}
			class ui << (T,lightblue) >> #Orange { 
				Manages the UI 
				}
			class sm << (T,lightblue) >> #lightBlue { 
				Main application state machine 
				}
			class link_policy << (T,lightblue) >> {
				Manages the device link policy settings
				}
			class av << (T,lightblue) >> #Orange {
				Manages A2DP and AVRCP connections
				}
			class charger << (T,lightblue) >> { 
				Manages the charger
				}
			class battery << (T,lightblue) >> { 
				Manages the battery status
				}
			class temperature << (T,lightblue) >> {
				Temperature measurement
				}
			class power << (T,lightblue) >> #Orange {
				Manages power, shutdown, sleep
				}
			class pairing << (T,lightblue) >> #Orange { 
				Manages peer and handset pairing 
				}
			class scanning << (T,lightblue) >> { 
				Manages the device scanning state 
				}
			class device << (T,lightblue) >> { 
				Paired devices database
				}
			class con_manager << (T,lightblue) >> { 
				Manages the device connection state machine 
				}
			class peer_sig << (T,lightblue) >> { 
				Manages peer signalling messages
				}
			class handset_sig << (T,lightblue) >> { 
				Manages the handset signalling messages 
				}
			class phy_state << (T,lightblue) >> #Orange { 
				Manages physcial state of the Earbud
				}
			class kymera << (T,lightblue) >> { 
				Manages the audio subsystem
				}
			class upgrade << (T,red) >> #lightGrey {     
				Manages the device upgrade 
				}
			class proximity << (T,lightblue) >> { 
				Manages the proximity sensor
				}
			class accelerometer << (T,lightblue) >> { 
				Manages the accelerometer
				}
			class rules << (.,red) >> #yellow { 
				Application rules engine 
				}
			class chains << (.,red) >> #pink { 
				User defined audio chains 
				}
			class buttons << (.,red) >> #pink { 
				User defined button inputs 
				}
			class peer_sync << (T,lightblue) >> { 
				Peer Earbud state synchronisation
				}
			class scofwd << (T,lightblue) >> { 
				SCO and MIC forwarding
				}

			class A << (T,lightblue) >> {
			}

			class B << (T,lightblue) >> {
			}

			init -[hidden]right->upgrade
			upgrade -[hidden]right->device
                        device -[hidden]right-> A
                        A -[hidden]d-> charger

                        A -right-> B : Function Call
                        A -right[#blue]-> B : Registered Event
                        A <-right[#green]-> B : Function and Registered Event

                        rules-d[#blue]->sm : Actions
                        sm -u-> rules : Events
                        chains-l->kymera
                        buttons-l->ui

                        ' Physical Inputs
                        temperature -d[#blue]-> power : temperature status
                        temperature -r[#blue]-> charger : temperature status
                        charger -d[#blue]-> phy_state : charger status (in/out case)
                        'charger -[hidden]r->accelerometer
                        battery -[hidden]r->temperature
                        charger -d[#blue]-> power : charger status
                        battery -d[#blue]-> power : battery status
                        proximity -d[#blue]-> phy_state : in/out ear
                        proximity -[hidden]r->accelerometer
                        accelerometer -d[#blue]-> phy_state :  motion status
                        power -d[#blue]-> sm : power events
                        phy_state -d[#blue]-> sm : phy state
                        phy_state -[#blue]-> handset_sig : phy state

                        'UI control
                        sm -r-> ui : ui indications
                        ui -u-> led : enable/disable
                        ui -d-> kymera : tones

                        'Registered con_manager tasks
                        con_manager -[#blue]> sm : con status
                        con_manager -r[hidden]-> link_policy
                        av <-[#green]-> con_manager : con/dis/status
                        hfp <-[#green]-> con_manager : con/dis/status
                        peer_sig -r-> con_manager : con/dis

                        sm -l-> pairing : Pair handset and earbud
                        pairing -d-> scanning : enable/disable
                        sm -d-> scanning : enable/disable

                        sm <-d[#green]-> hfp : con/dis/status
                        hfp -d[#blue]-> scofwd : hfp status
                        scofwd -r-> kymera : voice call
                        hfp <-u[#green]-> handset_sig : con/dis/sig
                        hfp -d-> link_policy

                        sm <-d[#green]-> av : con/dis/status
                        av <-u[#green]-> handset_sig : con/dis/sig
                        av -d-> link_policy
                        av -r-> kymera : stereo music

                        peer_sig -d[#blue]-> sm
                        peer_sig -u-> scanning : enable/disable

                        peer_sync -r[hidden]-> pairing
                        peer_sync -r[#blue]->sm : sync status
                        peer_sync <-d[#green]-> peer_sig : peer sync sig

                        sm -d-> scofwd : con/dis
                        scofwd -l-> peer_sig : scofwd sig
                        scofwd -u-> link_policy

                        @enduml		

*/

#ifndef _AV_HEADSET_H_
#define _AV_HEADSET_H_

#include <connection.h>
#include <a2dp.h>
#include <avrcp.h>
#include <hfp.h>
#include <message.h>
#include <stream.h>
#include <bdaddr.h>
#include <stdio.h>
#include <source.h>
#include <stdlib.h>
#include <pio.h>

#include "av_headset_message.h"
#include "av_headset_config.h"
#include "av_headset_led.h"
#include "av_headset_power.h"
#include "av_headset_av.h"
#include "av_headset_a2dp.h"
#include "av_headset_avrcp.h"
#include "av_headset_hfp.h"
#include "av_headset_battery.h"
#include "av_headset_charger.h"
#include "av_headset_ui.h"
#include "av_headset_link_policy.h"
#include "av_headset_kymera.h"
#include "av_headset_device.h"
#include "av_headset_pairing.h"
#include "av_headset_scan_manager.h"
#include "av_headset_con_manager.h"
#include "av_headset_init.h"
#include "av_headset_peer_signalling.h"
#include "av_headset_sm.h"
#include "av_headset_handset_signalling.h"
#include "av_headset_phy_state.h"
#include "av_headset_kymera.h"
#include "av_headset_test.h"
#include "av_headset_conn_rules.h"
#include "av_headset_upgrade.h"
#include "av_headset_gaia.h"
#include "av_headset_accelerometer.h"
#include "av_headset_proximity.h"
#include "av_headset_scofwd.h"
#include "av_headset_peer_sync.h"
#include "av_headset_anc_tuning.h"
#include "av_headset_adv_manager.h"
#include "av_headset_gatt.h"

/*!@{ \name Defines used to the class of device */
#define AUDIO_MAJOR_SERV_CLASS      0x200000
#define RENDER_MAJOR_SERV_CLASS     0x040000
#define AV_MAJOR_DEVICE_CLASS       0x000400
#define HEADSET_MINOR_DEVICE_CLASS  0x000004
/*!@}*/

/*! Minor version number used for the GAIA interface */
#define GAIA_API_MINOR_VERSION 5

/*! Internal messages */
enum
{
                                            /*!  Connecting reminder timeout */
    APP_INTERNAL_UI_CONNECTING_TIMEOUT= INTERNAL_MESSAGE_BASE,     
    APP_INTERNAL_UI_INQUIRY_TIMEOUT,        /*!< Inquiry reminder timeout */
    APP_INTERNAL_PAIRING_INIT
};

/* Useful macro to ensure linker doesn't discard a function that's not
 * referenced in the code but maybe called from pydbg.  Example:
 *
 * keep void *test_func(void)
 */
#ifdef GC_SECTIONS
#define keep _Pragma("codesection KEEP_PM")
#else
#define keep
#endif

/*! Trusted device index used for AV devices */
#define APP_AV_TDI  (1)

/*! Trusted device index used for HFP devices */
#define APP_HFP_TDI (2)

/*! User PSKEY base for attributes */
#define APP_ATTRIBUTE_PS_BASE   (16)

/*! User PSKEY to store AV configuration */
#define PS_AV_CONFIG            (0)

/*! User PSKEY to store HFP configuration */
#define PS_HFP_CONFIG           (1)

/*! Application task data */
typedef struct appTaskData
{
    TaskData            task;                   /*!< Application task */
    TaskData            systask;                /*!< Handler for system messages */
    Task                codec_task;             /*!< CODEC task */

    initData            init;                   /*!< Structure used while initialising */
    ledTaskData         led;                    /*!< LED data structure */
#ifdef INCLUDE_HFP
    hfpTaskData         hfp;                    /*!< HFP data structure */
#endif
    uiTaskData          ui;                     /*!< UI data structure */
    lpTaskData          lp;                     /*!< Link Policy Manager data structure */
#ifdef INCLUDE_AV
    avTaskData          av;                     /*!< AV data structure */
#endif
#ifdef INCLUDE_CHARGER
    chargerTaskData     charger;                /*!< Charger data structure */
#endif
    batteryTaskData     battery;                /*!< Battery data structure */
    gattTaskData        gatt;                   /*!< App GATT component task */
    pairingTaskData     pairing;                /*!< App pairing task */
    scanManagerTaskData scanning;               /*!< Scan Manager data structure. */
    deviceTaskData      device;                 /*!< App device management task */
    conManagerTaskData  con_manager;            /*!< Connection manager task data */
    peerSigTaskData     peer_sig;               /*!< Peer earbud signalling */
    handsetSigTaskData  handset_sig;            /*!< Handset signalling */
    phyStateTaskData    phy_state;              /*!< Physical state of the Earbud. */
    kymeraTaskData      kymera;                 /*!< State data for the DSP configuration */
    smTaskData          sm;                     /*!< Application state machine. */
    connRulesTaskData   conn_rules;             /*!< Connection rules. */
#ifdef INCLUDE_DFU
    gaiaTaskData        gaia;                   /*!< Task information for GAIA support */
    upgradeTaskData     upgrade;                /*!< Task information for UPGRADE support */
#endif
#ifdef INCLUDE_PROXIMITY
    proximityTaskData   proximity;              /*!< Task information for proximity sensor */
#endif
#ifdef INCLUDE_ACCELEROMETER
    accelTaskData       accelerometer;          /*!< Task information for accelerometer */
#endif
    powerTaskData       power;                  /*!< Information for power_control */
    scoFwdTaskData      scoFwd;                 /*!< Task information for SCO forwarding */
#ifdef INCLUDE_TEMPERATURE
    temperatureTaskData temperature;            /*!< Task information for temperature */
#endif
    peerSyncTaskData    peer_sync;              /*!< Task information for peer synchronisation. */
    ancTuningTaskData   anc_tuning;             /*!< Task information for ANC tuning. */
    advManagerTaskData  adv_manager;            /*!< Task information for the advertising manager */
} appTaskData;

/*! The global application data structure. 

\note Do not access directly */
extern appTaskData globalApp;

/*! Get pointer to application data structure */    
#define appGetApp()         (&globalApp)

/*! Get pointer to application task */    
#define appGetAppTask()     (&globalApp.task)

/*! Get pointer to the system message task */    
#define appGetSysTask()     (&globalApp.systask)

/*! Get pointer to init data structure */
#define appGetInit()        (&globalApp.init)

/*! Get pointer to UI data structure */    
#define appGetUi()          (&globalApp.ui)

/*! Get pointer to UI task */    
#define appGetUiTask()      (&globalApp.ui.task)

/*! Get pointer to LED controller data structure */    
#define appGetLed()         (&globalApp.led)

/*! Get pointer to the proximity sensor data structure */
#define appGetProximity()   (&globalApp.proximity)

/*! Get pointer to the acceleromter data structure */
#define appGetAccelerometer() (&globalApp.accelerometer)

/*! Get pointer to Link Policy Manager data structure */    
#define appGetLp()          (&globalApp.lp)

/*! Get pointer to CODEC task */    
#define appGetCodecTask()   (globalApp.codec_task)

/*! Get pointer to HFP data structure */    
#define appGetHfp()         (&globalApp.hfp)

/*! Get pointer to HFP task */    
#define appGetHfpTask()     (&globalApp.hfp.task)

/*! Get pointer to the GATT modules task data */
#define appGetGatt()                        (&globalApp.gatt)

/*! Get pointer to the GATT modules task */
#define appGetGattTask()                    (&globalApp.gatt.gatt_task)
/*! Get pointer to the GATT modules battery task */
#define appGetGattBatteryTask()             (&globalApp.gatt.gatt_battery_task)
/*! Get pointer to the GAP server task */
#define appGetGattGapTask()                 (&globalApp.gatt.gatt_task)

/*! Get pointer to the GATT server info of the GATT module */
#define appGetGattInstance(inst)            (&globalApp.gatt.instance[inst])
/*! Get pointer to instance of the GATT server info */
#define appGetGattServerInst(inst)          (&appGetGattInstance(inst)->gatt_server)
/*! Get pointer to instance of the GAP server info  */
#define appGetGattGapServerInst(inst)       (&appGetGattInstance(inst)->gap_server)

#if defined(INCLUDE_GATT_BATTERY_SERVER)
/*! Get pointer to instance of the Battery server for Left earbud battery */
#define appGetGattBatteryServerLeft(inst)   (&appGetGattInstance(inst)->battery_server_left)
/*! Get pointer to instance of the Battery server for the right earbud battery */
#define appGetGattBatteryServerRight(inst)  (&appGetGattInstance(inst)->battery_server_right)
#endif

/*! Get pointer to Pairing data structure */
#define appGetPairing()             (&globalApp.pairing)

/*! Get pointer to scan manager data structure */
#define appGetScanning()            (&globalApp.scanning)

/*! Get pointer to Device Management data structure */
#define appGetDevice()              (&globalApp.device)

/*! Get pointer to Connection Management data structure */
#define appGetConManager()          (&globalApp.con_manager)

/*! Get pointer to the peer signalling modules data structure */
#define appGetPeerSig()             (&globalApp.peer_sig)

/*! Get pointer to handset signalling data structure */
#define appGetHandsetSig()          (&globalApp.handset_sig)

/*! Get pointer to physical state data structure */
#define appGetPhyState()            (&globalApp.phy_state)

/*! Get pointer to Kymera structure */
#define appGetKymera()              (&globalApp.kymera)

/*! Get pointer to application state machine task data struture. */
#define appGetSm()                  (&globalApp.sm)
/*! Get pointer to the state machine modules task. */
#define appGetSmTask()              (&globalApp.sm.task)

/*! Get pointer to the connection rules task data structure. */
#define appGetConnRules()           (&globalApp.conn_rules)

#ifdef INCLUDE_DFU
/*! Get the info for the applications Gaia support */
#define appGetGaia()                (&globalApp.gaia)

/*! Get the Task info for the applications Gaia task */
#define appGetGaiaTask()            (&globalApp.gaia.gaia_task)

/*! Get the info for the applications upgrade support */
#define appGetUpgrade()             (&globalApp.upgrade)

/*! Get the Task info for the applications Upgrade task */
#define appGetUpgradeTask()         (&globalApp.upgrade.upgrade_task)
#endif /* INCLUDE_DFU */

#ifdef INCLUDE_AV
/*! Get pointer to AV data structure */    
#define appGetAv()                  (&globalApp.av)
#endif

#ifdef INCLUDE_CHARGER
/*! Get pointer to charger data structure */    
#define appGetCharger()             (&globalApp.charger)
#endif

/*! Get pointer to data for power control */
#define appGetPower()        (&globalApp.power)

/*! Get pointer to data for power control */
#define appGetPowerTask()           (&globalApp.power.task)

/*! Get pointer to SCO forward data */
#define appGetScoFwd()              (&globalApp.scoFwd)

/*! Get pointer to the SCO forward task */
#define appGetScoFwdTask()          (&globalApp.scoFwd.task)

#ifdef INCLUDE_TEMPERATURE
/*! Get pointer to temperature data */
#define appGetTemperature()         (&globalApp.temperature)
#endif


/*! Get pointer to battery data structure */
#define appGetBattery()     (&globalApp.battery)

/*! Get pointer to the data for the advertising manager */
#define appGetAdvManager()      (&globalApp.adv_manager)

/*! Get the advertising manager task */
#define appGetAdvManagerTask()  (&globalApp.adv_manager.task)

/*! Get pointer to the peer sync data structure */
#define appGetPeerSync()    (&globalApp.peer_sync)

/*! Get pointer to the ANC tuning data structure */
#define appGetAncTuning()    (&globalApp.anc_tuning)


#if defined(INCLUDE_AV) && (INCLUDE_HFP)
#define appHandleUnexpected(id) \
    DEBUG_LOGF("appHandleUnexpected, id = %x, sm = %d, hfp = %d, av = %d", (id), appGetApp()->sm.state, appGetApp()->hfp.state, appGetApp()->av.state)
#elif defined(INCLUDE_AV)
#define appHandleUnexpected(id) \
    DEBUG_LOGF("appHandleUnexpected, id = %x, sm = %d, av = %d", (id),  appGetApp()->sm.state, appGetApp()->av.state)
#elif defined(INCLUDE_HFP)
#define appHandleUnexpected(id) \
    DEBUG_LOGF("appHandleUnexpected, id = %x, sm = %d, hfp = %d", (id),  appGetApp()->sm.state, appGetApp()->hfp.state)
#else
#define appHandleUnexpected(id) ((void)0)
#endif

/*! Message handler function for unexpected system messages */
#define appHandleSysUnexpected(id) \
    DEBUG_LOGF("appHandleSysUnexpected, id = 0x%x (%d)", (id), (id))

#endif /* _AV_HEADSET_H_ */
