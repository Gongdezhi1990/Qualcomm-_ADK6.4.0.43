/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
*/
#ifndef _SINK_USB_H_
#define _SINK_USB_H_

#include <stdlib.h>
#include <usb_device_class.h>

#if defined(ENABLE_USB)
#include "sink_powermanager.h"
#include "sink_volume.h"
#include "sink_audio_routing.h"
#endif


#define USB_DEVICE_CLASS_AUDIO      (USB_DEVICE_CLASS_TYPE_AUDIO_SPEAKER | USB_DEVICE_CLASS_TYPE_AUDIO_MICROPHONE)

#ifdef ENABLE_USB

typedef enum
{
    usb_audio_suspend_none,
    usb_audio_suspend_local,
    usb_audio_suspend_remote
} usb_audio_suspend_state;

typedef enum
{
    usb_plugin_stereo,
    usb_plugin_mono_nb,
    usb_plugin_mono_wb,
    usb_plugin_mono_uwb,
    usb_plugin_mono_swb
} usb_plugin_type;

/*  Indices into Charge Current array  */
typedef enum
{
    usb_i_disc,                    /* Current when USB disconnected but charger connected */
    usb_i_susp,                    /* Current when suspended */
    usb_i_susp_db,                 /* Current when suspended prior to enumeration with dead battery*/
    usb_i_att,                     /* Current when attached but not enumerated */
#ifdef ENABLE_CHARGER_V2
    usb_i_att_trickle = usb_i_att, /* Current when attached but not enumerated and trickle charging */
#else
    usb_i_att_trickle,             /* Current when attached but not enumerated and trickle charging */
#endif
    usb_i_conn,                    /* Current when connected to host/hub */
#ifdef ENABLE_CHARGER_V2
    usb_i_conn_trickle = usb_i_conn, /* Current when connected to host/hub and trickle charging */
#else
    usb_i_conn_trickle,            /* Current when connected to host/hub and trickle charging */
#endif
    usb_i_chg,                     /* Current when connected to a charging host/hub */
#ifdef ENABLE_CHARGER_V2
    usb_i_dchg,                    /* Current when connected to a Type-C (0.5A) or Type-A dedicated charger */
    usb_i_dchg_1500mA,             /* Current when connected to a Type-C (1.5A) dedicated charger */
    usb_i_dchg_3000mA,             /* Current when connected to a Type-C (3.0A) dedicated charger */
#else
    usb_i_dchg,                    /* Current when connected to a dedicated charger */
#endif
    usb_i_fchg,                    /* Current when connected to charger with floating datalines */
    usb_i_lim                      /* Current when vbus drop is detected */
} usb_charge_mode;

typedef struct
{
    usb_device_class_type   device_class;       /* Class of device */
    unsigned            ready:1;                /* Ready */
    unsigned            enumerated:1;           /* USB enumerated */
    unsigned            suspended:1;            /* Suspended */
    unsigned            mic_active:1;           /* USB Mic in use */
    unsigned            spkr_active:1;          /* USB Speaker in use */
    unsigned            vbus_okay:1;            /* VBUS above threshold */
    unsigned            dead_battery:1;         /* Battery is below dead battery threshold */
    unsigned            deconfigured:1;         /* USB has been deconfigured */
    unsigned            audio_suspend_state:2;  /* Current USB audio suspend state */
    unsigned            vol_event_allowed:1;    /* Volume event recently sent */
    unsigned            enumerated_spkr:1;      /* USB Enumerated as a speaker/headset */
    unsigned            unused:4;
    
    volume_info         volume;                 /* USB Volume settings */
    int16               out_l_vol;              /* Speaker l volume from USB host */
    usb_attached_status attached_status;        /* Type of charger */
#ifdef ENABLE_CHARGER_V2
    unsigned            data_plus;              /* Millivolts on D+ for enumeration */
    unsigned            data_minus;             /* Millivolts on D- for enumeration */
#endif
#ifdef ENABLE_USB_TYPE_C
    usb_type_c_advertisement cc_status;         /* USB Type C, CC Advert */
#endif
} usb_info;

typedef struct
{
    uint16 current;
    bool disable_leds;
    bool power_off;
    bool vsel;
    bool charge;
    power_boost_enable boost;
} usb_charge_current_setting;

#if defined(ENABLE_USB_AUDIO)

