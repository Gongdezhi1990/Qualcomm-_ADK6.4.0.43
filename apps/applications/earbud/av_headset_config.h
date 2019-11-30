/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_config.h
\brief      Application configuration file
*/

#ifndef _AV_HEADSET_CONFIG_H_
#define _AV_HEADSET_CONFIG_H_

/*! Setting used to indicate that the MIC to use is not configured */
#define NO_MIC  (0xFF)

/*! Allow 2nd Earbud to connect to TWS+ Handset after pairing */
#define ALLOW_CONNECT_AFTER_PAIRING (TRUE)

/*! Maximum number of earbud initiated pairing attemts to the handset. (limit is 7) */
#define appConfigMaxHandsetPairingAttempts() (4)

/*! Can BLE be used to perform upgrades when not in the case */
#define appConfigDfuAllowBleUpgradeOutOfCase()      TRUE


/*! Can BREDR be used to perform upgrades when not in the case */
#define appConfigDfuAllowBredrUpgradeOutOfCase()    FALSE


/*! Only allow upgrades when the request has been made by the user (through the UI) 
    and the device is in the case.
  */
#define appConfigDfuOnlyFromUiInCase()              FALSE

/*! Timeout for entering the case after selecting DFU from the user
    interface */
#define appConfigDfuTimeoutToPlaceInCaseMs()        (D_SEC(60))

/*! Timeout for starting DFU mode after a restart caused by an
    upgrade completing. The timeout will only apply if the device
    is out of the case. 

    The timeout can be set to 0, in which case there is no limit. */
#define appConfigDfuTimeoutToStartAfterRestartMs()  (D_SEC(30))

/*! Timeout for DFU mode, entered after a reboot in DFU mode. 

    This is the timeout for an abnormal restart. Restarts that occur
    as part of the normal upgrade process use 
    appConfigDfuTimeoutToStartAfterRestartMs()

    The timeout can be set to 0, in which case there is no limit. */
#define appConfigDfuTimeoutToStartAfterRebootMs()   (D_SEC(15))

/*! Timeout for DFU mode, entered from UI 

    This is the timeout for starting an upgrade when the user has
    requested DFU and then placed the device into the case.

    The timeout can be set to 0, in which case there is no limit. */
#define appConfigDfuTimeoutAfterEnteringCaseMs()    (D_SEC(50))

/*! Timeout for DFU mode, requested from GAIA

    This is the timeout for starting an upgrade after the GAIA 
    upgrade protocol has been connected. Only applicable in the
    in case DFU mode.

    The timeout can be set to 0, in which case there is no limit.
*/
#define appConfigDfuTimeoutAfterGaiaConnectionMs()  (D_SEC(45))


/*! Timeout to detect ending a GAIA upgrade connection shortly after starting

    This can be used by a host application to check whether the upgrade
    feature is supported. This should not count as an upgrade connection.
 */
#define appConfigDfuTimeoutCheckForSupportMs()      (D_SEC(3))

/*! Default speaker gain */
#define HFP_SPEAKER_GAIN    (10)

/*! Default microphone gain */
#define HFP_MICROPHONE_GAIN (15)

/*! Auto answer call on connect */
#define HFP_CONNECT_AUTO_ANSWER

/*! Disable - Auto transfer call on connect */
#undef HFP_CONNECT_AUTO_TRANSFER

/*! Enable HF battery indicator */
#define appConfigHfpBatteryIndicatorEnabled() (1)


/*! Disable support for TWS+ */
//#define DISABLE_TWS_PLUS

/*! Default BR/EDR Authenticated Payload Timeout */
#define DEFAULT_BR_EDR_AUTHENTICATED_PAYLOAD_TIMEOUT (10000)

/*! Maximum Authenticated Payload Timeout as per HFP spec */
#define HFP_AUTHENTICATED_PAYLOAD_TIMEOUT_SC_MAX_MS (10000)

