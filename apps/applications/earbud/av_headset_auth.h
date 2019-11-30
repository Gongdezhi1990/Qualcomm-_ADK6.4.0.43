/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_auth.h
\brief      Header file for authentication and security functions
*/

#ifndef __AV_HEADSET_AUTH_H
#define __AV_HEADSET_AUTH_H

#include <connection.h>



/*! Handler for connection library messages not sent directly

    This function is called to handle any connection library messages sent to
    the application that the authorisation module is interested in. If a message 
    is processed then the function returns TRUE.

    \note Some connection library messages can be sent directly as the 
        request is able to specify a destination for the response.

    \param  id              Identifier of the connection library message 
    \param  message         The message content (if any)
    \param  already_handled Indication whether this message has been processed by
                            another module. The handler may choose to ignore certain
                            messages if they have already been handled.

    \returns TRUE if the message has been processed, otherwise returns the
        value in already_handled
 */
extern bool appAuthHandleConnectionLibraryMessages(MessageId id, Message message, 
                                                   bool already_handled);


#endif
