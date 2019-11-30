/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp.h
    
DESCRIPTION 

    Header file for the Audio/Video Remote Control Profile library.  This
    profile library implements the AVRCP using the services of the AVCTP
    library which is hidden from the Client application by this library
    
    The library exposes a functional downstream API and an upstream message 
    based API.
    
     CLIENT APPLICATION
      |         |
      |    AVRCP Library
      |         |
      |      |
     CONNECTION Library
             |
         BLUESTACK
    
*/
/*!

\defgroup avrcp avrcp
\ingroup vm_libs

\brief    Interface to the Audio Video Remote Control Profile library.

\section avrcp_intro INTRODUCTION
        This profile library implements the AVRCP.  This library permits one
        device known as the controller (CT) to send dedicated user actions to
        another device known as the target (TG).
        
        Note: This library does not handle audio streaming, this is implemented
        in the GAVDP library.
    
        The library exposes a functional downstream API and an upstream message
        based API.
*/

/** @{ */

#ifndef AVRCP_H_
#define AVRCP_H_


#include <bdaddr_.h>
#include <library.h>
#include <message.h>


struct __AVRCP;
/*!
    @brief The Audio Video Remote Control Profile structure.
*/
typedef struct __AVRCP AVRCP;


/*!
    @brief The page data length.
*/
#define PAGE_DATA_LENGTH    (4)

/*!  AVRCP Supported Features Flag Defines

    These flags can be or'd together and used as the supported_features field
    of an avrcp_init_params structure. 
*/

/*!
    @brief Setting this flag when the library is initialised indicates that 
    this device implements category 1 commands.
*/
#define AVRCP_CATEGORY_1                    0x01
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 2 commands.
*/
#define AVRCP_CATEGORY_2                    0x02
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 3 commands.
*/
#define AVRCP_CATEGORY_3                    0x04
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements category 4 commands.
*/
#define AVRCP_CATEGORY_4                    0x08
/*!
    @brief Setting this flag when the library is initialised indicates that
     this device implements player application settings. Player application
     settings are valid only for Category 1 Target devices. 
     AVRCP_VERSION_1_3 flag shall be enabled in the profile_extensions
     to turn ON this flag. 
*/
#define AVRCP_PLAYER_APPLICATION_SETTINGS    (0x10 |  AVRCP_CATEGORY_1)
/*!
    @brief Setting this flag when the library is initialised indicates that 
    this device implements Group Navigation. Group Navigation is valid only
    for  Category 1 Target devices.
*/
#define AVRCP_GROUP_NAVIGATION                (0x20 | AVRCP_CATEGORY_1)
/*!
     @brief Setting this flag when the library is initialised indicates that
     this device implements Virtual File system browsing. Library ignores this
     bit if AVRCP_BROWSING_SUPPORTED bit is not set in the profile_extensions 
     field of the avrcp_init_params structure while initialising the library.
*/
#define AVRCP_VIRTUAL_FILE_SYSTEM_BROWSING     0x40
/*!
    @brief Setting this flag when the library is initialised indicates that
     this Target device implements Multiple Media Player applications.
     It is valid  only for Category 1 Target devices. Library ignores this
     bit if AVRCP_BROWSING_SUPPORTED bit is not set in the profile_extensions.
*/
#define AVRCP_MULTIPLE_MEDIA_PLAYERS           0x80



/*! This flag enables the Deprecated part of APIs and Code */
/* #define AVRCP_ENABLE_DEPRECATED 1 */

/*!  AVRCP Extensions Flag Defines

    These flags can be or'd together and used as the profile_extensions field
    of an avrcp_init_params structure.
*/

/*!
    @brief Setting this flag when the library is initialised indicates that
    this device implements the AVRCP version 1.3 . If this bit is not set
    in profile_extensions field for the target device, AVRCP Lib will act as 
    v1.0 Library.
*/
#define AVRCP_VERSION_1_3                   0x01

#ifdef AVRCP_ENABLE_DEPRECATED
/*! This flag is deprecated */
#define AVRCP_EXTENSION_METADATA            AVRCP_VERSION_1_3
#endif /* AVRCP_ENABLE_DEPRECATED */


/*!
    @brief This bit shall be set if the library supports version 1.4 AVRCP
*/
#define AVRCP_VERSION_1_4                  (0x02 | AVRCP_VERSION_1_3)

/*!
   @brief Setting this flag when the library is initialised indicates that 
   this device implements the AVRCP 1.4 browsing channel support. If this bit
   is not set in profile_extensions field for the target device, AVRCP library
   will be initialised as version 1.3 for Cat 1 or 3 devices.     
*/
#define AVRCP_BROWSING_SUPPORTED          (0x04  | AVRCP_VERSION_1_4 )
                                              

/*!
   @brief Setting this flag in profile_extensions field when the library is 
   initialised indicates that this device implements the AVRCP 1.4 
   Search feature. 
*/
#define AVRCP_SEARCH_SUPPORTED            (0x08 | AVRCP_BROWSING_SUPPORTED)

/*!
   @brief Setting this flag in profile_extensions field when the library is 
   initialised indicates that this Target device supports Database Aware 
   Media Players.
*/
#define AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED (0x10 | AVRCP_BROWSING_SUPPORTED)

/*!
    @brief This bit shall be set if the library supports version 1.5 AVRCP
*/
#define AVRCP_VERSION_1_5                  (0x20 | AVRCP_VERSION_1_4)

/*!
    @brief This bit shall be set if the library supports version 1.6 AVRCP
*/
#define AVRCP_VERSION_1_6                  (0x40 | AVRCP_VERSION_1_5)



/*!
   @brief Setting this flag in profile_extensions field while initialising 
   the library will turn on all AVRCP 1.4 features.
*/
#define AVRCP_14_ALL_FEATURES_SUPPORTED     0x1F




/*!  List of Media Attributes definitions.
    Application must use defined values for Media Attributes while framing the 
    attributes Data for AvrcpGetElementAttributesRequest() and 
    AvrcpGetElementAttributesResponse() APIs.
*/

/*!    @brief Title of the media.*/
#define AVRCP_MEDIA_ATTRIBUTE_TITLE     0x01

/*!    @brief Name of the artist.*/
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST    0x02

/*!    @brief Name of the album*/
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM     0x03

/*! @brief Number of the media(e.g. Track number of the CD) */
#define AVRCP_MEDIA_ATTRIBUTE_NUMBER    0x04

/*! @brief Total number of the media (e.g.Total track number of the CD)*/
#define AVRCP_MEDIA_ATTRIBUTE_TOTAL_NUMBER  0x05

/*! @brief Genre*/
#define AVRCP_MEDIA_ATTRIBUTE_GENRE     0x06

/*! @brief Playing Time in milliseconds*/
#define AVRCP_MEDIA_ATTRIBUTE_PLAYING_TIME 0x07


/*!   List of defined Player Application Settings Attributes and Values. 
    Application should use defined values for Player Application Settings
    Attributes and values while framing the attributes and values for the APIs.

    AvrcpListAppAttributeResponse(),
    AvrcpListAppValueRequest(),
    AvrcpListAppValueResponse(),
    AvrcpGetCurrentAppValueRequest(),
    AvrcpGetCurrentAppValueResponse(),
    AvrcpSetAppValueRequest(),
    AvrcpGetAppAttributeTextRequest(),
    AvrcpGetAppAttributeTextResponse() and
    AvrcpGetAppValueTextRequest().
*/


/*!  @brief Equalizer ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_EQUALIZER      0x01

/*!  @brief Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_REPEAT_MODE    0x02

/*!  @brief Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SHUFFLE        0x03

/*!  @brief Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN           0x04

/*!  @brief OFF status value all Equalizer ON/OFF Attribute ID.*/
#define AVRCP_PLAYER_VALUE_EQUALIZER_OFF      0x01

/*!  @brief ON status value all Equalizer ON/OFF Attribute ID.*/
#define AVRCP_PLAYER_VALUE_EQUALIZER_ON       0x02

/*!  @brief OFF value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_OFF    0x01

/*!  @brief Single Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_SINGLE  0x02

/*!  @brief All Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_ALL     0x03

/*!  @brief Group Track Repeat value for Repeat Mode status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_REPEAT_MODE_GROUP   0x04

/*!  @brief OFF value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_OFF         0x01   

/*!  @brief All Track Shuffle value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_ALL         0x02   

/*!  @brief Group Track Shuffle value for Shuffle ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_VALUE_SHUFFLE_GROUP       0x03   

/*!  @brief OFF value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_OFF        0x01

/*!  @brief All Track value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_ALL        0x02

/*!  @brief Group Track value for Scan ON/OFF status Attribute ID.*/
#define AVRCP_PLAYER_ATTRIBUTE_SCAN_GROUP      0x03



/*!  List of AVRCP PDU IDs defined in AVRCP 1.4 specification.
    Application should use these values for requesting or aborting 
    the Continuation response if the last metadata response from 
    the Target device was a fragmented response.
*/  



/*!  @brief PDU ID of GetCapabilities Command */
#define AVRCP_GET_CAPS_PDU_ID                       0x10

/*!  @brief ListPlayerApplicationSettingAttributes command PDU ID*/
#define AVRCP_LIST_APP_ATTRIBUTES_PDU_ID            0x11

/*!  @brief ListPlayerApplicationSettingValues command PDU ID */
#define AVRCP_LIST_APP_VALUE_PDU_ID                 0x12

/*!  @brief GetCurrentPlayerApplicationSettingValue command PDU ID*/
#define AVRCP_GET_APP_VALUE_PDU_ID                  0x13

/*!  @brief SetPlayerApplicationSettingValue command PDU ID*/
#define AVRCP_SET_APP_VALUE_PDU_ID                  0x14

/*!  @brief GetPlayerApplicationSettingAttributeText command PDU ID*/
#define AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID         0x15

/*!  @brief GetPlayerApplicationSettingValueText command PDU ID*/
#define AVRCP_GET_APP_VALUE_TEXT_PDU_ID             0x16

/*!  @brief InformDisplayableCharacterSet command PDU ID*/
#define AVRCP_INFORM_CHARACTER_SET_PDU_ID           0x17

/*!  @brief GetElementAttributes command PDU ID*/
#define AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID         0x20



/*!   AVRCP Response and Status values.
    Target application must use the defined avrcp_response_type values while 
    using the Response APIs. AVRCP library uses the avrcp_status_code in the
    confirmation message to the application on completion of requested
    operation.
*/


/*! 
    @brief AVRCP responses. Values are defined in AV/C Digital Interface spec.
    Additional library defined responses have been added.
*/
typedef enum
{
    /*! The specified profile is not acceptable. */
    avctp_response_bad_profile = 0x01,
    /*! The request is not implemented. */
    avctp_response_not_implemented = 0x08,    
    /*! The request has been accepted. This response should be used when
        accepting commands    with AV/C command type of CONTROL. */
    avctp_response_accepted = 0x09,            
    /*! The request has been rejected. */
    avctp_response_rejected = 0x0A,            
    /*! The target is in a state of transition. */
    avctp_response_in_transition = 0x0B,    
    /*! A stable response.This response should be used when accepting commands
    with AV/C command type of STATUS. */
    avctp_response_stable = 0x0C,            
    /*! The target devices state has changed. This response should be used 
        when accepting commands    with AV/C command type of NOTIFY. */
    avctp_response_changed = 0x0D,            
    /*! The response is an interim response. This response should be used when
        accepting commands    with AV/C command type of NOTIFY. */
    avctp_response_interim = 0x0F, 
           
    /*! More specific error status responses for rejecting the Meta Data AVC
        commands and Browsing commands are as follows.Error status codes
        defined in the AVRCP1.4 specification can be retrieved by masking msb
        (0x80) of the defined response codes here. Ensure to keep the same
        values while inserting or modifying following enum values.  
    */

       /*! The request has been rejected if TG received a PDU that it did not 
        understand. This is valid for all Command Responses*/
    avrcp_response_rejected_invalid_pdu = 0x80,

    /*! The request has been rejected with reason - invalid parameter.
        If the TG received a PDU with a parameter ID that it did not 
        understand.
        Send if there is only one parameter ID in the PDU. Valid for all
        Commands.
    */
    avrcp_response_rejected_invalid_param, /* 0x80 + 0x01 */

    /*! The request has been rejected with reason - invalid content.
       Send if the parameter ID is understood, but content is wrong or 
       corrupted.
       Valid for all commands.*/
    avrcp_response_rejected_invalid_content, /* 0x80 + 0x2 */

    /*! The request has been rejected with reason - internal error.
        Valid for all commands*/
    avrcp_response_rejected_internal_error, /* 0x80 + 0x03*/

    /*! The request has been successful. This response shall be used
        with Browsing commands only */
    avrcp_response_browsing_success, /* 0x80 + 0x04 */

    /*! The request has been rejected with reason - UID Changed. 
        The UIDs on the device have changed */
    avrcp_response_rejected_uid_changed = 0x85, /* 0x80 + 0x05 */

    /*!  The request has been rejected with reason -  Invalid Direction. 
         The Direction parameter is invalid. Response code is valid only for 
         ChangePath command */
    avrcp_response_rejected_invalid_direction = 0x87, /* 0x80 + 0x07 */

    /*! The request has been rejected with reason - Not a Directory. 
        The UID provided does not refer to a folder item. Response code is
        valid only for ChangePath command */
    avrcp_response_rejected_not_directory, /*0x80 + 0x08 */

    /*! The request has been rejected with reason - Does not exist.
        The UID provided does not refer to any currently valid Item. 
        This response code is valid for commands - Change Path, PlayItem, 
        AddToNowPlaying, GetItemAttributes */
    avrcp_response_rejected_uid_not_exist, /*0x80 + 0x09 */

    /*! The request has been rejected with reason - Invalid Scope.
        The scope parameter is invalid. This response code is valid for 
        commands
        - GetFolderItems, PlayItem, AddToNowPlayer, GetItemAttributes.
    */
    avrcp_response_rejected_invalid_scope, /*0x80 + 0x0A */

    /*! The request has been rejected with reason - Range Out of Bounds 
        The start of range provided is not valid. This response is valid for 
        GetFolderItems command.*/
    avrcp_response_rejected_out_of_bound, /* 0x80 + 0x0B */

    /*! The request has been rejected with reason - UID is a Directory.
        The UID provided refers to a directory, which cannot be handled by
        this media player. This response is valid for commands - PlayItem and
        AddToNowPlaying
    */
    avrcp_response_rejected_uid_directory, /*0x80 + 0x0C */

    /*! The request has been rejected with reason - Media in Use.
        The media is not able to be used for this operation at this time.
        This response is valid for commands - PlayItem and AddToNowPlaying.
     */
    avrcp_response_rejected_media_in_use, /*0x80 + 0x0D */

    /*! The request has been rejected with reason - Now Playing List Full.
        No more items can be added to the Now Playing List.
        This response is valid for command - AddToNowPlaying
    */
    avrcp_response_rejected_play_list_full, /*0x80 + 0x0E */

    /*! This request has been rejected with reason - Search Not Supported.
        The Browsed Media Player does not support search.
        This response is valid for command - Search.
    */
    avrcp_response_rejected_search_not_supported, /*0x80 + 0x0F*/

    /*! This request has been rejected with reason - Search in Progress
        A search operation is already in progress.
        This response is valid for command - Search
    */
    avrcp_response_rejected_search_in_progress, /*0x80 + 0x10*/

    /*! This request has been rejected with reason - Invalid Player Id
        The specified Player Id does not refer to a valid player.
        This response is valid for commands - SetAddressedPlayer and
        SetBrowsedPlayer
    */
    avrcp_response_rejected_invalid_player_id, /*0x80 + 0x11*/

    /*! This request has been rejected with reason - Player Not Browsable
        The Player Id supplied refers to a Media Player which does not 
        support browsing. This response is valid for commands - 
        SetBrowsedPlayer
    */
    avrcp_response_rejected_player_not_browsable, /*0x80 + 0x12*/

    /*! This request has been rejected with reason - Player Not Addressed.
        The Player Id supplied refers to a player which is not currently
        addressed, and the command is not able to be performed if the player
        is not set as addressed. This response is valid for commands - 
        Search and SetBrowsedPlayer.
    */
    avrcp_response_rejected_player_not_addressed, /*0x80 + 0x13*/

    /*! This request has been rejected with reason - No valid Search Results.
        The Search result list does not contain valid entries, e.g. after 
        being invalidated due to change of browsed player - This response is
        valid for GetFolderItems
    */
    avrcp_response_rejected_no_valid_search_results, /*0x80 + 0x14*/

    /*! This request has been rejected with reason - No available players */
    avrcp_response_rejected_no_available_players, /*0x80 + 0x15*/

    /*! This request has been rejected with reason - Addressed Player Changed.
        This is valid for command - Register Notifications*/
    avrcp_response_rejected_addressed_player_changed, /*0x80 + 0x16 */ 

    /* Dummy Place Holder */
    avrcp_response_guard_reserved = 0xFF

} avrcp_response_type;

