/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_init.h
\brief      Header file for initialisation module
*/

#ifndef AV_HEADSET_INIT_H
#define AV_HEADSET_INIT_H


/*! Value used to reduce the chance that a random RAM value might affect a test
    system reading the initialised flag */
#define APP_INIT_COMPLETED_MAGIC (0x2D)


/*! \brief Initialisation messages */
enum av_headset_init_messages
{
    INIT_CFM = INIT_MESSAGE_BASE, /*!< Confirmation of initialisation completion. */
};

/*! \brief Initialisation module data */
typedef struct
{
    uint8 init_index;       /*!< The index into table of initialisation functions */
    uint8 initialised;      /*!< Flag used to indicate that the full initialisation has completed */
    uint16 id;              /*!< The current expected message id */
#ifdef USE_BDADDR_FOR_LEFT_RIGHT
    uint8 appInitIsLeft:1;  /*!< Set if this is the left earbud */
#endif
} initData;

/*! \brief Initialise init module
    This function is the start of all initialisation.
*/
extern void appInit(void);

/*! \brief Return message ID init module is waiting for.
    \return The message ID.

    This function is called by the main task message handler to determine
    which message the init module is currently waiting for.
*/
extern uint16 appInitWaitingForMessageId(void);

/*! \brief Handle message that the ID init module is waiting for.
    \param id The message ID to handle.
    \param message The message content.

    This function is called by the main task message handler when
    the message the init module is waiting for has been received.
    If there is a handler function for the message it is called.
    The next entry in the init table is then handled.
*/
extern void appInitHandleMessage(MessageId id, Message message);

/*! Query if the initialisation module has completed all initialisation

    \returns TRUE if all the init tasks have completed
 */
#define appInitCompleted()  (appGetInit()->initialised == APP_INIT_COMPLETED_MAGIC)

#endif // AV_HEADSET_INIT_H
