/*!
\copyright  Copyright (c) 2018 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file
\brief      Header file for management of Bluetooth Low Energy advertising settings

Provides control for Bluetooth Low Energy (BLE) advertisements. 
The API for this module allows for multiple adverts to be configured.
            
\ref appAdvManagerNewAdvert() is used to get the identifier for 
a collection of advertisement settings, and is used in the other API
calls.

Support for multiple simultaneous advertisements is TBD.

Connections intended to use Bluetooth Low Energy should be controlled 
through the connection manager (av_headset_con_manager.h). The 
adverts associated with a connection are configured here.
*/


#ifndef _AV_HEADSET_ADV_MANAGER_H_
#define _AV_HEADSET_ADV_MANAGER_H_

#include "av_headset_message.h"

/*! Messages sent by the advertising manager.  */
enum av_headset_advertising_messages
{
        /*! Message signalling the battery module initialisation is complete */
    APP_ADVMGR_INIT_CFM = ADV_MANAGER_MESSAGE_BASE,
        /*! Message responding to appAdvManagerStart */
    APP_ADVMGR_ADVERT_START_CFM,
        /*! Message responding to appAdvManagerSetAdvertData */
    APP_ADVMGR_ADVERT_SET_DATA_CFM,
};


/*! Message sent when appAdvManagerStart() completes */
typedef struct
{
    /*! Final result of the appAdvManagerStart() operation */
    connection_lib_status   status;
} APP_ADVMGR_ADVERT_START_CFM_T;


/*! Message sent when appAdvManagerSetAdvertData() completes */
typedef struct
{
        /*! Final result of the appAdvManagerSetAdvertData() operation */
    connection_lib_status   status;
} APP_ADVMGR_ADVERT_SET_DATA_CFM_T;


/*! State of the advertising manager (internal) */
typedef enum _avHeadsetAdvManagerState
{
        /*! Initial state of advertising manager */
    ADV_MGR_STATE_STARTING,
        /*! Initialisation completed. Any external information has now been retrieved. */
    ADV_MGR_STATE_INITIALISED,
        /*! There is an active advert (on the stack) */
    ADV_MGR_STATE_ADVERTISING,
} avHeadsetAdvManagerState;


/*! Type of discoverable advertising to use.

    The value of defines from connection.h are used, but note that only
    a subset are available in the enum. */
typedef enum 
{
        /*! Not using discoverable advertising */
    avHeadsetBleDiscoverableModeNone = 0,
        /*! LE Limited Discoverable Mode */
    avHeadsetBleDiscoverableModeLimited = BLE_FLAGS_LIMITED_DISCOVERABLE_MODE,
        /*! LE General Discoverable Mode */
    avHeadsetBleDiscoverableModeGeneral = BLE_FLAGS_GENERAL_DISCOVERABLE_MODE,
} avHeadsetBleDiscoverableMode;


/*! \todo work out what this is for. Note that have removed broadcasting reasons. */
typedef enum
{
    avHeadsetBleGapReadNameAdvertising  = 1,
    avHeadsetBleGapReadNameGapServer,// = 2,
    avHeadsetBleGapReadNameAssociating,//= 8,
} avHeadsetBleGapReadName;


struct _advManagerAdvert;

/*! Definition for an advert. The structure will contain all needed settings.

    This is an anonymous type as no external code should need direct access.
*/
typedef struct _advManagerAdvert advManagerAdvert;

/*! Advertising manager task structure */
typedef struct
{
        /*! Task for advertisement management */
    TaskData                    task;
        /*! Local state for advertising manager */
    avHeadsetAdvManagerState    state;
        /*! Allocated storage for the device name */
    uint8*                      localName;

        /*! An advert that is currently in the middle of an operation
            that blocks other activity */
    advManagerAdvert           *blockingAdvert;
        /*! The message that will be sent once the current blocking operation
            is completed */
    MessageId                   blockingOperation;
        /*! The condition (internal) that the blocked operation is waiting for */
    uint16                      blockingCondition;
        /*! The task to send a message to once the blocking operation completes */
    Task                        blockingTask;
} advManagerTaskData;


/*! Get the local name */
#define appAdvManagerGetLocalName() (appGetAdvManager()->localName)


/*! Retrieve a handle for a new advertisement 

    New adverts reset all setting to zero or equivalent. The exception
    is for the advertising channels to use, where all channels are selected.

    \return NULL if no unused advertisements are available
*/
advManagerAdvert *appAdvManagerNewAdvert(void);


/*! Stop using an advertisement 

    This releases one of the adverts for re-use. 
    
    Advertising using the advert should be stopped before calling this function.
    If it is not and the advert is in use, then the advert \b will be stopped, but 
    the function may return FALSE if the advert cannot be stopped synchronously.

    \param advert   The advert-settings to be released

    \return FALSE if the advertisement could not be released. 
*/
bool appAdvManagerDeleteAdvert(advManagerAdvert *advert);