#ifdef USE_BDADDR_FOR_LEFT_RIGHT
/*! Left and right earbud roles are selected from Bluetooth address. */
/*! TRUE if this is the left earbud (Bluetooth address LAP is odd). */
#define appConfigIsLeft()           (appGetInit()->appInitIsLeft)
/*! TRUE if this is the right earbud (Bluetooth address LAP is even). */
#define appConfigIsRight()          (appConfigIsLeft() ^ 1)
#else
/*! Left and right earbud roles are selected from the state of this PIO */
#define appConfigHandednessPio()    (2)
/*! TRUE if this is the left earbud (the #appConfigHandednessPio state is 1) */
#define appConfigIsLeft()           ((PioGet32Bank(appConfigHandednessPio() / 32) & (1UL << appConfigHandednessPio())) ? 1 : 0)
/*! TRUE if this is the right earbud (the #appConfigHandednessPio state is 0) */
#define appConfigIsRight()          (appConfigIsLeft() ^ 1)
#endif

/*! Number of paired devices that are remembered */
#define appConfigMaxPairedDevices() (4)

#ifdef CF133_BATT 
/*! Minimum RSSI to pair with a device */
#define appConfigPeerPairingMinRssi() (-60)
#else
/*! Minimum RSSI to pair with a device */
#define appConfigPeerPairingMinRssi() (-50)
#endif

/*! Minimum difference in RSSI between devices discovered 
    with the highest RSSI in order to pair */
#define appConfigPeerPairingMinRssiDelta() (10)

/*! Timeout in seconds for user initiated peer pairing */
#define appConfigPeerPairingTimeout()       (120)
/*! Timeout in seconds for user initiated handset pairing */
#define appConfigHandsetPairingTimeout()    (120)
/*! Timeout in seconds to disable page/inquiry scan after entering idle state */
#define appConfigPairingScanDisableDelay()  (5)

/*! Timeout in seconds for automatic peer pairing */
#define appConfigAutoPeerPairingTimeout()       (0)
/*! Timeout in seconds for automatic handset pairing */
#define appConfigAutoHandsetPairingTimeout()    (300)

/*! Qualcomm Bluetooth SIG company ID */
#define appConfigBtSigCompanyId() (0x00AU)
/*! Qualcomm IEEE company ID */
#define appConfigIeeeCompanyId()  (0x00025BUL)

/*! Key ID peer Earbud link-key derivation */
#define appConfigTwsKeyId()       (0x74777332UL)

/*!@{ @name LED pin PIO assignments (chip specific)
      @brief The LED pads can either be controlled by the led_controller hardware
             or driven as PIOs. The following define the PIO numbers used to
             control the LED pads as PIOs.
*/
#define CHIP_LED_0_PIO CHIP_LED_BASE_PIO
#if CHIP_NUM_LEDS > 1
#define CHIP_LED_1_PIO CHIP_LED_BASE_PIO + 1
#endif
#if CHIP_NUM_LEDS > 2
#define CHIP_LED_2_PIO CHIP_LED_BASE_PIO + 2
#endif
#if CHIP_NUM_LEDS > 3
#define CHIP_LED_3_PIO CHIP_LED_BASE_PIO + 3
#endif
#if CHIP_NUM_LEDS > 4
#define CHIP_LED_4_PIO CHIP_LED_BASE_PIO + 4
#endif
#if CHIP_NUM_LEDS > 5
#define CHIP_LED_5_PIO CHIP_LED_BASE_PIO + 5
#endif
//!@}

#if defined(HAVE_1_LED)

/*! The number of LEDs av_headset_led will control. If one LED is configured,
    it will use appConfigLed0Pio(), if two LEDs are configured it will use
    appConfigLed0Pio() and appConfigLed1Pio() etc. */
#define appConfigNumberOfLeds()  (1)

/*! PIO to control LED0 */
#define appConfigLed0Pio()       CHIP_LED_0_PIO
/*! PIO to control LED1 (not used) */
#define appConfigLed1Pio()       (0)
/*! PIO to control LED2 (not used) */
#define appConfigLed2Pio()       (0)

#elif defined(HAVE_3_LEDS)

/* The number of LEDs av_headset_led will control. */
#define appConfigNumberOfLeds()  (3)
/*! PIO to control LED0 */
#define appConfigLed0Pio()       CHIP_LED_0_PIO
/*! PIO to control LED1 */
#define appConfigLed1Pio()       CHIP_LED_1_PIO
/*! PIO to control LED2 */
#define appConfigLed2Pio()       CHIP_LED_2_PIO

#else
#error LED config not correctly defined.
#endif