typedef struct
{
    uint32                               sample_rate;
    const usb_device_class_audio_config* usb_descriptors_mic_speaker;
    const usb_device_class_audio_config* usb_descriptors_mic;
    const usb_device_class_audio_config* usb_descriptors_speaker;
    usb_plugin_type                      plugin_type;
} usb_plugin_info;

#endif /* def ENABLE_USB_AUDIO */

#endif /* def ENABLE_USB */

/****************************************************************************
NAME 
    usbUpdateChargeCurrent
    
DESCRIPTION
    Set the charge current based on USB state
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbUpdateChargeCurrent(void);
#else
#define usbUpdateChargeCurrent() ((void)(0))
#endif

/****************************************************************************
NAME
    usbUpdateConnectorInformation

DESCRIPTION
    Store information related to USB connector

RETURNS
    void
*/
#ifdef ENABLE_CHARGER_V2
void usbUpdateConnectorInformation(usb_attached_status attached_status, uint16 dp, uint16 dm, usb_type_c_advertisement cc_status);
#else
#define usbUpdateConnectorInformation(attached_status, dp, dm, cc_status) ((void)(0))
#endif

/****************************************************************************
NAME 
    usbSetBootMode
    
DESCRIPTION
    Set the boot mode to default or low power
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbSetBootMode(uint8 bootmode);
#else
#define usbSetBootMode(x) ((void)(0))
#endif



/****************************************************************************
NAME 
    handleUsbMessage
    
DESCRIPTION
    Handle firmware USB messages
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void handleUsbMessage(Task task, MessageId id, Message message);
#else
#define handleUsbMessage(task, id, message) ((void)(0))
#endif


/****************************************************************************
NAME 
    usbTimeCriticalInit
    
DESCRIPTION
    Initialise USB. This function is time critical and must be called from
    _init. This will fail if either Host Interface is not set to USB or
    VM control of USB is FALSE in PS. It may also fail if Transport in the
    project properties is not set to USB VM.
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbTimeCriticalInit(void);
#else
#define usbTimeCriticalInit() ((void)(0))
#endif


/****************************************************************************
NAME 
    usbInit
    
DESCRIPTION
    Initialisation done once the main loop is up and running. Determines USB
    attach status etc.
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbInit(void);
#else
#define usbInit() ((void)(0))
#endif


/****************************************************************************
NAME 
    usbSetVbusLevel
    
DESCRIPTION
    Set whether VBUS is above or below threshold
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbSetVbusLevel(voltage_reading vbus);
#else
#define usbSetVbusLevel(x) ((void)(0))
#endif


/****************************************************************************
NAME 
    usbSetDeadBattery
    
DESCRIPTION
    Set whether VBAT is below the dead battery threshold
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
void usbSetVbatDead(bool dead);
#else
#define usbSetVbatDead(x) ((void)(0))
#endif


/****************************************************************************
NAME 
    usbGetChargeCurrent
    
DESCRIPTION
    Get USB charger limits
    
RETURNS
    void
*/ 
#ifdef ENABLE_USB
bool usbGetChargeCurrent(usb_charge_current_setting* usb_limit);
#else
#define usbGetChargeCurrent(x) (FALSE)
#endif

/****************************************************************************
NAME 
    usbIsAttached
    
DESCRIPTION
    Determine if USB is attached
    
RETURNS
    TRUE if USB is attached, FALSE otherwise
*/ 
#ifdef ENABLE_USB
bool usbIsAttached(void);
#else
#define usbIsAttached() (FALSE)
#endif

/****************************************************************************
NAME
    usbIsVoiceSupported

DESCRIPTION
    checks for a usb voice support.

RETURNS
    true if usb voice is supported, false if no usb voice or USB music is supported
*/
#ifdef ENABLE_USB
bool usbIsVoiceSupported(void);
#else
#define usbIsVoiceSupported() (FALSE)
#endif

/****************************************************************************
NAME 
    usbAudioIsAttached
    
DESCRIPTION
    Determine if USB Audio is attached
    
RETURNS
    TRUE if USB Audio is attached, FALSE otherwise
*/ 
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbAudioIsAttached(void);
#else
#define usbAudioIsAttached() (FALSE)
#endif

