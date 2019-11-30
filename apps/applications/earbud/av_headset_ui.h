/*!
\copyright  Copyright (c) 2008 - 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_ui.h
\brief	    Header file for the application User Interface
*/

#ifndef _AV_HEADSET_UI_H_
#define _AV_HEADSET_UI_H_

#include <a2dp.h>
#include "av_headset_kymera.h"
#include "av_headset_led.h"

/*! \brief Time between mute reminders (in seconds) */
#define APP_UI_MUTE_REMINDER_TIME               (15)

/*! \brief Time between SCO un-encrpyted warninged (in seconds) */
#define APP_UI_SCO_UNENCRYPTED_REMINDER_TIME	(5)

/*! \brief Time between inquiry in progress reminders (in seconds) */
#define APP_UI_INQUIRY_REMINDER_TIME            (5)

/*! \brief Time between connecting reminder tone (in seconds) */
#define APP_UI_CONNECTING_TIME                  (5)

/*! \brief Time between volume changes (in milliseconds) */
#define APP_UI_VOLUME_REPEAT_TIME               (300)

/*! \brief Fixed tone volume in dB */
#define APP_UI_TONE_VOLUME                      (-20)

/*! \brief Fixed prompt volume in dB */
#define APP_UI_PROMPT_VOLUME                    (-10)


/*! A list of prompts supported by the application.
    This list should be used as an index into a configuration array of
    promptConfigs defining the properties of each prompt. The array length should
    be NUMBER_OF_PROMPTS.
*/
typedef enum prompt_name
{
    PROMPT_POWER_ON = 0,
    PROMPT_POWER_OFF,
    PROMPT_PAIRING,
    PROMPT_PAIRING_SUCCESSFUL,
    PROMPT_PAIRING_FAILED,
    PROMPT_CONNECTED,
    PROMPT_DISCONNECTED,
    NUMBER_OF_PROMPTS,
    PROMPT_NONE = 0xffff,
} voicePromptName;

/*! \brief UI task structure */
typedef struct
{
    /*! The UI task. */
    TaskData task;
    /*! Input event manager task, can be used to generate virtual PIO events. */
    Task input_event_task;
    /*! Cache of the file index of each prompt */
    FILE_INDEX prompt_file_indexes[NUMBER_OF_PROMPTS];
    /*! The last prompt played, used to avoid repeating prompts */
    voicePromptName prompt_last;
} uiTaskData;

/*! Audio prompt configuration */
typedef struct prompt_configuration
{
    const char *filename; /*!< Prompt filename */
    uint32 rate;          /*!< Prompt sample rate */
    promptFormat format;  /*!< Prompt format */
} promptConfig;

/*! \brief The colour filter for the led_state applicable when the battery is low.
    \param led_state The input state.
    \return The filtered led_state.
*/
extern uint16 app_led_filter_battery_low(uint16 led_state);

/*! \brief The colour filter for the led_state applicable when charging but
           the battery voltage is still low.
    \param led_state The input state.
    \return The filtered led_state.
*/
extern uint16 app_led_filter_charging_low(uint16 led_state);

/*! \brief The colour filter for the led_state applicable when charging and the
           battery voltage is ok.
    \param led_state The input state.
    \return The filtered led_state.
*/
extern uint16 app_led_filter_charging_ok(uint16 led_state);

/*! \brief The colour filter for the led_state applicable when charging is complete.
    \param led_state The input state.
    \return The filtered led_state.
*/
extern uint16 app_led_filter_charging_complete(uint16 led_state);

//!@{ \name LED pattern and ringtone note sequence arrays.
extern const ledPattern app_led_pattern_power_on[];
extern const ledPattern app_led_pattern_power_off[];
extern const ledPattern app_led_pattern_error[];
extern const ledPattern app_led_pattern_idle[];
extern const ledPattern app_led_pattern_idle_connected[];
extern const ledPattern app_led_pattern_pairing[];
extern const ledPattern app_led_pattern_pairing_deleted[];
extern const ledPattern app_led_pattern_sco[];
extern const ledPattern app_led_pattern_call_incoming[];
extern const ledPattern app_led_pattern_battery_empty[];
extern const ledPattern app_led_pattern_peer_pairing[];

extern const ringtone_note app_tone_button[];

