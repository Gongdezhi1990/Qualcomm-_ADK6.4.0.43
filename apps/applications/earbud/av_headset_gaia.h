/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       
\brief      Header file for handling the GAIA transport interface


*/

#ifndef _AV_HEADSET_GAIA_H_
#define _AV_HEADSET_GAIA_H_

#ifdef INCLUDE_DFU

#include <gaia.h>


/*! Messages that are sent by the av_headset_gaia module */
typedef enum {
    APP_GAIA_INIT_CFM = AV_GAIA_MESSAGE_BASE,   /*!< Application GAIA module has initialised */
    APP_GAIA_CONNECTED,                         /*!< A GAIA connection has been made */
    APP_GAIA_DISCONNECTED,                      /*!< A GAIA connection has been lost */
    APP_GAIA_UPGRADE_CONNECTED,                 /*!< An upgrade protocol has connected through GAIA */
    APP_GAIA_UPGRADE_DISCONNECTED,              /*!< An upgrade protocol has disconnected through GAIA */
    APP_GAIA_UPGRADE_ACTIVITY,                  /*!< The GAIA module has seen some upgrade activity */
} av_headet_gaia_messages;


/*! Data used by the GAIA module */
typedef struct
{
        /*! Task for handling messaging from upgrade library */
    TaskData        gaia_task;  
        /*! The current transport (if any) for GAIA */
    GAIA_TRANSPORT *transport;
        /*! Whether a GAIA connection is allowed, or will be rejected immediately */
    bool            connections_allowed;
        /*! List of tasks to notify of GAIA activity. */
    TaskList       *client_list;    

} gaiaTaskData;

/*! Get the transport for the current GAIA connection */
#define appGetGaiaTransport()           (appGetGaia()->transport)

/*! Set the transport for the current GAIA connection */
#define appSetGaiaTransport(_transport)  do { \
                                            appGetGaia()->transport = (_transport);\
                                           } while(0)

/*! Initialise the GAIA Module */
extern void appGaiaInit(void);

/*! Add a client to the GAIA module 

    Messages from #av_headet_gaia_messages will be sent to any task
    registered through this API

    \param task Task to register as a client
 */
extern void appGaiaClientRegister(Task task);


/*! \brief Disconnect any active gaia connection
 */
extern void appGaiaDisconnect(void);


/*! \brief Let GAIA know whether to allow any connections

    \param  allow A new gaia connection is allowed
 */
extern void appGaiaAllowNewConnections(bool allow);



#endif /* INCLUDE_DFU */
#endif /* _AV_HEADSET_GAIA_H_ */

