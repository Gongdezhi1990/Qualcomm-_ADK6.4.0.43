/****************************************************************************
Copyright (c) 2010 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gaia.c

DESCRIPTION
    Interface with the Generic Application Interface Architecture library
*/

#include "sink_gaia.h"

#ifdef ENABLE_GAIA
#include <vm.h>
#include <led.h>
#include <audio_plugin_voice_prompts_variants.h>
#include <audio.h>
#include <boot.h>
#include <byte_utils.h>
#include <csrtypes.h>
#include <gain_utils.h>

#include "sink_a2dp.h"
#include "sink_configmanager.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_server_ias.h"
#include "sink_gatt_server_lls.h"
#include "sink_hfp_data.h"
#include "sink_leds.h"
#include "sink_led_manager.h"
#include "sink_partymode.h"
#include "sink_tones.h"
#include "sink_audio_prompts.h"
#include "sink_buttons.h"
#include "sink_volume.h"
#include "sink_speech_recognition.h"
#include "sink_device_id.h"
#include "sink_sc.h"
#include "sink_statemanager.h"
#include "sink_slc.h"
#include "sink_link_policy.h"
#include "sink_audio_indication.h"
#include "sink_upgrade.h"
#include "sink_music_processing.h"
/* Include config store and definition headers */
#include "config_definition.h"
#include"sink_gaia_config_def.h"
#include "sink_powermanager.h"
#include "sink_usb.h"
#include "sink_wired.h"
#include "sink_main_task.h"
#include "sink_malloc_debug.h"
#include "sink_private_data.h"
#include "sink_va_if.h"
#include <config_store.h>

#include "sink_ble.h"

#ifdef ENABLE_PBAP        
#include "sink_pbap.h"
#endif

#ifdef ENABLE_MAPC
#include "sink_mapc.h"
#endif



#ifdef DEBUG_GAIA
#define GAIA_DEBUG(x) DEBUG(x)
#else
#define GAIA_DEBUG(x)
#endif

#ifdef CVC_PRODTEST
/* The prototype of exit is not in <stdlib.h> so define it. */
void exit(int status);
#endif

#define GAIA_AUDIO_GAIN_PAYLOAD_SHORT (6)
#define GAIA_AUDIO_GAIN_PAYLOAD_LONG (80)
#define GAIA_AUDIO_GAIN_MAX_TONE (161)  /* WAV_RING_TONE_TOP - 1, not quite right  */
#define GAIA_AUDIO_GAIN_MAX_GAIN (22)

#define GAIA_VOL_MAP_ELEMENTS (5)

/*  Digital Volume Control limits imposed by Sink Configuration Tool */
#define GAIA_VOL_MAX_STEPS (500)
#define GAIA_VOL_MAX_TRIM (720)     /* in 1/60 dB units */
#define GAIA_DEFAULT_VOL_MAX    (15)

#define NUM_BANDS_BANK (1)
#define NUM_BANDS_BAND (0)
#define NUM_BANDS_PARAM_TYPE (0)
#define MAX_EQ_BANDS (5)

typedef enum
{
    USER_EQ_NUM_PARAMS_HI_OFFSET,
    USER_EQ_NUM_PARAMS_LO_OFFSET
} gaia_cmd_user_eq_num_params_offset;

typedef enum
{
    SET_USER_EQ_PARAM_HI_OFFSET,
    SET_USER_EQ_PARAM_LO_OFFSET,
    SET_USER_EQ_VALUE_HI_OFFSET,
    SET_USER_EQ_VALUE_LO_OFFSET,
    SET_USER_EQ_RECALC
} gaia_cmd_set_eq_param_payload_offset;

typedef enum
{
    SET_USER_EQ_GROUP_PARAM_RESERVED_HI_OFFSET,
    SET_USER_EQ_GROUP_PARAM_RESERVED_LO_OFFSET
} gaia_cmd_set_eq_group_param_payload_reserved_offset;

typedef enum
{
    USER_EQ_PARAM_HI_OFFSET,
    USER_EQ_PARAM_LO_OFFSET,
    USER_EQ_VALUE_HI_OFFSET,
    USER_EQ_VALUE_LO_OFFSET
} gaia_cmd_user_eq_param_payload_offset;


typedef enum
{
    USER_EQ_NUM_PARAMS_OFFSET,
    USER_EQ_PARAM_OFFSET,
    USER_EQ_VALUE_OFFSET
} gaia_response_16_offset;


#define GAIA_NUM_BYTES_PER_SET_GET_HDR (2)
#define GAIA_NUM_BYTES_CMD_PER_PARAM (4)
#define GAIA_NUM_BYTES_SET_GROUP_PARAM_RESERVED (2)

#define GAIA_NUM_BYTES_PER_WORD (2)
#define GAIA_NUM_WORDS_PER_SET_GET_HDR ((GAIA_NUM_BYTES_PER_SET_GET_HDR) / GAIA_NUM_BYTES_PER_WORD)
#define GAIA_NUM_WORDS_PER_PARAM ((GAIA_NUM_BYTES_CMD_PER_PARAM) / (GAIA_NUM_BYTES_PER_WORD))

#define GAIA_GET_PARAM_PAYLOAD_SIZE (2)
#define MIN_GAIA_GET_GROUP_PARAM_PAYLOAD_SIZE (2)

#define GAIA_SET_PARAM_PAYLOAD_SIZE (5)
#define MIN_GAIA_SET_GROUP_PARAM_PAYLOAD_SIZE (8)

#define USER_BANK 1
#define PRE_GAIN_BAND 0
#define PRE_GAIN_PARAM_NUMBER 1
#define NUMBER_OF_PARAMS_PER_BAND 4

#define GAIA_MAX_BANDS_PER_EQ_BANK 5

#ifdef CVC_PRODTEST
    #define CVC_PRODTEST_PASS           0x0001
    #define CVC_PRODTEST_FAIL           0x0002
#endif

/* This is the binary format that the following GAIA commands expect:

   GAIA_COMMAND_SET_VOLUME_CONFIGURATION
   GAIA_COMMAND_GET_VOLUME_CONFIGURATION

   The sink stores this data in a different format across multiple sink
   modules. So, Gaia must convert the data from this format into the format
   that the sink expects.
*/
typedef struct
{
    volume_group_config_t group_config[audio_output_group_all];

    int16 device_trim_master;   /* TWS master device trim gain */
    int16 device_trim_slave;    /* TWS slave device trim gain */
    int16 device_trim_change;   /* TWS device trim step change */
    int16 device_trim_min;      /* TWS device trim minimum gain */
    int16 device_trim_max;      /* TWS device trim maximum gain */

} dsp_volume_user_config;

/* Valid values of Volume Parameter Number.

   These values must match the indices of the parameters in
   a dsp_volume_user_config structure.
*/
typedef enum
{
    gaia_vol_no_of_steps_main,           /* number of steps of volume change permitted */
	gaia_vol_volume_knee_value_1_main,   /* volume point at which curve of dB conversion changes */
	gaia_vol_volume_knee_value_2_main,   /* volume point at which curve of dB conversion changes */
	gaia_vol_dB_knee_value_1_main,       /* dB value for point at which curve of dB conversion changes */
	gaia_vol_dB_knee_value_2_main,       /* dB value for point at which curve of dB conversion changes */
	gaia_vol_dB_max_main,                /* dB value at maximum volume level */
	gaia_vol_dB_min_main,                /* dB value at minimum volume level */

	gaia_vol_no_of_steps_aux,           /* number of steps of volume change permitted */
	gaia_vol_volume_knee_value_1_aux,   /* volume point at which curve of dB conversion changes */
	gaia_vol_volume_knee_value_2_aux,   /* volume point at which curve of dB conversion changes */
	gaia_vol_dB_knee_value_1_aux,       /* dB value for point at which curve of dB conversion changes */
	gaia_vol_dB_knee_value_2_aux,       /* dB value for point at which curve of dB conversion changes */
	gaia_vol_dB_max_aux,                /* dB value at maximum volume level */
	gaia_vol_dB_min_aux,                /* dB value at minimum volume level */

    gaia_vol_device_trim_master,    /* TWS master device trim gain */
    gaia_vol_device_trim_slave,     /* TWS slave device trim gain */
    gaia_vol_device_trim_change,    /* TWS device trim step change */
    gaia_vol_device_trim_min,       /* TWS device trim minimum gain */
    gaia_vol_device_trim_max,       /* TWS device trim maximum gain */

    gaia_vol_param_top                  /* one step beyond greatest valid value */
} gaia_vol_param_t;

#define GAIA_VOL_NUM_PARAMETERS (gaia_vol_param_top)
STATIC_ASSERT(GAIA_VOL_NUM_PARAMETERS == (sizeof (dsp_volume_user_config)/sizeof(uint16)), gaia_vol);

/* Valid values of Timer Parameter Number.

   Used by:
   GAIA_COMMAND_SET_TIMER_CONFIGURATION
   GAIA_COMMAND_GET_TIMER_CONFIGURATION

   The order of timer parameters is the same as the old Timeouts_t structure.
   However, the Timeouts_t structure does not exist any more but the param
   order must be kept the same to avoid breaking the binary compatibility of
   the GAIA commmands.
*/
typedef enum
{
    gaia_timer_auto_switch_off_time_s,
    gaia_timer_limbo_timeout_s,
    gaia_timer_network_service_indicator_repeat_time_s,
    gaia_timer_disable_power_off_after_power_on_time_s,
    gaia_timer_pairmode_timeout_s,
    gaia_timer_mute_remind_time_s,
    gaia_timer_connectable_timeout_s,
    gaia_timer_pair_mode_timeout_if_pdl_s,
    gaia_timer_reconnection_attempts,       /* number of times to try and reconnect before giving up */
    gaia_timer_encryption_refresh_timeout_m,
    gaia_timer_inquiry_timeout_s,
    gaia_timer_second_ag_connect_delay_time_s,

    gaia_timer_missed_call_indicate_time_s,     /* the period in second between two indications */
    gaia_timer_missed_call_indicate_attempts,   /* number of times to indicate before stopping indication */

    gaia_timer_a2dp_link_loss_reconnection_time_s,  /* the amount of time in seconds to attempt to reconnect a2dp */
    gaia_timer_speech_rec_repeat_time_ms,           /* the between voice prompts asr restarts */
    gaia_timer_store_current_sink_volume_and_source_timeout_s, /* the time in seconds elapsed after the last volumeup/down or source button was pressed,
                                                       to store the volume information into ps store*/
    gaia_timer_wired_audio_connected_power_off_timeout_s, /* the time in seconds elapsed before the device is powered off after the
                                                                             wired audio is connected */
    gaia_timer_defrag_check_timer_s,  /* the interval in seconds to check the status of the ps and defragment if necessary */

    gaia_timer_audio_amp_power_down_timeout_in_limbo_s, /* the period in seconds for which the system waits before powering down audio amplifier for sound bar when in limbo state  */

    gaia_timer_immediate_alert_timer_s,         /* the period in seconds for which the system waits before playing alert tone again */
    gaia_timer_immediate_alert_stop_timeout_s,  /* the period in seconds for which the system waits before stoppingthe alert */

    gaia_timer_link_loss_timer_s,               /* the period in seconds for which the system waits before palying alert tone again */
    gaia_timer_link_loss_alert_stop_timeout_s,  /* the period in seconds for which the system waits before stoppingthe alert */

    gaia_timer_audio_amp_unmute_time_ms,   /* time to power up amp before unmuting (milliseconds) */
    gaia_timer_audio_amp_mute_time_ms,     /* time to mute before powering down amp (milliseconds) */

    gaia_timer_party_mode_music_timeout_s,          /* time in seconds for a newly connected device given, to start playing a track before disconnecting it */
    gaia_timer_party_mode_stream_resume_timeout_s,  /* the period in seconds after which partymode ag will be disconnected if it does not resume audio streaming when sink un-pauses it*/

    gaia_timer_bredr_authenticated_payload_timeout_s,   /* maximum amount of time specified between packets authenticated by a mic(seconds) on a br/edr link*/
    gaia_timer_le_authenticated_payload_timeout_s,      /* maximum amount of time specified between packets authenticated by a mic(seconds) on a le link*/

    gaia_timer_max
} gaia_timer_param_t;

#define GAIA_TIMER_NUM_PARAMETERS (gaia_timer_max)


/* Sink GAIA global data */
typedef struct __sink_gaia_global_data_t
{
    GAIA_TRANSPORT *gaia_transport;

    pio_common_allbits pio_change_mask;
    pio_common_allbits pio_old_state;
    ringtone_note *alert_tone;

    unsigned notify_ui_event:1;
    unsigned notify_charger_connection:1;
    unsigned notify_battery_charged:1;
    unsigned notify_speech_rec:1;
    unsigned dfu_boot_status:2;
    unsigned num_mapped_pios:6;
    unsigned activate_peq_done:1;
    unsigned gaia_remain_connected:1;
    unsigned gatt_upgrade_in_progress:1;
    unsigned unused:1;
}sink_gaia_global_data_t;

/* GAIA global data structure instance */
static sink_gaia_global_data_t gaia_gdata;

/*  Gaia-global data stored in app-allocated structure */
#define GAIA_GDATA gaia_gdata

/* Static functions used to map GAIA PIOS (32 bit) to device PIOS
   using a translation table in the GAIA configuration data.
 */
static uint32 gaia_translate_allbits(pio_common_allbits *allbits_mask);
static gaia_pio_translation_config_def_t *gaia_config_get_pio_map(void);
static void gaia_config_release_pio_map(void);
static void gaia_get_pio_mapping(void);

static bool gaia_is_acknowledgement(uint16 command_id);
static void gaia_read_remain_connected(void);

/*************************************************************************
NAME
    initGaia

DESCRIPTION
    Initialize GAIA system by allocating memory for GAIA global structure
*/
void initGaia(void)
{
    /* Initialise GAIA global structure */
    memset(&GAIA_GDATA, 0, sizeof(GAIA_GDATA));

    /* Retrieve the number of PIOS that GAIA can use, updating GAIA_GDATA */
    gaia_get_pio_mapping();

    gaia_set_activate_peq_done(FALSE);
    
    /* Read the default value into GAIA_GDATA */
    gaia_read_remain_connected();
    gaiaSetGattUpgradeInProgress(FALSE);
}

/****************************************************************************
NAME
	gaia_get_transport

DESCRIPTION
    Get the GAIA transport
*/
GAIA_TRANSPORT* gaia_get_transport(void)
{
    return GAIA_GDATA.gaia_transport;
}

/****************************************************************************
NAME
	gaia_set_transport

DESCRIPTION
    Set the GAIA transport
*/
void gaia_set_transport(GAIA_TRANSPORT* transport)
{
    GAIA_GDATA.gaia_transport = transport;
}

/****************************************************************************
NAME
    gaia_get_pio_change_mask32

DESCRIPTION
    Get the GAIA pio change mask, translating to the GAIA 32 bit bitmask.
*/
uint32 gaia_get_pio_change_mask32(void)
{
    pio_common_allbits mask;

    if (!gaia_get_pio_change_mask(&mask))
    {
        return 0;
    }

    return gaia_translate_allbits(&mask);
}

/****************************************************************************
NAME
	gaia_get_pio_change_mask

DESCRIPTION
    Get the GAIA pio change mask
*/
bool gaia_get_pio_change_mask(pio_common_allbits *mask)
{
    if (mask)
    {
        *mask = GAIA_GDATA.pio_change_mask;
        return !PioCommonBitsIsEmpty(mask);
}

    /* Return "empty" */
    return FALSE;
}

/****************************************************************************
NAME
	gaia_set_pio_change_mask

DESCRIPTION
    Set the GAIA pio change mask, converting the gaia 32 bit mask to a mask
    representing device PIOs
*/
void gaia_set_pio_change_mask(uint32 mask)
{
    if (GAIA_GDATA.num_mapped_pios)
    {
        gaia_pio_translation_config_def_t *table = gaia_config_get_pio_map();
        int index = 0;
        pio_common_allbits allbits;

        PioCommonBitsInit(&allbits);

        /* For every bit in the mask that matches a PIO */
        while (mask && (index < GAIA_GDATA.num_mapped_pios))
        {
            if (mask & 0x1)
            {
                unsigned pio;

                pio = table->gaia_pio_array[index].pio;
                PioCommonBitsSetBit(&allbits, pio);
}

            /* Move to next bit and translation entry */
            mask = mask >> 1;
            index ++;
        }

        GAIA_GDATA.pio_change_mask = allbits;
        gaia_config_release_pio_map();
    }
}

/****************************************************************************
NAME
    gaia_translate_allbits

DESCRIPTION
    Translate an allbits mask into a uint32 value matching the translated
    GAIA bitmask.
*/
static uint32 gaia_translate_allbits(pio_common_allbits *allbits_mask)
{
    uint32 result = 0;

    /* No need to do anything if no PIOs */
    if (GAIA_GDATA.num_mapped_pios)
    {
        gaia_pio_translation_config_def_t *table = gaia_config_get_pio_map();
        int index = GAIA_GDATA.num_mapped_pios;

        /* Loop from the last entry, setting the bottom bit if the matching
         * device PIO for the entry is set, and shifting to the correct
         * position as we go */
        while (index-- > 0)
        {
            unsigned pio = table->gaia_pio_array[index].pio;
            result <<= 1;

            if (PioCommonBitsBitIsSet(allbits_mask, pio))
            {
                result |= 0x1;
            }
        }
        gaia_config_release_pio_map();
    }

    return result;
}


/****************************************************************************
NAME
    gaia_clear_pio_change_mask

DESCRIPTION
    Set the GAIA pio change mask to "no bits"
*/
void gaia_clear_pio_change_mask(void)
{
    gaia_set_pio_change_mask(0);
}

/****************************************************************************
NAME
	gaia_get_pio_old_state

DESCRIPTION
    Get the GAIA pio old state into passed state and return TRUE if non-empty.
*/
bool gaia_get_pio_old_state(pio_common_allbits *state)
{
    if (state)
    {
        *state = GAIA_GDATA.pio_old_state;
        return !PioCommonBitsIsEmpty(state);
    }

    /* Return "empty" */
    return FALSE;
}

/****************************************************************************
NAME
	gaia_set_pio_old_state

DESCRIPTION
    Set the GAIA pio old state
*/
void gaia_set_pio_old_state(pio_common_allbits *state)
{
    GAIA_GDATA.pio_old_state = *state;
}

/****************************************************************************
NAME
	gaia_get_alert_tone

DESCRIPTION
    Get the GAIA alert tone
*/
ringtone_note* gaia_get_alert_tone(void)
{
    return GAIA_GDATA.alert_tone;
}

/****************************************************************************
NAME
	gaia_set_alert_tone

DESCRIPTION
    Set the GAIA alert tone
*/
void gaia_set_alert_tone(ringtone_note* tone)
{
    GAIA_GDATA.alert_tone = tone;
}

/****************************************************************************
NAME
	gaia_get_notify_ui_event

DESCRIPTION
    Get the GAIA notify ui event
*/
uint8 gaia_get_notify_ui_event(void)
{
    return GAIA_GDATA.notify_ui_event;
}

/****************************************************************************
NAME
	gaia_set_notify_ui_event

DESCRIPTION
    Set the GAIA notify ui event
*/
void gaia_set_notify_ui_event(uint8 event)
{
    GAIA_GDATA.notify_ui_event = event;
}

/****************************************************************************
NAME
	gaia_get_notify_charger_connection

DESCRIPTION
    Get the GAIA notify charger connection
*/
uint8 gaia_get_notify_charger_connection(void)
{
    return GAIA_GDATA.notify_charger_connection;
}

/****************************************************************************
NAME
	gaia_set_notify_charger_connection

DESCRIPTION
    Set the GAIA notify charger connection
*/
void gaia_set_notify_charger_connection(uint8 connection)
{
    GAIA_GDATA.notify_charger_connection = connection;
}

/****************************************************************************
NAME
	gaia_get_notify_battery_charged

DESCRIPTION
    Get the GAIA notify battery charged
*/
uint8 gaia_get_notify_battery_charged(void)
{
    return GAIA_GDATA.notify_battery_charged;
}

/****************************************************************************
NAME
	gaia_set_notify_battery_charged

DESCRIPTION
    Set the GAIA notify battery charged
*/
void gaia_set_notify_battery_charged(uint8 charged)
{
    GAIA_GDATA.notify_battery_charged = charged;
}