/*!
    @brief AVRCP status codes 
*/
typedef enum
{
    /*! Operation was successful. */
    avrcp_success = (0),            
    /*! Operation failed. */
     avrcp_fail,                        
    /*! Not enough resources. */
    avrcp_no_resource,                
    /*! Request is not supported in the current state. */
    avrcp_bad_state,                
    /*! Operation timed out before completion. */
    avrcp_timeout,                    
    /*! Device specified is not connected. */
    avrcp_device_not_connected,        
    /*! Operation is already in progress. */
    avrcp_busy,                        
    /*! Requested operation is not supported. */
    avrcp_unsupported,                
    /*! Sink supplied was invalid. */
    avrcp_invalid_sink,
    /*! Link loss occurred. */
    avrcp_link_loss,
    /*! The operation was rejected. */
    avrcp_rejected=0x0A,
    /*! General failure during AVRCP Browsing channel initialization*/
    avrcp_browsing_fail,
    /*! Browsing channel is not connected */
    avrcp_browsing_channel_not_connected,
    /*! Remote device does not support Browsing */
    avrcp_remote_browsing_not_supported,
    /*! Timeout on browsing channel */
    avrcp_browsing_timedout,
    /*! Operation was successful, but has only received an
      interim response.*/
    avrcp_interim_success=0x0F,

    /* Below status codes depends on the error status code received from the 
       remote device. Retain the same values while inserting new values or
       modifying this enum */

    /*! The operation was rejected with reason - invalid PDU. */
    avrcp_rejected_invalid_pdu = 0x80,
    /*! The operation was rejected with reason - invalid parameter. */
    avrcp_rejected_invalid_param,
    /*! The operation was rejected with reason - invalid content. */
    avrcp_rejected_invalid_content,
    /*! The operation was rejected with reason - internal error. */
    avrcp_rejected_internal_error,

    /*! The operation was rejected with reason - UID Changed. */
    avrcp_rejected_uid_changed = 0x85, 
    /*! The command has been rejected with reason -Invalid Direction.*/
    avrcp_rejected_invalid_direction = 0x87, 
    /*! The command has been rejected with reason -Not a Directory.*/
    avrcp_rejected_not_directory, 
    /*! The command has been rejected with reason -Does not exist.*/
    avrcp_rejected_uid_not_exist, 
    /*! The command has been rejected with reason -Invalid Scope.*/
    avrcp_rejected_invalid_scope, 
    /*! The command has been rejected with reason - Range Out of Bounds.*/
    avrcp_rejected_out_of_bound, 
    /*! The command has been rejected with reason - UID is a Directory.*/
    avrcp_rejected_uid_directory, 
    /*! The command has been rejected with reason - Media in Use.*/
    avrcp_rejected_media_in_use, 
    /*! The command has been rejected with reason - Now Playing List Full.*/
    avrcp_rejected_play_list_full, 
    /*! The command has been rejected with reason - Search Not Supported.*/
    avrcp_rejected_search_not_supported, 
    /*! The command has been rejected with reason - Search in Progress.*/
    avrcp_rejected_search_in_progress, 
    /*! This command has been rejected with reason - Invalid Player ID.*/
    avrcp_rejected_invalid_player_id, 
    /*! This command has been rejected with reason - Player Not Browsable.*/
    avrcp_rejected_player_not_browsable,
    /*! This command has been rejected with reason - Player Not Addressed.*/
    avrcp_rejected_player_not_addressed, 
    /*! This command has been rejected with reason - No valid Search Results.*/
    avrcp_rejected_no_valid_search_results, 
    /*! This command has been rejected with reason - No available players.*/
    avrcp_rejected_no_available_players, 
    /*! This command has been rejected with reason -Addressed Player Changed.*/
    avrcp_rejected_addressed_player_changed,

    /* Dummy Place Holder */
    avrcp_status_guard_reserverd = 0xFF
} avrcp_status_code;



/*! 
    @brief Operation ID, used to identify operation. See table 9.21 AV/C Panel
    Subunit spec. 1.1 #
*/
typedef enum
{
    opid_select                = (0x0),
    opid_up,
    opid_down,
    opid_left,
    opid_right,
    opid_right_up,
    opid_right_down,
    opid_left_up,
    opid_left_down,
    opid_root_menu,
    opid_setup_menu,
    opid_contents_menu,
    opid_favourite_menu,
    opid_exit,
    /* 0x0E to 0x1F Reserved */
    opid_0                    = (0x20),
    opid_1,
    opid_2,
    opid_3,
    opid_4,
    opid_5,
    opid_6,
    opid_7,
    opid_8,
    opid_9,
    opid_dot,
    opid_enter,
    opid_clear,
    /* 0x2D - 0x2F Reserved */
    opid_channel_up            = (0x30),
    opid_channel_down,
    opid_sound_select,
    opid_input_select,
    opid_display_information,
    opid_help,
    opid_page_up,
    opid_page_down,
    /* 0x39 - 0x3F Reserved */
    opid_power                = (0x40),
    opid_volume_up,
    opid_volume_down,
    opid_mute,
    opid_play,
    opid_stop,
    opid_pause,
    opid_record,
    opid_rewind,
    opid_fast_forward,
    opid_eject,
    opid_forward,
    opid_backward,
    /* 0x4D - 0x4F Reserved */
    opid_angle                = (0x50),
    opid_subpicture,
    /* 0x52 - 0x70 Reserved */
    opid_f1                    = (0x71),
    opid_f2,
    opid_f3,
    opid_f4,
    opid_f5,
    opid_vendor_unique        = (0x7E)
    /* Ox7F Reserved */
} avc_operation_id; 


/*!
    @brief Subunit types 
*/
typedef enum
{
    subunit_monitor            = (0x0),
    subunit_audio,
    subunit_printer,
    subunit_disc,
    subunit_tape_recorder_player,
    subunit_tuner,
    subunit_CA,
    subunit_camera,
    subunit_reserved,
    subunit_panel,
    subunit_bulletin_board,
    subunit_camera_storage,
    /* 0x0C - 0x1B Reserved */
    subunit_vendor_unique    = (0x1C),
    subunit_reserved_for_all,
    subunit_extended,
    subunit_unit
} avc_subunit_type;

/*!
    @brief AVRCP device type

    The AVRCP library can be configured to be either a target or a controller
    device.
*/
typedef enum
{
    avrcp_target = 0x01,
    avrcp_controller,
    avrcp_target_and_controller
} avrcp_device_type;


/*!
    @brief AVRCP Metadata transfer capability ID.

    The capability ID type for capabilities supported by the target. 
*/
typedef enum
{
    avrcp_capability_company_id = 0x02,
    avrcp_capability_event_supported = 0x03
} avrcp_capability_id;


/*!
    @brief AVRCP Metadata transfer event IDs.

    The specification mandates the TG to support a number of event IDs.
    Optionally it may also support a number of other events not defined 
    by the specification. This type covers all events defined by the 
    Metadata transfer specification.
*/
typedef enum 
{
    avrcp_event_playback_status_changed = 0x01,
    avrcp_event_track_changed,                      /* 0x02 */
    avrcp_event_track_reached_end,                  /* 0x03 */
    avrcp_event_track_reached_start,                /* 0x04 */
    avrcp_event_playback_pos_changed,               /* 0x05 */
    avrcp_event_batt_status_changed,                /* 0x06 */
    avrcp_event_system_status_changed,              /* 0x07 */
    avrcp_event_player_app_setting_changed,         /* 0x08 */
    avrcp_event_now_playing_content_changed,        /* 0x09 */
    avrcp_event_available_players_changed,          /* 0x0A */
    avrcp_event_addressed_player_changed,           /* 0x0B */
    avrcp_event_uids_changed,                       /* 0x0C */
    avrcp_event_volume_changed                      /* 0x0D */
} avrcp_supported_events;


/*!
    @brief AVRCP play status events.

    Possible values of play status.
*/
typedef enum 
{
    avrcp_play_status_stopped = 0x00,
    avrcp_play_status_playing = 0x01,
    avrcp_play_status_paused = 0x02,
    avrcp_play_status_fwd_seek = 0x03,
    avrcp_play_status_rev_seek = 0x04,
    avrcp_play_status_error = 0xFF
} avrcp_play_status;


/*!
    @brief AVRCP battery status events.

    Possible values of battery status.
*/
typedef enum 
{
    avrcp_battery_status_normal = 0x00,
    avrcp_battery_status_warning,
    avrcp_battery_status_critical,
    avrcp_battery_status_external,
    avrcp_battery_status_full_charge
} avrcp_battery_status;


/*!
    @brief AVRCP system status events.

    Possible values of system status.
*/
typedef enum 
{
    avrcp_system_status_power_on = 0x00,
    avrcp_system_status_power_off,
    avrcp_system_status_unplugged
} avrcp_system_status;


/*!
    @brief AVRCP character sets.

    character set values defined in IANA character set document available at 
    http://www.iana.org/assignments/character-sets
*/
typedef enum
{
    avrcp_char_set_ascii = 3,
    avrcp_char_set_iso_8859_1 = 4,
    avrcp_char_set_jis_x0201 = 15,
    avrcp_char_set_shift_jis = 17,
    avrcp_char_set_ks_c_5601_1987 = 36,
    avrcp_char_set_utf_8 = 106,
    avrcp_char_set_ucs2 = 1000,
    avrcp_char_set_utf_16be = 1013,
    avrcp_char_set_gb2312 = 2025,
    avrcp_char_set_big5 = 2026
} avrcp_char_set;

/*!
    @brief AVRCP initialisation parameters

    The initialisation parameters allow the profile instance to be configured
    either as a controller or target. 
*/

typedef struct
{
    /*! Specifies if this device is controller (CT), target (TG), or both. 
        This filed will be ineffective if the library is enabled with 
        only Controller or Target role support.*/ 
    avrcp_device_type device_type;

    /*! The supported controller features must be filled in if the device 
        supports the controller (CT) role or the library will default to a 
        possibly undesired default setting. The features for the CT must state
        which categories are supported (1 - 4). See the AVRCP Supported Feature
        Flag Defines at the top of avrcp.h. This value reflects the Supported
        features attribute value in the SDP service record of controller. 
        This field will be ignored if the library is built with only the 
        Target support.*/
    uint8 supported_controller_features;

    /*! The supported target features must be filled in if the device supports
        the target(TG) role or the library will default to a possibly undesired
        default setting.The features of the TG must state which categories are 
        supported (1 - 4),and can also indicate support for the Player 
        Application settings and Group Navigation. 
        See the AVRCP Supported Features Flag Defines at the top of avrcp.h. 
        This field will be ignored if the library is built with only the 
        Controller support.*/
    uint8 supported_target_features;

    /*! Set to zero if no extensions are supported in the Target application. 
        If this bit is not set, library acts as v1.0 . If
        supported_target_features sets the PlayerApplicationSettings or Group 
        Navigation bits, this value must be set.
        If extensions are supported (eg. AVRCP Metadata extensions), 
        use the AVRCP Extensions Flag Defines from the top of avrcp.h. 
     */
    uint8 profile_extensions;

} avrcp_init_params;

/* Browsing Data Structures */
/*!
    @brief Direction for browsing navigation.
*/
typedef enum 
{
    avrcp_direction_up   = 0x00,
    avrcp_direction_down = 0x01

}avrcp_browse_direction;

/*!
    @brief The four scopes in which media content navigation may take place.
*/
typedef enum{
    avrcp_media_player_scope = 0,
    avrcp_virtual_filesystem_scope,
    avrcp_search_scope,
    avrcp_now_playing_scope 
}avrcp_browse_scope;

/*!
    @brief The 8 octets UID to identify the media elements.
*/
typedef struct{
    uint32      msb;
    uint32      lsb;
}avrcp_browse_uid;

/*!
    @brief AV/C protocol - Used to form the targets address
*/
typedef uint16 avc_subunit_id;



/*
    Do not document this part.
*/
#ifndef DO_NOT_DOCUMENT

/* 
    Upstream AVRCP library messages base. Do not document this.
    Its base value is AVRCP_MESSAGE_BASE, which is defined in
    library.h.
*/


