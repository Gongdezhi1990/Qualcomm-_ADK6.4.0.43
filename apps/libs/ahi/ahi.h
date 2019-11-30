/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    ahi.h

DESCRIPTION
    Header file for the Application Host Interface library.

*/
/*!
@file   ahi.h
@brief  Definitions for the Application Host Interface library.

        The Application Host Interface (AHI) is an library
        that allows a Host to do various tasks related to the
        VM application on a device.

        Areas of functionality include:

          Configuring the VM application.

          Automated test harness support.
*/

#ifndef AHI_H_
#define AHI_H_

#include <library.h>
#include <message.h>


/*!
    @brief Status codes returned by AHI library.
*/
typedef enum {
    /*! Operation completed successfully. */
    ahi_status_success = 0,

    /*! The AHI library must be initialised for this operation. */
    ahi_status_not_initialised,

    /*! One of the parameters is out of range or otherwise 'bad'. */
    ahi_status_bad_parameter,

    /*! Requested operation uses a shared resource that is already in use. */
    ahi_status_already_in_use,
    
    /*! Operation failed due to a lack of memory. */
    ahi_status_no_memory
} ahi_status_t;

/*!
    @brief Application mode enum used by AHI.

    The VM application can be running in one of the modes
    defined in this enum.
*/
typedef enum
{
    /*! Application is running in the mode it would use in a production device.
        Changing configuration data by the Host is not allowed.
        AHI will return an error to the Host if it attempts to change any
        configuration data.
        The automated test functionality of AHI is disabled. */
    ahi_app_mode_normal,

    /*! Application is running in a minimal functionality mode that allows
        configuration data changes to be made without risk of causing a panic.
        Changing configuration data by the Host is allowed. 
        The automated test functionality of AHI is disabled.*/
    ahi_app_mode_configuration,

    /*! Application is running with additional functionality that allows
        automated testing of the device from the Host.
        AHI will return an error to the Host if it attempts to change any
        configuration data.
        The automated test functionality of AHI is enabled. */
    ahi_app_mode_normal_test,

    /*! Application mode is not recognised. For example, if the application
        mode in an AHI message cannot be converted into a valid enum value. */
    ahi_app_mode_undefined,

    ahi_app_mode_max
} ahi_application_mode_t;

/*!
    @brief Messages from AHI to the application
*/
typedef enum
{
    AHI_APP_MODE_CHANGE_REQ = AHI_MESSAGE_BASE,
    AHI_APP_TRANSPORT_DISABLE_REQ,

    AHI_LAST_APP_MESSAGE
} ahi_application_message_t;

/*!
    @brief Request the VM app to go into the given mode.

    When the VM app receives this message it must reply by calling
    AhiAppModeChangeCfm so that AHI can inform the Host of the result of the
    mode change request.

    If the mode change requires a reboot then the application should
    allow a short amount of time after calling AhiAppModeChangeCfm so
    that AHI can complete sending the reply to the Host.
*/
typedef struct
{
    ahi_application_mode_t app_mode;
} AHI_APP_MODE_CHANGE_REQ_T;

/*!
    @brief Request the VM app to disable the AHI transport on the next reboot.
*/
typedef struct
{
    bool enabled;
} AHI_APP_TRANSPORT_DISABLE_REQ_T;

/*!
    @brief Initialise the Application Host Interface.

    @param app_task Task that AHI will send messages intended for the application to.
    @param app_mode The current application mode.

    @return ahi_status_success if init went ok.
            ahi_status_bad_parameter if app_mode is invalid.
            ahi_status_already_in_use if ahi is already initialised.
*/
ahi_status_t AhiInit(Task app_task, ahi_application_mode_t app_mode);

/*!
    @brief Give AHI the reply to a AHI_APP_MODE_CHANGE_REQ.

    The VM app must call this function to tell AHI if the request to change
    app mode is allowed. 

    If the change is allowed then app_mode should be set to the new app mode.
    If the change is not allowed then app_mode should be set to the current
    app mode.

    AHI will return app_mode to the Host and the Host can use it
    to confirm if the mode change request was successful.

    A mode change may require a reboot of the device. If this is the case then
    the reboot must not happen directly after this function has been called.
    Instead there should be a short delay in order to allow AHI to inform
    the Host about the impending reboot.

    @param app_mode The current application mode.
    @param need_reboot TRUE if the device will soon reboot; FALSE otherwise.
*/
void AhiAppModeChangeCfm(ahi_application_mode_t app_mode, bool need_reboot);

/*!
    @brief Give AHI the reply to a AHI_TRANSPORT_DISBALE_REQ.
*/
void AhiAppTransportDisableCfm(ahi_status_t status);

