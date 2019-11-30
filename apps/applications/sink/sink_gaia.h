/****************************************************************************
Copyright (c) 2010 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gaia.h

DESCRIPTION
    Header file for interface with Generic Application Interface Architecture
    library

NOTES

*/
#ifndef _SINK_GAIA_H_
#define _SINK_GAIA_H_


/* Work around gcc bug converting uppercase defines to lower case */
#ifdef enable_gaia
#define ENABLE_GAIA
#endif

#include <stream.h>
#include <gaia.h>
#include <pio_common.h>
#include <audio.h>
#include "sink_va_if.h"


#ifndef GATT_ENABLED
#undef GATT_GAIA_SERVER
#endif

#ifdef GATT_GAIA_SERVER
#include "sink_gatt_db.h"
#endif

#define GAIA_API_MINOR_VERSION (5)

#define GAIA_TONE_BUFFER_SIZE (94)
#define GAIA_TONE_MAX_LENGTH ((GAIA_TONE_BUFFER_SIZE - 4) / 2)

#define GAIA_ILLEGAL_PARTITION (15)
#define GAIA_DFU_REQUEST_TIMEOUT (30)

/* Message delay (ms) when applying or retry of Upgrade optimisation*/
#define GAIA_UPGRADE_OPTIMISATION_DELAY (30)

typedef struct
{
    unsigned fixed:1;
    unsigned size:15;
} gaia_config_entry_size_t;

typedef struct
{
    bool apply;/*Apply or Revert Upgrade Optimisation*/
} EVENT_UPGRADE_OPTIMISATION_T;

/*For complete information about the GAIA commands, refer to the document GAIAHeadsetCommandReference*/

#define CHANGE_NUMBER_OF_ACTIVE_BANKS   0xFFFF
#define CHANGE_NUMBER_OF_BANDS          0xF0FF
#define CHANGE_BANK_MASTER_GAIN         0xF0FE
#define CHANGE_BAND_PARAMETER           0xF000
#define USER_EQ_BANK_INDEX              1

/*************************************************************************
NAME
    initGaia

DESCRIPTION
    Initialize GAIA system by allocating memory for GAIA global structure
*/
#if defined ENABLE_GAIA
void initGaia(void);
#else
#define initGaia() ((void) 0)
#endif

/*************************************************************************
NAME
    gaiaReportPioChange

DESCRIPTION
    Relay any registered PIO Change events to the Gaia client
    We handle the PIO-like GAIA_EVENT_CHARGER_CONNECTION here too
*/
#if defined ENABLE_GAIA
void gaiaReportPioChange(pio_common_allbits *pio_state);
#else
#define gaiaReportPioChange(pio_state) UNUSED(pio_state)
#endif


/*************************************************************************
NAME
    gaiaReportEvent

DESCRIPTION
    Relay any significant application events to the Gaia client
*/
#if defined ENABLE_GAIA
void gaiaReportEvent(uint16 id);
#else
#define gaiaReportEvent(id) UNUSED(id)
#endif


/*************************************************************************
NAME
    gaiaReportUserEvent

DESCRIPTION
    Relay any user-generated events to the Gaia client
*/
#if defined ENABLE_GAIA
void gaiaReportUserEvent(uint16 id);
#else
#define gaiaReportUserEvent(id) UNUSED(id)
#endif


/*************************************************************************
NAME
    gaiaReportSpeechRecResult

DESCRIPTION
    Relay a speech recognition result to the Gaia client
*/
#if defined ENABLE_GAIA
void gaiaReportSpeechRecResult(uint16 id);
#else
#define gaiaReportSpeechRecResult(id) UNUSED(id)
#endif


/*************************************************************************
NAME
    handleGaiaMessage

DESCRIPTION
    Handle messages passed up from the Gaia library
*/
#if defined ENABLE_GAIA
void handleGaiaMessage(Task task, MessageId id, Message message);
#else
#define handleGaiaMessage(task, id, message) UNUSED(task, id, message)
#endif

