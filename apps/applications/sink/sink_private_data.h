/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_private_data.h
 
DESCRIPTION
    This module interface has to be used to access and set generic and common sink related 
    configuration items and module specific runtime data. API's are provided to access the same
*/
 
 
/*!
@file    sink_private_data.h
@brief  This module intented to handle all common and private data of sink. It exposes interface
            API's to access the same. The data could be a configurable item or a module specific
            runtime data. Only the required interfaces need to be provided to access the data.
            This module does not have any inteligence on how to use the data or
            how to manupulate the data,it just maintain the data and provides API to use it.
 
           This file provides documentation for the sink_private_data API.
*/

#ifndef _SINK_PRIVATE_DATA_H_
#define _SINK_PRIVATE_DATA_H_

#include <csrtypes.h>

#include <bdaddr_.h>
#include <connection_no_ble.h>

#include "sink_linkloss.h"
#include "sink_private_data_config_def.h"

typedef struct
{
    unsigned        normalRole:2;       /* Master (0), Slave (1) or passive (2) */
    unsigned        normalEntries:2;    /* 0-2 */

    unsigned        SCORole:2;          /* Master (0), Slave (1) or passive (2) */
    unsigned        SCOEntries:2;       /* 0-2 */
  
    unsigned        A2DPStreamRole:2;   /* Master (0), Slave (1) or passive (2) */
    unsigned        A2DPStreamEntries:2;/* 0-2 */
       
    /* pointers to arrays of lp_power_tables */
    lp_power_table powertable[1];

} power_table;

/*!
    @brief This interface need to be used to read the local bd address from config entity. 
 
    @return void 
    Note: This interface must be invoked before using sinkDataGetLocalBdAddress()
*/
bool sinkDataInitLocalBdAddrFromPs(void);

/*!
    @brief This interface could be used to get reconnect on panic flag status. This flag could
    be used for checking panic action should be consider or not for panic occurred
 
    @return bool  TRUE if flag is enabled, else FALSE 
*/
bool sinkDataIsReconnectOnPanic(void);

/*!
    @brief This interface could be used to check after PDL reset a power cycle is required or not.
    If this function returns TRUE, then a power cycle need to be executed after a PDL reset.
 
    @return bool  TRUE if reset required, else FALSE.
*/
bool sinkDataIsPowerOffAfterPDLReset(void);

/*!
    @brief This interface could be used to check sink need to be discoverable all the time.
 
    @return bool  TRUE if discoverable all time, else FALSE.
*/
bool sinkDataIsDiscoverableAtAllTimes(void);

/*!
    @brief This interface could be used to power off need to disabled after power on.
 
    @return bool  TRUE if power off need to be disabled, else FALSE.
*/
bool sinkDataCheckDisablePowerOffAfterPowerOn(void);

/*!
    @brief This interface could be used to check sink need to entre pairing mode on a connection
    failure.
 
    @return bool  TRUE ,need to entre pairing mode, else FALSE.
*/
bool sinkDataEntrePairingModeOnConFailure(void);

/*!
    @brief This interface could be used to check power off condition based on V reg button
 
    @return bool  TRUE if flag is set, else FALSE.
*/
bool sinkDataIsPowerOffOnlyIfVRegEnlow(void);

/*!
    @brief This interface returns if we have been configured to power off when we have
    a critical battery and the charger is attached.
 
    @return bool  TRUE if flag is set, else FALSE.
*/
bool sinkDataAllowAutomaticPowerOffWhenCharging(void);

/*!
    @brief This interface could be used to check is it required to entre pairing mode automatically
 
    @return bool  TRUE if required to entre auto pairing mode, else FALSE.
*/
bool sinkDataAutoPairModeEnabled(void);

/*!
    @brief This interface could be used to check is it required to do auto reconnect on power on
 
    @return bool  TRUE if required to entre auto reconnect mode, else FALSE.
*/
bool sinkDataCheckAutoReconnectPowerOn(void);