extern const ringtone_note app_tone_hfp_connect[];
extern const ringtone_note app_tone_hfp_connected[];
extern const ringtone_note app_tone_hfp_disconnected[];
extern const ringtone_note app_tone_hfp_link_loss[];
extern const ringtone_note app_tone_hfp_sco_connected[];
extern const ringtone_note app_tone_hfp_sco_disconnected[];
extern const ringtone_note app_tone_hfp_mute_reminder[];
extern const ringtone_note app_tone_hfp_sco_unencrypted_reminder[];
extern const ringtone_note app_tone_hfp_ring[];
extern const ringtone_note app_tone_hfp_ring_caller_id[];
extern const ringtone_note app_tone_hfp_voice_dial[];
extern const ringtone_note app_tone_hfp_voice_dial_disable[];
extern const ringtone_note app_tone_hfp_answer[];
extern const ringtone_note app_tone_hfp_hangup[];
extern const ringtone_note app_tone_hfp_mute_active[];
extern const ringtone_note app_tone_hfp_mute_inactive[];
extern const ringtone_note app_tone_hfp_talk_long_press[];
extern const ringtone_note app_tone_pairing[];
extern const ringtone_note app_tone_paired[];
extern const ringtone_note app_tone_pairing_deleted[];
extern const ringtone_note app_tone_volume[];
extern const ringtone_note app_tone_volume_limit[];
extern const ringtone_note app_tone_error[];
extern const ringtone_note app_tone_battery_empty[];
extern const ringtone_note app_tone_power_on[];
extern const ringtone_note app_tone_power_off[];
extern const ringtone_note app_tone_paging_reminder[];
extern const ringtone_note app_tone_peer_pairing[];
extern const ringtone_note app_tone_peer_pairing_error[];

#ifdef INCLUDE_DFU
extern const ledPattern app_led_pattern_dfu[];
extern const ringtone_note app_tone_dfu[];
#endif

#ifdef INCLUDE_AV
extern const ledPattern app_led_pattern_streaming[];
extern const ledPattern app_led_pattern_streaming_aptx[];
extern const ringtone_note app_tone_av_connect[];
extern const ringtone_note app_tone_av_disconnect[];
extern const ringtone_note app_tone_av_remote_control[];
extern const ringtone_note app_tone_av_connected[];
extern const ringtone_note app_tone_av_disconnected[];
extern const ringtone_note app_tone_av_link_loss[];
#endif
//!@}

/*! \brief Play button held/press connect tone */
#define appUiButton() \
    appUiPlayTone(app_tone_button)

/*! \brief Play button held/press connect tone, 2 beeps */
#define appUiButton2() \
    appUiPlayTone(app_tone_button_2)

/*! \brief Play button held/press connect tone, 3 beeps */
#define appUiButton3() \
    appUiPlayTone(app_tone_button_3)

/*! \brief Play button held/press connect tone, 4 beeps */
#define appUiButton4() \
    appUiPlayTone(app_tone_button_4)

/*! \brief Play DFU button held tone */
#define appUiButtonDfu() \
    appUiPlayTone(app_tone_button_dfu)

/*! \brief Play factory reset button held tone */
#define appUiButtonFactoryReset() \
    appUiPlayTone(app_tone_button_factory_reset)

/*! \brief Play HFP connect tone */
#define appUiHfpConnect() \
    appUiPlayTone(app_tone_hfp_connect)
 
/*! \brief Play HFP voice dial tone */
#define appUiHfpVoiceDial() \
    appUiPlayTone(app_tone_hfp_voice_dial)

/*! \brief Play HFP voice dial disable tone */
#define appUiHfpVoiceDialDisable() \
    appUiPlayTone(app_tone_hfp_voice_dial_disable)

/*! \brief Play HFP last number redial tone */
#define appUiHfpLastNumberRedial() \
    /* Tone already played by appUiHfpTalkLongPress() */

/*! \brief Play HFP answer call tone */
#define appUiHfpAnswer() \
    appUiPlayTone(app_tone_hfp_answer)

/*! \brief Play HFP reject call tone */
#define appUiHfpReject() \
    /* Tone already played by appUiHfpTalkLongPress() */

/*! \brief Play HFP hangup call tone */
#define appUiHfpHangup() \
    appUiPlayTone(app_tone_hfp_hangup)

/*! \brief Play HFP transfer call tone */
#define appUiHfpTransfer() \
    /* Tone already played by appUiHfpTalkLongPress() */

/*! \brief Play HFP volume down tone */
#define appUiHfpVolumeDown() \
    appUiPlayToneNotQueueable(app_tone_volume)

/*! \brief Play HFP volume up tone */
#define appUiHfpVolumeUp() \
    appUiPlayToneNotQueueable(app_tone_volume)

/*! \brief Play HFP volume limit reached tone */
#define appUiHfpVolumeLimit() \
    appUiPlayToneNotQueueable(app_tone_volume_limit)

