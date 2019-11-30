/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_private.h

DESCRIPTION
    File which defines private data structure of the library
*/


/*!
@file    broadcast_cmd_private.h
@brief   Write a short description about what the module does and how it
         should be used.

         File which defines private data structure of the library
*/


#ifndef BROADCAST_CMD_PRIVATE_
#define BROADCAST_CMD_PRIVATE_

/*! Message IDs for SCM message types used by CSB. */
typedef enum
{
    /*! Reserved for system use. */
    bcmd_msg_id_reserved         = 0x00,
    /*! AFH Update full channel map SCM message. */
    bcmd_scm_msg_id_afh_update_full  = 0x01,
    /*! AFH Update short channel map SCM message. */
    bcmd_scm_msg_id_afh_update_short = 0x02
} bcmd_scm_msg_id;

typedef struct {
     /*! Place holder for Afh update */
    unsigned afh_update_handle;
}broadcaster_t;

/*!Data structure of the broadcast cmd library
 */
typedef struct {
    /*! Task for the Broadcast cmd instance. */
    TaskData lib_task;
    /*! Task of user of the client instance. */
    Task client_task;
    /*! Task to notify of scm disable cfm messages. */
    Task scm_disable_task;
    /*! Broadcaster handles */
    broadcaster_t* broadcaster_handle;
    /* Flag to maintain which role SCM is initialized */
    unsigned role:2;
}broadcast_cmd_t;

/* Message macros */
#define MAKE_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);
#define MAKE_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN);

/*!
    @brief A private utility function to get the instance of the brocast cmd library

    @param None
    @return Valid broadcast cmd library instance, else NULL
*/
broadcast_cmd_t* BroadcastcmdGetInstance(void);

#endif /* BROADCAST_CMD_PRIVATE_ */