/*!
    @brief This interface could be used to check is it required to disconnect during link loose do
    not want to  discoverable in link loss
 
    @return bool  TRUE if flag set, else FALSE.
*/
bool sinkDataCheckDoNotDisconDuringLinkLoss(void);

/*!
    @brief This interface could be used check MITM is required in authentication
 
    @return bool  TRUE if MITM required, else FALSE.
*/
bool sinkDataIsMITMRequired(void);

/*!
    @brief This interface could be used to check to send different event if first connection 
    since power on
 
    @return bool  TRUE if required to send different event, else FALSE.
*/
bool sinkDataDiffConnEventAtPowerOnEnabled(void);

/*!
    @brief This interface could be used to check if secure pairing is enabled
 
    @return bool  TRUE if secure pairing is required, else FALSE.
*/
bool sinkDataIsSecureParingEnabled(void);

/*!
    @brief This interface could be used to check if a reset is required after power off
 
    @return bool  TRUE if reset required, else FALSE.
*/
bool sinkDataIsResetAfterPowerOff(void);

/*!
    @brief This interface could be used to check if auto power on enabled after Init
 
    @return bool  TRUE if auto power on after init configured, else FALSE.
*/
bool sinkDataIsAutoPoweronAfterInit(void);

/*!
    @brief This interface could be used to if role switch is disabled or not
 
    @return bool  TRUE if role switch disabled, else FALSE.
*/
bool sinkDataIsRoleSwitchingDisbaled(void);

/*!
    @brief This interface could be used to check is there a need to be in connectable mode 
    during link loss
 
    @return bool  TRUE if need to go to connectable, else FALSE.
*/
bool sinkDataGoConnectableDuringLinkLoss(void);

/*!
    @brief This interface could be used to get the pairing mode timeout action
 
    @return uint8 value representing pairing mode time out action.
        PAIRTIMEOUT_CONNECTABLE    = 0,
        PAIRTIMEOUT_POWER_OFF       = 1,
        PAIRTIMEOUT_POWER_OFF_IF_NO_PDL = 2
*/
uint8 sinkDataGetActionOnParingmodeTimeout(void);

/*!
    @brief This interface could be used to get the power on action
 
    @return uint8 value representing power on action as defined in ARAction_t
*/
uint8 sinkDataGetPowerOnAction(void);

/*!
    @brief This interface could be used to check sink need to do to discovery mode if PDL is less
    than a configured value
 
    @return uint8 value representing the limiting number
*/
uint8 sinkDataGetPDLLimitForDiscovery(void);

/*!
    @brief This interface could be used to check sink need to go to RSSI pairing mode if PDL is less
    than a configured value
 
    @return uint8 value representing the limiting number
*/
uint8 sinkDataGetPDLLimitforRSSIPairing(void);

/*!
    @brief This interface could be used to get the panic reset action
 
    @return uint8 value as defined in ARAction_t
*/
uint8 sinkDataGetPanicResetAction(void);

/*!
    @brief This interface could be used to get number of attempts need to be done for a reconnection
 
    @return uint8 value represents the member of attempts
*/
uint8 sinkDataGetReconnectLastAttempts(void);

/*!
    @brief This interface could be used to get number of times to try reconnect before giving up
 
    @return uint16 value represents the member of times for retry
*/
uint16 sinkDataGetReconnectionAttempts(void);

/*!
    @brief This interface could be used to set number of times to try reconnect before giving up
    
    @param attempts Value represents the member of times for retry
 
    @return bool
*/
bool sinkDataSetReconnectionAttempts(uint16 attempts);

/*!
    @brief This interface could be used to get defragment key size
 
    @return uint8 value represents defragment key size
*/
uint8 sinkDataGetDefragKeySize(void);

/*!
    @brief This interface could be used to get defragment key min value
 
    @return uint8 value represents defragment key min value
*/
uint8 sinkDataGetDefragKeyMin(void);


/*!
    @brief This interfaces could be used to get the sub rate parameters for slc connection 
    
    @param slcParams Pointer to  ssr_params_t
 
    @return void
*/
void sinkDataGetSsrSlcParams(ssr_params_t*slcParams);

