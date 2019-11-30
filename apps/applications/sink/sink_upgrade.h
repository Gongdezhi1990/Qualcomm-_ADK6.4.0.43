/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_upgrade.h
    
DESCRIPTION
    Interface to the upgrade library.

*/

#ifndef _SINK_UPGRADE_H_
#define _SINK_UPGRADE_H_


/*************************************************************************
NAME
    sinkUpgradeInit

DESCRIPTION
    Initialise the Upgrade library
*/
#ifdef ENABLE_UPGRADE
void sinkUpgradeInit(Task task);
#else
#define sinkUpgradeInit(task) ((void)0)
#endif

/*******************************************************************************
NAME
    sinkUpgradeIsUpgradeMsg
    
DESCRIPTION
    Check if a message should be handled by sinkUpgradeMsgHandler
    
PARAMETERS
    id      The ID for the message
    
RETURNS
    bool TRUE if it is an upgrade message, FALSE otherwise.
*/
#ifdef ENABLE_UPGRADE
bool sinkUpgradeIsUpgradeMsg(uint16 id);
#else
#define sinkUpgradeIsUpgradeMsg(id) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkUpgradeMsgHandler
    
DESCRIPTION
    Handle messages specific to the Upgrade library.
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the Upgrade message
    message The message payload
    
RETURNS
    void
*/
#ifdef ENABLE_UPGRADE
void sinkUpgradeMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkUpgradeMsgHandler(task, id, message) ((void)0)
#endif

/*******************************************************************************
NAME
    sinkUpgradePowerEventHandler

DESCRIPTION
    Handle power events and pass relevant ones to the Upgrade library.

PARAMETERS
    void

RETURNS
    void
*/
#ifdef ENABLE_UPGRADE
void sinkUpgradePowerEventHandler(void);
#else
#define sinkUpgradePowerEventHandler() ((void)0)
#endif

/*******************************************************************************
NAME
    SinkUpgradeRunningNewImage

DESCRIPTION
    Determines if the current image is the new image after upgrade
    Based on the current upgrade state, and the sink_upgrade config values,
    decide if the config_store should be started in the special 'const' mode
    or not.

    Note: This should only ever be called during the early init when
          ConfigStoreInit is called.

PARAMETERS
    void

RETURNS
    void
*/
#ifdef ENABLE_UPGRADE
bool SinkUpgradeRunningNewImage(void);
#else
#define SinkUpgradeRunningNewImage() (FALSE)
#endif

/*******************************************************************************
NAME
    sinkUpgradeSetUpgradeTransportType

DESCRIPTION
    Store the upgrade transport type in the sink_upgrade context PS key.

PARAMETERS
    trasnport_type gaia transport type to store.

RETURNS
    void
*/
#ifdef ENABLE_UPGRADE
void sinkUpgradeSetUpgradeTransportType(uint16 transport_type);
#else
#define sinkUpgradeSetUpgradeTransportType(type) ((void)0)
#endif

/*******************************************************************************
NAME
    sinkUpgradeGetUpgradeTransportType

DESCRIPTION
    Retrieve the upgrade transport type from the sink_upgrade context PS key.

PARAMETERS
    void

RETURNS
    Stored gaia_transport_type.
*/
#ifdef ENABLE_UPGRADE
uint16 sinkUpgradeGetUpgradeTransportType(void);
#else
#define sinkUpgradeGetUpgradeTransportType() (0)
#endif

/*******************************************************************************
NAME
    sinkUpgradeIsProtectAudio

DESCRIPTION
    Get the value of 'protect audio during audio'
    This will be Disabled by default, meaning, on an ongoing upgrade, audio will be suspended and 
    Upgrade improvements will apply.
    If Enabled, ongoing audio will be streaming, while upgrade improvements may not be applicable.

RETURNS
    TRUE if audio needs to be retained, FALSE otherwise
*/
#ifdef ENABLE_UPGRADE
bool sinkUpgradeIsProtectAudio(void);
#else
#define sinkUpgradeIsProtectAudio() (0)
#endif


#endif /* _SINK_UPGRADE_H_ */
