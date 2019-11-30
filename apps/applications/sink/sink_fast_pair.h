/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_fast_pair.h

DESCRIPTION
    Header file for interface with fast pair application

NOTES

*/

#ifndef _SINK_FAST_PAIR_H_
#define _SINK_FAST_PAIR_H_

#include <csrtypes.h>
#include <message.h>
#include "sink_private_data.h"
#include "sink_gatt_db.h"
#include "sink_statemanager.h"

/* Work around gcc bug converting uppercase defines to lower case */
#ifdef enable_fast_pair
#define ENABLE_FAST_PAIR
#endif

/* Fast Pair requires GATT */
#ifndef GATT_ENABLED
#undef ENABLE_FAST_PAIR
#endif

#ifdef ENABLE_FAST_PAIR

#ifndef ENABLE_RANDOM_ADDR
#error It is mandated by Google Fast Pair Service Specification to use Fast Pair Service with BLE Random Resolvable Private Address. Please enable ENABLE_RANDOM_ADDR define in sink project properties.
#endif

#ifndef GATT_FP_SERVER
#error It is mandated by Google Fast Pair Service Specification to support Google Fast Pair Service. Please enable GATT_FP_SERVER define in sink project properties.
#endif


#endif

#ifdef ENABLE_FAST_PAIR
/* Library headers */
#include <gatt_fast_pair_server.h>
#include <cryptoalgo.h>
#endif

#define PASSKEY_INVALID         0xFF000000UL


#define MAX_FAST_PAIR_ACCOUNT_KEYS              (5)

#define FAST_PAIR_ENCRYPTED_REQUEST_LEN         (16)
#define FAST_PAIR_ENCRYPTED_PASSKEY_BLOCK_LEN   (16)
#define FAST_PAIR_ENCRYPTED_ACCOUNT_KEY_LEN     (16)

#define FAST_PAIR_PUBLIC_KEY_LEN                (64)
#define FAST_PAIR_PRIVATE_KEY_LEN               (32)
#define FAST_PAIR_ACCOUNT_KEY_LEN               (16)


#define FAST_PAIR_PROVIDER_ADDRESS_OFFSET       (2)
#define FAST_PAIR_SEEKER_ADDRESS_OFFSET         (8)


typedef enum
{
    fast_pair_passkey_seeker = 2,
    fast_pair_passkey_provider = 3
} fast_pair_passkey_owner_t;

/* Fast pair state machine used to iterate trhough the fast pair procedure */
typedef enum
{
    fast_pair_state_idle,
    fast_pair_state_wait_aes_key,
    fast_pair_state_key_based_pairing_response,
    fast_pair_state_wait_passkey,
    fast_pair_state_passkey_response,
    fast_pair_state_wait_account_key,
    fast_pair_state_fail_quarantine
} fast_pair_state_t;

typedef enum
{
    fast_pair_success,
    fast_pair_io_cap_failure
} fast_pair_status_t;

/* Structure to hold Key based Pairing Request values written by provider */
typedef struct
{
    bdaddr provider_addr;
    bdaddr seeker_addr;
    bool request_discoverability;
    bool request_bonding;
} fast_pair_key_based_pairing_req_t;

/* Structure to hold Passkey genrated by Bluestack and written by provider */
typedef struct
{
    uint32 seeker_passkey;
    uint32 provider_passkey;
    bool passkey_match;
} fast_pair_passkey_t;

/* Stucture used to to store account keys read from PS store 
   and iterate through the list to calculate the right 'K' 
 */
typedef struct
{
    uint8 num_keys;
    uint8 num_keys_processed;
    uint16 key_index[MAX_FAST_PAIR_ACCOUNT_KEYS];
    uint8 (*keys)[FAST_PAIR_ACCOUNT_KEY_LEN];
} fast_pair_account_keys_t;

/* Fast Pair data structure containing current state and other information */
typedef struct 
{
    TaskData                            task;               /* task associated with fast pair */
    fast_pair_state_t                   fp_state;           /* Fast pair state  */
    fast_pair_status_t                  fp_status;          /* Fast pair status state */
    const GFPS                          *fast_pair_server;  /* GFPS used for Write Indication and to send Notifications */
    uint16                              cid;                /* Current connection ID. On entering fast pair procedure,
                                                               any requests from only matching connection ID will be taken
                                                               for processing and all other request from different 
                                                               conenction ID will not be processed. */
    uint8                               *aes_key;           /* Current AES Key. If not valid will be NULL */
    uint8                               *encrypted_data;    /* Stored encrypted request */
    uint16                              encrypted_data_size;/* Size of encrypted data - 80 Bytes if along with Public key, else 16 Bytes */
    uint16                              *public_key;        /* Public key to be used */
    uint16                              *private_key;       /* Cached Anti-Spoofing Private Key */
    fast_pair_key_based_pairing_req_t   kbp_req;            /* Decoded Key based Pairing Request values */
    fast_pair_passkey_t                 passkey;            /* Passkey values */
    fast_pair_account_keys_t            account_key;        /* Cached account keys read from PS */
    uint16                              failure_count;      /* Used for remembering KbP decode failures */
    fp_discoverability_status_t   discoverability_status;  /* Used to remember sink discoverability status */
}sink_fast_pair_data_t;

/****************************************************************************
NAME    
    sinkFastPairIsInProgress

DESCRIPTION
    Checks if Fast Pairing is in Progress or not
*/
#ifdef ENABLE_FAST_PAIR
bool sinkFastPairIsInProgress(void);
#else
#define sinkFastPairIsInProgress() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkFastPairIsInQuarantine

