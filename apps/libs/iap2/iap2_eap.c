/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_eap.c
    
DESCRIPTION
    This file contains code for the iAP2 External Accessory Protocol functions.
*/

#include <string.h>
#include "iap2.h"
#include "iap2_private.h"
#include "iap2_eap.h"

#define EAP_MAX_PROTOCOLS (10)
#define EAP_MAX_SESSIONS (10)

/* Convert 1-based Protocol ID to 0-based array index */
#define EAP_INDEX(protocol_id) ((protocol_id) - 1)

typedef struct
{
    const char *name;
    iap2_app_match_action_t match_action;
    Task task;
} iap2_eap_t;

typedef struct
{
    iap2_link *link;
    uint16 session_id;
    uint8 protocol_id;
} iap2_session_t;


static iap2_eap_t eap[EAP_MAX_PROTOCOLS];
static iap2_session_t session[EAP_MAX_SESSIONS];
static uint8 num_protocols;

static uint8 findName(const char * const name)
{
    uint8 protocol_id = 1;
    
    while (protocol_id <= num_protocols && strcmp(name, eap[EAP_INDEX(protocol_id)].name))
    {
        ++protocol_id;
    }
    
    if (protocol_id <= num_protocols)
    {
        return protocol_id;
    }
    
    return 0;
}


Task iap2EAGetProtocolTask(uint8 protocol_id)
{
    if (protocol_id > 0 && protocol_id <= num_protocols)
    {
        return eap[EAP_INDEX(protocol_id)].task;
    }
    
    return NULL;
}


static void setProtocolTask(uint8 protocol_id, Task task)
{
    eap[EAP_INDEX(protocol_id)].task = task;
}


uint8 iap2EAGetNumProtocols(void)
{
    return num_protocols;
}


void iap2EASetSessionLink(uint16 session_id, iap2_link *link)
{
    uint16 i;
    
    for (i = 0; i < EAP_MAX_SESSIONS; ++i)
    {
        if (session[i].session_id == session_id)
        {
            session[i].link = link;
            return;
        }
    }
}


void iap2EASetSessionProtocol(uint16 session_id, uint8 protocol_id)
{
    uint16 i;
    
    if (protocol_id == 0)
    {
        for (i = 0; i < EAP_MAX_SESSIONS; ++i)
        {
            if (session[i].session_id == session_id)
            {
                session[i].protocol_id = 0;
                session[i].session_id = 0;
                return;
            }
        }
    }
    else
    {
        for (i = 0; i < EAP_MAX_SESSIONS; ++i)
        {
            if (session[i].session_id == 0 || session[i].session_id == session_id)
            {
                session[i].protocol_id = protocol_id;
                session[i].session_id = session_id;
                return;
            }
        }
    }
}


uint8 iap2EAGetSessionProtocol(uint16 session_id)
{
    uint16 i;
    
    for (i = 0; i < EAP_MAX_SESSIONS; ++i)
    {
        if (session[i].session_id == session_id)
        {
            return session[i].protocol_id;
        }
    }
    
    return 0;
}

uint16 iap2EAGetSessionIdFromLink(iap2_link *link)
{
    uint16 i;
    
    for (i = 0; i < EAP_MAX_SESSIONS; ++i)
    {
        if (session[i].link== link)
        {
            return session[i].session_id;
        }
    }
    return INVALID_SESSION_ID;
}


iap2_app_match_action_t iap2EAGetMatchAction(uint8 protocol_id)
{
    if (protocol_id > 0 && protocol_id <= num_protocols)
    {
        return eap[EAP_INDEX(protocol_id)].match_action;
    }
    
    return iap2_app_match_no_action;   
}


const char *iap2EAGetName(uint8 protocol_id)
{
    if (protocol_id > 0 && protocol_id <= num_protocols)
    {
        return eap[EAP_INDEX(protocol_id)].name;
    }
    
    return NULL;
}


uint8 Iap2EARegisterProtocol(const char * const name, iap2_app_match_action_t match_action)
{
    uint8 protocol_id = findName(name);
    
    if (protocol_id == 0)
    {
        if (num_protocols < EAP_MAX_PROTOCOLS)
        {
            protocol_id = ++num_protocols;

            eap[EAP_INDEX(protocol_id)].name = name;
            eap[EAP_INDEX(protocol_id)].match_action = match_action;
        }
    }
    else if (match_action != iap2EAGetMatchAction(protocol_id))
    {
        protocol_id = 0;
    }
  
    DEBUG_PRINT(("iap2: reg \"%s\" as %u\n", name, protocol_id));
    return protocol_id;
}


void Iap2EARegisterHandler(Task task, uint8 protocol_id)
{
    MAKE_MESSAGE(cfm, IAP2_EA_REGISTER_HANDLER_CFM);
    
    cfm->protocol_id = protocol_id;
    
    if (protocol_id > 0 && protocol_id <= num_protocols && iap2EAGetProtocolTask(protocol_id) == NULL)
    {
        setProtocolTask(protocol_id, task);
        cfm->status = iap2_status_success;
    }
    else
    {
        cfm->status = iap2_status_fail;
    }
    
    MessageSend(task, IAP2_EA_REGISTER_HANDLER_CFM, cfm);
}


void Iap2EAUnregisterHandler(Task task, uint8 protocol_id)
{
    MAKE_MESSAGE(cfm, IAP2_EA_UNREGISTER_HANDLER_CFM);
    
    cfm->protocol_id = protocol_id;
    
    if (iap2EAGetProtocolTask(protocol_id) == task)
    {
        setProtocolTask(protocol_id, NULL);
        cfm->status = iap2_status_success;
    }
    else
    {
        cfm->status = iap2_status_fail;
    }
    
    MessageSend(task, IAP2_EA_UNREGISTER_HANDLER_CFM, cfm);
}

bool Iap2EASendData(uint16 transport_id, const uint8 *data, uint16 size_data)
{
    uint16 i;
    bool sent = FALSE;

    for (i = 0; i < EAP_MAX_SESSIONS; ++i)
    {
        if (session[i].protocol_id == transport_id)
        {
            iap2_status_t status = Iap2EAMessage(session[i].link, session[i].session_id, size_data, data, TRUE);
            sent = status == iap2_status_success;
            break;
        }
    }
    
    return sent;
}

uint16 Iap2EAGetSlackForProtocol(uint8 protocol_id)
{
    uint16 i;
    uint16 slack = 0;
    
    for (i = 0; i < EAP_MAX_SESSIONS; ++i)
    {
        if (session[i].protocol_id == protocol_id)
        {
            slack = Iap2GetSlackForLink(session[i].link);
            break;
        }
    }
    
    return slack;
}