/*! Returns boolean TRUE if PIO is an LED pin, otherwise FALSE */
#define appConfigPioIsLed(pio) (((pio) >= CHIP_LED_BASE_PIO) && ((pio < (CHIP_LED_BASE_PIO + CHIP_NUM_LEDS))))
/*! Returns the LED number for a LED PIO. Assumes led is an LED PIO. */
#define appConfigPioLedNumber(pio) ((pio) - CHIP_LED_BASE_PIO)
/*! Product specific range of PIO that can wake the chip from dormant */
#define appConfigPioCanWakeFromDormant(pio) ((pio) >= 1 && ((pio) <= 8))

/*! Allow LED indications when Earbud is in ear */
#define appConfigInEarLedsEnabled() (TRUE)

/*! Page timeout to use as left earbud attempting connection to the right Earbud. */
#define appConfigLeftEarbudPageTimeout()    (0x1000)

/*! Page timeout to use as right earbud attempting connection to the left Earbud. */
#define appConfigRightEarbudPageTimeout()   (0x2000)

/*! Page timeout to use for connecting to any non-peer Earbud devices. */
#define appConfigDefaultPageTimeout()       (0x4000)

/*! The page timeout multiplier for Earbuds after link-loss.
    Multiplier should be chosen carefully to make sure total page timeout doesn't exceed 0xFFFF */
#define appConfigEarbudLinkLossPageTimeoutMultiplier()  (3)

/*! The page timeout multiplier for Handsets after link-loss.
    Multiplier should be chosen carefully to make sure total page timeout doesn't exceed 0xFFFF */
#define appConfigHandsetLinkLossPageTimeoutMultiplier() (4)

/*! Inactivity timeout after which peer signalling channel will be disconnected, 0 to leave connected (in sniff) */
#define appConfigPeerSignallingChannelTimeoutSecs()   (0)


/*! Link supervision timeout for ACL between Earbuds (in milliseconds) */
#define appConfigEarbudLinkSupervisionTimeout()  (5000)

/*! Default link supervision timeout for other ACLs (in milliseconds) */
#define appConfigDefaultLinkSupervisionTimeout()  (5000)


/*! Minimum volume gain in dB */
#define appConfigMinVolumedB() (-45)

/*! Maximum volume gain in dB */
#define appConfigMaxVolumedB() (0)

/*! Default volume gain in dB */
#define appConfigDefaultVolumedB() (-10)

/*! The volume setting to use for no gain, when volume is specifed on range of 0-127 */
#define appConfigVolumeNoGain127Step()  (127)

/*! Number of volume steps to use per AV UI volume event. 
 The full volume range is 0-127 */
#define appConfigGetAvVolumeStep()  (8)

/*! Number of volume steps to use per HFP UI volume event. 
 The full volume range is 0-15 */
 #define appConfigGetHfpVolumeStep() (1)

/*! Enable Super-wideband SCO */
#define appConfigScoSwbEnabled() (TRUE)

//!@{ @name Parameters for microphone 0 - Left analog MIC */
#define appConfigMic0Bias()                     (BIAS_CONFIG_MIC_BIAS_0)
#define appConfigMic0BiasVoltage()              (3) /* 1.9v */
#define appConfigMic0PreAmp()                   (TRUE)
#define appConfigMic0Pio()                      (0x13)
#define appConfigMic0Gain()                     (0x5)
#define appConfigMic0IsDigital()                (FALSE)
#define appConfigMic0AudioInstance()            (AUDIO_INSTANCE_0)
#define appConfigMic0AudioChannel()             (AUDIO_CHANNEL_A)
//!@}

//!@{ @name Parameters for microphone 1 - Right analog MIC */
#define appConfigMic1Bias()                     (BIAS_CONFIG_MIC_BIAS_0)
#define appConfigMic1BiasVoltage()              (3) /* 1.9v */
#define appConfigMic1PreAmp()                   (TRUE)
#define appConfigMic1Pio()                      (0x16)
#define appConfigMic1Gain()                     (0x5)
#define appConfigMic1IsDigital()                (FALSE)
#define appConfigMic1AudioInstance()            (AUDIO_INSTANCE_0)
#define appConfigMic1AudioChannel()             (AUDIO_CHANNEL_B)
//!@}

