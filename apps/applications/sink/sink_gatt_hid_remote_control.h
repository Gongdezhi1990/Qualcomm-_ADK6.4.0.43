/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_hid_remote_control.h

DESCRIPTION
    Routines to handle the HID Remote Control.
*/

#ifndef _SINK_GATT_HID_REMOTE_CONTROL_H_
#define _SINK_GATT_HID_REMOTE_CONTROL_H_


#if (defined(GATT_ENABLED) && defined(GATT_HID_CLIENT))
#define GATT_HID_REMOTE_CONTROL
#else
#undef GATT_HID_REMOTE_CONTROL
#endif

#ifdef GATT_HID_REMOTE_CONTROL
#include "sink_input_manager.h"
#endif

#include <message.h>
#include <panic.h>
#include <bdaddr.h>

/* Macro used by the HID RC to generate messages to send itself */
#define MAKE_GATT_HID_RC_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/* Remote Control Message Base */
#define GATT_HID_RC_MSG_BASE 0

/* USB HID Main Item Tags  (Ref HID1_11.pdf )*/
#define USB_HID_TAG_INPUT           0x0020  /* 1000 00nn */
#define USB_HID_TAG_OUTPUT          0x0024  /* 1001 00nn */
#define USB_HID_TAG_FEATURE         0x002C  /* 1011 00nn */
#define USB_HID_TAG_COLLECTION      0x0028  /* 1010 00nn */
#define USB_HID_TAG_END_COLLECTION  0x0030  /* 1100 00nn */
/* USB HID Global Item Tags (Ref HID1_11.pdf ) */
#define USB_HID_TAG_USAGE_PAGE     0x0001  /* 0000 01nn */
#define USB_HID_TAG_LOGICAL_MIN     0x0005  /* 0001 01nn */
#define USB_HID_TAG_LOGICAL_MAX     0x0009  /* 0010 01nn */
#define USB_HID_TAG_PHYSICAL_MIN    0x000D  /* 0011 01nn */
#define USB_HID_TAG_PHYSICAL_MAX    0x0011  /* 0100 01nn */
#define USB_HID_TAG_UNIT_EXPONENT   0x0015  /* 0101 01nn */
#define USB_HID_TAG_UNIT            0x0019  /* 0110 01nn */
#define USB_HID_TAG_REPORT_TAG      0x001D  /* 0111 01nn */
#define USB_HID_TAG_REPORT_ID       0x0021  /* 1000 01nn */
#define USB_HID_TAG_REPORT_COUNT    0x0025  /* 1001 01nn */
#define USB_HID_TAG_PUSH            0x0029  /* 1010 01nn */
#define USB_HID_TAG_POP             0x002D  /* 1011 01nn */
/* USB HID Local Item Tags  (Ref HID1_11.pdf ) */
#define USB_HID_TAG_USAGE           0x0002  /* 0000 10nn */
#define USB_HID_TAG_USAGE_MIN       0x0006  /* 0001 10nn */
#define USB_HID_TAG_USAGE_MAX       0x000A  /* 0010 10nn */
#define USB_HID_TAG_DESIGNATOR_IDX  0x000E  /* 0011 10nn */
#define USB_HID_TAG_DESIGNATOR_MIN  0x0012  /* 0100 10nn */
#define USB_HID_TAG_DESIGNATOR_MAX  0x0016  /* 0101 10nn */
#define USB_HID_TAG_STRING_IDX      0x001E  /* 0111 10nn */
#define USB_HID_TAG_STRING_MIN      0x0022  /* 1000 10nn */
#define USB_HID_TAG_STRING_MAX      0x0026  /* 1001 10nn */
#define USB_HID_TAG_DELIMITER       0x002A  /* 1010 10nn */