/****************************************************************************
NAME
	gaia_get_notify_speech_rec

DESCRIPTION
    Get the GAIA notify speech recognization
*/
uint8 gaia_get_notify_speech_rec(void)
{
    return GAIA_GDATA.notify_speech_rec;
}

/****************************************************************************
NAME
	gaia_set_notify_speech_rec

DESCRIPTION
    Set the GAIA notify speech recognization
*/
void gaia_set_notify_speech_rec(uint8 rec)
{
    GAIA_GDATA.notify_speech_rec = rec;
}

/****************************************************************************
NAME
	gaia_get_dfu_boot_status

DESCRIPTION
    Get the GAIA dfu boot status
*/
uint8 gaia_get_dfu_boot_status(void)
{
    return GAIA_GDATA.dfu_boot_status;
}

/****************************************************************************
NAME
	gaia_set_dfu_boot_status

DESCRIPTION
    Set the GAIA dfu boot status
*/
void gaia_set_dfu_boot_status(uint8 status)
{
    GAIA_GDATA.dfu_boot_status = status;
}

/****************************************************************************
NAME
    gaia_get_activate_peq_done

DESCRIPTION
    Get the activate_peq_done flag
*/
bool gaia_get_activate_peq_done(void)
{
    return GAIA_GDATA.activate_peq_done;
}

/****************************************************************************
NAME
    gaia_set_activate_peq_done

DESCRIPTION
    Set the GAIA activate_peq_done flag
*/
void gaia_set_activate_peq_done(bool status)
{
    GAIA_GDATA.activate_peq_done = status;
}

/****************************************************************************
NAME
	gaia_set_remain_connected

DESCRIPTION
    Set the GAIA remain connected flag
*/
static void gaia_set_remain_connected(bool connected)
{
    GAIA_GDATA.gaia_remain_connected = connected;
}

/****************************************************************************
NAME
	gaia_get_remain_connected

DESCRIPTION
    Get the GAIA remain connected status
*/
bool gaia_get_remain_connected(void)
{
    return GAIA_GDATA.gaia_remain_connected;
}

/****************************************************************************
NAME
    gaiaIsGattUpgradeInProgress

DESCRIPTION
    Get the gatt_upgrade_in_progress flag
    This is enabled only when Upgrade is on BLE/GATT transport
*/
bool gaiaIsGattUpgradeInProgress(void)
{
    return GAIA_GDATA.gatt_upgrade_in_progress;
}

/****************************************************************************
NAME
    gaiaSetGattUpgradeInProgress

DESCRIPTION
    Set the GAIA gatt_upgrade_in_progress flag
    This is enabled only when Upgrade is on BLE/GATT transport
*/
void gaiaSetGattUpgradeInProgress(bool status)
{
    GAIA_DEBUG(("G: gaiaSetGattUpgradeInProgress 0x%x\n", status));

    GAIA_GDATA.gatt_upgrade_in_progress = status;
}


/****************************************************************************
NAME
    gaia_is_acknowledgement
    
DESCRIPTION
    Return TRUE if Command ID represents an acknowledgement
*/
static bool gaia_is_acknowledgement(uint16 command_id)
{
    return (command_id & GAIA_ACK_MASK) != 0;
}

/****************************************************************************
NAME
    gaia_config_get_pio_map

DESCRIPTION
    Load the mapping table, and update the number of entries in GAIA data

    Must be followed by a (matched) call to gaia_config_release_pio_map
*/
static gaia_pio_translation_config_def_t *gaia_config_get_pio_map(void)
{
    gaia_pio_translation_config_def_t *pio_config_data = NULL;
    uint16 size_u16;
    uint16 mapped_pios;

    /* read the existing configuration */
    size_u16 = configManagerGetReadOnlyConfig(GAIA_PIO_TRANSLATION_CONFIG_BLK_ID, (const void **)&pio_config_data);
    mapped_pios = (uint16)((size_u16*sizeof(uint16))/sizeof(gaia_pio_pattern_t));

    if (mapped_pios)
    {
        GAIA_GDATA.num_mapped_pios = mapped_pios;
    }
    return pio_config_data;
}

/****************************************************************************
NAME
    gaia_config_release_pio_map

DESCRIPTION
    Release the pio map grabbed by call to gaia_config_get_pio_map
*/
static void gaia_config_release_pio_map(void)
{
    configManagerReleaseConfig(GAIA_PIO_TRANSLATION_CONFIG_BLK_ID);
}

/****************************************************************************
NAME
    gaia_get_pio_mapping

DESCRIPTION
    Load the number of PIOS that are being mapped for GAIA.
*/
static void gaia_get_pio_mapping(void)
{
    gaia_config_get_pio_map();
    /* Get has read the number of entries, so we can discard immediately */
    gaia_config_release_pio_map();
}


/****************************************************************************
NAME
	gaia_get_enable_session

DESCRIPTION
    Get the GAIA enable session
*/
bool gaia_get_enable_session(void)
{
    gaia_read_only_config_def_t *read_config_data = NULL;
    bool enable = FALSE;
    /* read the existing configuration */
    if(configManagerGetReadOnlyConfig(GAIA_READ_ONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        enable = read_config_data->GaiaEnableSession;
        configManagerReleaseConfig(GAIA_READ_ONLY_CONFIG_BLK_ID);
    }
    return enable;
}

/****************************************************************************
NAME
	gaia_read_remain_connected

DESCRIPTION
    Read the default GAIA remain connected value and update global flag.
*/
static void gaia_read_remain_connected(void)
{
    gaia_read_only_config_def_t *read_config_data = NULL;

    if (configManagerGetReadOnlyConfig(GAIA_READ_ONLY_CONFIG_BLK_ID, (const void **)&read_config_data))
    {
        GAIA_GDATA.gaia_remain_connected = read_config_data->GaiaRemainConnected;
        configManagerReleaseConfig(GAIA_READ_ONLY_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
	gaia_get_dfu_partition

DESCRIPTION
    Get the GAIA dfu partition
*/
uint16 gaia_get_dfu_partition(void)
{
    gaia_read_only_config_def_t *ro_config_data = NULL;
    uint16 result = 0;
    /* read the existing configuration */
    if(configManagerGetReadOnlyConfig(GAIA_READ_ONLY_CONFIG_BLK_ID, (const void **)&ro_config_data))
    {
        result = ro_config_data->dfu_partition;
        configManagerReleaseConfig(GAIA_READ_ONLY_CONFIG_BLK_ID);
    }
    return result;
}

/*************************************************************************
NAME
    gaia_send_packet

DESCRIPTION
    Build and Send a Gaia protocol packet

*/
static void gaia_send_packet(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload)
{
    if(gaia_get_transport()) /* Only attempt to send when transport up */
    {
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = GaiaTransportGetFlags(gaia_get_transport());

    packet_length = GAIA_HEADER_SIZE + payload_length + 2;
    packet = mallocPanic(packet_length);

    if (packet)
    {
        packet_length = GaiaBuildResponse(packet, flags,
                                          vendor_id, command_id,
                                          status, payload_length, payload);

        GaiaSendPacket(gaia_get_transport(), packet_length, packet);
    }
    }
}

/*************************************************************************
NAME
    gaia_send_response

DESCRIPTION
    Build and Send a Gaia acknowledgement packet

*/
void gaia_send_response(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload)
{
    gaia_send_packet(vendor_id, command_id | GAIA_ACK_MASK, status,
                     payload_length, payload);
}

/*************************************************************************
NAME
    gaia_send_response_16

DESCRIPTION
    Build and Send a Gaia acknowledgement packet from a uint16[] payload

*/
void gaia_send_response_16(uint16 command_id, uint16 status,
                          uint16 payload_length, uint16 *payload)
{
    uint16 packet_length;
    uint8 *packet;
    uint8 flags = GaiaTransportGetFlags(gaia_get_transport());

    packet_length = GAIA_HEADER_SIZE + 2 * payload_length + 2;
    packet = mallocPanic(packet_length);

    if (packet)
    {
        packet_length = GaiaBuildResponse16(packet, flags,
                                          GAIA_VENDOR_QTIL, command_id | GAIA_ACK_MASK,
                                          status, payload_length, payload);

        GaiaSendPacket(gaia_get_transport(), packet_length, packet);
    }
}


/*************************************************************************
NAME
    gaia_send_simple_response
    gaia_send_success
    gaia_send_success_payload
    gaia_send_invalid_parameter
    gaia_send_insufficient_resources
    gaia_send_incorrect_state

DESCRIPTION
    Convenience macros for common responses
*/
#define gaia_send_simple_response(command_id, status) \
    gaia_send_response(GAIA_VENDOR_QTIL, command_id, status, 0, NULL)

#define gaia_send_success(command_id) \
    gaia_send_simple_response(command_id, GAIA_STATUS_SUCCESS)

#define gaia_send_success_payload(command_id, payload_len, payload) \
    gaia_send_response(GAIA_VENDOR_QTIL, command_id, GAIA_STATUS_SUCCESS, payload_len, (uint8 *) payload)

#define gaia_send_invalid_parameter(command_id) \
    gaia_send_simple_response(command_id, GAIA_STATUS_INVALID_PARAMETER)

#define gaia_send_insufficient_resources(command_id) \
    gaia_send_simple_response(command_id, GAIA_STATUS_INSUFFICIENT_RESOURCES)

#define gaia_send_incorrect_state(command_id) \
    gaia_send_simple_response(command_id, GAIA_STATUS_INCORRECT_STATE)


/*************************************************************************
NAME
    gaia_send_notification

DESCRIPTION
    Send a Gaia notification packet

*/
static void gaia_send_notification(uint8 event, uint16 payload_length, uint8 *payload)
{
    gaia_send_packet(GAIA_VENDOR_QTIL, GAIA_EVENT_NOTIFICATION, event, payload_length, payload);
}

/*************************************************************************
NAME
    wpack

DESCRIPTION
    Pack an array of 2n uint8s into an array of n uint16s
*/
static void wpack(uint16 *dest, uint8 *src, uint16 n)
{
    while (n--)
    {
        *dest = *src++ << 8;
        *dest++ |= *src++;
    }
}

/*************************************************************************
NAME
    dwunpack

DESCRIPTION
    Unpack a uint32 into an array of 4 uint8s
*/
static void dwunpack(uint8 *dest, uint32 src)
{
    *dest++ = src >> 24;
    *dest++ = (src >> 16) & 0xFF;
    *dest++ = (src >> 8) & 0xFF;
    *dest = src & 0xFF;
}


/*************************************************************************
NAME
    app_event_from_opid

DESCRIPTION
    Translate an AVC operation identifier to an application event code
*/
#ifdef ENABLE_AVRCP
static sinkEvents_t app_event_from_opid(avc_operation_id opid)
{
    switch (opid)
    {
    case opid_power:
        return stateManagerGetState() == deviceLimbo ? EventUsrPowerOn : EventUsrPowerOff;

    case opid_volume_up:
        return EventUsrMainOutVolumeUp;

    case opid_volume_down:
        return EventUsrMainOutVolumeDown;

    case opid_mute:
        return EventUsrMainOutMuteToggle;

    case opid_play:
        return EventUsrAvrcpPlay;

    case opid_stop:
        return EventUsrAvrcpStop;

    case opid_pause:
        return EventUsrAvrcpPause;

    case opid_forward:
        return EventUsrAvrcpSkipForward;

    case opid_backward:
        return EventUsrAvrcpSkipBackward;

    default:
        return EventInvalid;
    }
}
#endif


/*************************************************************************
NAME
    pitch_length_tone

DESCRIPTION
    Pack a pitch + length pair into a ringtone note
*/
static ringtone_note pitch_length_tone(uint8 pitch, uint8 length)
{
    return (ringtone_note)
            ((pitch << RINGTONE_SEQ_NOTE_PITCH_POS)
            | (length << RINGTONE_SEQ_NOTE_LENGTH_POS));
}

/*************************************************************************
NAME
    send_app_message

DESCRIPTION
    Send a message to the main application
*/
static void send_app_message(uint16 message)
{
    MessageSend(&theSink.task, message, NULL);
}


/*************************************************************************
NAME
    set_abs_eq_bank

DESCRIPTION
    Select a Music Manager equaliser bank by absolute index
*/
static void set_abs_eq_bank(uint16 eq_bank)
{
    if(sinkMusicProcessingUpdateEqBank(eq_bank))
    {
        AUDIO_MODE_T mode = AUDIO_MODE_CONNECTED;
        A2dpPluginModeParams* mode_params = getAudioPluginModeParams();

        GAIA_DEBUG(("G: EQ -> %u\n", eq_bank));

        usbAudioGetMode(&mode);
        AudioSetMode(mode, mode_params);
        AudioSetMusicProcessingEnhancements(mode_params->music_mode_processing, mode_params->music_mode_enhancements);
#ifdef ENABLE_PEER
        peerSendAudioEnhancements();
#endif
    }
}


/*************************************************************************
NAME
    gaia_device_reset

DESCRIPTION
    Action a GAIA_COMMAND_DEVICE_RESET command by queueing an immediate
    power off followed by a reboot
*/
static void gaia_device_reset(void)
{
    gaia_set_remain_connected(FALSE);
    sinkDataSetPoweroffEnabled(TRUE);
    send_app_message(EventUsrPowerOff);
    MessageSendLater(&theSink.task, EventSysReboot, NULL, D_SEC(2));
    gaia_send_success(GAIA_COMMAND_DEVICE_RESET);
}


/*************************************************************************
NAME
    gaia_change_volume

DESCRIPTION
    Respond to GAIA_COMMAND_CHANGE_VOLUME request by sending the
    appropriate event message to the sink device task.  The device inverts
    the message meanings if it thinks the buttons are inverted so we have
    to double bluff it.

    direction 0 means volume up, 1 means down.
*/
static void gaia_change_volume(uint8 direction)
{
    uint16 message = 0;
    uint8 status;

    if (sinkVolumeIsVolButtonInverted())
        direction ^= 1;

    if (direction == 0)
    {
        message = EventUsrMainOutVolumeUp;
        status = GAIA_STATUS_SUCCESS;
    }

    else if (direction == 1)
    {
        message = EventUsrMainOutVolumeDown;
        status = GAIA_STATUS_SUCCESS;
    }

    else
        status = GAIA_STATUS_INVALID_PARAMETER;

    if (status == GAIA_STATUS_SUCCESS)
        send_app_message(message);

    gaia_send_simple_response(GAIA_COMMAND_CHANGE_VOLUME, status);
}


/*************************************************************************
NAME
    gaia_alert_leds

DESCRIPTION
    Act on a GAIA_COMMAND_ALERT_LEDS request

*/
static void gaia_alert_leds(uint8 *payload)
{
    LEDPattern_t pattern;

    if (LedManagerGetEventPattern(EventSysGaiaAlertLEDs, &pattern))
    {
        uint16 time;
        uint16 timeMultiplier = LedManagerGetTimeMultiplier();

        time = (payload[0] << 8) | payload[1];
        pattern.OnTime = (time >> timeMultiplier) / 10;

        time = (payload[2] << 8) | payload[3];
        pattern.OffTime = (time >> timeMultiplier) / 10;

        time = (payload[4] << 8) | payload[5];
        pattern.RepeatTime = (time >> timeMultiplier) / 50;

        pattern.DimTime = payload[7];
        pattern.NumFlashes = payload[8];
        pattern.TimeOut = payload[9];
        pattern.LED_A = payload[10];
        pattern.LED_B = payload[11];
        pattern.OverideDisable = FALSE;
        pattern.Colour = payload[12];

        if (LedManagerSetEventPattern(EventSysGaiaAlertLEDs, &pattern))
        {
            LedManagerIndicateEvent(EventSysGaiaAlertLEDs);
            gaia_send_success(GAIA_COMMAND_ALERT_LEDS);
}
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_LEDS);
        }
    }
    else
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_LEDS);
    }
}

/*************************************************************************
NAME
    gaia_alert_tone

DESCRIPTION
    Act on a GAIA_COMMAND_ALERT_TONE request

    The command payload holds tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and duration values for each note.

NOTE
    Since we can't be sure when the tone will start playing, let alone
    when it will finish, we can't free the storage.
*/
static void gaia_alert_tone(uint8 length, uint8 *tones)
{

/*  Must be at least 4 octets and an even number in total  */
    if ((length < 4) || (length & 1))
        gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_TONE);

/*  If there are no notes then we're done  */
    else if (length == 4)
        gaia_send_success(GAIA_COMMAND_ALERT_TONE);

    else
    {
        if (gaia_get_alert_tone()== NULL)
            gaia_set_alert_tone(mallocPanic(GAIA_TONE_BUFFER_SIZE));

        if ((gaia_get_alert_tone() == NULL) || (length > GAIA_TONE_MAX_LENGTH))
            gaia_send_insufficient_resources(GAIA_COMMAND_ALERT_TONE);

        else
        {
            uint16 idx;
            uint16 volume;
            ringtone_note *t = gaia_get_alert_tone();

            GAIA_DEBUG(("G: at: %d %d %d %d\n", tones[0], tones[1], tones[2], tones[3]));

            AudioStopToneAndPrompt( TRUE );


        /*  Set up tempo, volume, timbre and decay  */
            *t++ = RINGTONE_TEMPO(tones[0] * 4);
            volume = tones[1];
            *t++ = (ringtone_note) (RINGTONE_SEQ_TIMBRE | tones[2]);
            *t++ = RINGTONE_DECAY(tones[3]);


        /*  Set up note-duration pairs  */
            for (idx = 4; idx < length; idx += 2)
            {
                GAIA_DEBUG((" - %d %d\n", tones[idx], tones[idx + 1]));
                *t++ = pitch_length_tone(tones[idx], tones[idx + 1]);
            }

        /*  Mark the end  */
            *t = RINGTONE_END;

            AudioPlayTone(gaia_get_alert_tone(), FALSE, (int16)volume, sinkAudioGetPluginFeatures());
            gaia_send_success(GAIA_COMMAND_ALERT_TONE);
        }
    }
}


/*************************************************************************
NAME
    gaia_alert_event

DESCRIPTION
    Act on a GAIA_COMMAND_ALERT_EVENT request by indicating the associated
    event

*/
static void gaia_alert_event(uint16 event)
{
    GAIA_DEBUG(("G: ae: %04X\n", event));

    if (is_valid_event_id(event))
    {
        if (event != EventSysLEDEventComplete)
            LedManagerIndicateEvent(event);

        sinkAudioIndicationPlayEvent(event);

        gaia_send_success(GAIA_COMMAND_ALERT_EVENT);
    }

    else
        gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_EVENT);
}


/*************************************************************************
NAME
    gaia_alert_voice

DESCRIPTION
    Act on a GAIA_COMMAND_ALERT_VOICE request
    Play the indicated voice prompt
    We have already established that theSink.num_audio_prompt_languages != 0

*/
static void gaia_alert_voice(uint8 payload_length, uint8 *payload)
{
    TaskData *plugin = NULL;
    uint16 nr_prompts = 0;
    uint16 num_lang = SinkAudioPromptsGetNumAudioPromptLanguages();
    uint16 ap_id = (payload[0] << 8) | payload[1];

    if(num_lang)
    {
        nr_prompts= SinkAudioPromptsGetNumAudioPromptFiles() / num_lang;
    }
    else
    {
        GAIA_DEBUG(("G: Number of Audio prompt languages is 0"));
        Panic();
    }

    UNUSED(payload_length);

    GAIA_DEBUG(("G: ti: %d/%d + %d\n", ap_id, nr_prompts, payload_length - 2));

    if (ap_id < nr_prompts)
    {
        plugin = (TaskData *) &csr_voice_prompts_plugin;

        AudioPromptPlay(plugin, ap_id, TRUE, FALSE);
        gaia_send_success(GAIA_COMMAND_ALERT_VOICE);
    }

    else
        gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_VOICE);
}


