/****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ahi.c

DESCRIPTION
    Implementation of the Sink AHI.

*/
/*!
@file   sink_ahi.c
@brief  Implementation of the Sink AHI.
*/

#include <string.h>
#include <ps.h>
#include <ahi.h>

#include "ahi_host_usb.h"
#include "ahi_host_spi.h"
#include "ahi_host_trb.h"
#include "sink_ahi.h"
#include "sink_configmanager.h"
#include "sink_led_err.h"
#include "sink_leds.h"


/* PS Key used for Sink AHI config. */
#define SINK_AHI_CONFIG_PS_KEY                          (CONFIG_AHI)

/* Number of words used in the ps key for Sink AHI config. */
#define SINK_AHI_CONFIG_ITEMS                           (2)

/* Offset of words used in ps key for Sink AHI config. */
#define SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET (0)
#define SINK_AHI_CONFIG_APP_MODE_OFFSET                 (1)

/* Default value for USB HID Datalink */
#define SINK_AHI_CONFIG_USB_HID_DATALINK_STATE_DEFAULT  sink_ahi_usb_hid_datalink_enabled

/* Default value for app mode */
#define SINK_AHI_CONFIG_APP_MODE_DEFAULT                ahi_app_mode_normal


/* Private functions */
static ahi_application_mode_t getAppModeFromPsKey(void);
static ahi_status_t initAhi(Task app_task, ahi_application_mode_t app_mode);
static sink_ahi_status_t checkSinkAhiConfig(void);
static sink_ahi_status_t createDefaultSinkAhiConfig(void);
static void readConfigItem(uint16 offset, uint16* item);
static sink_ahi_status_t writeConfigItem(uint16 offset, uint16 item);
static uint16 getDefaultConfigValue(uint16 offset);
static bool isUsbHidDatalinkStateCorrect(uint16 state);
static bool isAppModeCorrect(uint16 app_mode);


ahi_application_mode_t currentAppMode;


/******************************************************************************/
void sinkAhiEarlyInit(void)
{
    currentAppMode = getAppModeFromPsKey();
}


/******************************************************************************/
void sinkAhiInit(Task app_task)
{
    if( checkSinkAhiConfig() != sink_ahi_status_succeeded )
        LedsIndicateError(led_err_id_enum_ahi);
    
    if( initAhi(app_task, sinkAhiGetAppMode()) != ahi_status_success )
        LedsIndicateError(led_err_id_enum_ahi);
}