/*!
    @brief AHI -> transport message types.

    When AHI needs to send and AHI message to the Host, it will send
    one of the messages defined here to the AHI transport.

    The AHI transport must perform any transport specific formatting
    when sending the AHI data packet to the host.

    The AHI transport must tell the AHI library that it has finished
    processing the message by calling AhiTransportSendDataAck.
    It must call this even if there was an error sending the message.
*/
typedef enum
{
    AHI_TRANSPORT_SEND_DATA = (AHI_MESSAGE_BASE + 0x80),

    AHI_LAST_TRANSPORT_MESSAGE
} ahi_transport_message_t;

/*!
    @brief A Device -> Host AHI data packet.

    The contents of payload are stored as an array of bytes that must be sent
    to the host in the same order as they are stored in memory.
*/
typedef struct
{
    uint16 size;        /**< Number of elements in the payload array */
    uint8 payload[1];   /**< Contents of the data packet to send to the host */
} AHI_TRANSPORT_SEND_DATA_T;

/*!
    @brief Process a Host -> Device AHI data packet.

    The data packet must contain only the AHI message data.
    Any transport specific data should not be passed into this function.

    @param data Contents of the data packet.
    @param size Size of the data packet.
*/
void AhiTransportProcessData(Task transport_task, const uint8 *data);

/*!
    @brief Acknowledge that an AHI data packet has been consumed.

    This function tells the AHI library that the outgoing transport
    has finished processing an AHI_TRANSPORT_SEND_ message and has
    released any resources it used.

    It does not need to indicate if the message was sent to the Host
    successfully or not. AHI only needs to know when the resources
    have been released.
*/
void AhiTransportSendDataAck(void);

/*!
    @brief Returns the initialized app mode.
*/
ahi_application_mode_t AhiGetSupportedMode(void);

/*!
    @brief Send an application event to the Host.

    Echo an application or library message to the host.
    If the message contains data this can optionally be echoed to the Host
    as well. If it is larger than a single AHI data packet it will be
    truncated.

    This functionality is for automated testing only.

    The Event Id is only sent to the Host if there is an active AHI
    connection and AHI is in the
    ahi_application_mode_t::ahi_app_mode_normal_test mode.

    @param event_id Id of the event.
    @param payload Pointer to the extra data of the event. If there is no
                   extra data set this to 0.
    @param payload_size Size of the payload in words, for example the value
                        returned by sizeof(). Use 0 if there is no payload.
*/
void AhiTestReportEvent(uint16 event_id, const uint8 *payload, uint16 payload_size);

/*
    @brief Send the current state of a state machine to the Host.

    This functionality is for automated testing only.

    The current state of the given state machine is only sent to the Host if
    there is an active AHI connection and AHI is in the 
    ahi_application_mode_t::ahi_app_mode_normal_test mode.

    @param machine_id State machine Id. The meaning of the Id is 
                      specific to the VM application.
    @param state Current state Id of the given state machine. The meaning
                 of the Id is specific to the state machine.
*/
void AhiTestReportStateMachineState(uint16 machine_id, uint16 state);

/*
    @brief Send the negotiated GATT MTU 

    This functionality is for automated testing only.

    The current state of the given state machine is only sent to the Host if
    there is an active AHI connection and AHI is in the 
    ahi_application_mode_t::ahi_app_mode_normal_test mode.

    @param mtu GATT MTU size. The meaning of the Id is 
                      specific to the VM application.
*/
void AhiTestReportMtu(uint16 mtu);


/*
    @brief Send the BLE Connection Interval params

    This functionality is for automated testing only.

    The current state of the given state machine is only sent to the Host if
    there is an active AHI connection and AHI is in the 
    ahi_application_mode_t::ahi_app_mode_normal_test mode.

    @param ci  negotiated connection interval.The meaning of the Id is 
                      specific to the VM application.
*/
void AhiTestReportConnectionInterval(uint16 conn_interval);

/*
    @brief Send event to HATS for printing VM logs
 
    Sends an event to HATS test system to print VM code logs in conjunction
    with HATS system logs for debugging purpose.
 
    @param vm_print_buffer contains vm log string.
*/
void AhiPrintVmLogs(const char *vm_print_buffer);

/*!
    @brief Set the application build identifier.

    @param build_id Application Build ID.
*/
ahi_status_t AhiSetAppBuildId(uint16 build_id);

/*!
    @brief Get the application build identifier.

*/
ahi_status_t AhiGetAppBuildId(uint16 *build_id);


#endif /* AHI_H_ */