/****************************************************************************
NAME
    usbSpeakerClassIsEnumerated

DESCRIPTION
    Determine if USB speaker class is enumerated. 

RETURNS
    TRUE if USB speaker class is enumerated, FALSE otherwise
*/
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbSpeakerClassIsEnumerated(void);
#else
#define usbSpeakerClassIsEnumerated() (FALSE)
#endif

/****************************************************************************
NAME 
    usbAudioSinkMatch
    
DESCRIPTION
    Compare sink to the USB audio sink
    
RETURNS
    TRUE if sink matches USB audio sink, otherwise FALSE
*/ 
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbAudioSinkMatch(Sink sink);
#else
#define usbAudioSinkMatch(sink) (FALSE)
#endif

#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbAudioPopulateConnectParameters(audio_connect_parameters *connect_parameters);
bool usbVoicePopulateConnectParameters(audio_connect_parameters *connect_parameters);
#else
#define usbAudioPopulateConnectParameters(x) (FALSE)
#define usbVoicePopulateConnectParameters(x) (FALSE)
#endif

/*************************************************************************
NAME    
    usbAudioResume
    
DESCRIPTION
    Issue HID consumer control command to attempt to resume current USB stream

RETURNS
    None
    
**************************************************************************/
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
void usbAudioResume(void);
#else
#define usbAudioResume() ((void)(0))
#endif

/****************************************************************************
NAME 
    usbAudioDisconnect
    
DESCRIPTION
    Disconnect USB audio stream
    
RETURNS
    void
*/ 
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
void usbAudioPostDisconnectConfiguration(void);
#else
#define usbAudioPostDisconnectConfiguration() ((void)(0))
#endif

#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
void usbVoicePostDisconnectConfiguration(void);
#else
#define usbVoicePostDisconnectConfiguration() ((void)(0))
#endif

/****************************************************************************
NAME 
    usbAudioGetMode
    
DESCRIPTION
    Get the current USB audio mode if USB in use
    
RETURNS
    void
*/ 
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
void usbAudioGetMode(AUDIO_MODE_T* mode);
#else
#define usbAudioGetMode(x) ((void)(0))
#endif

/****************************************************************************
NAME 
    usbGetAudioSink
    
DESCRIPTION
    check USB state and return sink if available
    
RETURNS
   sink if available, otherwise 0
*/ 
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
Sink usbGetAudioSink(void);
#else
#define usbGetAudioSink() ((Sink)(NULL))
#endif

/****************************************************************************
NAME
    usbGetVoiceSink

DESCRIPTION
    check USB voice state and return sink if available

RETURNS
   sink if available, otherwise 0
*/
#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
Sink usbGetVoiceSink(void);
#else
#define usbGetVoiceSink() ((Sink)(NULL))
#endif


#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbIsCurrentAudioSinkUsb(void);
#else
#define usbIsCurrentAudioSinkUsb() (FALSE)
#endif

#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbIsCurrentVoiceSinkUsb(void);
#else
#define usbIsCurrentVoiceSinkUsb() (FALSE)
#endif


#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
bool usbIfUsbSinkExists(void);
#else
#define usbIfUsbSinkExists() (FALSE)
#endif

#if defined(ENABLE_USB) && defined(ENABLE_USB_AUDIO)
int16 usbGetVolumeLevel(void);
#else
#define usbGetVolumeLevel() ((int16)(0))
#endif

#if defined(ENABLE_USB)
void sinkUsbVolumeIncrement(void);
#else
#define sinkUsbVolumeIncrement() ((void)0)
#endif

#if defined(ENABLE_USB)
void sinkUsbVolumeDecrement(void);
#else
#define sinkUsbVolumeDecrement() ((void)0)
#endif

#if defined(ENABLE_USB)
#define sinkUsbDeviceClassMatch(class)  (sinkUsbconfigGetDeviceClassType() & class)
#else
#define sinkUsbDeviceClassMatch(x)  (FALSE)
#endif

#if defined(ENABLE_USB)
bool sinkUsbIsMicrophoneActive(void);
#else
#define sinkUsbIsMicrophoneActive()  (FALSE)
#endif

#if defined(ENABLE_USB)
bool sinkUsbIsSpeakerActive(void);
#else
#define sinkUsbIsSpeakerActive()  (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsUsbPluginTypeStereo()  (sinkUsbconfigGetPluginType() == usb_plugin_stereo)
#else
#define sinkUsbIsUsbPluginTypeStereo()  (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsUsbPluginTypeMonoNb()  (sinkUsbconfigGetPluginType() == usb_plugin_mono_nb)
#else
#define sinkUsbIsUsbPluginTypeMonoNb()  (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsUsbPluginTypeMonoWb()  (sinkUsbconfigGetPluginType() == usb_plugin_mono_wb)
#else
#define sinkUsbIsUsbPluginTypeMonoWb()  (FALSE)
#endif