#if defined(CORVUS_PG806)

//!@{ @name Parameters for microphone 2 - HBL_L_FB Analog MIC connected to digital MIC ADC */
#define appConfigMic2Bias()                     (BIAS_CONFIG_MIC_BIAS_0)
#define appConfigMic2BiasVoltage()              (3)
#define appConfigMic2PreAmp()                   (FALSE)
#define appConfigMic2Pio()                      (0)
#define appConfigMic2Gain()                     (0)
#define appConfigMic2IsDigital()                (TRUE)
#define appConfigMic2AudioInstance()            (AUDIO_INSTANCE_1)
#define appConfigMic2AudioChannel()             (AUDIO_CHANNEL_B)
//!@}

//!@{ @name Parameters microphone 3 - HBL_L_FF Analog MIC connected to digital MIC ADC */
#define appConfigMic3Bias()                     (BIAS_CONFIG_MIC_BIAS_0)
#define appConfigMic3BiasVoltage()              (3)
#define appConfigMic3PreAmp()                   (FALSE)
#define appConfigMic3Pio()                      (0)
#define appConfigMic3Gain()                     (0)
#define appConfigMic3IsDigital()                (TRUE)
#define appConfigMic3AudioInstance()            (AUDIO_INSTANCE_1)
#define appConfigMic3AudioChannel()             (AUDIO_CHANNEL_A)
//!@}

/*! @{ Which microphones to use for SCO */
    /*! microphone to use for the first SCO mic */    
#define appConfigScoMic1()                      (0)
    /*! microphone to use for the second SCO mic. This should be defined as 
        NO_MIC if using 1-mic CVC */
#define appConfigScoMic2()                      (NO_MIC)     /* Don't use microphone for SCO 2nd mic (CVC 1-mic) */
/*! @} */

//!@{ @name ANC configuration */
#define appConfigAncPathEnable()                (feed_forward_mode_left_only)
#define appConfigAncFeedForwardMic()            (3)         /* Use microphone 3 for ANC feed-forward */
#define appConfigAncFeedBackMic()               (NO_MIC)    /* Don't use microphone for ANC feed-back */
#define appConfigAncMicGainStepSize()           (5)
#define appConfigAncSidetoneGain()              (10)
#define appConfigAncMode()                      (anc_mode_1)
//!@}


/*! Enable ANC tuning functionality */
#define appConfigAncTuningEnabled()             (TRUE)

/*! ANC tuning monitor microphone */
#define appConfigAncTuningMonitorMic()          (2)

#else

//!@{ @name Parameters for microphone 2 */
#define appConfigMic2Bias()                     (BIAS_CONFIG_PIO)
#define appConfigMic2BiasVoltage()              (0)
#define appConfigMic2PreAmp()                   (FALSE)
#define appConfigMic2Pio()                      (4)
#define appConfigMic2Gain()                     (0)
#define appConfigMic2IsDigital()                (TRUE)
#define appConfigMic2AudioInstance()            (AUDIO_INSTANCE_1)
#define appConfigMic2AudioChannel()             (AUDIO_CHANNEL_A)
//!@}

//!@{ @name Parameters microphone 3 */
#define appConfigMic3Bias()                     (BIAS_CONFIG_PIO)
#define appConfigMic3BiasVoltage()              (0)
#define appConfigMic3PreAmp()                   (FALSE)
#define appConfigMic3Pio()                      (4)
#define appConfigMic3Gain()                     (0)
#define appConfigMic3IsDigital()                (TRUE)
#define appConfigMic3AudioInstance()            (AUDIO_INSTANCE_1)
#define appConfigMic3AudioChannel()             (AUDIO_CHANNEL_B)
//!@}

/*! @{ Which microphones to use for SCO */
    /*! microphone to use for the first SCO mic */    
#define appConfigScoMic1()                      (0)          /* Use microphone 0 for SCO 1st mic */
    /*! microphone to use for the second SCO mic. This should be defined as 
        NO_MIC if using 1-mic CVC */
#define appConfigScoMic2()                      (NO_MIC)     /* Don't use microphone for SCO 2nd mic (CVC 1-mic) */
/*! @} */