/*************************************************************************
NAME
    gaiaGetBdAddrNonGaiaDevice

DESCRIPTION
    Determines the BD Address of the device which is not connected to GAIA.
*/
#if defined ENABLE_GAIA
bool gaiaGetBdAddrNonGaiaDevice(bdaddr *bdaddr_non_gaia_device);
#else
#define gaiaGetBdAddrNonGaiaDevice(bdaddr_non_gaia_device) UNUSED(bdaddr_non_gaia_device)
#endif


/*************************************************************************
NAME
    gaiaDfuRequest

DESCRIPTION
    Request Device Firmware Upgrade from the GAIA host
*/
#if defined ENABLE_GAIA
void gaiaDfuRequest(void);
#else
#define gaiaDfuRequest() ((void) 0)
#endif


/*************************************************************************
NAME
    gaiaDisconnect

DESCRIPTION
    Disconnect from GAIA client
*/
#if defined ENABLE_GAIA
void gaiaDisconnect(void);
#else
#define gaiaDisconnect() ((void) 0)
#endif


/*************************************************************************
NAME
    gaia_send_response

DESCRIPTION
    Build and Send a Gaia acknowledgement packet

*/
#if defined ENABLE_GAIA
void gaia_send_response(uint16 vendor_id, uint16 command_id, uint16 status,
                          uint16 payload_length, uint8 *payload);
#else
#define gaia_send_response(vendor_id, command_id, status,\
                          payload_length, payload) UNUSED(vendor_id, command_id, status,\
                          payload_length, payload)
#endif

/*************************************************************************
NAME
    GaiaHandleGetUserEqParamResponse
DESCRIPTION
    Send get user eq param response message to gaia
*/
#if defined ENABLE_GAIA
void GaiaHandleGetUserEqParamResponse(AUDIO_GET_USER_EQ_PARAMETER_CFM_T* get_user_eq_resp_msg);
#else
#define GaiaHandleGetUserEqParamResponse(get_user_eq_resp_msg) UNUSED(get_user_eq_resp_msg)
#endif

/*************************************************************************
NAME
    GaiaHandleGetUserEqParamsResponse
DESCRIPTION
    Send get user eq params response message to gaia
*/
#if defined ENABLE_GAIA
void GaiaHandleGetUserEqParamsResponse(AUDIO_GET_USER_EQ_PARAMETERS_CFM_T* get_user_eq_resp_msg);
#else
#define GaiaHandleGetUserEqParamsResponse(get_user_eq_resp_msg) UNUSED(get_user_eq_resp_msg)
#endif

/*************************************************************************
NAME
    gaia_send_response_16

DESCRIPTION
    Build and Send a Gaia acknowledgement packet from a uint16[] payload

*/
#if defined ENABLE_GAIA
void gaia_send_response_16(uint16 command_id, uint16 status,
                          uint16 payload_length, uint16 *payload);
#else
#define gaia_send_response_16(command_id, status,\
                          payload_length, payload) UNUSED(command_id, status,\
                          payload_length, payload)
#endif						  

/****************************************************************************
NAME
	gaia_get_transport

DESCRIPTION
    Get the GAIA transport

PARAMS
    void

RETURNS
	GAIA_TRANSPORT *
*/
#if defined ENABLE_GAIA
GAIA_TRANSPORT* gaia_get_transport(void);
#else
#define gaia_get_transport() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_transport

DESCRIPTION
    Set the GAIA transport

PARAMS
    GAIA_TRANSPORT* transport

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_transport(GAIA_TRANSPORT* transport);
#else
#define gaia_set_transport(transport) UNUSED(transport)
#endif

