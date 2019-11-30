/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ahi.h

DESCRIPTION
    Header file for Sink AHI utility.

*/
/*!
@file   sink_ahi.h
@brief  Functions which are helping with maintenance of AHI related
        configuration of Sink application.


*/

#ifndef SINK_AHI_H_
#define SINK_AHI_H_

#include <ahi.h>
#include <stdlib.h>


/*!
    @brief Sink AHI operation status codes
 
    Defines all possible status codes that may be returned
    by Sink AHI public API functions.
*/
typedef enum __sink_ahi_status
{
    sink_ahi_status_succeeded = 0,          /*! Operation Succeeded */
    sink_ahi_status_config_incorrect,       /*! Existing configuration kept under Sink Ahi ps key is incorrect */
    sink_ahi_status_config_read,            /*! Retrieving Sink AHI config from ps store failed */
    sink_ahi_status_config_write,           /*! Storing Sink AHI config in ps store failed */
    sink_ahi_status_wrong_param             /*! Wrong parameters passed to the function */
 
} sink_ahi_status_t;


/*!
    @brief States of AHI's USB HID Datalink transport
 
    It can be either enabled or disabled.
*/
typedef enum __sink_ahi_usb_hid_datalink_state
{
    sink_ahi_usb_hid_datalink_disabled = 0,    /*! USB HID Datalink transport disabled in Sink App */
    sink_ahi_usb_hid_datalink_enabled          /*! USB HID Datalink transport enabled in Sink App */
 
} sink_ahi_usb_hid_datalink_state_t;





/*
    @brief Function that initializes Sink Ahi private data.
           It should be called before any other Sink Ahi API is used.
*/
void sinkAhiEarlyInit(void);


/*
    @brief Function that initializes Sink Ahi. If there are any errors
           with Sink Ahi config, it will enter the infinite loop, and indicate
           error using LEDs.
           
    @param app_task Task that AHI will send messages intended for the
                         application to.
*/
void sinkAhiInit(Task app_task);


/*
    @brief Function that checks if after the transition to the new 
           application mode reboot will be required.

    @param new_app_mode Application mode into which we want to switch.

    @return TRUE if reboot is required, FALSE otherwise.
*/
bool sinkAhiIsRebootRequired(ahi_application_mode_t new_app_mode);


/*
    @brief  Function that checks if the USB HID Datalink transport is enabled.
    
    @return TRUE if USB HID Datalink is enabled, FALSE otherwise.
*/
bool sinkAhiIsUsbHidDataLinkEnabled(void);


/*
    @brief Function that returns the app mode that is currently set.

    @return app mode that is currently set.
*/
ahi_application_mode_t sinkAhiGetAppMode(void);


/*
    @brief Function that sets the USB HID Datalink transport status in Sink AHI
           configuration ps key with a given value.

    @param state USB HID Datalink transport state to be set.
                    
    @return      sink_ahi_status_succeeded if everything was OK,
                 err code otherwise.
*/
sink_ahi_status_t sinkAhiSetUsbHidDataLinkState(sink_ahi_usb_hid_datalink_state_t state);


/*
    @brief Function that sets the application mode in Sink AHI configuration
           ps key with a given value.

    @param mode Application mode to be set.
                    
    @return     sink_ahi_status_succeeded if everything was OK,
                err code otherwise.
*/
sink_ahi_status_t sinkAhiSetAppMode(ahi_application_mode_t mode);


#endif /* SINK_AHI_H_ */