//!@{ @name ANC configuration */
#define appConfigAncPathEnable()                (all_disabled)
#define appConfigAncFeedForwardMic()            (0)         /* Use microphone 0 for ANC feed-forward */
#define appConfigAncFeedBackMic()               (NO_MIC)    /* Don't use microphone for ANC feed-back */
#define appConfigAncMicGainStepSize()           (5)
#define appConfigAncSidetoneGain()              (10)
#define appConfigAncMode()                      (anc_mode_1)
//!@}

/*! Enable ANC tuning functionality */
#define appConfigAncTuningEnabled()             (FALSE)

/*! ANC tuning monitor microphone */
#define appConfigAncTuningMonitorMic()          (NO_MIC)

#endif

/*! Enable SCO forwarding */
#define appConfigScoForwardingEnabled()         (TRUE)

/*! Enable microphone forwarding (dependant on SCO forwarding) */
#define appConfigMicForwardingEnabled()         (TRUE)

/*! SCO Forwarding Ring tone delay in (ms) */
#define appConfigScoFwdRingMs()                 (1000)


/*! The minimum time to play to be added on incoming SCO connections 
    to allow synchronisation. This should represent the total propagation 
    delay in the chain */
#define appConfigScoChainBaseTTP()      (15000)

/*! Time to play to be applied on this earbud, based on the Wesco
    value specified when creating the connection.

    With TWS+ the following SCO parameters are used
        EB1: Desco:0 Wesco:6
        EB2: Desco:2 Wesco:8

    This causes the EB2 audio to be sampled 2.5ms later in the AudioSS
    so we adjust EB1 by 2.5ms to align them.

    NB: A value of 0 will disable TTP */
#define appConfigScoChainTTP(wesco)     (wesco == 6 ? 22500 : 20000)


/*! Should the earbud automatically try to pair with a peer earbud
 * if no pairing exists? */
#define appConfigAutoPeerPairingEnabled()   (TRUE)

/*! Should the earbud automatically try to pair with a handset
 * if no pairing exists? */
#define appConfigAutoHandsetPairingEnabled()   (TRUE)

/*! Should the earbud automatically reconnect to last connected
 * handset on startup? */
#define appConfigAutoReconnectHandset()     (TRUE)

/*! The time before the TTP at which a packet should be transmitted */
#define appConfigTwsTimeBeforeTx()  MAX(70000, TWS_STANDARD_LATENCY_US-200000)

/*! The last time before the TTP at which a packet may be transmitted */
#define appConfigTwsDeadline()      MAX(35000, TWS_STANDARD_LATENCY_US-250000)

/*! Charger configuration */

/*! The time to debounce charger state changes (ms).  
    The charger hardware will have a more limited range. */
#define appConfigChargerStateChangeDebounce()          (128)

/*! Trickle-charge current (mA) */
#define appConfigChargerTrickleCurrent()               (10)

/*! Pre-charge current (mA)*/
#define appConfigChargerPreCurrent()                   (20)

/*! Pre-charge to fast-charge threshold */
#define appConfigChargerPreFastThresholdVoltage()      (3000)

/*! Fast-charge current (mA) */
#define appConfigChargerFastCurrent()                  (100)

/*! Fast-charge (constant voltage) to standby transition point.
    Percentage of the fast charge current */
#define appConfigChargerTerminationCurrent()           (40)

/*! Fast-charge Vfloat voltage */
#define appConfigChargerTerminationVoltage()           (4200)

/*! Standby to fast charge hysteresis (mV) */
#define appConfigChargerStandbyFastVoltageHysteresis() (250)

/* Enable short timeouts for charger/battery platform testing */
#ifdef CF133_BATT
    /*! Pre-configured settings for the pre-charge timeout. See
        appConfigChargerPreChargeTimeoutMs */
#define CHARGER_PRE_CHARGE_TIMEOUT_MS D_MIN(5)
    /*! Pre-configured settings for the fast-charge timeout. See
        appConfigChargerFastChargeTimeoutMs */
#define CHARGER_FAST_CHARGE_TIMEOUT_MS D_MIN(15)
#else
    /*! Pre-configured settings for the pre-charge timeout. See
        appConfigChargerPreChargeTimeoutMs */
#define CHARGER_PRE_CHARGE_TIMEOUT_MS D_MIN(0)
    /*! Pre-configured settings for the fast-charge timeout. See
        appConfigChargerFastChargeTimeoutMs */