/* USB HID Collection, End Collection Items (Ref HID1_11.pdf ) */
#define USB_HID_COLLECTION_PHYSICAL         0x00
#define USB_HID_COLLECTION_APPLICATION      0x01
#define USB_HID_COLLECTION_LOGICAL          0x02
#define USB_HID_COLLECTION_REPORT           0x03
#define USB_HID_COLLECTION_ARRAY            0x04
#define USB_HID_COLLECTION_USE_SWITCH       0x05
#define USB_HID_COLLECTION_USE_MODIFIER     0x06
#define USB_HID_COLLECTION_RESERVED_BASE    0x07
#define USB_HID_COLLECTION_RESERVED_TOP     0x7F
#define USB_HID_COLLECTION_VENDOR_BASE      0x80
#define USB_HID_COLLECTION_VENDOR_TOP       0xFF


/* USB Usage Page ID's (Ref HID1_11.pdf ) */
#define USB_USAGE_PAGE_UNKNOWN      (0xFFFF)
#define USB_USAGE_PAGE_CONSUMER     (0x000C)
#define USB_USAGE_PAGE_GAME_CONTROL (0x0005)

#define USB_VENDOR_SPECIFIC_BASE        0xFF00
#define USB_VENDOR_SPECIFIC_TOP         0xFFFF

/* USB Consumer Page Usage Page ID's  (Ref HID1_11.pdf ) */
#define USB_CONSUMER_USAGE_UNASSIGNED          0x0000
#define USB_CONSUMER_USAGE_CONTROL          (0x0001)
#define USB_CONSUMER_USAGE_NUMERIC_KEYPAD      (0x0002)
#define USB_CONSUMER_USAGE_UNKNOWN          (0xFFFF)



#define USB_HID_MAP_ITEM_MAX_SIZE       (5)

#ifdef GATT_HID_REMOTE_CONTROL
/* 
    Defines the data structure of the BLE HID remote control
*/
typedef struct
{ 
    uint16 rc_button_mask; /* Current Button mask for the remote */
    inputState_t rc_button_state:4; /* Button state of the remote */
    unsigned rc_buttons_down:4; /* Buttons down or not */
    unsigned rc_sent_down_notify:1;  /* Flag used when the DOWN notification has already sent in inputMultipleDetect state */
    unsigned _SPARE_:7; /* Spare Bits available */
}gattHidRcDevData_t;
#endif
    
/*
 RC button event messages 
 */
typedef enum
{
    GATT_HID_RC_BUTTON_DOWN_MSG = GATT_HID_RC_MSG_BASE,   /* Sent when a button is pressed */
    GATT_HID_RC_BUTTON_TIMER_MSG, /* Sent when a button duration (SHORT, LONG, VLONG, VVLONG) timer fires */
    GATT_HID_RC_BUTTON_REPEAT_MSG, /* Sent when a button REPEAT timer fires */
    GATT_HID_RC_BUTTON_UP_MSG, /* Sent when a button is released */ 
    GATT_HID_RC_BUTTON_MSG_TOP /* TOP of mssage base, all new messages should be added above this */
}gattHidRcMessageID_t;

/*
    Defines the Timer IDs to differentiate which timer has fired
*/
typedef enum
{
    GATT_HID_RC_TIMER_MULTIPLE = 0,  /* The MULTIPLE timer has fired whilst the button is still pressed (the button down will be considered a single button press) */
    GATT_HID_RC_TIMER_SHORT, /* The SHORT timer has fired whilst the button is still pressed */
    GATT_HID_RC_TIMER_LONG, /* The LONG timer has fired whilst the button is still pressed */
    GATT_HID_RC_TIMER_VLONG, /* The VLONG timer has fired whilst the button is still pressed */
    GATT_HID_RC_TIMER_VVLONG /* The VVLONG timer has fired whilst the button is still pressed */
} gattHidRcTimerID_t;

/* 
    Defines the data structure for BUTTON EVENT messages
*/
typedef struct
{
    uint16 cid; /* Connection ID to identify the RC the button press was generated on */
    uint16 mask; /* Identifies which of the buttons are currently pressed down */
    gattHidRcTimerID_t timer; /* Identifies the button timer that has fired (Only used for BLE_HID_RC_BUTTON_TIMER_MSG) */
} GATT_HID_RC_BUTTON_EVENT_MSG_T;