typedef enum
{
    /* Library initialisation*/
    AVRCP_INIT_CFM = AVRCP_MESSAGE_BASE,        /* 0x5E00 */     
            
    /* Connection/ disconnection management */
    AVRCP_CONNECT_CFM,                          /* 0x5E01 */
    AVRCP_CONNECT_IND,                          /* 0x5E02 */
    AVRCP_DISCONNECT_IND,                       /* 0x5E03 */
    
    /* AV/C Specific */
    AVRCP_PASSTHROUGH_CFM,                      /* 0x5E04 */
    AVRCP_PASSTHROUGH_IND,                      /* 0x5E05 */
    AVRCP_UNITINFO_CFM,                         /* 0x5E06 */
    AVRCP_UNITINFO_IND,                         /* 0x5E07 */
    AVRCP_SUBUNITINFO_IND,                      /* 0x5E08 */ 
    AVRCP_SUBUNITINFO_CFM,                      /* 0x5E09 */ 
    AVRCP_VENDORDEPENDENT_CFM,                  /* 0x5E0A */
    AVRCP_VENDORDEPENDENT_IND,                  /* 0x5E0B */

    /* AVRCP Metadata transfer extension */
    AVRCP_GET_CAPS_CFM,                         /* 0x5E0C */
    AVRCP_GET_CAPS_IND,                         /* 0x5E0D */
    AVRCP_LIST_APP_ATTRIBUTE_CFM,               /* 0x5E0E */
    AVRCP_LIST_APP_ATTRIBUTE_IND,               /* 0x5E0F */
    AVRCP_LIST_APP_VALUE_IND,                   /* 0x5E10 */
    AVRCP_LIST_APP_VALUE_CFM,                   /* 0x5E11 */
    AVRCP_GET_APP_VALUE_CFM,                    /* 0x5E12 */
    AVRCP_GET_APP_VALUE_IND,                    /* 0x5E13 */
    AVRCP_SET_APP_VALUE_CFM,                    /* 0x5E14 */
    AVRCP_SET_APP_VALUE_IND,                    /* 0x5E15 */
    AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM,           /* 0x5E16 */
    AVRCP_GET_APP_ATTRIBUTE_TEXT_IND,           /* 0x5E17 */
    AVRCP_GET_APP_VALUE_TEXT_CFM,               /* 0x5E18 */
    AVRCP_GET_APP_VALUE_TEXT_IND,               /* 0x5E19 */
    AVRCP_GET_ELEMENT_ATTRIBUTES_CFM,           /* 0x5E1A */
    AVRCP_GET_ELEMENT_ATTRIBUTES_IND,           /* 0x5E1B */
    AVRCP_GET_PLAY_STATUS_CFM,                  /* 0x5E1C */
    AVRCP_GET_PLAY_STATUS_IND,                  /* 0x5E1D */
    AVRCP_REGISTER_NOTIFICATION_CFM,            /* 0x5E1E */
    AVRCP_REGISTER_NOTIFICATION_IND,            /* 0x5E1F */
    AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND,    /* 0x5E20 */
    AVRCP_EVENT_TRACK_CHANGED_IND,              /* 0x5E21 */
    AVRCP_EVENT_TRACK_REACHED_END_IND,          /* 0x5E22 */
    AVRCP_EVENT_TRACK_REACHED_START_IND,        /* 0x5E23 */
    AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND,       /* 0x5E24 */
    AVRCP_EVENT_BATT_STATUS_CHANGED_IND,        /* 0x5E25 */
    AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND,      /* 0x5E26 */
    AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND, /* 0x5E27 */
    AVRCP_REQUEST_CONTINUING_RESPONSE_CFM,      /* 0x5E28 */
    AVRCP_ABORT_CONTINUING_RESPONSE_CFM,        /* 0x5E29 */
    AVRCP_NEXT_GROUP_CFM,                       /* 0x5E2A */
    AVRCP_NEXT_GROUP_IND,                       /* 0x5E2B */
    AVRCP_PREVIOUS_GROUP_CFM,                   /* 0x5E2C */
    AVRCP_PREVIOUS_GROUP_IND,                   /* 0x5E2D */
    AVRCP_INFORM_BATTERY_STATUS_CFM,            /* 0x5E2E */
    AVRCP_INFORM_BATTERY_STATUS_IND,            /* 0x5E2F */
    AVRCP_INFORM_CHARACTER_SET_CFM,             /* 0x5E30 */
    AVRCP_INFORM_CHARACTER_SET_IND,             /* 0x5E31 */
    AVRCP_GET_SUPPORTED_FEATURES_CFM,           /* 0x5E32 */
    AVRCP_GET_EXTENSIONS_CFM,                   /* 0x5E33 */
    AVRCP_SET_ABSOLUTE_VOLUME_IND,              /* 0x5E34 */
    AVRCP_SET_ABSOLUTE_VOLUME_CFM,              /* 0x5E35 */
    AVRCP_EVENT_VOLUME_CHANGED_IND,             /* 0x5E36 */

    /* AVRCP Browsing related Messages */
    AVRCP_SET_ADDRESSED_PLAYER_IND,             /* 0x5E37 */
    AVRCP_SET_ADDRESSED_PLAYER_CFM,             /* 0x5E38 */
    AVRCP_PLAY_ITEM_IND,                         /*0x5E39 */
    AVRCP_PLAY_ITEM_CFM,                        /* 0x5E3A */
    AVRCP_ADD_TO_NOW_PLAYING_IND,               /* 0x5E3B */
    AVRCP_ADD_TO_NOW_PLAYING_CFM,               /* 0x5E3C */
    AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND,   /* 0x5E3D */
    AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND,  /* 0x5E3E */
    AVRCP_EVENT_UIDS_CHANGED_IND,               /* 0x5E3F */
    AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND,/* 0x5E40 */
    AVRCP_BROWSE_CONNECT_IND,                   /* 0x5E41 */
    AVRCP_BROWSE_CONNECT_CFM,                   /* 0x5E42 */
    AVRCP_BROWSE_DISCONNECT_IND,                /* 0x5E43 */
    AVRCP_BROWSE_SET_PLAYER_IND,                /* 0x5E44 */
    AVRCP_BROWSE_SET_PLAYER_CFM,                /* 0x5E45 */
    AVRCP_BROWSE_CHANGE_PATH_IND,               /* 0x5E46 */
    AVRCP_BROWSE_CHANGE_PATH_CFM,               /* 0x5E47 */
    AVRCP_BROWSE_GET_FOLDER_ITEMS_IND,          /* 0x5E48 */
    AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM,          /* 0x5E49 */
    AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND,       /* 0x5E4A */
    AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM,       /* 0x5E4B */
    AVRCP_BROWSE_SEARCH_IND,                    /* 0x5E4C */
    AVRCP_BROWSE_SEARCH_CFM,                    /* 0x5E4D */
    AVRCP_BROWSE_TIMEOUT_IND,                   /* 0x5E4E */
    AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM, /* 0x5E4F */
    AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND, /* 0x5E50 */
    
    AVRCP_MESSAGE_TOP
} AvrcpMessageId;
#endif

/* Upstream messages from the AVRCP library */

/*!
    @brief This message is generated as a result of a call to AvrcpInit().
*/
typedef struct
{
    /*! The current AVRCP status. */    
    avrcp_status_code    status;        
    /*! Registered SDP handle */
    uint32              sdp_handle;
} AVRCP_INIT_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpConnectRequest().
*/
typedef struct
{
    AVRCP                *avrcp;  /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /*!< The current AVRCP status. */
    bdaddr               bd_addr; /*! The Bluetooth Device Address */ 
    Sink                 sink;    /*!< Connection handle */    
} AVRCP_CONNECT_CFM_T;

/*!
    @brief  This message indicates that a remote device wishes to connect.

    The application must respond to this message using 
    AvrcpConnectResponse() API.
*/
typedef struct
{
    /*! The Bluetooth Device Address of device connecting */
    bdaddr        bd_addr;            
    /*! Connection identifier */    
    uint16        connection_id;    
    /*! Signalling identifier */
    uint16        signal_id;
} AVRCP_CONNECT_IND_T;


/*!
    @brief This message indicates that the link has disconnected.
*/
typedef struct
{
    AVRCP                *avrcp;  /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /*!< The current AVRCP status. */
    Sink                 sink;    /*!< Connection handle */    
} AVRCP_DISCONNECT_IND_T;


/*!
    @brief This message is generated as a result of a call to 
    AvrcpBrowseConnectRequest().
*/
typedef struct
{
    AVRCP                *avrcp;  /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /*!< The current AVRCP status. */
    uint16               max_browse_data;/*!< Maximum Browse Data allowed */
} AVRCP_BROWSE_CONNECT_CFM_T;

/*!
    @brief This message indicates that a remote device wishes to connect the 
    Browsing channel. 

    The application must respond to this message using 
    AvrcpBrowseConnectResponse() API.
   
*/
typedef struct
{
    /*! The Bluetooth Device Address of device connecting */
    bdaddr        bd_addr;            
    /*! Connection identifier */    
    uint16        connection_id;    
    /*! Signalling identifier */
    uint16        signal_id;
} AVRCP_BROWSE_CONNECT_IND_T;

/*!
    @brief This message indicates that the browsing channel has been 
    disconnected.
*/
typedef struct
{
    AVRCP                *avrcp;  /* Pointer to AVRCP profile instance. */
    avrcp_status_code    status;  /* The current AVRCP status. */
} AVRCP_BROWSE_DISCONNECT_IND_T;

/*!
    @brief  This message is generated as a result of a call to 
    AvrcpGetSupportedFeatures().

    This message is confirmation of getting the supported features 
    of the remote device.
*/
typedef struct
{
    AVRCP                *avrcp;  /* Pointer to AVRCP profile instance. */
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! The features supported by the remote device. */
    uint16                features;
} AVRCP_GET_SUPPORTED_FEATURES_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpGetProfileExtensions().

    This message is confirmation of getting the supported extensions 
    of the remote device.
*/
typedef struct
{
    AVRCP                *avrcp;  /* Pointer to AVRCP profile instance. */
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! The extensions supported on the remote device. */
    uint16                extensions;
} AVRCP_GET_EXTENSIONS_CFM_T;

#ifndef AVRCP_TG_ONLY_LIB  /* CT APIs are not exposed on TG only lib */ 
/*!
    @brief This message is generated as a result of a call to
    AvrcpPassthroughRequest().
*/
typedef struct
{
    AVRCP                *avrcp;   /*!< Pointer to AVRCP profile instance. */
    avrcp_status_code    status;   /*!< The current AVRCP status. */
    Sink                sink;      /*!< Connection handle */    
} AVRCP_PASSTHROUGH_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpUnitInfoRequest().
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                
    /*! The current AVRCP status. */
    avrcp_status_code    status;                
    /*! The sink. */
    Sink                sink;                
    /*! The unit type. */
    avc_subunit_type    unit_type;            
    /*! The unit. */
    uint16                unit;                
    /*! The company identifier. */    
    uint32                company_id;            
} AVRCP_UNITINFO_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpSubUnitInfoRequest().
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                            
    /*! The current AVRCP status. */
    avrcp_status_code    status;                            
    /*! Connection handle */
    Sink                sink;                            
    /*! Requested page on the target device. */
    uint8                page;                            
    /*! Four entries from the subunit table for the requested page on the
      target device.*/    
    uint8                page_data[PAGE_DATA_LENGTH];
} AVRCP_SUBUNITINFO_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpVendorDependentRequest().
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;       
    /*! The current AVRCP status. */
    avrcp_status_code    status;    
    /*! Connection handle */
    Sink                sink;  

#ifdef AVRCP_ENABLE_DEPRECATED    
    /*! Response from remote end if applicable. "status" field 
        gives the respective status for the received response. It is not 
        recommended to use this field since this has be deprecated and 
        will be removed in future.
     */     
    uint8               response; 
#endif  
} AVRCP_VENDORDEPENDENT_CFM_T;

/*!
    @brief This is generated as a result of a call to 
    AvrcpGetCapsRequest().

    This message indicates the outcome of the request to get the
    supported capabilities of the remote device. 
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP               *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code   status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The type of the capability returned (only valid if status indicates 
        success). Only valid for first packet in fragmented response.*/
    avrcp_capability_id caps;
    /*! The total number of capabilities returned. Only valid for first 
       Metadata packet in a fragmented response. */
    uint16              number_of_caps;
    /*! The size, in bytes, of the list of supported capabilities in 
        this packet (only valid if status indicates success). */
    uint16              size_caps_list; 
    /*! The list of supported capabilities (only valid if status indicates 
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source              caps_list;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_CAPS_CFM_T; 

/*!
    @brief  This message is generated as a result of a call
    to AvrcpListAppAttributeRequest().

    This message contains the player application settings attributes
    of the  remote device. 
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first packet 
        in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success). The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_LIST_APP_ATTRIBUTE_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpListAppValueRequest().

    This message contains the entire player application settings
    values supported by the  remote device, for the attribute requested.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16 metadata_packet_type;
    /*! The total number of values returned. Only valid for first packet 
        in fragmented response. */
    uint16 number_of_values;
    /*! The size, in bytes, of the list of supported values */
    uint16 size_values;
    /*! The list of supported values (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it has
        finished processing the Source data. */
    Source values;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_LIST_APP_VALUE_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpGetAppValueRequest().

    This message contains the current player application settings 
    values of the remote device for each attribute requested. 
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16 metadata_packet_type;
    /*! The total number of values returned. Only valid for first packet 
        in fragmented response. */
    uint16 number_of_values;
    /*! The size, in bytes, of the list of supported values */
    uint16 size_values;
    /*! The list of supported values (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source values;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_APP_VALUE_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpSetAppValueRequest(). 

    This message is confirmation of setting the player application 
    settings values of the  remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! The transaction. */
    uint16 transaction;
#endif
} AVRCP_SET_APP_VALUE_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpGetAppAttributeTextRequest().

    This message is confirmation of retrieving the attribute text of 
    the  remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first 
        packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16               data_offset;
#endif

} AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T;

/*!
    @brief This message is generated as a result of a call to
    AvrcpGetAppValueTextRequest().

    This message is confirmation of retrieving the value text of the 
    remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /* If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /* The total number of attributes returned. Only valid for first packet 
       in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif
} AVRCP_GET_APP_VALUE_TEXT_CFM_T;


/*!
    @brief This message is generated as a result of a call to
    AvrcpGetElementAttributesRequest().

    This message contains the attributes of the element requested.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code    status;
    /*! If the Metadata packet is of type single, start, continue, or end. */
    uint16              metadata_packet_type;
    /*! The total number of attributes returned. Only valid for first packet
        in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
#endif

} AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpGetPlayStatusRequest().

    This message contains the current play status of the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The total length of the playing song in milliseconds. */
    uint32 song_length;
    /*! The current position of the playing song in milliseconds elapsed. */
    uint32 song_elapsed;
    /*! Current status of playing media. */
    avrcp_play_status play_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
#endif

} AVRCP_GET_PLAY_STATUS_CFM_T;


/*!
    @brief This message is generated for the failure confirmation of
    a call to AvrcpRegisterNotificationRequest().

    This message will be generated if the library is not able to send 
    Register Notification command to the Target device or the
    Target device is not responding for the  Registered event on time.
    On success, application will not receive this event and it will 
    receive the corresponding event indication message.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The event that the device asked to be notified of changes. */
    avrcp_supported_events event_id;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REGISTER_NOTIFICATION_CFM_T;

/*!
    @brief This message indicates the 
    avrcp_event_playback_status_changed event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current play status on the remote device. */
    avrcp_play_status play_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the avrcp_event_track_changed
    event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! Index of the current track - upper 4 bytes */
    uint32 track_index_high;
    /*! Index of the current track - lower 4 bytes */
    uint32 track_index_low;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_CHANGED_IND_T;


/*!
    @brief This message indicates the avrcp_event_track_reached_end
    event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;

    /*! The response from the remote device. */
    avrcp_response_type response;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_REACHED_END_IND_T;


/*!
    @brief This message indicates the avrcp_event_track_reached_start
    event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;


#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_TRACK_REACHED_START_IND_T;


/*!
    @brief This message indicates the avrcp_event_playback_pos_changed
    event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current playback position in milliseconds. */
    uint32 playback_pos;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND_T;


/*!
    @brief This message indicates the avrcp_event_batt_status_changed
    event from the remote device.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current battery status. */
    avrcp_battery_status battery_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated.*/
    uint16 transaction;
#endif
} AVRCP_EVENT_BATT_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the avrcp_event_system_status_changed
    event from the remote device.