#define CHARGER_FAST_CHARGE_TIMEOUT_MS D_MIN(0)
#endif

/*! The charger will be disabled if the pre-charge time exceeds this limit.
    Following a timeout, the charger will be re-enabled when the charger is detached.
    Set to zero to disable the timeout. */
#define appConfigChargerPreChargeTimeoutMs() CHARGER_PRE_CHARGE_TIMEOUT_MS

/*! The charger will be disabled if the fast-charge time exceeds this limit.
    Following a timeout, the charger will be re-enabled when the charger is detached.
    Set to zero to disable the timeout. */
#define appConfigChargerFastChargeTimeoutMs() CHARGER_FAST_CHARGE_TIMEOUT_MS

//!@{ @name Battery voltage levels in milli-volts
#define appConfigBatteryFullyCharged()      (4200)
#define appConfigBatteryVoltageOk()         (3600)
#define appConfigBatteryVoltageLow()        (3300)
#define appConfigBatteryVoltageCritical()   (3000)
//!@}

//!@{ @name Battery temperature limits in degrees Celsius.
#define appConfigBatteryChargingTemperatureMax() 45
#define appConfigBatteryChargingTemperatureMin() 0
#define appConfigBatteryDischargingTemperatureMax() 60
#define appConfigBatteryDischargingTemperatureMin() -20
//!@}

/*! The interval at which the battery voltage is read. */
#define appConfigBatteryReadPeriodMs() D_SEC(2)

/*! Margin to apply on battery readings before accepting that 
    the level has changed. Units of milli-volts */
#define appConfigSmBatteryHysteresisMargin() (50)


/*! @{ Define the hardware settings for the left audio*/
/*! Define which channel the 'left' audio channel comes out of. */
#define appConfigLeftAudioChannel()              (AUDIO_CHANNEL_A)
/*! Define the type of Audio Hardware for the 'left' audio channel. */
#define appConfigLeftAudioHardware()             (AUDIO_HARDWARE_CODEC)
/*! Define the instance for the 'left' audio channel comes. */
#define appConfigLeftAudioInstance()             (AUDIO_INSTANCE_0)
/*! @} */

/*! Define whether audio should start with or without a soft volume ramp */
#define appConfigEnableSoftVolumeRampOnStart() (FALSE)

/*! Time to wait for successful disconnection of links to peer and handset
 *  before forcing factory reset. */
#define appConfigFactoryResetTimeoutMs()        (5000)

/*! Time to wait for successful disconnection of links to peer and handset
 *  in terminating substate before shutdown/sleep.
 * Setting this to zero will force the ACL to be disconnected immediately */
#define appConfigLinkDisconnectionTimeoutTerminatingMs() D_SEC(0)

/*! Time to wait for successful dis connection of links to peer and handset
 *  in terminating substate before entering pairing. */
#define appConfigLinkPairingDisconnectionTimeoutTerminatingMs() D_SEC(5)


/*!@{ @name External AMP control
      @brief If required, allows the PIO/bank/masks used to control an external
             amp to be defined.
*/
#if defined(CE821_CF212) || defined(CF376_CF212)

#define appConfigExternalAmpControlRequired()    (TRUE)
#define appConfigExternalAmpControlPio()         (32)
#define appConfigExternalAmpControlPioBank()     (1)
#define appConfigExternalAmpControlEnableMask()  (0 << 0)
#define appConfigExternalAmpControlDisableMask() (1 << (appConfigExternalAmpControlPio() % 32))

#else

#define appConfigExternalAmpControlRequired()    (FALSE)
#define appConfigExternalAmpControlPio()         (0)
#define appConfigExternalAmpControlEnableMask()  (0)
#define appConfigExternalAmpControlDisableMask() (0)

#endif /* defined(CE821_CF212) or defined(CF376_CF212) */
//!@}


#if !defined(CF133) && !defined(CG437) && !defined(CF020)

/*! Only enable LED indications when Earbud is out of ear */
#undef appConfigInEarLedsEnabled()
#define appConfigInEarLedsEnabled() (FALSE)

#endif /* !defined(CF133) && !defined(CG437) !defined(CF020) */