/*! \brief Play HFP SLC connected prompt */
#define appUiHfpConnected(silent) \
    { if (!(silent)) appUiPlayPrompt(PROMPT_CONNECTED); }

/*! \brief Play HFP SLC disconnected prompt */
#define appUiHfpDisconnected() \
    appUiPlayPrompt(PROMPT_DISCONNECTED)

/*! \brief Play HFP SLC link loss tone */
#define appUiHfpLinkLoss() \
    appUiPlayTone(app_tone_hfp_link_loss)

/*! \brief Play HFP ring indication tone */
#define appUiHfpRing(caller_id) \
    appUiPlayTone(app_tone_hfp_ring)

/*! \brief Handle caller ID */
#define appUiHfpCallerId(caller_number, size_caller_number, caller_name, size_caller_name)
    /* Add text to speech call here */

/*! \brief Play HFP SCO connected tone */
#define appUiHfpScoConnected() \
    appUiPlayTone(app_tone_hfp_sco_connected)

/*! \brief Play HFP SCO disconnected tone */
#define appUiHfpScoDisconnected() \
    appUiPlayTone(app_tone_hfp_sco_disconnected)

/*! \brief Show HFP incoming call LED pattern */
#define appUiHfpCallIncomingActive() \
    appLedSetPattern(app_led_pattern_call_incoming, LED_PRI_HIGH)

/*! \brief Cancel HFP incoming call LED pattern */
#define appUiHfpCallIncomingInactive() \
    appLedStopPattern(LED_PRI_HIGH)
    
/*! \brief Show HFP call active LED pattern */
#define appUiHfpCallActive() \
    appLedSetPattern(app_led_pattern_sco, LED_PRI_HIGH)

/*! \brief Show HFP call imactive LED pattern */
#define appUiHfpCallInactive() \
    appLedStopPattern(LED_PRI_HIGH)

/*! \brief Play HFP mute active tone */
#define appUiHfpMuteActive() \
    appUiPlayTone(app_tone_hfp_mute_active)

/*! \brief Play HFP mute inactive tone */
#define appUiHfpMuteInactive() \
    appUiPlayTone(app_tone_hfp_mute_inactive)

/*! \brief Play HFP mute reminder tone */
#define appUiHfpMuteReminder() \
    appUiPlayTone(app_tone_hfp_mute_reminder)

/*! \brief Play HFP SCO un-encrypted tone */
#define appUiHfpScoUnencryptedReminder() \
    appUiPlayTone(app_tone_hfp_sco_unencrypted_reminder)

/*! \brief Handle UI changes for HFP state change */
#define appUiHfpState(state) \
    /* Add any HFP state indication here */

/*! \brief Play HFP talk button long press tone */
#define appUiHfpTalkLongPress() \
    appUiPlayTone(app_tone_hfp_talk_long_press)

#ifdef INCLUDE_AV
/*! \brief Play AV connect tone */
#define appUiAvConnect() \
    appUiPlayTone(app_tone_av_connect)

/*! \brief Play AV disconnect tone */
#define appUiAvDisconnect() \
    appUiPlayTone(app_tone_av_disconnect)

/*! \brief Play AV volume down tone */
#define appUiAvVolumeDown() \
    appUiPlayToneNotQueueable(app_tone_volume)

/*! \brief Play AV volume up tone */
#define appUiAvVolumeUp() \
   appUiPlayToneNotQueueable(app_tone_volume)

/*! \brief Play AV volume limit reached tone */
#define appUiAvVolumeLimit() \
    appUiPlayToneNotQueueable(app_tone_volume_limit)

/*! \brief Play AVRCP remote control tone */
#define appUiAvRemoteControl() \
    appUiPlayToneNotQueueable(app_tone_av_remote_control)

/*! \brief Play AV connected prompt */
#define appUiAvConnected(silent) \
    { if (!(silent)) appUiPlayPrompt(PROMPT_CONNECTED);}

/*! \brief Play AV peer connected indication */
#define appUiAvPeerConnected(silent) \
    { if (!(silent)) appUiPlayTone(app_tone_av_connected);}

/*! \brief Play AV disconnected prompt */
#define appUiAvDisconnected() \
    appUiPlayPrompt(PROMPT_DISCONNECTED)

/*! \brief Play AV link-loss tone */
#define appUiAvLinkLoss() \
    appUiPlayTone(app_tone_av_link_loss)

/*! \brief Show AV streaming active LED pattern */
#define appUiAvStreamingActive() \
    (appLedSetPattern(app_led_pattern_streaming, LED_PRI_MED))

/*! \brief Show AV APIX streaming active LED pattern */
#define appUiAvStreamingActiveAptx() \
    ( appLedSetPattern(app_led_pattern_streaming_aptx, LED_PRI_MED))