/****************************************************************************
NAME
    gaia_get_pio_change_mask

DESCRIPTION
    Get the GAIA pio change mask which is a 32 bit value, mapped against the
    translation table.

RETURNS
    The 32 bit mask.
*/
#if defined ENABLE_GAIA
uint32 gaia_get_pio_change_mask32(void);
#else
#define gaia_get_pio_change_mask32() ((void) 0)
#endif

/****************************************************************************
NAME
    gaia_get_pio_change_mask

DESCRIPTION
    Get the GAIA pio change mask, populating the supplied pio_common_allbits
    structure.

RETURNS
    TRUE if the mask is non-empty
*/
#if defined ENABLE_GAIA
bool gaia_get_pio_change_mask(pio_common_allbits *mask);
#else
#define gaia_get_pio_change_mask(mask) UNUSED(mask)
#endif

/****************************************************************************
NAME
    gaia_set_pio_change_mask

DESCRIPTION
    Set the GAIA pio change mask

PARAMS
    mask A 32 bit value representing "gaia PIOS" that are mapped using a
    configuration table

*/
#if defined ENABLE_GAIA
void gaia_set_pio_change_mask(uint32 mask);
#else
#define gaia_set_pio_change_mask(mask) UNUSED(mask)
#endif

/****************************************************************************
NAME
    gaia_clear_pio_change_mask

DESCRIPTION
    Clear the GAIA pio change mask so it represents "no bits"

*/
#if defined ENABLE_GAIA
void gaia_clear_pio_change_mask(void);
#else
#define gaia_clear_pio_change_mask() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_get_pio_old_state

DESCRIPTION
    Get the old state of PIO bits, populating the supplied pio_common_allbits
    structure.

PARAMS
    state   Pointer to pio_common_allbits structure to take the old state of the PIOs

RETURNS
    TRUE if the mask is non-empty
*/
#if defined ENABLE_GAIA
bool gaia_get_pio_old_state(pio_common_allbits *state);
#else
#define gaia_get_pio_old_state(state) UNUSED(state)
#endif

/****************************************************************************
NAME
    gaia_set_pio_old_state

DESCRIPTION
    Set the GAIA pio old state

PARAMS
    state  Pointer to the pio_common_allbits svariable holding the state to
    set PIOs to.

RETURNS
    void
*/
#if defined ENABLE_GAIA
void gaia_set_pio_old_state(pio_common_allbits *state);
#else
#define gaia_set_pio_old_state(state) UNUSED(state)
#endif

/****************************************************************************
NAME
	gaia_get_alert_tone

DESCRIPTION
    Get the GAIA alert tone

PARAMS
    void

RETURNS
	uint32
*/
#if defined ENABLE_GAIA
ringtone_note* gaia_get_alert_tone(void);
#else
#define gaia_get_alert_tone() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_alert_tone

DESCRIPTION
    Set the GAIA alert tone

PARAMS
    ringtone_note* tone

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_alert_tone(ringtone_note* tone);
#else
#define gaia_set_alert_tone(tone) UNUSED(tone)
#endif

/****************************************************************************
NAME
	gaia_get_notify_ui_event

DESCRIPTION
    Get the GAIA notify ui event

PARAMS
    void

RETURNS
	uint8
*/
#if defined ENABLE_GAIA
uint8 gaia_get_notify_ui_event(void);
#else
#define gaia_get_notify_ui_event() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_notify_ui_event

DESCRIPTION
    Set the GAIA notify ui event

PARAMS
    event GAIA notification event

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_notify_ui_event(uint8 event);
#else
#define gaia_set_notify_ui_event(event) UNUSED(event)
#endif

/****************************************************************************
NAME
	gaia_get_notify_charger_connection

DESCRIPTION
    Get the GAIA notify charger connection

PARAMS
    void

RETURNS
	uint8
*/
#if defined ENABLE_GAIA
uint8 gaia_get_notify_charger_connection(void);
#else
#define gaia_get_notify_charger_connection() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_notify_charger_connection

DESCRIPTION
    Set the GAIA notify charger connection