/*************************************************************************
NAME
    gaia_set_led_config

DESCRIPTION
    Respond to GAIA_COMMAND_SET_LED_CONFIGURATION request

    State and Event config requests are 17 octets long and look like
    +--------+-----------------+-----------------+-----------------+-----------------+
    |  TYPE  |   STATE/EVENT   |   LED ON TIME   |    OFF TIME     |   REPEAT TIME   |
    +--------+--------+--------+--------+--------+--------+--------+--------+--------+ ...
    0        1        2        3        4        5        6        7        8
        +-----------------+--------+--------+--------+--------+--------+--------+
        |    DIM TIME     |FLASHES |TIMEOUT | LED_A  | LED_B  |OVERRIDE| COLOUR |
    ... +--------+--------+--------+--------+--------+--------+DISABLE-+--------+
        9       10       11       12       13       14       15       16

    Filter config requests are 6 to 8 octets depending on the filter action and look like
    +--------+--------+-----------------+--------+--------+--------+--------+
    |  TYPE  | INDEX  |      EVENT      | ACTION |OVERRIDE| PARAM1 |[PARAM2]|
    +--------+--------+--------+--------+--------+DISABLE-+--------+--------+
    0        1        2        3        4        5        6       [7]
*/
static void gaia_set_led_config(uint16 request_length, uint8 *request)
{
    uint16 event;
    uint8 expect_length;
    uint8 type = request[0];
    bool changed = FALSE;


    switch (type)
    {
    case GAIA_LED_CONFIGURATION_FILTER:

        switch (request[4])
        {
        case GAIA_LED_FILTER_OVERRIDE:
            expect_length = 6;
            break;

        case GAIA_LED_FILTER_CANCEL:
        case GAIA_LED_FILTER_COLOUR:
        case GAIA_LED_FILTER_FOLLOW:
            expect_length = 7;
            break;

        case GAIA_LED_FILTER_SPEED:
            expect_length = 8;
            break;

        default:
            expect_length = 0;
            break;
        }
        break;

    case GAIA_LED_CONFIGURATION_STATE:
    case GAIA_LED_CONFIGURATION_EVENT:
        expect_length = 17;
        break;

    default:
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);
        return;
    }

    GAIA_DEBUG(("G: lc: t=%d id=%d r=%d x=%d\n",
                 type, request[2], request_length, expect_length));

    if (request_length != expect_length)
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);
        return;
    }

    if (type == GAIA_LED_CONFIGURATION_FILTER)
    {
        uint16 index;
        LEDFilter_t filter;

        index = request[1] - 1;
        event = (request[2] << 8) | (request[3] & 0xFF);

        if (!is_valid_event_id(event))
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);
            return;
        }

        memset(&filter, 0, sizeof(filter));
        filter.Event = event;

        switch (request[4])
        {
        case GAIA_LED_FILTER_CANCEL:
            filter.FilterType = CANCEL;
            filter.FilterToCancel = request[6];
            break;

        case GAIA_LED_FILTER_SPEED:
            filter.FilterType = SPEED;
            filter.SpeedAction = request[6];
            filter.Speed = request[7];
            break;

        case GAIA_LED_FILTER_OVERRIDE:
            filter.FilterType = OVERRIDE;
            break;

        case GAIA_LED_FILTER_COLOUR:
            filter.FilterType = COLOUR;
            filter.Colour = request[6];
            break;

        case GAIA_LED_FILTER_FOLLOW:
            filter.FilterType = FOLLOW;
            filter.FollowerLEDDelay = request[6];
            break;

        default:
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);
            return;
        }

        changed = LedManagerSetEventFilter(index, &filter);
                }

    else
    {
        LEDPattern_t pattern;

        event = (request[1] << 8) | (request[2] & 0xFF);
        if (type == GAIA_LED_CONFIGURATION_EVENT && !is_valid_event_id(event))
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);
            return;
        }

        memset(&pattern, 0, sizeof(pattern));

        pattern.OnTime = ((uint16) (request[3] << 8) | request[4]) / 10;
        pattern.OffTime = ((uint16) (request[5] << 8) | request[6]) / 10;
        pattern.RepeatTime = ((uint16) (request[7] << 8) | request[8]) / 50;
        pattern.DimTime = request[10];
        pattern.NumFlashes = request[11];
        pattern.TimeOut = request[12];
        pattern.LED_A = request[13];
        pattern.LED_B = request[14];
        pattern.OverideDisable = request[15];
        pattern.Colour = request[16];

        if (type == GAIA_LED_CONFIGURATION_STATE)
        {
            /* Event should be an extended state but this will be defined in the GAIA request.
             * This way, if GAIA uses extended states and implement the request for it, it will
             * work. If not, sink states and extended states still correspond so far (index-wise) */
            changed = LedManagerSetExtendedStatePattern(event, &pattern);
        }
        else if (type == GAIA_LED_CONFIGURATION_EVENT)
            changed = LedManagerSetEventPattern(event, &pattern);
    }

    if (!changed)
        gaia_send_insufficient_resources(GAIA_COMMAND_SET_LED_CONFIGURATION);

    else if (type == GAIA_LED_CONFIGURATION_FILTER)
    {
        uint8 payload[4];
        payload[0] = type;          /* type */
        payload[1] = request[1];    /* index  */
        payload[2] = event >> 8;    /* event */
        payload[3] = event & 0xFF;
        gaia_send_success_payload(GAIA_COMMAND_SET_LED_CONFIGURATION, 4, payload);
    }
    else
    {
        uint8 payload[3];
        payload[0] = type;          /* type */
        payload[1] = event >> 8;    /* event */
        payload[2] = event & 0xFF;
        gaia_send_success_payload(GAIA_COMMAND_SET_LED_CONFIGURATION, 3, payload);
    }
}


/*************************************************************************
NAME
    gaia_send_led_config

DESCRIPTION
    Respond to GAIA_COMMAND_GET_LED_CONFIGURATION request for the
    configuration of a given state, event or filter
*/
static void gaia_send_led_config(uint8 type, uint16 index)
{
    uint16 payload_len;
    uint8 payload[17];


    payload[0] = type;
    payload[1] = index >> 8;
    payload[2] = index & 0xFF;

    switch (type)
    {
    case GAIA_LED_CONFIGURATION_FILTER:
    {
            LEDFilter_t filter;

            /*  Filter numbers are 1-based  */
            index--;

            if (!LedManagerGetEventFilter(index, &filter))
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_LED_CONFIGURATION);

        else
        {
                payload[3] = filter.Event >> 8;
                payload[4] = filter.Event & 0xFF;
                payload[6] = filter.OverideDisable;

                switch (filter.FilterType)
            {
            case CANCEL:
                payload[5] = GAIA_LED_FILTER_CANCEL;
                    payload[7] = filter.FilterToCancel;
                payload_len = 8;
                break;

            case SPEED:
                payload[5] = GAIA_LED_FILTER_SPEED;
                    payload[7] = filter.SpeedAction;
                    payload[8] = filter.Speed;
                payload_len = 9;
                break;

            case OVERRIDE:
                payload[5] = GAIA_LED_FILTER_OVERRIDE;
                payload_len = 7;
                break;

            case COLOUR:
                payload[5] = GAIA_LED_FILTER_COLOUR;
                    payload[7] = filter.Colour;
                payload_len = 8;
                break;

            case FOLLOW:
                payload[5] = GAIA_LED_FILTER_FOLLOW;
                    payload[7] = filter.FollowerLEDDelay;
                payload_len = 8;
                break;

            default:
            /*  Can't infer the filter action  */
                payload_len = 0;
                break;
            }


            if (payload_len == 0)
                gaia_send_insufficient_resources(GAIA_COMMAND_GET_LED_CONFIGURATION);

            else
                gaia_send_success_payload(GAIA_COMMAND_GET_LED_CONFIGURATION, payload_len, payload);
        }
    }
        break;

    case GAIA_LED_CONFIGURATION_STATE:
    case GAIA_LED_CONFIGURATION_EVENT:
    {
            bool found = FALSE;
            uint16 time;
            LEDPattern_t pattern;

            memset(&pattern, 0, sizeof(LEDPattern_t));

            if (type == GAIA_LED_CONFIGURATION_STATE)
			{
                /* Index should be an extended state but this will be defined in the GAIA request.
                * This way, if GAIA uses extended states and implement the request for it, it will
                * work. If not, sink states and extended states still correspond (index-wise) so far */
                found = LedManagerGetExtendedStatePattern(index, &pattern);
            }
            else if (type == GAIA_LED_CONFIGURATION_EVENT)
                found = LedManagerGetEventPattern(index, &pattern);

            if (!found)
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_LED_CONFIGURATION);

        else
        {
            GAIA_DEBUG(("G: e: %d %d\n", type, index));
            time = pattern.OnTime * 10;
            payload[3] = time >> 8;
            payload[4] = time & 0xFF;

            time = pattern.OffTime * 10;
            payload[5] = time >> 8;
            payload[6] = time & 0xFF;

            time = pattern.RepeatTime * 50;
            payload[7] = time >> 8;
            payload[8] = time & 0xFF;

            payload[9] = 0;
            payload[10] = pattern.DimTime;

            payload[11] = pattern.NumFlashes;
            payload[12] = pattern.TimeOut;
            payload[13] = pattern.LED_A;
            payload[14] = pattern.LED_B;
            payload[15] = pattern.OverideDisable;
            payload[16] = pattern.Colour;

            payload_len = 17;
            gaia_send_success_payload(GAIA_COMMAND_GET_LED_CONFIGURATION, payload_len, payload);
        }
    }
        break;

    default:
        gaia_send_invalid_parameter(GAIA_COMMAND_GET_LED_CONFIGURATION);
        break;
    }
}

/*************************************************************************
NAME
    gaia_set_tone_config

DESCRIPTION
    Set the tone associated with a given event.  We update the stored
    configuration, not the running device.
*/
static void gaia_set_tone_config(uint16 event, uint8 tone)
{
    tone_config_type tone_config;

    if (!is_valid_event_id(event))
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_TONE_CONFIGURATION);

    tone_config.event = event;
    tone_config.tone = tone;

    if (SinkTonesSetToneConfig(&tone_config))
                {
                    uint8 payload[3];

                    payload[0] = event >> 8;
                    payload[1] = event & 0xFF;
                    payload[2] = tone;
        gaia_send_success_payload(GAIA_COMMAND_SET_TONE_CONFIGURATION, sizeof(payload), payload);
                }
    else
    {
        gaia_send_insufficient_resources(GAIA_COMMAND_SET_TONE_CONFIGURATION);
    }
}


/*************************************************************************
NAME
    gaia_send_tone_config

DESCRIPTION
    Respond to GAIA_COMMAND_GET_TONE_CONFIGURATION request for a single
    event-tone pair
*/
static void gaia_send_tone_config(uint16 event)
{
    tone_config_type tone_config;

    if (SinkTonesGetToneConfig(event, &tone_config))
    {
        uint8 payload[3];
        payload[0] = tone_config.event >> 8;
        payload[1] = tone_config.event & 0xFF;
        payload[2] = tone_config.tone;
        gaia_send_success_payload(GAIA_COMMAND_GET_TONE_CONFIGURATION, sizeof payload, payload);
    }
    else
    {
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_TONE_CONFIGURATION);
        }
    }

/*************************************************************************
NAME
    gaia_send_default_volumes

DESCRIPTION
    Respond to GAIA_COMMAND_GET_DEFAULT_VOLUME
    Three octets after the status represent tones, speech and music volumes
*/
static void gaia_send_default_volumes(void)
{
        uint8 payload[3];
    volume_group_config_t volume_config;
    uint8 a2dp_volume;

    volume_config.no_of_steps = GAIA_DEFAULT_VOL_MAX;
    a2dp_volume = GainUtilsConvertScaledVolumePercentageToStep(sinkA2dpGetDefaultVolumeInPercentage() *
                                                               VOLUME_PERCENTAGE_SCALING_FACTOR,
                                                               &volume_config);

    payload[0] = sinkTonesGetFixedToneVolumeLevel();
    payload[1] = sinkHfpDataGetDefaultVolume();
    payload[2] = a2dp_volume;

        gaia_send_success_payload(GAIA_COMMAND_GET_DEFAULT_VOLUME, 3, payload);
}

/*************************************************************************
NAME
    gaia_set_default_volumes

DESCRIPTION
    Respond to GAIA_COMMAND_SET_DEFAULT_VOLUME
    The three payload octets represent tones, speech and music volumes
*/
static void gaia_set_default_volumes(uint8 volumes[])
{
    if ((volumes[0] > 22)  /* should be def or const for MAX_SPEAKER_GAIN_INDEX */
        || (volumes[1] >= VOL_NUM_VOL_SETTINGS)
        || (volumes[2] >= VOL_NUM_VOL_SETTINGS))
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_DEFAULT_VOLUME);
    }

    else
    {
        uint8 a2dp_volume = ((volumes[2] * 100) / GAIA_DEFAULT_VOL_MAX);

        if (sinkTonesSetFixedToneVolumeLevel(volumes[0])
            && sinkHfpDataSetDefaultVolume(volumes[1])
            && sinkA2dpSetDefaultVolumeInPercentage(a2dp_volume))
        {
                gaia_send_success(GAIA_COMMAND_SET_DEFAULT_VOLUME);
        }
            else
        {
                gaia_send_insufficient_resources(GAIA_COMMAND_SET_DEFAULT_VOLUME);
        }
    }
}

/*************************************************************************
NAME
    gaia_set_ap_config

DESCRIPTION
    Set the Audio Prompts configuration requested by a
    GAIA_COMMAND_SET_VOICE_PROMPT_CONFIGURATION command
*/
static void gaia_set_ap_config(uint8 *payload)
{
    audio_prompt_config_t config;
    uint16 event = (payload[0] << 8) | payload[1];
    uint8 status = GAIA_STATUS_SUCCESS;

    if (!is_valid_event_id(event))
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_VOICE_PROMPT_CONFIGURATION);
        return;
    }

    GAIA_DEBUG(("G: AP: %02X\n", event));

    config.event = event;
    config.prompt_id = payload[2];
    config.cancel_queue_play_immediate = payload[3];
    config.sco_block = payload[4];
    config.state_mask = (payload[5] << 8) | payload[6];

    if (!SinkAudioPromptsSetPromptConfig(&config))
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;

    gaia_send_simple_response(GAIA_COMMAND_SET_VOICE_PROMPT_CONFIGURATION, status);
}


/*************************************************************************
NAME
    gaia_send_ap_config

DESCRIPTION
    Send the Audio Prompt Event configuration requested by a
    GAIA_COMMAND_GET_VOICE_PROMPT_CONFIGURATION command. The configuration
    for an event is sent in six octets representing
        o  event, upper 8 bits
        o  event, lower 8 bits
        o  prompt_id (the audio prompt number)
        o  cancel_queue_play_immediate(Any queued Audio Prompts or Tones will be flushed when an event occurs with an Audio Prompt configured with this bit set.)
        o  sco_block (TRUE meaning 'do not play if SCO is present')
        o  state_mask, upper 6 bits (states in which prompt will play)
        o  state_mask, lower 8 bits
*/
static void gaia_send_ap_config(uint16 event)
{
    audio_prompt_config_t prompt_config;
    uint16 payload_len = 0;
    uint8 payload[7];
    uint8 status;

    if (!is_valid_event_id(event))
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_GET_VOICE_PROMPT_CONFIGURATION);
        return;
    }

    GAIA_DEBUG(("G: AP: %02X\n", event));

    if (!SinkAudioPromptsGetPromptConfig(event, &prompt_config))
    {
        status = GAIA_STATUS_INVALID_PARAMETER;
    }
    else
    {
        payload[0] = event >> 8;
        payload[1] = event & 0xFF;
        payload[2] = prompt_config.prompt_id;
        payload[3] = prompt_config.cancel_queue_play_immediate;
        payload[4] = prompt_config.sco_block;
        payload[5] = prompt_config.state_mask >> 8;
        payload[6] = prompt_config.state_mask & 0xFF;

        payload_len = 7;
        status = GAIA_STATUS_SUCCESS;
    }

    gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_VOICE_PROMPT_CONFIGURATION,
                     status, payload_len, payload);
}

/*************************************************************************
NAME
    is_valid_timer_number

DESCRIPTION
    Helper function to determine if a number represents an implemented
    timer

RETURNS
    TRUE if timer number is valid, FALSE otherwise (e.g. the number
    is out of range or refers to a conditionally compiled module which
    is not enabled).

*/
static bool is_valid_timer_number(uint8 timer_num)
{
    if (timer_num < GAIA_TIMER_NUM_PARAMETERS)
    {
        switch (timer_num)
        {
#ifndef ENABLE_SPEECH_RECOGNITION
        case gaia_timer_speech_rec_repeat_time_ms:
            return FALSE;
#endif

#ifndef ENABLE_WIRED
        case gaia_timer_wired_audio_connected_power_off_timeout_s:
            return FALSE;
#endif

#ifndef GATT_IAS_SERVER
        case gaia_timer_immediate_alert_timer_s:
        case gaia_timer_immediate_alert_stop_timeout_s:
            return FALSE;
#endif

#ifndef GATT_LLS_SERVER
        case gaia_timer_link_loss_timer_s:
        case gaia_timer_link_loss_alert_stop_timeout_s:
            return FALSE;
#endif

#ifndef ENABLE_PARTYMODE
        case gaia_timer_party_mode_music_timeout_s:
        case gaia_timer_party_mode_stream_resume_timeout_s:
            return FALSE;
#endif

#ifndef ENABLE_BREDR_SC
        case gaia_timer_bredr_authenticated_payload_timeout_s:
            return FALSE;
#endif

#ifndef GATT_ENABLED
        case gaia_timer_le_authenticated_payload_timeout_s:
            return FALSE;
#endif

        default:
            return TRUE;
        }
    }

    return FALSE;
}

/*************************************************************************
NAME
    get_timer_value

DESCRIPTION
    Helper function to get the value for the given timeout parameter.

*/
static uint16 get_timer_value(gaia_timer_param_t timer)
{
    switch (timer)
        {
    case gaia_timer_auto_switch_off_time_s: return sinkDataGetAutoSwitchOffTimeout();
    case gaia_timer_limbo_timeout_s: return sinkDataGetLimboTimeout();
    case gaia_timer_network_service_indicator_repeat_time_s: return sinkHfpDataGetNWSIndicatorRepeatTime();
    case gaia_timer_disable_power_off_after_power_on_time_s: return sinkDataGetDisablePoweroffAfterPoweronTimeout();
    case gaia_timer_pairmode_timeout_s: return sinkDataGetPairModeTimeout();
    case gaia_timer_mute_remind_time_s: return sinkHfpDataGetMuteRemindTime();
    case gaia_timer_connectable_timeout_s: return sinkDataGetConnectableTimeout();
    case gaia_timer_pair_mode_timeout_if_pdl_s: return sinkDataGetPairModePDLTimeout();
    case gaia_timer_reconnection_attempts: return sinkDataGetReconnectionAttempts();
    case gaia_timer_encryption_refresh_timeout_m: return sinkDataGetEncryptionRefreshTimeout();

    case gaia_timer_inquiry_timeout_s: return sinkInquiryGetInquiryTimeout();
    case gaia_timer_second_ag_connect_delay_time_s: return sinkDataSecondAGConnectDelayTimeout();
    case gaia_timer_missed_call_indicate_time_s: return sinkHfpDataGetMissedCallIndicateTime();
    case gaia_timer_missed_call_indicate_attempts: return sinkHfpDataGetMissedCallIndicateAttempsTime();
    case gaia_timer_a2dp_link_loss_reconnection_time_s: return getA2dpLinkLossReconnectTimeout();
    case gaia_timer_speech_rec_repeat_time_ms: return speechRecognitionGetRepeatTime();
    case gaia_timer_store_current_sink_volume_and_source_timeout_s: return sinkvolumeGetVolumeStoreTimeout();
    case gaia_timer_wired_audio_connected_power_off_timeout_s: return SinkWiredGetAudioConnectedPowerOffTimeout();

    case gaia_timer_defrag_check_timer_s: return sinkDataDefragCheckTimout();
    case gaia_timer_audio_amp_power_down_timeout_in_limbo_s: return sinkDataAmpPowerDownTimeout();
    case gaia_timer_immediate_alert_timer_s: return sinkGattIasServerGetImmediateAlertTimer_s();
    case gaia_timer_immediate_alert_stop_timeout_s: return sinkGattIasClientGetImmAlertStopTimer();
    case gaia_timer_link_loss_timer_s: return sinkGattGetLinkLossTimer_s();
    case gaia_timer_link_loss_alert_stop_timeout_s: return sinkGattGetlinkLossAlertstopTimeout_s();
    case gaia_timer_audio_amp_unmute_time_ms: return sinkAudioAmpPowerUpTime();
    case gaia_timer_audio_amp_mute_time_ms: return sinkAudioAmpPowerDownTime();
    case gaia_timer_party_mode_music_timeout_s: return sinkPartymodeGetMusicTimeOut();
    case gaia_timer_party_mode_stream_resume_timeout_s: return sinkPartymodeGetStreamResumeTimeOut();

    case gaia_timer_bredr_authenticated_payload_timeout_s: return sinkSCGetBrEdrAuthenticationPaylodTO();
    case gaia_timer_le_authenticated_payload_timeout_s: return sinkBLEGetLeAuthenticatedPayloadTO();

    default: return 0;
    }
}