/*!
    @brief This interfaces could be used to get the sub rate parameters for sco connection 
    
    @param scoParams Pointer to  ssr_params_t
 
    @return void
*/
void sinkDataGetSsrScoParams(ssr_params_t*scoParams);
/*!
    @brief This interfaces could be used to get local BD address
    
    @param local_bd_addr Pointer to  bdaddr
 
    @return void
*/
void sinkDataGetLocalBdAddress(bdaddr *local_bd_addr);


/***************************   Timer API's  *******************************/

/*!
    @brief Interface to get the auto switch off timeout timer
 
    @return uint16 timer value
*/
uint16 sinkDataGetAutoSwitchOffTimeout(void);

/*!
    @brief Interface to set the auto switch off timeout timer
    
    @param timeout Timeout value.
 
    @return bool
*/
bool sinkDataSetAutoSwitchOffTimeout(uint16 timeout);

/*!
    @brief Interface to get the limbo timeout timer
 
    @return uint16 timer value
*/
uint16 sinkDataGetLimboTimeout(void);

/*!
    @brief Interface to set the limbo timeout timer
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetLimboTimeout(uint16 timeout);

/*!
    @brief Interface to get the power off after power on timeout timer
 
    @return uint16 timer value
*/
uint16 sinkDataGetDisablePoweroffAfterPoweronTimeout(void);

/*!
    @brief Interface to set the power off after power on timeout timer
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetDisablePoweroffAfterPoweronTimeout(uint16 timeout);

/*!
    @brief Interface to get the pairing mode timeout time

    @return uint16 time out value in secs
*/
uint16 sinkDataGetPairModeTimeout(void);

/*!
    @brief Interface to set the pairing mode timeout time

    @param timeout time value in secs

    @return bool true if the value was successfully written
*/
bool sinkDataSetPairModeTimeout(uint16 timeout);

/*!
    @brief Interface to get the connectable mode timeout time

    @return uint16 timeout time in secs
*/
uint16 sinkDataGetConnectableTimeout(void);

/*!
    @brief Interface to set the connectable mode timeout time

    @param timeout time in secs
 
    @return bool true if the value was successfully written
*/
bool sinkDataSetConnectableTimeout(uint16 timeout);

/*!
    @brief Interface to get the pairing mode PDL empty timeout time
 
    @return uint16 timeout time in secs
*/
uint16 sinkDataGetPairModePDLTimeout(void);

/*!
    @brief Interface to set the pairing mode PDL empty timeout time
    
    @param timeout time in secs
 
    @return bool true if the value was successfully written
*/
bool sinkDataSetPairModePDLTimeout(uint16 timeout);

/*!
    @brief Interface to get the encryption refresh timer
 
    @return uint16 timer value
*/
uint16 sinkDataGetEncryptionRefreshTimeout(void);

/*!
    @brief Interface to set the encryption refresh timer
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetEncryptionRefreshTimeout(uint16 timeout);

/*!
    @brief Interface to get the audio amp power down timeout
 
    @return uint16 timer value
*/
uint16 sinkDataAmpPowerDownTimeout(void);

/*!
    @brief Interface to set the audio amp power down timeout
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetAmpPowerDownTimeout(uint16 timeout);

/*!
    @brief Interface to get the second AG connection delay timeout timer
 
    @return uint16 timer value
*/
uint16 sinkDataSecondAGConnectDelayTimeout(void);

/*!
    @brief Interface to set the second AG connection delay timeout timer
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetSecondAGConnectDelayTimeout(uint16 timeout);

/*!
    @brief Interface to get the defragmanet check timeout timer
 
    @return uint16 timer value
*/
uint16 sinkDataDefragCheckTimout(void);

/*!
    @brief Interface to set the defragmanet check timeout timer
    
    @param timeout timer value
 
    @return bool
*/
bool sinkDataSetDefragCheckTimout(uint16 timeout);

/********************** Runtime Data Interfaces ****************************/