PARAMS
    connection GAIA charger connection

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_notify_charger_connection(uint8 connection);
#else
#define gaia_set_notify_charger_connection(connection) UNUSED(connection)
#endif

/****************************************************************************
NAME
	gaia_get_notify_battery_charged

DESCRIPTION
    Get the GAIA notify battery charged

PARAMS
    void

RETURNS
	uint8
*/
#if defined ENABLE_GAIA
uint8 gaia_get_notify_battery_charged(void);
#else
#define gaia_get_notify_battery_charged() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_notify_battery_charged

DESCRIPTION
    Set the GAIA notify battery charged

PARAMS
    charged Battery charged state

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_notify_battery_charged(uint8 charged);
#else
#define gaia_set_notify_battery_charged(charged) UNUSED(charged)
#endif

/****************************************************************************
NAME
	gaia_get_notify_speech_rec

DESCRIPTION
    Get the GAIA notify speech recognization

PARAMS
    void

RETURNS
	uint8
*/
#if defined ENABLE_GAIA
uint8 gaia_get_notify_speech_rec(void);
#else
#define gaia_get_notify_speech_rec() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_notify_speech_rec

DESCRIPTION
    Set the GAIA notify speech recognization

PARAMS
    rec Speech recognition notification

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_notify_speech_rec(uint8 rec);
#else
#define gaia_set_notify_speech_rec(rec) UNUSED(rec)
#endif

/****************************************************************************
NAME
	gaia_get_dfu_boot_status

DESCRIPTION
    Get the GAIA dfu boot status

PARAMS
    void

RETURNS
	uint8
*/
#if defined ENABLE_GAIA
uint8 gaia_get_dfu_boot_status(void);
#else
#define gaia_get_dfu_boot_status() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_set_dfu_boot_status

DESCRIPTION
    Set the GAIA dfu boot status

PARAMS
    status DFU boot status to set

RETURNS
	void
*/
#if defined ENABLE_GAIA
void gaia_set_dfu_boot_status(uint8 status);
#else
#define gaia_set_dfu_boot_status(status) UNUSED(status)
#endif

/****************************************************************************
NAME
    gaia_get_activate_peq_done

DESCRIPTION
    Get the GAIA activate_peq_done flag

PARAMS
    void

RETURNS
    bool
*/
#if defined ENABLE_GAIA
bool gaia_get_activate_peq_done(void);
#else
#define gaia_get_activate_peq_done() ((void) 0)
#endif

/****************************************************************************
NAME
    gaia_set_activate_peq_done

DESCRIPTION
    Set the GAIA activate_peq_done flag

PARAMS
    status activate_peq_done status to set

RETURNS
    void
*/
#if defined ENABLE_GAIA
void gaia_set_activate_peq_done(bool status);
#else
#define gaia_set_activate_peq_done(status) UNUSED(status)
#endif

/****************************************************************************
NAME
	gaia_get_enable_session

DESCRIPTION
    Get the GAIA enable session

PARAMS
    void

RETURNS
	bool
*/
#if defined ENABLE_GAIA
bool gaia_get_enable_session(void);
#else
#define gaia_get_enable_session() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_get_remain_connected

DESCRIPTION
    Get the GAIA remain connected status

PARAMS
    void

RETURNS
	bool
*/
#if defined ENABLE_GAIA
bool gaia_get_remain_connected(void);
#else
#define gaia_get_remain_connected() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_get_dfu_partition

DESCRIPTION
    Get the GAIA dfu partition

PARAMS
    void

RETURNS
	uint16
*/
#if defined ENABLE_GAIA
uint16 gaia_get_dfu_partition(void);
#else
#define gaia_get_dfu_partition() ((void) 0)
#endif