/*************************************************************************
NAME
    set_timer_value

DESCRIPTION
    Helper function to set the value for the given timer parameter.

RETURNS
    TRUE is value was set ok, FALSE otherwise.
*/
static bool set_timer_value(gaia_timer_param_t timer, uint16 value)
{
    switch (timer)
            {
    case gaia_timer_auto_switch_off_time_s: return sinkDataSetAutoSwitchOffTimeout(value);
    case gaia_timer_limbo_timeout_s: return sinkDataSetLimboTimeout(value);
    case gaia_timer_network_service_indicator_repeat_time_s: return sinkHfpDataSetNWSIndicatorRepeatTime(value);
    case gaia_timer_disable_power_off_after_power_on_time_s: return sinkDataSetDisablePoweroffAfterPoweronTimeout(value);
    case gaia_timer_pairmode_timeout_s: return sinkDataSetPairModeTimeout(value);
    case gaia_timer_mute_remind_time_s: return sinkHfpDataSetMuteRemindTime(value);
    case gaia_timer_connectable_timeout_s: return sinkDataSetConnectableTimeout(value);
    case gaia_timer_pair_mode_timeout_if_pdl_s: return sinkDataSetPairModePDLTimeout(value);
    case gaia_timer_reconnection_attempts: return sinkDataSetReconnectionAttempts(value);
    case gaia_timer_encryption_refresh_timeout_m: return sinkDataSetEncryptionRefreshTimeout(value);

    case gaia_timer_inquiry_timeout_s: return sinkInquirySetInquiryTimeout(value);
    case gaia_timer_second_ag_connect_delay_time_s: return sinkDataSetSecondAGConnectDelayTimeout(value);
    case gaia_timer_missed_call_indicate_time_s: return sinkHfpDataSetMissedCallIndicateTime(value);
    case gaia_timer_missed_call_indicate_attempts: return sinkHfpDataSetMissedCallIndicateAttempsTime(value);
    case gaia_timer_a2dp_link_loss_reconnection_time_s: return setA2dpLinkLossReconnectTimeout(value);
    case gaia_timer_speech_rec_repeat_time_ms: return speechRecognitionSetRepeatTime(value);
    case gaia_timer_store_current_sink_volume_and_source_timeout_s: return sinkvolumeSetVolumeStoreTimeout(value);
    case gaia_timer_wired_audio_connected_power_off_timeout_s: return SinkWiredSetAudioConnectedPowerOffTimeout(value);

    case gaia_timer_defrag_check_timer_s: return sinkDataSetDefragCheckTimout(value);
    case gaia_timer_audio_amp_power_down_timeout_in_limbo_s: return sinkDataSetAmpPowerDownTimeout(value);
    case gaia_timer_immediate_alert_timer_s: return sinkGattIasServerSetImmediateAlertTimer_s(value);
    case gaia_timer_immediate_alert_stop_timeout_s: return sinkGattIasClientSetImmAlertStopTimer(value);
    case gaia_timer_link_loss_timer_s: return sinkGattSetLinkLossTimer_s(value);
    case gaia_timer_link_loss_alert_stop_timeout_s: return sinkGattSetlinkLossAlertstopTimeout_s(value);
    case gaia_timer_audio_amp_unmute_time_ms: return sinkAudioSetAmpPowerUpTime(value);
    case gaia_timer_audio_amp_mute_time_ms: return sinkAudioSetAmpPowerDownTime(value);
    case gaia_timer_party_mode_music_timeout_s: return sinkPartymodeSetMusicTimeOut(value);
    case gaia_timer_party_mode_stream_resume_timeout_s: return sinkPartymodeSetStreamResumeTimeOut(value);

    case gaia_timer_bredr_authenticated_payload_timeout_s: return sinkSCSetBrEdrAuthenticationPaylodTO(value);
    case gaia_timer_le_authenticated_payload_timeout_s: return sinkBLESetLeAuthenticatedPayloadTO(value);

    default: return FALSE;
    }
}

/*************************************************************************
NAME
    gaia_set_timer_config

DESCRIPTION
    Handle GAIA_COMMAND_SET_TIMER_CONFIGURATION

    The Short Form has a timer number and a value for that timer.

    Note: The Long Form of this command is no longer supported after the
          configuration refactor. The description below is kept for reference.

    The Long Form of the command has a value for every system timer; for
    simplicity we just pack the payload into a Timeouts_t structure,
    which works so long as nobody minds EncryptionRefreshTimeout_m being
    in minutes.
*/
static void gaia_set_timer_config(uint8 payload_len, uint8 *payload)
{
    uint16 timer_num;
    uint16 timer_value;
    uint8 status = GAIA_STATUS_INVALID_PARAMETER;

    if (payload_len == 3 && is_valid_timer_number(payload[0]))
        {
        /*  Short form has timer number and value  */
        timer_num = payload[0];

        timer_value = ByteUtilsGet2BytesFromStream(&payload[1]);
        if (set_timer_value(timer_num, timer_value))
            status = GAIA_STATUS_SUCCESS;
        else
                status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
            }

    gaia_send_simple_response(GAIA_COMMAND_SET_TIMER_CONFIGURATION, status);
}

/*************************************************************************
NAME
    gaia_send_timer_config

DESCRIPTION
    Handle GAIA_COMMAND_GET_TIMER_CONFIGURATION

    The Short Form has a timer number and a value for that timer.

    Note: The Long Form of this command is no longer supported after the
          configuration refactor. The description below is kept for reference.

    The Long Form of the response has a value for every system timer; for
    simplicity we just unpack the Timeouts_t structure, which works so long
    as nobody minds EncryptionRefreshTimeout_m being in minutes.
*/
static void gaia_send_timer_config(uint8 payload_len, uint8 *payload)
{
    uint16 timer_num;
    uint16 timer;

        if (payload_len == 1)
        {
        /*  Short Form; extract one uint16 value from the array  */
        timer_num = payload[0];

        if (is_valid_timer_number(timer_num))
            {
                uint8 response[3];

            timer = get_timer_value(timer_num);

                response[0] = timer_num;
            response[1] = timer >> 8;
            response[2] = timer & 0xFF;

                gaia_send_success_payload(GAIA_COMMAND_GET_TIMER_CONFIGURATION,
                                          sizeof response, response);
            }
            else
            {
                gaia_send_invalid_parameter(GAIA_COMMAND_GET_TIMER_CONFIGURATION);
            }
        }
    else
    {
        /*  Long Form; deprecated.  */
        gaia_send_invalid_parameter(GAIA_COMMAND_GET_TIMER_CONFIGURATION);
        }
}

/*************************************************************************
NAME
    store_audio_gains

DESCRIPTION
    Store the audio gains parameters for a given volume level
    Returns TRUE if values are within allowed range
*/
static bool store_audio_gains(volume_mapping_t *mapping, uint8 *params)
{
    if (params[0] >= VOL_NUM_VOL_SETTINGS)
    {
        return FALSE;
    }

    mapping->inc_vol = params[0];

    if (params[1] >= VOL_NUM_VOL_SETTINGS)
    {
        return FALSE;
    }

    mapping->dec_vol = params[1];

    if (params[2] > GAIA_AUDIO_GAIN_MAX_TONE)
    {
        return FALSE;
    }

    mapping->tone = params[2];

/*  params[3], A2dpGain, is no longer used  */

    if (params[4] > GAIA_AUDIO_GAIN_MAX_GAIN)
    {
        return FALSE;
    }

    mapping->vol_gain = params[4];

    return TRUE;
}

/*************************************************************************
NAME
    load_audio_gains

DESCRIPTION
    Load the audio gains parameters for a given volume level
*/
static void load_audio_gains(uint8 *params, const volume_mapping_t *mapping)
{
    params[0] = mapping->inc_vol;
    params[1] = mapping->dec_vol;
    params[2] = mapping->tone;
    params[3] = 0;
    params[4] = mapping->vol_gain;
}

/*************************************************************************
NAME
    gaia_set_audio_gain_config

DESCRIPTION
    Handle GAIA_COMMAND_SET_AUDIO_GAIN_CONFIGURATION

    The long form of the command payload contains five octets for each of
    the 16 volume levels, corresponding to IncVol, DecVol, Tone, one reserved
    octet and VolGain (80 octets in total).

    The short form of the command payload has one octet to select a volume
    level and five further octets holding the settings for that volume level
    (6 octets in total).
*/
static void gaia_set_audio_gain_config(uint8 payload_len, uint8 *payload)
{
    uint8 status = GAIA_STATUS_INVALID_PARAMETER;
    uint8 *params;
    uint16 level;
    bool data_ok = TRUE;
    volume_mapping_t vol_mapping;

    if (payload_len == GAIA_AUDIO_GAIN_PAYLOAD_LONG
        || (payload_len == GAIA_AUDIO_GAIN_PAYLOAD_SHORT && payload[0] < VOL_NUM_VOL_SETTINGS))
    {
            if (payload_len == GAIA_AUDIO_GAIN_PAYLOAD_LONG)
            {
            /*  Long Form payload contains five parameters for each level  */
                params = payload;
            for (level = 0; data_ok && (level < VOL_NUM_VOL_SETTINGS); level++)
                {
                data_ok = store_audio_gains(&vol_mapping, params);
                if (data_ok)
                {
                    data_ok = sinkVolumeSetVolumeMappingForLevel(level, &vol_mapping);
                }
                    params += GAIA_VOL_MAP_ELEMENTS;

                if (!data_ok)
                    break;
                }
            }
            else
            {
            /*  Short Form payload contains volume level and parameters for that level  */
                level = payload[0];
                params = payload + 1;
            data_ok = store_audio_gains(&vol_mapping, params);
            if (data_ok)
            {
                data_ok = sinkVolumeSetVolumeMappingForLevel(level, &vol_mapping);
                }
                }

        if (data_ok)
            status = GAIA_STATUS_SUCCESS;
        else
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
    }

    gaia_send_simple_response(GAIA_COMMAND_SET_AUDIO_GAIN_CONFIGURATION, status);
}

/*************************************************************************
NAME
    gaia_send_audio_gain_config

DESCRIPTION
    Handle GAIA_COMMAND_GET_AUDIO_GAIN_CONFIGURATION

    The long format of the response contains five octets for each volume
    mapping, corresponding to IncVol, DecVol, Tone, one reserved octet and
    VolGain.

    The short form has one octet to indcate a volume level and five further
    octets holding the parameters for that level.
*/
static void gaia_send_audio_gain_config(uint8 payload_len, uint8 *payload)
{
    uint8 size_response;
    uint8 *response;
    bool ok = TRUE;
    uint16 level;
    volume_mapping_t vol_mapping;

    if (payload_len == 0)
    {
        size_response = GAIA_AUDIO_GAIN_PAYLOAD_LONG;
    }
    else if (payload_len == 1 && payload[0] < VOL_NUM_VOL_SETTINGS)
    {
        size_response = GAIA_AUDIO_GAIN_PAYLOAD_SHORT;
    }
    else
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_GET_AUDIO_GAIN_CONFIGURATION);
        return;
    }

    response = mallocPanic(size_response);

        if (payload_len == 0)
        {
        /*  Long Form: response holds settings for all levels  */
            uint8 *params = response;

        for (level = 0; level < VOL_NUM_VOL_SETTINGS; level++)
            {
            if (!sinkVolumeGetVolumeMappingForLevel(level, &vol_mapping))
            {
                ok = FALSE;
                break;
            }

            load_audio_gains(params, &vol_mapping);
                params += GAIA_VOL_MAP_ELEMENTS;
            }
        }
        else
        {
        /*  Short Form: response holds settings for specified level  */
            level = payload[0];
        if (sinkVolumeGetVolumeMappingForLevel(level, &vol_mapping))
        {
            response[0] = level;
            load_audio_gains(&response[1], &vol_mapping);
        }
        else
        {
            ok = FALSE;
        }
    }

    if (ok)
        gaia_send_success_payload(GAIA_COMMAND_GET_AUDIO_GAIN_CONFIGURATION, size_response, response);
    else
        gaia_send_insufficient_resources(GAIA_COMMAND_GET_AUDIO_GAIN_CONFIGURATION);

        freePanic(response);
    }

/*************************************************************************
NAME
    volume_parameter_valid

DESCRIPTION
    Determine if a given volume parameter is within the legal range
*/
static bool volume_parameter_valid(gaia_vol_param_t index, uint16 parameter)
{
    int16 value = (int16) parameter;
    int16 min_value = SHRT_MIN;
    int16 max_value = SHRT_MAX;

    switch (index)
    {
    	case gaia_vol_no_of_steps_main:
    	case gaia_vol_no_of_steps_aux:
    		min_value = 1;
    		max_value = GAIA_VOL_MAX_STEPS;
    		break;

    	case gaia_vol_volume_knee_value_1_main:
    	case gaia_vol_volume_knee_value_2_main:
    	case gaia_vol_volume_knee_value_1_aux:
    	case gaia_vol_volume_knee_value_2_aux:
    		min_value = 0;
    		max_value = GAIA_VOL_MAX_STEPS - 1;
    		break;

    	case gaia_vol_dB_knee_value_1_main:
    	case gaia_vol_dB_knee_value_2_main:
    	case gaia_vol_dB_knee_value_1_aux:
    	case gaia_vol_dB_knee_value_2_aux:
    		min_value = SHRT_MIN;
    		max_value = 0;
    		break;

    	case gaia_vol_dB_max_main:
    	case gaia_vol_dB_min_main:
    	case gaia_vol_dB_max_aux:
    	case gaia_vol_dB_min_aux:
    		min_value = SHRT_MIN;
    		max_value = 0;
    		break;

		case gaia_vol_device_trim_master:
		case gaia_vol_device_trim_slave:
			min_value = -GAIA_VOL_MAX_TRIM;
			max_value = GAIA_VOL_MAX_TRIM;
			break;

		case gaia_vol_device_trim_change:
			min_value = 0;
			max_value = GAIA_VOL_MAX_TRIM;
			break;

		case gaia_vol_device_trim_min:
		case gaia_vol_device_trim_max:
			min_value = -GAIA_VOL_MAX_TRIM;
			max_value = GAIA_VOL_MAX_TRIM;
			break;

		case gaia_vol_param_top:
			break;
    }

    GAIA_DEBUG(("G: vol param %u: %d <= %d <= %d: %c\n",
                index, min_value, value, max_value,
                value >= min_value && value <= max_value ? 'Y' : 'N'));

    return value >= min_value && value <= max_value;
}

/*************************************************************************
NAME
    get_dsp_volume_config

DESCRIPTION
    Read the dsp volume config values from the sink modules into
    the data structure that gaia uses.
*/
static bool get_dsp_volume_config(dsp_volume_user_config *dsp_config)
{
    uint16 i;
    peer_tws_trim_config_t trim_config;

    /* Note: ENABLE_PEER may not be defined, so default to zero values
       if peerGetTWSDeviceTrimConfig returns FALSE. */
    memset(&trim_config, 0, sizeof(trim_config));

    /* Get the group_config from sink_volume */
    for (i = 0; i < audio_output_group_all; i++)
    {
        sinkVolumeGetGroupConfig(i, &dsp_config->group_config[i]);
    }

    (void) peerGetTWSDeviceTrimConfig(&trim_config);

    dsp_config->device_trim_master = trim_config.device_trim_master;
    dsp_config->device_trim_slave = trim_config.device_trim_slave;
    dsp_config->device_trim_change = trim_config.device_trim_change;
    dsp_config->device_trim_min = trim_config.device_trim_min;
    dsp_config->device_trim_max = trim_config.device_trim_max;

    return TRUE;
}

/*************************************************************************
NAME
    set_dsp_volume_config

DESCRIPTION
    Write the dsp volume config values from the gaia data structure into
    the relevant sink modules.
*/
static bool set_dsp_volume_config(const dsp_volume_user_config *dsp_config)
{
    uint16 i;
    peer_tws_trim_config_t trim_config;

    /* Set the group_config in sink_volume */
    for (i = 0; i < audio_output_group_all; i++)
    {
        if (!sinkVolumeSetGroupConfig(i, &dsp_config->group_config[i]))
            return FALSE;
    }

    /* Set the TWS trims in sink_peer.
       Note: ENABLE_PEER may not be defined, so do not check the status. */
    trim_config.device_trim_master = dsp_config->device_trim_master;
    trim_config.device_trim_slave = dsp_config->device_trim_slave;
    trim_config.device_trim_change = dsp_config->device_trim_change;
    trim_config.device_trim_min = dsp_config->device_trim_min;
    trim_config.device_trim_max = dsp_config->device_trim_max;

    (void) peerSetTWSDeviceTrimConfig(&trim_config);

    return TRUE;
}


/*************************************************************************
NAME
    gaia_set_volume_config

DESCRIPTION
    Handle GAIA_COMMAND_SET_VOLUME_CONFIGURATION

    The long format of the command contains all 16 volume parameters, the
    short form has a parameter number and the value of that parameter.

    Individual parameters are range checked but not the relationships between
    them, since these may be transiently violated by updating them separately.
*/
static void gaia_set_volume_config(uint8 payload_len, uint8 *payload)
{
    uint8 status = GAIA_STATUS_INVALID_PARAMETER;
    dsp_volume_user_config *dsp_config = mallocPanic(sizeof(*dsp_config));

    if (dsp_config && get_dsp_volume_config(dsp_config))
    {
        uint16 *parameters = (uint16 *) dsp_config;
        uint16 index;

        if (payload_len == 2 * GAIA_VOL_NUM_PARAMETERS)
        {
        /*  Long form; pack all parameters  */
            bool valid = TRUE;

            wpack(parameters, payload, GAIA_VOL_NUM_PARAMETERS);

            for (index = 0; valid && index < GAIA_VOL_NUM_PARAMETERS; ++index)
            {
                valid = volume_parameter_valid(index, parameters[index]);
            }

            if (valid)
            {
                status = GAIA_STATUS_SUCCESS;
            }
        }
        else if (payload_len == 3 && payload[0] < GAIA_VOL_NUM_PARAMETERS)
        {
        /*  Short form; overwrite one parameter  */
            index = payload[0];
            parameters[index] = (payload[1] << 8) | payload[2];
            if (volume_parameter_valid(index, parameters[index]))
            {
                status = GAIA_STATUS_SUCCESS;
            }
        }
    }
    else
    {
        status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
    }

    if (status == GAIA_STATUS_SUCCESS)
    {
        if (!set_dsp_volume_config(dsp_config))
        {
            status = GAIA_STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    free(dsp_config);
    gaia_send_simple_response(GAIA_COMMAND_SET_VOLUME_CONFIGURATION, status);
}

/*************************************************************************
NAME
    gaia_send_volume_config

DESCRIPTION
    Handle GAIA_COMMAND_GET_VOLUME_CONFIGURATION

    The long format of the response contains all 16 volume parameters, the
    short form has a parameter number and the value of that parameter.
*/
static void gaia_send_volume_config(uint8 payload_len, uint8 *payload)
{
    dsp_volume_user_config *dsp_config = mallocPanic(sizeof(*dsp_config));

    if (dsp_config && get_dsp_volume_config(dsp_config))
    {
        uint16 *parameters = (uint16 *) dsp_config;

        if (payload_len == 0)
        {
        /*  Long form; send unpacked array  */
            gaia_send_response_16(
                    GAIA_COMMAND_GET_VOLUME_CONFIGURATION,
                    GAIA_STATUS_SUCCESS,
                    sizeof(*dsp_config),
                    parameters);
        }
        else if (payload_len == 1 && payload[0] < GAIA_VOL_NUM_PARAMETERS)
        {
        /*  Short form; send one element  */
            uint16 index = payload[0];
            uint8 response[3];

            response[0] = index;
            response[1] = parameters[index] >> 8;
            response[2] = parameters[index] & 0xFF;

            gaia_send_success_payload(GAIA_COMMAND_GET_VOLUME_CONFIGURATION, sizeof(response), response);
        }
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_VOLUME_CONFIGURATION);
        }
    }
    else
    {
        gaia_send_insufficient_resources(GAIA_COMMAND_GET_VOLUME_CONFIGURATION);
    }

    free(dsp_config);
}