/****************************************************************************
NAME    
    sinkGattHidRcInit
    
DESCRIPTION
    Initialize the remote controler task
    
PARAMETERS
    void
*/
#ifdef GATT_HID_REMOTE_CONTROL
void sinkGattHidRcInit(void);
#else
#define sinkGattHidRcInit(void) ((void)(0))
#endif


/****************************************************************************
NAME    
    sinkGattHidRcProcessButtonPress
    
DESCRIPTION
    Process the notifications related to HID remote control
    
PARAMETERS
    size            Size fo HID notification
    value           Value in Notification
    cid             Connection ID of remote
*/
#ifdef GATT_HID_REMOTE_CONTROL
void sinkGattHidRcProcessButtonPress(uint16 size,uint8* value,uint16 cid);
#else
#define sinkGattHidRcProcessButtonPress(size,value,cid) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattHidRcResetParser
    
DESCRIPTION
    Initialise the HID report map parser, discarding any incomplete fragment
    
PARAMETERS
     void

*/
#ifdef GATT_HID_REMOTE_CONTROL
void sinkGattHidRcResetParser(void);
#else
#define sinkGattHidRcResetParser(void) ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattHidRcProcessReportMapDescriptor
    
DESCRIPTION
    Process the report map descriptor to indetify the report ID' and consumer report support
    
PARAMETERS
    cid            Connection ID of remote
    size_data      Size fo HID map descriptor
    data           Data for the descriptor
    more_to_come   Size of further data

*/
#ifdef GATT_HID_REMOTE_CONTROL
void sinkGattHidRcProcessReportMapDescriptor(uint16 cid,uint16 size_data,uint8 *data,uint16 more_to_come);
#else
#define sinkGattHidRcProcessReportMapDescriptor(cid,size_data,data,more_to_come)  ((void)(0))
#endif


/****************************************************************************
NAME    
    sinkGattHidRcAddRemote
    
DESCRIPTION
    Add New Remote 
    
PARAMETERS
    cid         Connection ID for new Remote
*/
#ifdef GATT_HID_REMOTE_CONTROL
void sinkGattHidRcAddRemote(uint16 cid);
#else
#define sinkGattHidRcAddRemote(cid) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattHidRcGetRemoteControlCid
    
DESCRIPTION
    This functions find if a remote control is connected and then returns its corresponding CID
    
PARAMETERS
    
RETURNS    
    CID of the remote control link else GATT_CLIENT_INVALID_CID.
    
*/
#ifdef GATT_HID_REMOTE_CONTROL
uint16 sinkGattHidRcGetRemoteControlCid(void);
#else
#define sinkGattHidRcGetRemoteControlCid() (GATT_CLIENT_INVALID_CID)
#endif

/*******************************************************************************
NAME
    sinkGattHidRcGetConnectedRemote
    
DESCRIPTION
    Interface function for getting the connected active remotes for a CID.
    
PARAMETERS
    cid      Connection ID for the remote
    
RETURNS    
    gattHidRcDevData_t Type remote control pointer if there is a remote assiciated with given CID
    
*/
#ifdef GATT_HID_REMOTE_CONTROL
gattHidRcDevData_t * sinkGattHidRcGetConnectedRemote(uint16 cid);
#else
#define sinkGattHidRcGetConnectedRemote(cid) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkGattRemoteControlGetSize
    
DESCRIPTION
    Returns the required size of HRS client if enabled.

*/
#ifdef GATT_HID_REMOTE_CONTROL
#define sinkGattRemoteControlGetSize(void) (sizeof(gattHidRcDevData_t))
#else
#define sinkGattRemoteControlGetSize(void) (0)
#endif

#endif /* _SINK_GATT_HID_REMOTE_CONTROL_H_ */