DESCRIPTION
    Checks if fast pair state machine is in Quarantine.
CONDITIONS:
    Fastpair state machine would exhibit quarantine state
    when the failure_count is reaching 10. SO following are
    the conditions when we would increment the failure_count:
    1. when the size in "GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND"
        is not equal to "(FAST_PAIR_ENCRYPTED_REQUEST_LEN+FAST_PAIR_PUBLIC_KEY_LEN)"
        and not equal to FAST_PAIR_ENCRYPTED_REQUEST_LEN.
    2. When there are no valid AES key stored

*/
#ifdef ENABLE_FAST_PAIR
bool sinkFastPairIsInQuarantineState(void);
#else
#define sinkFastPairIsInQuarantineState() ((void)(0))
#endif



/****************************************************************************
NAME
    sinkFastPairGetStatus

DESCRIPTION
    Checks if fast pair is in successful or not.
*/
#ifdef ENABLE_FAST_PAIR
bool sinkFastPairGetStatus(void);
#else
#define sinkFastPairGetStatus() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairSetStatus

DESCRIPTION
    Sets the status for fast pair.
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairSetStatus(fast_pair_status_t fp_status);
#else
#define sinkFastPairSetStatus(fp_status) ((void)(0))
#endif



/****************************************************************************
NAME    
    sinkFastPairHandleProviderPasskey

DESCRIPTION
    Saves the Fast Pair Provider Passkey to checking later during paring
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairHandleProviderPasskey(uint32 passkey);
#else
#define sinkFastPairHandleProviderPasskey(passkey) ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairDisconnectLELink

DESCRIPTION
    Disconnects LE link and resets fastpair state back to idle.
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairDisconnectLELink(void);
#else
#define sinkFastPairDisconnectLELink() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairGetFastPairState

DESCRIPTION
    Gets the Fast Pair state
*/
#ifdef ENABLE_FAST_PAIR
fast_pair_state_t sinkFastPairGetFastPairState(void);
#else
#define sinkFastPairGetFastPairState() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairResetFailCount

DESCRIPTION
    Reset the failure count. This can happen
    1. When the procedure is succesful after a failure attempt.
    2. When there is power cycle performed.
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairResetFailCount(void);
#else
#define sinkFastPairResetFailCount() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairSetFastPairState

DESCRIPTION
    Sets the Fast Pair state
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairSetFastPairState(fast_pair_state_t new_state);
#else
#define sinkFastPairSetFastPairState(new_state) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairKeybasedPairingWrite

DESCRIPTION
    Handle Keybased Pairing Write request from Fast Pair seeker. The AES key is 
    calculated and notification will be send if decryption is susccessful. 
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairKeybasedPairingWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data, uint16 size);
#else
#define sinkFastPairKeybasedPairingWrite(fast_pair_server, cid, enc_data, size) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairPasskeyWrite

DESCRIPTION
    Handle Passkey Write request from Fast Pair seeker. The passkey is 
    decrypted and matched with provider's passkey and encrypted 
    notification will be send if decryption is susccessful. 
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairPasskeyWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data);
#else
#define sinkFastPairPasskeyWrite(fast_pair_server, cid, enc_data) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairAccountKeyWrite

DESCRIPTION
    Handle account key Write request from Fast Pair seeker. The account key is 
    decrypted and stored in NVM if decryption is susccessful. 
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairAccountKeyWrite(const GFPS* fast_pair_server, uint16 cid, uint8* enc_data);
#else
#define sinkFastPairAccountKeyWrite(fast_pair_server, cid, enc_data) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairHandleMessages

DESCRIPTION
    Handle fast pair messages from crypto library. 
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairHandleMessages(Task task, MessageId id, Message message);
#else
#define sinkFastPairHandleMessages(task, id, message) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairInit

DESCRIPTION
    Init Fast Pair application. 
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairInit(void);
#else
#define sinkFastPairInit() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairDeInit

DESCRIPTION
    De-Initialize memory for fast pair global structure. 
*/
#ifdef ENABLE_FAST_PAIR
void SinkFastPairDeInit(void);
#else
#define SinkFastPairDeInit() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkFastPairStopTimer

DESCRIPTION
    Stops a timer when the fast pair state changes.

*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairStopTimer(void);
#else
#define sinkFastPairStopTimer() ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkFastPairStopDiscoverabilityTimer

DESCRIPTION
    Stops a FP Seeker triggered discoverability timer 

PARAMETERS
    None

RETURNS
    None
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairStopDiscoverabilityTimer(void);
#else
#define sinkFastPairStopDiscoverabilityTimer() ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairDeleteAccountKeys

DESCRIPTION
    Delete Fast Pair account keys from PS.
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairDeleteAccountKeys(void);
#else
#define sinkFastPairDeleteAccountKeys() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkFastPairSetDiscoverabilityStatus

DESCRIPTION
    Function to set FP discoverability status
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairSetDiscoverabilityStatus(fp_discoverability_status_t status);
#else
#define sinkFastPairSetDiscoverabilityStatus(status) ((void)(0))
#endif

/****************************************************************************
NAME
    sinkFastPairHandleDiscoverability

DESCRIPTION
    Function to decide whether sink should go non-discoverable
    If sink discoverability is due to Kbp write request with discoverability set to 1 
    then on FP timer lapse, sink shall go non-discoverable
*/
#ifdef ENABLE_FAST_PAIR
void sinkFastPairHandleDiscoverability(void);
#else
#define sinkFastPairHandleDiscoverability() ((void)(0))
#endif

#endif /* _SINK_FAST_PAIR_H_ */