/******************************************************************************
NAME
    create_tone_from_payload

DESCRIPTION
    Helper function to parse a user-defined tone from a gaia data packet.

    If successful a new memory buffer will be returned in user_tone. It is
    the responsbility of the caller to free the buffer after it has been used.

PARAMS
    payload [in] Pointer to the gaia payload data.
    payload_len [in] Length of the gaia payload data.
    user_tone [out] Pointer to the memory buffer allocated by this function
                    to store the user-defined tone.
    user_tone_length [out] Pointer to the user-defined tone length.
                           Set by this function.

RETURNS
    bool TRUE if the tone was parsed ok, FALSE otherwise.
*/
static bool create_tone_from_payload(uint8 *payload, uint8 payload_len, ringtone_note **user_tone, uint16 *user_tone_length)
{
    uint16 i;
    ringtone_note *tone;
    uint16 tone_length = (payload_len - 4) / 2 + 1;

    /* allocate buffer for tone based on given length. Allow extra for RINGTONE_END. */
    tone = mallocZDebugNoPanic((4 + tone_length + 1)*sizeof(ringtone_note));
    if (!tone)
        return FALSE;

    /* Copy the header values. */
    tone_length = 0;
    if (payload[1] != 0)
        tone[tone_length++] = RINGTONE_TEMPO(payload[1] * 4);

    if (payload[2] != 0)
        tone[tone_length++] = RINGTONE_VOLUME(payload[2]);

    if (payload[3] != 0)
        tone[tone_length++] = (ringtone_note)(RINGTONE_SEQ_TIMBRE | payload[3]);

    if (payload[4] != 0)
        tone[tone_length++] = RINGTONE_DECAY(payload[4]);

    /* Copy the pitch and length values */
    for (i = 5; i < payload_len; i += 2)
        tone[tone_length++] = pitch_length_tone(payload[i], payload[i + 1]);

    /* Mark the end of the user-defined tone */
    tone[tone_length++] = RINGTONE_END;

    *user_tone = (ringtone_note *)tone;
    *user_tone_length = tone_length;

    return TRUE;
}

/*************************************************************************
NAME
    gaia_set_user_tone_config

DESCRIPTION
    Handle GAIA_COMMAND_SET_USER_TONE_CONFIGURATION command
    The command payload contains a tone number (1 to
    SinkTonesUserToneNumMax()) and note data for the indicated tone.

    Note data consists of tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and length values for each note.

    If the indicated tone number is not in use we add it, otherwise we
    delete the existing data, compact the free space and add the new tone
    data at the end of the list.
*/
static void gaia_set_user_tone_config(uint8 payload_len, uint8 *payload)
{
    uint8 tone_idx;
    uint16 tone_len;
    ringtone_note *tone;

    if ((payload_len < 5)
        || ((payload_len & 1) == 0)
        || (payload[0] < 1)
        || (payload[0] > SinkTonesUserToneNumMax()))
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_USER_TONE_CONFIGURATION);
        return;
    }

    if (!create_tone_from_payload(payload, payload_len, &tone, &tone_len))
    {
        gaia_send_insufficient_resources(GAIA_COMMAND_SET_USER_TONE_CONFIGURATION);
        return;
    }

    tone_idx = payload[0] - 1;

    if (!SinkTonesSetUserTone(tone_idx, tone, tone_len))
    {
            gaia_send_insufficient_resources(GAIA_COMMAND_SET_USER_TONE_CONFIGURATION);
        }
        else
        {
            gaia_send_success(GAIA_COMMAND_SET_USER_TONE_CONFIGURATION);
        }

    freePanic(tone);
}


/*************************************************************************
NAME
    gaia_send_user_tone_config

DESCRIPTION
    Act on a GAIA_COMMAND_GET_USER_TONE_CONFIGURATION command to send the
    note data for the indicated user-defined tone configuration.

    Note data consists of tempo, volume, timbre and decay values for
    the whole sequence followed by pitch and length values for each note.

    At most one of each tempo, volume, timbre and decay value is used and
    the tempo value is scaled, so if the configuration was set other than
    by Gaia it might not be accurately represented.

    We expect sizeof (ringtone_note) to be 1; see ringtone_if.h
*/
static void gaia_send_user_tone_config(uint8 id)
{
    uint16 tone_length = 0;
    const ringtone_note *tone = NULL;

    if (!SinkTonesGetUserTone((id - 1), &tone, &tone_length))
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_GET_USER_TONE_CONFIGURATION);
    }
    else
    {
        uint16 *find = (uint16 *)tone;
        uint8 payload_length = 0;
        uint8 *payload = NULL;

        payload_length = 2 * tone_length + 5;
        payload = mallocPanic(payload_length);

        if (payload == NULL)
        gaia_send_insufficient_resources(GAIA_COMMAND_GET_USER_TONE_CONFIGURATION);

    else
    {
            payload[0] = id;
            payload[1] = 30; /* default tempo (30 * 4 = 120) */
            payload[2] = 31; /* default volume (max) */
            payload[3] = ringtone_tone_sine;  /* default timbre (sine) */
            payload[4] = 32;  /* default decay rate */

            payload_length = 5;

            /*  Find the pitch and length of each note  */
            while ((ringtone_note)*find != RINGTONE_END)
            {
                if (*find & RINGTONE_SEQ_CONTROL_MASK)
                {
                    switch (*find & (RINGTONE_SEQ_CONTROL_MASK | RINGTONE_SEQ_CONTROL_COMMAND_MASK))
                    {
                    case RINGTONE_SEQ_TEMPO:
                        payload[1] = ((*find & RINGTONE_SEQ_TEMPO_MASK) + 2) / 4;
                        break;

                    case RINGTONE_SEQ_VOLUME:
                        payload[2] = *find & RINGTONE_SEQ_VOLUME_MASK;
                        break;

                    case RINGTONE_SEQ_TIMBRE:
                        payload[3] = *find & RINGTONE_SEQ_TIMBRE_MASK;
                        break;

                    case RINGTONE_SEQ_DECAY:
                        payload[4] = *find & RINGTONE_SEQ_DECAY_RATE_MASK;
                        break;
                    }
                }
                else
            {
                        payload[payload_length++] = (*find & RINGTONE_SEQ_NOTE_PITCH_MASK) >> RINGTONE_SEQ_NOTE_PITCH_POS;
                        payload[payload_length++] = *find & RINGTONE_SEQ_NOTE_LENGTH_MASK;
                    }

                    ++find;
                }

                gaia_send_success_payload(GAIA_COMMAND_GET_USER_TONE_CONFIGURATION,
                                          payload_length, payload);

                freePanic(payload);
            }
        }
}


/*************************************************************************
NAME
    gaia_send_application_version

DESCRIPTION
    Handle GAIA_COMMAND_GET_APPLICATION_VERSION by sending the Device ID
    setting
*/
static void gaia_send_application_version(void)
{
#ifdef DEVICE_ID_CONST
/*  Device ID is defined in sink_device_id.h  */
    uint16 payload[] = {DEVICE_ID_VENDOR_ID_SOURCE,
                        DEVICE_ID_VENDOR_ID,
                        DEVICE_ID_PRODUCT_ID,
                        DEVICE_ID_BCD_VERSION};

    gaia_send_response_16(GAIA_COMMAND_GET_APPLICATION_VERSION,
                         GAIA_STATUS_SUCCESS, PS_SIZE_ADJ(sizeof(payload)), payload);
#else
/*  Read Device ID from config, irrespective of DEVICE_ID_PSKEY  */
    uint16 payload[8];
    uint16 payload_length;

    payload_length = SinkDeviceIDRetrieveConfiguration(payload);

    gaia_send_response_16(GAIA_COMMAND_GET_APPLICATION_VERSION,
                         GAIA_STATUS_SUCCESS, payload_length, payload);
#endif
}

#if defined ENABLE_PEER && defined PEER_TWS
/*************************************************************************
NAME
    gaia_send_tws_forced_downmix_mode

DESCRIPTION
    Handle GAIA_COMMAND_GET_TWS_FORCED_DOWNMIX_MODE
*/
static void gaia_send_tws_forced_downmix_mode(void)
{
    uint8 mode = (uint8) peerGetTwsForcedDownmixMode();
    
    gaia_send_response(GAIA_VENDOR_QTIL,
                       GAIA_COMMAND_GET_TWS_FORCED_DOWNMIX_MODE,
                       GAIA_STATUS_SUCCESS,
                       1, 
                       &mode);
}

/*************************************************************************
NAME
    gaia_set_tws_routing

DESCRIPTION
    Handle GAIA_COMMAND_SET_TWS_AUDIO_ROUTING
*/
static void gaia_set_tws_routing(uint8 device, uint8 mode)
{
    if (PeerImplementsTwsForcedDownmix())
    {
    /*  Routing cannot be changed except by PIO  */
        gaia_send_incorrect_state(GAIA_COMMAND_SET_TWS_AUDIO_ROUTING);
    }
    else
    {
        uint8 response[2];
        uint8 peer_mode;
    
        response[0] = device;
        response[1] = mode;
    
        if (device == 0)
        {
            peer_mode = getTWSRoutingMode(tws_slave);
            peerSetPeerAudioRouting(mode, peer_mode);
        }
        else
        {
            peer_mode = getTWSRoutingMode(tws_master);
            peerSetPeerAudioRouting(peer_mode, mode);
        }
    
        GAIA_DEBUG(("G: tws set rtg %u %u\n", device, mode));
    
        gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_SET_TWS_AUDIO_ROUTING,
                           GAIA_STATUS_SUCCESS, sizeof response, response);
    }
}


/*************************************************************************
NAME
    gaia_send_tws_routing

DESCRIPTION
    Handle GAIA_COMMAND_GET_TWS_AUDIO_ROUTING
*/
static void gaia_send_tws_routing(uint8 device)
{
    uint8 response[2];

    response[0] = device;
    response[1] = (device == 0) ? getTWSRoutingMode(tws_master) : getTWSRoutingMode(tws_slave);

    GAIA_DEBUG(("G: tws get rtg %u %u\n", device, response[1]));

    gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_TWS_AUDIO_ROUTING,
                       GAIA_STATUS_SUCCESS, sizeof response, response);
}


/*************************************************************************
NAME
    gaia_trim_tws_volume

DESCRIPTION
    Handle GAIA_COMMAND_TRIM_TWS_VOLUME

    role 0 means master, 1 means slave
    direction 0 means volume up, 1 means down.

*/
static void gaia_trim_tws_volume(uint8 role, uint8 direction)
{
    if ((role < 2) && (direction < 2))
    {
        uint16 event = EventInvalid;

        switch ((role << 1) | direction)
        {
        case 0:
            event = EventUsrMasterDeviceTrimVolumeUp;
            break;

        case 1:
            event = EventUsrMasterDeviceTrimVolumeDown;
            break;

        case 2:
            event = EventUsrSlaveDeviceTrimVolumeUp;
            break;

        case 3:
            event = EventUsrSlaveDeviceTrimVolumeDown;
            break;
        }

        send_app_message(event);
        gaia_send_success(GAIA_COMMAND_TRIM_TWS_VOLUME);
    }

    else
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_TRIM_TWS_VOLUME);
    }
}


/*************************************************************************
NAME
    gaia_set_peer_link_reserved

DESCRIPTION
    Handle GAIA_COMMAND_SET_PEER_LINK_RESERVED
*/
static void gaia_set_peer_link_reserved(uint8 payload_length, uint8 *payload)
{
    if (payload_length == 1 && payload[0] < 2)
    {
        if(payload[0])
        {
            send_app_message(EventUsrPeerReserveLinkOn);
        }
        else
        {
            send_app_message(EventUsrPeerReserveLinkOff);
        }

        gaia_send_success(GAIA_COMMAND_SET_PEER_LINK_RESERVED);
    }
    else
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_PEER_LINK_RESERVED);
    }
}

#endif /* defined ENABLE_PEER && defined PEER_TWS */

/*************************************************************************
NAME
    gaia_register_notification

DESCRIPTION
    Handle GAIA_COMMAND_REGISTER_NOTIFICATION
*/
static void gaia_register_notification(uint8 payload_length, uint8 *payload)
{
    if (payload_length == 0)
        gaia_send_invalid_parameter(GAIA_COMMAND_REGISTER_NOTIFICATION);

    else
    {
        uint8 status = GAIA_STATUS_INVALID_PARAMETER;

        GAIA_DEBUG(("G: reg: %02X %d\n", payload[0], payload_length));
        switch (payload[0])
        {
        case GAIA_EVENT_PIO_CHANGED:
            if (payload_length == 5)
            {
                uint32 gaia_mask;

                gaia_mask = payload[1];
                gaia_mask = (gaia_mask << 8) | payload[2];
                gaia_mask = (gaia_mask << 8) | payload[3];
                gaia_mask = (gaia_mask << 8) | payload[4];

                gaia_set_pio_change_mask(gaia_mask);

                GAIA_DEBUG(("G: pm: %08lX\n", gaia_mask));
                status = GAIA_STATUS_SUCCESS;
            }

            break;


        case GAIA_EVENT_BATTERY_CHARGED:
            gaia_set_notify_battery_charged(TRUE);
            status = GAIA_STATUS_SUCCESS;
            break;


        case GAIA_EVENT_CHARGER_CONNECTION:
            gaia_set_notify_charger_connection(TRUE);
            status = GAIA_STATUS_SUCCESS;
            break;


        case GAIA_EVENT_USER_ACTION:
            gaia_set_notify_ui_event(TRUE);
            status = GAIA_STATUS_SUCCESS;
            break;


#ifdef ENABLE_SPEECH_RECOGNITION
        case GAIA_EVENT_SPEECH_RECOGNITION:
            gaia_set_notify_speech_rec(TRUE);
            status = GAIA_STATUS_SUCCESS;
            break;
#endif
        }

        gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_REGISTER_NOTIFICATION, status, 1, payload);

        if (payload[0] == GAIA_EVENT_CHARGER_CONNECTION)
        {
            status = powerManagerIsChargerConnected();
            gaia_send_notification(GAIA_EVENT_CHARGER_CONNECTION, 1, &status);
        }
    }
}


/*************************************************************************
NAME
    gaia_get_notification

DESCRIPTION
    Handle GAIA_COMMAND_GET_NOTIFICATION
    Respond with the current notification setting (enabled/disabled status
    and notification-specific data)
*/
static void gaia_get_notification(uint8 payload_length, uint8 *payload)
{
    uint16 response_len = 0;
    uint8 response[6];
    uint8 status = GAIA_STATUS_INVALID_PARAMETER;

    if (payload_length > 0)
    {
        response[0] = payload[0];
        response_len = 1;

        switch (payload[0])
        {
        case GAIA_EVENT_PIO_CHANGED:
            {
                uint32 mask = gaia_get_pio_change_mask32();

                response[1] = (mask != 0);
                dwunpack(response + 2, mask);
            response_len = 6;
            status = GAIA_STATUS_SUCCESS;
            }
            break;

        case GAIA_EVENT_BATTERY_CHARGED:
            response[1] = gaia_get_notify_battery_charged();
            response_len = 2;
            status = GAIA_STATUS_SUCCESS;
            break;

        case GAIA_EVENT_CHARGER_CONNECTION:
            response[1] = gaia_get_notify_charger_connection();
            response_len = 2;
            status = GAIA_STATUS_SUCCESS;
            break;

        case GAIA_EVENT_USER_ACTION:
            response[1] = gaia_get_notify_ui_event();
            response_len = 2;
            status = GAIA_STATUS_SUCCESS;
            break;

#ifdef ENABLE_SPEECH_RECOGNITION
        case GAIA_EVENT_SPEECH_RECOGNITION:
            response[1] = gaia_get_notify_speech_rec();
            response_len = 2;
            status = GAIA_STATUS_SUCCESS;
            break;
#endif
        }
    }

    gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_NOTIFICATION, status, response_len, response);
}


/*************************************************************************
NAME
    gaia_cancel_notification

DESCRIPTION
    Handle GAIA_COMMAND_CANCEL_NOTIFICATION
*/
static void gaia_cancel_notification(uint8 payload_length, uint8 *payload)
{
    if (payload_length != 1)
        gaia_send_invalid_parameter(GAIA_COMMAND_CANCEL_NOTIFICATION);

    else
    {
        uint8 status;

        switch (payload[0])
        {
        case GAIA_EVENT_PIO_CHANGED:
            {
                gaia_clear_pio_change_mask();
            status = GAIA_STATUS_SUCCESS;
            }
            break;

        case GAIA_EVENT_BATTERY_CHARGED:
            gaia_set_notify_battery_charged(FALSE);
            status = GAIA_STATUS_SUCCESS;
            break;

        case GAIA_EVENT_CHARGER_CONNECTION:
            gaia_set_notify_charger_connection(FALSE);
            status = GAIA_STATUS_SUCCESS;
            break;

        case GAIA_EVENT_USER_ACTION:
            gaia_set_notify_ui_event(FALSE);
            status = GAIA_STATUS_SUCCESS;
            break;

#ifdef ENABLE_SPEECH_RECOGNITION
        case GAIA_EVENT_SPEECH_RECOGNITION:
            gaia_set_notify_speech_rec(FALSE);
            status = GAIA_STATUS_SUCCESS;
            break;
#endif

        default:
            status = GAIA_STATUS_INVALID_PARAMETER;
            break;
        }

        gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_CANCEL_NOTIFICATION, status, 1, payload);
    }
}


/*************************************************************************
NAME
    gaia_handle_configuration_command

DESCRIPTION
    Handle a Gaia configuration command or return FALSE if we can't
*/
static bool gaia_handle_configuration_command(Task task, GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    UNUSED(task);
    switch (command->command_id)
    {
    case GAIA_COMMAND_SET_LED_CONFIGURATION:
        if (command->size_payload < 6)
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONFIGURATION);

        else
            gaia_set_led_config(command->size_payload, command->payload);

        return TRUE;


    case GAIA_COMMAND_GET_LED_CONFIGURATION:
        if (command->size_payload == 3)
            gaia_send_led_config(command->payload[0], (command->payload[1] << 8) |command->payload[2]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_LED_CONFIGURATION);

        return TRUE;


    case GAIA_COMMAND_SET_TONE_CONFIGURATION:
        if (command->size_payload == 3)
            gaia_set_tone_config((command->payload[0] << 8) | command->payload[1], command->payload[2]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_TONE_CONFIGURATION);

        return TRUE;


    case GAIA_COMMAND_GET_TONE_CONFIGURATION:
        if (command->size_payload == 2)
            gaia_send_tone_config((command->payload[0] << 8) | command->payload[1]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_TONE_CONFIGURATION);

        return TRUE;


    case GAIA_COMMAND_SET_DEFAULT_VOLUME:
        if (command->size_payload == 3)
            gaia_set_default_volumes(command->payload);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_DEFAULT_VOLUME);

        return TRUE;


    case GAIA_COMMAND_GET_DEFAULT_VOLUME:
        gaia_send_default_volumes();
        return TRUE;


    case GAIA_COMMAND_SET_VOICE_PROMPT_CONFIGURATION:
        if (command->size_payload == 7)
            gaia_set_ap_config(command->payload);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_VOICE_PROMPT_CONFIGURATION);

        return TRUE;


    case GAIA_COMMAND_GET_VOICE_PROMPT_CONFIGURATION:
        if (command->size_payload == 2)
            gaia_send_ap_config((command->payload[0] << 8) | command->payload[1]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_VOICE_PROMPT_CONFIGURATION);

        return TRUE;


    case GAIA_COMMAND_SET_TIMER_CONFIGURATION:
        gaia_set_timer_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_GET_TIMER_CONFIGURATION:
        gaia_send_timer_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_SET_AUDIO_GAIN_CONFIGURATION:
            gaia_set_audio_gain_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_GET_AUDIO_GAIN_CONFIGURATION:
        gaia_send_audio_gain_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_SET_VOLUME_CONFIGURATION:
        gaia_set_volume_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_GET_VOLUME_CONFIGURATION:
        gaia_send_volume_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_SET_USER_TONE_CONFIGURATION:
        gaia_set_user_tone_config(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_GET_USER_TONE_CONFIGURATION:
        if (command->size_payload == 1)
            gaia_send_user_tone_config(command->payload[0]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_USER_TONE_CONFIGURATION);

        return TRUE;

#ifdef ENABLE_SQIFVP
    case GAIA_COMMAND_GET_MOUNTED_PARTITIONS:
        {
            uint8 response[1];
            response[0] = sinkDataGetSquifMountedPartitions();
            gaia_send_success_payload(GAIA_COMMAND_GET_MOUNTED_PARTITIONS, 1, response);

            /*reread available partitions*/
            configManagerSqifPartitionsInit();
            return TRUE;
        }
#endif

    default:
        return FALSE;
    }
}