/*! Timeout for A2DP audio when earbud removed from ear. */
#define appConfigOutOfEarA2dpTimeoutSecs()      (2)

/*! Timeout within which A2DP audio will be automatically restarted
 *  if placed back in the ear. */
#define appConfigInEarA2dpStartTimeoutSecs()    (10)

/*! Timeout for SCO audio when earbud removed from ear. */
#define appConfigOutOfEarScoTimeoutSecs()      (2)

/*! Time to wait to connect AVRCP after a remotely initiated A2DP connection
    indication if the remote device does not initiate a AVRCP connection */
#define appConfigAvrcpConnectDelayAfterRemoteA2dpConnectMs() D_SEC(3)

/*! Time to wait for AVRCP playback status notification after sending an AVRCP
 *  passthrough command that would change the playback status */
#define appConfigAvrcpPlayStatusNotificationTimeout() D_SEC(1)

/*! Time to wait to connect A2DP media channel after a locally initiated A2DP connection */
#define appConfigA2dpMediaConnectDelayAfterLocalA2dpConnectMs() D_SEC(3)

/*! This timer is active in APP_STATE_OUT_OF_CASE_IDLE if set to a non-zero value.
    On timeout, the SM will allow sleep. */
#define appConfigIdleTimeoutMs()   D_SEC(300)

/*! Default DAC disconnection delay in milliseconds */
#define appConfigDacDisconnectionDelayMs() (60000)

/*! Microphone path delay variation. */
#define appConfigMicPathDelayVariationUs()      (10000)

/*! Enable or disable voice quality measurements for TWS+. */
#define appConfigVoiceQualityMeasurementEnabled() TRUE

/*! The worst reportable voice quality */
#define appConfigVoiceQualityWorst() 0
/*! The best reportable voice quality */
#define appConfigVoiceQualityBest() 15

/*! The voice quality to report if measurement is disabled. Must be in the
    range appConfigVoiceQualityWorst() to appConfigVoiceQualityBest(). */
#define appConfigVoiceQualityWhenDisabled() appConfigVoiceQualityBest()

/*! This config relates to the behavior of the TWS standard master when AAC sink.
    If TRUE, it will forward the received stereo AAC data to the slave earbud.
    If FALSE, it will transcode one channel to SBC mono and forward. */
#define appConfigAacStereoForwarding() TRUE

/*! This config controls if aptX A2DP support is enabled.  A valid license is also
 * required for aptX to enable this CODEC */
#define appConfigAptxEnabled() TRUE

/*! This config controls if AAC A2DP support is enabled */
#define appConfigAacEnabled() TRUE

/*! This config control if aptX adaptive A2DP support is enabled */
#if defined(CF376_CG724) || defined(CG437)
#define appConfigAptxAdaptiveEnabled() TRUE
#else
#define appConfigAptxAdaptiveEnabled() FALSE
#endif

/*! When a voice prompt is played, for this period of time, any repeated prompt
    will not be played. Set to zero to play all prompts, regardless of whether
    the prompt was recently played. */
#define appConfigPromptNoRepeatDelay() D_SEC(5)

/*! When the earbuds handover when A2DP audio is streaming, the new master earbud
    sends an AVRCP media play command to the handset when both AVRCP and A2DP media
    are connected. Some handsets may emit a burst of audio from their local speaker
    if the play command is sent to the handset too soon. To avoid this audio burst,
    the play command is delayed. */
#define appConfigHandoverMediaPlayDelay() D_SEC(1)

#if defined(INCLUDE_PROXIMITY)
#include "av_headset_proximity.h"
/*! The proximity sensor configuration */
extern const proximityConfig proximity_config;
/*! Returns the proximity sensor configuration */
#define appConfigProximity() (&proximity_config)
#endif /* INCLUDE_PROXIMITY */


#if defined(INCLUDE_ACCELEROMETER)
#include "av_headset_accelerometer.h"
/*! The accelerometer configuration */
extern const accelerometerConfig accelerometer_config;
/*! Returns the accelerometer configuration */
#define appConfigAccelerometer() (&accelerometer_config)
#endif /* INCLUDE_ACCELEROMETER */

#if defined(INCLUDE_TEMPERATURE)
#include "av_headset_temperature.h"
/*! The temperature sensor measurement interval in milli-seconds. */
#define appConfigTemperatureMeasurementIntervalMs() D_SEC(10)