*/
typedef struct
{
    AVRCP *avrcp;

    /*! The response from the remote device. */
    avrcp_response_type response;
    /*! The current system status. */
    avrcp_system_status system_status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND_T;


/*!
    @brief This message indicates the avrcp_event_player_app_setting_changed
    event from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The response. */
    avrcp_response_type response;
    /* The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet (only
        valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it 
        has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif
} AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND_T;

/*!
    @brief This message indicates the  avrcp_event_volume_changed
    event from the remote device.
*/
typedef struct{
    AVRCP               *avrcp;     /* Profile instance */        
    avrcp_response_type response;  /* The response from the remote device. */
    uint8               volume;    /* Volume at TG */

}AVRCP_EVENT_VOLUME_CHANGED_IND_T;

/*!
    @brief This message indicates the avrcp_event_addressed_player_changed
    event from the remote device.

*/

typedef struct{
    AVRCP               *avrcp;     /* Profile Instance */
    avrcp_response_type response;   /* The response from the remote device. */
    uint16              player_id;  /* Requested player ID to set */ 
    uint16              uid_counter;/* UID Counter */ 
}AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND_T;

/*!
    @brief This message indicates the avrcp_event_available_players_changed
    event from the remote device.
*/

typedef struct{
    AVRCP               *avrcp;     /* Profile Instance */
    avrcp_response_type response;   /* The response from the remote device. */
}AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND_T;

/*!
    @brief This message indicates the avrcp_event_uids_changed
    event from the remote device.
*/

typedef struct{
    AVRCP               *avrcp;      /* Profile Instance */
    avrcp_response_type response;    /* The response from the remote device.*/
    uint16               uid_counter;/*UID counter of current browsed player*/
}AVRCP_EVENT_UIDS_CHANGED_IND_T;

/*!
    @brief  This message indicates the avrcp_event_now_playing_content_changed
    event from the remote device.
*/

typedef struct{
    AVRCP               *avrcp;    /* Profile Instance */
    avrcp_response_type response;  /* The response from the remote device. */
}AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND_T;

/*!
    @brief This message is generated for the failure confirmation of
    a call to AvrcpRequestContinuingResponseRequest().
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;
    /*! The target PDU. */
    uint16 pdu_id;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REQUEST_CONTINUING_RESPONSE_CFM_T;


/*!
    @brief This message is generated as a result of a call to 
    AvrcpAbortContinuingResponseRequest(). 
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated. */
    uint16 transaction;
#endif

} AVRCP_ABORT_CONTINUING_RESPONSE_CFM_T;


/*!
    @brief This message is generated as a result of a call to
    AvrcpPreviousGroupRequest().
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;


#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_PREVIOUS_GROUP_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpNextGroupRequest().
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
     /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_NEXT_GROUP_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpInformBatteryStatusRequest().
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_BATTERY_STATUS_CFM_T;


/*!
    @brief  This message is generated as a result of a call to
    AvrcpInformCharacterSetRequest().
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

    /*! The outcome of the request. */
    avrcp_status_code status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_CHARACTER_SET_CFM_T;

/*!
    @brief  This message is generated as a result of a call to
    AvrcpSetAbsoluteVolumeRequest().

*/
typedef struct{
    /*! Profile Instance */
    AVRCP             *avrcp; 
    /*! Outcome of the request,returns avrcp_success on success */
    avrcp_status_code status; 
   /*! Volume at TG on success */ 
    uint8             volume; 
}AVRCP_SET_ABSOLUTE_VOLUME_CFM_T;

/*!
    @brief This message is generated as a result of a call to 
    AvrcpSetAddressedPlayerRequest().
*/

typedef struct{
    AVRCP             *avrcp;    /* Profile Instance */
    avrcp_status_code status;    /* Outcome of the request, 
                                    returns avrcp_success on success */
}AVRCP_SET_ADDRESSED_PLAYER_CFM_T;

/*!
    @brief This message is generated as a result of a call to 
    AvrcpPlayItemRequest().
*/

typedef struct{
    AVRCP               *avrcp;   /* Profile Instance */
    avrcp_status_code status;    /* Outcome of the request, 
                                    returns avrcp_success on success */
}AVRCP_PLAY_ITEM_CFM_T;

/*!
    @brief This message is generated as a result of a call to 
    AvrcpAddToNowPlayingRequest().
*/

typedef struct{
    AVRCP               *avrcp;   /* Profile Instance */
    avrcp_status_code status;    /* Outcome of the request, 
                                    returns avrcp_success on success */
}AVRCP_ADD_TO_NOW_PLAYING_CFM_T;

/*!
    @brief This message is generated as a result of a call to 
    AvrcpBrowseSetPlayerRequest().

    On success it returns the 
    browsed players current folder in the folder_path field of this 
    message. It contains the path of the folder  in the format like, 
    pairs of length of folder name and folder name repeated together 
    "folder_depth" times,   each pair representing one folder level.
    e.g: folder path A/BC/DEF is represented in UTF8 as 0x0001, 'A',
    0x0002, "BC",0x0003, "DEF". 
*/
typedef struct{
    AVRCP            *avrcp;       /* Profile Instance */
    avrcp_status_code status;      /* Outcome of the request, 
                                      returns avrcp_success on success */
    uint16            uid_counter; /* UID Counter of the Browsed player */
    uint32            num_items;   /* Number of items in the current folder */
    avrcp_char_set    char_type;   /* Character Set used for Folder names */
    uint8             folder_depth;/* depth of folders */ 
    uint16            size_path;   /* size of folder_path in bytes */
    Source            folder_path; /* This contains the path of the folder 
                                      in length-name pairs. Number of pairs 
                                      equal to folder_depth*/
}AVRCP_BROWSE_SET_PLAYER_CFM_T;

/*!
    @brief This message is generate as a result of a call to 
    AvrcpBrowseChangePathRequest().
    
    On success it returns the number 
    of items in the current browsing folder of the Target after changing the 
    path.
*/
typedef struct{
    AVRCP                    *avrcp;    /* Profile Instance */
    avrcp_status_code        status;    /* Returns avrcp_success on success*/
    uint32                   num_items; /* Total items in the changed folder*/ 
}AVRCP_BROWSE_CHANGE_PATH_CFM_T;

/*!
    @brief This message is generate as a result of a call to 
    AvrcpBrowseGetItemAttributesRequest().

    On success it returns 
    the number of requested attributes and the attribute Value list. 
    attr_value_list contains list of Attribute Value entries. Each attribute
    value entry contains a 4 octet attribute Id, 2 octets character set value,
    2 octets attribute value length followed by an attribute value string. 
*/

typedef struct{
    AVRCP               *avrcp;         /* Profile Instance */
    avrcp_status_code   status;         /* Returns avrcp_success on success*/
    uint8               num_attributes; /* Number of Attributes */
    uint16              size_attr_list; /* Size of attr_value_list in bytes */
    Source              attr_value_list;/* List of attribute Entry values */  
}AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T;

/*!
    @brief This message is generate as a result of a call to 
    AvrcpBrowseGetFolderItemsRequest().

    On success it returns 
    the number of items in the folder and the item list. Item list are 
    defined in section 6.10.2.1, 6.10.2.2 and 6.10.2.3 of Avrcp 1.4 
    specification for Media player, folder item and media element item
    respectively. Media element item will contain an attribute value list
    for the requested attributes in the request.   
*/
typedef struct{
    AVRCP               *avrcp;         /* Profile Instance */
    avrcp_status_code   status;         /* Returns avrcp_success on success*/
    uint16              uid_counter;    /* current  UID counter */
    uint16              num_items;      /* Number of items returned */
    uint16              item_list_size; /* Number of bytes in the item_list */
    Source              item_list;      /* Item list */
}AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T;


/*!
    @brief This message is generate as a result of a call to 
    AvrcpBrowseGetNumberOfItemsRequest().

    On success it returns 
    the number of items in the folder and the item list. Item list are 
    defined in section 6.10.2.1, 6.10.2.2 and 6.10.2.3 of Avrcp 1.4 
    specification for Media player, folder item and media element item
    respectively. Media element item will contain an attribute value list
    for the requested attributes in the request.   
*/
typedef struct{
    AVRCP               *avrcp;         /* Profile Instance */
    avrcp_status_code   status;         /* Returns avrcp_success on success*/
    uint16              uid_counter;    /* current  UID counter */
    uint32              num_items;      /* Number of items returned */
}AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM_T;

/*!
    @brief This message is generate as a result of a call to 
    AvrcpBrowseSearchRequest().

    On success it returns the new UID 
    counter for the search scope and the number of media element items 
    matching the search request. 
*/

typedef struct{
    AVRCP               *avrcp;       /* Profile Instance */
    avrcp_status_code   status;       /* Returns avrcp_success on success*/
    uint16               uid_counter; /* The UID counter*/
    uint32               num_items;   /* Number of matching media elements */
}AVRCP_BROWSE_SEARCH_CFM_T;

#endif /* !AVRCP_TG_ONLY_LIB */


#ifndef AVRCP_CT_ONLY_LIB /* TG APIs are not exposed on CT only Lib */

/*!
    @brief This message indicates that the CT device has send a Passthrough
    command
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;                        

#ifdef AVRCP_ENABLE_DEPRECATED
   /*! This field has be deprecated */
    uint16                 transaction;                
    /*! This field has been deprecated */
    uint16                 no_packets;  
#endif

    /*! The sink. */
    Sink                sink;                        
    /*! AV/C protocol - Used to form the targets address.*/
    avc_subunit_type     subunit_type;                
    /*! The subunit identifier. */
    avc_subunit_id         subunit_id;                    
    /*! Identifies the button pressed. */
    avc_operation_id     opid;                        
    /*! Indicates the user action of pressing or releasing the button
      identified by opid.  Active low.*/
    bool                 state;                        
    /*! Length of following operation_data */
    uint16                size_op_data;                
    /*! The op_data field is required for the Vendor Unique operation. For
      other operations, op_data length and data fields should be zero. The
      client should not attempt to free this pointer, the memory will be freed
      when the message is destroyed. If the client needs access to this data
      after the message has been destroyed it is the client's responsibility to
      copy it. */
    uint8                op_data[1]; 
} AVRCP_PASSTHROUGH_IND_T;


/*!
    @brief This message indicates that a remote device is requesting unit
    information.
*/
typedef struct
{
    /*! Pointer to AVRCP profile instance. */
    AVRCP                *avrcp;                
    /*! The sink */    
    Sink                 sink;                
} AVRCP_UNITINFO_IND_T;


/*!
    @brief This message indicates that a remote device is requesting subunit
    information.
*/
typedef struct
{
    AVRCP                *avrcp;    /*!< Pointer to avrcp profile instance. */
    Sink                sink;       /*!< Connection handle */
    uint8                page;      /*!< Requested page. */    
} AVRCP_SUBUNITINFO_IND_T;

/*!
    @brief This message indicates that a remote device is requesting vendor
    dependant information.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;                    

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! The transaction. This field will be deprecated */
    uint16                transaction;            
    /*! This field has been deprecated.*/
    uint16                no_packets;        
#endif             

    /*! The subunit type. */
    avc_subunit_type    subunit_type;            
    /*! The subunit identifier. */
    avc_subunit_id        subunit_id;                
    /*! The company identifier. */
    uint32                company_id;                
    /*! The command type. */
    uint8               command_type;           
    /*! The sink. */
    Sink                sink;                    
    /*! The length of op_data. */
    uint16                size_op_data;            
    /*! The operation data. The client should not attempt to free this pointer,
      the memory will be freed when the message is destroyed. If the client
      needs access to this data after the message has been destroyed it is the
      client's responsibility to copy it. */    
    uint8                op_data[1];    
} AVRCP_VENDORDEPENDENT_IND_T;


/*!
    @brief This message indicates that a request for the supported
    capabilities has been received from the remote device.
   
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The type of the capability requested. */
    avrcp_capability_id caps;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated */
    uint16              transaction;
#endif

} AVRCP_GET_CAPS_IND_T;



/*!
    @brief This message indicates that a request for the player application 
    settings attributes has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! The transaction. */
    uint16              transaction;
} AVRCP_LIST_APP_ATTRIBUTE_IND_T;

/*!
    @brief This message indicates that a request for the supported player
    application settings  values, for the specific attribute, has been 
    received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The player application setting attribute ID for which the possible 
        values should be returned. */
    uint16 attribute_id;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* This field will be deprecated */
    uint16 transaction;
#endif
} AVRCP_LIST_APP_VALUE_IND_T;


/*!
    @brief This message indicates that a request for the current player 
    application settings  values has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The total number of attributes for which values are requested.
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it 
        has finished processing the Source data.*/
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif
} AVRCP_GET_APP_VALUE_IND_T;


/*!
    @brief This message indicates that a request to set the player application
   settings  values has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /*! The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after 
        it has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif

} AVRCP_SET_APP_VALUE_IND_T;



/*!
    @brief This message indicates that a request to get attribute text has 
    been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /* The total number of attributes returned. Only valid for first packet
     in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates 
        success).  The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;


#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif
} AVRCP_GET_APP_ATTRIBUTE_TEXT_IND_T;

/*!
    @brief This message indicates that a request to get value text has been
     received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;
    /*! Attribute ID to retrieve values for. */
    uint16                attribute_id;
    /*! The total number of attributes returned. Only valid for first packet
       in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of supported attributes */
    uint16                size_attributes;
    /*! The list of supported attributes (only valid if status indicates 
        success). The application MUST call the AvrcpSourceProcessed API 
        after it has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif

} AVRCP_GET_APP_VALUE_TEXT_IND_T;


/*!
    @brief This message indicates that a request to get element attributes
    has been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /* Top 4 bytes of identifier to identify an element on TG */
    uint32                identifier_high;
    /* Bottom 4 bytes of identifier to identify an element on TG */
    uint32                identifier_low;
    /* The total number of attributes for which values are requested. 
        Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success). 
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source                attributes;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif

} AVRCP_GET_ELEMENT_ATTRIBUTES_IND_T;


/*!
    @brief This message indicates that a request to get the current play 
    status been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /*! This field has been deprecated */
    uint16 transaction;
#endif
} AVRCP_GET_PLAY_STATUS_IND_T;


/*!
    @brief This message indicates the remote device has requested to be 
    notified for changes in state for a certain event.
*/
typedef struct
{
    AVRCP *avrcp;
    /*! Event for which the CT is interested in receiving notifications. */
    avrcp_supported_events event_id;
    /*! Time interval, in seconds, at which the change in playback position 
        should be notified. Only applicable for EVENT_PLAYBACK_POS_CHANGED */
    uint32 playback_interval;

#ifdef AVRCP_ENABLE_DEPRECATED  
    /* This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_REGISTER_NOTIFICATION_IND_T;


/*!
    @brief This message indicates that a request for the previous group has 
    been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16              transaction;
#endif
} AVRCP_PREVIOUS_GROUP_IND_T;


/*!
    @brief This message indicates that a request for the next group has
     been received from the remote device.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP                *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
     /*! This field has been deprecated. */
    uint16              transaction;
#endif
} AVRCP_NEXT_GROUP_IND_T;


/*!
    @brief This message is a request to return the current battery status.
*/
typedef struct
{
    AVRCP *avrcp;
    /* The battery status */
    avrcp_battery_status battery_status;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /*! This field has been deprecated. */
    uint16 transaction;
#endif
} AVRCP_INFORM_BATTERY_STATUS_IND_T;


/*!
    @brief This message tells the Target what characters sets are supported 
    on the Controller.
*/
typedef struct
{
    /*! Pointer to avrcp profile instance. */
    AVRCP *avrcp;
    /* The total number of attributes for which values are requested.
       Only valid for first packet in fragmented response. */
    uint16              number_of_attributes;
    /*! The size, in bytes, of the list of attributes in this packet 
        (only valid if status indicates success). */
    uint16              size_attributes; 
    /*! The list of attributes (only valid if status indicates success).
        The application MUST call the AvrcpSourceProcessed API after it
        has finished processing the Source data. */
    Source              attributes;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Following fields have been deprecated and it is not recommended to use.
       These field will be removed in future versions*/
    uint16              transaction;
    uint16              no_packets;
    uint16              ctp_packet_type;
    uint16              data_offset;
    uint16              metadata_packet_type;
#endif

} AVRCP_INFORM_CHARACTER_SET_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving SetAbsoluteVolume request from the Controller device.
*/
typedef struct{
    /*! Profile Instance */
    AVRCP             *avrcp; 
    /*! Requested %Volume. 0 as 0% and 0x7F as 100%. Scaling should
       be applied to achieve other between these two*/ 
    uint8             volume; 
                                
                                
}AVRCP_SET_ABSOLUTE_VOLUME_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving SetAddressedPlayer request from the Controller device.
*/
typedef struct{
    AVRCP             *avrcp;       /* Profile Instance */
    uint16             player_id;   /* Requested player ID to set */ 
}AVRCP_SET_ADDRESSED_PLAYER_IND_T;