/*!
    @brief Interface to set user power table
    
    @param powerTable pointer to  power_table
 
    @return void
*/
void sinkDataSetUserPowerTable(power_table *powerTable);

/*!
    @brief Interface to get user power table
 
    @return power_table pointer to  power_table 
*/
power_table * sinkDataGetUserPowerTable(void);

/*!
    @brief Interface to set number of reconnection attempts 
    
    @param reconenctAttempts number of reconnection attempts
 
    @return void
*/
void sinkDataSetNoOfReconnectionAttempts(uint16 reconenctAttempts);

/*!
    @brief Interface to get number of reconnection attempts 
 
    @return uint16 number of reconnection attempts 
*/
uint16 sinkDataGetNoOfReconnectionAttempts(void);

/*!
    @brief Interface to set confirmation address 
    
    @param confirmation_addr Confirmation address
 
    @return void
*/
void sinkDataSetSMConfirmationAddress(tp_bdaddr *confirmation_addr);

/*!
    @brief Interface to get confirmation address 
 
    @return tp_bdaddr confirmation address
*/
tp_bdaddr * sinkDataGetSMConfirmationAddress(void);

/*!
    @brief Interface to set link loss bd address 
    
    @param linkloss_bd_addr Pointer to BD address
 
    @return void
*/
void sinkDataSetLinkLossBDaddr(bdaddr *linkloss_bd_addr);

/*!
    @brief Interface to get link loss bd address 
 
    @return bdaddr bd address of type bdaddr
*/
bdaddr *sinkDataGetLinkLossBDaddr(void);

/*!
    @brief Interface to set power off enable flag 
    
    @param poweroffEnable flag to set
 
    @return void
*/
void sinkDataSetPoweroffEnabled(bool poweroffEnable);

/*!
    @brief Interface to get power off enable  flag 
 
    @return bool TRUE  if power off enabled else FALSE
*/
bool sinkDataIsPowerOffEnabled(void);

/*!
    @brief Interface to set auth confirmation flag 
    
    @param authCfm TRUE to set FALSE to clear
 
    @return void
*/
void sinkDataSetAuthConfirmationFlag(bool authCfm);

/*!
    @brief Interface to get auth confirmation flag  
 
    @return  TRUE if set,else FALSE 
*/
bool sinkDataGetAuthConfirmationFlag(void);

/*!
    @brief Interface to set debug key enabled flag 
    
    @param debugKey TRUE to set ,FALSE to clear
 
    @return  void 
*/
void sinkDataSetDebugKeyEnabledFlag(bool debugKey);

/*!
    @brief Interface to get debug key enabled flag  
 
    @return  TRUE if set,else FALSE 
*/
bool sinkDataIsDebugKeyEnabled(void);

/*!
    @brief Interface to set multipoint enable 
    
    @param multipointEnable TRUE to set ,FALSE to clear
 
    @return  void 
*/
void sinkDataSetMultipointEnabled(bool multipointEnable);

/*!
    @brief Interface to get multipoint enabled flag 
 
    @return  TRUE if set,else FALSE 
*/
bool sinkDataIsMultipointEnabled(void);

/*!
    @brief Interface to set multipoint enabled flag in session data.For writing session data
 
    @return  TRUE if set,else FALSE 
*/
void sinkDataSetSessionMultiPointEnabled(void);

/*!
    @brief Interface to get multipoint enabled flag from session data. For reading session data
 
    @return  TRUE if set,else FALSE 
*/
bool sinkDataGetSessionMultiPointEnabled(void);


/*!
    @brief Interface to set power up no connection flag 
    
    @param powerupConenction TRUE to set FALSE to clear
 
    @return  void
*/
void sinkDataSetPowerUpNoConnectionFlag(bool powerupConenction);

/*!
    @brief Interface to get power up no connection flag 
 
    @return  TRUE if set,else FALSE 
*/
bool sinkDataGetPowerUpNoConnectionFlag(void);