/*************************************************************************
NAME
    get_num_params_from_payload

DESCRIPTION
    decode the number of parameters from a pointer into the payload.
*/
static uint16 get_num_params_from_payload(uint8* payload)
{
    return MAKEWORD_HI_LO(payload[USER_EQ_NUM_PARAMS_HI_OFFSET], payload[USER_EQ_NUM_PARAMS_LO_OFFSET]);
}

/*************************************************************************
NAME
    get_param_id_from_payload

DESCRIPTION
    decode a user eq parameter id from a pointer into the payload.
*/
static audio_plugin_user_eq_param_id_t get_param_id_from_payload(uint8* payload)
{
    audio_plugin_user_eq_param_id_t param_id;
    uint16 id = MAKEWORD_HI_LO(payload[USER_EQ_PARAM_HI_OFFSET], payload[USER_EQ_PARAM_LO_OFFSET]);
    param_id.bank = (id >> 8) & 0x000F;
    param_id.band = (id >> 4) & 0x000F;
    param_id.param_type = id & 0x000F;
    return param_id;
}

/*************************************************************************
NAME
    get_unused_from_payload

DESCRIPTION
    decode the unused bits of parameter id from a pointer into the payload.
*/
static uint8 get_unused_from_payload(const uint8* payload)
{
    return (uint8)((payload[USER_EQ_PARAM_HI_OFFSET] >> 4) & 0x0f);
}

/*************************************************************************
NAME
    isMasterGainSelected

DESCRIPTION
     if band == 0 & param_type == 1, combination is used to select master gain for selected bank
*/
static bool isMasterGainSelected(const uint16 band, const eq_param_type_t param_type)
{
    return ((band == 0) && (param_type == user_eq_param_type_cutoff_freq));
}

/*************************************************************************
NAME
    get_param_value_from_payload

DESCRIPTION
    decode a user eq parameter value from a pointer into the payload.
*/
static uint16 get_param_value_from_payload(uint8* payload)
{
    return MAKEWORD_HI_LO(payload[USER_EQ_VALUE_HI_OFFSET], payload[USER_EQ_VALUE_LO_OFFSET]);
}

/*************************************************************************
NAME
    isParamValueSigned

DESCRIPTION
    is the parameter a signed value.
*/
static bool isParamValueSigned(const audio_plugin_user_eq_param_id_t* param_id)
{
    if (isMasterGainSelected(param_id->band, param_id->param_type))
    {
        return TRUE;
    }
    else
    {
        switch (param_id->param_type)
        {
            case eq_param_type_gain:
                /* gain is a signed value so must take care when changing from 16 to 32 bits */
                return TRUE;

            case eq_param_type_filter:
            case eq_param_type_cutoff_freq:
            case eq_param_type_q:
            default:
                return FALSE;
        }
    }
}

/*************************************************************************
NAME
    convertValueto32bit

DESCRIPTION
    convert the value provided by gaia to a 32 bit number.
*/
static uint32 convertValueTo32Bit(const uint16 value, const bool valueIsSigned)
{
    if (valueIsSigned)
    {
        if (value >= 0x8000)
            return (uint32)(value - 0x10000);
        else
            return (uint32)value;
    }
    else
    {
        return value;
    }
}

/*************************************************************************
NAME
    get_and_check_reserved_valid

DESCRIPTION
    decode the reserved bits of from a pointer into the payload.
*/
static bool get_and_check_reserved_valid(const uint8* payload, unsigned number_of_params)
{
    unsigned reserved_offset = (GAIA_NUM_BYTES_CMD_PER_PARAM * number_of_params) +
                                     GAIA_NUM_BYTES_PER_SET_GET_HDR;

    uint16 reserved = (MAKEWORD_HI_LO(payload[reserved_offset + SET_USER_EQ_GROUP_PARAM_RESERVED_HI_OFFSET],
                                      payload[reserved_offset + SET_USER_EQ_GROUP_PARAM_RESERVED_LO_OFFSET]));

    return (reserved & 0xFFFF) == 0;
}

/*************************************************************************
NAME
    validate_peq_payload

DESCRIPTION
    Validate a Parametric Equaliser control payload composed of a
    Parameter ID and a Parameter Value; see CS-215201-DC

    Parameter ID is (binary) 0000XXXXYYYYZZZZ where
        0000 must be zero
        XXXX is the filter bank to be addressed
        YYYY is the band of the filter
        ZZZZ is the parameter number

    Parameter Value meaning is dependent on the parameter number

    The ranges of parameters which depend on the filter type are not
    currently validated.
*/
static bool validate_peq_payload(const audio_plugin_user_eq_param_t* param, const uint16 unused)
{
    GAIA_DEBUG(("G: U EQ bnk=%u bnd=%u par=%u val=%lu\n",
                param->id.bank, param->id.band, param->id.param_type, param->value));

    /* check unused contains all 0's */
    if (unused & 0x000f)
    {
        return FALSE;
    }

    if (param->id.bank != USER_EQ_BANK_INDEX)
    {
    /*  Bank 0 represents the number of active banks, which cannot
     *  be changed by GAIA, and only Bank 1 may be modified by GAIA
     */
        return FALSE;
    }

    if (isMasterGainSelected(param->id.band, param->id.param_type))
    {
        return TRUE;
    }

    if (param->id.band < 1 || param->id.band > 5)
    {
    /*  Valid bands are 1 to 5  */
        return FALSE;
    }

    switch (param->id.param_type)
    {
    case 0: /*  Filter Type (high bound 13, Parametric Equaliser) */
        return param->value <= 13;

    case 1: /*  Filter centre frequency (high bound 20 kHz scaled by 3) */
    case 2: /*  Filter gain (signed, high bound +12 dB scaled by 60) */
    case 3: /*  Filter Q (high bound 8.0 scaled by 4096) */
        return TRUE;

    default: /*  Unknown parameter  */
        return FALSE;
    }
}

/*************************************************************************
NAME
    updateStoredPeq

DESCRIPTION
    Store user peq values in the global theSink.
*/
static void updateStoredPeq(audio_plugin_user_eq_param_t* param)
{
    if(param)
    {
        /* Only interested in  USER_EQ_BANK_INDEX */
        if (param->id.bank == USER_EQ_BANK_INDEX)
        {
            user_eq_bank_t* PEQ = sinkAudioGetPEQ();
            if(isMasterGainSelected(param->id.band, param->id.param_type))
            {
                PEQ->preGain = param->value;
            }
            else if((param->id.band > 0) && (param->id.band < 6))
            {
                switch(param->id.param_type)
                {
                    /* Update values shifting band index by 1 as GAIA EQ commands are intended only for bands 1-2-3-4-5 in EQ bank 1 */
                    case eq_param_type_filter:
                      PEQ->bands[param->id.band - 1].filter = param->value;
                      break;
                    case eq_param_type_cutoff_freq:
                      PEQ->bands[param->id.band - 1].freq = param->value;
                      break;
                    case eq_param_type_gain:
                      PEQ->bands[param->id.band - 1].gain = param->value;
                      break;
                    case eq_param_type_q:
                      PEQ->bands[param->id.band - 1].Q = param->value;
                      break;
                    default:
                      break;
                }
            }
        }
    }
}


/*************************************************************************
NAME
    gaia_av_remote_control

DESCRIPTION
    Handle GAIA_COMMAND_AV_REMOTE_CONTROL
*/

static void gaia_av_remote_control(uint8 size_payload, uint8 *payload)
{
    uint8 status = GAIA_STATUS_NOT_SUPPORTED;

#ifdef ENABLE_AVRCP
    status = GAIA_STATUS_INVALID_PARAMETER;
    if (size_payload == 1)
    {
        sinkEvents_t app_event = app_event_from_opid(payload[0]);

        if (app_event != EventInvalid)
        {
            send_app_message(app_event);
            status = GAIA_STATUS_SUCCESS;
        }
    }
#endif

    gaia_send_simple_response(GAIA_COMMAND_AV_REMOTE_CONTROL, status);
}


/*************************************************************************
NAME
    gaia_find_my_remote

DESCRIPTION
    Handle GAIA_COMMAND_FIND_MY_REMOTE

    GAIA client has requested that we start Find Me alert on a
    BLE HID Remote connected to this device.
*/
static void gaia_find_my_remote(uint8 alert_level)
{
    uint8 status = GAIA_STATUS_NOT_SUPPORTED;
#ifdef GATT_IAS_CLIENT
    switch (sinkGattIasClientSetAlert(alert_level, sink_gatt_ias_alert_remote))
    {
    case ias_alert_success:
        status = GAIA_STATUS_SUCCESS;
        break;

    case ias_alert_param_invalid:
        status = GAIA_STATUS_INVALID_PARAMETER;
        break;

    case ias_alert_not_supported:
    default:
        status = GAIA_STATUS_NOT_SUPPORTED;
        break;
    }
#else   /* GATT_IAS_CLIENT */
    UNUSED(alert_level);
#endif  /* GATT_IAS_CLIENT */
    gaia_send_simple_response(GAIA_COMMAND_FIND_MY_REMOTE, status);
}


/*************************************************************************/
static void handleGaiaCommandSetPowerState(uint8 size_payload, uint8 * payload)
{
    uint16 event = EventInvalid;

    if (size_payload == 0)
    {
        event = EventUsrPowerOff;
    }
    else if ((size_payload == 1) && (payload[0] == GAIA_POWER_STATE_OFF))
    {
        event = EventUsrPowerOff;
    }
    else if ((size_payload == 1) && (payload[0] == GAIA_POWER_STATE_ON))
    {
        event = EventUsrPowerOn;
    }

    if (event != EventInvalid)
    {
        send_app_message(event);
        gaia_send_success(GAIA_COMMAND_SET_POWER_STATE);
    }
    else
    {
        gaia_send_invalid_parameter(GAIA_COMMAND_SET_POWER_STATE);
    }
}

/*************************************************************************/
static void handleGaiaCommandGetPowerState(void)
{
    uint8 response[1];

    if (stateManagerGetState() == deviceLimbo)
    {
        response[0] = GAIA_POWER_STATE_OFF;
    }
    else
    {
        response[0] = GAIA_POWER_STATE_ON;
    }

    gaia_send_success_payload(GAIA_COMMAND_GET_POWER_STATE, 1, response);
}

/*************************************************************************
NAME
    sendNumBandsEqParam

DESCRIPTION
    Set the number of bands on the dsp equaliser.
*/
static void sendNumBandsEqParam(void)
{
    const Task audioTask = sinkAudioGetRoutedAudioTask();

    if (audioTask)
    {
        audio_plugin_user_eq_param_t param;
        param.id.bank = NUM_BANDS_BANK;
        param.id.band = NUM_BANDS_BAND;
        param.id.param_type = NUM_BANDS_PARAM_TYPE;
        param.value = MAX_EQ_BANDS;
        AudioSetUserEqParameter(audioTask, &param);
    }
    AudioApplyUserEqParameters(audioTask, TRUE);
}

/*************************************************************************
NAME
    toggleEqControlState

DESCRIPTION
    When this function is called the eq control should already be enabled. This function should
    therefore disable, then re-enable the eq control. The consequences of this is that the eq state
    shpuld be in the same state after the call as it was before the call was made.
*/
static void toggleEqControlState(void)
{
    send_app_message(EventUsrUserEqOnOffToggle);
    send_app_message(EventUsrUserEqOnOffToggle);
}

/*************************************************************************
NAME
    ActivateDspBands

DESCRIPTION
    In order for the applied eq paramters to have any effect, the peq must be functional - each band must be made active.
    This is achieved by sending the num_bands eq parameter. This is currently restricted to only be applicable for
    bank1, and is further restricted to 5 bands. Then the eq control state must be toggled off, then on again, to ensure
    the updated parameter is processed by the dsp.
    This is only required once, as after the number of bands update has been processed by the dsp its state persists.
*/
static void ActivateDspBands(void)
{
    if (!gaia_get_activate_peq_done())
    {
        sendNumBandsEqParam();
        toggleEqControlState();
        gaia_set_activate_peq_done(TRUE);
    }
}

/*************************************************************************
NAME
    process_user_eq_param

DESCRIPTION
    Validate the supplied parameter and send to AUDIO Plugin.

    If ack_required is TRUE then a gaia_send_success() message is sent upon successful completion.
    Failure acknowledgemnts are always sent.

    Returns TRUE if parameters are valid, and FALSE otherwise.
*/
static bool process_user_eq_param(audio_plugin_user_eq_param_t* param, uint16 unused, uint16 gaia_cmd)
{
    if (validate_peq_payload(param, unused))
    {
            ActivateDspBands();
        AudioSetUserEqParameter(sinkAudioGetRoutedAudioTask(), param);
                updateStoredPeq(param);

#if defined ENABLE_PEER && defined PEER_TWS
                /*Update the peer with the user EQ settings */
                peerSendUserEqSettings();
#endif

                return TRUE;
            }
            else
            {
        gaia_send_invalid_parameter(gaia_cmd);
        return FALSE;
        }
    }

/*************************************************************************
NAME
    isEnhancementEnabled

DESCRIPTION
    Return a bool indicating whether the specified enhancement is enabled
*/
static bool isEnhancementEnabled(uint16 enhancement_bypass_mask)
{
    return (getMusicProcessingBypassFlags() & enhancement_bypass_mask) ? 0 : 1;
}

/*************************************************************************
NAME 
    isGetSetPayloadNotNull
 
DESCRIPTION
    Check the if the payload received in the 
    GAIA_COMMAND_GET_USER_EQ_PARAMETER 
    GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETERS
    GAIA_COMMAND_SET_USER_EQ_PARAMETER 
    GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETERS
    command is not NULL.
    TRUE if not NULL, FALSE otherwise
*/
static bool isGetSetPayloadNotNull(uint8* payload)
{
    return (payload != NULL);
}

/*************************************************************************
NAME 
    isGetParamSizeValid
 
DESCRIPTION
    Check the payload size matches the minimum payload size required for GAIA_COMMAND_GET_USER_EQ_PARAMETER command,
    if it does return TRUE, FALSE otherwise;
*/
static bool isGetParamSizeValid(uint16 size_payload)
{
    bool valid = FALSE;
    if(size_payload == GAIA_GET_PARAM_PAYLOAD_SIZE)
    {
        valid = TRUE;
    }
    return valid;
}

/*************************************************************************
NAME 
    isGetGroupParamSizeGreaterThanMin
 
DESCRIPTION
    Check the payload size matches the minimum payload size required for GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETERS command,
    if it does return TRUE, FALSE otherwise;
*/
static bool isGetGroupParamSizeGreaterThanMin(uint16 size_payload)
{
    bool valid = FALSE;
    if(size_payload >= MIN_GAIA_GET_GROUP_PARAM_PAYLOAD_SIZE)
    {
        valid = TRUE;
    }
    return valid;
}

/*************************************************************************
NAME 
    isSetParamSizeValid
 
DESCRIPTION
    Check the payload size matches the minimum payload size required for GAIA_COMMAND_SET_USER_EQ_PARAMETER command,
    if it does return TRUE, FALSE otherwise;
*/
static bool isSetParamSizeValid(uint16 size_payload)
{
    bool valid = FALSE;
    if(size_payload == GAIA_SET_PARAM_PAYLOAD_SIZE)
    {
        valid = TRUE;
    }
    return valid;
}

/*************************************************************************
NAME 
    isSetGroupParamSizeGreaterThanMin
 
DESCRIPTION
    Check the payload size matches the minimum payload size required for GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETERS command,
    if it does return TRUE, FALSE otherwise;
*/
static bool isSetGroupParamSizeGreaterThanMin(uint16 size_payload)
{
    bool valid = FALSE;
    if(size_payload >= MIN_GAIA_SET_GROUP_PARAM_PAYLOAD_SIZE)
    {
        valid = TRUE;
    }
    return valid;
}

/*************************************************************************
NAME 
    isGetParamPayloadValid
 
DESCRIPTION
    Check the payload received in the GAIA_COMMAND_GET_USER_EQ_PARAMETER command is valid.
*/
static bool isGetParamPayloadValid(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    bool valid = FALSE;

    if(isGetSetPayloadNotNull(command->payload))
    {
        if (isGetParamSizeValid(command->size_payload))
        {
            valid = TRUE;
        }
    }
    return valid;
}

/*************************************************************************
NAME 
    getNumberOfParamsFromGetGroupParamCommand
 
DESCRIPTION
    Get the number of parameters from the GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETERS command,
*/
static unsigned getNumberOfParamsFromGetGroupParamCommand(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    unsigned number_of_params = 0;

    if (isGetGroupParamSizeGreaterThanMin(command->size_payload))
    {
        number_of_params = get_num_params_from_payload(command->payload);
    }
    return number_of_params;
}

/*************************************************************************
NAME 
    isSetParamPayloadValid
 
DESCRIPTION
    Check the payload received in the GAIA_COMMAND_SET_USER_EQ_PARAMETER command is valid.
*/
static bool isSetParamPayloadValid(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    bool valid = FALSE;

    if(isGetSetPayloadNotNull(command->payload))
    {
        if (isSetParamSizeValid(command->size_payload))
        {
            valid = TRUE;
        }
            }
    return valid;
        }

/*************************************************************************
NAME 
    getNumberOfParamsFromSetGroupParamCommand
 
DESCRIPTION
    Get the number of parameters from the GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETERS command,
*/
static unsigned getNumberOfParamsFromSetGroupParamCommand(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    unsigned number_of_params = 0;

    if (isSetGroupParamSizeGreaterThanMin(command->size_payload))
        {
        number_of_params = get_num_params_from_payload(command->payload);
        }

    return number_of_params;
    }

static bool isGetGroupParamPayloadSizeValidForNumberOfParams(unsigned number_of_params, unsigned size_payload)
    {
    unsigned expected_payload_size = (GAIA_NUM_BYTES_CMD_PER_PARAM * number_of_params) + GAIA_NUM_BYTES_PER_SET_GET_HDR;

    return expected_payload_size == size_payload;
    }

static bool isSetGroupParamPayloadSizeValidForNumberOfParams(unsigned number_of_params, unsigned size_payload)
{
    unsigned expected_payload_size = (GAIA_NUM_BYTES_CMD_PER_PARAM * number_of_params) +
                                     GAIA_NUM_BYTES_PER_SET_GET_HDR + GAIA_NUM_BYTES_SET_GROUP_PARAM_RESERVED;

    return expected_payload_size == size_payload;
}

/*************************************************************************
NAME 
    isGetGroupParamsPayloadValid
 
DESCRIPTION
    Return TRUE if the payload received in the GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETERS command
    is valid, FALSE otherwise.
*/
static bool isGetGroupParamsPayloadValid(GAIA_UNHANDLED_COMMAND_IND_T *command)
    {
    bool valid = FALSE;

    if(isGetSetPayloadNotNull(command->payload))
{
        unsigned number_of_params_in_command = getNumberOfParamsFromGetGroupParamCommand(command);
        if(number_of_params_in_command > 0)
    {
            valid = isGetGroupParamPayloadSizeValidForNumberOfParams(number_of_params_in_command, command->size_payload);
        }
    }
    return valid;
}

/*************************************************************************
NAME 
    isSetGroupParamsPayloadValid

DESCRIPTION
    Return TRUE if the payload received in the GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETERS command 
    is valid, FALSE otherwise.
*/
static bool isSetGroupParamsPayloadValid(GAIA_UNHANDLED_COMMAND_IND_T *command)
        {
    bool valid = FALSE;

    if(isGetSetPayloadNotNull(command->payload))
    {
        unsigned number_of_params_in_command = getNumberOfParamsFromSetGroupParamCommand(command);
        bool reserved_valid = get_and_check_reserved_valid(command->payload, number_of_params_in_command);

        if(number_of_params_in_command > 0 && reserved_valid)
            {
            valid = isSetGroupParamPayloadSizeValidForNumberOfParams(number_of_params_in_command, command->size_payload);
        }
            }
    return valid;
        }

/*************************************************************************
NAME 
    gaia_get_user_eq_param
 
DESCRIPTION
    Handler for GAIA_COMMAND_GET_USER_EQ_PARAMETER message
*/
static void gaia_get_user_eq_param(GAIA_UNHANDLED_COMMAND_IND_T* command)
{
    if(sinkAudioGetRoutedAudioTask())
    {
        if(isGetParamPayloadValid(command) != 0)
        {
            audio_plugin_user_eq_param_id_t param = get_param_id_from_payload(command->payload);
            AudioGetUserEqParameter(sinkAudioGetRoutedAudioTask(), &param, &theSink.task);
        }
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_USER_EQ_PARAMETER);
        }
    }
    else
    {
         gaia_send_incorrect_state(GAIA_COMMAND_GET_USER_EQ_PARAMETER);
    }
}