#if defined(HAVE_THERMISTOR)
#include "peripherals/thermistor.h"
/*! The thermistor configuration */
extern const thermistorConfig thermistor_config;
/*! Returns the thermistor configration */
#define appConfigThermistor() (&thermistor_config)
#endif  /* HAVE_THERMISTOR */

#endif /* INCLUDE_TEMPERATURE */


/*! The time to play delay added in the SCO receive path.  

    A value of 40 will cause some missing / delayed packets in
    good test conditions. 
    It is estimated that 60 is the lowest value that may be
    used in the real world, with random 2.4GHz interference.
    It is recommended that the final value used should be selected 
    based on expected useage, and tolerance for delays vs. errors
    introduced by Packet Loss Concealment. */
#define appConfigScoFwdVoiceTtpMs()         (70)

#include "av_headset_ui.h"
/*! The audio prompt configuration */
extern const promptConfig prompt_config[];
/*! Return the indexed audio prompt configuration */
#define appConfigGetPromptConfig(index) &prompt_config[index]


    /********************************************
     *   SETTINGS for Bluetooth Low Energy (BLE) 
     ********************************************/

/*! Define whether BLE is allowed when out of the case.

    Restricting to use in the case only will reduce power
    consumption and extend battery life. It will not be possible
    to start an upgrade or read battery information.

    \note Any existing BLE connections will not be affected 
    when leaving the case.
 */
#define appConfigBleAllowedOutOfCase()          (FALSE)


/*! Should new connections be allowed when music is being played
    or when we are in a call.

    Selecting this option 
    \li reduces power consumption slightly as the advertisements neccesary 
    for a connection are relatively low power, 
    \li stops any distortion from connections

    \note Existing connections are not affected by this option
 */
#define appConfigBleNewConnectionsWhenBusy()   (FALSE)


/*! Number of simultaneous GATT connections allowed */
#define appConfigBleGetGattServerInstances()    (1)

/*! Minimum value of MTU for use with GATT over BLE */
#define appConfigBleGattMtuMin()                (64)


/*! Minimum interval between adverts when advertising at a slow rate */
#define appConfigBleSlowAdvertisingRateMin()    0x170
/*! Maximum interval between adverts when advertising at a slow rate */
#define appConfigBleSlowAdvertisingRateMax()    0x190

/*! Minimum interval between adverts when advertising at a fast rate */
#define appConfigBleFastAdvertisingRateMin()    0xA0
/*! Maximum interval between adverts when advertising at a fast rate */
#define appConfigBleFastAdvertisingRateMax()    0xC0


/*! Enumerated type to select pre-defined advertising rate settings */
typedef enum 
{
    APP_ADVERT_RATE_SLOW,       /*!< Select a slow advertising rate */
    APP_ADVERT_RATE_FAST,       /*!< Select a fast advertising rate */
} appConfigBleAdvertisingMode;


/*! Get the minimum and maximum advertising rates for the requested pre-defined mode

    Populate values for min and max advertising interval

    \param rate             Advertising mode requested
    \param[out] min_rate    Pointer to value for lowest advertising interval 
    \param[out] max_rate    Pointer to value for highest advertising interval

    \return TRUE if the mode is known, and min_rate and max_rate could be populated.
 */
bool appConfigBleGetAdvertisingRate(appConfigBleAdvertisingMode rate, uint16 *min_rate, uint16 *max_rate);


/*! Should we use a random address in our BLE advertisements

    When using Resolvable Private Addresses the address used will change between 
    connections and over time for adverts. When paired with a device the public
    address is still available to the peer.
 */
#define appConfigBleUseResolvablePrivateAddress()   (TRUE)


/*! The time before changing the address used for Bluetooth Low Energy adverts and connections

    The earbuds use Resolvable Private Addresses (RPA) for BLE connections.
    This address must be updated on a timer, the update being handled
    by the libraries.

    \note When using RPA the device is not allowed to use its real address
    in any adverts. Responding to own address is allowed.
 */
#define appConfigBleAddressChangeTime() (BLE_RPA_TIMEOUT_DEFAULT)

#endif /* _AV_HEADSET_CONFIG_H_ */