/*!
    @brief This message is an indication to the Target on 
    receiving PlayItem request from the Controller device.
*/

typedef struct{
    AVRCP                *avrcp;     /* Profile Instance */
    avrcp_browse_scope   scope;      /* Browse Scope in which UID is valid */
    avrcp_browse_uid     uid;        /* media element item or folder item UID*/
    uint16               uid_counter;/* The UID counter value associated 
                                         with the uid and scope */
}AVRCP_PLAY_ITEM_IND_T;

/*!
    @brief This message is an indication to the Target on 
    receiving AddToNowPlaying request from the Controller device.
*/
typedef struct{
    AVRCP                *avrcp;     /* Profile Instance */
    avrcp_browse_scope   scope;      /* Browse Scope in which UID is valid */
    avrcp_browse_uid     uid;        /* media element item or folder item UID*/
    uint16               uid_counter;/* The UID counter value associated 
                                         with the uid and scope */
}AVRCP_ADD_TO_NOW_PLAYING_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving SetBrowsedPlayer request from the Controller device.
*/

typedef struct{
    AVRCP             *avrcp;      /* Profile Instance */
    uint16            player_id;   /* Requested player ID to set */ 
}AVRCP_BROWSE_SET_PLAYER_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving ChangePath request from the Controller device.
*/
typedef struct{
    AVRCP                    *avrcp;       /* Profile Instance */
    uint16                   uid_counter;  /* The UID counter value */
    avrcp_browse_direction   direction;    /* Direction of browsing */
    avrcp_browse_uid         folder_uid;   /* UID of the folder to navigate*/ 
}AVRCP_BROWSE_CHANGE_PATH_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving GetItemAttributes request from the Controller device. 

    The uid=0x0 is a special value to request the Metadata of the currently
    playing Media. attr_list field will be set to 0 if num_attr=0 or 255.
    For any other value of num_attr, size of the attributes in the attr_list
    will be 4*num_attr. num_attr=0 is a special value to request for all 
    attributes and num_attr=255 is a request for no attributes. All attribute
    IDS in the list are defined in the interface header file under "List of 
    Media Attribute definitions".
*/

typedef struct{
    AVRCP               *avrcp;      /* Profile Instance */
    avrcp_browse_scope  scope;       /* Scope in which UID is valid */
    avrcp_browse_uid    uid;         /* UID of the media element or folder.*/
    uint16              uid_counter; /* The UID counter value */
    uint8               num_attr;    /* number of requested attributes */ 
    Source              attr_list;    /* attribute List */
}AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND_T;



/*!
    @brief This message is an indication to the Target on 
    receiving GetFolderItems request from the Controller device. 

    The attr_list field will be set to 0 if num_attr=0 or 255.
    For any other value of num_attr, size of the attributes in the attr_list
    will be 4*num_attr. num_attr=0 is a special value to request for all 
    attributes and num_attr=255 is a request for no attributes. All attribute
    IDS in the list are defined in the interface header file under "List of 
    Media Attribute definitions".
*/

typedef struct{
    AVRCP                *avrcp;     /* Profile Instance */
    avrcp_browse_scope   scope;      /* Scope in which UID is valid */
    uint32               start;      /* Start item Index. 0 for first item */
    uint32               end;        /* index of the final item to retrieve.*/
    uint8                num_attr;   /* requested attributes count */
    Source               attr_list;  /* List of Metadata attributes. */
}AVRCP_BROWSE_GET_FOLDER_ITEMS_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving GetNumberOfItems request from the Controller device. 
*/

typedef struct{
    AVRCP                *avrcp;     /* Profile Instance */
    avrcp_browse_scope   scope;      /* Scope in which UID is valid */
}AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND_T;


/*!
    @brief This message is an indication to the Target on 
    receiving Search request from the Controller device.
    
    Target must respond with AvrcpBrowseSearchResponse() on receiving this
    message indication.
    
    The library will reject an incoming search request and will not generate
    this event if the library was initialised without search support.
*/
typedef struct{
    AVRCP            *avrcp;       /* Profile Instance */
    avrcp_char_set   char_set;     /* Must be avrcp_char_set_utf_8*/
    uint16           str_length;   /* Length of Search string in bytes */
   Source            string;       /* Source containing the search String */
}AVRCP_BROWSE_SEARCH_IND_T;

#endif /* !AVRCP_CT_ONLY_LIB */

/*!
    @brief Initialise the AVRCP library.

    @param theAppTask The current application task.    
    @param config Configuration parameters for initialising the AVRCP library
    instance. This must not be set to null.

    This function also takes care of registering a service record for an AVRCP
    device.  No further library functions should be called until the
    @link AVRCP_INIT_CFM_T AVRCP_INIT_CFM@endlink message has been
    received by the client.
*/
void AvrcpInit(Task theAppTask, const avrcp_init_params *config);

/*!
    @brief Initiate an AVRCP connection to a remote device.

    @param theAppTask The application task handling this connection.
    @param bd_addr The Bluetooth address of the remote device.

    Application must call AvrcpGetProfileExtensions() and/or 
    AvrcpGetSupportedFeatures() after the connection establishment 
    to find out the remote Target device features
    before calling any APIs for the optional features.  

    @return TRUE if AVRCP connection is initiated else FALSE.

    @return Message @link AVRCP_CONNECT_CFM_T AVRCP_CONNECT_CFM @endlink 
    is sent to the application. 
   
*/
bool AvrcpConnectRequest(Task theAppTask, const bdaddr *bd_addr);


/*!
    @brief Either accept or reject the incoming connection from the 
    remote device.

    @param theAppTask   The application Task to handle the connection
    @param connection_id Connection identifier
    @param signal_id    Signalling identifier
    @param accept Yes(TRUE) or No(FALSE)
    @param bd_addr   Remote Device Bluetooth Address.

    This function is called on receipt of a message 
    @link AVRCP_CONNECT_IND_T AVRCP_CONNECT_IND @endlink.
*/
void AvrcpConnectResponse(  Task theAppTask, 
                            uint16 connection_id, 
                            uint16 signal_id, 
                            bool accept,
                            const bdaddr *bd_addr);

/*!
    @brief Initiate a browsing channel connection to a remote device. 

    @param avrcp The profile instance which will be used.
    @param bd_addr The Bluetooth address of the remote device.

    @return Message @link AVRCP_BROWSE_CONNECT_CFM_T AVRCP_BROWSE_CONNECT_CFM
    @endlink  is sent to the application. 
   
*/
void AvrcpBrowseConnectRequest(AVRCP *avrcp, const bdaddr *bd_addr);

/*!
    @brief Either accept or reject the incoming connection for a 
    browsing channel from the remote device.

    @param avrcp The profile instance which will be used.
    @param connection_id Connection identifier
    @param signal_id    Signalling identifier
    @param accept Yes(TRUE) or No(FALSE)

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_CONNECT_IND_T AVRCP_BROWSE_CONNECT_IND @endlink.
*/
void AvrcpBrowseConnectResponse(AVRCP   *avrcp, 
                                uint16  connection_id, 
                                uint16  signal_id,
                                bool    accept);


/*!
    @brief Request an AVRCP browsing Channel disconnection.  
    @param avrcp The profile instance which will be used.

    @return Message @link AVRCP_BROWSE_DISCONNECT_IND_T 
    AVRCP_BROWSE_DISCONNECT_IND @endlink  is sent to the application. 
*/
void AvrcpBrowseDisconnectRequest(AVRCP *avrcp);


/*!
    @brief Request an AVRCP disconnection.  
    @param avrcp The profile instance which will be used.

    @return Message @link AVRCP_DISCONNECT_IND_T 
    AVRCP_DISCONNECT_IND @endlink  is sent to the application. 
*/
void AvrcpDisconnectRequest(AVRCP *avrcp);

 /*!
    @brief This function is used by the Controller to retrieve the 
    supported features of the Target.

    @param avrcp The profile instance which will be used.

    This will include  which category commands are supported, 
    and if player application settings or group navigation is supported for
    Metadata extensions.

    @return Message @link AVRCP_GET_SUPPORTED_FEATURES_CFM_T
    AVRCP_GET_SUPPORTED_FEATURES_CFM  @endlink 
    is sent to the application. 
*/
void AvrcpGetSupportedFeatures(AVRCP *avrcp);

/*!
    @brief This function is used to retrieve if any profile
    extensions are available on the peer device(Typically remote Target).

    @param avrcp The profile instance which will be used.

    This will return profile extension flag value if the remote device 
    supports  AVRCP Metadata extensions or Browsing channel support. 
    Search and Database aware player support are depending on the 
    Media player at the remote end and this can be identified from the 
    player Feature bit mask  retrieved using AvrcpGetFolderItemsRequest()
    for any particular media player item. Profile extension value returned
    by this API does not set AVRCP_DATABASE_AWARE_PLAYER_SUPPORTED
    and AVRCP_SEARCH_SUPPORTED flags.

    @return Message @link
    AVRCP_GET_EXTENSIONS_CFM_T  AVRCP_GET_EXTENSIONS_CFM 
    @endlink is sent to the application. 
   
*/
void AvrcpGetProfileExtensions(AVRCP *avrcp);

/*!
    @brief Free the buffer resources after finished processing the
    Source data contained in an upstream message.

    @param avrcp The profile instance which will be used.

    Some of the Metadata messages sent up to the application contain 
    Sources which hold the message data (e.g. caps_list of AVRCP_GET_CAPS_CFM 
    message). When  the application has finished with the Source data, this
    API MUST be called, otherwise the library won't process any more data
    arriving until the application calls any other request or response API.
    If the application is not using this API, the Library flushes the Source
    before sending the next packet to the peer, until then the library will not
    able to receive any messages from the peer. ( A Controller application
    calling a request API triggers the library to flush the Source data 
    associated with the previous transaction. A Target application 
    responding to a request triggers the library to flush the Source data 
    associated with the request indication message.)
   
*/
void AvrcpSourceProcessed(AVRCP *avrcp);

/*!
    @brief Retrieve the Sink of the AVRCP connection. This will be 0 if no
    connection exists. 

    @param avrcp The profile instance which will be used.
*/
Sink AvrcpGetSink(AVRCP *avrcp);

#ifndef AVRCP_TG_ONLY_LIB  /* CT APIs are not exposed on TG only lib */  

/*!
    @brief Request that a Pass Through control command is sent to the 
    target on the connection identified by the profile instance.   
    @param avrcp The profile instance which will be used.

    @param subunit_type The subunit type. This shall be PANEL (0x09) for AVRCP

    @param subunit_id AV/C protocol. Used to form the targets address.

    @param state Indicates the user action of pressing or releasing the button
    identified by opid.  Active low.

    @param opid Identifies the button pressed.

    @param size_operation_data Size of operation_data. Should be zero if no
    operation_data passed.

    @param operation_data Required for the Vendor Unique operation. The app
    should use StreamRegionSource as defined in stream.h, and pass the source
    returned as this parameter.

    The Passthrough command is used to convey the proper user operation to 
    the target(transparently to the user).
    
    The Application  shall pay  attention on the state flag. On each button
    press and  release , it shall call this function with state flag=0 
    for press and state flag=1 for  release. The Library never tracks 
    the state of the button whether it is  pressed down or released. 
    Application shall ensure there is a button press  and a release and
    maintain  the time requirements between each.

    @return Message @link
    AVRCP_PASSTHROUGH_CFM_T AVRCP_PASSTHROUGH_CFM
    @endlink is sent to the application. 
    
*/
/*
                     -----------------------------------------------
                    | MSB |      |     |     |     |     |    | LSB |
                    |-----------------------------------------------|
    opcode          |           PASSTHROUGH (0x7C)                  |
                     -----------------------------------------------
    operand(0)      |state|         operation_id                    |
                      -----------------------------------------------
    operand(1)      |       operation data field length             |
                     -----------------------------------------------
    operand(2)      |    operation data(operation id dependant)     |
        :           |                                               |
                     -----------------------------------------------

*/
void AvrcpPassthroughRequest(AVRCP             *avrcp, 
                      avc_subunit_type  subunit_type,
                      avc_subunit_id    subunit_id,
                      bool              state, 
                      avc_operation_id  opid, 
                      uint16            size_operation_data, 
                      Source            operation_data);


/*!
    @brief Cancel all queued Passthrough requests.

    @param avrcp The profile instance which will be used.
*/
void AvrcpPassthroughRequestCancelAll(AVRCP *avrcp);


/*!
    @brief Request that a UnitInfo control command is sent to the target on the
    connection identified by the specified sink.
    
    @param avrcp The profile instance which will be used.

    The UnitInfo command is used to obtain information that pertains to the
    AV/C unit as a whole
 
    @return Message @link
    AVRCP_UNITINFO_CFM_T AVRCP_UNITINFO_CFM
    @endlink is sent to the application.    
*/
/*        
                     -----------------------------------------------
                    | MSB |     |    |      |     |      |    | LSB |
                    |-----------------------------------------------|
    opcode          |               UNITINFO (0x30)                 |
                     -----------------------------------------------
    operand[0]      |                      0xFF                     |
                     -----------------------------------------------
    operand[1]      |                      0xFF                     |
                     -----------------------------------------------
    operand[2]      |                      0xFF                     |
                     -----------------------------------------------
    operand[3]      |                      0xFF                     |
                     -----------------------------------------------
    operand[4]      |                      0xFF                     |
                     -----------------------------------------------
*/
void AvrcpUnitInfoRequest(AVRCP *avrcp);


/*!
    @brief Request that a SubUnitInfo control command is sent to the target on
    the connection identified by the specified sink.
    
    @param avrcp The profile instance which will be used.

    @param page Specifies which part of the subunit table is to be returned.
    Each page consists of at most four subunits, and each AV/C unit contains up
    to 32 AV/C subunits

    The UnitInfo command is used to obtain information about the subunit(s) of
    a device. The extension code is not used at present, should always be 0x7.

    @return Message @link
    AVRCP_SUBUNITINFO_CFM_T AVRCP_SUBUNITINFO_CFM
    @endlink is sent to the application.    
*/
/*
                     -----------------------------------------------
                    | MSB |     |    |      |     |      |     |LSB |
                    |-----------------------------------------------|
    opcode          |              SUBUNITINFO (0x31)               |
                     -----------------------------------------------
    operand[0]      |  0  |       Page      |  0  | Extension code  |
                      -----------------------------------------------
    operand[1]      |                      0xFF                     |
                     -----------------------------------------------
    operand[2]      |                      0xFF                     |
                     -----------------------------------------------
    operand[3]      |                      0xFF                     |
                     -----------------------------------------------
    operand[4]      |                      0xFF                     |
                     -----------------------------------------------

*/
void AvrcpSubUnitInfoRequest(AVRCP *avrcp, uint8 page);