/*************************************************************************
NAME 
    gaia_get_user_eq_group_params
 
DESCRIPTION
    Handler for GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETERS message
*/
static void gaia_get_user_eq_group_params(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    if (sinkAudioGetRoutedAudioTask())
    {
        if(isGetGroupParamsPayloadValid(command)) 
        {
            uint16 number_of_params = get_num_params_from_payload(command->payload);
            audio_plugin_user_eq_param_id_t* param_ids = malloc(number_of_params * sizeof(audio_plugin_user_eq_param_id_t));

            if (param_ids)
            {
                unsigned i;
                for (i = 0; i < number_of_params; i++)
                {
                    unsigned param_offset = GAIA_NUM_BYTES_PER_SET_GET_HDR + (i * GAIA_NUM_BYTES_CMD_PER_PARAM);
                    param_ids[i]= get_param_id_from_payload(&command->payload[param_offset]);
                }
                AudioGetUserEqParameters(sinkAudioGetRoutedAudioTask(), number_of_params, param_ids, &theSink.task);
                free(param_ids);
            }
            else
            {
                gaia_send_insufficient_resources(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
            }
        }
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
        }
    }
    else
    {
        gaia_send_incorrect_state(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER); 
    }
    /* no "gaia_send_success" as this is done when message is returned from DSP */
}

/*************************************************************************
NAME 
    gaia_set_user_eq_param

DESCRIPTION
    Handler for GAIA_COMMAND_SET_USER_EQ_PARAMETER message
*/
static void gaia_set_user_eq_param(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    if(sinkAudioGetRoutedAudioTask())
    {
        if(isSetParamPayloadValid(command) != 0)
        {
            audio_plugin_user_eq_param_t param;
            uint16 unused;

            param.id = get_param_id_from_payload(command->payload);
            param.value = convertValueTo32Bit(get_param_value_from_payload(command->payload), isParamValueSigned(&param.id));

            unused = get_unused_from_payload(command->payload);

            if(process_user_eq_param(&param, unused, GAIA_COMMAND_SET_USER_EQ_PARAMETER))
            {
                bool recalculate_coefficients = command->payload[SET_USER_EQ_RECALC];
                AudioApplyUserEqParameters(sinkAudioGetRoutedAudioTask(), recalculate_coefficients);
                gaia_send_success(GAIA_COMMAND_SET_USER_EQ_PARAMETER);
            }
        }
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_USER_EQ_PARAMETER);
        }
    }
    else
    {
        gaia_send_incorrect_state(GAIA_COMMAND_SET_USER_EQ_PARAMETER); 
    }
}

/*************************************************************************
NAME
    gaia_set_user_eq_group_params

DESCRIPTION
    Handler for GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETERS message
*/
static void gaia_set_user_eq_group_params(GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    if(sinkAudioGetRoutedAudioTask())
    {
        if(isSetGroupParamsPayloadValid(command))
        {
            unsigned i;
            bool args_ok = TRUE;
            uint16 number_of_params = get_num_params_from_payload(command->payload);

            for(i = 0; i < number_of_params; i++)
            {
                unsigned param_offset = GAIA_NUM_BYTES_PER_SET_GET_HDR + (i * GAIA_NUM_BYTES_CMD_PER_PARAM);
                audio_plugin_user_eq_param_t param;
                uint16 unused;

                param.id = get_param_id_from_payload(&command->payload[param_offset]);
                param.value = convertValueTo32Bit(get_param_value_from_payload(&command->payload[param_offset]),isParamValueSigned(&param.id));
                unused = get_unused_from_payload(&command->payload[param_offset]);

                if (!process_user_eq_param(&param, unused, GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETER))
                {
                    args_ok = FALSE;
                    break;
                }
            }

            if(args_ok)
            {
                gaia_send_success(GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETER);
                AudioApplyUserEqParameters(sinkAudioGetRoutedAudioTask(), TRUE);
            }
            else
            {
                AudioClearUserEqParameters(sinkAudioGetRoutedAudioTask());
            }
        }
        else
        {
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETER);
        }
    }
    else
{
        gaia_send_incorrect_state(GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETER); 
    }
}

/*************************************************************************
NAME
    gaia_handle_control_command

DESCRIPTION
    Handle a Gaia control command or return FALSE if we can't
*/
static bool gaia_handle_control_command(Task task, GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    uint8 response[1];
    uint8 num_lang = SinkAudioPromptsGetNumAudioPromptLanguages();

    UNUSED(task);

    switch (command->command_id)
    {
    case GAIA_COMMAND_DEVICE_RESET:
        gaia_device_reset();
        return TRUE;

    case GAIA_COMMAND_CHANGE_VOLUME:
        if (command->size_payload == 1)
            gaia_change_volume(command->payload[0]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_CHANGE_VOLUME);

        return TRUE;


    case GAIA_COMMAND_ALERT_LEDS:
        if (command->size_payload == 13)
            gaia_alert_leds(command->payload);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_LEDS);

        return TRUE;


    case GAIA_COMMAND_ALERT_TONE:
        gaia_alert_tone(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_ALERT_EVENT:
        if (command->size_payload == 2)
            gaia_alert_event((command->payload[0] << 8) | command->payload[1]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_EVENT);

        return TRUE;


    case GAIA_COMMAND_ALERT_VOICE:
        if (num_lang == 0)
            return FALSE;

        if (command->size_payload >= 2)
            gaia_alert_voice(command->size_payload, command->payload);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_ALERT_VOICE);

        return TRUE;

    case GAIA_COMMAND_SET_POWER_STATE:
        handleGaiaCommandSetPowerState(command->size_payload, command->payload);
        return TRUE;

    case GAIA_COMMAND_GET_POWER_STATE:
        handleGaiaCommandGetPowerState();
        return TRUE;

    case GAIA_COMMAND_SET_VOLUME_ORIENTATION:
        if ((command->size_payload == 1) && (command->payload[0] == 0))
        {
            if (sinkVolumeIsVolButtonInverted())
            {
                sinkVolumeSetVolButtonInverted(FALSE);
            }

            gaia_send_success(GAIA_COMMAND_SET_VOLUME_ORIENTATION);
        }

        else if ((command->size_payload == 1) && (command->payload[0] == 1))
        {
            if (!sinkVolumeIsVolButtonInverted())
            {
                sinkVolumeSetVolButtonInverted(TRUE);
            }

            gaia_send_success(GAIA_COMMAND_SET_VOLUME_ORIENTATION);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_VOLUME_ORIENTATION);

        return TRUE;


    case GAIA_COMMAND_GET_VOLUME_ORIENTATION:
        response[0] = sinkVolumeIsVolButtonInverted();
        gaia_send_success_payload(GAIA_COMMAND_GET_VOLUME_ORIENTATION, 1, response);
        return TRUE;


    case GAIA_COMMAND_SET_LED_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] == 0))
        {
            GAIA_DEBUG(("G: SET_LED_CONTROL: 0\n"));
            send_app_message(EventUsrLedsOff);
            gaia_send_success(GAIA_COMMAND_SET_LED_CONTROL);
        }

        else if ((command->size_payload == 1) && (command->payload[0] == 1))
        {
            GAIA_DEBUG(("G: SET_LED_CONTROL: 1\n"));
            send_app_message(EventUsrLedsOn);
            gaia_send_success(GAIA_COMMAND_SET_LED_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_LED_CONTROL);

        return TRUE;


    case GAIA_COMMAND_GET_LED_CONTROL:
        response[0] = LedManagerIsEnabled();
        gaia_send_success_payload(GAIA_COMMAND_GET_LED_CONTROL, 1, response);
        return TRUE;

    case GAIA_COMMAND_PLAY_TONE:
        if (command->size_payload == 1)
        {
            TonesPlayTone(command->payload[0], TRUE);
            gaia_send_success(GAIA_COMMAND_PLAY_TONE);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_PLAY_TONE);

        return TRUE;


    case GAIA_COMMAND_SET_VOICE_PROMPT_CONTROL:
        if ((command->size_payload == 1) && ((command->payload[0] == 0) || (command->payload[0] == 1)))
        {
            GAIA_DEBUG(("G: SET_VOICE_PROMPT_CONTROL: %d\n", command->payload[0]));
            if (SinkAudioPromptsIsEnabled() != command->payload[0])
            {
                SinkAudioPromptsSetEnabled(command->payload[0]);
            }

            gaia_send_success(GAIA_COMMAND_SET_VOICE_PROMPT_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_VOICE_PROMPT_CONTROL);

        return TRUE;


    case GAIA_COMMAND_GET_VOICE_PROMPT_CONTROL:
        response[0] = SinkAudioPromptsIsEnabled();
        gaia_send_success_payload(GAIA_COMMAND_GET_VOICE_PROMPT_CONTROL, 1, response);
        return TRUE;

    case GAIA_COMMAND_CHANGE_AUDIO_PROMPT_LANGUAGE:
        if (num_lang == 0)
            return FALSE;

        send_app_message(EventUsrSelectAudioPromptLanguageMode);
        gaia_send_success(GAIA_COMMAND_CHANGE_AUDIO_PROMPT_LANGUAGE);
        return TRUE;


    case GAIA_COMMAND_SET_AUDIO_PROMPT_LANGUAGE:
        if (num_lang == 0)
            return FALSE;

        if ((command->size_payload == 1) && (command->payload[0] < num_lang))
        {
            SinkAudioPromptsSetLanguage(command->payload[0]);
            gaia_send_success(GAIA_COMMAND_SET_AUDIO_PROMPT_LANGUAGE);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_AUDIO_PROMPT_LANGUAGE);

        return TRUE;


    case GAIA_COMMAND_GET_AUDIO_PROMPT_LANGUAGE:
        if (num_lang == 0)
            return FALSE;

        response[0] = SinkAudioPromptsGetLanguage();
        gaia_send_success_payload(GAIA_COMMAND_GET_AUDIO_PROMPT_LANGUAGE, 1, response);
        return TRUE;

#ifdef ENABLE_SPEECH_RECOGNITION
    case GAIA_COMMAND_SET_SPEECH_RECOGNITION_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrSSROn : EventUsrSSROff);
            gaia_send_success(GAIA_COMMAND_SET_SPEECH_RECOGNITION_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_SPEECH_RECOGNITION_CONTROL);

        return TRUE;

    case GAIA_COMMAND_GET_SPEECH_RECOGNITION_CONTROL:
        response[0] = speechRecognitionIsSSREnabled();
        gaia_send_success_payload(GAIA_COMMAND_GET_SPEECH_RECOGNITION_CONTROL, 1, response);
        return TRUE;

    case GAIA_COMMAND_START_SPEECH_RECOGNITION:
        speechRecognitionStart();
        gaia_send_success(GAIA_COMMAND_START_SPEECH_RECOGNITION);
        return TRUE;
#endif

    case GAIA_COMMAND_SWITCH_EQ_CONTROL:
        send_app_message(EventUsrSelectNextUserEQBank);
        gaia_send_success(GAIA_COMMAND_SWITCH_EQ_CONTROL);
        return TRUE;


    case GAIA_COMMAND_SET_EQ_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] <= A2DP_MUSIC_MAX_EQ_BANK))
        {
            set_abs_eq_bank(command->payload[0]);
            gaia_send_success(GAIA_COMMAND_SET_EQ_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_EQ_CONTROL);

        return TRUE;


    case GAIA_COMMAND_GET_EQ_CONTROL:
        response[0] = getMusicProcessingMode();
        if (response[0] < A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0)
            gaia_send_insufficient_resources(GAIA_COMMAND_GET_EQ_CONTROL);

        else
        {
            response[0] -= A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0;
            gaia_send_success_payload(GAIA_COMMAND_GET_EQ_CONTROL, 1, response);
        }
        return TRUE;


    case GAIA_COMMAND_SET_BASS_BOOST_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrBassEnhanceOn : EventUsrBassEnhanceOff);
            gaia_send_success(GAIA_COMMAND_SET_BASS_BOOST_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_BASS_BOOST_CONTROL);

        return TRUE;


    case GAIA_COMMAND_GET_BASS_BOOST_CONTROL:
        response[0] = isEnhancementEnabled(MUSIC_CONFIG_BASS_ENHANCE_BYPASS);
        gaia_send_success_payload(GAIA_COMMAND_GET_BASS_BOOST_CONTROL, 1, response);
        return TRUE;


    case GAIA_COMMAND_TOGGLE_BASS_BOOST_CONTROL:
        send_app_message(EventUsrBassEnhanceEnableDisableToggle);
        gaia_send_success(GAIA_COMMAND_TOGGLE_BASS_BOOST_CONTROL);
        return TRUE;


    case GAIA_COMMAND_SET_3D_ENHANCEMENT_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsr3DEnhancementOn : EventUsr3DEnhancementOff);
            gaia_send_success(GAIA_COMMAND_SET_3D_ENHANCEMENT_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_3D_ENHANCEMENT_CONTROL);

        return TRUE;


    case GAIA_COMMAND_GET_3D_ENHANCEMENT_CONTROL:
        response[0] = isEnhancementEnabled(MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS);
        gaia_send_success_payload(GAIA_COMMAND_GET_3D_ENHANCEMENT_CONTROL, 1, response);
        return TRUE;


    case GAIA_COMMAND_TOGGLE_3D_ENHANCEMENT_CONTROL:
        send_app_message(EventUsr3DEnhancementEnableDisableToggle);
        gaia_send_success(GAIA_COMMAND_TOGGLE_3D_ENHANCEMENT_CONTROL);
        return TRUE;


    case GAIA_COMMAND_GET_USER_EQ_CONTROL:
         response[0] = isEnhancementEnabled(MUSIC_CONFIG_USER_EQ_BYPASS);
         gaia_send_success_payload(GAIA_COMMAND_GET_USER_EQ_CONTROL, 1, response);
        return TRUE;


    case GAIA_COMMAND_SET_USER_EQ_CONTROL:
        if ((command->size_payload == 1) && (command->payload[0] < 2))
        {
            send_app_message(command->payload[0] ? EventUsrUserEqOn : EventUsrUserEqOff);
            gaia_send_success(GAIA_COMMAND_SET_USER_EQ_CONTROL);
        }

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_USER_EQ_CONTROL);

        return TRUE;


    case GAIA_COMMAND_TOGGLE_USER_EQ_CONTROL:
        send_app_message(EventUsrUserEqOnOffToggle);
        gaia_send_success(GAIA_COMMAND_TOGGLE_USER_EQ_CONTROL);
        return TRUE;


    case GAIA_COMMAND_SET_USER_EQ_PARAMETER:
    {
        gaia_set_user_eq_param(command);
        return TRUE;
    }

    case GAIA_COMMAND_SET_USER_EQ_GROUP_PARAMETER:
    {
        gaia_set_user_eq_group_params(command);
        return TRUE;
    }

    case GAIA_COMMAND_GET_USER_EQ_PARAMETER:
    {
        gaia_get_user_eq_param(command);
        return TRUE;
    }

    case GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER:
    {
        gaia_get_user_eq_group_params(command);
        return TRUE;
    }

    case GAIA_COMMAND_ENTER_BT_PAIRING_MODE:
    {
        sinkState sink_state = stateManagerGetState();

        if (sink_state == deviceLimbo || sink_state == deviceConnDiscoverable)
        {
            gaia_send_incorrect_state(GAIA_COMMAND_ENTER_BT_PAIRING_MODE);
        }

        else
        {
            send_app_message(EventUsrEnterPairing);
            gaia_send_success(GAIA_COMMAND_ENTER_BT_PAIRING_MODE);
        }
        return TRUE;
    }

#if defined ENABLE_PEER && defined PEER_TWS
    case GAIA_COMMAND_SET_TWS_AUDIO_ROUTING:
        if (command->size_payload == 2 && command->payload[0] < 2 && command->payload[1] < 4)
            gaia_set_tws_routing(command->payload[0], command->payload[1]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_SET_TWS_AUDIO_ROUTING);

        return TRUE;


    case GAIA_COMMAND_GET_TWS_AUDIO_ROUTING:
        if (command->size_payload == 1 && command->payload[0] < 2)
            gaia_send_tws_routing(command->payload[0]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_GET_TWS_AUDIO_ROUTING);

        return TRUE;


    case GAIA_COMMAND_TRIM_TWS_VOLUME:
        if (command->size_payload == 2)
            gaia_trim_tws_volume(command->payload[0], command->payload[1]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_TRIM_TWS_VOLUME);

        return TRUE;


    case GAIA_COMMAND_SET_PEER_LINK_RESERVED:
        gaia_set_peer_link_reserved(command->size_payload, command->payload);
        return TRUE;


    case GAIA_COMMAND_GET_PEER_LINK_RESERVED:
        response[0] = peerGetLinkReserved();
        gaia_send_success_payload(GAIA_COMMAND_GET_PEER_LINK_RESERVED, 1, response);
        return TRUE;

#endif /* defined ENABLE_PEER && defined PEER_TWS */

    case GAIA_COMMAND_AV_REMOTE_CONTROL:
        gaia_av_remote_control(command->size_payload, command->payload);
        return TRUE;

    case GAIA_COMMAND_FIND_MY_REMOTE:
        if (command->size_payload == 1)
            gaia_find_my_remote(command->payload[0]);

        else
            gaia_send_invalid_parameter(GAIA_COMMAND_FIND_MY_REMOTE);

        return TRUE;

    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    gaia_handle_status_command

DESCRIPTION
    Handle a Gaia polled status command or return FALSE if we can't