/****************************************************************************
NAME
	gaia_get_SC_PEQ_Settings_Timeout

DESCRIPTION
    Get the GAIA Store PEQ settings time out

PARAMS
    void

RETURNS
	uint16
*/
#if defined ENABLE_GAIA
uint16 gaia_get_SC_PEQ_Settings_Timeout(void);
#else
#define gaia_get_SC_PEQ_Settings_Timeout() ((void) 0)
#endif

#if defined GATT_GAIA_SERVER && defined ENABLE_GAIA
#define gaiaGattEnabled() (TRUE)
#else
#define gaiaGattEnabled() (FALSE)
#endif


#if defined GATT_GAIA_SERVER && defined ENABLE_GAIA
#define gaiaGattServerInitialise(void) GaiaStartGattServer(HANDLE_GAIA_SERVICE, HANDLE_GAIA_SERVICE_END)
#else
#define gaiaGattServerInitialise(void)
#endif


#if defined GATT_GAIA_SERVER && defined ENABLE_GAIA
#define gaiaGattConnect(cid) GaiaConnectGatt(cid)
#else
#define gaiaGattConnect(cid) UNUSED(cid)
#endif

#if defined GATT_GAIA_SERVER && defined ENABLE_GAIA
#define gaiaGattDisconnect(cid) GaiaDisconnectGatt(cid)
#else
#define gaiaGattDisconnect(cid) UNUSED(cid)
#endif


#if defined ENABLE_GAIA
bool gaiaIsGattUpgradeInProgress(void);
#else
#define gaiaIsGattUpgradeInProgress() (FALSE)
#endif

#if defined ENABLE_GAIA
void gaiaSetGattUpgradeInProgress(bool status);
#else
#define gaiaSetGattUpgradeInProgress(status) UNUSED(status)
#endif

#if defined ENABLE_GAIA
void gaiaResetUpgradeInProgress(void);
#else
#define gaiaResetUpgradeInProgress()
#endif

/****************************************************************************
NAME    
    sinkGaiaUpgradeScoConnectInd
    
DESCRIPTION
    Handle HFP SCO Connect Ind for Upgrade over GATT
    Check if Upgrade in progress and in case of SCO connect, revert Upgrade configuration
    
RETURNS
    void
*/

#if defined  ENABLE_GAIA
void sinkGaiaUpgradeScoConnectInd(void);
#else
#define sinkGaiaUpgradeScoConnectInd()
#endif

/****************************************************************************
NAME    
    sinkGaiaUpgradeScoDisconnectInd
    
DESCRIPTION
    Handle HFP SCO Disconnect Ind for Upgrade over GATT
    Check if Upgrade in progress and in case of SCO disconnect, apply Upgrade configuration
    
RETURNS
    void
*/
#if defined  ENABLE_GAIA
void sinkGaiaUpgradeScoDisconnectInd(void);
#else
#define sinkGaiaUpgradeScoDisconnectInd()
#endif

/****************************************************************************
NAME    
    sinkGaiaProcessUpgradeOptimisation
    
DESCRIPTION
    Apply or revert Optimisations for Upgrade
    
RETURNS
    void
*/
#if defined ENABLE_GAIA
void sinkGaiaProcessUpgradeOptimisation(bool apply_optimisation);
#else
#define sinkGaiaProcessUpgradeOptimisation(apply) UNUSED(apply)
#endif


/*************************************************************************
NAME
    sinkGaiaSendUpgradeOptimisationEvent

DESCRIPTION
    Send Upgrade optimisation event after a delay. 
    On the event, apply or revert optimisations based on the flag.
    This delay is added to take care of scenarios where there is a LE transaction collision issue, 
    especially when slave is requesting for BLE Connection parameters while master is trying to update DLE parameters
*/
#if defined ENABLE_GAIA
void sinkGaiaSendUpgradeOptimisationEvent(bool apply_optimisation);
#else
#define sinkGaiaSendUpgradeOptimisationEvent(apply) UNUSED(apply)
#endif

#endif /*_SINK_GAIA_H_*/