/*!
    @brief Call to send vendor specific data to the peer entity.
    
    If the data length is greater than the l2cap mtu then the message becomes
    fragmented.
    
    @param avrcp The profile instance which will be used.

    @param subunit_type The subunit type.

    @param subunit_id  AV/C protocol. Used to form the targets address.

    @param ctype The ctype.

    @param company_id 24-bit unique ID obtained from IEEE RAC.

    @param size_data Length of the data field.

    @param data Required for the Vendor Unique operation. The app should use
    StreamRegionSource as defined in stream.h, and pass the source returned as
    this parameter.

    @return Message @link
    AVRCP_VENDORDEPENDENT_CFM_T AVRCP_VENDORDEPENDENT_CFM
    @endlink is sent to the application.       
*/
/*
                     -----------------------------------------------
                    | MSB |      |      |      |    |     |    |LSB |
                    |-----------------------------------------------|
    opcode          |           VENDOR-DEPENDENT (0x00)             |
                     -----------------------------------------------
    operand(0)      |MSB                                            |
    operand(1)      |                 company_id                    |
    operand(2)      |                                        LSB    |
                    |-----------------------------------------------| 
    operand(3)      |                                               |
        :           |              vendor_dependent_data            |
    operand(n)      |                                               | 
                     -----------------------------------------------
*/
void AvrcpVendorDependentRequest(  AVRCP            *avrcp, 
                            avc_subunit_type subunit_type, 
                            avc_subunit_id   subunit_id, 
                            uint8            ctype, 
                            uint32           company_id, 
                            uint16           size_data, 
                            Source           data);


/*!
    @brief Request the capabilities supported by the remote device (target).

    @param avrcp The profile instance which will be used.

    @param caps The type of the requested capabilities (currently only 
    company ID and supported events defined by the specification).

    This function is called to request the capabilities supported by the 
    remote (target) device. 
    
    @return Message @link
    AVRCP_GET_CAPS_CFM_T AVRCP_GET_CAPS_CFM
    @endlink is sent to the application.  

*/
void AvrcpGetCapsRequest(AVRCP *avrcp, avrcp_capability_id caps);


/*!
    @brief Request the player application settings attributes supported by 
    the Target device.

    @param avrcp The profile instance which will be used.

    This function is called from the controller (CT) to obtain the target
    supported player application setting attributes.

    @return Message @link
    AVRCP_LIST_APP_ATTRIBUTE_CFM_T AVRCP_LIST_APP_ATTRIBUTE_CFM
    @endlink is sent to the application. 

*/
void AvrcpListAppAttributeRequest(AVRCP *avrcp);


/*!
    @brief Request the player application settings values supported by the Target
    device.

    @param avrcp The profile instance which will be used.

    @param attribute_id The application setting attribute to retrieve the 
    possible values of from the TG.

    This function is called from the controller (CT) to obtain the target
    supported player application setting values.

    @return Message @link
    AVRCP_LIST_APP_VALUE_CFM_T AVRCP_LIST_APP_VALUE_CFM
    @endlink is sent to the application. 

*/
void AvrcpListAppValueRequest(AVRCP *avrcp, uint16 attribute_id);


/*!
    @brief Request for the current set values for the provided player 
    application  setting attributes list.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute IDs for which the corresponding
    current set value is requested.

    This function is sent to the TG to request it to provide the current set
    values on the target for the provided  player application setting
    attributes list. 

    @return Message @link
    AVRCP_GET_APP_VALUE_CFM_T AVRCP_GET_APP_VALUE_CFM
    @endlink is sent to the application. 

*/
void AvrcpGetAppValueRequest( AVRCP *avrcp, 
                               uint16 size_attributes, 
                               Source attributes);

/*!
    @brief Request to set change the value on the TG, for the provided 
    player application attribute.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute/value pairs for which the value
    should be set.

    @return Message @link
    AVRCP_SET_APP_VALUE_CFM_T AVRCP_SET_APP_VALUE_CFM
    @endlink is sent to the application. 
*/
void AvrcpSetAppValueRequest( AVRCP *avrcp, 
                              uint16 size_attributes, 
                              Source attributes );

/*!
    @brief This function is used to request the Target device to return
    displayable text for the requested PlayerAppSettingAttributeIDs.

   
    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attributes (in bytes).

    @param attributes The list of attributes for which the text should be
    retrieved.

    @return Message @link
    AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM_T  AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM
    @endlink is sent to the application. 
*/
void AvrcpGetAppAttributeTextRequest( AVRCP *avrcp, 
                                      uint16 size_attributes, 
                                      Source attributes );

/*!
    @brief This function is used to request the Target device to return 
    displayable text for the requested Player Application value.
    

    @param avrcp The profile instance which will be used.

    @param attribute_id Attribute ID for which the value text should
    be retrieved.

    @param size_values The length of the supplied values (in bytes).

    @param values The list of values for which the text should be retrieved.

    @return Message @link
    AVRCP_GET_APP_VALUE_TEXT_CFM_T AVRCP_GET_APP_VALUE_TEXT_CFM
    @endlink is sent to the application. 
*/
void AvrcpGetAppValueTextRequest(AVRCP *avrcp,
                                 uint16 attribute_id, 
                                 uint16 size_values, 
                                 Source values);
/*!
    @brief Request the attributes of the element specified in remote 
    device (target).

    @param avrcp The profile instance which will be used.

    @param identifier_high Top 4 bytes of identifier of element on TG.

    @param identifier_low Bottom 4 bytes of identifier of element on TG.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The list of attribute IDs of the attributes to be
     retrieved.

    This function is called to request the attributes of the element 
    specified in  the parameter.

    @return Message @link
    AVRCP_GET_ELEMENT_ATTRIBUTES_CFM_T AVRCP_GET_ELEMENT_ATTRIBUTES_CFM
    @endlink is sent to the application. 
    
*/
void AvrcpGetElementAttributesRequest(AVRCP *avrcp,
                               uint32 identifier_high, 
                               uint32 identifier_low, 
                               uint16 size_attributes, 
                               Source attributes);
/*!
    @brief Used by the CT to request the status of the currently playing
     media at the TG.

    @param avrcp The profile instance which will be used.

    @return Message @link
    AVRCP_GET_PLAY_STATUS_CFM_T AVRCP_GET_PLAY_STATUS_CFM
    @endlink is sent to the application. 

*/
void AvrcpGetPlayStatusRequest(AVRCP *avrcp);


/*!
    @brief Register with the TG to get notifications based on the specific
     events occurring. If the application wants to register for multiple
    events, register next event after getting a response for the previous
    event registration. Response could be a AVRCP_REGISTER_CFM for failure or
    a interim  or a final event indication message for the registered event.

    @param avrcp The profile instance which will be used.

    @param event_id Event to receive notification of.

    @param playback_interval Only valid for EVENT_PLAYBACK_POS_CHANGED event. 
           Time interval which notification will be sent.

    @return Message @link
    AVRCP_REGISTER_NOTIFICATION_CFM_T AVRCP_REGISTER_NOTIFICATION_CFM
    @endlink is sent to 
    indicate the outcome of this request for failure, otherwise it will 
    receive the corresponding event indication message.

*/
void AvrcpRegisterNotificationRequest( AVRCP                   *avrcp, 
                                       avrcp_supported_events  event_id, 
                                       uint32           playback_interval);

/*!
    @brief Used by CT to request for continuing response packets for the 
    sent PDU command, that has not completed. 
    This command will be invoked by CT after receiving a response with
    Packet Type - Start or Continue.

    @param avrcp The profile instance which will be used.

    @param pdu_id PDU ID of the last fragmented response to request for 
    further continuation responses. All meta data PDU IDs are defined above.

    @return Message @link AVRCP_REQUEST_CONTINUING_RESPONSE_CFM_T
    AVRCP_REQUEST_CONTINUING_RESPONSE_CFM @endlink is sent to 
    indicate the outcome of this request for failure, 
    
*/
void AvrcpRequestContinuingResponseRequest(AVRCP *avrcp, uint16 pdu_id);

/*!
    @brief Used by CT to abort continuing response. 
    This command will be invoked by CT after receiving a response with
    Packet Type - Start or Continue.

    @param avrcp The profile instance which will be used.

    @param pdu_id PDU ID of the last fragmented response to abort any 
    further continuation responses. All meta data PDU IDs are defined above. 

    @return Message @link 
    AVRCP_ABORT_CONTINUING_RESPONSE_CFM_T AVRCP_ABORT_CONTINUING_RESPONSE_CFM
    @endlink  is sent to the application. 
   
*/
void AvrcpAbortContinuingResponseRequest(AVRCP *avrcp, uint16 pdu_id);

/*!
    @brief This function is used to move to the first song in the next group.

    @param avrcp The profile instance which will be used.

    @param state Indicates the user action of pressing or releasing the button
    identified by opid.  Active low.

    @return Message @link 
    AVRCP_NEXT_GROUP_CFM_T AVRCP_NEXT_GROUP_CFM
    @endlink  is sent to the application. 
*/
void AvrcpNextGroupRequest(AVRCP *avrcp, bool state);

/*!
    @brief This function is used to move to the first song in the 
    previous group.

    @param avrcp The profile instance which will be used.

    @param state Indicates the user action of pressing or releasing the button
    identified by opid.  Active low.

    @return Message @link 
    AVRCP_PREVIOUS_GROUP_CFM_T AVRCP_PREVIOUS_GROUP_CFM
    @endlink  is sent to the application. 
*/
void AvrcpPreviousGroupRequest(AVRCP *avrcp, bool state);

/*!
    @brief This function is used to inform TG when the CT's battery status 
    has changed.

    @param avrcp The profile instance which will be used.

    @param battery_status The current status of the battery. 

    @return Message @link 
    AVRCP_INFORM_BATTERY_STATUS_CFM_T AVRCP_INFORM_BATTERY_STATUS_CFM
    @endlink  is sent to the application. 
*/
void AvrcpInformBatteryStatusRequest(AVRCP                *avrcp, 
                                    avrcp_battery_status battery_status);


/*!
    @brief This function is used to move to inform TG of the character set
     to use when sending strings.

    @param avrcp The profile instance which will be used.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    @return Message @link 
    AVRCP_INFORM_CHARACTER_SET_CFM_T AVRCP_INFORM_CHARACTER_SET_CFM
    @endlink  is sent to the application. 

*/
void AvrcpInformCharacterSetRequest(AVRCP   *avrcp, 
                                    uint16  size_attributes, 
                                    Source  attributes);

/*!
    @brief This function is used by the CT (Category 2) to set the absolute
    volume at category 2 TG.

    @param avrcp The profile instance.

    @param volume Relative volume specified as max %, 0 as 0% 
                  and 0x7F as 100%. This API sets the maximum value 
                  for this parameter as 0x7F.

    @return Message @link 
    AVRCP_SET_ABSOLUTE_VOLUME_CFM_T AVRCP_SET_ABSOLUTE_VOLUME_CFM
    @endlink  is sent to the application. 
*/
void  AvrcpSetAbsoluteVolumeRequest( AVRCP *avrcp, uint8  volume);



/*!
    @brief This function is used by the CT to inform the TG of which media 
    player the CT wishes to control. Since the TG may also be controlled 
    locally it is recommended that the CT also to register for  the event
    avrcp_event_addressed_player_changed. 

    @param avrcp The AVRCP profile instance.
    
    @param player_id Unique Media player identifier, which is known to CT 
                     or received from TG using GetFolderItems command or 
                     EVENT_ADDRESSED_CHANGED_EVENT Notification. 

    @return Message @link 
    AVRCP_SET_ADDRESSED_PLAYER_CFM_T AVRCP_SET_ADDRESSED_PLAYER_CFM
    @endlink  is sent to the application. 
*/
void AvrcpSetAddressedPlayerRequest(AVRCP*  avrcp, uint16  player_id);

/*!
    @brief This function is used by the CT to send PlayItem Command to the 
    TG to start playing an Item indicated by the uid. It is routed to the 
    addressed player at TG. 

    @param avrcp The AVRCP profile instance.
    
    @param scope The scope in which the UID of the media element item or 
    folder item. Scope can be File System, Search Result 
    list or Now Playing list, in which the uid and uid_counter are valid.
    
    @param uid   The UID of the media Item or folder item. Item_list returned
    in AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM will contain the UID. The UID will be
    media element UID or Folder UID depends on the scope of that GetFolderItems
    request.

    @param uid_counter The UID counter value received using 
    AvrcpBrowseGetFolderItemsRequest() or AvrcpBrowseSearchRequest() or through
    the Event Notifications like  AVRCP_EVENT_UIDS_CHANGED_IND or
    AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND. 

    @return Message @link 
    AVRCP_PLAY_ITEM_CFM_T AVRCP_PLAY_ITEM_CFM
    @endlink  is sent to the application. 
*/
void AvrcpPlayItemRequest( AVRCP*              avrcp, 
                    avrcp_browse_scope  scope,    
                    avrcp_browse_uid    uid,  
                    uint16              uid_counter);

/*!
    @brief This function is used by the CT to send AddToNowPlaying Command 
    to the TG for adding an item indicated by the uid to the Now Playing
    queue.
 
    @param avrcp The AVRCP profile instance.

    @param scope The scope in which the UID of the media element item or 
    folder item. Scope can be File System, Search Result 
    list or Now Playing list, in which the uid and uid_counter are valid.
    
    @param uid   The UID of the media Item or folder item. Item_list returned
    in AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM will contain the UID. The UID will be
    media element UID or Folder UID depends on the scope of that GetFolderItems
    request.

    @param uid_counter The UID counter value received using 
    AvrcpBrowseGetFolderItemsRequest or AvrcpBrowseSearchRequest or through
    the Event Notifications like  AVRCP_EVENT_UIDS_CHANGED_IND or
    AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND. 

    @return Message @link 
    AVRCP_ADD_TO_NOW_PLAYING_CFM_T AVRCP_ADD_TO_NOW_PLAYING_CFM
    @endlink  is sent to the application. 
*/

void AvrcpAddToNowPlayingRequest(AVRCP*               avrcp,  
                          avrcp_browse_scope   scope,  
                          avrcp_browse_uid     uid, 
                          uint16               uid_counter);

/*!
    @brief This function is used by the CT to send SetBrowsedPlayer Command 
    to the TG. This command is used to control to which player Browsing 
    commands should be routed. Some players may support browsing only when 
    set as AddressedPlayer. This is shown in the player feature bit mask 
    retrieved using  AvrcpBrowseGetFolderItemsRequest for Media players. 
 
    @param avrcp The AVRCP profile instance.

    @param player_id Unique Media player ID of the Browsed Player.

    @return Message @link 
    AVRCP_BROWSE_SET_PLAYER_CFM_T AVRCP_BROWSE_SET_PLAYER_CFM
    @endlink  is sent to the application. 
*/
void AvrcpBrowseSetPlayerRequest(AVRCP*    avrcp,   
                          uint16    player_id);

/*!
    @brief This function is used by the CT to send ChangePath Command 
    to the TG. This command is used to navigate the virtual file system. 
 
    @param avrcp The AVRCP profile instance.

    @param uid_counter The UID counter value of the virtual File system 
     received using  AvrcpBrowseGetFolderItemsRequest() or 
     AvrcpBrowseSearchRequest() or through the Event Notifications like  
     AVRCP_EVENT_UIDS_CHANGED_IND or AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND. 
   
    @param direction The direction of  browsing.

    @param folder_uid   The UID of the folder item. Item_list returned
    in AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM as a result of 
    AvrcpBrowseGetFolderItemsRequest() for folder items will contain the UID.

    @return Message @link 
    AVRCP_BROWSE_CHANGE_PATH_CFM_T AVRCP_BROWSE_CHANGE_PATH_CFM
    @endlink  is sent to the application. 
*/
void AvrcpBrowseChangePathRequest( AVRCP*                  avrcp,  
                            uint16                  uid_counter,
                            avrcp_browse_direction  direction,
                            avrcp_browse_uid        folder_uid);  