*/
static bool gaia_handle_status_command(Task task, GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    UNUSED(task);
    
    switch (command->command_id)
    {
    case GAIA_COMMAND_GET_APPLICATION_VERSION:
        gaia_send_application_version();
        return TRUE;

#ifdef PEER_TWS
    case GAIA_COMMAND_GET_TWS_FORCED_DOWNMIX_MODE:
        gaia_send_tws_forced_downmix_mode();
        return TRUE;
#endif
        
    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    gaia_handle_notification_command

DESCRIPTION
    Handle a Gaia notification command or return FALSE if we can't
    Notification acknowledgements are swallowed
*/
static bool gaia_handle_notification_command(Task task, GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    UNUSED(task);
    if (command->command_id & GAIA_ACK_MASK)
    {
        GAIA_DEBUG(("G: NOTIF ACK\n"));
        return TRUE;
    }

    switch (command->command_id)
    {
    case GAIA_COMMAND_REGISTER_NOTIFICATION:
        gaia_register_notification(command->size_payload, command->payload);
        return TRUE;

    case GAIA_COMMAND_GET_NOTIFICATION:
        gaia_get_notification(command->size_payload, command->payload);
        return TRUE;

    case GAIA_COMMAND_CANCEL_NOTIFICATION:
        gaia_cancel_notification(command->size_payload, command->payload);
        return TRUE;

    case GAIA_EVENT_NOTIFICATION:
        gaia_send_invalid_parameter(GAIA_EVENT_NOTIFICATION);
        return TRUE;

    default:
        return FALSE;
    }
}


/*************************************************************************
NAME
    gaia_handle_command

DESCRIPTION
    Handle a GAIA_UNHANDLED_COMMAND_IND from the Gaia library
*/
static void gaia_handle_command(Task task, GAIA_UNHANDLED_COMMAND_IND_T *command)
{
    bool handled = FALSE;

    GAIA_DEBUG(("G: cmd: %04x:%04x %d\n",
                command->vendor_id, command->command_id, command->size_payload));

    if (command->vendor_id == GAIA_VENDOR_QTIL)
    {
        switch (command->command_id & GAIA_COMMAND_TYPE_MASK)
        {
        case GAIA_COMMAND_TYPE_CONFIGURATION:
            handled = gaia_handle_configuration_command(task, command);
            break;

        case GAIA_COMMAND_TYPE_CONTROL:
            handled = gaia_handle_control_command(task, command);
            break;

        case GAIA_COMMAND_TYPE_STATUS :
            handled = gaia_handle_status_command(task, command);
            break;

        case GAIA_COMMAND_TYPE_NOTIFICATION:
             handled = gaia_handle_notification_command(task, command);
            break;

        default:
            GAIA_DEBUG(("G: type unknown\n"));
            break;
        }
    }

    if (!handled && !gaia_is_acknowledgement(command->command_id))
    {
        gaia_send_response(command->vendor_id, command->command_id, GAIA_STATUS_NOT_SUPPORTED, 0, NULL);
    }
}


/*************************************************************************
NAME
    gaia_handle_disconnect

DESCRIPTION
    Handle a disconnection from the Gaia library
    Cancel all notifications and clear the transport
*/
static void gaia_handle_disconnect(void)
{
    gaia_clear_pio_change_mask();
    gaia_set_notify_battery_charged(FALSE);
    gaia_set_notify_charger_connection(FALSE);
    gaia_set_notify_ui_event(FALSE);
    gaia_set_notify_speech_rec(FALSE);
    gaia_set_transport(NULL);
}



/*************************************************************************
NAME
    gaiaApplyGattUpgradeOptimisations

DESCRIPTION
    Apply Upgrade optimisations when GATT transport is used
*/
static void gaiaApplyGattUpgradeOptimisations(void)
{
    uint16 cid = GaiaGetCidOverGattTransport();
     
    /*Upgrade optimisations do not apply if Active SCO is present*/
    if ((cid!=INVALID_CID) && (gaiaIsGattUpgradeInProgress()) && (!audioRouteIsScoActive()))
    {        
        /*Apply new BLE params for Upgrade*/
        sinkBleApplyParamsForUpgrade(cid, TRUE);
        sinkA2dpSuspendAll();
        
#ifdef ENABLE_PBAP    
        /*Disconnect from all PBAP Servers */  
        pbapDisconnect();
#endif

 #ifdef ENABLE_MAPC 
        /*Disable Map Message Notification Service */
        mapcDisconnectMns();   
 #endif
    }
}

/*************************************************************************
NAME
    gaiaRevertGattUpgradeOptimisations

DESCRIPTION
    Revert any BLE optimisations when Upgrade is completed
    Note A2DP will not be resumed in case of Upgrade success or abort
*/
static void gaiaRevertGattUpgradeOptimisations(void)
{
    uint16 cid = GaiaGetCidOverGattTransport();

    if (cid!=INVALID_CID)
    {           
        /*Revert BLE params, if applied*/
        sinkBleApplyParamsForUpgrade(cid, FALSE);
    }
#ifdef ENABLE_PEER
    sinkA2dpResumePeerSession();
#endif
}


/*************************************************************************
NAME
    gaiaResetUpgradeInProgress

DESCRIPTION
    Reset Upgrade In progress flag
*/
void gaiaResetUpgradeInProgress(void)
{
    if (gaiaIsGattUpgradeInProgress())
    {    
        gaiaSetGattUpgradeInProgress(FALSE);
    }
}

/*************************************************************************
NAME
    handleGaiaMessage

DESCRIPTION
    Handle messages passed up from the Gaia library
*/
void handleGaiaMessage(Task task, MessageId id, Message message)
{
    switch (id)
    {
    case GAIA_INIT_CFM:
        {
            GAIA_INIT_CFM_T *m = (GAIA_INIT_CFM_T *) message;

            GAIA_DEBUG(("G: _INIT_CFM: %d\n", m->success));
            GaiaSetAppWillHandleCommand(GAIA_COMMAND_DEVICE_RESET, TRUE);
#ifdef CVC_PRODTEST
            if (BootGetMode() != BOOTMODE_CVC_PRODTEST)
            {
                GAIA_DEBUG(("G: StartService\n"));
                if (m->success)
                {
#if defined ENABLE_GAIA_RFCOMM
                    GaiaStartService(gaia_transport_rfcomm);
#else
                    GaiaStartService(gaia_transport_spp);
#endif /* ENABLE_GAIA_RFCOMM */
                }
            }
            else
            {
                if (m->success)
                {
                    GAIA_DEBUG(("G: CVC_PRODTEST_PASS\n"));
                    exit(CVC_PRODTEST_PASS);
                }
                else
                {
                    GAIA_DEBUG(("G: CVC_PRODTEST_FAIL\n"));
                    exit(CVC_PRODTEST_FAIL);
                }
            }
#else
            if (m->success)
            {
                GaiaSetApiMinorVersion(GAIA_API_MINOR_VERSION);
#if defined ENABLE_GAIA_RFCOMM
                GaiaStartService(gaia_transport_rfcomm);
#else
                GaiaStartService(gaia_transport_spp);
#endif /* ENABLE_GAIA_RFCOMM */
            }
#endif /*CVC_PRODTEST */
        }
        break;

    case GAIA_CONNECT_IND:
        {
            bool enable = gaia_get_enable_session();
            GAIA_CONNECT_IND_T *m = (GAIA_CONNECT_IND_T *) message;
            GAIA_DEBUG(("G: _CONNECT_IND: %p s=%u e=%u\n",
                        (void *) m->transport,
                        m->success,
                        enable));

            if (m->success)
            {
                gaia_set_transport(m->transport);
                GaiaSetSessionEnable(m->transport, enable);
                if(enable)
                {
                    GaiaOnTransportConnect(m->transport);
                }
            }
        }
        break;

    case GAIA_DISCONNECT_IND:
        {
            GAIA_DISCONNECT_IND_T *m = (GAIA_DISCONNECT_IND_T *) message;
            GAIA_DEBUG(("G: _DISCONNECT_IND: %p\n", (void *) m->transport));
            
            gaia_handle_disconnect();
            GaiaDisconnectResponse(m->transport);
            gaiaResetUpgradeInProgress();
        }
        break;

    case GAIA_DISCONNECT_CFM:
        {
#ifdef DEBUG_GAIA
            GAIA_DISCONNECT_CFM_T *m = (GAIA_DISCONNECT_CFM_T *) message;
            GAIA_DEBUG(("G: _DISCONNECT_CFM: %p\n", (void *) m->transport));
#endif
            gaia_handle_disconnect();
            gaiaResetUpgradeInProgress();
        }
        break;

    case GAIA_START_SERVICE_CFM:
        GAIA_DEBUG(("G: _START_SERVICE_CFM: %d %d\n", ((GAIA_START_SERVICE_CFM_T *)message)->success, ((GAIA_START_SERVICE_CFM_T *)message)->transport_type));
        break;

    case GAIA_UNHANDLED_COMMAND_IND:
        GAIA_DEBUG(("G: _UNHANDLED_COMMAND_IND\n"));
        gaia_handle_command(task, (GAIA_UNHANDLED_COMMAND_IND_T *) message);
        break;

    case GAIA_SEND_PACKET_CFM:
        {
            GAIA_SEND_PACKET_CFM_T *m = (GAIA_SEND_PACKET_CFM_T *) message;
            GAIA_DEBUG(("G: _SEND_PACKET_CFM: s=%d\n", VmGetAvailableAllocations()));

            if (m->packet)
                freePanic(m->packet);
        }
        break;

    case GAIA_DFU_IND:
        {
            GAIA_DFU_IND_T *ind = (GAIA_DFU_IND_T *) message;
            Sink sink;

            GAIA_DEBUG(("G: _DFU_IND: state %u\n", ind->state));

            sink = GaiaTransportGetSink(ind->transport);
            if (sink)
            {
                if (ind->state == GAIA_DFU_STATE_DOWNLOAD)
                {
                /*  Set link policy to expedite download  */
                    linkPolicySetDfuActiveMode(sink);
                }
                else
                {
                    linkPolicyDfuAccessComplete(sink);
                }
            }
            else
            {
                GAIA_DEBUG(("G: no sink\n"));
            }
        }
        break;

    case GAIA_DFU_CFM:
        {
            GAIA_DFU_CFM_T *m = (GAIA_DFU_CFM_T *) message;
            GAIA_DEBUG(("G: _DFU_CFM: s=%u\n", m->success));
            if (!m->success)
            {
                send_app_message(EventSysError);
            }
        }
        break;

    case GAIA_UPGRADE_CONNECT_IND:
        {
            GAIA_UPGRADE_CONNECT_IND_T *ind = (GAIA_UPGRADE_CONNECT_IND_T *) message;

            GAIA_DEBUG(("G: GAIA_UPGRADE_CONNECT_IND type 0x%x\n", ind->transport_type));

            if(ind->transport_type != gaia_transport_none)
            {
                sinkUpgradeSetUpgradeTransportType((uint16)ind->transport_type);
            }
        }
        break;

    case GAIA_UPGRADE_DISCONNECT_IND:
        {
            GAIA_UPGRADE_DISCONNECT_IND_T *ind = (GAIA_UPGRADE_DISCONNECT_IND_T *) message;

            GAIA_DEBUG(("G: GAIA_UPGRADE_DISCONNECT_IND type 0x%x\n", ind->transport_type));

            /*Re-apply parameters before Upgrade OTA Upgrade improvements on an Upgrade Connect from host*/
            if (ind->transport_type == gaia_transport_gatt)
            {
               sinkGaiaSendUpgradeOptimisationEvent(FALSE);
               gaiaResetUpgradeInProgress();
            }
        }
        break;
      
    default:
        GAIA_DEBUG(("G: unhandled 0x%04X\n", id));
        break;
    }
}


/****************************************************************************
NAME    
    sinkGaiaUpgradeScoDisconnectInd
    
DESCRIPTION
    Handle HFP SCO Disconnect Ind for Upgrade over GATT
    Check if Upgrade in progress and in case of SCO disconnect, apply Upgrade configuration
    
RETURNS
    void
*/
void sinkGaiaUpgradeScoDisconnectInd(void)
{    
    sinkBleResetParamsForUpgrade();
    sinkGaiaSendUpgradeOptimisationEvent(TRUE);
}

/****************************************************************************
NAME    
    sinkGaiaUpgradeScoConnectInd
    
DESCRIPTION
    Handle HFP SCO Connect Ind for Upgrade over GATT
    Check if Upgrade in progress and in case of SCO connect, revert Upgrade configuration
    
RETURNS
    void
*/
void sinkGaiaUpgradeScoConnectInd(void)
{
    /*Revert BLE params to original connection parameters*/
    sinkGaiaSendUpgradeOptimisationEvent(FALSE);
}

/****************************************************************************
NAME    
    sinkGaiaProcessUpgradeOptimisation
    
DESCRIPTION
    Apply or revert Optimisations for Upgrade
    
RETURNS
    void
*/
void sinkGaiaProcessUpgradeOptimisation(bool apply_optimisation)
{
    if (apply_optimisation)
    {
        gaiaApplyGattUpgradeOptimisations();
    }
    else
    {
        gaiaRevertGattUpgradeOptimisations();
    }
}

/*************************************************************************
NAME
    sinkGaiaSendUpgradeOptimisationEvent

DESCRIPTION
    Send Upgrade optimisation event after a delay. 
    On the event, apply or revert optimisations based on the flag.
    This delay is added to take care of scenarios where there is a LE transaction collision issue, 
    especially when slave is requesting for BLE Connection parameters while master is trying to update DLE parameters
*/
void sinkGaiaSendUpgradeOptimisationEvent(bool apply_optimisation)
{
    if (gaiaIsGattUpgradeInProgress())
    {
        EVENT_UPGRADE_OPTIMISATION_T *message = PanicUnlessNew(EVENT_UPGRADE_OPTIMISATION_T);

        MessageCancelAll(&theSink.task, EventSysUpgradeOptimisation);

        message->apply = apply_optimisation;
        MessageSendLater(&theSink.task, EventSysUpgradeOptimisation, message, GAIA_UPGRADE_OPTIMISATION_DELAY);
    }
}


/*************************************************************************
NAME
    gaiaGetBdAddrNonGaiaDevice

DESCRIPTION
    Determines the BD Address of the device which is not connected to GAIA.
*/
bool gaiaGetBdAddrNonGaiaDevice(bdaddr *bdaddr_non_gaia_device)
{
    typed_bdaddr gaia_taddr;

    /* Determine the BD Address of the AG connected to GAIA */
    if(GaiaTransportGetBdAddr(gaia_get_transport(), &gaia_taddr))
    {
        /* Determine the BD Address of another AG connected */
        if(deviceManagerNumOtherConnectedDevs(&gaia_taddr.addr) > deviceManagerNumConnectedPeerDevs())
        {
            if(a2dpGetNextAvBdAddress(&gaia_taddr.addr , bdaddr_non_gaia_device) ||
                slcGetNextAvBdAddress(&gaia_taddr.addr , bdaddr_non_gaia_device))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/*************************************************************************
NAME
    gaiaReportPioChange

DESCRIPTION
    Relay any registered PIO Change events to the Gaia client
*/
void gaiaReportPioChange(pio_common_allbits *pio_state)
{
    uint8 payload[8];
    pio_common_allbits change_mask;
    pio_common_allbits old_state;
    pio_common_allbits result;

    gaia_get_pio_change_mask(&change_mask);
    gaia_get_pio_old_state(&old_state);

    if (PioCommonBitsXOR(&result, pio_state, &old_state))
    {
        /* Bits have changed, see if they match the change mask */
        if (PioCommonBitsAND(&result,&result,&change_mask))
    {
            uint32 gaia_pios = gaia_translate_allbits(pio_state);
        dwunpack(payload + 4, VmGetClock());
            dwunpack(payload, gaia_pios);
            GAIA_DEBUG(("G: _EVENT_PIO_CHANGED: %08lx\n", gaia_pios));
        gaia_send_notification(GAIA_EVENT_PIO_CHANGED, sizeof payload, payload);
    }
    }

    gaia_set_pio_old_state(pio_state);
}


/*************************************************************************
NAME
    gaiaReportEvent

DESCRIPTION
    Relay any significant application events to the Gaia client
*/
void gaiaReportEvent(uint16 id)
{
    uint8 payload[1];

    switch (id)
    {
    case EventSysChargeComplete:
        if (gaia_get_notify_battery_charged())
            gaia_send_notification(GAIA_EVENT_BATTERY_CHARGED, 0, NULL);

        break;

    case EventUsrChargerConnected:
        if (gaia_get_notify_charger_connection())
        {
            payload[0] = 1;
            gaia_send_notification(GAIA_EVENT_CHARGER_CONNECTION, 1, payload);
        }
        break;

    case EventUsrChargerDisconnected:
        if (gaia_get_notify_charger_connection())
        {
            payload[0] = 0;
            gaia_send_notification(GAIA_EVENT_CHARGER_CONNECTION, 1, payload);
        }
        break;
    }
}


/*************************************************************************
NAME
    gaiaReportUserEvent

DESCRIPTION
    Relay any user-generated events to the Gaia client
*/
void gaiaReportUserEvent(uint16 id)
{
    if (gaia_get_transport() != NULL)
    {
        GAIA_DEBUG(("G: ev %04x en %u\n", id, gaia_get_notify_ui_event()));

        if (gaia_get_notify_ui_event())
        {
            uint8 payload[2];

            switch (id)
            {
        /*  Avoid redundant and irrelevant messages  */
            case EventSysLongTimer:
            case EventSysVLongTimer:
            case EventUsrLedsOnOffToggle:
                break;

            default:
                payload[0] = id >> 8;
                payload[1] = id & 0xFF;
                gaia_send_notification(GAIA_EVENT_USER_ACTION, 2, payload);
                break;
            }
        }
    }
}

/*************************************************************************
NAME
    gaiaDfuRequest

DESCRIPTION
    Request Device Firmware Upgrade from the GAIA host
*/
void gaiaDfuRequest(void)
{
    uint16 partition = gaia_get_dfu_partition();
    GAIA_DEBUG(("G: gaiaDfuRequest t=0x%p p=%u\n",
                (void *) gaia_get_transport(),
                partition));

    if ((gaia_get_transport() != NULL) &&
        (partition != GAIA_ILLEGAL_PARTITION))
    {
        GaiaDfuRequest(gaia_get_transport(),
                       partition,
                       GAIA_DFU_REQUEST_TIMEOUT);
}
}


/*************************************************************************
NAME
    gaiaDisconnect

DESCRIPTION
    Disconnect from GAIA client
*/
void gaiaDisconnect(void)
{
    GAIA_DEBUG(("G: disconnect %p\n", (void *) gaia_get_transport()));

    if (gaia_get_transport() != NULL && gaia_transport_gatt != GaiaTransportGetType(gaia_get_transport()))
    {
       GaiaDisconnectRequest(gaia_get_transport());
    }
}


/*************************************************************************
NAME
    GaiaHandleGetUserEqParamResponse
DESCRIPTION
    Send get user eq param response message to gaia
*/
void GaiaHandleGetUserEqParamResponse(AUDIO_GET_USER_EQ_PARAMETER_CFM_T* get_user_eq_resp_msg)
{
    bool data_valid = get_user_eq_resp_msg->data_valid;

    if (data_valid)
    {
        uint8 payload[4];
        user_eq_param_t* gaia_param = get_user_eq_resp_msg->param;

        payload[0] = gaia_param->id.bank & 0xf;
        payload[1] = ((gaia_param->id.band & 0xf) << 4) | (gaia_param->id.param_type & 0xf);
        payload[2] = (uint8)(gaia_param->value >> 8);
        payload[3] = (uint8)(gaia_param->value & 0xff);
        gaia_send_response(GAIA_VENDOR_QTIL, GAIA_COMMAND_GET_USER_EQ_PARAMETER, GAIA_STATUS_SUCCESS, 4, payload);
    }
    else
    {
        gaia_send_incorrect_state(GAIA_COMMAND_GET_USER_EQ_PARAMETER);
    }
}

/*************************************************************************
NAME
    GaiaHandleGetUserEqParamsResponse
DESCRIPTION
    Send get user eq params response message to gaia
*/
void GaiaHandleGetUserEqParamsResponse(AUDIO_GET_USER_EQ_PARAMETERS_CFM_T* get_user_eq_resp_msg)
{
    bool data_valid = get_user_eq_resp_msg->data_valid;
    if (data_valid)
    {
        unsigned number_of_params = get_user_eq_resp_msg->number_of_params;
        unsigned payload_size_words = GAIA_NUM_WORDS_PER_SET_GET_HDR + (GAIA_NUM_WORDS_PER_PARAM * number_of_params);
        uint16* payload = PanicUnlessMalloc(payload_size_words);

        if (payload != NULL)
        {
            unsigned i;

            payload[USER_EQ_NUM_PARAMS_OFFSET] = (uint16)number_of_params;

            for (i = 0; i < number_of_params; i++)
            {
                unsigned offset = i * GAIA_NUM_WORDS_PER_PARAM;
                user_eq_param_t gaia_param = get_user_eq_resp_msg->params[i];
                payload[offset + USER_EQ_PARAM_OFFSET] = ((gaia_param.id.bank & 0xf) << 8) | ((gaia_param.id.band & 0xf) << 4) | (gaia_param.id.param_type & 0xf);
                /* Any values passed from the audio plugins should be in suitable for transfer to gaia via a uint16.
                   If they are not then something has gone wrong! */
                payload[offset + USER_EQ_VALUE_OFFSET] = (uint16)gaia_param.value;
            }
            gaia_send_response_16(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER,
                                  GAIA_STATUS_SUCCESS,
                                  payload_size_words,
                                  payload);
            free(payload);
        }
        else
        {
            gaia_send_insufficient_resources(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
        }
    }
    else
    {
        gaia_send_incorrect_state(GAIA_COMMAND_GET_USER_EQ_GROUP_PARAMETER);
    }
}

#ifdef ENABLE_SPEECH_RECOGNITION
/*************************************************************************
NAME
    gaiaReportSpeechRecResult

DESCRIPTION
    Relay a speech recognition result to the Gaia client
*/
void gaiaReportSpeechRecResult(uint16 id)
{
    if (gaia_get_notify_speech_rec())
    {
        uint8 payload[1];
        bool ok = TRUE;

        switch (id)
        {
        case CSR_SR_WORD_RESP_YES:
            payload[0] = GAIA_ASR_RESPONSE_YES;
            break;

        case CSR_SR_WORD_RESP_NO:
            payload[0] = GAIA_ASR_RESPONSE_NO;
            break;

        case CSR_SR_WORD_RESP_FAILED_YES:
        case CSR_SR_WORD_RESP_FAILED_NO:
        case CSR_SR_WORD_RESP_UNKNOWN:
        case CSR_SR_APP_TIMEOUT:
        /*  Known unknowns  */
            payload[0] = GAIA_ASR_RESPONSE_UNRECOGNISED;
            break;

        default:
            GAIA_DEBUG(("gaia: ASR: %04x?\n", id));
            ok = FALSE;
            break;
        }

        if (ok)
            gaia_send_notification(GAIA_EVENT_SPEECH_RECOGNITION, 1, payload);
    }
}
#endif  /*  ifdef ENABLE_SPEECH_RECOGNITION  */

#endif  /*  ifdef ENABLE_GAIA  */