/*! Set the name to use in the specified advert

    The name will be shorted if necessary to fit into the advertising packet

    \param advert   The advert-settings to be updated
    \param name     The name string to use (null terminated)

    \return TRUE if name set successfully
*/
bool appAdvManagerSetName(advManagerAdvert *advert,uint8 *name);


/*! Use the device name in the specified advert

    The name will be shortened if necessary to fit into the advertising packet

    If no local name can be found found, a blank name will be set.

    \param advert   The advert-settings to be updated
*/
void appAdvManagerUseLocalName(advManagerAdvert *advert);


/*! Set the flags to use in the specified advert based on modes to use

    \param advert   The advert-settings to be updated
    \param discoverable_mode    The type of discoverable advert

    \return TRUE if flags set successfully
*/
bool appAdvManagerSetDiscoverableMode(advManagerAdvert *advert, 
                                      avHeadsetBleDiscoverableMode discoverable_mode);


/*! Set the reason for the advert (based on GAP)

    \param advert   The advert-settings to be updated
    \param reason   The reason for this advert

    \return TRUE if the reason was saved successfully
*/
bool appAdvManagerSetReadNameReason(advManagerAdvert *advert,
                                    avHeadsetBleGapReadName reason);


/*! Add a service UUID to data that can be advertised

    \param advert   The advert-settings to be updated
    \param uuid     The service UUID to add

    \return TRUE if the UUID was added successfully. Note that the
    number of services supported is limited.
*/
bool appAdvManagerSetService(advManagerAdvert *advert, uint16 uuid);


/*! Set the parameters for advertising.

    This allows the setting of parameters for directed and undirected
    advertising.

    For undirected advertising the advertising interval and
    filtering policy can be specified.

    For directed advertising only the target address (and its type)
    can be specified.

    See the description for ble_adv_params_t in connection.h

    \param advert   The advert-settings to be updated
    \param[in]  adv_params  Pointer to structure with parameters
*/
void appAdvManagerSetAdvertParams(advManagerAdvert *advert, ble_adv_params_t *adv_params);


/*! Set the type of advert to be used

    \param advert   The advert-settings to be updated
    \param advert_type The type of advertising that will be performed
*/
void appAdvManagerSetAdvertisingType(advManagerAdvert *advert, ble_adv_type advert_type);


/*! Set the advertising channels to be used for the advert

    \param advert   The advert-settings to be updated
    \param channel_mask A mask specifying the channels to be used. The define
        BLE_ADV_CHANNEL_ALL will select all channels
*/
void appAdvManagerSetAdvertisingChannels(advManagerAdvert *advert, uint8 channel_mask);


/*! Select whether to use a random address as our address

    \param advert   The advert-settings to be updated
    \param use_random_address Set this if wish to use a random address when advertising
*/
void appAdvManagerSetUseOwnRandomAddress(advManagerAdvert *advert, bool use_random_address);


/*! Handler for connection library messages not sent directly

    This function is called to handle any connection library messages sent to
    the application that the advertising module is interested in. If a message 
    is processed then the function returns TRUE.

    \note Some connection library messages can be sent directly as the 
        request is able to specify a destination for the response.

    \param  id              Identifier of the connection library message 
    \param  message         The message content (if any)
    \param  already_handled Indication whether this message has been processed by
                            another module. The handler may choose to ignore certain
                            messages if they have already been handled.

    \returns TRUE if the message has been processed, otherwise returns the
        value in already_handled
 */
extern bool appAdvManagerHandleConnectionLibraryMessages(MessageId id, Message message, 
                                                   bool already_handled);


/*! Enable advertising with the specified advert

    If multiple adverts are enabled simultaneously, then the advertising
    manager will schedule them.

    If the function returns TRUE then when the advert has been configured
    and started the message APP_ADVMGR_ADVERT_START_CFM is sent to the 
    requester task. The message can indicate a failure.

    \note As this is an asynchronous command, requests will be serialised and may
    be delayed by the advertising manager.

    \param advert       The advert-to enable
    \param requester    Task to send response to

    \return FALSE if there was a problem with the advert.
            TRUE the advert appears correct and the message APP_ADVMGR_ADVERT_START_CFM 
            will be sent with the final status
*/
bool appAdvManagerStart(advManagerAdvert *advert, Task requester);


/*! Set-up the advertising data to match the specified advert

    This is intended for use with connectable adverts which are managed by
    GATT.

    If the function returns TRUE then the advertising data is configured
    and the message APP_ADVMGR_ADVERT_SET_DATA_CFM is sent to the 
    requester task. The message can indicate a failure.

    \note As this is an asynchronous command, requests will be serialised and may
    be delayed by the advertising manager.

    \param advert       The advert-to enable
    \param requester    Task to send response to

    \return FALSE if there was a problem with the advert.
            TRUE the advert settings appear correct and the message 
            APP_ADVMGR_ADVERT_SET_DATA_CFM will be sent with the final status.
*/
bool appAdvManagerSetAdvertData(advManagerAdvert *advert, Task requester);


/*! Initialise the advertising manager */
extern void appAdvManagerInit(void);

#endif /* _AV_HEADSET_ADV_MANAGER_H_ */
