/*
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief
    Configuration manager for the sink device - responsible for extracting user information out of the
    PSKEYs and initialising the configurable nature of the devices' components

*/
#ifndef SINK_CONFIG_MANAGER_H
#define SINK_CONFIG_MANAGER_H

#include <csrtypes.h>
#include <message.h>

/* DSP keys */
#define CONFIG_DSP_BASE  (50)
#define CONFIG_DSP(x)    (CONFIG_DSP_BASE + x)
#define CONFIG_DSP_SESSION_KEY     (CONFIG_DSP(49))
#define CONFIG_DSP_SESSION_SIZE    (48) /* allow 4 x entry of 12 words currently used by CVC */

/* Persistent store key allocation  */
#define CONFIG_BASE  (0)


/***********************************************************************/
/***********************************************************************/
/* ***** do not alter order or insert gaps as device will panic ***** */
/***********************************************************************/
/***********************************************************************/
enum
{
    CONFIG_AHI                                = 1,
    CONFIG_DEFRAG_TEST                        = 2,
    CONFIG_PERMANENT_PAIRING                  = 3,
    CONFIG_UPGRADE_CONTEXT                    = 4,
    CONFIG_BROADCAST_AUDIO_MODE               = 5,
    CONFIG_FAST_PAIR_ACCOUNT_KEY_INDEX        = 6,
    CONFIG_FAST_PAIR_ACCOUNT_KEYS             = 7
};



/****************************************************************************
NAME
 	configManagerInit

DESCRIPTION
 	Initialises all subcomponents in order. If the full_init is set to false then only some of the
 	subcomponents are initialized which are required to display the LED pattern on EventUsrEnterDFUMode event.

RETURNS
 	void

*/
void configManagerInit(  bool full_init);


/****************************************************************************
NAME
  	configManagerInitFeatures

DESCRIPTION
  	Read and configure the system features from PS

RETURNS
  	void

*/
void configManagerInitFeatures( void );


/****************************************************************************
NAME
  	configManagerInitMemory

DESCRIPTION
  	Init static size memory blocks that are required early in the boot sequence

RETURNS
  	void

*/

void configManagerInitMemory( void ) ;

/****************************************************************************
NAME
 	configManagerEnableMultipoint

DESCRIPTION
    Enable or disable multipoint

RETURNS
 	void
*/
void configManagerEnableMultipoint(bool enable);

/****************************************************************************
NAME
 	configManagerWriteSessionData

DESCRIPTION
    Stores the persistent session data across power cycles.
	This includes information like volume button orientation
	Audio Prompt language etc.

RETURNS
 	void
*/
void configManagerWriteSessionData( void ) ;


/****************************************************************************
NAME
     configManagerReadSessionData

DESCRIPTION
    Reads the persistent session data after power cycles.
	This includes information like volume button orientation
	Audio Prompt language. last requested source etc.

RETURNS
 	void
*/
void configManagerReadSessionData( void );


/****************************************************************************
NAME
 	configManagerWriteDspData

*/
void configManagerWriteDspData( void );

/****************************************************************************
NAME
  	configManagerFillPs

DESCRIPTION
  	Fill PS to the point defrag is required (for testing only)

RETURNS
  	void
*/
void configManagerFillPs(void);

/****************************************************************************
NAME
    configManagerDefragIfRequired

DESCRIPTION
    Check if PS Defrag is required, if it is then flood PS to force a
    PS Defragment operation on next reboot.

RETURNS
    void
*/
void configManagerDefragIfRequired(void);

/****************************************************************************
NAME
  	configManagerRestoreDefaults

DESCRIPTION
    Restores default PSKEY settings.
    This function restores the following:
        1. CONFIG_SESSION_DATA
        2. HFP data Phone Number config Block
        3. Clears the paired device list
        4. Enables the LEDs
        5. Disable multipoint
        6. Disable lbipm
        7. Reset EQ

RETURNS
  	void
*/

void configManagerRestoreDefaults( void ) ;

/****************************************************************************
NAME
  	configManagerSqifPartitionsInit

DESCRIPTION
    Reads the PSKEY containing the SQIF partition configuration.

RETURNS
  	void
*/
#if defined ENABLE_SQIFVP
void configManagerSqifPartitionsInit( void );
#endif

/****************************************************************************
NAME
    configManagerProcessEventSysDefrag

DESCRIPTION
    Processes defrag related system events

RETURNS
    void
*/
void configManagerProcessEventSysDefrag(const MessageId defragEvent);

/******************************************************************************
NAME    
    configManagerGetReadOnlyConfig
    
DESCRIPTION
    Open a config block as read-only.

    The pointer to the memory for the config block is returned in *data.

    If the block cannot be opened, this function will panic.

PARAMS
    config_id [in] Id of config block to open
    data [out] Will be set with the pointer to the config data buffer if
               successful, NULL otherwise.

RETURNS
    uint16 size of the opened config block buffer. The size can be 0,
    e.g. if the config is an empty array.Here size returned is not 
    equivalent to sizeof(), but represents the number of uint16's
    containing config blocks information.
*/
uint16 configManagerGetReadOnlyConfig(uint16 config_id, const void **data);

/******************************************************************************
NAME
    configManagerGetWriteableConfig
 
DESCRIPTION
    Open a config block as writeable.

    The pointer to the memory for the config block is returned in *data.

    If the block cannot be opened, this function will panic.
 
PARAMS
    config_id [in] Id of config block to open
    data [out] Will be set with the pointer to the config data buffer if
               successful, NULL otherwise.
    size [in] Size of the buffer to allocate for the config data.
              Set this to 0 to use the size of config block in the
              config store.

RETURNS
    uint16 Size of the opened config block buffer. The size can be 0,
           e.g. if the config is an empty array.
*/
uint16 configManagerGetWriteableConfig(uint16 config_id, void **data, uint16 size);

/******************************************************************************
NAME    
    configManagerReleaseConfig
    
DESCRIPTION
    Release the given config block so that config_store can release any
    resources it is using to keep track of it.

    After this has been called any pointers to the config block data buffer
    will be invalid.

PARAMS
    config_id Id of the config block to release.

RETURNS
    void
*/
void configManagerReleaseConfig(uint16 config_id);

/******************************************************************************
NAME    
    configManagerUpdateWriteableConfig
    
DESCRIPTION
    Update the config block data in the config store and release it.
   
    After this has been called any pointers to the config block data buffer
    will be invalid.

PARAMS
    config_id Id of the config block to update and release.

RETURNS
    void
*/
void configManagerUpdateWriteableConfig(uint16 config_id);

#endif