/*!
    @brief This function is used by the CT to send GetItemAttributes Command 
    to the TG. This command is used to retrieve the metadata attributes
    for a particular metadata element item or folder item.
 
    @param avrcp The AVRCP profile instance.

    @param scope The scope in which the UID of the media element item or 
    folder item. Scope can be File System, Search Result 
    list or Now Playing list, in which the uid and uid_counter are valid.

    @param uid   The UID of the folder item or media element item. Item_list
    returned in AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM as a result of 
    AvrcpBrowseGetFolderItemsRequest() for folder items will contain the UID.

    @param uid_counter The UID counter value of the virtual File system 
     received using  AvrcpBrowseGetFolderItemsRequest() or 
     AvrcpBrowseSearchRequest() or through the Event Notifications like  
     AVRCP_EVENT_UIDS_CHANGED_IND or AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND.

    @param num_attr Number of attributes requested. This value shall be set to
     0 while requesting for all attributes or set to 255 while requesting for 
     no attributes.  
  
    @param attr_list  List of Metadata attributes. The library ignores this 
    parameter if num_attr is 0 or 255. Number of bytes in the Source 
    should not be greater than max_browse_data value returned in 
    AVRCP_BROWSE_CONNECT_CFM message. Attribute IDs are valid for browsing 
    the File System for media element Items.All attribute IDs are defined in 
    avrcp.h under "List of Media Attribute definitions".  Use 4 octets for 
    each attribute ID.i.e, size of attr_list  should be num_attr * 4.

    @return Message @link 
    AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM_T AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_CFM
    @endlink  is sent to the application. 
*/
void AvrcpBrowseGetItemAttributesRequest(  AVRCP*               avrcp,
                                    avrcp_browse_scope   scope,  
                                    avrcp_browse_uid     uid,   
                                    uint16               uid_counter, 
                                    uint8                num_attr, 
                                    Source               attr_list);

/*!
    @brief This function is used by the CT to send GetFolderItems Command 
    to the TG. This command is used to retrieve a listing of the contents
    of the folder. 
 
    @param avrcp The AVRCP profile instance.

    @param scope The scope in which the command is requested. 
    Scope can be Media player, File System, Search Result 
    list or Now Playing list.

    @param start The offset within the listing of the item which should be 
    the  first returned item. The first media element item in the listing 
    starts at offset 0

    @param end The offset within the listing of the item which should be the
    final returned item. If this is set to a value beyond what is available 
    the TG shall return items from the provided Start item index to the index
    of the final item. The library returns error if this value is smaller 
    than the Start index. If the application requests too many Items, TG 
    may return a sub set of items. 

    @param num_attr Number of attributes requested. This value shall be set to
     0 while requesting for all attributes or set to 255 while requesting for 
     no attributes.  
  
    @param attr_list  List of Metadata attributes. The library ignores this 
    parameter if num_attr is 0 or 255. Number of bytes in the Source 
    should not be greater than max_browse_data value returned in 
    AVRCP_BROWSE_CONNECT_CFM message. Attribute IDs are valid for browsing 
    the File System for media element Items.All attribute IDs are defined in 
    avrcp.h under "List of Media Attribute definitions".  Use 4 octets for 
    each attribute ID.i.e. size of attr_list  should be num_attr * 4.

    @return Message @link 
    AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM_T AVRCP_BROWSE_GET_FOLDER_ITEMS_CFM
    @endlink  is sent to the application. 

*/

void AvrcpBrowseGetFolderItemsRequest( AVRCP*              avrcp,   
                                avrcp_browse_scope  scope,   
                                uint32              start,   
                                uint32              end,      
                                uint8               num_attr, 
                                Source              attr_list); 

/*!
    @brief This function is used by the CT to send GetTotalNumberOfItems Command 
    to the TG. This command is used to retrieve number of items based on the browsing scope. 
 
    @param avrcp The AVRCP profile instance.

    @param scope The scope in which the command is requested. 
    Scope can be Media player, File System, Search Result 
    list or Now Playing list.

    @return Message @link 
    AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM_T AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_CFM
    @endlink  is sent to the application. 

*/
void AvrcpBrowseGetNumberOfItemsRequest( AVRCP*              avrcp,   
                                avrcp_browse_scope  scope);

/*!
    @brief This function is used by the CT to send Search Command 
    to the TG. This command provides basic search functionality. 
    Regular expressions are not supported in this search. The search 
    applies to the current browsed folder of the Browsed player in the 
    TG and its sub-folders.  
 
    @param avrcp The AVRCP profile instance.

    @param char_set Character Set used for search string. This must be 
    avrcp_char_set_utf_8. All other values are reserved for future.

    @param str_length The length of the search string in octets.

    @param string The string to search in the specified character set.

    @return Message @link 
    AVRCP_BROWSE_SEARCH_CFM_T AVRCP_BROWSE_SEARCH_CFM
    @endlink is sent to the application. 
*/

void AvrcpBrowseSearchRequest( AVRCP*              avrcp,
                        avrcp_char_set      char_set, 
                        uint16              str_length,
                        Source              string);


#endif /* !AVRCP_TG_ONLY_LIB */


#ifndef AVRCP_CT_ONLY_LIB /* TG APIs are not exposed on CT only Lib */

/*!
    @brief Verify the data that was sent.
    @param avrcp The profile instance which will be used.
    @param response The AVRCP response.    

    This function is called on receipt of a message 
    @link AVRCP_PASSTHROUGH_IND_T AVRCP_PASSTHROUGH_IND @endlink.
*/
void AvrcpPassthroughResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief Provide unit information of the device.
    
    @param avrcp The profile instance which will be used.
    @param accept Yes(TRUE) or No(FALSE).
    @param unit_type The unit type.
    @param unit      The unit
    @param company_id The company identifier.

    This function is called on receipt of a message 
    @link AVRCP_UNITINFO_IND_T AVRCP_UNITINFO_IND @endlink.
*/
void AvrcpUnitInfoResponse( AVRCP               *avrcp, 
                            bool                accept, 
                            avc_subunit_type    unit_type, 
                            uint8               unit, 
                            uint32              company_id);

/*!
    @brief Provide information about the subunit(s) of a device.
    
    @param avrcp The profile instance which will be used.

    @param accept Flag accepting or rejecting request for SubUnitInfo.

    @param page_data Four entries from the subunit table for the requested page
    on the target device.

    This function is called on receipt of a message 
    @link AVRCP_SUBUNITINFO_IND_T AVRCP_SUBUNITINFO_IND @endlink.

*/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp,bool accept,const uint8 *page_data);

/*!
    @brief Verify the data that was sent. 

    @param avrcp The profile instance which will be used.
    @param response The AVRCP response.

    This function is called on receipt of a message 
    @link AVRCP_VENDORDEPENDENT_IND_T AVRCP_VENDORDEPENDENT_IND @endlink.
    
*/
void AvrcpVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response);


/*!
    @brief Respond to a request for the supported capabilities.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param caps The type of the requested capabilities (currently only 
    company ID and supported events defined by the specification).

    @param size_caps_list Length, in bytes, of the list of supported
    capabilities.

    @param caps_list The list of supported capabilities.

    This function is called in response to an AVRCP_GET_CAPS_IND message being
    received from the remote device (the controller). If either one or both of
    the size_caps_list and caps_list parameters are set to zero then only the
    mandatory capabilities(as defined in the AVRCP specification) are sent out.
    
    If the client wishes to supply other capabilities then they must be passed 
    to the avrcp library using this function.The client does NOT need to supply
    the mandatory capabilities as defined in the AVRCP specification as those
    will be inserted automatically by the library. If the client does 
    supply other capabilities then they are passed on directly as is.The AVRCP 
    library cannot and does not sanity check them.

    The application owns the caps_list source (if supplied) so the source
    data must be available until all the data has been sent to the remote
    device. Once the data has been sent the source will be emptied by the
    library.

   This function is called on receipt of a message 
    @link AVRCP_GET_CAPS_IND_T AVRCP_GET_CAPS_IND @endlink.

*/
void AvrcpGetCapsResponse(  AVRCP               *avrcp, 
                            avrcp_response_type response,
                            avrcp_capability_id caps, 
                            uint16              size_caps_list, 
                            Source              caps_list);


/*!
    @brief Respond to a request for a list of the player application 
    setting attributes.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_attributes The length of the supplied attributes (in bytes)

    @param attributes The supported attributes.

    This function is called on receipt of a message 
    @link AVRCP_LIST_APP_ATTRIBUTE_IND_T AVRCP_LIST_APP_ATTRIBUTE_IND @endlink.

*/
void AvrcpListAppAttributeResponse(AVRCP                *avrcp,
                                   avrcp_response_type  response, 
                                   uint16               size_attributes,
                                   Source               attributes);

/*!
    @brief Respond to a request for a list of the player application 
    setting attributes.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_values The length of the supplied values (in bytes)

    @param values The supported values.

    This function is called on receipt of a message
    @link AVRCP_LIST_APP_VALUE_IND_T AVRCP_LIST_APP_VALUE_IND @endlink.

*/
void AvrcpListAppValueResponse( AVRCP               *avrcp, 
                                avrcp_response_type response, 
                                uint16              size_values, 
                                Source              values);

/*!
    @brief Response to a request for the current set values for the 
    provided player application  setting attributes list.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param size_values The length of the supplied values (in bytes)

    @param values The list of attribute/value pairs for which the 
    corresponding current set value is requested.

    This function is called on receipt of a message
    @link AVRCP_GET_APP_VALUE_IND_T AVRCP_GET_APP_VALUE_IND @endlink.   

*/
void AvrcpGetAppValueResponse( AVRCP               *avrcp,
                               avrcp_response_type response,
                               uint16              size_values, 
                               Source              values);

/*!
    @brief Response to a request to set the values for the provided player 
    application  setting attributes.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This function is called on receipt of a message
    @link AVRCP_SET_APP_VALUE_IND_T AVRCP_SET_APP_VALUE_IND @endlink.   
*/
void AvrcpSetAppValueResponse( AVRCP *avrcp,
                               avrcp_response_type response);

/*!
    @brief This function is used to return the displayable text for
     provided PlayerAppSettingAttributeIDs.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_attributes Number of attributes for which information
    is returned.

    @param size_attributes The length of the supplied attribute text(in bytes).

    @param attributes The list of attribute text information.

    This function is called on receipt of a message
    @link AVRCP_GET_APP_ATTRIBUTE_TEXT_IND_T AVRCP_GET_APP_ATTRIBUTE_TEXT_IND
    @endlink. 

*/
void AvrcpGetAppAttributeTextResponse(AVRCP *avrcp,
                                avrcp_response_type response,
                                uint16              number_of_attributes,
                                uint16              size_attributes, 
                                Source              attributes);

/*!
    @brief This function is used to return displayable text for provided
    Player Application values.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_values Number of values for which information is returned.

    @param size_values The length of the supplied values (in bytes).

    @param values The list of values for which the text is sent.

    This function is called on receipt of a message
    @link AVRCP_GET_APP_VALUE_TEXT_IND_T AVRCP_GET_APP_VALUE_TEXT_IND
    @endlink. 

*/
void AvrcpGetAppValueTextResponse( AVRCP *avrcp, 
                                   avrcp_response_type response, 
                                   uint16 number_of_values,
                                   uint16 size_values,
                                   Source values );
/*!
    @brief Respond with the element attribute data that was requested
    by the CT.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param number_of_attributes Number of attributes supplied.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    This function is called on receipt of a message
    @link AVRCP_GET_ELEMENT_ATTRIBUTES_IND_T AVRCP_GET_ELEMENT_ATTRIBUTES_IND
    @endlink. 
*/ 
void AvrcpGetElementAttributesResponse(AVRCP *avrcp, 
                                       avrcp_response_type response,
                                       uint16 number_of_attributes,
                                       uint16 size_attributes, 
                                       Source attributes);
/*!
    @brief Used by the TG to respond with the status of the currently
     playing media.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_stable.

    @param song_length The total length of the playing song in milliseconds.

    @param song_elapsed The current position of the playing song in
     milliseconds elapsed.

    @param play_status Current status of playing media.

    This function is called on receipt of a message
    @link AVRCP_GET_PLAY_STATUS_IND_T AVRCP_GET_PLAY_STATUS_IND 
    @endlink. 
*/
void AvrcpGetPlayStatusResponse(AVRCP               *avrcp, 
                                avrcp_response_type response, 
                                uint32              song_length, 
                                uint32              song_elapsed, 
                                avrcp_play_status   play_status);

/*!
    @brief Send event to the CT on playback status changing if the CT 
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param play_status Current play status of TG.

    The TG must call this function when the playback status has changed, 
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_playback_status_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 

*/
void AvrcpEventPlaybackStatusChangedResponse(AVRCP              *avrcp, 
                                            avrcp_response_type response, 
                                            avrcp_play_status   play_status);


/*!
    @brief Send event to the CT on track changing if the CT registered
    to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param track_index_high Top 4 bytes of the currently selected track.

    @param track_index_low Bottom 4 bytes of the currently selected track.

    The TG must call this function when the track has changed, 
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_track_changed.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 

*/
void AvrcpEventTrackChangedResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint32              track_index_high, 
                                    uint32              track_index_low);


/*!
    @brief Send event to the CT on track reaching end if the CT registered
    to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    The TG must call this function when the track has reached the end,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_track_reached_end.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventTrackReachedEndResponse(AVRCP               *avrcp, 
                                       avrcp_response_type response);


/*!
    @brief Send event to the CT on track reaching end if the CT registered
     to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    The TG must call this function when the track has reached the start,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_track_reached_start.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventTrackReachedStartResponse(AVRCP               *avrcp,
                                         avrcp_response_type response);


/*!
    @brief Send event to the CT on playback position changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param playback_pos Current playback position in milliseconds.

    The TG must call this function on changing the playback position,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_playback_pos_changed.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventPlaybackPosChangedResponse(AVRCP                 *avrcp, 
                                          avrcp_response_type   response, 
                                          uint32                playback_pos);


/*!
    @brief Send event to the CT on battery status changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param battery_status The current battery status.

    The TG must call this function on changing the battery status,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_batt_status_changed.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 

*/
void AvrcpEventBattStatusChangedResponse(AVRCP                *avrcp, 
                                         avrcp_response_type  response, 
                                         avrcp_battery_status battery_status);


/*!
    @brief Send event to the CT on system status changing if the CT 
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param system_status The current system status.

    The TG must call this function on changing the system status,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event  avrcp_event_system_status_changed.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventSystemStatusChangedResponse(AVRCP                *avrcp,
                                           avrcp_response_type  response, 
                                           avrcp_system_status  system_status);


/*!
    @brief Send event to the CT on a player app setting changing if the CT
    registered to receive this event.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_interim, avctp_response_changed.

    @param size_attributes The length of the supplied attribute data (in bytes)

    @param attributes The list of attribute data returned in the response.

    The TG must call this function on changing the player application settings,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_player_app_setting_changed.

    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 

*/
void AvrcpEventPlayerAppSettingChangedResponse(AVRCP        *avrcp, 
                                        avrcp_response_type response, 
                                        uint16              size_attributes, 
                                        Source              attributes);

/*!
    @brief This function is used in response to move to the first song 
    in the next group.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected.

    This function is called on receipt of a message 
    @link AVRCP_NEXT_GROUP_IND_T AVRCP_NEXT_GROUP_IND @endlink.
*/
void AvrcpNextGroupResponse(AVRCP *avrcp, avrcp_response_type response);