/******************************************************************************/
bool sinkAhiIsRebootRequired(ahi_application_mode_t new_app_mode){
    
    ahi_application_mode_t current_app_mode = sinkAhiGetAppMode();
    
     /* Reboot is needed if we are switching from or to config mode */
    if(((current_app_mode == ahi_app_mode_configuration) && (new_app_mode != ahi_app_mode_configuration)) ||
       ((current_app_mode != ahi_app_mode_configuration) && (new_app_mode == ahi_app_mode_configuration)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


/******************************************************************************/
bool sinkAhiIsUsbHidDataLinkEnabled(void)
{
    uint16 usb_enabled;

    readConfigItem(SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET, &usb_enabled);
    return (usb_enabled == sink_ahi_usb_hid_datalink_enabled ? TRUE : FALSE );
}


/******************************************************************************/
ahi_application_mode_t sinkAhiGetAppMode(void)
{
    return currentAppMode;
}


/******************************************************************************/
sink_ahi_status_t sinkAhiSetUsbHidDataLinkState(sink_ahi_usb_hid_datalink_state_t state)
{
    if( !isUsbHidDatalinkStateCorrect(state) )
    {
        return sink_ahi_status_wrong_param;
    }
        
    return writeConfigItem(SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET, state);
}


/******************************************************************************/
sink_ahi_status_t sinkAhiSetAppMode(ahi_application_mode_t mode)
{
    if( !isAppModeCorrect(mode) )
    {
        return sink_ahi_status_wrong_param;
    }
    
    if( sinkAhiIsRebootRequired(mode) )
    {
        return writeConfigItem(SINK_AHI_CONFIG_APP_MODE_OFFSET, mode);
    }
    else
    {
        currentAppMode = mode;
        return sink_ahi_status_succeeded;
    }
}


/***************************************************************************
NAME
    getAppModeFromPsKey
 
DESCRIPTION
    Function that retrieves the stored app mode from PS Store.

    This should only ever be called once from sinkAhiEarlyInit.
    Once sink_ahi is intitialised all app mode get and set requests
    must go via sinkAhiGetAppMode and sinkAhiSetAppMode.

RETURNS
    Application mode stored in dedicated PS key.
*/
static ahi_application_mode_t getAppModeFromPsKey(void)
{
    uint16 app_mode;

    readConfigItem(SINK_AHI_CONFIG_APP_MODE_OFFSET, &app_mode);

    /* Always revert the app mode value in the ps key to "normal",
       so that it does not persist over another reboot. */
    writeConfigItem(SINK_AHI_CONFIG_APP_MODE_OFFSET, ahi_app_mode_normal);

    return app_mode;
}


/***************************************************************************
NAME
    initAhi
 
DESCRIPTION
    Function that initializes AHI library.
    
PARAMS
    app_task Task that AHI will send messages intended for the application to.
    app_mode The current application mode
    
RETURNS
    ahi_status_success if everything was OK, err code otherwise.
*/
static ahi_status_t initAhi(Task app_task, ahi_application_mode_t app_mode)
{
    ahi_status_t status;

    status = AhiInit(app_task, app_mode);
    if (ahi_status_success != status)
    {
        return status;
    }
    
#ifdef APP_BUILD_ID
    status = AhiSetAppBuildId(APP_BUILD_ID);
    if (ahi_status_success != status)
    {
        return status;
    }
#endif /* APP_BUILD_ID */
        
#ifdef ENABLE_AHI_USB_HOST
    AhiUSBHostInit();
#endif

#ifdef ENABLE_AHI_SPI
    AhiSpiHostInit();
#endif

#ifdef ENABLE_AHI_TRB
    AhiTrbHostInit();
#endif

    return ahi_status_success;
}


/***************************************************************************
NAME
    checkSinkAhiConfig
 
DESCRIPTION
    Function that checks if Sink Ahi config stored in dedicated PS Key
    has correct values.
 
RETURNS
    sink_ahi_status_succeeded if everything was OK, err code otherwise.
*/
static sink_ahi_status_t checkSinkAhiConfig(void){

    uint16 read_data_size;
    uint16 usb_enabled;
    uint16 app_mode;
    
    /* Check if the size of config data stored in ps key is correct */
    read_data_size = PsRetrieve(SINK_AHI_CONFIG_PS_KEY, 0, 0);
    if( read_data_size == 0 )
    {
        /* There is no data in sink ahi ps key */
        return createDefaultSinkAhiConfig();
    }
    else if( read_data_size != SINK_AHI_CONFIG_ITEMS )
    {
        return sink_ahi_status_config_incorrect;
    }

    readConfigItem(SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET, &usb_enabled);
    if( !isUsbHidDatalinkStateCorrect(usb_enabled) )
    {
        return sink_ahi_status_config_incorrect;
    }

    readConfigItem(SINK_AHI_CONFIG_APP_MODE_OFFSET, &app_mode);
    if( !isAppModeCorrect(app_mode) )
    {
        return sink_ahi_status_config_incorrect;
    }
    
    return sink_ahi_status_succeeded;
}


/***************************************************************************
NAME
    createDefaultSinkAhiConfig
 
DESCRIPTION
    Function that writes default configuration for Sink AHI to the
    ps key which is selected for that purpose.
 
RETURNS
    sink_ahi_status_succeeded if everything was OK, err code otherwise.
*/
static sink_ahi_status_t createDefaultSinkAhiConfig(void)
{
    uint16 default_config[SINK_AHI_CONFIG_ITEMS];
    uint16 data_written;

    /* Set default sink ahi config data */
    default_config[SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET] = SINK_AHI_CONFIG_USB_HID_DATALINK_STATE_DEFAULT;
    default_config[SINK_AHI_CONFIG_APP_MODE_OFFSET] = SINK_AHI_CONFIG_APP_MODE_DEFAULT;
    /* Write default sink ahi congig to ps key */
    data_written = PsStore(SINK_AHI_CONFIG_PS_KEY, (const void*)default_config, SINK_AHI_CONFIG_ITEMS);
   
    return (data_written == SINK_AHI_CONFIG_ITEMS ? sink_ahi_status_succeeded : sink_ahi_status_config_write );
}


/***************************************************************************
NAME
    readConfigItem
 
DESCRIPTION
    Function that reads selected item from Sink AHI config ps key.
*/
static void readConfigItem(uint16 offset, uint16* item)
{
    uint16 read_data_size;
    uint16 config_data[SINK_AHI_CONFIG_ITEMS];


    read_data_size = PsRetrieve(SINK_AHI_CONFIG_PS_KEY, (void*)config_data, SINK_AHI_CONFIG_ITEMS);

    if( read_data_size == SINK_AHI_CONFIG_ITEMS )
    {
        *item = config_data[offset];
    }
    else
    {
        *item = getDefaultConfigValue(offset);
    }
}


/***************************************************************************
NAME
    WriteConfigItem
 
DESCRIPTION
    Function that writes selected item to the Sink AHI config ps key.
 
RETURNS
    sink_ahi_status_succeeded if everything was OK, err code otherwise.
*/
static sink_ahi_status_t writeConfigItem(uint16 offset, uint16 item)
{
    uint16 config_data_size;
    
    /* Check if the size of config data stored in ps key is correct */
    config_data_size = PsRetrieve(SINK_AHI_CONFIG_PS_KEY, 0, 0);
    if( config_data_size == SINK_AHI_CONFIG_ITEMS )
    {
        uint16 written_data_size;
        uint16 config_data[SINK_AHI_CONFIG_ITEMS];
        /* Retrieve whole sink ahi config */
        config_data_size = PsRetrieve(SINK_AHI_CONFIG_PS_KEY, (void*)config_data, SINK_AHI_CONFIG_ITEMS);
        if( config_data_size != SINK_AHI_CONFIG_ITEMS )
        {
            return sink_ahi_status_config_read;
        }
        /* Modify selected item */
        config_data[offset] = item;
        written_data_size = PsStore(SINK_AHI_CONFIG_PS_KEY, (const void*)config_data, SINK_AHI_CONFIG_ITEMS);
        
        if( written_data_size != SINK_AHI_CONFIG_ITEMS )
        {
            return sink_ahi_status_config_write;
        }
        
        return sink_ahi_status_succeeded;
    }
        
    return sink_ahi_status_config_incorrect;
}


/***************************************************************************
NAME
    getDefaultConfigValue
 
DESCRIPTION
    Function that returns default config item
    for a given item offset in Sink AHI config ps key.
    
PARAMS
    offset offset of a given item.

RETURNS
    default config value.
*/
static uint16 getDefaultConfigValue(uint16 offset)
{
    switch(offset)
    {
        case SINK_AHI_CONFIG_USB_HID_DATALINK_ENABLED_OFFSET:
            return SINK_AHI_CONFIG_USB_HID_DATALINK_STATE_DEFAULT;
        case SINK_AHI_CONFIG_APP_MODE_OFFSET:
            return SINK_AHI_CONFIG_APP_MODE_DEFAULT;
        default:
            return 0;
    }
}


/***************************************************************************
NAME
    isUsbHidDatalinkStateCorrect
 
DESCRIPTION
    Function that checks if passed USB HID state
    has correct value.
    
PARAMS
    state USB HID state that is being checked.

RETURNS
    True if value is correct, false otherwise.
*/
static bool isUsbHidDatalinkStateCorrect(uint16 state)
{
    if( state == sink_ahi_usb_hid_datalink_enabled ||
        state == sink_ahi_usb_hid_datalink_disabled )
    {
        return TRUE;
    }
    return FALSE;
}


/***************************************************************************
NAME
    isAppModeCorrect
 
DESCRIPTION
    Function that checks if passed app mode
    has correct value.

PARAMS
    app_mode application mode that is being checked.
 
RETURNS
    True if value is correct, false otherwise.
*/
static bool isAppModeCorrect(uint16 app_mode){
    
    if( app_mode == ahi_app_mode_normal ||
        app_mode == ahi_app_mode_configuration ||
        app_mode == ahi_app_mode_normal_test )
    {
        return TRUE;
    }
    return FALSE;
}
