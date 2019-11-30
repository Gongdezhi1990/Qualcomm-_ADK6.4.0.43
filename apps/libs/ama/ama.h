/*****************************************************************************

Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    ama.h

DESCRIPTION
    API for AMA library

*********************************************************************************/

#ifndef _AMA_H_
#define _AMA_H_

#include <voice_assistant_audio_manager.h>
#include <library.h>

/* Specific 'pmalloc' pool configurations. */
#define PMALLOC_AMA_EXTRAS { 512, 4 },

#define AMA_VERSION_MAJOR  1
#define AMA_VERSION_MINOR  0

typedef enum {
    ama_transport_ble,
    ama_transport_rfcomm,
    ama_transport_iap,
    ama_transport_none /* Keep this as the last one */
}ama_transport_t;


typedef enum
{
    AMA_SPEECH_STATE_IND = AMA_MSG_BASE,
    AMA_UPGRADE_TRANSPORT_IND,
    AMA_SWITCH_TRANSPORT_IND,
    AMA_LINK_DROP_IND,
    AMA_ENABLE_CLASSIC_PAIRING_IND,
    AMA_STOP_ADVERTISING_AMA_IND,
    AMA_START_ADVERTISING_AMA_IND,
    AMA_SEND_AT_COMMAND_IND,
    AMA_SEND_TRANSPORT_VERSION_ID,
    AMA_MESSAGE_TOP
}ama_message_type_t;

typedef enum{
    ama_speech_state_idle,
    ama_speech_state_listening,
    ama_speech_state_processing,
    ama_speech_state_speaking,
    ama_speech_state_err = 0xff
}ama_speech_state_t;

typedef enum{
    ama_at_cmd_ata_ind,
    ama_at_cmd_at_plus_chup_ind,
    ama_at_cmd_at_plus_bldn_ind,
    ama_at_cmd_at_plus_chld_eq_0_ind,
    ama_at_cmd_at_plus_chld_eq_1_ind,
    ama_at_cmd_at_plus_chld_eq_2_ind,
    ama_at_cmd_at_plus_chld_eq_3_ind,
    ama_at_cmd_atd_ind
}ama_at_cmd_t;

typedef struct
{
    ama_at_cmd_t at_command;
}AMA_SEND_AT_COMMAND_IND_T;


typedef struct{
    ama_speech_state_t speech_state;
}AMA_SPEECH_STATE_IND_T;

typedef struct{
    ama_transport_t transport;
}AMA_SWITCH_TRANSPORT_IND_T;

typedef enum  {
    ama_speech_initiator_none,
    ama_speech_initiator_press_and_hold,
    ama_speech_initiator_tap,
    ama_speech_initiator_wakeword,
} ama_speech_initiator_t;

/*!
    @brief AMA Data base for storing the device information.

    During AMA intialising library stores the application's device information configuration.
*/
typedef struct __ama_device_config
{
    char    *serial_number;
    char    *name;
    char    *device_type;
}ama_device_config_t;

/*!
    @brief Initialise the AMA library before use

    @param application_task Application task handler to receive messages from the library
    @param feature_config The va feature config for for the voice assitant plugin
    @param audio_config The va audio config for for the voice assitant plugin
    @param ama_device_config The AMA device information configurations
	
	@return bool Result of the initialise operation
*/
bool AmaInit(Task application_task,
             const va_audio_mgr_feature_config_t *feature_config,
             const va_audio_mgr_audio_config_t *audio_config,
             const ama_device_config_t *ama_device_config);

/*!
    @brief Data source from the VA plugin. To be used by the AMA packetiser
           to send voice

    @param src Source providing the data
*/
void AmaInitSetSpeechDataSource(Source src);

/*!
    @brief Get the current transport being used by AMA
*/
ama_transport_t AmaTransportGet(void);

/*!
    @brief Start sending speech to the phone
*/
bool AmaStartSpeech(ama_speech_initiator_t initiator);

/*!
    @brief Stop sending speech to the phone. Send Stop Speech message
*/
void AmaStopSpeech(bool send_stop);

/*!
    @brief Suspend detecting trigger word/wake word in case of voice trigger enabled 
*/
void AmaSuspendSpeech(void);

/*!
    @brief Resume detecting wake word.
*/
void AmaResumeSpeech(void);

/*!
    @brief Stop sending speech to the phone. 
    Sends the endpoint speech message
*/
void AmaEndSpeech(void);

/*!
    @brief Entry point for incoming AMA data.

    @param stream Pointer to incoming data

    @param size Length of incoming data
*/
bool AmaParseData(const uint8* stream, uint16 size);

/*!
    @brief Is the current AMA transport Ble
*/
bool AmaTransportIsBle(void);

/*!
    @brief Notify the AMA transport of the peer address

    @param bd_addr Address of the peer
*/
void AmaTransportStorePeerAddr(bdaddr* bd_addr);


/*!
    @brief Send the version information to the remote device

    @param uint8 major Version major number

    @param uint8 minor Version minor number
*/
void AmaTransportSendVersion(uint8 major, uint8 minor);

/*!
    @brief Clears down the parse state variables which may be
           invalid after link loss occurances mid-transmission

*/
void AmaResetTransportState(void);

/*!
    @brief If connected over RFCOMM this will cause a reset message to the
           phone so that it closes the channel.

    @param bd_addr address of the link to be reset

*/
void AmaResetRfcommLink(const bdaddr *bd_addr);


#endif /* _AMA_H_ */