#if defined(ENABLE_USB)
bool sinkUsbAudioIsSuspended(void);
#else
#define sinkUsbAudioIsSuspended()   (FALSE)
#endif

#if defined(ENABLE_USB)
bool sinkUsbAudioIsSuspendedLocal(void);
#else
#define sinkUsbAudioIsSuspendedLocal()   (FALSE)
#endif

#ifdef ENABLE_USB_AUDIO
#define sinkUsbAudioIsSuspendedNone()   (USB_RUNDATA.audio_suspend_state == usb_audio_suspend_none)
#else
#define sinkUsbAudioIsSuspendedNone()   (FALSE)
#endif

#if defined(ENABLE_USB)
 bool sinkUsbIsEnumerated(void);
#else
#define sinkUsbIsEnumerated() (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsDeconfigured()  (USB_RUNDATA.deconfigured)
#else
#define sinkUsbIsDeconfigured() (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsSuspended()  (USB_RUNDATA.suspended)
#else
#define sinkUsbIsSuspended() (FALSE)
#endif
#if defined(ENABLE_USB)
#define sinkUsbIsDeadBattery()  (USB_RUNDATA.dead_battery)
#else
#define sinkUsbIsDeadBattery() (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsVolEventAllowed()  (USB_RUNDATA.vol_event_allowed)
#else
#define sinkUsbIsVolEventAllowed() (FALSE)
#endif

#if defined(ENABLE_USB)
bool sinkUsbProcessEventUsb(const MessageId EventUsb);
#else
#define sinkUsbProcessEventUsb(x) (FALSE)
#endif

#if defined(ENABLE_USB) && defined(ENABLE_USB_HUB_SUPPORT)
void usbAttachToHub(void);
#else
#define usbAttachToHub(x) ((void)0)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsReady()  (USB_RUNDATA.ready)
#else
#define sinkUsbIsReady() (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbGetOutLVol()  (USB_RUNDATA.out_l_vol)
#else
#define sinkUsbGetOutLVol() (FALSE)
#endif

#if defined(ENABLE_USB)
#define sinkUsbIsVbusOkay()  (USB_RUNDATA.vbus_okay)
#else
#define sinkUsbIsVbusOkay() (FALSE)
#endif


/*************************************************************************
NAME    
    sinkUsbconfigGetUsbAttachTimeout
    
DESCRIPTION
    Get the usb attach timeout value.
RETURNS
    uint16
*/
#if defined(ENABLE_USB)
uint16 sinkUsbconfigGetUsbAttachTimeout(void);
#else
#define sinkUsbconfigGetUsbAttachTimeout() ((void)0)
#endif

/*************************************************************************
NAME    
    sinkUsbconfigGetUsbDeconfigTimeout
    
DESCRIPTION
    Get the usb deconfig timeout value.
RETURNS
    uint16
*/
#if defined(ENABLE_USB)
uint16 sinkUsbconfigGetUsbDeconfigTimeout(void);
#else
#define sinkUsbconfigGetUsbDeconfigTimeout() ((void)0)
#endif

/*************************************************************************
NAME    
    sinkUsbconfigIsAudioAlwaysOn
    
DESCRIPTION
    Get the state of usb audio always on from configuration.
RETURNS
    TRUE or FALSE
*/
#if defined(ENABLE_USB)
bool sinkUsbconfigIsAudioAlwaysOn(void);
#else
#define sinkUsbconfigIsAudioAlwaysOn() (FALSE)
#endif

/*************************************************************************
NAME    
    sinkUsbconfigGetPauseWhenSwitchingSource
    
DESCRIPTION
    Get the state of usb pause when switching usb source.
RETURNS
    TRUE or FALSE
*/
#if defined(ENABLE_USB)
bool sinkUsbconfigGetPauseWhenSwitchingSource(void);
#else
#define sinkUsbconfigGetPauseWhenSwitchingSource() (FALSE)
#endif

