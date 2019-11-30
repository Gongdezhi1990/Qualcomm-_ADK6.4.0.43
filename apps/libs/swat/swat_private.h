    
/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_private.h

DESCRIPTION
    This file contains data private to the swat library.

*/


#ifndef SWAT_PRIVATE_H_
#define SWAT_PRIVATE_H_

/****************************************************************************
    Header files
*/
#include "swat.h"

/* External lib includes */
#include <bdaddr.h>

/* Firmware */
#include <panic.h>
#include <sink_.h>
#include <message.h>


/****************************************************************************
    Macros
*/
/* Macros to generate SWAT messages */
#define MAKE_SWAT_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
#define MAKE_SWAT_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

/* Macro used to generate debug lib printfs */
#ifdef SWAT_DEBUG_LIB
#include <stdio.h>
#define SWAT_DEBUG(x) printf x
#else
#define SWAT_DEBUG(x)
#endif

#include <stdio.h>


/****************************************************************************
    Library Specific Defines
*/
#define SUBWOOFER_MAJOR_VERSION_LEGACY 0x01
#define SUBWOOFER_MAJOR_VERSION 0x02
#define SUBWOOFER_MINOR_VERSION 0x01

#define SWAT_MSG_BASE           0x0      /* SWAT Profile library private messages */
#define SWAT_COMMAND_BASE       0x1      /* SWAT command ID base (as specified by SWAT specification) */

#define SWAT_MESSAGE_TIMEOUT    D_SEC(8) /* Specifies the RTX_SIG_TIMEOUT (max time remote device has to respond to signalling commands) */

#define DATA_BLOCK              0xffff   /* message blocking mechanism to flow control signalling cmd's */

/* Internal SWAT library messages */
enum
{
    SWAT_INTERNAL_SIGNALLING_CONNECT_REQ = SWAT_MSG_BASE,
    SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ,
    SWAT_INTERNAL_MEDIA_OPEN_REQ,
    SWAT_INTERNAL_MEDIA_START_REQ,
    SWAT_INTERNAL_MEDIA_SUSPEND_REQ,
    SWAT_INTERNAL_MEDIA_CLOSE_REQ,
    SWAT_INTERNAL_SET_VOLUME_REQ,
    SWAT_INTERNAL_MEDIA_WATCHDOG_BASE = (SWAT_INTERNAL_SET_VOLUME_REQ + 7),
    SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE = (SWAT_INTERNAL_MEDIA_WATCHDOG_BASE + 7),
    SWAT_INTERNAL_MESSAGE_TOP = (SWAT_INTERNAL_MEDIA_LL_WATCHDOG_BASE + 7)
};

/* Internal SWAT signalling command library messages */
enum
{
    SWAT_COMMAND_OPEN = SWAT_COMMAND_BASE,
    SWAT_COMMAND_CLOSE,
    SWAT_COMMAND_START,
    SWAT_COMMAND_SUSPEND,
    SWAT_COMMAND_SET_VOLUME,
    SWAT_COMMAND_LATENCY_ADJUST,
    SWAT_COMMAND_CODEC_TYPE,
    SWAT_COMMAND_GET_STATS,
    SWAT_COMMAND_GET_VERSION,
    SWAT_COMMAND_POWER_OFF,
    SWAT_COMMAND_POWER_ON,
    SWAT_COMMAND_SAMPLE_RATE,
    SWAT_COMMAND_GENERAL_REJECT,
    SWAT_COMMAND_DEFAULT
};

/****************************************************************************
    Data structures
*/

/* Enum defining the SWAT signalling Command IDs as specified by the SWAT specification */
typedef enum
{
    SWAT_OPEN = SWAT_COMMAND_BASE,
    SWAT_CLOSE,
    SWAT_START,
    SWAT_SUSPEND,
    SWAT_SET_VOLUME,
    SWAT_LATENCY_ADJUST,
    SWAT_CODEC_TYPE,
    SWAT_GET_STATS,
    SWAT_GET_VERSION,
    SWAT_POWER_OFF,
    SWAT_POWER_ON,
    SWAT_SAMPLE_RATE,
    SWAT_GENERAL_REJECT
} swatCommandId;


/* Enum defining the SWAT message type as specified by the SWAT specification */
typedef enum
{
    SWAT_CMD = 0x0,
    SWAT_RSP = 0x1
} swatMessageType;


/* Enum defining the accept / reject values as specified by the SWAT specification */
typedef enum
{
    SWAT_ACCEPT_MEDIA = 0x1,
    SWAT_REJECT_MEDIA = 0x2
} swatMediaResponse;


/* Enum defining the SWAT signalling channel states */
typedef enum
{
/*00*/  swat_signalling_idle,
/*01*/  swat_signalling_remote_connecting,
/*02*/  swat_signalling_local_connecting,
/*03*/  swat_signalling_local_connecting_xover,
/*04*/  swat_signalling_connected,
/*05*/  swat_signalling_disconnecting
} swatSignallingState;