/*!
    @brief Interface to set paging in progress 
    
    @param paging TRUE to set FALSE to clear
 
    @return  void
*/
void sinkDataSetPagingInProgress(bool paging);
/*!
    @brief Interface to get paging in progress status 
 
    @return  TRUE if pagin in proress, else FALSE
*/
bool sinkDataIsPagingInProgress(void);

/*!
    @brief Interface to set  event queued on connection
    
    @param event Event ID
 
    @return  void
*/
void sinkDataSetEventQueuedOnConnection(uint8 event);

/*!
    @brief Interface to get  event queued on connection
 
    @return  uint8  event ID
*/
uint8 sinkDataGetEventQueuedOnConnection(void);

/*!
    @brief Interface to set stream protection state
    
    @param streamState linkloss_stream_protection_state_t type
 
    @return void
*/
void sinkDataSetStreamProtectionState(linkloss_stream_protection_state_t streamState);

/*!
    @brief Interface to get stream protection state
 
    @return linkloss_stream_protection_state_t
*/
linkloss_stream_protection_state_t sinkDataGetStreamProtectionState(void);

/*!
    @brief Interface to set sin init status
    
    @param initialising TRUE if initialising ,else FALSE
 
    @return void
*/
void sinkDataSetSinkInitialisingStatus(bool initialising);

/*!
    @brief Interface to get sink init status
 
    @return bool TRUE if initialising ,else FALSE
*/
bool sinkDataGetSinkInitialisingStatus(void);


#ifdef ENABLE_SQIFVP 
/*!
    @brief Interface to set SQUIF partition mount
    
    @param mount Partition  mount
 
    @return void
*/
void sinkDataSetSquifPartitionMount(uint8 mount);
/*!
    @brief Interface to get SQUIF partition mount
 
    @return uint8 partition  mount
*/
uint8 sinkDataGetSquifMountedPartitions(void);

/*!
    @brief Interface to get free SQUIF partition 
 
    @return uint8  Free partition
*/
uint8 sinkDataGetSquifFreePartitions(void);
#endif /* ENABLE_SQIFVP */

/*!
    @brief Interface to set connection in progress 
    
    @param connectionProgress TRUE if connection in progress, else FALSE
 
    @return void
*/
void sinkDataSetConnectionInProgress(bool connectionProgress);

/*!
    @brief Interface to get connection in progress 
 
    @return bool TRUE if connection in progress, else FALSE
*/
uint16 *sinkDataIsConnectionInProgress(void);

/*!
    @brief Interface to  set panic reconnect flag
    
    @param panicReset TRUE to set,FALSE to clear
 
    @return void
*/
void sinkDataSetPanicReconnect(bool panicReset);

/*!
    @brief Interface to get  panic reconnect flag
 
    @return bool TRUE if set,else FALSE
*/
bool sinkDataGetPanicReconnect(void);
/**********************************************************************
    Interfaces for geting DisableCompletePowerOff.
*/
bool sinkDataIsDisableCompletePowerOff(void);

/**********************************************************************
    Interfaces for geting EncryptOnSLCEstablishment.
*/
bool sinkDataIsEncryptOnSLCEstablishment(void);

/**********************************************************************
    Interfaces for geting EncryptOnSLCEstablishment.
*/
uint16 sinkDataGetPDLSize(void);

/*!
    @brief Interface to set dfu access state
    
    @param access Access
 
    @return void
*/
void sinkDataSetDfuAccess(bool access);

/*!
    @brief Interface to get dfu access state
 
    @return bool
*/
bool sinkDataGetDfuAccess(void);

/**********************************************************************
    @brief Interfaces to get display link keys flag
    
    @return bool
 */
bool sinkDataGetDisplayLinkKeys(void);

/**********************************************************************
    @brief Interfaces to set display link keys flag
    
    @return void
 */
void sinkDataSetDisplayLinkKeys(bool displayLinkKeys);

/**********************************************************************
    @brief Interface to get pairing mode reminder interval
 */
uint16 sinkDataGetPairingReminderInterval(void);

#endif /* _SINK_PRIVATE_DATA_H_ */