/*************************************************************************
NAME    
    sinkUsbconfigGetPluginType
    
DESCRIPTION
    Get usb plugin type from configuration.
RETURNS
    usb_plugin_type
*/
#if defined(ENABLE_USB)
usb_plugin_type sinkUsbconfigGetPluginType(void);
#else
#define sinkUsbconfigGetPluginType() (FALSE)
#endif

/*************************************************************************
NAME    
    sinkUsbconfigGetPluginIndex
    
DESCRIPTION
    Get usb plugin index from configuration.
RETURNS
    uint8
*/
#if defined(ENABLE_USB)
uint8 sinkUsbconfigGetPluginIndex(void);
#else
#define sinkUsbconfigGetPluginIndex() (FALSE)
#endif

/*************************************************************************
NAME    
    sinkUsbSetUsbVolumeFromSessionData
    
DESCRIPTION
    Set the usb volume from session data.

RETURNS
    void
    
**************************************************************************/

#if defined(ENABLE_USB)
uint16 sinkUsbGetUsbVolumeIndex(void);
#else
#define sinkUsbGetUsbVolumeIndex() 0
#endif
/*************************************************************************
NAME    
    sinkUsbSetSessionData
    
DESCRIPTION
    Set the usb volume from usb.

RETURNS
    void
    
**************************************************************************/
#if defined(ENABLE_USB)
void sinkUsbSetSessionData(void);
#else
#define sinkUsbSetSessionData() ((void)(0))
#endif

/*************************************************************************
NAME    
    sinkUsbconfigGetDeviceClassType
    
DESCRIPTION
    Get the usb config device class.

RETURNS
    usb_device_class_type
    
**************************************************************************/
#if defined(ENABLE_USB)
usb_device_class_type sinkUsbconfigGetDeviceClassType(void);
#endif

/*************************************************************************
NAME    
    sinkUsbSetMainVolume
    
DESCRIPTION
    Set the usb volume.

RETURNS
    void
    
**************************************************************************/
#if defined(ENABLE_USB)
void sinkUsbSetMainVolume(int16 usb_volume);
#else
#define sinkUsbSetVolume(x) ((void)(0))
#endif

/*************************************************************************
NAME    
    sinkUsbGetUsbVolume
    
DESCRIPTION
    Get the usb volume.

RETURNS
    void
    
**************************************************************************/
#if defined(ENABLE_USB)
void sinkUsbGetUsbVolume(volume_info *usb_volume);
#else
#define sinkUsbGetUsbVolume(x) ((void)(0))
#endif

/*************************************************************************
NAME    
    sinkUsbSetUsbVolume
    
DESCRIPTION
    Set the usb volume.

RETURNS
    void
    
**************************************************************************/
#if defined(ENABLE_USB)
void sinkUsbSetUsbVolume(const volume_info usb_volume);
#else
#define sinkUsbSetUsbVolume(x) ((void)(0))
#endif

/*************************************************************************
NAME    
    sinkUsbGetSessionData
    
DESCRIPTION
    Get the USB session data from config store.

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_USB
void sinkUsbGetSessionData(void);
#else
#define sinkUsbGetSessionData() ((void)(0))
#endif

/*************************************************************************
NAME    
    sinkUsbGetHfpPluginParams
    
DESCRIPTION
    Get the HFP Plugin Parameters from the USB Device Class library

RETURNS
    void
    
**************************************************************************/
#ifdef ENABLE_USB
void sinkUsbGetHfpPluginParams(usb_common_params *usb_params);
#else
#define sinkUsbGetHfpPluginParams(x) ((void) 0)
#endif

/****************************************************************************
NAME
    sinkUsbGetBoost

DESCRIPTION
    Get the USB charger boost settings based on USB limits.

RETURNS
    void
*/
#ifdef ENABLE_USB
power_boost_enable sinkUsbGetBoost(usb_charge_current_setting *usb_limits);
#else
#define sinkUsbGetBoost(usb_limits) (power_boost_disabled)
#endif

/*************************************************************************
NAME
    sinkUsbVoicePostConnectConfiguration

DESCRIPTION
    Enters to 'deviceActiveCallNoSCO' state post successful connection.

RETURNS
    void

**************************************************************************/
#ifdef ENABLE_USB
void sinkUsbVoicePostConnectConfiguration(void);
#else
#define sinkUsbVoicePostConnectConfiguration() ((void) 0)
#endif

#endif /* _SINK_USB_H_ */