/* Enum defining the SWAT transport states for opening & closing media connections */
typedef enum
{
/*00*/  swat_transport_idle,        /* Transport is closed */
/*01*/  swat_transport_incoming,    /* Transport is opening; initiated locally */
/*02*/  swat_transport_outgoing,    /* Transport is opening; initiated remotely */
/*03*/  swat_transport_connected,   /* Transport is connected */
/*04*/  swat_transport_closing      /* Transport is closing */
} swatTransportState;


/* Enum defining the type of L2CAP channel for which we're performing
 * an SDP search to retrieve the remote PSM. */
typedef enum
{
    swat_psm_search_sig     = 0x1,
    swat_psm_search_media   = 0x2
} swatPSMSearchType;

/* Structure to store info for each remote device */
typedef struct
{
    bdaddr               bd_addr;                   /* The Bluetooth address for the remote device */
    uint16               id;                        /* Unique ID for the device */
    
    /* Signalling channel data */
    swatSignallingState  signalling_state;          /* Stores the state of the SWAT signalling channel for the device */
    Sink                 signalling_sink;           /* Sink identifying the signalling connection */
    
    /* Media channel data */
    swatMediaState      media_state;                /* Stores the state of the SWAT media channel for the device */
    swatMediaState      media_ll_state;             /* Stores the state of the low latency SWAT media channel for the device */
    swatTransportState  transport_state;            /* Stores the state of the transport channel for the SWAT media channel for the device */
    swatTransportState  transport_ll_state;         /* Stores the state of the transport channel for the SWAT low latency media channel for the device */
    Sink                media_sink;                 /* Sink identifying the media connection (for standard media connections) */
    Sink                media_ll_sink;              /* Sink identifying the media connection (for low latency media connections) */
    
    /* SWAT Control flags (1 word) */
    unsigned  unused:14;
    unsigned  initiated_esco_disconnect:1;          /* Flag set when the local device initiated the eSCO disconnection (to get around the CL having no CL_DM_SYNC_DISCONNECT_CFM message */
    unsigned  disconnecting_signalling:1;           /* Flag set when disconnecting the signalling channel whilst there is/are active media channels */
    
    /* Volume control */
    unsigned    volume:8;                           /* Stores the volume level to the remote device */
    unsigned    sub_trim:8;                         /* Stores the sub trim volume level to the remote device */
    
    uint16      sample_rate;                        /* Stores the current sample rate being used by the device */
    
    uint16      signalling_block;                   /* flag used to control the signalling message flow */

    swatPSMSearchType   psm_search_type;            /* are we searching for a signalling or media remote PSM */
    
    uint8              version_major;               /* subwoofer version number */
    uint8              version_minor;
} remoteDevice;


/* The Subwoofer Audio Transfer Protocol structure */
typedef struct
{
    TaskData        l2cap_task;      /* task handler for the L2CAP messages */
    TaskData        profile_task;    /* task handler for the SWAT messages */
    TaskData        command_task;    /* task handler for signalling command messages */
    Task            clientTask;      /* the client task for sending messages to */
    swat_role       role;            /* The SWAT role either SRC or SNK */
    bool            auto_handle;     /* Flag to indicate whether SWAT library should auto handle messages or leave for the application to handle */
    const sync_config_params   *esco_config;  /* Pointer to the eSCO configuration parameters for Low Latency media connections */
    
    /* Remote device data */
    uint16          max_remote_devs;  /* Max number of remote devices allowed */
    remoteDevice    *remote_devs;     /* Pointer to remote device(s) data */
    
} swatTaskData;

/* Provide a reference for all SWAT lib modules - should not be exposed at API level */
extern swatTaskData * swat;


/****************************************************************************
    Internal library messages
*/

typedef struct
{
    bdaddr          addr;
} SWAT_INTERNAL_SIGNALLING_CONNECT_REQ_T;

typedef struct
{
    uint16          device_id;
} SWAT_INTERNAL_SIGNALLING_DISCONNECT_REQ_T;

typedef struct
{
    uint16          device_id;
    swatMediaType   media_type;
} SWAT_INTERNAL_MEDIA_OPEN_REQ_T;

typedef struct
{
    uint16          device_id;
    swatMediaType   media_type;
} SWAT_INTERNAL_MEDIA_START_REQ_T;


typedef struct
{
    uint16          device_id;
    swatMediaType   media_type;
} SWAT_INTERNAL_MEDIA_SUSPEND_REQ_T;


typedef struct
{
    uint16          device_id;
    unsigned        volume:8;
    unsigned        sub_trim:8;
} SWAT_INTERNAL_SET_VOLUME_REQ_T;


typedef struct
{
    uint16          device_id;
    swatMediaType   media_type;
} SWAT_INTERNAL_MEDIA_CLOSE_REQ_T;


/* Messages used by the signalling command handler */


typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_OPEN_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_CLOSE_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_START_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_SUSPEND_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_SET_VOLUME_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_GENERAL_REJECT_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_DEFAULT_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_LATENCY_ADJUST_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_CODEC_TYPE_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_GET_STATS_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_GET_VERSION_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_POWER_OFF_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_POWER_ON_T;

typedef struct
{
    remoteDevice *  device;
    uint8           data[1];
} SWAT_COMMAND_SAMPLE_RATE_T;

#endif /* SWAT_PRIVATE_H_ */