/*!
    @brief This function is used in response to move to the first song
    in the previous group.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected.

    This function is called on receipt of a message 
    @link AVRCP_PREVIOUS_GROUP_IND_T AVRCP_PREVIOUS_GROUP_IND @endlink.
*/
void AvrcpPreviousGroupResponse(AVRCP *avrcp, avrcp_response_type response);

/*!
    @brief This function is used by TG to respond to CT's inform battery 
    status command.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This function is called on receipt of a message 
    @link AVRCP_INFORM_BATTERY_STATUS_IND_T AVRCP_INFORM_BATTERY_STATUS_IND
    @endlink.
*/
void AvrcpInformBatteryStatusResponse(AVRCP               *avrcp, 
                                      avrcp_response_type response);

/*!
    @brief This function is used by TG to respond to the CT's character 
    set command.

    @param avrcp The profile instance which will be used.

    @param response Response indicating whether request was accepted or
    rejected. The valid responses are: avctp_response_not_implemented, 
    avctp_response_rejected, avctp_response_accepted.

    This function is called on receipt of a message 
    @link AVRCP_INFORM_CHARACTER_SET_IND_T AVRCP_INFORM_CHARACTER_SET_IND
    @endlink.
*/
void AvrcpInformCharacterSetResponse(AVRCP               *avrcp, 
                                     avrcp_response_type response);


/*!
    @brief  This  function is used by TG to restrict metadata response PDU data size
      to  a configured value.
     The default metadata response PDU data size is  AVRCP_AVC_MAX_DATA_SIZE,
     which will be used if metadata response PDU data size is not configured
   
    @param avrcp The profile instance which will be used.

    @param data_size AVRCP metadata response PDU data size which is requested
       data_size should less than or equal to the max AVRCP_AVC_MAX_DATA_SIZE.

     @return TRUE if the data_size is configured successfully else FALSE.
*/
bool AvrcpSetMetadataResponsePDUDataSize(AVRCP                   *avrcp,
                                     uint16 data_size);

/*!
    @brief This function is used by the TG (Category 2) to respond to
    SetAbsoluteVolume request received from CT. If the volume got changed 
    due to this procedure, Application must call 
    AvrcpEventVolumeChangedResponse() if Controller device has registered for 
    avrcp_event_volume_changed notification.

    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    responses are avctp_response_accepted or avctp_response_rejected.

    @param volume Relative volume at TG (Sink device), 0 as Min and 0x7F as
    Maximum.  This API sets the maximum value for this parameter as 0x7F.

    This function is called on receipt of a message 
    @link AVRCP_SET_ABSOLUTE_VOLUME_IND_T AVRCP_SET_ABSOLUTE_VOLUME_IND
    @endlink.   
*/

void AvrcpSetAbsoluteVolumeResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint8               volume);  
/*!
    @brief This function is used by the TG (Category 2) to notify CT on 
    volume change events at TG if CT has registered for 
    avrcp_event_volume_changed event.

    @param avrcp The profile instance.

    @param response Response indicating whether the volume has been changed or
    the notification was rejected. All valid responses are defined in 
    avrcp_response_type. Expected response values avctp_response_changed, 
    avctp_response_rejected.

    @param volume Relative volume at TG (Sink device), 0 as Min and 0x7F as
    Maximum.  This API sets the maximum value for this parameter as 0x7F.

    The TG must call this function when the volume settings has been changed, 
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event  avrcp_event_volume_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
  
*/

void AvrcpEventVolumeChangedResponse(AVRCP               *avrcp, 
                                     avrcp_response_type response,
                                     uint8               volume);
         
/*!
    @brief This function is used by the TG  to respond to
    SetAddressedPlayer request received from CT. If the addressed player has 
    changed due to this procedure, the application must call 
    AvrcpEventAddressedPlayerChangedResponse(), if the Controller has
    registered for the event avrcp_event_addressed_player_changed.

    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    responses are avctp_response_accepted, avctp_response_rejected,  
    avrcp_response_rejected_invalid_player_id, 
    avrcp_response_rejected_no_available_players or 
    avrcp_response_rejected_internal_error.

    This function is called on receipt of a message 
    @link AVRCP_SET_ADDRESSED_PLAYER_IND_T AVRCP_SET_ADDRESSED_PLAYER_IND
    @endlink.  
*/
void AvrcpSetAddressedPlayerResponse(AVRCP*              avrcp,
                                     avrcp_response_type response);

/*!
    @brief This function is used by the TG to notify CT on 
    addressed player change at TG.

    @param avrcp The profile instance.

    @param response Response indicating whether the addressed player 
    has been changed or the notification was rejected. All valid responses 
    are defined in avrcp_response_type. Expected response values 
    avctp_response_changed, avctp_response_interim, 
    avctp_response_rejected. 

    @param player_id The changed player ID

    @param uid_counter  For database aware players, non-zero UID counter 
    shall be incremented every time the TG makes an update which invalidates 
    existing UIDs. For database unaware players this will be set to 0.

    The TG must call this function when the addressed player has changed,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_addressed_player_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventAddressedPlayerChangedResponse(AVRCP*    avrcp, 
                                    avrcp_response_type response, 
                                    uint16              player_id, 
                                    uint16              uid_counter);
/*!
    @brief This function is used by the TG to notify CT on 
    Available players change event at TG.

    @param avrcp The profile instance.

    @param response Response indicating whether the number of available
    players  has been changed or the notification was rejected. All valid
    responses  are defined in avrcp_response_type. Expected response values
    are avctp_response_changed, avctp_response_interim, 
    avctp_response_rejected. 

    The TG must call this function when the available players has changed,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_available_players_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/
void AvrcpEventAvailablePlayersChangedResponse( AVRCP*              avrcp,
                                                avrcp_response_type response);

/*!
    @brief This function is used by the TG to respond to PlayItem request 
    received from the CT. If the UID counter changed on TG after receiving the
    request and before responding to it, Application must return a failure
    response. Application must also return failure response if the requested
    folder is not playable.
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    responses are avctp_response_accepted, avctp_response_rejected, 
    avrcp_response_rejected_uid_not_exist, 
    avrcp_response_rejected_uid_directory,   
    avrcp_response_rejected_media_in_use, 
    avrcp_response_rejected_invalid_scope etc.

    This function is called on receipt of a message 
    @link AVRCP_PLAY_ITEM_IND_T AVRCP_PLAY_ITEM_IND
    @endlink. 
*/
void AvrcpPlayItemResponse( AVRCP*              avrcp,  
                            avrcp_response_type response);


/*!
    @brief This function is used by the TG to respond to AddToNowPlaying 
    received from the CT. If the UID counter changed on TG after receiving the
    request and before responding to it, Application must return a failure
    response. Application must also return failure response if the requested
    folder is not playable.
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    responses are avctp_response_accepted, avctp_response_rejected, 
    avrcp_response_rejected_uid_not_exist,
    avrcp_response_rejected_uid_directory,   
    avrcp_response_rejected_media_in_use, 
    avrcp_response_rejected_invalid_scope ,
    avrcp_response_rejected_play_list_full etc.

    This function is called on receipt of a message 
    @link AVRCP_ADD_TO_NOW_PLAYING_IND_T AVRCP_ADD_TO_NOW_PLAYING_IND
    @endlink.
*/
void AvrcpAddToNowPlayingResponse( AVRCP*              avrcp,  
                                   avrcp_response_type response);

/*!
    @brief This function is used by the TG to notify CT on 
   UIDs  changed event at TG.

    @param avrcp The profile instance.

    @param response Response indicating whether the UID has been changed
    or the notification was rejected. All valid responses 
    are defined in avrcp_response_type. Expected response values are
    avctp_response_changed, avctp_response_interim, 
    avctp_response_rejected. 

    @param uid_counter  For database aware players, non-zero UID counter 
    shall be incremented every time the TG makes an update which invalidates 
    existing UIDs. For database unaware players this will be set to 0.

    The TG must call this function when the  UIDs has been changed,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event  avrcp_event_uids_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/

void AvrcpEventUidsChangedResponse(AVRCP*               avrcp,
                                  avrcp_response_type  response,
                                  uint16               uid_counter);

/*!
    @brief This function is used by the TG to notify CT on Now Playing
    Content Changed Response.

    @param avrcp The profile instance.

    @param response Response indicating whether the Now playing content 
    has been changed at TG or the notification registration has been 
    rejected.  All valid responses are defined in avrcp_response_type.
    Expected response values are avctp_response_changed, 
    avctp_response_interim or avctp_response_rejected. 

    The TG must call this function when the currently playing content
    has been changed on the current media player on TG,
    if message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink has been received from the CT for
    the registration of the event avrcp_event_now_playing_content_changed.
    
    The TG must call this function with the response as avrcp_response_interim
    before sending a final response if there is no event status change 
    immediately on receiving the message @link AVRCP_REGISTER_NOTIFICATION_IND_T 
    AVRCP_REGISTER_NOTIFICATION_IND @endlink. 
*/

void AvrcpEventNowPlayingContentChangedResponse(AVRCP*               avrcp,
                                                avrcp_response_type  response);

/*!
    @brief This function is used by the TG to respond to SetBrowsedPlayer
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_invalid_player_id,
    avrcp_response_rejected_player_not_browsable,
    avrcp_response_rejected_player_not_addressed etc.

    @param uid_counter  For database aware players, non-zero UID counter 
    shall be incremented every time the TG makes an update which invalidates 
    existing UIDs. For database unaware players this will be set to 0.

    @param num_items Number of items in the browsed player's current folder.

    @param char_type Character Set used for Folder names 

    @param folder_depth depth of browsed folder given in "folder_path"
  
    @param  size_folder_path size of folder_path in bytes.

    @param  folder_path  On success this should contain the path to the
    browsed players currently browsed folder. This should be in a
    in pairs of length and name of each folder in the path repeated together
    "folder_depth" times, each pair representing one folder level.
    e.g: folder path A/BC/DEF  is represented in UTF8 as 0x01, 
    'A', 0x02, "BC",0x03, "DEF".  

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_SET_PLAYER_IND_T AVRCP_BROWSE_SET_PLAYER_IND
    @endlink.
*/
void AvrcpBrowseSetPlayerResponse(AVRCP*                avrcp, 
                                  avrcp_response_type   response,
                                  uint16                uid_counter,
                                  uint32                num_items, 
                                  avrcp_char_set        char_type, 
                                  uint8                 folder_depth, 
                                  uint16                size_folder_path,
                                  Source                folder_path);


/*!
    @brief This function is used by the TG to respond to ChangePath
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_invalid_direction, 
    avrcp_response_rejected_not_directory,
    avrcp_response_rejected_uid_not_exist, 
    avrcp_response_rejected_uid_changed etc.

    @param num_items Number of items in the browsed players current folder.

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_CHANGE_PATH_IND_T AVRCP_CHANGE_PATH_IND
    @endlink.
*/

void AvrcpBrowseChangePathResponse(AVRCP*               avrcp,
                                   avrcp_response_type  response,
                                   uint32               num_items);

/*!
    @brief This function is used by the TG to respond to GetItemAttributes
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_uid_not_exist, 
    avrcp_response_rejected_invalid_scope ,
    avrcp_response_rejected_uid_changed etc.

    @param num_attributes Number of attributes values returned

    @param size_attr_list Number of bytes in the attr_value_list

    @param attr_value_list Contains the list of Attribute Value entries. 
    Each attribute value entry contains a 4 octet attribute Id, 2 octets 
    character set value, 2 octets attribute value length followed by a 
    attribute value string. 

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND_T
    AVRCP_BROWSE_GET_ITEM_ATTRIBUTES_IND 
    @endlink.
*/
void AvrcpBrowseGetItemAttributesResponse(AVRCP*              avrcp,
                                          avrcp_response_type response,
                                          uint8               num_attributes,
                                          uint16              size_attr_list,
                                          Source              attr_value_list);

   
/*!
    @brief This function is used by the TG to respond to GetFolderItems
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_invalid_scope ,
    avrcp_response_rejected_out_of_bound ,
    avrcp_response_rejected_no_valid_search_results etc.

    @param uid_counter The UID counter value of the virtual File system. 
    Database unaware players at the TG always return this value as 0. 
    Database aware players increment this value whenever there is any change
    in the data base and notify the CT using EVENT_UIDS_CHANGED notification
    if it is registered with the TG.This shall be a non zero value for 
    database aware players.  The library always set this value as 0 if the
    library is initialised with  non support for Database aware players.

    @param num_items Number of items returned in this listing.

    @param item_list_size Number of bytes in the item_list

    @param item_list List of items retuned. Items include Media player item,
    Folder Items and Media element items. Format of each item is defined in 
    Section 6.10.2.1, 6.10.2.2 and 6.10.2.3 of AVRCP 1.4 specification. 
    The attributes returned with each item shall be the supported attributes
    from the list provided in the attributes from the list provided in the
    attribute list parameter of the request. 

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_GET_FOLDER_ITEMS_IND_T  AVRCP_BROWSE_GET_FOLDER_ITEMS_IND
    @endlink. 
*/
void AvrcpBrowseGetFolderItemsResponse( AVRCP*              avrcp,         
                                        avrcp_response_type response,     
                                        uint16              uid_counter, 
                                        uint16              num_items,  
                                        uint16              item_list_size,
                                        Source              item_list);      


/*!
    @brief This function is used by the TG to respond to GetNumberOfItems
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_invalid_scope ,
    avrcp_response_rejected_out_of_bound ,
    avrcp_response_rejected_no_valid_search_results etc.

    @param uid_counter The UID counter value of the virtual File system. 
    Database unaware players at the TG always return this value as 0. 
    Database aware players increment this value whenever there is any change
    in the data base and notify the CT using EVENT_UIDS_CHANGED notification
    if it is registered with the TG.This shall be a non zero value for 
    database aware players.  The library always set this value as 0 if the
    library is initialised with  non support for Database aware players.

    @param num_items Number of items returned in this listing.

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND_T  AVRCP_BROWSE_GET_NUMBER_OF_ITEMS_IND
    @endlink. 
*/
void AvrcpBrowseGetNumberOfItemsResponse( AVRCP*              avrcp,         
                                        avrcp_response_type response,     
                                        uint16              uid_counter, 
                                        uint32              num_items);

/*!
    @brief This function is used by the TG to respond to Search
    request received from the CT. 
   
    @param avrcp The profile instance.

    @param response Response indicating whether request was accepted or
    rejected. All valid responses are defined in avrcp_response_type. Expected
    common responses are avrcp_response_browsing_success, 
    avrcp_response_rejected_search_not_supported,
    avrcp_response_rejected_search_in_progress,
    avrcp_response_rejected_player_not_addressed etc.

    @param uid_counter The UID counter value of the virtual File system, 
    where the search result is valid. Database unaware players at the TG 
    always return this value as 0. Database aware players increment this 
    value whenever there is any change in the data base and notify the CT
    using EVENT_UIDS_CHANGED notification if it is registered with the TG.
    This shall be a non zero value for  database aware players. The
    library always set this value as 0 if the library is initialised with 
    non support for Database aware players.

    @param num_items Number of items found in the search.

    This function is called on receipt of a message 
    @link AVRCP_BROWSE_SEARCH_IND_T AVRCP_BROWSE_SEARCH_IND
    @endlink.
*/

void AvrcpBrowseSearchResponse(AVRCP*               avrcp,         
                               avrcp_response_type  response,     
                               uint16               uid_counter, 
                               uint32               num_items);   

#endif /* !AVRCP_CT_ONLY_LIB */


#endif /* AVRCP_H_ */

/** @} */