/*! \brief Cancel AV SBC/MP3 streaming active LED pattern */
#define appUiAvStreamingInactive() \
    (appLedStopPattern(LED_PRI_MED))
    
/*! \brief Handle UI changes for AV state change */
#define appUiAvState(state) \
    /* Add any AV state indication here */    
#endif

/*! \brief Battery OK, cancel any battery filter */
#define appUiBatteryOk() \
    appLedCancelFilter(0)

/*! \brief Enable battery low filter */
#define appUiBatteryLow() \
    appLedSetFilter(app_led_filter_battery_low, 0)

/*! \brief Play tone and show LED pattern for battery critical status */
#define appUiBatteryCritical() \
    { appLedSetPattern(app_led_pattern_battery_empty, LED_PRI_EVENT); \
      appUiPlayTone(app_tone_battery_empty); }

/*! \brief Start paging reminder */
#define appUiPagingStart() \
    MessageSendLater(appGetUiTask(), APP_INTERNAL_UI_CONNECTING_TIMEOUT, NULL, D_SEC(APP_UI_CONNECTING_TIME))

/*! \brief Stop paging reminder */
#define appUiPagingStop() \
    MessageCancelFirst(appGetUiTask(), APP_INTERNAL_UI_CONNECTING_TIMEOUT)

/*! \brief Play paging reminder tone */
#define appUiPagingReminder() \
    { appUiPlayTone(app_tone_paging_reminder); \
      MessageSendLater(appGetUiTask(), APP_INTERNAL_UI_CONNECTING_TIMEOUT, NULL, D_SEC(APP_UI_CONNECTING_TIME)); }

/*! \brief Show LED pattern for idle headset */
#define appUiIdleActive() \
    appLedSetPattern(app_led_pattern_idle, LED_PRI_LOW)

/*! \brief Show LED pattern for connected headset */
#define appUiIdleConnectedActive() \
    appLedSetPattern(app_led_pattern_idle_connected, LED_PRI_LOW)

/*! \brief Cancel LED pattern for idle/connected headset */
#define appUiIdleInactive() \
    appLedStopPattern(LED_PRI_LOW)

/*! \brief Play pairing start tone */
#define appUiPairingStart() \
    appUiPlayTone(app_tone_pairing)

/*! \brief Show pairing active LED pattern and play prompt */
#define appUiPairingActive(is_user_initiated) \
do \
{  \
    appUiPlayPrompt(PROMPT_PAIRING); \
    appLedSetPattern(app_led_pattern_pairing, LED_PRI_MED); \
} while(0)

/*! \brief Cancel pairing active LED pattern */
#define appUiPairingInactive(is_user_initiated) \
    appLedStopPattern(LED_PRI_MED)

/*! \brief Play pairing complete prompt */
#define appUiPairingComplete() \
    appUiPlayPrompt(PROMPT_PAIRING_SUCCESSFUL)

/*! \brief Play pairing failed prompt */
#define appUiPairingFailed() \
    appUiPlayPrompt(PROMPT_PAIRING_FAILED)

/*! \brief Play pairing deleted tone */
#define appUiPairingDeleted() \
    { appUiPlayTone(app_tone_pairing_deleted); \
      appLedSetPattern(app_led_pattern_pairing_deleted, LED_PRI_EVENT); }

/*! \brief Play inquiry active tone, show LED pattern */
#define appUiPeerPairingActive(is_user_initiated) \
    { if (is_user_initiated) \
        appUiPlayTone(app_tone_peer_pairing); \
      appLedSetPattern(app_led_pattern_peer_pairing, LED_PRI_MED); \
      MessageSendLater(appGetUiTask(), APP_INTERNAL_UI_INQUIRY_TIMEOUT, NULL, D_SEC(APP_UI_INQUIRY_REMINDER_TIME)); }

/*! \brief Play inquiry active reminder tone */
#define appUiPeerPairingReminder() \
    { appUiPlayTone(app_tone_peer_pairing_reminder); \
      MessageSendLater(appGetUiTask(), APP_INTERNAL_UI_INQUIRY_TIMEOUT, NULL, D_SEC(APP_UI_INQUIRY_REMINDER_TIME)); }
		
/*! \brief Cancel inquiry active LED pattern */
#define appUiPeerPairingInactive() \
    { appLedStopPattern(LED_PRI_MED); \
      MessageCancelFirst(appGetUiTask(), APP_INTERNAL_UI_INQUIRY_TIMEOUT); }

/*! \brief Play inquiry error tone */
#define appUiPeerPairingError() do { \
      appUiPlayTone(app_tone_peer_pairing_error); \
      appLedSetPattern(app_led_pattern_error, LED_PRI_EVENT); \
      } while (0)

#ifdef INCLUDE_DFU
/*! \brief Play DFU active tone, show LED pattern */
#define appUiDfuActive() \
    { appUiPlayTone(app_tone_dfu); \
      appLedEnable(TRUE); \
      appLedSetPattern(app_led_pattern_dfu, LED_PRI_LOW); }

/*! \brief Play DFU active tone, show LED pattern */
#define appUiDfuRequested() do { \
      appUiPlayTone(app_tone_dfu); \
      appLedSetPattern(app_led_pattern_dfu, LED_PRI_EVENT); \
      } while(0)

#endif

#ifdef INCLUDE_CHARGER
/*! \brief Charger connected */
#define appUiChargerConnected()
#endif

#ifdef INCLUDE_CHARGER
/*! \brief Charger disconnected, cancel any charger filter */
#define appUiChargerDisconnected() \
    appLedCancelFilter(1)
#endif

#ifdef INCLUDE_CHARGER
/*! \brief Charger charging, enable charging filter */
#define appUiChargerChargingLow() \
    appLedSetFilter(app_led_filter_charging_low, 1)
#endif

#ifdef INCLUDE_CHARGER
/*! \brief Charger charging, enable charging filter */
#define appUiChargerChargingOk() \
    appLedSetFilter(app_led_filter_charging_ok, 1)
#endif

#ifdef INCLUDE_CHARGER
/*! \brief Charger charging complete, enable charging complete filter */
#define appUiChargerComplete() \
    appLedSetFilter(app_led_filter_charging_complete, 1)
#endif

extern void appUiInit(void);
extern void appUiPlayToneCore(const ringtone_note *tone, bool interruptible, bool queueable,
                              uint16 *client_lock, uint16 client_lock_mask);
extern void appUiPlayPromptCore(voicePromptName prompt, bool interruptible, bool queueable,
                                uint16 *client_lock, uint16 client_lock_mask);
extern void appUiError(void);
extern void appUiHfpError(bool silent);
extern void appUiAvError(bool silent);
extern void appUiPowerOn(void);
extern void appUiPowerOff(uint16 *lock, uint16 lock_mask);
extern void appUiSleep(void);
/*! \brief Play a tone to completion */
#define appUiPlayTone(tone) appUiPlayToneCore(tone, FALSE, TRUE, NULL, 0)
/*! \brief Play a tone allowing another tone/prompt/event to interrupt (stop) this tone
     before completion. */
#define appUiPlayToneInterruptible(tone) appUiPlayToneCore(tone, TRUE, TRUE, NULL, 0)
/*! \brief Play a tone only if it's not going to be queued. */
#define appUiPlayToneNotQueueable(tone) appUiPlayToneCore(tone, FALSE, FALSE, NULL, 0)
/*! \brief Play a tone to completion. mask bits will be cleared in lock
    when the tone completes, or if it is not played at all. */
#define appUiPlayToneClearLock(tone, lock, mask) appUiPlayToneCore(tone, FALSE, TRUE, lock, mask)
/*! \brief Play a tone allowing another tone/prompt/event to interrupt (stop) this tone
     before completion. mask bits will be cleared in lock when the tone completes or
     is interrupted, or if it is not played at all. */
#define appUiPlayToneInterruptibleClearLock(tone, lock, mask) appUiPlayToneCore(tone, TRUE, TRUE, lock, mask)

/*! \brief Play a prompt to completion */
#define appUiPlayPrompt(prompt) appUiPlayPromptCore(prompt, FALSE, TRUE, NULL, 0)
/*! \brief Play a prompt allowing another tone/prompt/event to interrupt (stop) this prompt
     before completion. */
#define appUiPlayPromptInterruptible(prompt) appUiPlayPromptCore(prompt, TRUE, TRUE, NULL, 0)
/*! \brief Play a prompt to completion. mask bits will be cleared in lock
    when the prompt completes, or if it is not played at all. */
#define appUiPlayPromptClearLock(prompt, lock, mask) appUiPlayPromptCore(prompt, FALSE, TRUE, lock, mask)
/*! \brief Play a prompt allowing another tone/prompt/event to interrupt (stop) this prompt
     before completion. mask bits will be cleared in lock when the prompt completes or
     is interrupted, or if it is not played at all. */
#define appUiPlayPromptInterruptibleClearLock(prompt, lock, mask) appUiPlayPromptCore(prompt, TRUE, TRUE, lock, mask)

#endif
